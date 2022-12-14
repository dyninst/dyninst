#[=======================================================================[.rst:
FindLibValgrind
---------------

Find valgrind, a dynamic binary instrumentation framework.

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` target:

``Valgrind::Valgrind``
  The valgrind library, if found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``Valgrind_INCLUDE_DIRS``
  where to find valgrind.h, etc.
``Valgrind_LIBRARIES``
  the libraries to link against to use valgrind.
``Valgrind_FOUND``
  If false, do not try to use valgrind.
``Valgrind_VERSION``
  the version of the valgrind library found

#]=======================================================================]
cmake_policy(SET CMP0074 NEW) # Use <Package>_ROOT

if(Valgrind_FIND_QUIETLY)
    set(_quiet "QUIET")
endif()

if(NOT "x${Valgrind_FIND_VERSION}" STREQUAL "x")
    set(_version ">=${Valgrind_FIND_VERSION}")
endif()

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_VALGRIND ${_quiet} "valgrind${_version}")
endif()

if(PC_VALGRIND_FOUND)
		# FindPkgConfig sometimes gets the include dir wrong
		if("x${PC_VALGRIND_INCLUDE_DIRS}" STREQUAL "x")
			pkg_get_variable(PC_VALGRIND_INCLUDE_DIRS valgrind includedir)
		endif()
		
    set(Valgrind_INCLUDE_DIRS
        ${PC_VALGRIND_INCLUDE_DIRS}
        CACHE PATH "")
    set(Valgrind_LIBRARIES
        ${PC_VALGRIND_LINK_LIBRARIES}
        CACHE PATH "")
    set(Valgrind_VERSION
        ${PC_VALGRIND_VERSION}
        CACHE STRING "")
else()
    find_path(
        Valgrind_INCLUDE_DIRS
        NAMES valgrind.h
        PATH_SUFFIXES valgrind)

    find_library(
        Valgrind_LIBRARIES
        NAMES valgrind
        PATH_SUFFIXES valgrind)

    macro(_check_valgrind_version _file)
        file(STRINGS ${_file} _version_line
             REGEX "^#define __VALGRIND_MAJOR__[ \t]+[0-9]+")
        string(REGEX MATCH "[0-9]+" _major "${_version_line}")
        file(STRINGS ${_file} _version_line
             REGEX "^#define __VALGRIND_MINOR__[ \t]+[0-9]+")
        string(REGEX MATCH "[0-9]+" _minor "${_version_line}")
        set(Valgrind_VERSION "${_major}.${_minor}")
        unset(_version_line)
        unset(_major)
        unset(_minor)
    endmacro()
    
    if(EXISTS "${Valgrind_INCLUDE_DIRS}/valgrind.h")
    	_check_valgrind_version("${Valgrind_INCLUDE_DIRS}/valgrind.h")
    elseif(EXISTS "${Valgrind_INCLUDE_DIRS}/valgrind/valgrind.h")
    	_check_valgrind_version("${Valgrind_INCLUDE_DIRS}/valgrind/valgrind.h")
    endif()

    if("x${Valgrind_VERSION}" STREQUAL "x")
        message(FATAL_ERROR "Unable to find version for valgrind")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Valgrind
    FOUND_VAR Valgrind_FOUND
    REQUIRED_VARS Valgrind_LIBRARIES Valgrind_INCLUDE_DIRS
    VERSION_VAR Valgrind_VERSION)

if(Valgrind_FOUND)
    mark_as_advanced(Valgrind_INCLUDE_DIR)
    mark_as_advanced(Valgrind_LIBRARIES)

    # Some platforms explicitly list libelf as a dependency, so separate it out
    list(LENGTH Valgrind_LIBRARIES _cnt)
    if(${_cnt} GREATER 1)
        foreach(_l ${Valgrind_LIBRARIES})
            if(${_l} MATCHES "valgrind")
                set(_libdw ${_l})
            else()
                list(APPEND _link_libs ${_l})
            endif()
        endforeach()
    endif()
    unset(_cnt)

    if(NOT TARGET Valgrind::Valgrind)
        add_library(Valgrind::Valgrind UNKNOWN IMPORTED)
        set_target_properties(Valgrind::Valgrind PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                                      "${Valgrind_INCLUDE_DIRS}")

        if(NOT "x${_link_libs}" STREQUAL "x")
            set_target_properties(
                Valgrind::Valgrind PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                                        IMPORTED_LINK_DEPENDENT_LIBRARIES "${_link_libs}")
            set(Valgrind_LIBRARIES ${_libdw})
            unset(_libdw)
            unset(_link_libs)
        endif()

        set_target_properties(
            Valgrind::Valgrind PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                                    IMPORTED_LOCATION "${Valgrind_LIBRARIES}")
    endif()
endif()

unset(_quiet)
unset(_version)
