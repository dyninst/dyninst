include_guard(GLOBAL)

dyninst_add_test(
  instructionAPI_call_tests
  TYPE integration
  SOURCE_FILES call.cpp
  DEPS instructionAPI cft_tests memory_tests register_tests opcode_tests
)

dyninst_add_test(
  instructionAPI_ptwrite_tests
  TYPE integration
  SOURCE_FILES ptwrite.cpp
  DEPS instructionAPI cft_tests memory_tests register_tests opcode_tests
)

dyninst_add_test(
  instructionAPI_x86_bind_eval_tests
  TYPE integration
  SOURCE_FILES bind_eval.cpp
  DEPS instructionAPI
)

dyninst_add_test(
  instructionAPI_x86_farcall_tests
  TYPE integration
  SOURCE_FILES farcall.cpp
  DEPS instructionAPI
)

dyninst_add_test(
  instructionAPI_x86_fucompp_tests
  TYPE integration
  SOURCE_FILES fucompp.cpp
  DEPS instructionAPI register_tests
)

dyninst_add_test(
  instructionAPI_x86_mov_size_details_tests
  TYPE integration
  SOURCE_FILES mov_size_details.cpp
  DEPS instructionAPI
)

dyninst_add_test(
  instructionAPI_x86_read_write_tests
  TYPE integration
  SOURCE_FILES read_write.cpp
  DEPS instructionAPI register_tests
)
