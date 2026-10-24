include_guard(GLOBAL)

# Only Power and AArch64 emulate single steps through load-reserve/store-conditional
# sequences.
if(NOT (DYNINST_HOST_ARCH_PPC64LE OR DYNINST_HOST_ARCH_AARCH64))
  return()
endif()

add_executable(atomic_seq_inner_branch_mutatee mutatee.c)

add_executable(atomic_seq_inner_branch inner_branch.cpp)
target_compile_options(atomic_seq_inner_branch PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(atomic_seq_inner_branch PRIVATE pcontrol symtabAPI)

add_test(
  NAME proccontrol_atomic_seq_inner_branch
  COMMAND atomic_seq_inner_branch $<TARGET_FILE:atomic_seq_inner_branch_mutatee>
)
set_tests_properties(
  proccontrol_atomic_seq_inner_branch
  PROPERTIES LABELS "regression" TIMEOUT 120
)
