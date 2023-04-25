#===========================================================
#
# Configure Boost
#
#   ----------------------------------------
#
# Boost_ROOT_DIR - Location of Boost installation
#
# The individual find-modules use the <Package>_ROOT convention
# as the first location to search for the package. If the user
# specifies Boost_ROOT_DIR, we override the <Package>_ROOT
# values and require that each package ignores system directories.
# In effect, this forces the package search to find only
# candidates in <Package>_ROOT or CMAKE_PREFIX_PATH.
#
#===========================================================

include_guard(GLOBAL)

# Need at least 1.71 for a usable BoostConfig.cmake
set(_min_version 1.71.0)

# Use multithreaded libraries
set(Boost_USE_MULTITHREADED ON)

# Don't use libraries linked statically to the C++ runtime
set(Boost_USE_STATIC_RUNTIME OFF)

if(Boost_ROOT_DIR)
  set(Boost_NO_SYSTEM_PATHS ON)
  set(Boost_ROOT ${Boost_ROOT_DIR})
endif()

# Starting in CMake 3.20, suppress "unknown version" warnings
set(Boost_NO_WARN_NEW_VERSIONS ON)

# Library components that need to be linked against
set(_boost_components atomic chrono date_time filesystem thread timer)
find_package(
  Boost
  ${_min_version}
  QUIET
  REQUIRED
  HINTS
  ${PATH_BOOST}
  ${BOOST_ROOT}
  COMPONENTS ${_boost_components})

# Don't let Boost variables seep through
mark_as_advanced(Boost_DIR)

if(NOT TARGET Dyninst::Boost)
  # Make an interface dummy target to force includes to be treated as SYSTEM
  add_library(Dyninst::Boost INTERFACE IMPORTED)
  target_link_libraries(Dyninst::Boost INTERFACE ${Boost_LIBRARIES})
  target_include_directories(Dyninst::Boost SYSTEM INTERFACE ${Boost_INCLUDE_DIRS})
  target_compile_definitions(Dyninst::Boost
                             INTERFACE BOOST_MULTI_INDEX_DISABLE_SERIALIZATION)

  # Just the headers (effectively a simplified Boost::headers target)
  add_library(Dyninst::Boost_headers INTERFACE IMPORTED)
  target_include_directories(Dyninst::Boost_headers SYSTEM
                             INTERFACE ${Boost_INCLUDE_DIRS})
  target_compile_definitions(Dyninst::Boost_headers
                             INTERFACE BOOST_MULTI_INDEX_DISABLE_SERIALIZATION)
endif()
message(STATUS "Found Boost ${Boost_VERSION}")
message(STATUS "Boost include directories: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
