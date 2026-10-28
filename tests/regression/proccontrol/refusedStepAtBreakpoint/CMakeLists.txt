include_guard(GLOBAL)

# Only Power and AArch64 emulate single steps through load-reserve/store-conditional
# sequences, so only they can refuse one, and the test runs a native mutatee, so the code
# generator must target the host.  The test is registered everywhere and reports itself
# skipped elsewhere.
if(
  (DYNINST_HOST_ARCH_PPC64LE AND DYNINST_CODEGEN_ARCH_PPC64LE)
  OR (DYNINST_HOST_ARCH_AARCH64 AND DYNINST_CODEGEN_ARCH_AARCH64)
)
  set(_refused_step_llsc_host 1)
else()
  set(_refused_step_llsc_host 0)
endif()

add_executable(refused_step_mutatee mutatee.c)

add_executable(refused_step_at_breakpoint refused_step.cpp)
target_compile_options(refused_step_at_breakpoint PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(refused_step_at_breakpoint PRIVATE pcontrol symtabAPI)
target_compile_definitions(
  refused_step_at_breakpoint
  PRIVATE REFUSED_STEP_LLSC_HOST=${_refused_step_llsc_host}
)

# Drives a live process: DYNINST_ENABLE_DYNAMIC_TESTS=OFF turns it into a skip.  The option is
# ON when not defined.
if(DEFINED DYNINST_ENABLE_DYNAMIC_TESTS AND NOT DYNINST_ENABLE_DYNAMIC_TESTS)
  target_compile_definitions(
    refused_step_at_breakpoint
    PRIVATE DYNINST_ENABLE_DYNAMIC_TESTS=0
  )
else()
  target_compile_definitions(
    refused_step_at_breakpoint
    PRIVATE DYNINST_ENABLE_DYNAMIC_TESTS=1
  )
endif()

add_test(
  NAME proccontrol_refused_step_at_breakpoint
  COMMAND refused_step_at_breakpoint $<TARGET_FILE:refused_step_mutatee>
)
set_tests_properties(
  proccontrol_refused_step_at_breakpoint
  PROPERTIES LABELS "regression" SKIP_RETURN_CODE 77 SKIP_REGULAR_EXPRESSION "skipping:"
)
