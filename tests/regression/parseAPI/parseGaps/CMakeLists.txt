include_guard(GLOBAL)

if(NOT DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)
  return()
endif()

add_executable(parseGaps parseGaps.cpp)
target_compile_options(parseGaps PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(parseGaps PRIVATE parseAPI)

add_test(NAME parseAPI_parseGaps_test
         COMMAND parseGaps
                 "${DYNINST_TEST_BINARIES_DIR}/parseAPI/parseGaps/aarch64_el0_stripped")
set_tests_properties(parseAPI_parseGaps_test PROPERTIES LABELS "regression")
