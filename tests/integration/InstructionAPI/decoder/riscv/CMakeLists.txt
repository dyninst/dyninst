include_guard(GLOBAL)

# Create set of warning flags used to compiler tests.
# Most of these tests use more than or are close to the default stack size
# warning limit, so increase the amount for all tests.
#
set(RISCV_INST_TESTS_CXX_WARNING_FLAGS ${SUPPORTED_CXX_WARNING_FLAGS})
if(maxFrameSizeOverrideRiscvInstructionApiTests)
  list(APPEND RISCV_INST_TESTS_CXX_WARNING_FLAGS
       "-Wframe-larger-than=${maxFrameSizeOverrideRiscvInstructionApiTests}")
endif()

add_executable(rv64i_base rv64i_base.cpp)
target_compile_options(rv64i_base PRIVATE ${RISCV_INST_TESTS_CXX_WARNING_FLAGS})
target_link_libraries(rv64i_base PRIVATE instructionAPI cft_tests memory_tests
                                         register_tests opcode_tests)
add_test(NAME instructionAPI_rv64i_base_riscv64 COMMAND rv64i_base)
set_tests_properties(instructionAPI_rv64i_base_riscv64 PROPERTIES LABELS "integration")

add_executable(zicsr_ext zicsr_ext.cpp)
target_compile_options(zicsr_ext PRIVATE ${RISCV_INST_TESTS_CXX_WARNING_FLAGS})
target_link_libraries(zicsr_ext PRIVATE instructionAPI cft_tests memory_tests
                                        register_tests opcode_tests)
add_test(NAME instructionAPI_zicsr_ext_riscv64 COMMAND zicsr_ext)
set_tests_properties(instructionAPI_zicsr_ext_riscv64 PROPERTIES LABELS "integration")

add_executable(m_ext m_ext.cpp)
target_compile_options(m_ext PRIVATE ${RISCV_INST_TESTS_CXX_WARNING_FLAGS})
target_link_libraries(m_ext PRIVATE instructionAPI cft_tests memory_tests register_tests
                                    opcode_tests)
add_test(NAME instructionAPI_m_ext_riscv64 COMMAND m_ext)
set_tests_properties(instructionAPI_m_ext_riscv64 PROPERTIES LABELS "integration")

add_executable(a_ext a_ext.cpp)
target_compile_options(a_ext PRIVATE ${RISCV_INST_TESTS_CXX_WARNING_FLAGS})
target_link_libraries(a_ext PRIVATE instructionAPI cft_tests memory_tests register_tests
                                    opcode_tests)
add_test(NAME instructionAPI_a_ext_riscv64 COMMAND a_ext)
set_tests_properties(instructionAPI_a_ext_riscv64 PROPERTIES LABELS "integration")

add_executable(f_ext f_ext.cpp)
target_compile_options(f_ext PRIVATE ${RISCV_INST_TESTS_CXX_WARNING_FLAGS})
target_link_libraries(f_ext PRIVATE instructionAPI cft_tests memory_tests register_tests
                                    opcode_tests)
add_test(NAME instructionAPI_f_ext_riscv64 COMMAND f_ext)
set_tests_properties(instructionAPI_f_ext_riscv64 PROPERTIES LABELS "integration")

add_executable(d_ext d_ext.cpp)
target_compile_options(d_ext PRIVATE ${RISCV_INST_TESTS_CXX_WARNING_FLAGS})
target_link_libraries(d_ext PRIVATE instructionAPI cft_tests memory_tests register_tests
                                    opcode_tests)
add_test(NAME instructionAPI_d_ext_riscv64 COMMAND d_ext)
set_tests_properties(instructionAPI_d_ext_riscv64 PROPERTIES LABELS "integration")

add_executable(c_ext c_ext.cpp)
target_compile_options(c_ext PRIVATE ${RISCV_INST_TESTS_CXX_WARNING_FLAGS})
target_link_libraries(c_ext PRIVATE instructionAPI cft_tests memory_tests register_tests
                                    opcode_tests)
add_test(NAME instructionAPI_c_ext_riscv64 COMMAND c_ext)
set_tests_properties(instructionAPI_c_ext_riscv64 PROPERTIES LABELS "integration")

add_executable(branches branches.cpp)
target_compile_options(branches PRIVATE ${RISCV_INST_TESTS_CXX_WARNING_FLAGS})
target_link_libraries(branches PRIVATE instructionAPI cft_tests memory_tests
                                       register_tests opcode_tests)
add_test(NAME instructionAPI_branches_riscv64 COMMAND branches)
set_tests_properties(instructionAPI_branches_riscv64 PROPERTIES LABELS "integration")

add_executable(pseudo pseudo.cpp)
target_compile_options(pseudo PRIVATE ${RISCV_INST_TESTS_CXX_WARNING_FLAGS})
target_link_libraries(pseudo PRIVATE instructionAPI cft_tests memory_tests register_tests
                                     opcode_tests)
add_test(NAME instructionAPI_pseudo_riscv64 COMMAND pseudo)
set_tests_properties(instructionAPI_pseudo_riscv64 PROPERTIES LABELS "integration")
