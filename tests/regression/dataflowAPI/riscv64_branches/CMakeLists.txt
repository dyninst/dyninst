include_guard(GLOBAL)

set(_dir "${DYNINST_TEST_BINARIES_DIR}/dyninstAPI/findMain/riscv64")

dyninst_add_test(
  dataflowAPI_riscv64_branches
  TYPE regression
  SOURCE_FILES riscv64_branches.cpp
  DEPS parseAPI
  ARGS "${_dir}/dyn.nopie.nostripped" "${_dir}/dyn.pie.nostripped"
  REQUIRES capstone test_binaries
)

unset(_dir)
