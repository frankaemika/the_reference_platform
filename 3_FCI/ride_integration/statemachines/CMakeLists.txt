cmake_minimum_required(VERSION 3.5)

project(fci_tutorial_statemachines)

add_custom_target(
  remove_fci_tutorial_statemachines
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/
  COMMAND ${CMAKE_CURRENT_LIST_DIR}/cleanup.sh
)

add_custom_target(
  fci_tutorial_statemachines
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/
  COMMAND ${CMAKE_CURRENT_LIST_DIR}/cleanup.sh
  COMMAND make
)
