include_guard(GLOBAL)

if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)

  set(_targ unrelocated_cft)
  add_executable(${_targ} unrelocated_cft.cpp)
  target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
  target_link_libraries(${_targ} PRIVATE parseAPI)
  set(_fpath "${DYNINST_TEST_BINARIES_DIR}/parseAPI/UnappliedRelocations")

  # loadmsgcat.o ends section __libc_freeres_fn [0,0xe5) with an unrelocated
  # 'jmp free' at 0xe0 whose placeholder target 0xe5 lies in the overlapping
  # .text section. The parsing log must show the target being rejected, and
  # must not show it being taken as a cross-region tail call.
  set(_tname parseAPI_${_targ}_test)
  add_test(NAME ${_tname} COMMAND ${_targ} "${_fpath}/loadmsgcat.o")
  set_tests_properties(
    ${_tname}
    PROPERTIES
      LABELS
      "regression"
      ENVIRONMENT
      "DYNINST_DEBUG_PARSING=1;OMP_NUM_THREADS=1"
      PASS_REGULAR_EXPRESSION
      "REJECTED: CF target of '[^']*' at 0xe0 carries an unapplied relocation \\(placeholder target 0xe5\\)"
      FAIL_REGULAR_EXPRESSION
      "jump to 0xe5 in other regions, TAIL CALL")

  # indirect_nopic.o (-fno-pic -fno-plt) has memory-indirect control flow
  # whose relocations are on the memory operand, not on a control-flow
  # displacement: the switch dispatch 'jmp *0x0(,%rdi,8)' at 0xf and calls
  # through the GOT 'call *0x0(%rip)'. Those must keep their normal indirect
  # handling: jump-table analysis attempted, calls classified dynamic.
  set(_tname parseAPI_${_targ}_indirect_test)
  add_test(NAME ${_tname} COMMAND ${_targ} "${_fpath}/indirect_nopic.o")
  set_tests_properties(
    ${_tname}
    PROPERTIES LABELS
               "regression"
               ENVIRONMENT
               "DYNINST_DEBUG_PARSING=1;OMP_NUM_THREADS=1"
               PASS_REGULAR_EXPRESSION
               "jump table candidate [^\n]* at 0xf"
               FAIL_REGULAR_EXPRESSION
               "not treating as indirect;has an unresolved relocation, not dynamic")

endif()
