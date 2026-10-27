include_guard(GLOBAL)

set(_targ findEnclosingRegion)
add_executable(${_targ} findEnclosingRegion.cpp)
target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(${_targ} PRIVATE symtabAPI)

# Needs no test binaries, so it runs in every configuration.
add_test(NAME symtabAPI_${_targ}_addRegion_test COMMAND ${_targ})
set_tests_properties(symtabAPI_${_targ}_addRegion_test PROPERTIES LABELS "regression")

# Without the test binaries no path is passed, and the test reports a skip.
set(_binary)
if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)
  set(
    _binary
    "${DYNINST_TEST_BINARIES_DIR}/dyninstAPI/findMain/ppc64le/stat.nopie.stripped"
  )
endif()
add_test(NAME symtabAPI_${_targ}_tbss_test COMMAND ${_targ} --binary ${_binary})
set_tests_properties(
  symtabAPI_${_targ}_tbss_test
  PROPERTIES LABELS "regression" SKIP_REGULAR_EXPRESSION "skipping:"
)
