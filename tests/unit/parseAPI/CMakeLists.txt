include_guard(GLOBAL)

dyninst_add_test(
  parseAPI_lea_nop_x86
  TYPE unit
  SOURCE_FILES lea-nop-x86.cpp
  DEPS parseAPI
)

dyninst_add_test(
  parseAPI_simplify_constants
  TYPE unit
  SOURCE_FILES simplify_constants.cpp
  DEPS parseAPI
)
