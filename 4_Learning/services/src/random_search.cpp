#include <iostream>
#include <limits>

#include "random_search.h"

RandomSearch::RandomSearch() = default;

void RandomSearch::init(const Hyperparameters& hyperparams) {
  hyperparams_ = hyperparams;
  // reset optimizer
  current_iteration_ = 0;
  samples_.clear();
  costs_.clear();
  constraints_.clear();
  best_sample_.clear();
  best_cost_ = std::numeric_limits<double>::max();
  best_iter_ = 0;
}

std::vector<double> RandomSearch::getNextSample() {
  std::vector<double> next_sample;
  // generate random number between lower and upper_bound for every parameter
  next_sample.reserve(hyperparams_.param_count);
  for (int i = 0; i < hyperparams_.param_count; i++) {
    next_sample.push_back(
        generateRandomNumber(hyperparams_.lower_bounds[i], hyperparams_.upper_bounds[i]));
  }
  // store sample
  samples_.push_back(next_sample);
  return next_sample;
}

void RandomSearch::setResult(const double cost, const bool constraint_satisfied) {
  costs_.push_back(cost);
  constraints_.push_back(constraint_satisfied);
  // check if cost is smaller
  if (cost < best_cost_ && constraint_satisfied) {
    best_sample_ = samples_.back();  // last added element
    best_cost_ = cost;
    best_iter_ = current_iteration_;
  }
  // print result of exploration and best parameters
  std::cout << "Iteration: " << current_iteration_ << std::endl;
  std::cout << "Parameters: ";
  for (const auto& item : samples_.back()) {
    std::cout << item << ' ';
  }
  std::cout << "Cost: " << cost << " Success:" << static_cast<int>(constraint_satisfied)
            << std::endl;
  std::cout << "Best Parameters: ";
  for (const auto& item : best_sample_) {
    std::cout << item << ' ';
  }
  std::cout << "Best Cost: " << best_cost_ << std::endl;
  std::cout << "=======" << std::endl;

  current_iteration_++;
}

bool RandomSearch::hasFinished() {
  // stop after certain number of iteration
  return current_iteration_ >= hyperparams_.max_iterations;
}

double RandomSearch::generateRandomNumber(const double lower_bound, const double upper_bound) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> dist(lower_bound, upper_bound);
  double random_number = dist(gen);
  return random_number;
}
