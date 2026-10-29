include_guard(GLOBAL)

add_executable(liveness_amdgpu_m0_scc amdgpu_m0_scc.cpp)
target_compile_options(liveness_amdgpu_m0_scc PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(liveness_amdgpu_m0_scc PRIVATE parseAPI)

# Without the test binaries, the test runs without an argument and skips.
set(_binary)
if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)
  set(
    _binary
    "${DYNINST_TEST_BINARIES_DIR}/parseAPI/IndirectControlFlowAnalysis/parseAPI_IndirectControlFlowAnalysis_908.gpubin"
  )
endif()

add_test(NAME liveness_amdgpu_m0_scc COMMAND liveness_amdgpu_m0_scc ${_binary})
set_tests_properties(
  liveness_amdgpu_m0_scc
  PROPERTIES LABELS "regression" SKIP_RETURN_CODE 77 SKIP_REGULAR_EXPRESSION "skipping:"
)
