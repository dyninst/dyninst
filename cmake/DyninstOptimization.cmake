#[=======================================================================[
DyninstOptimization
-------------------

This module provides the global compiler and linker flags.

#]=======================================================================]
include_guard(GLOBAL)

if(ENABLE_LTO)
  include(CheckIPOSupported)
  check_ipo_supported(LANGUAGES "C" "CXX")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

# Make sure we don't get something like CC=gcc CXX=clang++
if(NOT ${CMAKE_C_COMPILER_ID} STREQUAL ${CMAKE_CXX_COMPILER_ID})
  message(FATAL_ERROR "C and C++ compilers are not the same vendor")
endif()

set(_linux_compilers "GNU" "Clang" "Intel" "IntelLLVM")

if(${CMAKE_CXX_COMPILER_ID} IN_LIST _linux_compilers)
  if(DYNINST_LINKER)
    list(APPEND DYNINST_LINK_FLAGS "-fuse-ld=${DYNINST_LINKER}")
  endif()

  if(ENABLE_LTO)
    if(${DYNINST_LINKER} MATCHES "gold")
      message(FATAL_ERROR "Cannot use the gold linker for LTO")
    endif()
  endif()

  # Used in stackwalk
  set(DYNINST_FORCE_FRAME_POINTER "-fno-omit-frame-pointer")

  set(_DEBUG "-Og -g3 ${DYNINST_FORCE_FRAME_POINTER}")
  set(_RELEASE "-O3 -g3")
  set(_RELWITHDEBINFO "-O2 -g3")
  set(_MINSIZEREL "-Os")

  # Ensure each library is fully linked
  list(APPEND DYNINST_LINK_FLAGS "-Wl,--no-undefined")
elseif(MSVC)
  set(DYNINST_FORCE_FRAME_POINTER "/Oy-")

  set(_DEBUG "/MP /Od /Zi /MDd /D_DEBUG ${DYNINST_FORCE_FRAME_POINTER}")
  set(_RELEASE "/MP /O3 /MD")
  set(_RELWITHDEBINFO "/MP /O2 /Zi /MD")
  set(_MINSIZEREL "/MP /O1 /MD")
else()
  message(FATAL_ERROR "Unknown compiler '${CMAKE_CXX_COMPILER_ID}'")
endif()

# Set the relevant CMake globals
foreach(bt "DEBUG" "RELWITHDEBINFO" "RELEASE" "MINSIZEREL")
  set(CMAKE_C_FLAGS_${bt} ${_${bt}})
  set(CMAKE_CXX_FLAGS_${bt} ${_${bt}})
  unset(_${bt})
endforeach()
