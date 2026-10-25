include_guard(GLOBAL)

# The trap comes from relocating a function shorter than a jump, which the
# mutatee writes in x86_64 assembly. The mutatee is a host program, so Dyninst
# must also generate code for x86_64.
if(NOT DYNINST_OS_Linux OR NOT (DYNINST_HOST_ARCH_X86_64 AND DYNINST_CODEGEN_ARCH_X86_64))
  return()
endif()

# early_trap_ctor's constructor calls into the mutatee. It is a dependency of
# early_trap_mid, so it is loaded after the runtime library and initialized
# before it.
add_library(early_trap_ctor SHARED ctor.c)
add_library(early_trap_mid SHARED mid.c)
target_link_libraries(early_trap_mid PRIVATE early_trap_ctor)

add_executable(early_trap_mutatee mutatee.c)
target_link_libraries(early_trap_mutatee PRIVATE early_trap_mid)
set_target_properties(early_trap_mutatee PROPERTIES ENABLE_EXPORTS ON)

add_executable(rewrite_early_trap rewrite_early_trap.cpp)
target_compile_options(rewrite_early_trap PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(rewrite_early_trap PRIVATE dyninstAPI)
add_dependencies(rewrite_early_trap dyninstAPI_RT)

add_test(
  NAME dyninstAPI_rewrite_early_trap
  COMMAND
    rewrite_early_trap $<TARGET_FILE:early_trap_mutatee>
    ${CMAKE_CURRENT_BINARY_DIR}/rewritten
)
set_tests_properties(
  dyninstAPI_rewrite_early_trap
  PROPERTIES
    LABELS "regression"
    ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
    SKIP_RETURN_CODE 77
)
