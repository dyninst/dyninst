#################################################################
# ThreadingBuildingBlocks.cmake
#
# Configure Intel's Threading Building Blocks for Dyninst
#
#################################################################

# TBB root directory
set(TBB_ROOT_DIR "/usr" CACHE PATH "TBB root directory")

# TBB include directory hint
set(TBB_INCLUDE_DIR "${TBB_ROOT_DIR}/include" CACHE PATH "TBB include directory")

# TBB library directory hint
set(TBB_LIBRARY_DIR "${TBB_ROOT_DIR}/lib" CACHE PATH "TBB library directory")

# FindTBB uses 'TBB_LIBRARY' instead of 'TBB_LIBRARY_DIR'.
set(TBB_LIBRARY ${TBB_LIBRARY_DIR})

# Use debug versions of TBB libraries
set(TBB_USE_DEBUG_BUILD OFF CACHE BOOL "Use debug versions of TBB libraries")

# Minimum version of TBB
set(TBB_MIN_VERSION 2018.0 CACHE STRING "Minimum version of TBB")

# The specific TBB libraries we need
# NB: This should _NOT_ be a cache variable
set(_tbb_components tbb tbbmalloc tbbmalloc_proxy)

find_package(TBB ${TBB_MIN_VERSION} COMPONENTS ${_tbb_components})

if(NOT TBB_FOUND)
  message(STATUS "Attempting to build TBB as external project")
  
  if(NOT UNIX)
    message(FATAL_ERROR "Building TBB from source is not supported on this platform")
  endif()
  
  set(TBB_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include CACHE PATH "TBB include directory" FORCE)
  set(TBB_LIBRARY_DIRS ${CMAKE_INSTALL_PREFIX}/lib CACHE PATH "TBB library directory" FORCE)

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
  	list(APPEND _tbb_libraries ${TBB_LIBRARY_DIRS}/lib${c}.so)
  endforeach()
  
  set(TBB_LIBRARIES ${_tbb_libraries} CACHE FILEPATH "" FORCE)
    
  set(TBB_ROOT_DIR ${CMAKE_INSTALL_PREFIX} CACHE PATH "TBB root directory" FORCE)

  include(ExternalProject)
  ExternalProject_Add(
    TBB
    PREFIX ${TBB_ROOT_DIR}
    URL https://github.com/01org/tbb/archive/2019_U5.tar.gz
    URL_MD5 38eae1abb55e1663257f29e8748d3798
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND
      $(MAKE) -C src
      ${_tbb_components_cfg}
      tbb_build_dir=${TBB_ROOT_DIR}/src
      tbb_build_prefix=tbb
    INSTALL_COMMAND
      ${CMAKE_COMMAND}
      	-DLIBDIR=${TBB_LIBRARY_DIRS}
      	-DINCDIR=${TBB_INCLUDE_DIRS}
        -DROOTDIR=${TBB_ROOT_DIR}
      	-P ${CMAKE_CURRENT_LIST_DIR}/ThreadingBuildingBlocks.install.cmake
  )
endif()

include_directories(${TBB_INCLUDE_DIRS})
link_directories(${TBB_LIBRARY_DIRS})

message(STATUS "TBB include directory: ${TBB_INCLUDE_DIRS}")
message(STATUS "TBB library directory: ${TBB_LIBRARY_DIRS}")
message(STATUS "TBB libraries: ${TBB_LIBRARIES}")

