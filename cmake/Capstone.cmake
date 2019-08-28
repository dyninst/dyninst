#======================================================================================
# Capstone.cmake
#
# Configure Capstone for Dyninst
#
#   ----------------------------------------
#
# Accepts the following CMake variables
#
# Capstone_ROOT_DIR            - Base directory of the Capstone installation
# Capstone_INCLUDEDIR          - Hint directory that contains the Capstone headers files
# Capstone_LIBRARYDIR          - Hint directory that contains the Capstone library files
# Capstone_MIN_VERSION         - Minimum acceptable version of Capstone
# Capstone_USE_STATIC_RUNTIME  - Link to the static runtime library
#
# Exports the following CMake cache variables
#
# Capstone_ROOT_DIR       - Base directory of the Capstone installation
# Capstone_INCLUDE_DIRS   - Capstone include directories
# Capstone_LIBRARY_DIRS   - Link directories for Capstone libraries
# Capstone_LIBRARIES      - Capstone library files
#
#======================================================================================

if(Capstone_FOUND)
  return()
endif()

# -------------- VERSION ------------------------------------------------------

# Minimum acceptable version
set(_min_version 5.0)

set(Capstone_MIN_VERSION ${_min_version}
    CACHE STRING "Minimum acceptable capstone version")

if(${Capstone_MIN_VERSION} VERSION_LESS ${_min_version})
  message(
    FATAL_ERROR
      "Requested version ${Capstone_MIN_VERSION} is less than minimum supported version (${_min_version})"
    )
endif()

# -------------- PATHS --------------------------------------------------------

# Base directory the of capstone installation
set(Capstone_ROOT_DIR "/usr"
    CACHE PATH "Base directory the of capstone installation")

# Hint directory that contains the capstone headers files
set(Capstone_INCLUDEDIR "${Capstone_ROOT_DIR}/include"
    CACHE PATH "Hint directory that contains the capstone headers files")

# Hint directory that contains the capstone library files
set(Capstone_LIBRARYDIR "${Capstone_ROOT_DIR}/lib"
    CACHE PATH "Hint directory that contains the capstone library files")

# -----------------------------------------------------------------------------

find_package(Capstone ${Capstone_MIN_VERSION} REQUIRED)

if(NOT Capstone_FOUND)
  message(
    FATAL_ERROR
      "Capstone was not found. See https://github.com/dyninst/dyninst/wiki/third-party-deps#capstone for instructions on building it from source."
    )
endif()

link_directories(${Capstone_LIBRARY_DIRS})
include_directories(${Capstone_INCLUDE_DIRS})

message(STATUS "Capstone includes: ${Capstone_INCLUDE_DIRS}")
message(STATUS "Capstone library dirs: ${Capstone_LIBRARY_DIRS}")
message(STATUS "Capstone libraries: ${Capstone_LIBRARIES}")

if(USE_COTIRE)
  cotire(Capstone)
endif()
