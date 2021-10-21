#========================================================================================================
# Boost.cmake
#
# Configure Boost for Dyninst
#
#   ----------------------------------------
#
# Accepts the following CMake variables
#
# Boost_ROOT_DIR            - Hint directory that contains the Boost installation
# PATH_BOOST                - Alias for Boost_ROOT_DIR
# Boost_MIN_VERSION         - Minimum acceptable version of Boost
# Boost_USE_MULTITHREADED   - Use the multithreaded version of Boost
# Boost_USE_STATIC_RUNTIME  - Use libraries linked statically to the C++ runtime
#
# Options inherited from Modules/FindBoost.cmake that may be useful
#
# BOOST_INCLUDEDIR          - Hint directory that contains the Boost headers files
# BOOST_LIBRARYDIR          - Hint directory that contains the Boost library files
#
# Advanced options:
#
# Boost_DEBUG               - Enable debug output from FindBoost
# Boost_NO_SYSTEM_PATHS     - Disable searching in locations not specified by hint variables 
#
# Exports the following CMake cache variables
#
# Boost_ROOT_DIR            - Computed base directory the of Boost installation
# Boost_INCLUDE_DIRS        - Boost include directories
# Boost_INCLUDE_DIR         - Alias for Boost_INCLUDE_DIRS
# Boost_LIBRARY_DIRS        - Link directories for Boost libraries
# Boost_DEFINES             - Boost compiler definitions
# Boost_LIBRARIES           - Boost library files
# Boost_<C>_LIBRARY_RELEASE - Release libraries to link for component <C> (<C> is upper-case)
# Boost_<C>_LIBRARY_DEBUG   - Debug libraries to link for component <C>
# Boost_THREAD_LIBRARY      - The filename of the Boost thread library
# Boost_USE_MULTITHREADED   - Use the multithreaded version of Boost
# Boost_USE_STATIC_RUNTIME  - Use libraries linked statically to the C++ runtime
#
# NOTE:
# The exported Boost_ROOT_DIR can be different from the value provided by the user in the case that
# it is determined to build Boost from source. In such a case, Boost_ROOT_DIR will contain the
# directory of the from-source installation.
#
# See Modules/FindBoost.cmake for additional input and exported variables
#
#========================================================================================================

if(Boost_FOUND)
  return()
endif()

# Need at least Boost-1.67 because of deprecated headers
set(_boost_min_version 1.70.0)

# Provide a default, if the user didn't specify
set(Boost_MIN_VERSION ${_boost_min_version} CACHE STRING "Minimum Boost version")

# Enforce minimum version
if(${Boost_MIN_VERSION} VERSION_LESS ${_boost_min_version})
  message(FATAL_ERROR "Requested Boost-${Boost_MIN_VERSION} is less than minimum supported version (${_boost_min_version})")
endif()

# -------------- RUNTIME CONFIGURATION ----------------------------------------

# Use the multithreaded version of Boost
# NB: This _must_ be a cache variable as it
#     controls the tagged layout of Boost library names
set(Boost_USE_MULTITHREADED ON CACHE BOOL "Enable multithreaded Boost libraries")

# Don't use libraries linked statically to the C++ runtime
# NB: This _must_ be a cache variable as it
#     controls the tagged layout of Boost library names
set(Boost_USE_STATIC_RUNTIME OFF CACHE BOOL
    "Enable usage of libraries statically linked to C++ runtime")

# If using multithreaded Boost, make sure Threads has been intialized
if(Boost_USE_MULTITHREADED AND NOT DEFINED CMAKE_THREAD_LIBS_INIT)
  find_package(Threads)
endif()

# Enable debug output from FindBoost
set(Boost_DEBUG OFF CACHE BOOL "Enable debug output from FindBoost")

# -------------- PATHS --------------------------------------------------------

# By default, search system paths
set(Boost_NO_SYSTEM_PATHS OFF CACHE BOOL "Disable searching in locations not specified by hint variables")

# A sanity check
# This must be done _before_ the cache variables are set
if(PATH_BOOST AND Boost_ROOT_DIR)
  message(FATAL_ERROR "PATH_BOOST AND Boost_ROOT_DIR both specified. Please provide only one")
endif()

# Provide a default root directory
if(NOT PATH_BOOST AND NOT Boost_ROOT_DIR)
  set(PATH_BOOST "/usr")
endif()

# Set the default location to look for Boost
set(Boost_ROOT_DIR ${PATH_BOOST} CACHE PATH "Base directory the of Boost installation")

# In FindBoost, Boost_ROOT_DIR is spelled BOOST_ROOT
set(BOOST_ROOT ${Boost_ROOT_DIR})

# -------------- COMPILER DEFINES ---------------------------------------------

set(_boost_defines)

# Disable auto-linking
list(APPEND _boost_defines -DBOOST_ALL_NO_LIB=1)

# Disable generating serialization code in boost::multi_index
list(APPEND _boost_defines -DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION)
  
# There are broken versions of MSVC that won't handle variadic templates
# correctly (despite the C++11 test case passing).
if(MSVC)
  list(APPEND _boost_defines -DBOOST_NO_CXX11_VARIADIC_TEMPLATES)
endif()

set(Boost_DEFINES ${_boost_defines} CACHE STRING "Boost compiler defines")
add_definitions(${Boost_DEFINES})

# -------------- INTERNALS ----------------------------------------------------

# Disable Boost's own CMake as it's known to be buggy
# NB: This should not be a cache variable
set(Boost_NO_BOOST_CMAKE ON)

# The required Boost library components
# NB: These are just the ones that require compilation/linking
#     This should _not_ be a cache variable
set(_boost_components atomic chrono date_time filesystem thread timer)

find_package(Boost ${Boost_MIN_VERSION} COMPONENTS ${_boost_components})

# -------------- SOURCE BUILD -------------------------------------------------

if(Boost_FOUND)
  # Force the cache entries to be updated
  # Normally, these would not be exported. However, we need them in the Testsuite
  set(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} CACHE PATH "Boost include directory" FORCE)
  set(Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIRS} CACHE PATH "Boost library directory" FORCE)
  set(Boost_INCLUDE_DIR ${Boost_INCLUDE_DIR} CACHE PATH "Boost include directory" FORCE)
  add_library(boost SHARED IMPORTED)
elseif(NOT Boost_FOUND AND STERILE_BUILD)
  message(FATAL_ERROR "Boost not found and cannot be downloaded because build is sterile.")
else()
  # If we didn't find a suitable version on the system, then download one from the web
  set(_boost_download_version "1.69.0")
  
  # If the user specifies a version other than _boost_download_version, use that version.
  # NB: We know Boost_MIN_VERSION is >= _boost_min_version from earlier checks
  if(${Boost_MIN_VERSION} VERSION_LESS ${_boost_download_version} OR
     ${Boost_MIN_VERSION} VERSION_GREATER ${_boost_download_version})
    set(_boost_download_version ${Boost_MIN_VERSION})
  endif()
  message(STATUS "${Boost_ERROR_REASON}")
  message(STATUS "Attempting to build ${_boost_download_version} as external project")

  # This is an internal consistency check. Normal users should not trip this since
  # they cannot affect _boost_download_version.  
  if(${_boost_download_version} VERSION_LESS ${Boost_MIN_VERSION})
    message(FATAL_ERROR "Download version of Boost (${_boost_download_version}) "
                        "is older than minimum allowed version (${Boost_MIN_VERSION})")
  endif()
  
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
  set(Boost_ROOT_DIR ${CMAKE_INSTALL_PREFIX} CACHE PATH "Base directory the of Boost installation" FORCE)

  # Update the exported variables  
  set(Boost_INCLUDE_DIRS ${Boost_ROOT_DIR}/include CACHE PATH "Boost include directory" FORCE)
  set(Boost_LIBRARY_DIRS ${Boost_ROOT_DIR}/lib CACHE PATH "Boost library directory" FORCE)
  set(Boost_INCLUDE_DIR ${Boost_INCLUDE_DIRS} CACHE PATH "Boost include directory" FORCE)
  
  set(BOOST_ARGS
      --ignore-site-config
      --link=static
      --runtime-link=${_boost_runtime_link}
      --threading=${_boost_threading})
  if(WIN32)
    # NB: We need to build both debug/release on windows
    #     as we don't use CMAKE_BUILD_TYPE
    set(BOOST_BOOTSTRAP call bootstrap.bat)
    set(BOOST_BUILD ".\\b2")
    if(CMAKE_SIZEOF_VOID_P STREQUAL "8")
      list(APPEND BOOST_ARGS address-model=64)
    endif()
  else()
    set(BOOST_BOOTSTRAP "./bootstrap.sh")
    set(BOOST_BUILD "./b2")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      list(APPEND BOOST_ARGS variant=debug)
    else()
      list(APPEND BOOST_ARGS variant=release)
    endif()
  endif()

  # Join the component names together to pass to --with-libraries during bootstrap
  set(_boost_lib_names "")
  foreach(c ${_boost_components})
	# list(JOIN ...) is in cmake 3.12
    string(CONCAT _boost_lib_names "${_boost_lib_names}${c},")
  endforeach()

  include(ExternalProject)
  string(REPLACE "." "_" _boost_download_filename ${_boost_download_version})
  ExternalProject_Add(
    boost
    PREFIX ${CMAKE_BINARY_DIR}/boost
    URL http://downloads.sourceforge.net/project/boost/boost/${_boost_download_version}/boost_${_boost_download_filename}.zip
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} ${BOOST_BOOTSTRAP} --prefix=${Boost_ROOT_DIR} --with-libraries=${_boost_lib_names}
    BUILD_COMMAND ${BOOST_BUILD} ${BOOST_ARGS} install
    INSTALL_COMMAND ""
  )

  if(WIN32)
    # We need to specify different library names for debug vs release
    set(Boost_LIBRARIES "")
    foreach(c ${_boost_components})
      list(APPEND Boost_LIBRARIES "optimized libboost_${c} debug libboost_${c}-gd ")
      
      # Also export cache variables for the file location of each library
      string(TOUPPER ${c} _basename)
      set(Boost_${_basename}_LIBRARY_RELEASE "${Boost_LIBRARY_DIRS}/libboost_${c}.dll" CACHE FILEPATH "" FORCE)
      set(Boost_${_basename}_LIBRARY_DEBUG "${Boost_LIBRARY_DIRS}/libboost_${c}-gd.dll" CACHE FILEPATH "" FORCE)
    endforeach()
  else()
    # Transform the component names into the library filenames
    # e.g., system -> boost_system
    set(Boost_LIBRARIES "")
    foreach(c ${_boost_components})
      list(APPEND Boost_LIBRARIES "${Boost_LIBRARY_DIRS}/libboost_${c}.so")
      
      # Also export cache variables for the file location of each library
      string(TOUPPER ${c} _basename)
      set(Boost_${_basename}_LIBRARY_RELEASE "${Boost_LIBRARY_DIRS}/libboost_${c}.so" CACHE FILEPATH "" FORCE)
      set(Boost_${_basename}_LIBRARY_DEBUG "${Boost_LIBRARY_DIRS}/libboost_${c}.so" CACHE FILEPATH "" FORCE)
    endforeach()
  endif()
endif()

# -------------- EXPORT VARIABLES ---------------------------------------------

# Export Boost_THREAD_LIBRARY
list(FIND _boost_components "thread" _building_threads)
if(Boost_USE_MULTITHREADED AND ${_building_threads})
  # On Windows, always use the debug version
  # On Linux, we don't use tagged builds, so the debug/release filenames are the same
  set(Boost_THREAD_LIBRARY ${Boost_THREAD_LIBRARY_DEBUG} CACHE FILEPATH "Boost thread library")
endif()

# Add the system thread library
if(Boost_USE_MULTITHREADED)
  list(APPEND Boost_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
endif()

# Export the complete set of libraries
set(Boost_LIBRARIES ${Boost_LIBRARIES} CACHE FILEPATH "Boost library files" FORCE)

link_directories(${Boost_LIBRARY_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

message(STATUS "Boost includes: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost library dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost thread library: ${Boost_THREAD_LIBRARY}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
