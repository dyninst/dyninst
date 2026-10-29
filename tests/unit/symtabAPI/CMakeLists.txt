include_guard(GLOBAL)

dyninst_add_test(
  symtabAPI_address_lookup
  TYPE unit
  SOURCE_FILES address_lookup.cpp
  DEPS symtabAPI
  PROPERTIES SKIP_REGULAR_EXPRESSION "skipping:"
)

dyninst_add_test(
  symtabAPI_add_region
  TYPE unit
  SOURCE_FILES add_region.cpp
  DEPS symtabAPI
)
