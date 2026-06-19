include_guard(GLOBAL)

include(DyninstBinaryTests)

set(_targ IndirectControlFlowAnalysis)
add_executable(${_targ} branchInfo.cpp)
target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(${_targ} PRIVATE parseAPI)
set(_fpath "${DYNINST_TEST_BINARIES_DIR}/parseAPI/IndirectControlFlowAnalysis")

add_test(NAME parseAPI_${_targ}_908 COMMAND ${_targ} "${_fpath}/1-908.gpubin" 1.out)
add_test(NAME  compare_${_targ}_908 COMMAND ${CMAKE_COMMAND} -E compare_files 1.out "${_fpath}/1-908.ref.out")
add_test(NAME parseAPI_${_targ}_90a COMMAND ${_targ} "${_fpath}/2-90a.gpubin" 2.out)
add_test(NAME  compare_${_targ}_90a COMMAND ${CMAKE_COMMAND} -E compare_files 2.out "${_fpath}/2-90a.ref.out")
add_test(NAME parseAPI_${_targ}_940 COMMAND ${_targ} "${_fpath}/3-940.gpubin" 3.out)
add_test(NAME  compare_${_targ}_940 COMMAND ${CMAKE_COMMAND} -E compare_files 3.out "${_fpath}/3-940.ref.out")

set_tests_properties(parseAPI_${_targ}_908 PROPERTIES LABELS "regression")
set_tests_properties(parseAPI_${_targ}_90a PROPERTIES LABELS "regression")
set_tests_properties(parseAPI_${_targ}_940 PROPERTIES LABELS "regression")

set_tests_properties(compare_${_targ}_908 PROPERTIES LABELS "regression")
set_tests_properties(compare_${_targ}_90a PROPERTIES LABELS "regression")
set_tests_properties(compare_${_targ}_940 PROPERTIES LABELS "regression")
