#include <cmath>
#include <functional>
#include <iostream>

#include <Eigen/Dense>

#include <franka/duration.h>
#include <franka/exception.h>
#include <franka/model.h>
#include <franka/robot.h>

#include <plug_in_controller.h>

namespace plug_in_controller {

void run(const std::string& robot_ip,
         const Parameters plug_in_params,
         const double duration,  // in seconds
         const std::array<double, 3> target_position_array,
         const std::array<double, 3> target_tolerance_array) {
  // Parameters for stopping criterion
  Eigen::Vector3d target_position(Eigen::Vector3d::Map(target_position_array.data()));
  Eigen::Vector3d target_tolerance(Eigen::Vector3d::Map(target_tolerance_array.data()));

  // Compliance parameters
  Eigen::MatrixXd cartesian_stiffness(6, 6), cartesian_damping(6, 6);
  cartesian_stiffness.setZero();
  cartesian_stiffness.topLeftCorner(3, 3)
      << plug_in_params.translational_stiffness * Eigen::MatrixXd::Identity(3, 3);
  cartesian_stiffness.bottomRightCorner(3, 3)
      << plug_in_params.rotational_stiffness * Eigen::MatrixXd::Identity(3, 3);
  cartesian_damping.setZero();
  cartesian_damping.topLeftCorner(3, 3)
      << 2.0 * sqrt(plug_in_params.translational_stiffness) * Eigen::MatrixXd::Identity(3, 3);
  cartesian_damping.bottomRightCorner(3, 3)
      << 2.0 * sqrt(plug_in_params.rotational_stiffness) * Eigen::MatrixXd::Identity(3, 3);

  // Force parameters
  Eigen::VectorXd force_error_integral(6);
  // force control P, I gain
  double k_p{0.0};
  double k_i{1.0};

  franka::Robot robot(robot_ip);
  // load the kinematics and dynamics model
  franka::Model model = robot.loadModel();

  // initial robot state
  franka::RobotState initial_state = robot.readOnce();
  Eigen::Map<Eigen::Matrix<double, 6, 1>> force_initial(initial_state.O_F_ext_hat_K.data());
  force_error_integral.setZero();

  // equilibrium point is the initial position
  Eigen::Affine3d initial_transform(Eigen::Matrix4d::Map(initial_state.O_T_EE.data()));
  Eigen::Vector3d position_d(initial_transform.translation());
  Eigen::Quaterniond orientation_initial(initial_transform.linear());

  double time = 0.0;

  // define callback for the torque control loop
  std::function<franka::Torques(const franka::RobotState&, franka::Duration)>
      plug_in_control_callback =
          [&](const franka::RobotState& robot_state, franka::Duration period) -> franka::Torques {
    time += period.toSec();

    // get state variables
    std::array<double, 7> coriolis_array = model.coriolis(robot_state);
    std::array<double, 42> jacobian_array =
        model.zeroJacobian(franka::Frame::kEndEffector, robot_state);

    // convert to Eigen
    Eigen::Map<const Eigen::Matrix<double, 7, 1>> coriolis(coriolis_array.data());
    Eigen::Map<const Eigen::Matrix<double, 6, 7>> jacobian(jacobian_array.data());
    Eigen::Map<const Eigen::Matrix<double, 6, 1>> force(robot_state.O_F_ext_hat_K.data());
    Eigen::Map<const Eigen::Matrix<double, 7, 1>> dq(robot_state.dq.data());
    Eigen::Affine3d transform(Eigen::Matrix4d::Map(robot_state.O_T_EE.data()));
    Eigen::Vector3d position(transform.translation());
    Eigen::Quaterniond orientation(transform.linear());

    // Compute desired orientation of equilibrium pose and desired forces
    // wiggle motion
    Eigen::AngleAxisd angle_axis_wiggle_x;
    angle_axis_wiggle_x.axis() << 1, 0, 0;
    angle_axis_wiggle_x.angle() = sin(2.0 * M_PI * time * plug_in_params.wiggle_frequency_x) *
                                  plug_in_params.wiggle_amplitude_x;
    Eigen::AngleAxisd angle_axis_wiggle_y;
    angle_axis_wiggle_y.axis() << 0, 1, 0;
    angle_axis_wiggle_y.angle() = sin(2.0 * M_PI * time * plug_in_params.wiggle_frequency_y) *
                                  plug_in_params.wiggle_amplitude_y;
    Eigen::Quaterniond wiggle_x(angle_axis_wiggle_x);
    Eigen::Quaterniond wiggle_y(angle_axis_wiggle_y);
    Eigen::Quaterniond orientation_d(wiggle_y * (wiggle_x * orientation_initial));

    // desired forces
    Eigen::VectorXd desired_force(6);
    desired_force.setZero();
    desired_force(2) = -plug_in_params.desired_force;

    // Compute error to desired equilibrium pose
    // position error
    Eigen::Matrix<double, 6, 1> error;
    error.head(3) << position - position_d;

    // orientation error
    if (orientation_d.coeffs().dot(orientation.coeffs()) < 0.0) {
      orientation.coeffs() << -orientation.coeffs();
    }
    // "difference" quaternion
    Eigen::Quaterniond error_quaternion(orientation.inverse() * orientation_d);
    error.tail(3) << error_quaternion.x(), error_quaternion.y(), error_quaternion.z();
    // Transform to base frame
    error.tail(3) << -transform.linear() * error.tail(3);

    // Compute error to desired force removing initial bias
    Eigen::VectorXd force_error;
    force_error = desired_force - force + force_initial;
    force_error_integral = force_error_integral + period.toSec() * force_error;

    // Compute control
    Eigen::VectorXd force_control(6), tau_force(7), tau_cart(7), tau_cmd(7);

    // Force control term
    force_control = desired_force + k_p * force_error + k_i * force_error_integral;
    force_control << 0, 0, force_control(2), 0, 0, 0;
    tau_force = jacobian.transpose() * force_control;

    // Cartesian control term
    tau_cart << jacobian.transpose() *
                    (-cartesian_stiffness * error - cartesian_damping * (jacobian * dq));

    // Commanded torque
    tau_cmd << tau_cart + tau_force + coriolis;

    std::array<double, 7> tau_d_array{};
    Eigen::VectorXd::Map(&tau_d_array[0], 7) = tau_cmd;

    // check if motion is completed
    Eigen::Matrix<double, 3, 1> target_error((target_position - position).cwiseAbs());
    bool is_inserted =
        (target_error[0] <= target_tolerance[0] && target_error[1] <= target_tolerance[1] &&
         target_error[2] <= target_tolerance[2]);
    if (time <= duration && is_inserted) {
      return franka::MotionFinished(franka::Torques(tau_d_array));
    }
    if (time > duration) {
      throw franka::Exception("Timeout: Plug in controller failed!");
    }
    return tau_d_array;
  };

  robot.control(plug_in_control_callback);
}

}  // namespace plug_in_controller
