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

set(_min_version 6.0.0-Alpha5)

if(Capstone_ROOT)
  set(capstone_ROOT ${Capstone_ROOT})
endif()

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

find_package(capstone REQUIRED ${_find_path_args})

# Only major versions of Capstone are compatible, but we can use anything newer than
# the required minimum version.
if(capstone_VERSION VERSION_LESS ${_min_version})
  message(
    FATAL_ERROR
      "Capstone: found version ${capstone_VERSION}, but need at least ${_min_version}")
endif()

# We could use capstone::capstone_static, but there's currently no way to
# detect if it was build with POSITION_INDEPENDENT_CODE=ON which is required
# for linking into libinstructionAPI
if(TARGET capstone::capstone_shared)
  set(_cap_target capstone::capstone_shared)
else()
  message(FATAL_ERROR "A version of Capstone with shared libraries is required.")
endif()

if(NOT TARGET Dyninst::Capstone)
  add_library(Dyninst::Capstone INTERFACE IMPORTED)
  target_link_libraries(Dyninst::Capstone INTERFACE ${_cap_target})
  target_include_directories(
    Dyninst::Capstone SYSTEM
    INTERFACE $<TARGET_PROPERTY:${_cap_target},INTERFACE_INCLUDE_DIRECTORIES>)
endif()

message(STATUS "Found Capstone ${capstone_VERSION}")
get_target_property(_tmp ${_cap_target} INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "Capstone include directories: ${_tmp}")

unset(_find_path_args)
