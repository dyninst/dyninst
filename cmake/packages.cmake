if (PLATFORM MATCHES "bgq")
  # Not a find per se, just a magic include line
  set (PATH_BGQ "/bgsys/drivers/ppcfloor" CACHE STRING "Path to BG/Q include files")
  if (NOT (PATH_BGQ STREQUAL ""))
    include_directories (${PATH_BGQ})
  endif()
endif()


include(${DYNINST_ROOT}/cmake/CheckCXX11Features.cmake)
if(NOT HAS_CXX11_AUTO)
  message(FATAL_ERROR "No support for C++11 auto found. Dyninst requires this compiler feature.")
else()
  message(STATUS "C++11 support found, required flags are: ${CXX11_COMPILER_FLAGS}")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_COMPILER_FLAGS}")
