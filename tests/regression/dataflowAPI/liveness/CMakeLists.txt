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

# Branches into a function entry from x86_64 code, assembled for the test. The library is
# parsed, never run; the liveness ABI comes from the codegen architecture.
dyninst_test_skip_reason(_no_tailcall_fixture REQUIRES host:x86_64 codegen:x86_64)
if(NOT _no_tailcall_fixture)
  add_library(liveness_tailcall_fixture SHARED tailcall_fixture.S)
  set_target_properties(liveness_tailcall_fixture PROPERTIES LINKER_LANGUAGE C)
  target_link_options(liveness_tailcall_fixture PRIVATE -nostdlib)
endif()
dyninst_add_test(
  liveness_x86_64_tailcall
  TYPE regression
  SOURCE_FILES tailcall.cpp
  DEPS parseAPI
  ARGS $<TARGET_FILE:liveness_tailcall_fixture>
  REQUIRES host:x86_64 codegen:x86_64
)
