include_guard(GLOBAL)

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
