include_guard(GLOBAL)

# Only Power and AArch64 emulate single steps through load-reserve/store-conditional sequences.
# The test drives the mutatee from the test binaries, proccontrol/stepOffDuringEmulatedStep/<host
# arch>.  A test_binaries checkout that predates it makes the test report itself skipped.
dyninst_add_test(
  proccontrol_step_off_during_emulated_step
  TYPE regression
  SOURCE_FILES step_off.cpp
  DEPS pcontrol symtabAPI
  ARGS
    "${DYNINST_TEST_BINARIES_DIR}/proccontrol/stepOffDuringEmulatedStep/${DYNINST_HOST_ARCH}"
  REQUIRES dynamic test_binaries host:aarch64,ppc64le
)
