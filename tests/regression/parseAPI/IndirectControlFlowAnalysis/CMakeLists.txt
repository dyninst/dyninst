include_guard(GLOBAL)

if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)

set(_targ IndirectControlFlowAnalysis)
add_executable(${_targ} branchInfo.cpp)
target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(${_targ} PRIVATE parseAPI)
set(_fpath "${DYNINST_TEST_BINARIES_DIR}/parseAPI/IndirectControlFlowAnalysis")

foreach(_arch 908 90a 940)
add_test(NAME parseAPI_${_targ}_${_arch} COMMAND ${_targ} "${_fpath}/${_arch}.gpubin" ${_arch}.out)
add_test(NAME  compare_${_targ}_${_arch} COMMAND ${CMAKE_COMMAND} -E compare_files ${_arch}.out "${_fpath}/expected_result/${_arch}.ref.out")
add_test(NAME  cleanup_${_targ}_${_arch} COMMAND ${CMAKE_COMMAND} -E rm -f ${_arch}.out)
set_tests_properties(parseAPI_${_targ}_${_arch} PROPERTIES LABELS "regression" FIXTURES_SETUP    ${_targ}_${_arch})
set_tests_properties( compare_${_targ}_${_arch} PROPERTIES LABELS "regression" FIXTURES_REQUIRED ${_targ}_${_arch})
set_tests_properties( cleanup_${_targ}_${_arch} PROPERTIES LABELS "regression" FIXTURES_CLEANUP  ${_targ}_${_arch})
endforeach()

endif()
