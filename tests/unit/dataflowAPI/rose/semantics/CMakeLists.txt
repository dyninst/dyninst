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

dyninst_add_test(
  rose_semantics_riscv64_store
  TYPE unit
  SOURCE_FILES riscv64_store.cpp
  DEPS parseAPI
  REQUIRES capstone
)

dyninst_add_test(
  rose_semantics_riscv64
  TYPE unit
  SOURCE_FILES riscv64_semantics.cpp
  DEPS parseAPI
  REQUIRES capstone
)

dyninst_add_test(
  rose_semantics_amdgpu_scalar_add
  TYPE unit
  SOURCE_FILES amdgpu_scalar_add.cpp
  DEPS parseAPI
)
