#include <cstring>
#include <iostream>

#include <messages/GetParams.h>
#include <messages/Init.h>
#include <messages/SetResult.h>
#include <racecom/racecom.h>

#include <random_search.h>

/**
 * @brief Helper function to convert a c_array to a vector
 */
template <typename T>
std::vector<T> cArrayToVector(int size, T* c_array) {
  return std::vector<T>(c_array, c_array + size);
}

/**
 * @brief Helper function to convert a vector to a c_array
 */
template <typename T>
T* vectorToCArray(std::vector<T> vec) {
  auto* c_array = static_cast<T*>(allocateArray(static_cast<int>(vec.size()), sizeof(double)));
  std::copy(vec.begin(), vec.end(), c_array);
  return c_array;
}

/**
 * @brief Callback for init racecom operation. It calls init method of random search
 * class.
 */
void initCb(CallHandle h, struct Init_request* req, void* hint) {
  auto* random_search = static_cast<RandomSearch*>(hint);
  Hyperparameters hyperparams;
  hyperparams.max_iterations = req->max_iteration;
  hyperparams.param_count = req->param_count;
  hyperparams.lower_bounds = cArrayToVector<double>(req->param_count, req->lower_bounds);
  hyperparams.upper_bounds = cArrayToVector<double>(req->param_count, req->upper_bounds);
  random_search->init(hyperparams);
  succeedOperation(h, {});
  free_Init_request(req);
}

/**
 * @brief Callback for get_param racecom operation. It returns an array of
 * random sampled parameters by calling getNextSample method of random search class.
 */
void getParamsCb(CallHandle h, struct GetParams_request* req, void* hint) {
  auto* random_search = static_cast<RandomSearch*>(hint);
  std::vector<double> next_sample = random_search->getNextSample();
  GetParams_result result{vectorToCArray<double>(next_sample)};
  succeedOperation(h, &result);
  free_GetParams_request(req);
}

/**
 * @brief Callback for set_result racecom operation. It sets the result of
 * the execution for the random search class.
 */
void setResultCb(CallHandle h, struct SetResult_request* req, void* hint) {
  auto* random_search = static_cast<RandomSearch*>(hint);
  random_search->setResult(req->cost, req->success);
  bool stop = random_search->hasFinished();
  succeedOperation(h, &stop);
  free_SetResult_request(req);
}

/**
 * @brief Callback for get_best_params operation. It returns an array of the best
 * observed parameters.
 */
void getBestParamsCb(CallHandle h, struct GetParams_request* req, void* hint) {
  auto* random_search = static_cast<RandomSearch*>(hint);
  std::vector<double> best_params = random_search->getBestSample();
  if (best_params.empty()) {
    GetParams_error error = strdup("App hasn't been learned so far!");
    failOperation(h, &error);
    free_GetParams_request(req);
    return;
  }
  GetParams_result result{vectorToCArray<double>(best_params)};
  succeedOperation(h, &result);
  free_GetParams_request(req);
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0]
              << "<race-master-ip> <race-master-port> <race-master-network-interface>" << std::endl;
    return EXIT_FAILURE;
  }

  RandomSearch random_search;

  // Register Service
  installSigIntHandler();
  Service* race_service =
      registerService(("tcp://" + std::string(argv[1]) + ":" + std::string(argv[2])).c_str(),
                      ("tcp://" + std::string(argv[3])).c_str(), "learning");
  if (race_service == nullptr) {
    std::cerr << "Unable to register RaceCom Service: learning" << std::endl;
    return EXIT_FAILURE;
  }

  // Register all operations
  try {
    int rc = registerOperation(race_service, "init", InitDescriptor(),
                               reinterpret_cast<OperationHandler>(initCb), &random_search);
    if (rc != 0) {
      throw std::runtime_error("Could not register operation 'learning.init'.");
    }

    rc = registerOperation(race_service, "getParams", GetParamsDescriptor(),
                           reinterpret_cast<OperationHandler>(getParamsCb), &random_search);
    if (rc != 0) {
      throw std::runtime_error("Could not register operation 'learning.getParams'.");
    }

    rc = registerOperation(race_service, "setResult", SetResultDescriptor(),
                           reinterpret_cast<OperationHandler>(setResultCb), &random_search);
    if (rc != 0) {
      throw std::runtime_error("Could not register operation 'learning.setResult'.");
    }

    rc = registerOperation(race_service, "getBestParams", GetParamsDescriptor(),
                           reinterpret_cast<OperationHandler>(getBestParamsCb), &random_search);

    if (rc != 0) {
      throw std::runtime_error("Could not register operation 'learning.getBestParams'.");
    }

  } catch (std::exception const& e) {
    std::cerr << e.what() << std::endl;
    unregisterService(race_service);
    return EXIT_FAILURE;
  }

  // Start event loop
  spin(race_service);

  unregisterService(race_service);

  return EXIT_SUCCESS;
}
