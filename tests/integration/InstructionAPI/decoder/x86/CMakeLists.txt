include_guard(GLOBAL)

add_executable(arithmetic_tests arithmetic.cpp)
target_compile_options(arithmetic_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(arithmetic_tests PRIVATE instructionAPI cft_tests memory_tests
                                               register_tests)
add_test(NAME instructionAPI_arithmetic_tests COMMAND arithmetic_tests)
set_tests_properties(instructionAPI_arithmetic_tests PROPERTIES LABELS "integration")

add_executable(branches_tests branches.cpp)
target_compile_options(branches_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(branches_tests PRIVATE instructionAPI cft_tests memory_tests
                                             register_tests)
add_test(NAME instructionAPI_branches_tests COMMAND branches_tests)
set_tests_properties(instructionAPI_branches_tests PROPERTIES LABELS "integration")

add_executable(call_tests call.cpp)
target_compile_options(call_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(call_tests PRIVATE instructionAPI cft_tests memory_tests
                                         register_tests opcode_tests)
add_test(NAME instructionAPI_call_tests COMMAND call_tests)
set_tests_properties(instructionAPI_call_tests PROPERTIES LABELS "integration")

add_executable(data_transfer_tests data_transfer.cpp)
target_compile_options(data_transfer_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(data_transfer_tests PRIVATE instructionAPI cft_tests memory_tests
                                                  register_tests)
add_test(NAME instructionAPI_data_transfer_tests COMMAND data_transfer_tests)
set_tests_properties(instructionAPI_data_transfer_tests PROPERTIES LABELS "integration")

add_executable(sse_tests sse.cpp)
target_compile_options(sse_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(sse_tests PRIVATE instructionAPI cft_tests memory_tests
                                        register_tests)
add_test(NAME instructionAPI_sse_tests COMMAND sse_tests)
set_tests_properties(instructionAPI_sse_tests PROPERTIES LABELS "integration")

add_executable(stack_tests stack.cpp)
target_compile_options(stack_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(stack_tests PRIVATE instructionAPI memory_tests register_tests)
add_test(NAME instructionAPI_stack_tests COMMAND stack_tests)
set_tests_properties(instructionAPI_stack_tests PROPERTIES LABELS "integration")

add_executable(strings_tests strings.cpp)
target_compile_options(strings_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(strings_tests PRIVATE instructionAPI memory_tests register_tests)
add_test(NAME instructionAPI_strings_tests COMMAND strings_tests)
set_tests_properties(instructionAPI_strings_tests PROPERTIES LABELS "integration")
