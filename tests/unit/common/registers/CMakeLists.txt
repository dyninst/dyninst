include_guard(GLOBAL)

dyninst_add_test(
  common_registers_uniqueness
  TYPE unit
  SOURCE_FILES uniqueness.cpp
  DEPS symtabAPI
)
