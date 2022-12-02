#========================================================================================================
# Boost.cmake
#
# Configure Boost for Dyninst
#
#   ----------------------------------------
#
# Accepts the following CMake variables
#
# Boost_ROOT_DIR            - Hint directory that contains the Boost installation
# PATH_BOOST                - Alias for Boost_ROOT_DIR
# Boost_MIN_VERSION         - Minimum acceptable version of Boost
# Boost_USE_MULTITHREADED   - Use the multithreaded version of Boost
# Boost_USE_STATIC_RUNTIME  - Use libraries linked statically to the C++ runtime
#
# Advanced options:
#
# Boost_DEBUG               - Enable debug output from FindBoost
# Boost_NO_SYSTEM_PATHS     - Disable searching in locations not specified by hint variables
#
# Exports the following CMake cache variables
#
# Boost_ROOT_DIR            - Computed base directory the of Boost installation
# Boost_INCLUDE_DIRS        - Boost include directories
# Boost_INCLUDE_DIR         - Alias for Boost_INCLUDE_DIRS
# Boost_LIBRARY_DIRS        - Link directories for Boost libraries
# Boost_DEFINES             - Boost compiler definitions
# Boost_LIBRARIES           - Boost library files
# Boost_<C>_LIBRARY_RELEASE - Release libraries to link for component <C> (<C> is upper-case)
# Boost_<C>_LIBRARY_DEBUG   - Debug libraries to link for component <C>
# Boost_THREAD_LIBRARY      - The filename of the Boost thread library
# Boost_USE_MULTITHREADED   - Use the multithreaded version of Boost
# Boost_USE_STATIC_RUNTIME  - Use libraries linked statically to the C++ runtime
#
# NOTE:
# The exported Boost_ROOT_DIR can be different from the value provided by the user in the case that
# it is determined to build Boost from source. In such a case, Boost_ROOT_DIR will contain the
# directory of the from-source installation.
#========================================================================================================

if(Boost_FOUND)
    return()
endif()

# Need at least Boost-1.67 because of deprecated headers
set(_boost_min_version 1.70.0)

# Provide a default, if the user didn't specify
set(Boost_MIN_VERSION
    ${_boost_min_version}
    CACHE STRING "Minimum Boost version")

# Enforce minimum version
if(${Boost_MIN_VERSION} VERSION_LESS ${_boost_min_version})
    message(
        FATAL_ERROR
            "Requested Boost-${Boost_MIN_VERSION} is less than minimum supported version (${_boost_min_version})"
        )
endif()

# -------------- RUNTIME CONFIGURATION ----------------------------------------

# Use the multithreaded version of Boost NB: This _must_ be a cache variable as it
# controls the tagged layout of Boost library names
set(Boost_USE_MULTITHREADED
    ON
    CACHE BOOL "Enable multithreaded Boost libraries")

# Don't use libraries linked statically to the C++ runtime NB: This _must_ be a cache
# variable as it controls the tagged layout of Boost library names
set(Boost_USE_STATIC_RUNTIME
    OFF
    CACHE BOOL "Enable usage of libraries statically linked to C++ runtime")

# If using multithreaded Boost, make sure Threads has been intialized
if(Boost_USE_MULTITHREADED AND NOT DEFINED CMAKE_THREAD_LIBS_INIT)
    find_package(Threads)
endif()

# Enable debug output from FindBoost
set(Boost_DEBUG
    OFF
    CACHE BOOL "Enable debug output from FindBoost")

# -------------- PATHS --------------------------------------------------------

# By default, search system paths
set(Boost_NO_SYSTEM_PATHS
    OFF
    CACHE BOOL "Disable searching in locations not specified by hint variables")

# A sanity check This must be done _before_ the cache variables are set
if(PATH_BOOST AND Boost_ROOT_DIR)
    message(
        FATAL_ERROR
            "PATH_BOOST AND Boost_ROOT_DIR both specified. Please provide only one")
endif()

# Provide a default root directory
if(NOT PATH_BOOST AND NOT Boost_ROOT_DIR)
    set(PATH_BOOST "/usr")
endif()

# Set the default location to look for Boost
set(Boost_ROOT_DIR
    ${PATH_BOOST}
    CACHE PATH "Base directory the of Boost installation")

# In FindBoost, Boost_ROOT_DIR is spelled BOOST_ROOT
set(BOOST_ROOT ${Boost_ROOT_DIR})

# -------------- COMPILER DEFINES ---------------------------------------------

set(_boost_defines)

# Disable auto-linking
list(APPEND _boost_defines -DBOOST_ALL_NO_LIB=1)

# Disable generating serialization code in boost::multi_index
list(APPEND _boost_defines -DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION)

# There are broken versions of MSVC that won't handle variadic templates correctly
# (despite the C++11 test case passing).
if(MSVC)
    list(APPEND _boost_defines -DBOOST_NO_CXX11_VARIADIC_TEMPLATES)
endif()

set(Boost_DEFINES
    ${_boost_defines}
    CACHE STRING "Boost compiler defines")
add_definitions(${Boost_DEFINES})

# -------------- INTERNALS ----------------------------------------------------

# Disable Boost's own CMake as it's known to be buggy NB: This should not be a cache
# variable
set(Boost_NO_BOOST_CMAKE ON)

# The required Boost library components NB: These are just the ones that require
# compilation/linking This should _not_ be a cache variable
set(_boost_components atomic chrono date_time filesystem thread timer)

find_package(Boost ${Boost_MIN_VERSION} REQUIRED COMPONENTS ${_boost_components})

# -------------- EXPORT VARIABLES ---------------------------------------------

# Export Boost_THREAD_LIBRARY
list(FIND _boost_components "thread" _building_threads)
if(Boost_USE_MULTITHREADED AND ${_building_threads})
    # On Windows, always use the debug version On Linux, we don't use tagged builds, so
    # the debug/release filenames are the same
    set(Boost_THREAD_LIBRARY
        ${Boost_THREAD_LIBRARY_DEBUG}
        CACHE FILEPATH "Boost thread library")
endif()

# Add the system thread library
if(Boost_USE_MULTITHREADED)
    list(APPEND Boost_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
endif()

# Export the complete set of libraries
set(Boost_LIBRARIES
    ${Boost_LIBRARIES}
    CACHE FILEPATH "Boost library files" FORCE)

link_directories(${Boost_LIBRARY_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

message(STATUS "Boost includes: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost library dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost thread library: ${Boost_THREAD_LIBRARY}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
