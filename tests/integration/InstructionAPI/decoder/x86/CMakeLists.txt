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
