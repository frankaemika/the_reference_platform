cmake_minimum_required(VERSION 3.5)

project(fci_standalone_tutorial)

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_LIST_DIR}/../../cmake)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Eigen3 REQUIRED)
find_package(Franka)

add_library(plug_in_controller)
target_include_directories(
  plug_in_controller
  PUBLIC
    include
    ${Franka_INCLUDE_DIRS}
)
target_link_libraries(
  plug_in_controller
  PRIVATE
    Eigen3::Eigen3
    ${Franka_LIBRARIES}
)
target_sources(plug_in_controller PRIVATE src/plug_in_controller)

add_executable(plug_in_controller_standalone src/plug_in_controller_standalone.cpp)
target_link_libraries(plug_in_controller_standalone PRIVATE plug_in_controller)

# Tools
include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/ClangTools.cmake OPTIONAL RESULT_VARIABLE CLANG_TOOLS)
if(CLANG_TOOLS)
    file(
        GLOB_RECURSE
            SOURCES
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    )
    file(
        GLOB_RECURSE
            HEADERS
            ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/include/*/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    )
    add_format_target(${PROJECT_NAME} FILES ${SOURCES} ${HEADERS})
    add_tidy_target(${PROJECT_NAME} FILES ${SOURCES})
endif()
