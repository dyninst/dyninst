include_guard(GLOBAL)

add_executable(getUsedRegisters getUsedRegisters.cpp)
target_compile_options(getUsedRegisters PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(getUsedRegisters PRIVATE instructionAPI)
add_test(NAME instructionapi_getUsedRegisters COMMAND getUsedRegisters "64")
set_tests_properties(instructionapi_getUsedRegisters PROPERTIES LABELS "unit")
