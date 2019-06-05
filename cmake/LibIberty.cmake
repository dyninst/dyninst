#======================================================================================
# LibIberty.cmake
#
# Configure LibIberty for Dyninst
#
#   ----------------------------------------
#
# Accepts the following CMake variables
#
# USE_GNU_DEMANGLER        - Use the GNU C++ name demanger (if yes, this disables using LibIberty)
#
# Directly exports the following CMake variables
#
# LibIberty_ROOT_DIR       - Computed base directory the of LibIberty installation
# LibIberty_LIBRARY_DIRS   - Link directories for LibIberty libraries
# LibIberty_LIBRARIES      - LibIberty library files
#
# NOTE:
# The exported LibIberty_ROOT_DIR can be different from the value provided by the user
# in the case that it is determined to build LibIberty from source. In such a case,
# LibIberty_ROOT_DIR will contain the directory of the from-source installation.
#
# See Modules/FindLibIberty.cmake for details
#
#======================================================================================

if(NOT UNIX)
  return()
endif()

# Use the GNU C++ name demangler; if yes, this disables using LibIberty
set(USE_GNU_DEMANGLER ON CACHE BOOL "Use the GNU C++ name demangler")

# If we don't want to use/build LibIberty, then leave
if(USE_GNU_DEMANGLER)
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

find_package(LibIberty)

# -------------- SOURCE BUILD -------------------------------------------------
if(LibIberty_FOUND)
  set(_li_root ${LibIberty_ROOT_DIR})
  set(_li_lib_dirs ${LibIberty_LIBRARY_DIRS})
  set(_li_libs ${LibIberty_LIBRARIES})
  add_library(LibIberty STATIC IMPORTED)
else()
  message(STATUS "${LibIberty_ERROR_REASON}")
  message(STATUS "Attempting to build LibIberty as external project")

  include(ExternalProject)
  ExternalProject_Add(
    LibIberty
    PREFIX ${CMAKE_BINARY_DIR}/binutils
    URL http://ftp.gnu.org/gnu/binutils/binutils-2.31.1.tar.gz
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND
      CFLAGS=-fPIC
      <SOURCE_DIR>/configure --prefix=${CMAKE_BINARY_DIR}/binutils
    BUILD_COMMAND make
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib/libiberty
    INSTALL_COMMAND
      install <SOURCE_DIR>/libiberty/libiberty.a <INSTALL_DIR>
  )

  set(_li_root ${CMAKE_INSTALL_PREFIX})
  set(_li_lib_dirs ${_li_root}/lib)
  set(_li_libs ${_li_lib_dirs}/libiberty/libiberty.a)
  
  # For backward compatibility
  set(IBERTY_FOUND TRUE)
  set(IBERTY_BUILD TRUE)
endif()

# Add a dummy target to either the found LibIberty or the one built from source.
# NB: For backwards compatibility only
add_custom_target(libiberty_imp)
add_dependencies(libiberty_imp LibIberty)
  
# -------------- EXPORT VARIABLES ---------------------------------------------

set(LibIberty_ROOT_DIR ${_li_root}
    CACHE PATH "Base directory the of LibIberty installation"
    FORCE)
set(LibIberty_LIBRARY_DIRS ${_li_lib_dirs}
    CACHE PATH "LibIberty library directory"
    FORCE)
set(LibIberty_LIBRARIES ${_li_libs}
    CACHE FILEPATH "LibIberty library files"
    FORCE)

# This is only here to make ccmake work correctly
set(USE_GNU_DEMANGLER ${USE_GNU_DEMANGLER}
    CACHE BOOL "Use the GNU C++ name demangler"
    FORCE)

# For backward compatibility only
set(IBERTY_LIBRARIES ${LibIberty_LIBRARIES})

message(STATUS "LibIberty library dirs: ${LibIberty_LIBRARY_DIRS}")
message(STATUS "LibIberty libraries: ${LibIberty_LIBRARIES}")

if(USE_COTIRE)
  cotire(LibIberty)
endif()
