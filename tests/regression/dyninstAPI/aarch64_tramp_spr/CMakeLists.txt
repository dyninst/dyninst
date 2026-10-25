include_guard(GLOBAL)

# The mutatee reads and writes FPCR, and the mutator checks aarch64 base
# tramps, so both the host and the code Dyninst generates must be aarch64.
if(
  NOT DYNINST_OS_Linux
  OR NOT DYNINST_HOST_ARCH_AARCH64
  OR NOT DYNINST_CODEGEN_ARCH_AARCH64
)
  return()
endif()

add_executable(tramp_spr_mutatee mutatee.c)
# The rewritten mutatee calls record through its dynamic symbol table.
set_target_properties(tramp_spr_mutatee PROPERTIES ENABLE_EXPORTS ON)

add_executable(tramp_spr tramp_spr.cpp)
target_compile_options(tramp_spr PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(tramp_spr PRIVATE dyninstAPI)
add_dependencies(tramp_spr dyninstAPI_RT)

add_test(
  NAME dyninstAPI_aarch64_tramp_spr
  COMMAND tramp_spr $<TARGET_FILE:tramp_spr_mutatee> ${CMAKE_CURRENT_BINARY_DIR}/rewritten
)
set_tests_properties(
  dyninstAPI_aarch64_tramp_spr
  PROPERTIES
    LABELS "regression"
    ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
)
