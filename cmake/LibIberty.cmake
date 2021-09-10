#======================================================================================
# LibIberty.cmake
#
# Configure LibIberty for Dyninst
#
#   ----------------------------------------
#
# Directly exports the following CMake variables
#
# LibIberty_ROOT_DIR       - Computed base directory the of LibIberty installation
# LibIberty_LIBRARY_DIRS   - Link directories for LibIberty libraries
# LibIberty_LIBRARIES      - LibIberty library files
# LibIberty_INCLUDE        - LibIberty include files
#
# NOTE:
# The exported LibIberty_ROOT_DIR can be different from the value provided by the user
# in the case that it is determined to build LibIberty from source. In such a case,
# LibIberty_ROOT_DIR will contain the directory of the from-source installation.
#
# See Modules/FindLibIberty.cmake for details
#
#======================================================================================

if(LibIberty_FOUND)
  return()
endif()

if(NOT UNIX)
  return()
endif()

# -------------- PATHS --------------------------------------------------------

# Base directory the of LibIberty installation
set(LibIberty_ROOT_DIR "/usr"
    CACHE PATH "Base directory the of LibIberty installation")

# Hint directory that contains the LibIberty library files
set(LibIberty_LIBRARYDIR "${LibIberty_ROOT_DIR}/lib"
    CACHE PATH "Hint directory that contains the LibIberty library files")

# -------------- PACKAGES -----------------------------------------------------

find_package(LibIberty REQUIRED)


# -------------- EXPORT VARIABLES ---------------------------------------------

add_library(LibIberty STATIC IMPORTED GLOBAL)
set_target_properties(LibIberty PROPERTIES IMPORTED_LOCATION ${LibIberty_LIBRARIES})
set_target_properties(LibIberty PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${LibIberty_INCLUDE_DIRS})

set(LibIberty_ROOT_DIR ${LibIberty_ROOT_DIR}
    CACHE PATH "Base directory the of LibIberty installation"
    FORCE)
set(LibIberty_INCLUDE_DIRS ${LibIberty_INCLUDE_DIRS}
    CACHE PATH "LibIberty include directories"
    FORCE)
set(LibIberty_LIBRARY_DIRS ${LibIberty_LIBRARY_DIRS}
    CACHE PATH "LibIberty library directory"
    FORCE)
set(LibIberty_LIBRARIES ${LibIberty_LIBRARIES}
    CACHE FILEPATH "LibIberty library files"
    FORCE)

# For backward compatibility only
set(IBERTY_LIBRARIES ${LibIberty_LIBRARIES})

message(STATUS "LibIberty include dirs: ${LibIberty_INCLUDE_DIRS}")
message(STATUS "LibIberty library dirs: ${LibIberty_LIBRARY_DIRS}")
message(STATUS "LibIberty libraries: ${LibIberty_LIBRARIES}")
