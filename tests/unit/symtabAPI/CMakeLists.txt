include_guard(GLOBAL)

# address_lookup traces a child with ptrace; its namespace test also puts one in its
# own user and mount namespace.
# It is compiled with the library's platform macros (DYNINST_PLATFORM_CAPABILITIES), so
# os_linux is defined as in the library.
dyninst_add_test_executable(
  symtabAPI_address_lookup
  SOURCE_FILES address_lookup.cpp
  DEPS symtabAPI
  DEFINES ${DYNINST_PLATFORM_CAPABILITIES}
)
dyninst_add_test(
  symtabAPI_address_lookup_plain
  TYPE unit
  COMMAND symtabAPI_address_lookup plain
  REQUIRES dynamic
)
dyninst_add_test(
  symtabAPI_address_lookup_namespace
  TYPE unit
  COMMAND symtabAPI_address_lookup namespace
  REQUIRES dynamic userns
)

dyninst_add_test(
  symtabAPI_add_region
  TYPE unit
  SOURCE_FILES add_region.cpp
  DEPS symtabAPI
)
