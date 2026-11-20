include_guard(GLOBAL)

# The mutatee reads and writes FPCR in aarch64 assembly.
dyninst_add_test_executable(
  tramp_spr_mutatee
  SOURCE_FILES mutatee.c
  REQUIRES host:aarch64
)
if(TARGET tramp_spr_mutatee)
  # The rewritten mutatee calls record through its dynamic symbol table.
  set_target_properties(tramp_spr_mutatee PROPERTIES ENABLE_EXPORTS ON)
endif()

# The test runs the rewritten mutatee and checks aarch64 base tramps.
dyninst_add_test(
  dyninstAPI_aarch64_tramp_spr
  TYPE regression
  SOURCE_FILES tramp_spr.cpp
  DEPS dyninstAPI
  ARGS $<TARGET_FILE:tramp_spr_mutatee> ${CMAKE_CURRENT_BINARY_DIR}/rewritten
  REQUIRES dynamic host:aarch64 native
  PROPERTIES ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
)
if(TARGET dyninstAPI_aarch64_tramp_spr)
  add_dependencies(dyninstAPI_aarch64_tramp_spr dyninstAPI_RT)
endif()
