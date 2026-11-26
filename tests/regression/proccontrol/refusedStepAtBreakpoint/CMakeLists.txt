include_guard(GLOBAL)

# Only Power and AArch64 emulate single steps through load-reserve/store-conditional
# sequences, so only they can refuse one.  Other hosts, x86_64 included, always single-step
# in hardware, so the refusal this test sets up cannot happen there.  The test drives the
# mutatee from the test binaries, proccontrol/refusedStepAtBreakpoint/<host arch>.  A
# test_binaries checkout that predates it makes the test report itself skipped.
dyninst_add_test(
  proccontrol_refused_step_at_breakpoint
  TYPE regression
  SOURCE_FILES refused_step.cpp
  DEPS pcontrol symtabAPI
  ARGS
    "${DYNINST_TEST_BINARIES_DIR}/proccontrol/refusedStepAtBreakpoint/${DYNINST_HOST_ARCH}"
  REQUIRES dynamic test_binaries host:aarch64,ppc64le
)
