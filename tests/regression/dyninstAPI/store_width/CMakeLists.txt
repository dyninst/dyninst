include_guard(GLOBAL)

dyninst_add_test_executable(
  store_width_mutatee
  SOURCE_FILES mutatee.c
  COMPILE_OPTIONS -O0 -gdwarf-4 -fno-pie
  REQUIRES native
)
if(TARGET store_width_mutatee)
  target_link_options(store_width_mutatee PRIVATE -no-pie)
endif()

dyninst_add_test(
  dyninstAPI_store_width
  TYPE regression
  SOURCE_FILES store_width.cpp
  DEPS dyninstAPI
  ARGS $<TARGET_FILE:store_width_mutatee> ${CMAKE_CURRENT_BINARY_DIR}/rewritten constant
  REQUIRES native
  PROPERTIES ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
)
if(TARGET dyninstAPI_store_width)
  add_dependencies(dyninstAPI_store_width dyninstAPI_RT)
endif()

dyninst_add_test(
  dyninstAPI_store_width_computed
  TYPE regression
  COMMAND
    dyninstAPI_store_width $<TARGET_FILE:store_width_mutatee>
    ${CMAKE_CURRENT_BINARY_DIR}/rewritten_computed computed
  REQUIRES native
  PROPERTIES ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
)
