#===============================================================================================
#
# Configure Boost
#
#   ----------------------------------------
#
# Accepts the following CMake variables
#
# Boost_ROOT_DIR            - Hint directory that contains the Boost installation
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
#
#===============================================================================================

include_guard(GLOBAL)

# Need at least 1.70 because of deprecated headers
set(_boost_min_version 1.70.0)


# Use multithreaded libraries
set(Boost_USE_MULTITHREADED ON)

# Don't use libraries linked statically to the C++ runtime
set(Boost_USE_STATIC_RUNTIME OFF)

# Set the default location to look for Boost
set(Boost_ROOT_DIR
    "/usr"
    CACHE PATH "Boost root directory for Dyninst")
mark_as_advanced(Boost_ROOT_DIR)

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

# Disable Boost's own CMake as it's known to be buggy NB: This should not be a cache
# variable
set(Boost_NO_BOOST_CMAKE ON)

# The required Boost library components NB: These are just the ones that require
# compilation/linking This should _not_ be a cache variable
set(_boost_components atomic chrono date_time filesystem thread timer)

find_package(Boost ${Boost_MIN_VERSION} REQUIRED HINTS ${Boost_ROOT_DIR} ${PATH_BOOST} ${BOOST_ROOT} COMPONENTS ${_boost_components})

# Export the complete set of libraries
set(Boost_LIBRARIES
    ${Boost_LIBRARIES}
    CACHE FILEPATH "Boost library files" FORCE)

link_directories(${Boost_LIBRARY_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

message(STATUS "Boost includes: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost library dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
