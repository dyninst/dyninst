include_guard(GLOBAL)

# Disable warnings for stack frame sizes
set(_flags "-Wframe-larger-than=50000")

dyninst_add_test(
  instructionAPI_aarch64_addressing_test
  TYPE integration
  SOURCE_FILES addressing.cpp
  DEPS instructionAPI register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
)

dyninst_add_test(
  instructionAPI_aarch64_bits_test
  TYPE integration
  SOURCE_FILES bits.cpp
  DEPS instructionAPI register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
)

dyninst_add_test(
  instructionAPI_aarch64_branches_test
  TYPE integration
  SOURCE_FILES branches.cpp
  DEPS instructionAPI register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
)

dyninst_add_test(
  instructionAPI_aarch64_cft_test
  TYPE integration
  SOURCE_FILES cft.cpp
  DEPS instructionAPI
  COMPILE_OPTIONS ${_flags}
)

dyninst_add_test(
  instructionAPI_aarch64_comparisons_test
  TYPE integration
  SOURCE_FILES comparisons.cpp
  DEPS instructionAPI register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
)

dyninst_add_test(
  instructionAPI_aarch64_float_test
  TYPE integration
  SOURCE_FILES float.cpp
  DEPS instructionAPI register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
)

dyninst_add_test(
  instructionAPI_aarch64_ldst_test
  TYPE integration
  SOURCE_FILES ldst.cpp
  DEPS instructionAPI register_tests
  COMPILE_OPTIONS ${_flags}
)

dyninst_add_test(
  instructionAPI_aarch64_math_test
  TYPE integration
  SOURCE_FILES math.cpp
  DEPS instructionAPI register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
)

dyninst_add_test(
  instructionAPI_aarch64_move_test
  TYPE integration
  SOURCE_FILES move.cpp
  DEPS instructionAPI register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
)

dyninst_add_test(
  instructionAPI_aarch64_simd_test
  TYPE integration
  SOURCE_FILES simd.cpp
  DEPS instructionAPI register_tests
  COMPILE_OPTIONS ${_flags}
)

dyninst_add_test(
  instructionAPI_aarch64_sysctl_test
  TYPE integration
  SOURCE_FILES sysctl.cpp
  DEPS instructionAPI register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
)

unset(_flags)
