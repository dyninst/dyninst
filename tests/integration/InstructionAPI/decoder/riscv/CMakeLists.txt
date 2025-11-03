include_guard(GLOBAL)

add_executable(rv64i rv64i.cpp)
target_compile_options(rv64i PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(rv64i PRIVATE instructionAPI cft_tests memory_tests
                                        register_tests)
add_test(NAME instructionAPI_rv64i COMMAND rv64i)
set_tests_properties(instructionAPI_rv64i PROPERTIES LABELS "integration")
