#===========================================================
#
# Configure Boost
#
#   ----------------------------------------
#
# Boost_ROOT_DIR - Directory hint for Boost installation
#
#===========================================================

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

set(Boost_DEFINES
    ${_boost_defines}
    CACHE STRING "Boost compiler defines")
add_definitions(${Boost_DEFINES})

# The required Boost library components NB: These are just the ones that require
# compilation/linking This should _not_ be a cache variable
set(_boost_components atomic chrono date_time filesystem thread timer)

find_package(Boost ${Boost_MIN_VERSION} REQUIRED HINTS ${Boost_ROOT_DIR} ${PATH_BOOST} ${BOOST_ROOT} COMPONENTS ${_boost_components})

link_directories(${Boost_LIBRARY_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

message(STATUS "Boost includes: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost library dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
