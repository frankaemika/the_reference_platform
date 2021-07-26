cmake_minimum_required(VERSION 3.5)

project(tutorial)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_RIDE "Build RIDE tutorial" ON)
option(BUILD_FCI "Build FCI tutorial" ON)
option(BUILD_LEARNING "Build LEARNING tutorial" ON)

if(BUILD_RIDE)
  add_subdirectory(2_RIDE)
endif()

if(BUILD_FCI)
  add_subdirectory(3_FCI)
endif()

if(BUILD_LEARNING)
  add_subdirectory(4_Learning)
endif()
