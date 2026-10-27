include_guard(GLOBAL)

add_executable(vdsoSymtab vdsoSymtab.cpp)
target_compile_options(vdsoSymtab PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(vdsoSymtab PRIVATE stackwalk symtabAPI)
if(DYNINST_ENABLE_DYNAMIC_TESTS)
  target_compile_definitions(vdsoSymtab PRIVATE DYNINST_ENABLE_DYNAMIC_TESTS=1)
else()
  target_compile_definitions(vdsoSymtab PRIVATE DYNINST_ENABLE_DYNAMIC_TESTS=0)
endif()

add_test(NAME stackwalk_vdso_symtab COMMAND vdsoSymtab)
set_tests_properties(
  stackwalk_vdso_symtab
  PROPERTIES LABELS "regression" SKIP_RETURN_CODE 77 SKIP_REGULAR_EXPRESSION "skipping:"
)
