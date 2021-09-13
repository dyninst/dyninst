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

if(NOT UNIX)
    # add empty LibIberty interface target
    dyninst_add_interface_library(LibIberty "LibIberty interface library (empty)")
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

if(NOT BUILD_LIBIBERTY)
    find_package(LibIberty)
endif()

# -------------- SOURCE BUILD -------------------------------------------------
if(LibIberty_FOUND)
    set(_li_root ${LibIberty_ROOT_DIR})
    set(_li_inc_dirs ${LibIberty_INCLUDE_DIRS})
    set(_li_lib_dirs ${LibIberty_LIBRARY_DIRS})
    set(_li_libs ${LibIberty_LIBRARIES})
elseif(STERILE_BUILD)
    dyninst_message(FATAL_ERROR "LibIberty not found and cannot be downloaded because build is sterile.")
elseif(NOT BUILD_LIBIBERTY)
    dyninst_message(FATAL_ERROR "LibIberty was not found. Either configure cmake to find TBB properly or set BUILD_LIBIBERTY=ON to download and build")
else()
    dyninst_message(STATUS "${LibIberty_ERROR_REASON}")
    dyninst_message(STATUS "Attempting to build LibIberty as external project")

    include(ExternalProject)
    ExternalProject_Add(
        LibIberty-External
        PREFIX ${CMAKE_BINARY_DIR}/binutils
        URL http://ftp.gnu.org/gnu/binutils/binutils-2.31.1.tar.gz
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND
        CFLAGS=-fPIC
        CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
        <SOURCE_DIR>/configure --prefix=${CMAKE_BINARY_DIR}/binutils
        BUILD_COMMAND make
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib/dyninst-tpls/lib/libiberty
        INSTALL_COMMAND
        install <SOURCE_DIR>/libiberty/libiberty.a <INSTALL_DIR>
    )

    set(_li_root ${CMAKE_INSTALL_PREFIX}/lib/dyninst-tpls)
    set(_li_inc_dirs
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/binutils/src/LibIberty-External/include>
        $<INSTALL_INTERFACE:${_li_root}/include>)
    set(_li_lib_dirs
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/binutils/src/LibIberty-External/libiberty>
        $<INSTALL_INTERFACE:${_li_root}/lib>)
    set(_li_libs
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/binutils/src/LibIberty-External/libiberty/libiberty.a>
        $<INSTALL_INTERFACE:${_li_root}/lib/libiberty/libiberty.a>)

    # For backward compatibility
    set(IBERTY_FOUND TRUE)
    set(IBERTY_BUILD TRUE)
endif()

# -------------- EXPORT VARIABLES ---------------------------------------------

dyninst_add_interface_library(LibIberty "LibIberty interface library")

foreach(_DIR_TYPE inc lib)
    if(_li_${_DIR_TYPE}_dirs)
        list(REMOVE_DUPLICATES _li_${_DIR_TYPE}_dirs)
    endif()
endforeach()

target_include_directories(LibIberty INTERFACE ${_li_inc_dirs})
target_link_directories(LibIberty INTERFACE ${_lib_lib_dirs})
target_link_libraries(LibIberty INTERFACE ${_li_libs})

set(LibIberty_ROOT_DIR ${_li_root}
    CACHE PATH "Base directory the of LibIberty installation" FORCE)
set(LibIberty_INCLUDE_DIRS ${_li_inc_dirs}
    CACHE PATH "LibIberty include directories" FORCE)
set(LibIberty_LIBRARY_DIRS ${_li_lib_dirs}
    CACHE PATH "LibIberty library directory" FORCE)
set(LibIberty_LIBRARIES ${_li_libs}
    CACHE FILEPATH "LibIberty library files" FORCE)

# For backward compatibility only
set(IBERTY_LIBRARIES ${LibIberty_LIBRARIES})

dyninst_message(STATUS "LibIberty include dirs: ${LibIberty_INCLUDE_DIRS}")
dyninst_message(STATUS "LibIberty library dirs: ${LibIberty_LIBRARY_DIRS}")
dyninst_message(STATUS "LibIberty libraries: ${LibIberty_LIBRARIES}")
