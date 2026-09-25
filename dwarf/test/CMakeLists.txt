include(GoogleTest)

# OBJECT library so CMake's native ASM support (toolchain flags, dependency
# tracking) builds the hand-written DWARF fixture instead of a bespoke
# add_custom_command.
add_library(dwarf_test_fixtures OBJECT fixtures/subrange_bounds.s)

add_executable(test_dwarf_subrange test_dwarf_subrange.C)
add_dependencies(test_dwarf_subrange dwarf_test_fixtures)

# White-box test seam: dwarf_subrange_bounds() isn't in dynDwarf's public
# headers, only DYNDWARF_EXPORT'd for DwarfWalker.
target_include_directories(test_dwarf_subrange PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_compile_definitions(
  test_dwarf_subrange
  PRIVATE FIXTURE_OBJECT_PATH="$<TARGET_OBJECTS:dwarf_test_fixtures>")
target_link_libraries(test_dwarf_subrange PRIVATE dynDwarf dynElf Dyninst::ElfUtils
                                                   GTest::gtest GTest::gtest_main)

# gtest_discover_tests executes the test binary at build time to enumerate
# cases, which assumes a native (non-cross-compiled) build.
set(DYNINST_TEST_TIMEOUT_SECONDS
    60
    CACHE STRING "Per-test timeout (seconds) used by gtest_discover_tests for Dyninst unit tests")
gtest_discover_tests(test_dwarf_subrange PROPERTIES TIMEOUT ${DYNINST_TEST_TIMEOUT_SECONDS})
