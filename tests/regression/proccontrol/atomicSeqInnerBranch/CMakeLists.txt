include_guard(GLOBAL)

# Only Power and AArch64 emulate single steps through load-reserve/store-conditional
# sequences. ProcControl runs the mutatee without instrumenting it, so the test does not
# need native code generation.
dyninst_add_test(
  proccontrol_atomic_seq_inner_branch
  TYPE regression
  SOURCE_FILES inner_branch.cpp
  DEPS pcontrol symtabAPI
  ARGS
    "${DYNINST_TEST_BINARIES_DIR}/proccontrol/atomicSeqInnerBranch/${DYNINST_HOST_ARCH}"
  REQUIRES dynamic host:aarch64,ppc64le test_binaries
)
