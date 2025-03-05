include_guard(GLOBAL)

add_executable(lea_nop_x86 lea-nop-x86.cpp)
target_compile_options(lea_nop_x86 PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(lea_nop_x86 PRIVATE parseAPI)

add_test(NAME parseAPI_lea_nop_x86 COMMAND lea_nop_x86)
set_tests_properties(parseAPI_lea_nop_x86 PROPERTIES LABELS "unit")
