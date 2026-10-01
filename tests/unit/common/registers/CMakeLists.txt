include_guard(GLOBAL)

add_executable(uniqueness uniqueness.cpp)
target_compile_options(uniqueness PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(uniqueness PRIVATE symtabAPI)
add_test(NAME common_registers_uniqueness COMMAND uniqueness)
set_tests_properties(common_registers_uniqueness PROPERTIES LABELS "unit")

add_executable(liveness_set liveness_set.cpp)
target_compile_options(liveness_set PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(liveness_set PRIVATE common)
add_test(NAME common_registers_liveness_set COMMAND liveness_set)
set_tests_properties(common_registers_liveness_set PROPERTIES LABELS "unit")
