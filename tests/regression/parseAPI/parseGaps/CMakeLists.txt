include_guard(GLOBAL)

dyninst_add_test(
  parseAPI_parseGaps_test
  TYPE regression
  SOURCE_FILES parseGaps.cpp
  DEPS parseAPI
  ARGS "${DYNINST_TEST_BINARIES_DIR}/parseAPI/parseGaps/aarch64_el0_stripped"
  REQUIRES test_binaries
)
