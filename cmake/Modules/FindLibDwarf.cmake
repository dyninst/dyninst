# ===================================================================================
# FindLibDwarf.cmake
#
# Find libdw include dirs and libraries
#
# ----------------------------------------
#
# Use this module by invoking find_package with the form::
#
# find_package(LibDwarf [version] [EXACT]      # Minimum or EXACT version e.g. 0.173
# [REQUIRED]             # Fail with error if libdw is not found )
#
# This module reads hints about search locations from variables::
#
# LibDwarf_ROOT_DIR               - Base directory the of libdw installation
# LibDwarf_INCLUDEDIR             - Hint directory that contains the libdw headers files
# LibDwarf_LIBRARYDIR             - Hint directory that contains the libdw library files
#
# and saves search results persistently in CMake cache entries::
#
# LibDwarf_FOUND                  - True if headers and requested libraries were found
# LibDwarf_INCLUDE_DIRS   - libdw include directories LibDwarf_LIBRARY_DIRS   - Link
# directories for libdw libraries LibDwarf_LIBRARIES              - libdw library files
#
# ===================================================================================

include(DyninstSystemPaths)

# Non-standard subdirectories to search
set(_path_suffixes libdw libdwarf elfutils)

find_path(
    LibDwarf_INCLUDE_DIR
    NAMES libdw.h
    HINTS ${LibDwarf_ROOT_DIR}/include ${LibDwarf_ROOT_DIR} ${LibDwarf_INCLUDEDIR}
    PATHS ${DYNINST_SYSTEM_INCLUDE_PATHS}
    PATH_SUFFIXES ${_path_suffixes}
    DOC "libdw include directories")

find_library(
    LibDwarf_LIBRARIES
    NAMES libdw.so.1 libdw.so
    HINTS ${LibDwarf_ROOT_DIR}/lib ${LibDwarf_ROOT_DIR} ${LibDwarf_LIBRARYDIR}
    PATHS ${DYNINST_SYSTEM_LIBRARY_PATHS}
    PATH_SUFFIXES ${_path_suffixes})

# Find the library with the highest version
set(_max_ver 0.0)
set(_max_ver_lib)
foreach(l ${LibDwarf_LIBRARIES})
    get_filename_component(_dw_realpath ${LibDwarf_LIBRARIES} REALPATH)
    string(REGEX MATCH "libdw\\-(.+)\\.so\\.*$" res ${_dw_realpath})

    # The library version number is stored in CMAKE_MATCH_1
    set(_cur_ver ${CMAKE_MATCH_1})

    if(${_cur_ver} VERSION_GREATER ${_max_ver})
        set(_max_ver ${_cur_ver})
        set(_max_ver_lib ${l})
    endif()
endforeach()

# Set the exported variables to the best match
set(LibDwarf_LIBRARIES ${_max_ver_lib})
set(LibDwarf_VERSION ${_max_ver})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    LibDwarf
    FOUND_VAR LibDwarf_FOUND
    REQUIRED_VARS LibDwarf_LIBRARIES LibDwarf_INCLUDE_DIR
    VERSION_VAR LibDwarf_VERSION)

# Export cache variables
if(LibDwarf_FOUND)
    set(LibDwarf_INCLUDE_DIRS ${LibDwarf_INCLUDE_DIR})
    set(LibDwarf_LIBRARIES ${LibDwarf_LIBRARIES})

    # Because we only report the library with the largest version, we are guaranteed there
    # is only one file in LibDwarf_LIBRARIES
    get_filename_component(_dw_dir ${LibDwarf_LIBRARIES} DIRECTORY)
    set(LibDwarf_LIBRARY_DIRS ${_dw_dir})

    add_library(LibDwarf::LibDwarf INTERFACE IMPORTED)
    target_include_directories(LibDwarf::LibDwarf INTERFACE ${LibDwarf_INCLUDE_DIR})
    target_link_directories(LibDwarf::LibDwarf INTERFACE ${LibDwarf_LIBRARY_DIRS})
    target_link_libraries(LibDwarf::LibDwarf INTERFACE ${LibDwarf_LIBRARIES})
endif()
