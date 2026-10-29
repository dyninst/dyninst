project(tests_dataflowAPI_rose_semantics LANGUAGES CXX)

dyninst_add_test(
  rose_semantics_aarch64_ldst
  TYPE unit
  SOURCE_FILES aarch64_ldst.cpp
  DEPS parseAPI
)

dyninst_add_test(
  rose_semantics_aarch64_ldst_pair
  TYPE unit
  SOURCE_FILES aarch64_ldst_pair.cpp
  DEPS parseAPI
)

dyninst_add_test(
  rose_semantics_aarch64_branch_shift
  TYPE unit
  SOURCE_FILES aarch64_branch_shift.cpp
  DEPS parseAPI
)
