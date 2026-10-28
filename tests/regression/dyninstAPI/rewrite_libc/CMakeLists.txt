include_guard(GLOBAL)

set(_test dyninstAPI_rewrite_libc)

if(LIGHTWEIGHT_SYMTAB)
  # dyninstAPI is not built with LIGHTWEIGHT_SYMTAB, so the test cannot be built.
  add_test(
    NAME ${_test}
    COMMAND
      sh -c "echo 'skipping: dyninstAPI is not built with LIGHTWEIGHT_SYMTAB'; exit 77"
  )
  set_tests_properties(
    ${_test}
    PROPERTIES LABELS "regression" SKIP_RETURN_CODE 77 SKIP_REGULAR_EXPRESSION "skipping:"
  )
  return()
endif()

add_executable(rewrite_libc_mutatee mutatee.c)

add_executable(rewrite_libc rewrite_libc.cpp)
target_compile_options(rewrite_libc PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(rewrite_libc PRIVATE dyninstAPI)
add_dependencies(rewrite_libc dyninstAPI_RT)

# Rewrites the C library a program is linked with and runs the program with it.
# Both are host binaries, so Dyninst must generate code for the host.
if(
  DYNINST_OS_Linux
  AND
    (
      (DYNINST_HOST_ARCH_X86_64 AND DYNINST_CODEGEN_ARCH_X86_64)
      OR (DYNINST_HOST_ARCH_AARCH64 AND DYNINST_CODEGEN_ARCH_AARCH64)
      OR (DYNINST_HOST_ARCH_PPC64LE AND DYNINST_CODEGEN_ARCH_PPC64LE)
    )
)
  set(_args $<TARGET_FILE:rewrite_libc_mutatee> ${CMAKE_CURRENT_BINARY_DIR}/rewritten)
else()
  set(
    _args
    --skip
    "Dyninst does not generate code for this host, which runs the rewritten C library"
  )
endif()

add_test(NAME ${_test} COMMAND rewrite_libc ${_args})
set_tests_properties(
  ${_test}
  PROPERTIES
    LABELS "regression"
    ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
    SKIP_RETURN_CODE 77
    SKIP_REGULAR_EXPRESSION "skipping:"
)
