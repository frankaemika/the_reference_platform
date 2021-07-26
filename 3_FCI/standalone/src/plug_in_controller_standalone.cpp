#include <iostream>

#include <franka/exception.h>
#include <franka/robot.h>

#include <plug_in_controller.h>

int main(int argc, char** argv) {
  // Check whether the required arguments were passed
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <robot-hostname>" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "WARNING: Collision thresholds are set to high values. "
            << "Make sure you have the user stop at hand!" << std::endl
            << "WARNING: Make sure that end effector is in contact with a horizontal rigid surface "
               "before starting."
            << std::endl
            << "Press Enter to continue..." << std::endl;
  std::cin.ignore();
  try {
    {
      franka::Robot robot(argv[1]);
      robot.setCollisionBehavior({{100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0}},
                                 {{100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0}},
                                 {{100.0, 100.0, 100.0, 100.0, 100.0, 100.0}},
                                 {{100.0, 100.0, 100.0, 100.0, 100.0, 100.0}});
    }
    // some hardcode parameters
    plug_in_controller::Parameters params = {1000.0, 30.0, 3.0, 0.8, 0.5, 0.5, 0.8};
    double duration = 5.0;  // in [s]
    std::array<double, 3> target_position_array = {0., 0., 0.};
    std::array<double, 3> tolerance_position_array = {0., 0., 0.};
    plug_in_controller::run(argv[1], params, duration, target_position_array,
                            tolerance_position_array);
  } catch (const franka::Exception& ex) {
    std::cerr << ex.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
