include_guard(GLOBAL)

set(_targ IndirectControlFlowAnalysis)
dyninst_add_test_executable(
  ${_targ}
  SOURCE_FILES branchInfo.cpp
  DEPS parseAPI
  REQUIRES test_binaries
)
set(_fpath "${DYNINST_TEST_BINARIES_DIR}/parseAPI/IndirectControlFlowAnalysis")

# The parse writes its output, compare checks it, and cleanup removes it. All three
# require test_binaries, so they skip together: a skipped setup test counts as passed.
foreach(_arch 908 90a 940)
  set(_tfname parseAPI_${_targ}_${_arch})
  dyninst_add_test(
    parseAPI_${_targ}_${_arch}
    TYPE regression
    COMMAND ${_targ} "${_fpath}/${_tfname}.gpubin" ${_tfname}.out
    PROPERTIES FIXTURES_SETUP ${_targ}_${_arch}
  )
  dyninst_add_test(
    compare_${_targ}_${_arch}
    TYPE regression
    COMMAND
      ${CMAKE_COMMAND} -E compare_files ${_tfname}.out
      "${_fpath}/expected_result/${_tfname}.ref.out"
    REQUIRES test_binaries
    PROPERTIES FIXTURES_REQUIRED ${_targ}_${_arch}
  )
  dyninst_add_test(
    cleanup_${_targ}_${_arch}
    TYPE regression
    COMMAND ${CMAKE_COMMAND} -E rm -f ${_tfname}.out
    REQUIRES test_binaries
    PROPERTIES FIXTURES_CLEANUP ${_targ}_${_arch}
  )
endforeach()
