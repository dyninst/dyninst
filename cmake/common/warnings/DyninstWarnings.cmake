# The compiler warning flags: the flags of the warning levels up to the one chosen, see
# DyninstWarningLevels.cmake, and any the project adds.  The ones the C and C++
# compilers accept become SUPPORTED_C_WARNING_FLAGS and SUPPORTED_CXX_WARNING_FLAGS.
#
# A project can set, before including this module:
#
#   REQUESTED_WARNING_LEVEL  its default level, 0 to the highest or max (the default)
#   REQUESTED_WARNING_FLAGS  flags of its own; they follow the levels' flags, so that
#                            one such as Wno-unknown-pragmas wins

option(
  DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS
  "Disable all warning suppressions and frame size overrides."
  OFF
)

set(
  DYNINST_EXTRA_WARNINGS
  ""
  CACHE STRING
  "Additional warning options to enable if available.  ;-separated without leading '-' (Wopt1[;Wopt2]...)."
)

option(DYNINST_WARNINGS_AS_ERRORS "Treat compilation warnings as errors" OFF)

set(
  DYNINST_WARNING_LEVEL
  ""
  CACHE STRING
  "Warning level: 0 for none up to the highest, or max.  Empty for the project's default."
)

set(_project_warning_flags ${REQUESTED_WARNING_FLAGS})

if(DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS)
  message(
    STATUS
    "DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS set: disabling all dyninst warning suppressions and frame size overrides"
  )
  # The macros of Dyninst's compiler_diagnostics.h then expand to nothing, so
  # the warnings they suppress appear, in every target from here on
  add_compile_definitions(DYNINST_DIAGNOSTIC_NO_SUPPRESSIONS)
endif()

# The frame size limit for every build type.  A project can raise it for particular files
# with larger frames.
set(defaultMaxFrameSize 20480)

# The warning levels, in their own file.  The highest is the last one defined there.
include("${CMAKE_CURRENT_LIST_DIR}/DyninstWarningLevels.cmake")
set(_max_level 1)
while(DEFINED _dyninst_warning_level_${_max_level})
  math(EXPR _max_level "${_max_level} + 1")
endwhile()
math(EXPR _max_level "${_max_level} - 1")

# The level: DYNINST_WARNING_LEVEL when the user sets it, else the repository's
# REQUESTED_WARNING_LEVEL, else the highest
if(NOT "${DYNINST_WARNING_LEVEL}" STREQUAL "")
  set(_level "${DYNINST_WARNING_LEVEL}")
elseif(DEFINED REQUESTED_WARNING_LEVEL)
  set(_level "${REQUESTED_WARNING_LEVEL}")
else()
  set(_level max)
endif()
if("${_level}" STREQUAL "max")
  set(_level ${_max_level})
elseif(NOT "${_level}" MATCHES "^[0-9]+$" OR _level GREATER _max_level)
  message(FATAL_ERROR "Warning level '${_level}' is not 0 to ${_max_level}, or max")
endif()
message(STATUS "Warning level ${_level} of ${_max_level}")

set(REQUESTED_WARNING_FLAGS "")
if(_level GREATER 0)
  foreach(_l RANGE 1 ${_level})
    list(APPEND REQUESTED_WARNING_FLAGS ${_dyninst_warning_level_${_l}})
  endforeach()
endif()

list(APPEND REQUESTED_WARNING_FLAGS ${_project_warning_flags})

if(DYNINST_EXTRA_WARNINGS)
  list(APPEND REQUESTED_WARNING_FLAGS ${DYNINST_EXTRA_WARNINGS})
  message(
    STATUS
    "DYNINST_EXTRA_WARNINGS set, adding extra warnings:  ${DYNINST_EXTRA_WARNINGS}"
  )
endif()

if(DYNINST_WARNINGS_AS_ERRORS)
  list(APPEND REQUESTED_WARNING_FLAGS "Werror")
  message(STATUS "DYNINST_WARNINGS_AS_ERRORS set: treating warnings as errors")
endif()

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
  include(CheckCCompilerFlag)
  foreach(f IN LISTS REQUESTED_WARNING_FLAGS)
    string(REGEX REPLACE "[^a-zA-Z0-9]" "_" v "HAS_C_FLAG_${f}")
    set(CMAKE_REQUIRED_FLAGS "-${f}")
    check_c_source_compiles(
      "int main(){return 0;}"
      "${v}"
      FAIL_REGEX "warning: *command[- ]line option|-Wunknown-warning-option"
    )
    # Previous two lines are equivalent to below, but also catches a 0 exit status
    # with a warning message output: check_c_compiler_flag("-${f}" "${v}")
    if(${v})
      list(APPEND SUPPORTED_C_WARNING_FLAGS "-${f}")
      if(f MATCHES "^(.*)=[0-9]+$")
        # set generic variable if warning is parameterized with a number
        string(REGEX REPLACE "[^a-zA-Z0-9]" "_" v "HAS_C_FLAG_${CMAKE_MATCH_1}")
        set("${v}" 1)
      endif()
    endif()
  endforeach()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
  include(CheckCXXCompilerFlag)
  foreach(f IN LISTS REQUESTED_WARNING_FLAGS)
    string(REGEX REPLACE "[^a-zA-Z0-9]" "_" v "HAS_CPP_FLAG_${f}")
    set(CMAKE_REQUIRED_FLAGS "-${f}")
    check_cxx_source_compiles(
      "int main(){return 0;}"
      "${v}"
      FAIL_REGEX "warning: *command[- ]line option|-Wunknown-warning-option"
    )
    if(${v})
      list(APPEND SUPPORTED_CXX_WARNING_FLAGS "-${f}")
      if(f MATCHES "^(.*)=[0-9]+$")
        string(REGEX REPLACE "[^a-zA-Z0-9]" "_" v "HAS_CPP_FLAG_${CMAKE_MATCH_1}")
        set("${v}" 1)
      endif()
    endif()
  endforeach()
endif()

unset(CMAKE_REQUIRED_FLAGS)

if(MSVC)
  message(STATUS "TODO: Set up custom warning flags for MSVC")
endif()

message(STATUS "Using C warning flags: ${SUPPORTED_C_WARNING_FLAGS}")
message(STATUS "Using CXX warning flags: ${SUPPORTED_CXX_WARNING_FLAGS}")
