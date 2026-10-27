include_guard(GLOBAL)

if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)
  add_executable(liveness_amdgpu_m0_scc amdgpu_m0_scc.cpp)
  target_compile_options(liveness_amdgpu_m0_scc PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
  target_link_libraries(liveness_amdgpu_m0_scc PRIVATE parseAPI)
  add_test(
    NAME liveness_amdgpu_m0_scc
    COMMAND
      liveness_amdgpu_m0_scc
      "${DYNINST_TEST_BINARIES_DIR}/parseAPI/IndirectControlFlowAnalysis/parseAPI_IndirectControlFlowAnalysis_908.gpubin"
  )
  set_tests_properties(liveness_amdgpu_m0_scc PROPERTIES LABELS "regression")
endif()
