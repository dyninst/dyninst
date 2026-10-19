include_guard(GLOBAL)

# symtabAPI is not built with LIGHTWEIGHT_SYMTAB
if(LIGHTWEIGHT_SYMTAB)
  return()
endif()

add_executable(address_lookup address_lookup.cpp)
target_compile_options(address_lookup PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(address_lookup PRIVATE symtabAPI)
add_test(NAME symtabAPI_address_lookup COMMAND address_lookup)
set_tests_properties(symtabAPI_address_lookup
                     PROPERTIES LABELS "unit" SKIP_REGULAR_EXPRESSION "skipping:")
