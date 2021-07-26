#pragma once

#include <random>
#include <vector>

/**
 * @brief A structure representing hyperparameters for random search
 */
struct Hyperparameters {
  /** Maximum number of iterations*/
  int max_iterations;
  /** Number of parameters being optimized*/
  int param_count;
  /** Lower boundaries for each parameter being optimized*/
  std::vector<double> lower_bounds;
  /** Upper boundaries for each parameter being optimized*/
  std::vector<double> upper_bounds;
};

/**
 * @brief This class implements random search.
 */
class RandomSearch {
 public:
  RandomSearch();
  /**
   * @brief This method initializes random search by setting hyperparameters and
   * resetting the optimization.
   * @param hyperparams Hyperparmeters of random search
   */
  void init(const Hyperparameters& hyperparams);
  /**
   * @brief This method returns a vector of random sampled parameters.
   * @return parameters for the next experimential trial
   */
  std::vector<double> getNextSample();
  /**
   * @brief This method sets a cost value and an indicator if the execution was
   * successfull.
   * @param cost value of the cost function
   * @param contraint_statisfied boolean specifying if the constraint has been satisfied
   */
  void setResult(const double cost, const bool constraint_satisfied);
  /**
   * @brief This method returns true when the maximum number of iteration are reached.
   */
  bool hasFinished();
  /**
   * @brief This method returns the so far best sample
   * @return so far best sample minimizing the cost function and satisfying the constraint
   * (successful execution).
   */
  std::vector<double> getBestSample() const { return best_sample_; };
  /**
   * @brief This method returns the hyperparameters of random search.
   * @return hyperparameters of random search
   */
  Hyperparameters hyperParameters() const { return hyperparams_; }

 private:
  /**
   * @brief This method returns a random scalar between lower and upper bound
   * @param lower_bound
   * @param upper_bound
   */
  double generateRandomNumber(const double lower_bound, const double upper_bound);

  Hyperparameters hyperparams_;
  std::vector<std::vector<double>> samples_;
  std::vector<double> costs_;
  std::vector<bool> constraints_;
  std::vector<double> best_sample_;
  double best_cost_;
  int current_iteration_;
  int best_iter_;
};
