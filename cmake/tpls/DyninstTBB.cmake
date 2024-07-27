# =====================================================================================
# ThreadingBuildingBlocks.cmake
#
# Configure Intel's Threading Building Blocks for Dyninst
#
# ----------------------------------------
#
# Accepts the following CMake variables
#
# TBB_ROOT_DIR        - Hint directory that contains the TBB installation TBB_INCLUDEDIR -
# Hint directory that contains the TBB headers files TBB_LIBRARYDIR      - Hint directory
# that contains the TBB library files TBB_LIBRARY         - Alias for TBB_LIBRARYDIR
# TBB_USE_DEBUG_BUILD - Use debug version of tbb libraries, if present TBB_MIN_VERSION -
# Minimum acceptable version of TBB
#
# Directly exports the following CMake variables
#
# TBB_ROOT_DIR        - Computed base directory of TBB installation TBB_INCLUDE_DIRS    -
# TBB include directory TBB_INCLUDE_DIR     - Alias for TBB_INCLUDE_DIRS TBB_LIBRARY_DIRS
# - TBB library directory TBB_LIBRARY_DIR - Alias for TBB_LIBRARY_DIRS TBB_DEFINITIONS -
# TBB compiler definitions TBB_LIBRARIES       - TBB library files
#
# TBB_<c>_LIBRARY_RELEASE - Path to the release version of component <c>
# TBB_<c>_LIBRARY_DEBUG   - Path to the debug version of component <c>
#
# NOTE: The exported TBB_ROOT_DIR can be different from the value provided by the user in
# the case that it is determined to build TBB from source. In such a case, TBB_ROOT_DIR
# will contain the directory of the from-source installation.
#
# See Modules/FindTBB.cmake for additional input and exported variables
#
# =====================================================================================

include_guard(GLOBAL)

# always provide Dyninst::TBB even if it is a dummy
dyninst_add_interface_library(TBB "Threading Building Blocks")

if(TBB_FOUND)
    return()
endif()

# -------------- RUNTIME CONFIGURATION ----------------------------------------

# Use debug versions of TBB libraries
set(TBB_USE_DEBUG_BUILD
    OFF
    CACHE BOOL "Use debug versions of TBB libraries")

# Minimum version of TBB (assumes a dotted-decimal format: YYYY.XX)
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(_tbb_min_version 2019.7)
else()
    set(_tbb_min_version 2018.6)
endif()

set(TBB_MIN_VERSION
    ${_tbb_min_version}
    CACHE STRING "Minimum version of TBB (assumes a dotted-decimal format: YYYY.XX)")

if(${TBB_MIN_VERSION} VERSION_LESS ${_tbb_min_version})
    dyninst_message(
        FATAL_ERROR
        "Requested TBB version ${TBB_MIN_VERSION} is less than minimum supported version ${_tbb_min_version}"
        )
endif()

# -------------- PATHS --------------------------------------------------------

# TBB root directory
set(TBB_ROOT_DIR
    "/usr"
    CACHE PATH "TBB root directory")

# TBB include directory hint
set(TBB_INCLUDEDIR
    "${TBB_ROOT_DIR}/include"
    CACHE INTERNAL "TBB include directory")

# TBB library directory hint
set(TBB_LIBRARYDIR
    "${TBB_ROOT_DIR}/lib"
    CACHE INTERNAL "TBB library directory")

# Translate to FindTBB names
set(TBB_LIBRARY ${TBB_LIBRARYDIR})
set(TBB_INCLUDE_DIR ${TBB_INCLUDEDIR})

# The specific TBB libraries we need NB: This should _NOT_ be a cache variable
set(_tbb_components tbb tbbmalloc_proxy tbbmalloc)

if(NOT BUILD_TBB)
    find_package(TBB ${TBB_MIN_VERSION} COMPONENTS ${_tbb_components})
endif()

# -------------- SOURCE BUILD -------------------------------------------------
if(TBB_FOUND)
    # Force the cache entries to be updated Normally, these would not be exported.
    # However, we need them in the Testsuite
    set(TBB_INCLUDE_DIRS
        ${TBB_INCLUDE_DIRS}
        CACHE PATH "TBB include directory" FORCE)
    set(TBB_LIBRARY_DIRS
        ${TBB_LIBRARY_DIRS}
        CACHE PATH "TBB library directory" FORCE)
    set(TBB_DEFINITIONS
        ${TBB_DEFINITIONS}
        CACHE STRING "TBB compiler definitions" FORCE)
    set(TBB_LIBRARIES
        ${TBB_LIBRARIES}
        CACHE FILEPATH "TBB library files" FORCE)
elseif(STERILE_BUILD)
    dyninst_message(FATAL_ERROR
                    "TBB not found and cannot be downloaded because build is sterile.")
elseif(NOT BUILD_TBB)
    dyninst_message(
        FATAL_ERROR
        "TBB was not found. Either configure cmake to find TBB properly or set BUILD_TBB=ON to download and build"
        )
else()
    # If we didn't find a suitable version on the system, then download one from the web
    dyninst_message(STATUS "${ThreadingBuildingBlocks_ERROR_REASON}")
    dyninst_message(STATUS
                    "Attempting to build TBB(${TBB_MIN_VERSION}) as external project")

    if(NOT UNIX)
        dyninst_message(FATAL_ERROR
                        "Building TBB from source is not supported on this platform")
    endif()

    set(TBB_ROOT_DIR
        ${CMAKE_INSTALL_PREFIX}
        CACHE PATH "TBB root directory" FORCE)

    set(_tbb_libraries)
    set(_tbb_components_cfg)
    set(_tbb_library_dirs $<BUILD_INTERFACE:${TPL_STAGING_PREFIX}/lib>
                          $<INSTALL_INTERFACE:${INSTALL_LIB_DIR}/${TPL_INSTALL_LIB_DIR}>)
    set(_tbb_include_dirs
        $<BUILD_INTERFACE:${TPL_STAGING_PREFIX}/include>
        $<INSTALL_INTERFACE:${INSTALL_LIB_DIR}/${TPL_INSTALL_INCLUDE_DIR}>)

    # Forcibly update the cache variables
    set(TBB_INCLUDE_DIRS
        "${_tbb_include_dirs}"
        CACHE PATH "TBB include directory" FORCE)
    set(TBB_LIBRARY_DIRS
        "${_tbb_library_dirs}"
        CACHE PATH "TBB library directory" FORCE)
    set(TBB_DEFINITIONS
        ""
        CACHE STRING "TBB compiler definitions" FORCE)

    foreach(c ${_tbb_components})
        # Generate make target names
        if(${c} STREQUAL tbbmalloc_proxy)
            # tbbmalloc_proxy is spelled tbbproxy in their Makefiles
            list(APPEND _tbb_components_cfg tbbproxy_release)
        else()
            list(APPEND _tbb_components_cfg ${c}_release)
        endif()

        set(_tbb_${c}_lib
            $<BUILD_INTERFACE:${TPL_STAGING_PREFIX}/lib/lib${c}${CMAKE_SHARED_LIBRARY_SUFFIX}>
            $<INSTALL_INTERFACE:${c}>)

        # Generate library filenames
        list(APPEND _tbb_libraries ${_tbb_${c}_lib})
        list(APPEND _tbb_build_byproducts
             "${TPL_STAGING_PREFIX}/lib/lib${c}${CMAKE_SHARED_LIBRARY_SUFFIX}")

        foreach(t RELEASE DEBUG)
            set(TBB_${c}_LIBRARY_${t}
                "${_tbb_${c}_lib}"
                CACHE FILEPATH "" FORCE)
        endforeach()
    endforeach()

    set(TBB_LIBRARIES
        "${_tbb_libraries}"
        CACHE FILEPATH "TBB library files" FORCE)

    # Split the dotted decimal version into major/minor parts
    string(REGEX REPLACE "\\." ";" _tbb_download_name ${TBB_MIN_VERSION})
    list(GET _tbb_download_name 0 _tbb_ver_major)
    list(GET _tbb_download_name 1 _tbb_ver_minor)

    set(_tbb_prefix_dir ${PROJECT_BINARY_DIR}/tbb)

    # Set the compiler for TBB It assumes gcc and tests for Intel, so clang is the only
    # one that needs special treatment.
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        set(_tbb_compiler "compiler=clang")
    endif()

    find_program(
        MAKE_EXECUTABLE
        NAMES make gmake
        PATH_SUFFIXES bin)

    if(NOT MAKE_EXECUTABLE AND CMAKE_GENERATOR MATCHES "Ninja")
        dyninst_message(
            FATAL_ERROR
            "make/gmake executable not found. Please re-run with -DMAKE_EXECUTABLE=/path/to/make"
            )
    elseif(NOT MAKE_EXECUTABLE AND CMAKE_GENERATOR MATCHES "Makefiles")
        set(MAKE_EXECUTABLE "$(MAKE)")
    endif()

    include(ExternalProject)
    externalproject_add(
        TBB-External
        PREFIX ${_tbb_prefix_dir}
        URL https://github.com/ajanicijamd/oneTBB/archive/refs/tags/v${_tbb_ver_major}.${_tbb_ver_minor}.01.tar.gz
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ""
        BUILD_COMMAND
            ${CMAKE_COMMAND} -E env CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
            [=[LDFLAGS=-Wl,-rpath='$$ORIGIN']=] ${MAKE_EXECUTABLE} -C src
            ${_tbb_components_cfg} tbb_build_dir=${_tbb_prefix_dir}/src
            tbb_build_prefix=tbb ${_tbb_compiler}
        BUILD_BYPRODUCTS ${_tbb_build_byproducts}
        INSTALL_COMMAND "")

    # post-build target for installing build
    add_custom_command(
        TARGET TBB-External
        POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} ARGS -DLIBDIR=${TPL_STAGING_PREFIX}/lib
            -DINCDIR=${TPL_STAGING_PREFIX}/include -DPREFIX=${_tbb_prefix_dir}
            -DCMAKE_STRIP=${CMAKE_STRIP} -P
            ${CMAKE_CURRENT_LIST_DIR}/DyninstTBBInstall.cmake
        COMMENT "Installing TBB...")

    add_custom_target(
        install-tbb-external
        COMMAND
            ${CMAKE_COMMAND} -DLIBDIR=${TPL_STAGING_PREFIX}/lib
            -DINCDIR=${TPL_STAGING_PREFIX}/include -DPREFIX=${_tbb_prefix_dir} -P
            ${CMAKE_CURRENT_LIST_DIR}/DyninstTBBInstall.cmake
        COMMENT "Installing TBB...")
endif()

foreach(_DIR_TYPE INCLUDE LIBRARY)
    if(TBB_${_DIR_TYPE}_DIRS)
        list(REMOVE_DUPLICATES TBB_${_DIR_TYPE}_DIRS)
    endif()
endforeach()

target_include_directories(TBB SYSTEM INTERFACE ${TBB_INCLUDE_DIRS})
target_compile_definitions(TBB INTERFACE ${TBB_DEFINITIONS})
target_link_directories(TBB INTERFACE ${TBB_LIBRARY_DIRS})
target_link_libraries(TBB INTERFACE ${TBB_LIBRARIES})

dyninst_message(STATUS "TBB include directory: ${TBB_INCLUDE_DIRS}")
dyninst_message(STATUS "TBB library directory: ${TBB_LIBRARY_DIRS}")
dyninst_message(STATUS "TBB libraries: ${TBB_LIBRARIES}")
dyninst_message(STATUS "TBB definitions: ${TBB_DEFINITIONS}")
