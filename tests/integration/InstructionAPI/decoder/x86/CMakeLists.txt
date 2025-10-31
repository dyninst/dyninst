include_guard(GLOBAL)

add_executable(call_tests call.cpp)
target_compile_options(call_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(call_tests PRIVATE instructionAPI cft_tests memory_tests
                                         register_tests)
add_test(NAME instructionAPI_call_tests COMMAND call_tests)
set_tests_properties(instructionAPI_call_tests PROPERTIES LABELS "integration")
