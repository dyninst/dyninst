include_guard(GLOBAL)

list(INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/tests/cmake")

string(TOUPPER ${DYNINST_ENABLE_TESTS} _test_flag)

if(_test_flag STREQUAL "REGRESSION" OR _test_flag STREQUAL "ALL")
  add_subdirectory(regression)

  # Implicitly enable integration tests
  set(_test_flag "INTEGRATION")
endif()

if(_test_flag STREQUAL "INTEGRATION")
  add_subdirectory(integration)

  # Implicitly enable unit tests
  set(_test_flag "UNIT")
endif()

if(_test_flag OR _test_flag STREQUAL "UNIT")
  add_subdirectory(unit)
endif()

unset(_test_flag)
