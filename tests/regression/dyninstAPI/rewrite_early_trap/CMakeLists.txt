include_guard(GLOBAL)

set(_test dyninstAPI_rewrite_early_trap)

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

add_executable(rewrite_early_trap rewrite_early_trap.cpp)
target_compile_options(rewrite_early_trap PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(rewrite_early_trap PRIVATE dyninstAPI)
add_dependencies(rewrite_early_trap dyninstAPI_RT)

# The trap comes from relocating a function shorter than a jump, which the
# mutatee writes in x86_64 assembly. The mutatee is a host program, so Dyninst
# must also generate code for x86_64.
if(DYNINST_OS_Linux AND DYNINST_HOST_ARCH_X86_64 AND DYNINST_CODEGEN_ARCH_X86_64)
  # early_trap_ctor's constructor calls into the mutatee. It is a dependency of
  # early_trap_mid, so it is loaded after the runtime library and initialized
  # before it.
  add_library(early_trap_ctor SHARED ctor.c)
  add_library(early_trap_mid SHARED mid.c)
  target_link_libraries(early_trap_mid PRIVATE early_trap_ctor)

  add_executable(early_trap_mutatee mutatee.c)
  target_link_libraries(early_trap_mutatee PRIVATE early_trap_mid)
  set_target_properties(early_trap_mutatee PROPERTIES ENABLE_EXPORTS ON)

  set(_args $<TARGET_FILE:early_trap_mutatee> ${CMAKE_CURRENT_BINARY_DIR}/rewritten)
else()
  set(
    _args
    --skip
    "the mutatee is x86_64 code and needs an x86_64 host with x86_64 code generation"
  )
endif()

add_test(NAME ${_test} COMMAND rewrite_early_trap ${_args})
set_tests_properties(
  ${_test}
  PROPERTIES
    LABELS "regression"
    ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
    SKIP_RETURN_CODE 77
    SKIP_REGULAR_EXPRESSION "skipping:"
)
