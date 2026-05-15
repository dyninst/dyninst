include_guard(GLOBAL)

list(FIND DYNINST_PLATFORM_CAPABILITIES "-Dcap_stack_mods" cap_stack_mods_found)
if(cap_stack_mods_found EQUAL -1)
  add_library(dyninstAPI_stackmod INTERFACE)
  return()
endif()

if(POLICY CMP0076)
  # target_sources() converts relative paths to absolute
  cmake_policy(SET CMP0076 NEW)
endif()

set(_headers OffsetVector.h StackAccess.h StackLocation.h StackModChecker.h
             StackModExpr.h TMap.h)

set(_sources
    OffsetVector.C
    StackAccess.C
    StackLocation.C
    StackMod.C
    StackModExpr.C
    StackModChecker.C
    TMap.C)

add_library(dyninstAPI_stackmod INTERFACE)
target_sources(dyninstAPI_stackmod INTERFACE ${_headers} ${_sources})
target_include_directories(dyninstAPI_stackmod INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

unset(_headers)
unset(_sources)
