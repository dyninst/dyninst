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

# Drives a live process: DYNINST_ENABLE_DYNAMIC_TESTS=OFF turns it into a skip.  The option is
# ON when not defined.
if(DEFINED DYNINST_ENABLE_DYNAMIC_TESTS AND NOT DYNINST_ENABLE_DYNAMIC_TESTS)
  target_compile_definitions(
    step_off_during_emulated_step
    PRIVATE DYNINST_ENABLE_DYNAMIC_TESTS=0
  )
else()
  target_compile_definitions(
    step_off_during_emulated_step
    PRIVATE DYNINST_ENABLE_DYNAMIC_TESTS=1
  )
endif()

# The mutatee is in the test binaries, as proccontrol/stepOffDuringEmulatedStep/<arch>.
# Without them, or on a host with no mutatee, no path is passed and the test reports a skip.
set(_mutatee)
if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS AND _step_off_llsc_host)
  if(DYNINST_HOST_ARCH_AARCH64)
    set(
      _mutatee
      "${DYNINST_TEST_BINARIES_DIR}/proccontrol/stepOffDuringEmulatedStep/aarch64"
    )
  else()
    set(
      _mutatee
      "${DYNINST_TEST_BINARIES_DIR}/proccontrol/stepOffDuringEmulatedStep/ppc64le"
    )
  endif()
endif()

add_test(
  NAME proccontrol_step_off_during_emulated_step
  COMMAND step_off_during_emulated_step ${_mutatee}
)
set_tests_properties(
  proccontrol_step_off_during_emulated_step
  PROPERTIES LABELS "regression" SKIP_RETURN_CODE 77 SKIP_REGULAR_EXPRESSION "skipping:"
)
