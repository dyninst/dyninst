include_guard(GLOBAL)

list(INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/tests/cmake")

string(TOUPPER ${DYNINST_ENABLE_TESTS} _test_flag)

# If a TRUE "truthy" value was given, enable unit tests only
if("${_test_flag}")
  set(_test_flag "UNIT")
endif()

if(_test_flag STREQUAL "ALL")
  # Enable tests that need to download the test binary repo
  set(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS ON)

  include(DyninstBinaryTests)

  # Implicitly enable regression tests
  set(_test_flag "REGRESSION")
endif()

if(_test_flag STREQUAL "REGRESSION")
  add_subdirectory(regression)

  # Implicitly enable integration tests
  set(_test_flag "INTEGRATION")
endif()

if(_test_flag STREQUAL "INTEGRATION")
  add_subdirectory(integration)

  # Implicitly enable unit tests
  set(_test_flag "UNIT")
endif()

if(_test_flag STREQUAL "UNIT")
  add_subdirectory(unit)
else()
  message(
    FATAL_ERROR
      "Unknown test type '${DYNINST_ENABLE_TESTS}'. Must be one of ${_dyninst_test_types}"
    )
endif()

unset(_test_flag)

add_library(cft_tests OBJECT cft_tests.cpp cft_tests.h)
target_compile_options(cft_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(cft_tests PRIVATE instructionAPI)
target_include_directories(cft_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(memory_tests OBJECT memory_tests.cpp memory_tests.h)
target_compile_options(memory_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(memory_tests PRIVATE instructionAPI)
target_include_directories(memory_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(register_tests OBJECT register_tests.cpp register_tests.h)
target_compile_options(register_tests PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(register_tests PRIVATE instructionAPI)
target_include_directories(register_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
