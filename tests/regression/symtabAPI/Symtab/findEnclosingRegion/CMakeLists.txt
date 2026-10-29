include_guard(GLOBAL)

set(_targ findEnclosingRegion)
dyninst_add_test_executable(${_targ} SOURCE_FILES findEnclosingRegion.cpp DEPS symtabAPI)

# Needs no test binaries, so it runs in every configuration.
dyninst_add_test(symtabAPI_${_targ}_addRegion_test TYPE regression COMMAND ${_targ})

dyninst_add_test(
  symtabAPI_${_targ}_tbss_test
  TYPE regression
  COMMAND
    ${_targ} --binary
    "${DYNINST_TEST_BINARIES_DIR}/dyninstAPI/findMain/ppc64le/stat.nopie.stripped"
  REQUIRES test_binaries
)
