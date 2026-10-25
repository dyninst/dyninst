include_guard(GLOBAL)

# Rewrites the C library a program is linked with and runs the program with it.
# Both are host binaries, so Dyninst must generate code for the host.
if(
  NOT DYNINST_OS_Linux
  OR
    NOT
      (
        (DYNINST_HOST_ARCH_X86_64 AND DYNINST_CODEGEN_ARCH_X86_64)
        OR (DYNINST_HOST_ARCH_AARCH64 AND DYNINST_CODEGEN_ARCH_AARCH64)
        OR (DYNINST_HOST_ARCH_PPC64LE AND DYNINST_CODEGEN_ARCH_PPC64LE)
      )
)
  return()
endif()

add_executable(rewrite_libc_mutatee mutatee.c)

add_executable(rewrite_libc rewrite_libc.cpp)
target_compile_options(rewrite_libc PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(rewrite_libc PRIVATE dyninstAPI)
add_dependencies(rewrite_libc dyninstAPI_RT)

add_test(
  NAME dyninstAPI_rewrite_libc
  COMMAND
    rewrite_libc $<TARGET_FILE:rewrite_libc_mutatee> ${CMAKE_CURRENT_BINARY_DIR}/rewritten
)
set_tests_properties(
  dyninstAPI_rewrite_libc
  PROPERTIES
    LABELS "regression"
    ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
)
