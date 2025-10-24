include_guard(GLOBAL)

add_executable(uniqueness uniqueness.cpp)
target_compile_options(uniqueness PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(uniqueness PRIVATE symtabAPI)
add_test(NAME common_registers_uniqueness COMMAND uniqueness)
set_tests_properties(common_registers_uniqueness PROPERTIES LABELS "unit")
