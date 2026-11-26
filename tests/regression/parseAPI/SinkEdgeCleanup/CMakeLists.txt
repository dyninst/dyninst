include_guard(GLOBAL)

# jt_sink.so: finalize_jump_tables deletes an overrun jump-table target whose
# block ends with a sink edge. Every edge in the sink's sources must still be
# in its source block's targets.
dyninst_add_test(
  parseAPI_SinkEdgeCleanup
  TYPE regression
  SOURCE_FILES sinkSources.cpp
  DEPS parseAPI
  ARGS "${DYNINST_TEST_BINARIES_DIR}/parseAPI/SinkEdgeCleanup/jt_sink.so"
  REQUIRES test_binaries
)
