include_guard(GLOBAL)

# The test program defines a function with the version DYNINST_TEST_1.0, set by
# a linker version script, and exports it, so its input does not depend on the
# versions of the host's libc.
dyninst_add_test(
  symtab_symbol_versions
  TYPE regression
  SOURCE_FILES check_versions.cpp
  DEPS symtabAPI
)
target_link_options(
  symtab_symbol_versions
  PRIVATE
    "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/versioned.map"
    "-Wl,--export-dynamic"
)
set_target_properties(
  symtab_symbol_versions
  PROPERTIES LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/versioned.map"
)
