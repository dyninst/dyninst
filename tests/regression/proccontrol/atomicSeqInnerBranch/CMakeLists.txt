include_guard(GLOBAL)

add_executable(atomic_seq_inner_branch inner_branch.cpp)
target_compile_options(atomic_seq_inner_branch PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(atomic_seq_inner_branch PRIVATE pcontrol symtabAPI)
# DYNINST_ENABLE_DYNAMIC_TESTS counts as ON where it is not defined
if(NOT DEFINED DYNINST_ENABLE_DYNAMIC_TESTS OR DYNINST_ENABLE_DYNAMIC_TESTS)
  target_compile_definitions(
    atomic_seq_inner_branch
    PRIVATE DYNINST_ENABLE_DYNAMIC_TESTS=1
  )
else()
  target_compile_definitions(
    atomic_seq_inner_branch
    PRIVATE DYNINST_ENABLE_DYNAMIC_TESTS=0
  )
endif()

# The mutatee is in the test binaries. Without them no path is passed, and the test
# reports a skip; so does an architecture that has no mutatee.
set(_mutatee)
if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)
  if(DYNINST_HOST_ARCH_AARCH64)
    set(_mutatee "${DYNINST_TEST_BINARIES_DIR}/proccontrol/atomicSeqInnerBranch/aarch64")
  elseif(DYNINST_HOST_ARCH_PPC64LE)
    set(_mutatee "${DYNINST_TEST_BINARIES_DIR}/proccontrol/atomicSeqInnerBranch/ppc64le")
  endif()
endif()
add_test(
  NAME proccontrol_atomic_seq_inner_branch
  COMMAND atomic_seq_inner_branch ${_mutatee}
)
set_tests_properties(
  proccontrol_atomic_seq_inner_branch
  PROPERTIES LABELS "regression" SKIP_RETURN_CODE 77 SKIP_REGULAR_EXPRESSION "skipping:"
)
