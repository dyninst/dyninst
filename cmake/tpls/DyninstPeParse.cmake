#======================================================================================
# PeParse.cmake
#
# Configure PeParse for Dyninst
#
#   ----------------------------------------
#
# Directly exports the following CMake variables
#
# PeParse_ROOT_DIR       - Computed base directory the of PeParse installation
#
# NOTE:
# The exported PeParse_ROOT_DIR can be different from the value provided by the user
# in the case that it is determined to build PeParse from source. In such a case,
# PeParse_ROOT_DIR will contain the directory of the from-source installation.
#
#======================================================================================

include_guard(GLOBAL)

cmake_policy(SET CMP0074 NEW) # Use <Package>_ROOT

if(NOT UNIX)
  return()
endif()

if(PeParse_ROOT_DIR)
  set(pe-parse_ROOT "${PeParse_ROOT_DIR}")
  mark_as_advanced(pe-parse_ROOT)
  set(_find_path_args NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
endif()

if(DYNINST_ENABLE_FILEFORMAT_PE)
  find_package(pe-parse REQUIRED ${_find_path_args})
else()
  set(pe-parse_FOUND FALSE)
endif()

if(NOT TARGET Dyninst::PeParse)
  add_library(Dyninst::PeParse INTERFACE IMPORTED)
  if(pe-parse_FOUND)
    target_include_directories(
      Dyninst::PeParse SYSTEM
      INTERFACE $<TARGET_PROPERTY:pe-parse::pe-parse,INTERFACE_INCLUDE_DIRECTORIES>)
    target_link_libraries(Dyninst::PeParse INTERFACE pe-parse::pe-parse)
  endif()
endif()
