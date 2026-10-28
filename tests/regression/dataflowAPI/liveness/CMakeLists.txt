include_guard(GLOBAL)

dyninst_add_test(
  liveness_amdgpu_m0_scc
  TYPE regression
  SOURCE_FILES amdgpu_m0_scc.cpp
  DEPS parseAPI
  ARGS
    "${DYNINST_TEST_BINARIES_DIR}/parseAPI/IndirectControlFlowAnalysis/parseAPI_IndirectControlFlowAnalysis_908.gpubin"
  REQUIRES test_binaries
)
