include_guard(GLOBAL)

# Most of these tests use more than or are close to the default stack size warning
# limit, so increase the amount for all tests.
set(_flags)
if(maxFrameSizeOverrideRiscvInstructionApiTests)
  set(_flags "-Wframe-larger-than=${maxFrameSizeOverrideRiscvInstructionApiTests}")
endif()

dyninst_add_test(
  instructionAPI_rv64i_base_riscv64
  TYPE integration
  SOURCE_FILES rv64i_base.cpp
  DEPS instructionAPI cft_tests memory_tests register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
  REQUIRES capstone
)

dyninst_add_test(
  instructionAPI_zicsr_ext_riscv64
  TYPE integration
  SOURCE_FILES zicsr_ext.cpp
  DEPS instructionAPI cft_tests memory_tests register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
  REQUIRES capstone
)

dyninst_add_test(
  instructionAPI_m_ext_riscv64
  TYPE integration
  SOURCE_FILES m_ext.cpp
  DEPS instructionAPI cft_tests memory_tests register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
  REQUIRES capstone
)

dyninst_add_test(
  instructionAPI_a_ext_riscv64
  TYPE integration
  SOURCE_FILES a_ext.cpp
  DEPS instructionAPI cft_tests memory_tests register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
  REQUIRES capstone
)

dyninst_add_test(
  instructionAPI_f_ext_riscv64
  TYPE integration
  SOURCE_FILES f_ext.cpp
  DEPS instructionAPI cft_tests memory_tests register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
  REQUIRES capstone
)

dyninst_add_test(
  instructionAPI_d_ext_riscv64
  TYPE integration
  SOURCE_FILES d_ext.cpp
  DEPS instructionAPI cft_tests memory_tests register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
  REQUIRES capstone
)

dyninst_add_test(
  instructionAPI_c_ext_riscv64
  TYPE integration
  SOURCE_FILES c_ext.cpp
  DEPS instructionAPI cft_tests memory_tests register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
  REQUIRES capstone
)

dyninst_add_test(
  instructionAPI_branches_riscv64
  TYPE integration
  SOURCE_FILES branches.cpp
  DEPS instructionAPI cft_tests memory_tests register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
  REQUIRES capstone
)

dyninst_add_test(
  instructionAPI_pseudo_riscv64
  TYPE integration
  SOURCE_FILES pseudo.cpp
  DEPS instructionAPI cft_tests memory_tests register_tests opcode_tests
  COMPILE_OPTIONS ${_flags}
  REQUIRES capstone
)

unset(_flags)
