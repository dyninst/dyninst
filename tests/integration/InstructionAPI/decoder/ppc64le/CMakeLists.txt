include_guard(GLOBAL)

dyninst_add_test(
  instructionAPI_ppc64le_decode_test
  TYPE integration
  SOURCE_FILES decode.cpp
  DEPS instructionAPI register_tests
)

dyninst_add_test(
  instructionAPI_ppc64le_branches_test
  TYPE integration
  SOURCE_FILES branches.cpp
  DEPS instructionAPI
)
