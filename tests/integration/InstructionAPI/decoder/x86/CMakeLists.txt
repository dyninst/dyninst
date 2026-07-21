include_guard(GLOBAL)

add_executable(ptwrite_tests ptwrite.cpp)
target_compile_options(ptwrite_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(ptwrite_tests PRIVATE instructionAPI cft_tests memory_tests
                                            register_tests opcode_tests)
add_test(NAME instructionAPI_ptwrite_tests COMMAND ptwrite_tests)
set_tests_properties(instructionAPI_ptwrite_tests PROPERTIES LABELS "integration")

add_executable(x86_arithmetic arithmetic.cpp)
target_compile_options(x86_arithmetic PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(x86_arithmetic PRIVATE instructionAPI cft_tests memory_tests
                                             opcode_tests register_tests)
add_test(NAME instructionAPI_x86_arithmetic COMMAND x86_arithmetic)
set_tests_properties(instructionAPI_x86_arithmetic PROPERTIES LABELS "integration")

add_executable(x86_branches branches.cpp)
target_compile_options(x86_branches PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(x86_branches PRIVATE instructionAPI cft_tests memory_tests
                                           opcode_tests register_tests)
add_test(NAME instructionAPI_x86_branches COMMAND x86_branches)
set_tests_properties(instructionAPI_x86_branches PROPERTIES LABELS "integration")

add_executable(x86_call call.cpp)
target_compile_options(x86_call PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(x86_call PRIVATE instructionAPI cft_tests memory_tests opcode_tests
                                       register_tests)
add_test(NAME instructionAPI_x86_call COMMAND x86_call)
set_tests_properties(instructionAPI_x86_call PROPERTIES LABELS "integration")

add_executable(x86_data_transfer data_transfer.cpp)
target_compile_options(x86_data_transfer PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(x86_data_transfer PRIVATE instructionAPI cft_tests memory_tests
                                                opcode_tests register_tests)
add_test(NAME instructionAPI_x86_data_transfer_tests COMMAND x86_data_transfer)
set_tests_properties(instructionAPI_x86_data_transfer_tests PROPERTIES LABELS
                                                                       "integration")

add_executable(x86_sse sse.cpp)
target_compile_options(x86_sse PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(x86_sse PRIVATE instructionAPI cft_tests memory_tests opcode_tests
                                      register_tests)
add_test(NAME instructionAPI_x86_sse COMMAND x86_sse)
set_tests_properties(instructionAPI_x86_sse PROPERTIES LABELS "integration")

add_executable(x86_prefetch prefetch.cpp)
target_compile_options(x86_prefetch PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(x86_prefetch PRIVATE instructionAPI memory_tests opcode_tests
                                           register_tests)
add_test(NAME instructionAPI_x86_prefetch COMMAND x86_prefetch)
set_tests_properties(instructionAPI_x86_prefetch PROPERTIES LABELS "integration")

add_executable(x86_stack stack.cpp)
target_compile_options(x86_stack PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(x86_stack PRIVATE instructionAPI memory_tests
                                        opcode_tests register_tests)
add_test(NAME instructionAPI_x86_stack COMMAND x86_stack)
set_tests_properties(instructionAPI_x86_stack PROPERTIES LABELS "integration")

add_executable(x86_strings strings.cpp)
target_compile_options(x86_strings PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(x86_strings PRIVATE instructionAPI memory_tests
                                          opcode_tests register_tests)
add_test(NAME instructionAPI_x86_strings_test COMMAND x86_strings)
set_tests_properties(instructionAPI_x86_strings_test PROPERTIES LABELS "integration")

add_executable(sysctl_tests sysctl.cpp)
target_compile_options(sysctl_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(sysctl_tests PRIVATE instructionAPI cft_tests memory_tests
                                           register_tests)
add_test(NAME instructionAPI_sysctl_tests COMMAND sysctl_tests)
set_tests_properties(instructionAPI_sysctl_tests PROPERTIES LABELS "integration")

add_executable(x87_tests x87.cpp)
target_compile_options(x87_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(x87_tests PRIVATE instructionAPI cft_tests memory_tests opcode_tests
                                        register_tests)
add_test(NAME instructionAPI_x87_tests COMMAND x87_tests)
set_tests_properties(instructionAPI_x87_tests PROPERTIES LABELS "integration")
