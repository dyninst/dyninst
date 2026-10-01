include_guard(GLOBAL)

dyninst_add_test(
  common_registers_uniqueness
  TYPE unit
  SOURCE_FILES uniqueness.cpp
  DEPS symtabAPI
)

dyninst_add_test(
  common_registers_liveness_set
  TYPE unit
  SOURCE_FILES liveness_set.cpp
  DEPS common
)
