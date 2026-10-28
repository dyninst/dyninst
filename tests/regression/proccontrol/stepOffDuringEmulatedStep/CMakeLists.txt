include_guard(GLOBAL)

# Only Power and AArch64 emulate single steps through load-reserve/store-conditional sequences,
# and the test runs a native mutatee, so the code generator must target the host.  The test is
# registered everywhere and reports itself skipped elsewhere.
if(
  (DYNINST_HOST_ARCH_PPC64LE AND DYNINST_CODEGEN_ARCH_PPC64LE)
  OR (DYNINST_HOST_ARCH_AARCH64 AND DYNINST_CODEGEN_ARCH_AARCH64)
)
  set(_step_off_llsc_host 1)
else()
  set(_step_off_llsc_host 0)
endif()

add_executable(step_off_emulated_mutatee mutatee.c)

add_executable(step_off_during_emulated_step step_off.cpp)
target_compile_options(
  step_off_during_emulated_step
  PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS}
)
target_link_libraries(step_off_during_emulated_step PRIVATE pcontrol symtabAPI)
target_compile_definitions(
  step_off_during_emulated_step
  PRIVATE STEP_OFF_LLSC_HOST=${_step_off_llsc_host}
)

add_test(
  NAME proccontrol_step_off_during_emulated_step
  COMMAND step_off_during_emulated_step $<TARGET_FILE:step_off_emulated_mutatee>
)
set_tests_properties(
  proccontrol_step_off_during_emulated_step
  PROPERTIES
    LABELS "regression"
    TIMEOUT 120
    SKIP_RETURN_CODE 77
    SKIP_REGULAR_EXPRESSION "skipping:"
)
