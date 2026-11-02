include_guard(GLOBAL)

# The test creates a live mutatee.
dyninst_add_test_executable(
  crash_callback_mutatee
  SOURCE_FILES crash_callback_mutatee.c
  REQUIRES native dynamic
)

dyninst_add_test(
  proccontrol_crash_callback_test
  TYPE regression
  SOURCE_FILES crash_callback.cpp
  DEPS dyninstAPI
  ARGS $<TARGET_FILE:crash_callback_mutatee> $<TARGET_FILE_DIR:dyninstAPI_RT>
  REQUIRES native dynamic
  PROPERTIES TIMEOUT 120 ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
)
