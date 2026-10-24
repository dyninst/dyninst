include_guard(GLOBAL)

if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)
  set(_targ symeval_expand_slice)
  add_executable(${_targ} expand_slice.cpp)
  target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
  target_link_libraries(${_targ} PRIVATE parseAPI)

  # glibc's aarch64 _start: 'mov x6, sp'. Runs on every host: the binary is
  # only read.
  set(_tname dataflowAPI_${_targ}_test)
  add_test(
    NAME ${_tname}
    COMMAND
      ${_targ}
      "${DYNINST_TEST_BINARIES_DIR}/dyninstAPI/findMain/aarch64/stat.nopie.nostripped"
  )
  set_tests_properties(${_tname} PROPERTIES LABELS "regression")
endif()
