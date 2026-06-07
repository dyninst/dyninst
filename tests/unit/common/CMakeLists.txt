include_guard(GLOBAL)

add_executable(filesystem filesystem.cpp)
target_compile_options(filesystem PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(filesystem PRIVATE common)
add_test(NAME common_filesystem COMMAND filesystem)
set_tests_properties(common_filesystem PROPERTIES LABELS "unit" SKIP_REGULAR_EXPRESSION
                                                  "not defined in environment")

add_executable(bitmath_test bitmath.cpp)
target_compile_options(bitmath_test PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(bitmath_test PRIVATE common)
add_test(NAME common_bitmath_test COMMAND bitmath_test)
set_tests_properties(common_bitmath_test PROPERTIES LABELS "unit")

add_subdirectory(registers)
