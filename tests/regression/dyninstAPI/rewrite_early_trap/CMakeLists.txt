include_guard(GLOBAL)

# The trap comes from relocating a function shorter than a jump, which the
# mutatee writes in x86_64 assembly. The mutatee is a host program, so Dyninst
# must also generate code for x86_64.
set(_requires host:x86_64 native)

dyninst_add_test(
  dyninstAPI_rewrite_early_trap
  TYPE regression
  SOURCE_FILES rewrite_early_trap.cpp
  DEPS dyninstAPI
  ARGS $<TARGET_FILE:early_trap_mutatee> ${CMAKE_CURRENT_BINARY_DIR}/rewritten
  REQUIRES ${_requires}
  PROPERTIES ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
)
if(TARGET dyninstAPI_rewrite_early_trap)
  add_dependencies(dyninstAPI_rewrite_early_trap dyninstAPI_RT)

  # early_trap_ctor's constructor calls into the mutatee. It is a dependency of
  # early_trap_mid, so it is loaded after the runtime library and initialized
  # before it.
  add_library(early_trap_ctor SHARED ctor.c)
  add_library(early_trap_mid SHARED mid.c)
  target_link_libraries(early_trap_mid PRIVATE early_trap_ctor)

  dyninst_add_test_executable(
    early_trap_mutatee
    SOURCE_FILES mutatee.c
    DEPS early_trap_mid
  )
  set_target_properties(early_trap_mutatee PROPERTIES ENABLE_EXPORTS ON)
endif()
unset(_requires)
