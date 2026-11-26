include_guard(GLOBAL)

# Only a build with Zydis has the built-in unknown-instruction callback, and the
# fixture is x86-64 assembly.
if(NOT DYNINST_ENABLE_ZYDIS OR NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  return()
endif()

# Two instructions InstructionAPI cannot decode before a ret. The callback must
# let decoding and parsing continue past them.
add_library(zydis_unknown_insn SHARED unknown_insn.S)
set_target_properties(zydis_unknown_insn PROPERTIES LINKER_LANGUAGE C)
target_link_options(zydis_unknown_insn PRIVATE -nostdlib)
dyninst_add_test(
  parseAPI_ZydisUnknownInsn
  TYPE regression
  SOURCE_FILES zydisUnknownInsn.cpp
  DEPS parseAPI instructionAPI
  ARGS $<TARGET_FILE:zydis_unknown_insn>
)
