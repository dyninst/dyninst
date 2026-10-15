include_guard(GLOBAL)

set(_targ findEnclosingRegion)
add_executable(${_targ} findEnclosingRegion.cpp)
target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(${_targ} PRIVATE symtabAPI)

if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)
  add_test(
    NAME symtabAPI_${_targ}_tbss_test
    COMMAND ${_targ}
            "${DYNINST_TEST_BINARIES_DIR}/dyninstAPI/findMain/ppc64le/stat.nopie.stripped")
  set_tests_properties(symtabAPI_${_targ}_tbss_test PROPERTIES LABELS "regression")
endif()
