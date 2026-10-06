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
# For the same reason the find_program below is a default and an override
# point, not the last word: the path it caches is baked into the targets at
# generate time, so DyninstRunGersemi.cmake searches PATH again when it was
# not found here.  Installing gersemi into an already-configured build tree
# is the common case, and it should not need a re-configure to take.

find_program(
  DYNINST_GERSEMI_EXECUTABLE
  NAMES gersemi
  DOC "gersemi, the CMake formatter behind the cmake-check and cmake-reformat targets"
)
mark_as_advanced(DYNINST_GERSEMI_EXECUTABLE)

if(DYNINST_GERSEMI_EXECUTABLE)
  message(STATUS "Found gersemi: ${DYNINST_GERSEMI_EXECUTABLE}")
endif()

function(dyninst_add_gersemi_target _name _mode _comment)
  add_custom_target(
    ${_name}
    COMMAND
      ${CMAKE_COMMAND} "-DGERSEMI=${DYNINST_GERSEMI_EXECUTABLE}" "-DMODE=${_mode}"
      "-DSOURCE_DIR=${PROJECT_SOURCE_DIR}" -P
      "${PROJECT_SOURCE_DIR}/cmake/DyninstRunGersemi.cmake"
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
