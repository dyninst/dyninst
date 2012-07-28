#ifdef __cplusplus
extern "C" {
#endif
#include "../src/mutatee_call_info.h"

extern int test_lookup_func_mutatee();
extern int test_lookup_var_mutatee();

mutatee_call_info_t mutatee_funcs[] = {
  {"test_lookup_func", test_lookup_func_mutatee, GROUPED, "test_lookup_func"},
  {"test_lookup_var", test_lookup_var_mutatee, GROUPED, "test_lookup_var"}
};

int max_tests = 2;
int runTest[2];
int passedTest[2];
#ifdef __cplusplus
}
#endif
