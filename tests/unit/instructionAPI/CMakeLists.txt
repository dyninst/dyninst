include_guard(GLOBAL)

add_executable(syscall_x86 syscall-x86.cpp)
target_compile_options(syscall_x86 PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(syscall_x86 PRIVATE instructionAPI)
add_test(NAME instructionAPI_syscall_x86 COMMAND syscall_x86)
set_tests_properties(instructionAPI_syscall_x86 PROPERTIES LABELS "unit")

add_subdirectory(AST)
