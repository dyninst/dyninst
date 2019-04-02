###############################
#   Boost.cmake
#
# Configure Boost for Dyninst
#
###############################

if(NOT DEFINED CMAKE_THREAD_LIBS_INIT)
  message(FATAL_ERROR "Threads library not initialized before resolving Boost\n"
                      "Use 'find_package(Threads)' before including Boost")
endif()

# Disable generating serialization code in boost::multi_index
add_definitions(-DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION)

# Set to ON to enable debug output from FindBoost.
set(Boost_DEBUG OFF)

# Use the multithreaded version of Boost
set(Boost_USE_MULTITHREADED ON)

# Don't use libraries linked statically to the C++ runtime
set(Boost_USE_STATIC_RUNTIME OFF)

# Disable auto-linking
add_definitions(-DBOOST_ALL_NO_LIB=1)

# There are broken versions of MSVC that won't handle variadic templates
# correctly (despite the C++11 test case passing). Just build vanilla versions,
# boost can handle it.
if(MSVC)
  add_definitions(-DBOOST_NO_CXX11_VARIADIC_TEMPLATES)
endif()

# Need Boost >= 1.61 for filesytem components
set(BOOST_MIN_VERSION 1.61.0 CACHE STRING "Minimum Boost version")

set(Boost_ADDITIONAL_VERSIONS
    "1.69" "1.69.0"
    "1.68" "1.68.0"
    "1.67" "1.67.0"
    "1.66" "1.66.0"
    "1.65.1"
    "1.65" "1.65.0"
    "1.64" "1.64.0"
    "1.63" "1.63.0"
    "1.62" "1.62.0"
    "1.61" "1.61.0"
)

if(NOT ("${Boost_NO_BOOST_CMAKE}" STREQUAL "OFF"))
  message(STATUS "Disabling Boost's own CMake--known buggy in many cases")
  set(Boost_NO_BOOST_CMAKE ON)
endif()

# Set the default location to look for Boost
#
# NOTE: If you change this, also change the value in
#       the test for setting Boost_NO_SYSTEM_PATHS
#
set(PATH_BOOST "/usr" CACHE STRING "Path to Boost")

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
set(Boost_COMPONENTS atomic date_time filesystem system thread timer)

find_package(Boost ${BOOST_MIN_VERSION} COMPONENTS ${Boost_COMPONENTS})

# If we didn't find a suitable version on the system, then download one from the web
if(NOT Boost_FOUND)
  set(_Boost_download_version "1.69.0")
  message(STATUS "${Boost_ERROR_REASON}")
  message(STATUS "Attempting to build ${_Boost_download_version} as external project")

  if(${_Boost_download_version} VERSION_LESS ${BOOST_MIN_VERSION})
    message(FATAL_ERROR "Download version of Boost (${_Boost_download_version}) "
                        "is older than minimum allowed version (${BOOST_MIN_VERSION})")
  endif()
  
  set(BOOST_ARGS
      --ignore-site-config
      --link=static
      --runtime-link=shared
      --threading=multi)
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
  foreach(c ${Boost_COMPONENTS})
	# list(JOIN ...) is in cmake 3.12
    string(CONCAT Boost_lib_names "${Boost_lib_names}${c},")
  endforeach()

  include(ExternalProject)
  string(REPLACE "." "_" _Boost_download_filename ${_Boost_download_version})
  externalproject_add(
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
  set(Boost_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include CACHE STRING "Boost include directory" FORCE)
  set(Boost_LIBRARY_DIRS ${CMAKE_INSTALL_PREFIX}/lib CACHE STRING "Boost library directory" FORCE)

  if(MSVC)
    # We need to specify different library names for debug vs release
    set(Boost_LIBRARIES "")
    foreach(c ${Boost_COMPONENTS})
      list(APPEND Boost_LIBRARIES "optimized libboost_${c} debug libboost_${c}-gd ")
    endforeach()
  else()
    # Transform the component names into the library filenames
    # e.g., system -> boost_system
    set(Boost_LIBRARIES "")
    foreach(c ${Boost_COMPONENTS})
      list(APPEND Boost_LIBRARIES "boost_${c}")
    endforeach()
  endif()
endif()

# Add the system thread library
list(APPEND Boost_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

message(STATUS "Boost includes: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost library dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost thread library: ${Boost_THREAD_LIBRARY}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
