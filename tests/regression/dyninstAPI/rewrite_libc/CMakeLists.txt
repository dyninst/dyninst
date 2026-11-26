include_guard(GLOBAL)

# Rewrites the C library a program is linked with and runs the program with it.
# Both are host binaries, so Dyninst must generate code for the host.
set(_requires host:x86_64,aarch64,ppc64le native)

dyninst_add_test_executable(
  rewrite_libc_mutatee
  SOURCE_FILES mutatee.c
  REQUIRES ${_requires}
)

dyninst_add_test(
  dyninstAPI_rewrite_libc
  TYPE regression
  SOURCE_FILES rewrite_libc.cpp
  DEPS dyninstAPI
  ARGS $<TARGET_FILE:rewrite_libc_mutatee> ${CMAKE_CURRENT_BINARY_DIR}/rewritten
  REQUIRES ${_requires}
  PROPERTIES ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
)
if(TARGET dyninstAPI_rewrite_libc)
  add_dependencies(dyninstAPI_rewrite_libc dyninstAPI_RT)
endif()
unset(_requires)
