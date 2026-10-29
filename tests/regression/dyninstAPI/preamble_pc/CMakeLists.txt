include_guard(GLOBAL)

set(_targ preamble_pc)
set(_tname dyninstAPI_${_targ}_test)

# The skipped TOC preamble exists only on ppc64 (ELFv2 global entry points),
# and the test creates a live mutatee, so it needs a native ppc64le build.
# dyninstAPI is not built with LIGHTWEIGHT_SYMTAB. Where the test cannot be
# built, a stand-in reports it as skipped. Where ptrace is refused (some
# containers), the test skips itself.
if(LIGHTWEIGHT_SYMTAB)
  set(_skip_reason "dyninstAPI is not built with LIGHTWEIGHT_SYMTAB")
elseif(NOT (DYNINST_HOST_ARCH_PPC64LE AND DYNINST_CODEGEN_ARCH_PPC64LE))
  set(_skip_reason "needs a native ppc64le build (the TOC preamble is ppc64-only)")
endif()

if(DEFINED _skip_reason)
  add_executable(${_targ} skipped.c)
  target_compile_definitions(${_targ} PRIVATE "SKIP_REASON=\"${_skip_reason}\"")
  add_test(NAME ${_tname} COMMAND ${_targ})
else()
  add_executable(preamble_pc_mutatee preamble_pc_mutatee.c)

  add_executable(${_targ} preamble_pc.cpp)
  target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
  target_link_libraries(${_targ} PRIVATE dyninstAPI)

  add_test(
    NAME ${_tname}
    COMMAND ${_targ} $<TARGET_FILE:preamble_pc_mutatee> $<TARGET_FILE_DIR:dyninstAPI_RT>
  )
  set_tests_properties(
    ${_tname}
    PROPERTIES ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
  )
endif()

set_tests_properties(
  ${_tname}
  PROPERTIES
    LABELS "regression"
    TIMEOUT 120
    SKIP_RETURN_CODE 77
    SKIP_REGULAR_EXPRESSION "skipping:"
)
