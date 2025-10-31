include_guard(GLOBAL)

message(STATUS "Enabling regression tests")

add_subdirectory(symtabAPI)

if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)

  set(_input_file "${DYNINST_TEST_BINARIES_DIR}/parseAPI/test1")
  add_executable(binary_test binary_test.cpp)
  target_compile_options(binary_test PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
  target_link_libraries(binary_test PRIVATE symtabAPI)
  add_test(NAME common_binary_test COMMAND binary_test "${_input_file}")
  set_tests_properties(common_binary_test PROPERTIES LABELS "regression")

endif()
