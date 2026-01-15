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

if(DYNINST_ENABLE_LTO)
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
    list(APPEND DYNINST_LINK_FLAGS -fuse-ld=${DYNINST_LINKER})
  endif()

  if(DYNINST_ENABLE_LTO)
    if(${DYNINST_LINKER} MATCHES "gold")
      message(FATAL_ERROR "Cannot use the gold linker for LTO")
    endif()
  endif()

  # Used in stackwalk
  set(DYNINST_FORCE_FRAME_POINTER -fno-omit-frame-pointer)

  # Dyninst relies on `assert` for correctness. Never let CMake disable it
  set(DYNINST_FLAGS_DEBUG -Og -g3 ${DYNINST_FORCE_FRAME_POINTER} -UNDEBUG)
  set(DYNINST_FLAGS_RELEASE -O3 -UNDEBUG)
  set(DYNINST_FLAGS_RELWITHDEBINFO ${DYNINST_FLAGS_RELEASE} -g3)
  set(DYNINST_FLAGS_MINSIZEREL -Os -UNDEBUG)

  # Ensure each library is fully linked
  list(APPEND DYNINST_LINK_FLAGS -Wl,--no-undefined)

  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    if(DYNINST_CXXSTDLIB)
      list(APPEND DYNINST_CXX_FLAGS -stdlib=${DYNINST_CXXSTDLIB})
      list(APPEND DYNINST_CXX_LINK_FLAGS -stdlib=${DYNINST_CXXSTDLIB})
    endif()
  endif()

  if(DYNINST_FORCE_RUNPATH)
    list(APPEND DYNINST_LINK_FLAGS "-Wl,--enable-new-dtags")
  endif()

  if(DYNINST_WARNINGS_AS_ERRORS)
    list(APPEND DYNINST_LINK_FLAGS "-Wl,--fatal-warnings")
  endif()
elseif(MSVC)
  set(DYNINST_FORCE_FRAME_POINTER /Oy-)

  set(DYNINST_FLAGS_DEBUG /MP /Od /Zi /MDd /D_DEBUG ${DYNINST_FORCE_FRAME_POINTER})
  set(DYNINST_FLAGS_RELEASE /MP /O3 /MD /D_DEBUG)
  set(DYNINST_FLAGS_RELWITHDEBINFO ${DYNINST_FLAGS_RELEASE} /Zi)
  set(DYNINST_FLAGS_MINSIZEREL /MP /O1 /MD /D_DEBUG)
else()
  message(FATAL_ERROR "Unknown compiler '${CMAKE_CXX_COMPILER_ID}'")
endif()

# By default, CMake effectively passes compiler flags in the order
#
#   ${CMAKE_<LANG>_FLAGS} ${CMAKE_<LANG>_FLAGS_<BUILD>} <options>
#
# where `<options>` are the values passed to `target_compile_options`.
# This prevents users from overriding values manually computed by us. To
# work around this, we rearrange the values such that CMake now
# effectively (redundantly) does
#
#   ${CMAKE_<LANG>_FLAGS} ${CMAKE_<LANG>_FLAGS_<BUILD>} <options> ${CMAKE_<LANG>_FLAGS}
#
string(TOUPPER ${CMAKE_BUILD_TYPE} _build_type)
string(REPLACE " " ";" _c_flags "${CMAKE_C_FLAGS}")
string(REPLACE " " ";" _cxx_flags "${CMAKE_CXX_FLAGS}")

set(DYNINST_C_FLAGS_${_build_type} ${DYNINST_FLAGS_${_build_type}} ${_c_flags})
set(DYNINST_CXX_FLAGS_${_build_type} ${DYNINST_FLAGS_${_build_type}} ${DYNINST_CXX_FLAGS}
                                     ${_cxx_flags})

# Merge the link flags for C++
list(APPEND DYNINST_CXX_LINK_FLAGS ${DYNINST_LINK_FLAGS})

message(STATUS "DYNINST_LINK_FLAGS: ${DYNINST_LINK_FLAGS}")
message(STATUS "DYNINST_CXX_LINK_FLAGS: ${DYNINST_CXX_LINK_FLAGS}")
message(STATUS "DYNINST_C_FLAGS_${_build_type}: ${DYNINST_C_FLAGS_${_build_type}}")
message(STATUS "DYNINST_CXX_FLAGS_${_build_type}: ${DYNINST_CXX_FLAGS_${_build_type}}")

unset(_build_type)
