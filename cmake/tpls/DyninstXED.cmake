#===========================================================
#
# Configure Intel XED
#
#   ----------------------------------------
#
# XED_ROOT_DIR - Location of a prebuilt Intel XED installation
#
# Intel XED builds with its own 'mbuild' (Python) build system,
# not CMake, and ships no CMake package config. The user is
# expected to build/install XED separately (mbuild produces the
# headers and libxed) and point Dyninst at it with XED_ROOT_DIR.
# Here we just locate the installed headers and library and wrap
# them in the Dyninst::XED imported target.
#
# The header may be installed either as <xed/xed-interface.h> or
# as a flat <xed-interface.h>; the source selects the correct
# form with __has_include.
#
#===========================================================

include_guard(GLOBAL)

if(XED_ROOT)
  set(_xed_hints ${XED_ROOT})
endif()

if(XED_ROOT_DIR)
  set(_xed_hints ${XED_ROOT_DIR})
  mark_as_advanced(XED_ROOT_DIR)
endif()

find_path(
  XED_INCLUDE_DIR
  NAMES xed/xed-interface.h xed-interface.h
  HINTS ${_xed_hints} ENV XED_ROOT
  PATH_SUFFIXES include)

find_library(
  XED_LIBRARY
  NAMES xed
  HINTS ${_xed_hints} ENV XED_ROOT
  PATH_SUFFIXES lib lib64)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  XED
  REQUIRED_VARS XED_LIBRARY XED_INCLUDE_DIR
  FAIL_MESSAGE
    "Could not find Intel XED. Build it with mbuild and set XED_ROOT_DIR to the install prefix."
  )

if(NOT TARGET Dyninst::XED)
  add_library(Dyninst::XED INTERFACE IMPORTED)
  target_include_directories(Dyninst::XED SYSTEM INTERFACE ${XED_INCLUDE_DIR})
  target_link_libraries(Dyninst::XED INTERFACE ${XED_LIBRARY})
endif()

message(STATUS "Found Intel XED: ${XED_LIBRARY}")

mark_as_advanced(XED_INCLUDE_DIR XED_LIBRARY)
unset(_xed_hints)
