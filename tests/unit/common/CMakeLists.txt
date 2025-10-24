include_guard(GLOBAL)

add_executable(pathName filesystem.cpp)
target_compile_options(pathName PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(pathName PRIVATE common)
add_test(NAME common_pathName COMMAND pathName)
set_tests_properties(common_pathName PROPERTIES LABELS "unit" SKIP_REGULAR_EXPRESSION
                                                "not defined in environment")

add_subdirectory(registers)
