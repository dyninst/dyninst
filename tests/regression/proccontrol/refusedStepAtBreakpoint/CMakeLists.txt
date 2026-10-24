include_guard(GLOBAL)

# Only Power and AArch64 emulate single steps through load-reserve/store-conditional
# sequences, so only they can refuse one.
if(NOT (DYNINST_HOST_ARCH_PPC64LE OR DYNINST_HOST_ARCH_AARCH64))
  return()
endif()

add_executable(refused_step_mutatee mutatee.c)

add_executable(refused_step_at_breakpoint refused_step.cpp)
target_compile_options(refused_step_at_breakpoint PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(refused_step_at_breakpoint PRIVATE pcontrol symtabAPI)

add_test(
  NAME proccontrol_refused_step_at_breakpoint
  COMMAND refused_step_at_breakpoint $<TARGET_FILE:refused_step_mutatee>
)
set_tests_properties(
  proccontrol_refused_step_at_breakpoint
  PROPERTIES LABELS "regression" TIMEOUT 120
)
