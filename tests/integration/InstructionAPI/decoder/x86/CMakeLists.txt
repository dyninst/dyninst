include_guard(GLOBAL)

add_executable(call_tests call.cpp)
target_compile_options(call_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(call_tests PRIVATE instructionAPI cft_tests memory_tests
                                         register_tests opcode_tests)
add_test(NAME instructionAPI_call_tests COMMAND call_tests)
set_tests_properties(instructionAPI_call_tests PROPERTIES LABELS "integration")

add_executable(ptwrite_tests ptwrite.cpp)
target_compile_options(ptwrite_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(ptwrite_tests PRIVATE instructionAPI cft_tests memory_tests
                                            register_tests opcode_tests)
add_test(NAME instructionAPI_ptwrite_tests COMMAND ptwrite_tests)
set_tests_properties(instructionAPI_ptwrite_tests PROPERTIES LABELS "integration")

add_executable(decoding_tests decoding.cpp)
target_compile_options(decoding_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(decoding_tests PRIVATE instructionAPI cft_tests memory_tests
                                             register_tests opcode_tests)
add_test(NAME instructionAPI_decoding_tests COMMAND decoding_tests)
set_tests_properties(instructionAPI_decoding_tests PROPERTIES LABELS "integration")
