include_guard(GLOBAL)

# dyninstAPI is not built with LIGHTWEIGHT_SYMTAB
if(LIGHTWEIGHT_SYMTAB)
  return()
endif()

# The test creates a live mutatee, so it needs a native build: the code
# generator must target the host. Where ptrace is refused (some containers),
# the test is skipped.
if(
  (DYNINST_HOST_ARCH_X86_64 AND DYNINST_CODEGEN_ARCH_X86_64)
  OR (DYNINST_HOST_ARCH_PPC64LE AND DYNINST_CODEGEN_ARCH_PPC64LE)
  OR (DYNINST_HOST_ARCH_AARCH64 AND DYNINST_CODEGEN_ARCH_AARCH64)
)
  add_executable(crash_callback_mutatee crash_callback_mutatee.c)

  set(_targ crash_callback)
  add_executable(${_targ} crash_callback.cpp)
  target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
  target_link_libraries(${_targ} PRIVATE dyninstAPI)

  set(_tname proccontrol_${_targ}_test)
  add_test(
    NAME ${_tname}
    COMMAND
      ${_targ} $<TARGET_FILE:crash_callback_mutatee> $<TARGET_FILE_DIR:dyninstAPI_RT>
  )
  set_tests_properties(
    ${_tname}
    PROPERTIES
      LABELS "regression"
      TIMEOUT 120
      SKIP_RETURN_CODE 77
      ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
  )
endif()
