include_guard(GLOBAL)

# jt_reread.so: fb's jump is read first without a bound, past its table into
# fc's, and later again with the bound. Every indirect edge of a jump inside fb
# must stay inside fb. The frame order this needs holds in a serial parse.
dyninst_add_test(
  parseAPI_JumpTableReread
  TYPE regression
  SOURCE_FILES jtReread.cpp
  DEPS parseAPI
  ARGS "${DYNINST_TEST_BINARIES_DIR}/parseAPI/JumpTableReread/jt_reread.so"
  REQUIRES test_binaries
  PROPERTIES ENVIRONMENT "OMP_NUM_THREADS=1"
)
