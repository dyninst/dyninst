include(GoogleTest)

# fixtures/subrange_bounds.s is hand-written DWARF; it is never linked or
# executed, only assembled and then read directly with libdw at test time.
set(_fixture_src ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/subrange_bounds.s)
set(_fixture_obj ${CMAKE_CURRENT_BINARY_DIR}/subrange_bounds.o)

add_custom_command(
  OUTPUT ${_fixture_obj}
  COMMAND ${CMAKE_ASM_COMPILER} -c ${_fixture_src} -o ${_fixture_obj}
  DEPENDS ${_fixture_src}
  COMMENT "Assembling DWARF test fixture subrange_bounds.o")
add_custom_target(dwarf_test_fixtures DEPENDS ${_fixture_obj})

add_executable(test_dwarf_subrange test_dwarf_subrange.C)
add_dependencies(test_dwarf_subrange dwarf_test_fixtures)
target_include_directories(test_dwarf_subrange PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_compile_definitions(test_dwarf_subrange
                            PRIVATE FIXTURE_OBJECT_PATH="${_fixture_obj}")
target_link_libraries(test_dwarf_subrange PRIVATE dynDwarf Dyninst::ElfUtils
                                                   GTest::gtest GTest::gtest_main)
gtest_discover_tests(test_dwarf_subrange PROPERTIES TIMEOUT 60)
