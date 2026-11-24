include_guard(GLOBAL)

dyninst_add_test_executable(
  far_plt_call_mutatee
  SOURCE_FILES mutatee.c
  REQUIRES host:x86_64 native
)
if(TARGET far_plt_call_mutatee)
  # The instrumentation calls into this library, another object than the mutatee.
  add_library(far_plt_call_lib SHARED lib.c)
  target_link_libraries(far_plt_call_mutatee PRIVATE far_plt_call_lib)
endif()

# The test rewrites the mutatee with a call to the library more than 2 GiB from its
# jump slot, runs it and checks its exit status. The rewritten file is over 2 GB; the
# test deletes it.
dyninst_add_test(
  dyninstAPI_far_plt_call
  TYPE regression
  SOURCE_FILES far_plt_call.cpp
  DEPS dyninstAPI
  ARGS $<TARGET_FILE:far_plt_call_mutatee> ${CMAKE_CURRENT_BINARY_DIR}/rewritten
  REQUIRES host:x86_64 native
  PROPERTIES ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
)
if(TARGET dyninstAPI_far_plt_call)
  add_dependencies(dyninstAPI_far_plt_call dyninstAPI_RT)
endif()
