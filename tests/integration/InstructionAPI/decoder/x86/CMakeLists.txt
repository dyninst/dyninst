include_guard(GLOBAL)

add_executable(x86_arithmetic arithmetic.cpp)
target_compile_options(x86_arithmetic PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(x86_arithmetic PRIVATE instructionAPI cft_tests memory_tests
                                             opcode_tests register_tests)
add_test(NAME instructionAPI_x86_arithmetic COMMAND x86_arithmetic)
set_tests_properties(instructionAPI_x86_arithmetic PROPERTIES LABELS "integration")

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

add_executable(x86_bind_eval_tests bind_eval.cpp)
target_compile_options(x86_bind_eval_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(x86_bind_eval_tests PRIVATE instructionAPI)
add_test(NAME instructionAPI_x86_bind_eval_tests COMMAND x86_bind_eval_tests)
set_tests_properties(instructionAPI_x86_bind_eval_tests PROPERTIES LABELS "integration")

add_executable(x86_farcall_tests farcall.cpp)
target_compile_options(x86_farcall_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(x86_farcall_tests PRIVATE instructionAPI)
add_test(NAME instructionAPI_x86_farcall_tests COMMAND x86_farcall_tests)
set_tests_properties(instructionAPI_x86_farcall_tests PROPERTIES LABELS "integration")

add_executable(x86_fucompp_tests fucompp.cpp)
target_compile_options(x86_fucompp_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(x86_fucompp_tests PRIVATE instructionAPI register_tests)
add_test(NAME instructionAPI_x86_fucompp_tests COMMAND x86_fucompp_tests)
set_tests_properties(instructionAPI_x86_fucompp_tests PROPERTIES LABELS "integration")

add_executable(x86_mov_size_details_tests mov_size_details.cpp)
target_compile_options(x86_mov_size_details_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(x86_mov_size_details_tests PRIVATE instructionAPI)
add_test(NAME instructionAPI_x86_mov_size_details_tests
         COMMAND x86_mov_size_details_tests)
set_tests_properties(instructionAPI_x86_mov_size_details_tests PROPERTIES LABELS
                                                                          "integration")

add_executable(x86_read_write_tests read_write.cpp)
target_compile_options(x86_read_write_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(x86_read_write_tests PRIVATE instructionAPI register_tests)
add_test(NAME instructionAPI_x86_read_write_tests COMMAND x86_read_write_tests)
set_tests_properties(instructionAPI_x86_read_write_tests PROPERTIES LABELS "integration")
