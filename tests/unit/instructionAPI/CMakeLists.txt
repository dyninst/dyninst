include_guard(GLOBAL)

dyninst_add_test(
  instructionAPI_syscall_x86
  TYPE unit
  SOURCE_FILES syscall-x86.cpp
  DEPS instructionAPI
)

add_subdirectory(AST)
