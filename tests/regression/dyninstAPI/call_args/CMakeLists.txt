include_guard(GLOBAL)

dyninst_add_test_executable(call_args_mutatee SOURCE_FILES mutatee.c)
if(TARGET call_args_mutatee)
  # The rewritten mutatee calls check through its dynamic symbol table.
  set_target_properties(call_args_mutatee PROPERTIES ENABLE_EXPORTS ON)
endif()

# The test runs the rewritten mutatee, so the generated code must be for the
# host.
dyninst_add_test(
  dyninstAPI_call_args
  TYPE regression
  SOURCE_FILES call_args.cpp
  DEPS dyninstAPI
  ARGS $<TARGET_FILE:call_args_mutatee> ${CMAKE_CURRENT_BINARY_DIR}/rewritten
  REQUIRES dynamic native
  PROPERTIES ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
)
if(TARGET dyninstAPI_call_args)
  add_dependencies(dyninstAPI_call_args dyninstAPI_RT)
endif()
