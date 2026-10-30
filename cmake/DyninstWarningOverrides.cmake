# Dyninst's compiler warning flags: the shared DyninstWarnings module, in
# cmake/common/warnings, and the settings only Dyninst's own sources need

# If not building with OpenMP or if static libs are enabled, ignore OpenMP pragma
# warnings
if(NOT USE_OpenMP OR ENABLE_STATIC_LIBS)
  list(APPEND REQUESTED_WARNING_FLAGS "Wno-unknown-pragmas")
endif()

include(DyninstWarnings)

# If -Wframe-larger-than is available, allow larger frames in the riscv decoder tests
if(HAS_CPP_FLAG_Wframe_larger_than AND NOT DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS)
  set(maxFrameSizeOverrideRiscvInstructionApiTests 49152)
endif()

# clang >17.0 doesn't detect /usr/include/boost as a system directory
# and warns about use of deprecated STL functions in Boost
if(NOT DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS)
  set(_id CMAKE_CXX_COMPILER_ID)
  set(_ver CMAKE_CXX_COMPILER_VERSION)
  if("${_id}" STREQUAL "Clang" AND "${_ver}" VERSION_GREATER 17)
    set(clangBoostDeprecatedWarning -Wno-deprecated-declarations)
  endif()
endif()
