cmake_minimum_required(VERSION 3.5)

project(learning_tutorial_services)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(RaceCom)

racecom_message_library(
  TARGET learning_messages
  DIRECTORY msg
  STATIC)

add_library(optimizer)
target_include_directories(
  optimizer
  PUBLIC
    include/
)
target_sources(
  optimizer
  PRIVATE
    src/random_search.cpp
)

add_executable(learning_service src/learning_service.cpp)
target_include_directories(
  learning_service
  PRIVATE
    ${RACECOM_INCLUDE_DIR}
)
target_link_libraries(
  learning_service
  PRIVATE
    ${RACECOM_LIBRARY}
    optimizer
    learning_messages
)

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
