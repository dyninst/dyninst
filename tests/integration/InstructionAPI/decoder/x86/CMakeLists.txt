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
