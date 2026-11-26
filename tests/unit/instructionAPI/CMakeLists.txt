include_guard(GLOBAL)

dyninst_add_test(
  instructionAPI_syscall_x86
  TYPE unit
  SOURCE_FILES syscall-x86.cpp
  DEPS instructionAPI
)

dyninst_add_test(
  instructionAPI_prefix_reset_x86
  TYPE unit
  SOURCE_FILES prefix-reset-x86.cpp
  DEPS instructionAPI
)

dyninst_add_test(
  instructionAPI_tzcnt_lzcnt_x86
  TYPE unit
  SOURCE_FILES tzcnt-lzcnt-x86.cpp
  DEPS instructionAPI
)

add_subdirectory(AST)
