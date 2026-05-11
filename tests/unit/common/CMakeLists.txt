include_guard(GLOBAL)

add_executable(pathName filesystem.cpp)
target_compile_options(pathName PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(pathName PRIVATE common)
add_test(NAME common_pathName COMMAND pathName)
set_tests_properties(common_pathName PROPERTIES LABELS "unit" SKIP_REGULAR_EXPRESSION
                                                "not defined in environment")

add_executable(bitmath_test bitmath.cpp)
target_compile_options(bitmath_test PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(bitmath_test PRIVATE common)
add_test(NAME common_bitmath_test COMMAND bitmath_test)
set_tests_properties(common_bitmath_test PROPERTIES LABELS "unit")

add_subdirectory(registers)
