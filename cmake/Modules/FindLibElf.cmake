# ========================================================================================
# FindLibElf.cmake
#
# Find libelf include dirs and libraries
#
# ----------------------------------------
#
# Use this module by invoking find_package with the form::
#
# find_package(LibElf [version] [EXACT]      # Minimum or EXACT version e.g. 0.173
# [REQUIRED]             # Fail with error if libelf is not found )
#
# This module reads hints about search locations from variables::
#
# LibElf_ROOT_DIR         - Base directory the of libelf installation LibElf_INCLUDEDIR -
# Hint directory that contains the libelf headers files LibElf_LIBRARYDIR       - Hint
# directory that contains the libelf library files
#
# and saves search results persistently in CMake cache entries::
#
# LibElf_FOUND                    - True if headers and requested libraries were found
# LibElf_INCLUDE_DIRS     - libelf include directories LibElf_LIBRARY_DIRS - Link
# directories for libelf libraries LibElf_LIBRARIES                - libelf library files
#
# Based on the version by Bernhard Walle <bernhard.walle@gmx.de> Copyright (c) 2008
#
# ========================================================================================

include(DyninstSystemPaths)

# Non-standard subdirectories to search
set(_path_suffixes libelf libelfls elfutils)

find_path(
    LibElf_INCLUDE_DIR
    NAMES libelf.h
    HINTS ${LibElf_ROOT_DIR}/include ${LibElf_ROOT_DIR} ${LibElf_INCLUDEDIR}
    PATHS ${DYNINST_SYSTEM_INCLUDE_PATHS}
    PATH_SUFFIXES ${_path_suffixes}
    DOC "libelf include directories")

find_library(
    LibElf_LIBRARIES
    NAMES libelf.so.1 libelf.so
    HINTS ${LibElf_ROOT_DIR}/lib ${LibElf_ROOT_DIR} ${LibElf_LIBRARYDIR}
    PATHS ${DYNINST_SYSTEM_LIBRARY_PATHS}
    PATH_SUFFIXES ${_path_suffixes})

# Find the library with the highest version
set(_max_ver 0.0)
set(_max_ver_lib)
foreach(l ${LibElf_LIBRARIES})
    get_filename_component(_elf_realpath ${LibElf_LIBRARIES} REALPATH)
    string(REGEX MATCH "libelf\\-(.+)\\.so\\.*$" res ${_elf_realpath})

    # The library version number is stored in CMAKE_MATCH_1
    set(_cur_ver ${CMAKE_MATCH_1})

    if(${_cur_ver} VERSION_GREATER ${_max_ver})
        set(_max_ver ${_cur_ver})
        set(_max_ver_lib ${l})
    endif()
endforeach()

# Set the exported variables to the best match
set(LibElf_LIBRARIES ${_max_ver_lib})
set(LibElf_VERSION ${_max_ver})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    LibElf
    FOUND_VAR LibElf_FOUND
    REQUIRED_VARS LibElf_LIBRARIES LibElf_INCLUDE_DIR
    VERSION_VAR LibElf_VERSION)

# Export cache variables
if(LibElf_FOUND)
    set(LibElf_INCLUDE_DIRS ${LibElf_INCLUDE_DIR})
    set(LibElf_LIBRARIES ${LibElf_LIBRARIES})

    # Because we only report the library with the largest version, we are guaranteed there
    # is only one file in LibElf_LIBRARIES
    get_filename_component(_elf_dir ${LibElf_LIBRARIES} DIRECTORY)
    set(LibElf_LIBRARY_DIRS ${_elf_dir} "${_elf_dir}/elfutils")

    add_library(LibElf::LibElf INTERFACE IMPORTED)
    target_include_directories(LibElf::LibElf INTERFACE ${LibElf_INCLUDE_DIR})
    target_link_directories(LibElf::LibElf INTERFACE ${LibElf_LIBRARY_DIRS})
    target_link_libraries(LibElf::LibElf INTERFACE ${LibElf_LIBRARIES})
endif()
