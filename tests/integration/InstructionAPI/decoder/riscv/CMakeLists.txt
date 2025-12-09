include_guard(GLOBAL)

add_executable(rv64i_base rv64i_base.cpp)
target_compile_options(rv64i_base PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(rv64i_base PRIVATE instructionAPI cft_tests memory_tests
                                         register_tests opcode_tests)
add_test(NAME instructionAPI_rv64i_base COMMAND rv64i_base)
set_tests_properties(instructionAPI_rv64i_base PROPERTIES LABELS "integration")

add_executable(zicsr_ext zicsr_ext.cpp)
target_compile_options(zicsr_ext PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(zicsr_ext PRIVATE instructionAPI cft_tests memory_tests
                                        register_tests opcode_tests)
add_test(NAME instructionAPI_zicsr_ext COMMAND zicsr_ext)
set_tests_properties(instructionAPI_zicsr_ext PROPERTIES LABELS "integration")

add_executable(m_ext m_ext.cpp)
target_compile_options(m_ext PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(m_ext PRIVATE instructionAPI cft_tests memory_tests register_tests
                                    opcode_tests)
add_test(NAME instructionAPI_m_ext COMMAND m_ext)
set_tests_properties(instructionAPI_m_ext PROPERTIES LABELS "integration")

add_executable(a_ext a_ext.cpp)
target_compile_options(a_ext PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(a_ext PRIVATE instructionAPI cft_tests memory_tests register_tests
                                    opcode_tests)
add_test(NAME instructionAPI_a_ext COMMAND a_ext)
set_tests_properties(instructionAPI_a_ext PROPERTIES LABELS "integration")

add_executable(f_ext f_ext.cpp)
target_compile_options(f_ext PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(f_ext PRIVATE instructionAPI cft_tests memory_tests register_tests
                                    opcode_tests)
add_test(NAME instructionAPI_f_ext COMMAND f_ext)
set_tests_properties(instructionAPI_f_ext PROPERTIES LABELS "integration")

add_executable(d_ext d_ext.cpp)
target_compile_options(d_ext PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(d_ext PRIVATE instructionAPI cft_tests memory_tests register_tests
                                    opcode_tests)
add_test(NAME instructionAPI_d_ext COMMAND d_ext)
set_tests_properties(instructionAPI_d_ext PROPERTIES LABELS "integration")

add_executable(c_ext c_ext.cpp)
target_compile_options(c_ext PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(c_ext PRIVATE instructionAPI cft_tests memory_tests register_tests
                                    opcode_tests)
add_test(NAME instructionAPI_c_ext COMMAND c_ext)
set_tests_properties(instructionAPI_c_ext PROPERTIES LABELS "integration")

add_executable(branches branches.cpp)
target_compile_options(branches PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(branches PRIVATE instructionAPI cft_tests memory_tests
                                       register_tests opcode_tests)
add_test(NAME instructionAPI_branches COMMAND branches)
set_tests_properties(instructionAPI_branches PROPERTIES LABELS "integration")

add_executable(pseudo pseudo.cpp)
target_compile_options(pseudo PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(pseudo PRIVATE instructionAPI cft_tests memory_tests register_tests
                                     opcode_tests)
add_test(NAME instructionAPI_pseudo COMMAND pseudo)
set_tests_properties(instructionAPI_pseudo PROPERTIES LABELS "integration")
