include_guard(GLOBAL)

# The test rewrites the mutatee and runs the rewritten copy.
dyninst_add_test_executable(
  stringConstant_mutatee
  SOURCE_FILES stringConstant_mutatee.c
  REQUIRES native
)

dyninst_add_test(
  dyninstAPI_stringConstant_test
  TYPE regression
  SOURCE_FILES stringConstant.cpp
  DEPS dyninstAPI
  ARGS $<TARGET_FILE:stringConstant_mutatee> $<TARGET_FILE_DIR:dyninstAPI_RT>
  REQUIRES native
  PROPERTIES TIMEOUT 120 ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
)
