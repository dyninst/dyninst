include_guard(GLOBAL)

set(_targ crash_callback)
set(_tname proccontrol_${_targ}_test)

# The test creates a live mutatee, so it needs a native build: the code
# generator must target the host. dyninstAPI is not built with
# LIGHTWEIGHT_SYMTAB. Where the test cannot be built, a stand-in reports it
# as skipped. Where ptrace is refused (some containers), the test skips itself.
if(LIGHTWEIGHT_SYMTAB)
  set(_skip_reason "dyninstAPI is not built with LIGHTWEIGHT_SYMTAB")
elseif(
  NOT
    (
      (DYNINST_HOST_ARCH_X86_64 AND DYNINST_CODEGEN_ARCH_X86_64)
      OR (DYNINST_HOST_ARCH_PPC64LE AND DYNINST_CODEGEN_ARCH_PPC64LE)
      OR (DYNINST_HOST_ARCH_AARCH64 AND DYNINST_CODEGEN_ARCH_AARCH64)
    )
)
  set(_skip_reason "needs a native build (the code generator does not target the host)")
endif()

if(DEFINED _skip_reason)
  add_executable(${_targ} skipped.c)
  target_compile_definitions(${_targ} PRIVATE "SKIP_REASON=\"${_skip_reason}\"")
  add_test(NAME ${_tname} COMMAND ${_targ})
else()
  add_executable(crash_callback_mutatee crash_callback_mutatee.c)

  add_executable(${_targ} crash_callback.cpp)
  target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
  target_link_libraries(${_targ} PRIVATE dyninstAPI)

  add_test(
    NAME ${_tname}
    COMMAND
      ${_targ} $<TARGET_FILE:crash_callback_mutatee> $<TARGET_FILE_DIR:dyninstAPI_RT>
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
