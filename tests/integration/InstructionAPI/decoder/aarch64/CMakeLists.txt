include_guard(GLOBAL)

add_executable(aarch64_addressing_test addressing.cpp)
target_compile_options(aarch64_addressing_test PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(aarch64_addressing_test PRIVATE instructionAPI register_tests
                                                      opcode_tests)
add_test(NAME instructionAPI_aarch64_addressing_test COMMAND aarch64_addressing_test)
set_tests_properties(instructionAPI_aarch64_addressing_test PROPERTIES LABELS
                                                                       "integration")

add_executable(aarch64_bits_test bits.cpp)
target_compile_options(aarch64_bits_test PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(aarch64_bits_test PRIVATE instructionAPI register_tests
                                                opcode_tests)
add_test(NAME instructionAPI_aarch64_bits_test COMMAND aarch64_bits_test)
set_tests_properties(instructionAPI_aarch64_bits_test PROPERTIES LABELS "integration")

add_executable(aarch64_branches_test branches.cpp)
target_compile_options(aarch64_branches_test PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(aarch64_branches_test PRIVATE instructionAPI register_tests
                                                    opcode_tests)
add_test(NAME instructionAPI_aarch64_branches_test COMMAND aarch64_branches_test)
set_tests_properties(instructionAPI_aarch64_branches_test PROPERTIES LABELS "integration")

add_executable(aarch64_comparisons_test comparisons.cpp)
target_compile_options(aarch64_comparisons_test PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(aarch64_comparisons_test PRIVATE instructionAPI register_tests
                                                       opcode_tests)
add_test(NAME instructionAPI_aarch64_comparisons_test COMMAND aarch64_comparisons_test)
set_tests_properties(instructionAPI_aarch64_comparisons_test PROPERTIES LABELS
                                                                        "integration")

add_executable(aarch64_float_test float.cpp)
target_compile_options(aarch64_float_test PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(aarch64_float_test PRIVATE instructionAPI register_tests
                                                 opcode_tests)
add_test(NAME instructionAPI_aarch64_float_test COMMAND aarch64_float_test)
set_tests_properties(instructionAPI_aarch64_float_test PROPERTIES LABELS "integration")

add_executable(aarch64_math_test math.cpp)
target_compile_options(aarch64_math_test PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(aarch64_math_test PRIVATE instructionAPI register_tests
                                                opcode_tests)
add_test(NAME instructionAPI_aarch64_math_test COMMAND aarch64_math_test)
set_tests_properties(instructionAPI_aarch64_math_test PROPERTIES LABELS "integration")

add_executable(aarch64_move_test move.cpp)
target_compile_options(aarch64_move_test PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(aarch64_move_test PRIVATE instructionAPI register_tests
                                                opcode_tests)
add_test(NAME instructionAPI_aarch64_move_test COMMAND aarch64_move_test)
set_tests_properties(instructionAPI_aarch64_move_test PROPERTIES LABELS "integration")

add_executable(aarch64_sysctl_test sysctl.cpp)
target_compile_options(aarch64_sysctl_test PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(aarch64_sysctl_test PRIVATE instructionAPI register_tests
                                                  opcode_tests)
add_test(NAME instructionAPI_aarch64_sysctl_test COMMAND aarch64_sysctl_test)
set_tests_properties(instructionAPI_aarch64_sysctl_test PROPERTIES LABELS "integration")
