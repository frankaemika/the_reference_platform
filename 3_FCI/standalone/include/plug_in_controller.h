#include <array>
#include <string>

namespace plug_in_controller {

/**
 * @brief This struct defines the parameters of the plug in controller.
 */
struct Parameters {
  /**Stiffness in x,y,z direction of the end effector [N/m], Bounded: [0, 2000] N/m */
  double translational_stiffness;
  /**Stiffness around x,y,z axis of the end effector [N/rad], Bounded: [0, 300] N/m */
  double rotational_stiffness;
  /**Forces applied in the end effectors z direction [N] */
  double desired_force;
  /** Frequency of the wiggle in end effectors x direction [Hz] */
  double wiggle_frequency_x;
  /** Frequency of the wiggle in end effectors y direction [Hz] */
  double wiggle_frequency_y;
  /** Amplitude of the wiggle in end effectors x direction [m] */
  double wiggle_amplitude_x;
  /** Amplitude of the wiggle in end effectors y direction [m] */
  double wiggle_amplitude_y;
};

/**
 * @brief This function runs the plug in controller
 *
 * @param robot_ip IP of the robot
 * @param plug_in_param Parameters of the plug in controller
 * @param duration Amount of time the plug in controller should run. [s]
 * @param target_position_array Array defining the target position.  When this position is reached
 * within an tolerance the controller stops. [m]
 * @param target_tolerance_array Array specifying the maximum tolerance. If the end effector reaches
 * the target position within this tolerance the controller stops. [m]
 */
void run(const std::string& robot_ip,
         const Parameters plug_in_params,
         const double duration,  // in seconds
         const std::array<double, 3> target_position_array,
         const std::array<double, 3> target_tolerance_array);
}  // namespace plug_in_controller
