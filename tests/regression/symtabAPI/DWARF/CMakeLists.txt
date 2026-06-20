include_guard(GLOBAL)

set(_targ dwarf5_inline_func)
add_executable(${_targ} dwarf5_inline_func.cpp)
target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(${_targ} PRIVATE symtabAPI)

if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)
  add_test(NAME symtabAPI_${_targ}_test
           COMMAND ${_targ}
                   "${DYNINST_TEST_BINARIES_DIR}/symtabAPI/DWARF/dwarf5_inline_func")
  set_tests_properties(symtabAPI_${_targ}_test PROPERTIES LABELS "regression")
endif()
