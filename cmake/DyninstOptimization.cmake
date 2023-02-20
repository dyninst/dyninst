#[=======================================================================[
DyninstOptimization
-------------------

This module provides the global compiler and linker flags.

  Created variables:

  DYNINST_LINK_FLAGS
  	Generic linker flags that apply to all languages

  DYNINST_CXX_LINK_FLAGS
  	Linker flags that are specific to the C++ compiler

	DYNINST_FORCE_FRAME_POINTER
		Contains the compiler-specific flags needed to force the generation
		of a frame pointer in code compiled into a Dyninst library. Currently,
		this is only used in some portions of stackwalk.

  ---

  The global CMAKE_<LANG>_FLAGS_<BUILD_TYPE> variables are also
  populated. Values specified by the user in CMAKE_<LANG>_FLAGS
  are forcibly passed to the compiler after CMAKE_<LANG>_FLAGS_<BUILD_TYPE>
  so that values computed here can be overridden. By default, CMake does
  the opposite.

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

  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    if(DYNINST_CXXSTDLIB)
      list(APPEND DYNINST_CXX_FLAGS "-stdlib=${DYNINST_CXXSTDLIB}")
      list(APPEND DYNINST_CXX_LINK_FLAGS "-stdlib=${DYNINST_CXXSTDLIB}")
    endif()
  endif()
elseif(MSVC)
  set(DYNINST_FORCE_FRAME_POINTER "/Oy-")

  set(_DEBUG "/MP /Od /Zi /MDd /D_DEBUG ${DYNINST_FORCE_FRAME_POINTER}")
  set(_RELEASE "/MP /O3 /MD")
  set(_RELWITHDEBINFO "/MP /O2 /Zi /MD")
  set(_MINSIZEREL "/MP /O1 /MD")
else()
  message(FATAL_ERROR "Unknown compiler '${CMAKE_CXX_COMPILER_ID}'")
endif()

# Set the CMake globals, preserving user-specified values in CMAKE_<LANG>_FLAGS
string(TOUPPER ${CMAKE_BUILD_TYPE} _build_type)
foreach(l "C" "CXX")
  if(CMAKE_${l}_FLAGS)
    set(_p ${CMAKE_${l}_FLAGS})
  endif()
  set(CMAKE_${l}_FLAGS_${_build_type} "${_${_build_type}} ${DYNINST_${l}_FLAGS} ${_p}")
  set(CMAKE_${l}_FLAGS
      ""
      CACHE STRING "" FORCE)
  unset(_p)
endforeach()
unset(_build_type)

# Merge the link flags for C++
list(APPEND DYNINST_CXX_LINK_FLAGS ${DYNINST_LINK_FLAGS})
