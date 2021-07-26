cmake_minimum_required(VERSION 3.5)

project(ride_tutorial_statemachines)

add_custom_target(
  remove_ride_tutorial_statemachines
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/
  COMMAND ${CMAKE_CURRENT_LIST_DIR}/cleanup.sh
)

add_custom_target(
  ride_tutorial_statemachines
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/
  COMMAND ${CMAKE_CURRENT_LIST_DIR}/cleanup.sh
  COMMAND make
)
