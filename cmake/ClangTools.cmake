include(CMakeParseArguments)

find_program(CLANG_FORMAT_PROG clang-format-6.0 DOC "'clang-format' executable")
if(CLANG_FORMAT_PROG AND NOT TARGET format)
  add_custom_target(format)
  add_custom_target(check-format)
endif()
find_program(CLANG_TIDY_PROG clang-tidy-6.0 DOC "'clang-tidy' executable")
if(CLANG_TIDY_PROG AND NOT TARGET tidy)
  if(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
    message(WARNING "Invoke Catkin/CMake with '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'
                     to generate compilation database for 'clang-tidy'.")
  endif()

  add_custom_target(tidy)
  add_custom_target(check-tidy)
endif()
find_program(AUTOPEP8_PROG autopep8 DOC "'autopep8' executable")
if(AUTOPEP8_PROG AND NOT TARGET pep8)
  add_custom_target(pep8)
endif()
find_package(Git QUIET)
if(NOT GIT_FOUND)
  message(WARNING "Git cannot be found. ")
endif()
find_program(BASENAME_PROG basename DOC "'basename' executable")
if(NOT BASENAME_PROG)
  message(WARNING "basename executable cannot be found. ")
endif()
find_program(XARGS_PROG xargs DOC "'xargs' executable")
if(NOT XARGS_PROG)
  message(WARNING "xargs executable cannot be found. ")
endif()
find_program(PARALLEL_PROG parallel DOC "'parallel' executable")
if(PARALLEL_PROG)
  set(PARALLEL_DELIMITER ":::")
else()
  message(WARNING "parallel executable cannot be found. Try with 'sudo apt install parallel'.")
  set(PARALLEL_PROG "")
endif()

# Starting at SEARCH_DIR, look up directory hierarchy for directory containing NAME.
# SEARCH_DIR is a directory. Stop at the first parent directory containing a folder NAME,
# and return the directory in the parent scope in FOUND_DOMINATING_DIR.  Return empty if not found.
#
# USAGE:
#     locate_dominating_dir(
#         NAME               <dir name>
#         SEARCH_DIR         <dir path>
#     )
#
# ARGUMENTS:
#     NAME
#         name of the directory to be found
#     SEARCH_DIR
#         directory path where the search starts
#
# OUTPUT VARIABLES:
#     FOUND_DOMINATING_DIR
#         path to the directory where NAME is first found
#
# EXAMPLE:
#     locate_dominating_dir(
#         NAME               .git
#         SEARCH_URL         ${CMAKE_CURRENT_LIST_DIR}
#     )
#
function(locate_dominating_dir)
  cmake_parse_arguments(ARG "" "NAME;SEARCH_DIR" "" ${ARGN})
  if(IS_DIRECTORY ${ARG_SEARCH_DIR}/${ARG_NAME})
    set(FOUND_DOMINATING_DIR "${ARG_SEARCH_DIR}" PARENT_SCOPE)
  elseif(EXISTS ${ARG_SEARCH_DIR}/..)
    locate_dominating_dir(
      NAME ${ARG_NAME}
      SEARCH_DIR ${ARG_SEARCH_DIR}/..
    )
    set(FOUND_DOMINATING_DIR ${FOUND_DOMINATING_DIR} PARENT_SCOPE)
  endif()
endfunction()

# Return the git repository name of the calling CMakeLists.txt.
#
# USAGE:
#     get_git_repo_name()
#
# OUTPUT VARIABLES:
#     GIT_REPO_NAME
#         git repository name of the calling CMakeLists.txt
#
function(get_git_repo_name)
  if(NOT GIT_FOUND OR NOT BASENAME_PROG OR NOT XARGS_PROG)
    return()
  endif()
  execute_process(
    COMMAND ${GIT_EXECUTABLE} config --get remote.origin.url
    COMMAND xargs ${BASENAME_PROG} -s .git
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    OUTPUT_VARIABLE GIT_REPO_NAME
    RESULT_VARIABLE GIT_GET_NAME_RESULT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT GIT_GET_NAME_RESULT EQUAL "0")
    unset(GIT_REPO_NAME)
    message(WARNING "${CMAKE_CURRENT_LIST_FILE} is probably not inside git repository.")
  endif()
  set(GIT_REPO_NAME ${GIT_REPO_NAME} PARENT_SCOPE)
endfunction()

# Use clang-format to format c/c++/Objective-C code.
#
# USAGE:
#     add_format_target(_target
#         FILES              <group of file paths>
#     )
#
# ARGUMENTS:
#     _target
#         name of the target
#     FILES
#         group of paths to header and source files
#
# EXAMPLE:
#     add_format_target(${PROJECT_NAME}
#         FILES              ${SOURCES} ${HEADERS}
#     )
#
function(add_format_target _target)
  if(NOT CLANG_FORMAT_PROG)
    return()
  endif()
  cmake_parse_arguments(ARG "" "" "FILES" ${ARGN})
  locate_dominating_dir(
    NAME .git
    SEARCH_DIR ${CMAKE_CURRENT_LIST_DIR}
  )
  if (NOT FOUND_DOMINATING_DIR)
    set(WORKING_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
  else()
    set(WORKING_DIR ${FOUND_DOMINATING_DIR})
  endif()

  add_custom_target(format-${_target}
    COMMAND ${CLANG_FORMAT_PROG} -i ${ARG_FILES}
    WORKING_DIRECTORY ${WORKING_DIR}
    COMMENT "Formatting ${_target} source code with clang-format"
    VERBATIM
  )
  add_dependencies(format format-${_target})

  add_custom_target(check-format-${_target}
    COMMAND ${CLANG_FORMAT_PROG} -output-replacements-xml ${ARG_FILES} | grep "<replacement " > /dev/null && exit 1 || exit 0
    WORKING_DIRECTORY ${WORKING_DIR}
    COMMENT "Checking ${_target} code formatting with clang-format"
    VERBATIM
  )
  add_dependencies(check-format check-format-${_target})

  get_git_repo_name()
  if(GIT_REPO_NAME)
    if(NOT TARGET format-${GIT_REPO_NAME})
      add_custom_target(format-${GIT_REPO_NAME})
      add_custom_target(check-format-${GIT_REPO_NAME})
    endif()
    add_dependencies(format-${GIT_REPO_NAME} format-${_target})
    add_dependencies(check-format-${GIT_REPO_NAME} check-format-${_target})
  endif()

endfunction()

# Use clang-tidy to tidy c/c++ code.
#
# USAGE:
#     add_tidy_target(_target
#         FILES              <group of file paths>
#         DEPENDS            <group of symbols>
#     )
#
# ARGUMENTS:
#     _target
#         name of the target
#     FILES
#         group of paths to c/c++ source files
#     DEPENDS
#         group of dependency targets
#
# EXAMPLE:
#     add_tidy_target(${PROJECT_NAME}
#         FILES              ${SOURCES}
#         DEPENDS            dep_target_1 dep_target_2
#     )
#
function(add_tidy_target _target)
  if(NOT CLANG_TIDY_PROG)
    return()
  endif()

  cmake_parse_arguments(ARG "" "" "FILES;DEPENDS" ${ARGN})
  locate_dominating_dir(
    NAME .git
    SEARCH_DIR ${CMAKE_CURRENT_LIST_DIR})
  if (NOT FOUND_DOMINATING_DIR)
    set(WORKING_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
  else()
    set(WORKING_DIR ${FOUND_DOMINATING_DIR})
  endif()

  add_custom_target(tidy-${_target}
    COMMAND ${PARALLEL_PROG} ${CLANG_TIDY_PROG} -fix -p=${CMAKE_BINARY_DIR} ${PARALLEL_DELIMITER} ${ARG_FILES}
    WORKING_DIRECTORY ${WORKING_DIR}
    DEPENDS ${ARG_DEPENDS}
    COMMENT "Running clang-tidy for ${_target}"
    VERBATIM)
  add_dependencies(tidy tidy-${_target})

  add_custom_target(check-tidy-${_target}
    COMMAND ${PARALLEL_PROG} ${CLANG_TIDY_PROG} -p=${CMAKE_BINARY_DIR} ${PARALLEL_DELIMITER} ${ARG_FILES} | grep . && exit 1 || exit 0
    WORKING_DIRECTORY ${WORKING_DIR}
    DEPENDS ${ARG_DEPENDS}
    COMMENT "Running clang-tidy for ${_target}"
    VERBATIM)
  add_dependencies(check-tidy check-tidy-${_target})

  get_git_repo_name()
  if(GIT_REPO_NAME)
    if(NOT TARGET tidy-${GIT_REPO_NAME})
      add_custom_target(tidy-${GIT_REPO_NAME})
      add_custom_target(check-tidy-${GIT_REPO_NAME})
    endif()
    add_dependencies(tidy-${GIT_REPO_NAME} tidy-${_target})
    add_dependencies(check-tidy-${GIT_REPO_NAME} check-tidy-${_target})
  endif()
endfunction()

# Use autopep8 to format python code with pep8.
#
# USAGE:
#     add_pep8_target(_target
#         FILES              <group of file paths>
#     )
#
# ARGUMENTS:
#     _target
#         name of the target
#     FILES
#         group of paths to python files
#
# EXAMPLE:
#     add_pep8_target(${PROJECT_NAME}
#         FILES              ${PY_SOURCES}
#     )
#
function(add_pep8_target _target)
  if(NOT AUTOPEP8_PROG)
    return()
  endif()
  cmake_parse_arguments(ARG "" "" "FILES" ${ARGN})

  add_custom_target(pep8-${_target}
    COMMAND ${AUTOPEP8_PROG} -i ${ARG_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
    COMMENT "Formatting ${_target} source code with autopep8"
    VERBATIM)
  add_dependencies(pep8 pep8-${_target})
endfunction()
