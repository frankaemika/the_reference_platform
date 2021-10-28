#include <cstring>
#include <iostream>

#include <franka/exception.h>
#include <racecom/racecom.h>

#include <messages/PlugIn.h>

#include <plug_in_controller.h>

struct FCIParams {
  std::string robot_ip;
};

void plugInCb(CallHandle h, struct PlugIn_request* req, void* hint) {
  auto* fci_params = static_cast<FCIParams*>(hint);

  // required convertion
  std::array<double, 3> target_position, tolerance;
  std::copy(req->target_position, req->target_position + 3, target_position.begin());
  std::copy(req->tolerance, req->tolerance + 3, tolerance.begin());

  plug_in_controller::Parameters plug_in_params = {
      req->plug_in_params.translational_stiffness, req->plug_in_params.rotational_stiffness,
      req->plug_in_params.desired_force,           req->plug_in_params.wiggle_frequency_x,
      req->plug_in_params.wiggle_frequency_y,      req->plug_in_params.wiggle_amplitude_x,
      req->plug_in_params.wiggle_amplitude_y};

  // run the plug in controller with the desired parameters
  try {
    plug_in_controller::run(fci_params->robot_ip, plug_in_params, req->duration, target_position,
                            tolerance);

  } catch (const franka::Exception& ex) {
    std::cerr << ex.what() << std::endl;
    PlugIn_error error = strdup(ex.what());
    failOperation(h, &error);
    free_PlugIn_request(req);
    return;
  }
  succeedOperation(h, {});
  free_PlugIn_request(req);
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0]
              << "<race-master-ip> <race-master-port> <race-master-network-interface>" << std::endl;
    return EXIT_FAILURE;
  }

  FCIParams fci_params{argv[1]};

  installSigIntHandler();
  Service* race_service =
      registerService(("tcp://" + std::string(argv[1]) + ":" + std::string(argv[2])).c_str(),
                      ("tcp://" + std::string(argv[3])).c_str(), "fci");
  if (race_service == nullptr) {
    std::cerr << "Unable to register RaceCom Service: fci" << std::endl;
    return EXIT_FAILURE;
  }

  try {
    int rc = registerOperation(race_service, "plugIn", PlugInDescriptor(),
                               reinterpret_cast<OperationHandler>(plugInCb), &fci_params);
    if (rc != 0) {
      throw std::runtime_error("Could not register operation 'fci.plugIn'.");
    }

  } catch (std::exception const& e) {
    std::cerr << e.what() << std::endl;
    unregisterService(race_service);
    return EXIT_FAILURE;
  }

  spin(race_service);

  unregisterService(race_service);

  return EXIT_SUCCESS;
}
