# Formatting this project's own CMake code with gersemi:
#
#   cmake-check          report which files are not formatted
#   cmake-check-raw      the same, as gersemi itself prints it
#   cmake-reformat-diff  show what reformatting them would change
#   cmake-reformat       reformat them in place
#
# None of the four is part of 'all'; they are asked for by name.  gersemi is
# a developer tool rather than a build dependency, so a missing one is not a
# configure error -- it is an error only for whoever asks for one of these
# targets, which is where DyninstRunGersemi.cmake reports it.
#
# DyninstRunGersemi.cmake searches PATH again when this find_program fails.
#
# DYNINST_GERSEMI_VERSIONS_FILE, set before including this, names the file
# that pins the gersemi version, in a line 'gersemi:<version>'.  Without it,
# any gersemi is accepted.

find_program(
  DYNINST_GERSEMI_EXECUTABLE
  NAMES gersemi
  DOC "gersemi, the CMake formatter behind the cmake-check and cmake-reformat targets"
)
mark_as_advanced(DYNINST_GERSEMI_EXECUTABLE)

if(DYNINST_GERSEMI_EXECUTABLE)
  message(STATUS "Found gersemi: ${DYNINST_GERSEMI_EXECUTABLE}")
endif()

# This file's directory and the versions file, for the function below.  In
# the function CMAKE_CURRENT_LIST_DIR is the directory of the file that calls
# it, and a variable set here is not seen from a parent or sibling directory,
# so both are kept in global properties.
set_property(GLOBAL PROPERTY DYNINST_GERSEMI_DIR "${CMAKE_CURRENT_LIST_DIR}")
set_property(
  GLOBAL
  PROPERTY DYNINST_GERSEMI_VERSIONS_FILE "${DYNINST_GERSEMI_VERSIONS_FILE}"
)

function(dyninst_add_gersemi_target _name _mode _comment)
  get_property(_dir GLOBAL PROPERTY DYNINST_GERSEMI_DIR)
  get_property(_versions_file GLOBAL PROPERTY DYNINST_GERSEMI_VERSIONS_FILE)
  add_custom_target(
    ${_name}
    COMMAND
      ${CMAKE_COMMAND} "-DGERSEMI=${DYNINST_GERSEMI_EXECUTABLE}" "-DMODE=${_mode}"
      "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}" "-DVERSIONS_FILE=${_versions_file}" -P
      "${_dir}/DyninstRunGersemi.cmake"
    COMMENT "${_comment}"
    VERBATIM
    USES_TERMINAL
  )
endfunction()

dyninst_add_gersemi_target(cmake-check check "Checking the formatting of the CMake code")
dyninst_add_gersemi_target(
  cmake-check-raw
  check-raw
  "Checking the formatting of the CMake code (gersemi's own output)"
)
dyninst_add_gersemi_target(
  cmake-reformat-diff
  diff
  "Diffing the formatting of the CMake code"
)
dyninst_add_gersemi_target(cmake-reformat in-place "Reformatting the CMake code")
