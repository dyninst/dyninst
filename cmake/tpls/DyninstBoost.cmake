# ========================================================================================================
# Boost.cmake
#
# Configure Boost for Dyninst
#
# ----------------------------------------
#
# Accepts the following CMake variables
#
# Boost_ROOT_DIR            - Hint directory that contains the Boost installation
# PATH_BOOST                - Alias for Boost_ROOT_DIR Boost_MIN_VERSION         - Minimum
# acceptable version of Boost Boost_USE_MULTITHREADED   - Use the multithreaded version of
# Boost Boost_USE_STATIC_RUNTIME  - Use libraries linked statically to the C++ runtime
#
# Options inherited from Modules/FindBoost.cmake that may be useful
#
# BOOST_INCLUDEDIR          - Hint directory that contains the Boost headers files
# BOOST_LIBRARYDIR          - Hint directory that contains the Boost library files
#
# Advanced options:
#
# Boost_DEBUG               - Enable debug output from FindBoost Boost_NO_SYSTEM_PATHS -
# Disable searching in locations not specified by hint variables
#
# Exports the following CMake cache variables
#
# Boost_ROOT_DIR            - Computed base directory the of Boost installation
# Boost_INCLUDE_DIRS        - Boost include directories Boost_INCLUDE_DIR - Alias for
# Boost_INCLUDE_DIRS Boost_LIBRARY_DIRS        - Link directories for Boost libraries
# Boost_DEFINES             - Boost compiler definitions Boost_LIBRARIES           - Boost
# library files Boost_<C>_LIBRARY_RELEASE - Release libraries to link for component <C>
# (<C> is upper-case) Boost_<C>_LIBRARY_DEBUG   - Debug libraries to link for component
# <C> Boost_THREAD_LIBRARY      - The filename of the Boost thread library
# Boost_USE_MULTITHREADED   - Use the multithreaded version of Boost
# Boost_USE_STATIC_RUNTIME  - Use libraries linked statically to the C++ runtime
#
# NOTE: The exported Boost_ROOT_DIR can be different from the value provided by the user
# in the case that it is determined to build Boost from source. In such a case,
# Boost_ROOT_DIR will contain the directory of the from-source installation.
#
# See Modules/FindBoost.cmake for additional input and exported variables
#
# ========================================================================================================

include_guard(GLOBAL)

# always provide Dyninst::Boost even if it is empty
dyninst_add_interface_library(Boost "Boost interface library")

if(Boost_FOUND)
    return()
endif()

# Need at least Boost-1.67 because of deprecated headers
set(_boost_min_version 1.67.0)

# Provide a default, if the user didn't specify
set(Boost_MIN_VERSION
    ${_boost_min_version}
    CACHE STRING "Minimum Boost version")

# Enforce minimum version
if(${Boost_MIN_VERSION} VERSION_LESS ${_boost_min_version})
    dyninst_message(
        FATAL_ERROR
        "Requested Boost-${Boost_MIN_VERSION} is less than minimum supported version (${_boost_min_version})"
        )
endif()

# -------------- RUNTIME CONFIGURATION ----------------------------------------

# Use the multithreaded version of Boost NB: This _must_ be a cache variable as it
# controls the tagged layout of Boost library names
set(Boost_USE_MULTITHREADED
    ON
    CACHE BOOL "Enable multithreaded Boost libraries")

# Don't use libraries linked statically to the C++ runtime NB: This _must_ be a cache
# variable as it controls the tagged layout of Boost library names
set(Boost_USE_STATIC_RUNTIME
    OFF
    CACHE BOOL "Enable usage of libraries statically linked to C++ runtime")

# If using multithreaded Boost, make sure Threads has been intialized
if(Boost_USE_MULTITHREADED AND NOT DEFINED CMAKE_THREAD_LIBS_INIT)
    find_package(Threads)
endif()

# Enable debug output from FindBoost
set(Boost_DEBUG
    OFF
    CACHE BOOL "Enable debug output from FindBoost")

# -------------- PATHS --------------------------------------------------------

# By default, search system paths
set(Boost_NO_SYSTEM_PATHS
    OFF
    CACHE BOOL "Disable searching in locations not specified by hint variables")

# A sanity check This must be done _before_ the cache variables are set
if(PATH_BOOST AND Boost_ROOT_DIR)
    dyninst_message(
        FATAL_ERROR
        "PATH_BOOST AND Boost_ROOT_DIR both specified. Please provide only one")
endif()

# Provide a default root directory
if(NOT PATH_BOOST AND NOT Boost_ROOT_DIR)
    set(PATH_BOOST "/usr")
endif()

# Set the default location to look for Boost
set(Boost_ROOT_DIR
    ${PATH_BOOST}
    CACHE PATH "Base directory the of Boost installation")

# In FindBoost, Boost_ROOT_DIR is spelled BOOST_ROOT
set(BOOST_ROOT ${Boost_ROOT_DIR})

# -------------- COMPILER DEFINES ---------------------------------------------

set(_boost_defines)

# Disable auto-linking
list(APPEND _boost_defines BOOST_ALL_NO_LIB=1)

# Disable generating serialization code in boost::multi_index
list(APPEND _boost_defines BOOST_MULTI_INDEX_DISABLE_SERIALIZATION)

# There are broken versions of MSVC that won't handle variadic templates correctly
# (despite the C++11 test case passing).
if(MSVC)
    list(APPEND _boost_defines BOOST_NO_CXX11_VARIADIC_TEMPLATES)
endif()

set(Boost_DEFINES
    ${_boost_defines}
    CACHE STRING "Boost compiler defines")
add_compile_definitions(${Boost_DEFINES})

# -------------- INTERNALS ----------------------------------------------------

# Disable Boost's own CMake as it's known to be buggy NB: This should not be a cache
# variable
set(Boost_NO_BOOST_CMAKE ON)

# The required Boost library components NB: These are just the ones that require
# compilation/linking This should _not_ be a cache variable
set(_boost_components
    atomic
    chrono
    date_time
    filesystem
    system
    thread
    timer)

if(NOT BUILD_BOOST)
    find_package(Boost ${Boost_MIN_VERSION} QUIET COMPONENTS ${_boost_components})
endif()

# -------------- SOURCE BUILD -------------------------------------------------

if(Boost_FOUND AND NOT BUILD_BOOST)
    # Force the cache entries to be updated Normally, these would not be exported.
    # However, we need them in the Testsuite
    set(Boost_INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}
        CACHE PATH "Boost include directory" FORCE)
    set(Boost_LIBRARY_DIRS
        ${Boost_LIBRARY_DIRS}
        CACHE PATH "Boost library directory" FORCE)
    set(Boost_INCLUDE_DIR
        ${Boost_INCLUDE_DIR}
        CACHE PATH "Boost include directory" FORCE)
elseif(NOT Boost_FOUND AND STERILE_BUILD)
    dyninst_message(FATAL_ERROR
                    "Boost not found and cannot be downloaded because build is sterile.")
elseif(NOT BUILD_BOOST)
    dyninst_message(
        FATAL_ERROR
        "Boost was not found. Either configure cmake to find Boost properly or set BUILD_BOOST=ON to download and build"
        )
else()
    dyninst_add_option(BOOST_LINK_STATIC "Link to boost libraries statically" ON)
    # If we didn't find a suitable version on the system, then download one from the web
    dyninst_add_cache_option(BOOST_DOWNLOAD_VERSION "1.79.0"
                             CACHE STRING "Version of boost to download and install")

    # If the user specifies a version other than BOOST_DOWNLOAD_VERSION, use that version.
    if(${BOOST_DOWNLOAD_VERSION} VERSION_LESS ${Boost_MIN_VERSION})
        dyninst_message(
            FATAL_ERROR
            "Boost download version is set to ${BOOST_DOWNLOAD_VERSION} but Boost minimum version is set to ${Boost_MIN_VERSION}"
            )
    endif()

    dyninst_message(STATUS
                    "Attempting to build ${BOOST_DOWNLOAD_VERSION} as external project")

    if(Boost_USE_MULTITHREADED)
        set(_boost_threading multi)
    else()
        set(_boost_threading single)
    endif()

    if(Boost_USE_STATIC_RUNTIME)
        set(_boost_runtime_link static)
    else()
        set(_boost_runtime_link shared)
    endif()

    # Change the base directory
    set(Boost_ROOT_DIR
        ${TPL_STAGING_PREFIX}
        CACHE PATH "Base directory the of Boost installation" FORCE)

    # Update the exported variables
    set(Boost_INCLUDE_DIRS
        "$<BUILD_INTERFACE:${TPL_STAGING_PREFIX}/include>;$<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}/${TPL_INSTALL_INCLUDE_DIR}>"
        CACHE PATH "Boost include directory" FORCE)
    set(Boost_LIBRARY_DIRS
        "$<BUILD_INTERFACE:${TPL_STAGING_PREFIX}/lib>;$<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}/${TPL_INSTALL_LIB_DIR}>"
        CACHE PATH "Boost library directory" FORCE)
    set(Boost_INCLUDE_DIR
        ${Boost_INCLUDE_DIRS}
        CACHE PATH "Boost include directory" FORCE)

    if(BOOST_LINK_STATIC)
        set(_BOOST_LINK static)
    else()
        set(_BOOST_LINK shared)
    endif()

    set(BOOST_ARGS link=${_BOOST_LINK} runtime-link=${_boost_runtime_link}
                   threading=${_boost_threading})
    if(WIN32)
        # NB: We need to build both debug/release on windows as we don't use
        # CMAKE_BUILD_TYPE
        set(BOOST_BOOTSTRAP call bootstrap.bat)
        set(BOOST_BUILD ".\\b2")
        if(CMAKE_SIZEOF_VOID_P STREQUAL "8")
            list(APPEND BOOST_ARGS address-model=64)
        endif()
    else()
        set(BOOST_BOOTSTRAP "./bootstrap.sh")
        set(BOOST_BUILD "./b2")
        if(CMAKE_BUILD_TYPE MATCHES "^(Debug|DEBUG)$")
            list(APPEND BOOST_ARGS variant=debug)
        else()
            list(APPEND BOOST_ARGS variant=release)
        endif()
    endif()

    # Join the component names together to pass to --with-libraries during bootstrap
    set(_boost_lib_names "headers,")
    foreach(c ${_boost_components})
        # list(JOIN ...) is in cmake 3.12
        string(CONCAT _boost_lib_names "${_boost_lib_names}${c},")
    endforeach()

    if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang|Intel)")
        list(APPEND BOOST_ARGS cflags=-fPIC cxxflags=-fPIC)
    endif()

    string(REPLACE "." "_" _boost_download_filename ${BOOST_DOWNLOAD_VERSION})
    # zip is subject to locales on Unix
    set(_boost_download_ext "zip")
    if(UNIX)
        set(_boost_download_ext "tar.gz")
    endif()

    set(_LIB_SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
    if(BOOST_LINK_STATIC)
        set(_LIB_SUFFIX "${CMAKE_STATIC_LIBRARY_SUFFIX}")
    endif()

    if(WIN32)
        # We need to specify different library names for debug vs release
        set(Boost_LIBRARIES "")
        foreach(c ${_boost_components})
            list(APPEND Boost_LIBRARIES "optimized libboost_${c} debug libboost_${c}-gd ")
            list(APPEND _boost_build_byproducts
                 "{TPL_STAGING_PREFIX}/lib/libboost_${c}${_LIB_SUFFIX}")
            set(Boost_${c}_LIBRARY
                $<BUILD_INTERFACE:${TPL_STAGING_PREFIX}/lib/libboost_${c}${_LIB_SUFFIX}>
                $<INSTALL_INTERFACE:boost_${c}>)
            set(Boost_${c}_LIBRARY_DEBUG
                $<BUILD_INTERFACE:${TPL_STAGING_PREFIX}/lib/libboost_${c}${_LIB_SUFFIX}>
                $<INSTALL_INTERFACE:libboost_${c}-gd>)

            # Also export cache variables for the file location of each library
            string(TOUPPER ${c} _basename)
            set(Boost_${_basename}_LIBRARY_RELEASE
                "${Boost_${c}_LIBRARY}"
                CACHE FILEPATH "" FORCE)
            set(Boost_${_basename}_LIBRARY_DEBUG
                "${Boost_${c}_LIBRARY_DEBUG}"
                CACHE FILEPATH "" FORCE)
        endforeach()
    else()
        # Transform the component names into the library filenames e.g., system ->
        # boost_system
        set(Boost_LIBRARIES "")
        foreach(c ${_boost_components})
            set(Boost_${c}_LIBRARY
                $<BUILD_INTERFACE:${TPL_STAGING_PREFIX}/lib/libboost_${c}${_LIB_SUFFIX}>
                $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${INSTALL_LIB_DIR}/${TPL_INSTALL_LIB_DIR}/libboost_${c}${_LIB_SUFFIX}>
                )
            list(APPEND _boost_build_byproducts
                 "${TPL_STAGING_PREFIX}/lib/libboost_${c}${_LIB_SUFFIX}")
            list(APPEND Boost_LIBRARIES "${Boost_${c}_LIBRARY}")

            # Also export cache variables for the file location of each library
            string(TOUPPER ${c} _basename)
            set(Boost_${_basename}_LIBRARY_RELEASE
                "${Boost_${c}_LIBRARY}"
                CACHE FILEPATH "" FORCE)
            set(Boost_${_basename}_LIBRARY_DEBUG
                "${Boost_${c}_LIBRARY}"
                CACHE FILEPATH "" FORCE)
        endforeach()
    endif()

    include(ExternalProject)
    externalproject_add(
        Boost-External
        PREFIX ${PROJECT_BINARY_DIR}/boost
        GIT_REPOSITORY https://github.com/boostorg/boost.git
        GIT_TAG boost-${BOOST_DOWNLOAD_VERSION}
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ${BOOST_BOOTSTRAP} --prefix=${Boost_ROOT_DIR}
                          --with-libraries=${_boost_lib_names}
        BUILD_COMMAND ${BOOST_BUILD} --ignore-site-config --prefix=${Boost_ROOT_DIR} -j2
                      ${BOOST_ARGS} -d0 install
        BUILD_BYPRODUCTS ${_boost_build_byproducts}
        INSTALL_COMMAND "")

    # target for re-executing the installation
    add_custom_target(
        install-boost-external
        COMMAND ${BOOST_BUILD} ${BOOST_ARGS} -d0 install
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/boost/src/Boost-External
        COMMENT "Installing Boost...")
endif()

# -------------- EXPORT VARIABLES ---------------------------------------------

# Export Boost_THREAD_LIBRARY
list(FIND _boost_components "thread" _building_threads)
if(Boost_USE_MULTITHREADED AND ${_building_threads})
    # On Windows, always use the debug version On Linux, we don't use tagged builds, so
    # the debug/release filenames are the same
    set(Boost_THREAD_LIBRARY
        ${Boost_THREAD_LIBRARY_DEBUG}
        CACHE FILEPATH "Boost thread library")
endif()

# Add the system thread library
if(Boost_USE_MULTITHREADED)
    list(APPEND Boost_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
endif()

# Export the complete set of libraries
set(Boost_LIBRARIES
    ${Boost_LIBRARIES}
    CACHE FILEPATH "Boost library files" FORCE)

target_include_directories(Boost SYSTEM INTERFACE ${Boost_INCLUDE_DIRS})
target_compile_definitions(Boost INTERFACE ${Boost_DEFINITIONS})
target_link_directories(Boost INTERFACE ${Boost_LIBRARY_DIRS})
target_link_libraries(Boost INTERFACE ${Boost_LIBRARIES})

dyninst_message(STATUS "Boost includes: ${Boost_INCLUDE_DIRS}")
dyninst_message(STATUS "Boost library dirs: ${Boost_LIBRARY_DIRS}")
dyninst_message(STATUS "Boost thread library: ${Boost_THREAD_LIBRARY}")
dyninst_message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
