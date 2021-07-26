cmake_minimum_required(VERSION 3.5)

project(fci_tutorial)

option(BUILD_FCI_STANDALONE "Build fci standalone tutorial" ON)
option(BUILD_FCI_INTEGRATION "Build fci integration tutorial" ON)

if(BUILD_FCI_STANDALONE OR BUILD_FCI_INTEGRATION)
  add_subdirectory(standalone)
endif()

if(BUILD_FCI_INTEGRATION)
  add_subdirectory(ride_integration/services)
  add_subdirectory(ride_integration/statemachines)
endif()
