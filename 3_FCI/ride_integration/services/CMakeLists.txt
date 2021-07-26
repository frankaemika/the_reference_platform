cmake_minimum_required(VERSION 3.5)

project(fci_tutorial_services)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(RaceCom)

racecom_message_library(
  TARGET fci_messages
  DIRECTORY msg
  STATIC)

add_executable(fci_service src/fci_service.cpp)
target_include_directories(
  fci_service
  PRIVATE
    ${RACECOM_INCLUDE_DIR}
)
target_link_libraries(
  fci_service
  PRIVATE
    ${RACECOM_LIBRARY}
    plug_in_controller
    fci_messages
)

# Tools
include(${CMAKE_CURRENT_LIST_DIR}/../../../cmake/ClangTools.cmake OPTIONAL RESULT_VARIABLE CLANG_TOOLS)
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
