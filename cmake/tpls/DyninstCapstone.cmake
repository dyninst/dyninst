#===========================================================
#
# Configure Capstone
#
#   ----------------------------------------
#
# Capstone_ROOT_DIR - Location of Capstone installation
#
# The individual find-modules use the <Package>_ROOT convention
# as the first location to search for the package. If the user
# specifies Capstone_ROOT_DIR, we override the <Package>_ROOT
# values and require that each package ignores system directories.
# In effect, this forces the package search to find only
# candidates in <Package>_ROOT or CMAKE_PREFIX_PATH.
#
#===========================================================

include_guard(GLOBAL)

set(_min_version 6.0)

if(capstone_ROOT_DIR)
  set(Capstone_ROOT_DIR ${capstone_ROOT_DIR})
  mark_as_advanced(Capstone_ROOT_DIR)
endif()

if(Capstone_ROOT_DIR)
  set(capstone_ROOT ${Capstone_ROOT_DIR})
  mark_as_advanced(CAPSTONE_ROOT)
  set(capstone_DIR ${Capstone_ROOT_DIR})
  mark_as_advanced(capstone_DIR)
  set(_find_path_args NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
endif()

find_package(capstone ${_min_version} REQUIRED ${_find_path_args})

if(NOT TARGET Dyninst::Capstone)
  add_library(Dyninst::Capstone INTERFACE IMPORTED)
  target_link_libraries(Dyninst::Capstone INTERFACE capstone::capstone)
  target_include_directories(
    Dyninst::Capstone SYSTEM
    INTERFACE $<TARGET_PROPERTY:capstone::capstone,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

message(STATUS "Found Capstone ${Capstone_VERSION}")
get_target_property(_tmp capstone::capstone INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "Capstone include directories: ${_tmp}")

unset(_find_path_args)
