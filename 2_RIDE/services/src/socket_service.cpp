#include <array>
#include <cstring>
#include <iostream>
#include <vector>

#include <messages/GetSocketPose.h>
#include <messages/SocketInfo.h>
#include <racecom/racecom.h>

/** Callback for getSocketPose racecom operation. It returns the socket pose.
 */
void getSocketPoseCb(CallHandle h, struct GetSocketPose_request* req, void* /*hint*/) {
  /*Assume that a vision component determines the socket pose and returns this to the
    plug_in_vision statemachine. However, this example only shows how to connect a RaceCom service
    to a RIDE statemachine, the socket pose is hardcoded and no vision is performed here. To
    successfully run this demo, the user must manually obtain and copy actual socket information
    here. The information consists of the cartesian pose which can be obtained by
    moving the robot's end-effector to the socket pose and running following command: `ride service
    echo robot sensor_data` in a terminal. The cartesian pose is located in the O_T_EE entry.
   */
  // pose: column major
  std::array<double, 16> socket_pose = {
      -0.05183746976227782, 0.9983504730999704,   -0.024289019966200448, 0.,
      0.9982147812036288,   0.052514346441068005, 0.028111223837941478,  0.,
      0.02934094053127373,  -0.02278888279214321, -0.9993096497231606,   0.,
      0.6370806581504417,   0.06103184325272377,  0.24091866348352692,   1.};

  // Prepare result
  // First allocate array for pose
  auto* socket_pose_c_array = static_cast<double*>(allocateArray(16, sizeof(double)));
  // ... then copy content from pose to this allocated C-array
  std::copy(socket_pose.begin(), socket_pose.end(), socket_pose_c_array);
  GetSocketPose_result result = {socket_pose_c_array};

  // Return result
  succeedOperation(h, &result);
  free_GetSocketPose_request(req);
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0]
              << "<race-master-ip> <race-master-port> <race-master-network-interface>" << std::endl;
    return EXIT_FAILURE;
  }

  // Register Service
  installSigIntHandler();
  Service* race_service =
      registerService(("tcp://" + std::string(argv[1]) + ":" + std::string(argv[2])).c_str(),
                      ("tcp://" + std::string(argv[3])).c_str(), "socket-pose");
  if (race_service == nullptr) {
    std::cerr << "Could not register service 'socket-pose'." << std::endl;
    return EXIT_FAILURE;
  }

  Publisher* pub = nullptr;
  try {
    // Register operation
    int rc = registerOperation(race_service, "getSocketPose", GetSocketPoseDescriptor(),
                               reinterpret_cast<OperationHandler>(getSocketPoseCb), nullptr);
    if (rc != 0) {
      throw std::runtime_error("Could not register operation 'socket-pose.getSocketPose'.");
    }

    // Register event
    pub = registerEvent(race_service, "socket_info", SocketInfoDescriptor());

    if (pub == nullptr) {
      throw std::runtime_error("Could not register event 'socket-pose.socket_info'.");
    }
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    unregisterService(race_service);
    return EXIT_FAILURE;
  }

  while (!shutdownSignalled()) {
    spinOnce(race_service, 1000);
    // Determine number of sockets, e.g. by a vision component.
    // As we don't do any vision here, the number of sockets is hardcoded.
    int number_of_sockets = 1;
    // create message
    SocketInfo socket_info{.number_of_sockets = number_of_sockets};
    publish(pub, &socket_info);
  }

  unregisterService(race_service);

  return EXIT_SUCCESS;
}
