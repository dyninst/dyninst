###############################
#   Boost.cmake
#
# Configure Boost for Dyninst
#
###############################

# Enable debug output from FindBoost
set(Boost_DEBUG OFF CACHE BOOL "Enable debug output from FindBoost")

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

# Set up compiler defines
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

set(Boost_DEFINES ${_boost_defines} CACHE STRING "Boost compiler defines" FORCE)
add_definitions(${Boost_DEFINES})

# Need Boost >= 1.61 for filesytem components
set(BOOST_MIN_VERSION 1.61.0 CACHE STRING "Minimum Boost version")

# Disable Boost's own CMake as it's known to be buggy
# NB: This should not be a cache variable
set(Boost_NO_BOOST_CMAKE ON)

# Set the default location to look for Boost
#
# NOTE: If you change this, also change the value in
#       the test for setting Boost_NO_SYSTEM_PATHS
#
set(PATH_BOOST "/usr" CACHE PATH "Path to Boost")

# PATH_BOOST is the user-facing version of BOOST_ROOT
if(PATH_BOOST)
  set(BOOST_ROOT ${PATH_BOOST})
endif()

# If the user specifies a Boost directory, don't look in the system locations
# This prevents finding an unintended version of Boost
#
#       -- WARNING --
#
#       If the user specifies a system directory in PATH_BOOST,
#       cmake will not find the desired version.
#
if((PATH_BOOST AND NOT (PATH_BOOST STREQUAL "/usr")) OR Boost_INCLUDE_DIRS OR Boost_LIBRARY_DIRS)
  set(Boost_NO_SYSTEM_PATHS ON)
endif()

# The required Boost library components
# NB: These are just the ones that require compilation/linking
#     This should _not_ be a cache variable
set(_boost_components atomic chrono date_time filesystem system thread timer)

find_package(Boost ${BOOST_MIN_VERSION} COMPONENTS ${_boost_components})

# If we didn't find a suitable version on the system, then download one from the web
if(NOT Boost_FOUND)
  set(_Boost_download_version "1.69.0")
  message(STATUS "${Boost_ERROR_REASON}")
  message(STATUS "Attempting to build ${_Boost_download_version} as external project")

  if(${_Boost_download_version} VERSION_LESS ${BOOST_MIN_VERSION})
    message(FATAL_ERROR "Download version of Boost (${_Boost_download_version}) "
                        "is older than minimum allowed version (${BOOST_MIN_VERSION})")
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
  set(Boost_lib_names "")
  foreach(c ${_boost_components})
	# list(JOIN ...) is in cmake 3.12
    string(CONCAT Boost_lib_names "${Boost_lib_names}${c},")
  endforeach()

  include(ExternalProject)
  string(REPLACE "." "_" _Boost_download_filename ${_Boost_download_version})
  ExternalProject_Add(
    boost
    PREFIX ${CMAKE_BINARY_DIR}/boost
    URL http://downloads.sourceforge.net/project/boost/boost/${_Boost_download_version}/boost_${_Boost_download_filename}.zip
    URL_MD5 aec39b2e85552077e7f5c4e8cf9240cd
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ${BOOST_BOOTSTRAP} --prefix=${CMAKE_INSTALL_PREFIX} --with-libraries=${Boost_lib_names}
    BUILD_COMMAND ${BOOST_BUILD} ${BOOST_ARGS} install
    INSTALL_COMMAND ""
  )

  # Force the cache entries to be updated
  set(Boost_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include CACHE PATH "Boost include directory" FORCE)
  set(Boost_LIBRARY_DIRS ${CMAKE_INSTALL_PREFIX}/lib CACHE PATH "Boost library directory" FORCE)

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
      list(APPEND Boost_LIBRARIES "boost_${c}")
      
      # Also export cache variables for the file location of each library
      string(TOUPPER ${c} _basename)
      set(Boost_${_basename}_LIBRARY_RELEASE "${Boost_LIBRARY_DIRS}/libboost_${c}.so" CACHE FILEPATH "" FORCE)
      set(Boost_${_basename}_LIBRARY_DEBUG "${Boost_LIBRARY_DIRS}/libboost_${c}.so" CACHE FILEPATH "" FORCE)
    endforeach()
  endif()
  
  # Export Boost_THREAD_LIBRARY
  list(FIND _boost_components "thread" _building_threads)
  if(Boost_USE_MULTITHREADED AND ${_building_threads})
    # On Windows, always use the debug version
    # On Linux, we don't use tagged builds, so the debug/release filenames are the same
    set(Boost_THREAD_LIBRARY ${Boost_THREAD_LIBRARY_DEBUG})
  endif()
endif()

# Add the system thread library
if(Boost_USE_MULTITHREADED)
  list(APPEND Boost_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
endif()

link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

message(STATUS "Boost includes: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost library dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost thread library: ${Boost_THREAD_LIBRARY}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
