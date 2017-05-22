#ifdef __cplusplus
extern "C" {
#endif
#include "../src/mutatee_call_info.h"

extern int test_type_info_mutatee();
extern int test_anno_basic_types_mutatee();
extern int test_module_mutatee();
extern int test_lookup_func_mutatee();
extern int test_lookup_var_mutatee();
extern int test_line_info_mutatee();
extern int test_relocations_mutatee();
extern int test_add_symbols_mutatee();
extern int test_symtab_ser_funcs_mutatee();
extern int test_ser_anno_mutatee();

mutatee_call_info_t mutatee_funcs[] = {
  {"test_type_info", test_type_info_mutatee, GROUPED, "test_type_info"},
  {"test_anno_basic_types", test_anno_basic_types_mutatee, GROUPED, "test_anno_basic_types"},
  {"test_module", test_module_mutatee, GROUPED, "test_module"},
  {"test_lookup_func", test_lookup_func_mutatee, GROUPED, "test_lookup_func"},
  {"test_lookup_var", test_lookup_var_mutatee, GROUPED, "test_lookup_var"},
  {"test_line_info", test_line_info_mutatee, GROUPED, "test_line_info"},
  {"test_relocations", test_relocations_mutatee, GROUPED, "test_relocations"},
  {"test_add_symbols", test_add_symbols_mutatee, GROUPED, "test_add_symbols"},
  {"test_symtab_ser_funcs", test_symtab_ser_funcs_mutatee, GROUPED, "test_symtab_ser_funcs"},
  {"test_ser_anno", test_ser_anno_mutatee, GROUPED, "test_ser_anno"}
};

int max_tests = 10;
int runTest[10];
int passedTest[10];
#ifdef __cplusplus
}
#endif
