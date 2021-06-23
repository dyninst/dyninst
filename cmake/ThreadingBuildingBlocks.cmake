#=====================================================================================
# ThreadingBuildingBlocks.cmake
#
# Configure Intel's Threading Building Blocks for Dyninst
#
#   ----------------------------------------
#
# Accepts the following CMake variables
#
# TBB_ROOT_DIR        - Hint directory that contains the TBB installation
# TBB_INCLUDEDIR      - Hint directory that contains the TBB headers files
# TBB_LIBRARYDIR      - Hint directory that contains the TBB library files
# TBB_LIBRARY         - Alias for TBB_LIBRARYDIR
# TBB_USE_DEBUG_BUILD - Use debug version of tbb libraries, if present
# TBB_MIN_VERSION     - Minimum acceptable version of TBB
#
# Directly exports the following CMake variables
#
# TBB_ROOT_DIR        - Computed base directory of TBB installation
# TBB_INCLUDE_DIRS    - TBB include directory
# TBB_INCLUDE_DIR     - Alias for TBB_INCLUDE_DIRS
# TBB_LIBRARY_DIRS    - TBB library directory
# TBB_LIBRARY_DIR     - Alias for TBB_LIBRARY_DIRS
# TBB_DEFINITIONS     - TBB compiler definitions
# TBB_LIBRARIES       - TBB library files
#
# TBB_<c>_LIBRARY_RELEASE - Path to the release version of component <c>
# TBB_<c>_LIBRARY_DEBUG   - Path to the debug version of component <c>
#
# NOTE:
# The exported TBB_ROOT_DIR can be different from the value provided by the user
# in the case that it is determined to build TBB from source. In such a case,
# TBB_ROOT_DIR will contain the directory of the from-source installation.
#
#
# See Modules/FindTBB.cmake for additional input and exported variables
#
#=====================================================================================

if(TBB_FOUND)
  return()
endif()

# -------------- RUNTIME CONFIGURATION ----------------------------------------

# Use debug versions of TBB libraries
set(TBB_USE_DEBUG_BUILD OFF CACHE BOOL "Use debug versions of TBB libraries")

# Minimum version of TBB (assumes a dotted-decimal format: YYYY.XX)
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set(_tbb_min_version 2019.7)
else()
  set(_tbb_min_version 2018.6)
endif()

set(TBB_MIN_VERSION ${_tbb_min_version} CACHE STRING "Minimum version of TBB (assumes a dotted-decimal format: YYYY.XX)")

if(${TBB_MIN_VERSION} VERSION_LESS ${_tbb_min_version})
  message(
    FATAL_ERROR
    "Requested TBB version ${TBB_MIN_VERSION} is less than minimum supported version ${_tbb_min_version}"
  )
endif()

# -------------- PATHS --------------------------------------------------------

# TBB root directory
set(TBB_ROOT_DIR "/usr" CACHE PATH "TBB root directory")

# TBB include directory hint
set(TBB_INCLUDEDIR "${TBB_ROOT_DIR}/include" CACHE PATH "TBB include directory")

# TBB library directory hint
set(TBB_LIBRARYDIR "${TBB_ROOT_DIR}/lib" CACHE PATH "TBB library directory")

# Translate to FindTBB names
set(TBB_LIBRARY ${TBB_LIBRARYDIR})
set(TBB_INCLUDE_DIR ${TBB_INCLUDEDIR})

# The specific TBB libraries we need
# NB: This should _NOT_ be a cache variable
set(_tbb_components tbb tbbmalloc tbbmalloc_proxy)

find_package(TBB ${TBB_MIN_VERSION} COMPONENTS ${_tbb_components})

# -------------- SOURCE BUILD -------------------------------------------------
if(TBB_FOUND)
  # Force the cache entries to be updated
  # Normally, these would not be exported. However, we need them in the Testsuite
  set(TBB_INCLUDE_DIRS ${TBB_INCLUDE_DIRS} CACHE PATH "TBB include directory" FORCE)
  set(TBB_LIBRARY_DIRS ${TBB_LIBRARY_DIRS} CACHE PATH "TBB library directory" FORCE)
  set(TBB_DEFINITIONS ${TBB_DEFINITIONS} CACHE STRING "TBB compiler definitions" FORCE)
  set(TBB_LIBRARIES ${TBB_LIBRARIES} CACHE FILEPATH "TBB library files" FORCE)
  
  if(NOT TARGET TBB)
    add_library(TBB SHARED IMPORTED)
  endif()
elseif(NOT TBB_FOUND AND STERILE_BUILD)
  message(FATAL_ERROR "TBB not found and cannot be downloaded because build is sterile.")
else()
  # If we didn't find a suitable version on the system, then download one from the web
  message(STATUS "${ThreadingBuildingBlocks_ERROR_REASON}")
  message(STATUS "Attempting to build TBB(${TBB_MIN_VERSION}) as external project")
  
  if(NOT UNIX)
    message(FATAL_ERROR "Building TBB from source is not supported on this platform")
  endif()

  # Forcibly update the cache variables
  set(TBB_ROOT_DIR ${CMAKE_INSTALL_PREFIX} CACHE PATH "TBB root directory" FORCE)
  set(TBB_INCLUDE_DIRS ${TBB_ROOT_DIR}/include CACHE PATH "TBB include directory" FORCE)
  set(TBB_LIBRARY_DIRS ${TBB_ROOT_DIR}/lib CACHE PATH "TBB library directory" FORCE)
  set(TBB_DEFINITIONS "" CACHE STRING "TBB compiler definitions" FORCE)

  set(_tbb_libraries)
  set(_tbb_components_cfg)

  foreach(c ${_tbb_components})
    # Generate make target names
  	if(${c} STREQUAL tbbmalloc_proxy)
  	  # tbbmalloc_proxy is spelled tbbproxy in their Makefiles
  	  list(APPEND _tbb_components_cfg tbbproxy_release)
  	else()
  	  list(APPEND _tbb_components_cfg ${c}_release)
  	endif()
  	
  	# Generate library filenames
  	list(APPEND _tbb_libraries "${TBB_LIBRARY_DIRS}/lib${c}.so")
  	
    foreach(t RELEASE DEBUG)
      set(TBB_${c}_LIBRARY_${t} "${TBB_LIBRARY_DIRS}/lib${c}.so" CACHE FILEPATH "" FORCE)
    endforeach()
  endforeach()
  
  set(TBB_LIBRARIES ${_tbb_libraries} CACHE FILEPATH "TBB library files" FORCE)
  
  # Split the dotted decimal version into major/minor parts
  string(REGEX REPLACE "\\." ";" _tbb_download_name ${TBB_MIN_VERSION})
  list(GET _tbb_download_name 0 _tbb_ver_major)
  list(GET _tbb_download_name 1 _tbb_ver_minor)
  
  include(ExternalProject)
  set(_tbb_prefix_dir ${CMAKE_BINARY_DIR}/tbb)

  # Set the compiler for TBB
  # It assumes gcc and tests for Intel, so clang is the only
  # one that needs special treatment.
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  	set(_tbb_compiler "compiler=clang")
  endif()
  
  ExternalProject_Add(
    TBB
    PREFIX ${_tbb_prefix_dir}
    URL https://github.com/01org/tbb/archive/${_tbb_ver_major}_U${_tbb_ver_minor}.tar.gz
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND
      CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
      $(MAKE) -C src
      ${_tbb_components_cfg}
      tbb_build_dir=${_tbb_prefix_dir}/src
      tbb_build_prefix=tbb
      ${_tbb_compiler}
    INSTALL_COMMAND
      ${CMAKE_COMMAND}
      	-DLIBDIR=${TBB_LIBRARY_DIRS}
      	-DINCDIR=${TBB_INCLUDE_DIRS}
        -DPREFIX=${_tbb_prefix_dir}
      	-P ${CMAKE_CURRENT_LIST_DIR}/ThreadingBuildingBlocks.install.cmake
  )
endif()
 
include_directories(SYSTEM ${TBB_INCLUDE_DIRS})
link_directories(${TBB_LIBRARY_DIRS})

message(STATUS "TBB include directory: ${TBB_INCLUDE_DIRS}")
message(STATUS "TBB library directory: ${TBB_LIBRARY_DIRS}")
message(STATUS "TBB libraries: ${TBB_LIBRARIES}")
message(STATUS "TBB definitions: ${TBB_DEFINITIONS}")
