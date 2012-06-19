#include "mutatee_util.h"

int test_modification_var = 0;


int test_modification(int foo) {
   // For great justice, we want a single block
   // so we can split it and put in new code. 
   int bar;
   bar = foo * 3;
   return bar + 10;
}

int test_modification_aux() {
   // Gimme code to set the global to 42
   test_modification_var = 42;
   return test_modification_var;
}

int test_modification_mutatee() {
   /* No execution necessary... yet */
   test_modification(8);
   fprintf(stderr, "After test_modification: test_modification_var is %d\n", test_modification_var);
   if (test_modification_var == 42) {
      test_passes(testname);
      return 0;
   }
   return 1;
}
