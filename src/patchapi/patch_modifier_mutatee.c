#include "mutatee_util.h"

int patch_modifier_var = 0;


int patchMod_split(int foo) {
   int bar;
   /* We want a single entry block */
   bar = foo * 3;
   return bar + 10;
}

int patchMod_redirect(int foo) {
   int bar;
   /* We want a single entry block that ends in a conditional */
   if (foo) {
      bar = 1;
   }
   else {
      bar = 2;
   }
   return bar;
}

int patchMod_insert(int foo) {
   int bar;
   /* We want a single entry block */
   bar = foo * 3;
   return bar + 10;
}


int patch_modifier_mutatee() {
   /* No execution necessary... yet */
   patch_modifier_var = patchMod_split(patch_modifier_var);
   patch_modifier_var = patchMod_redirect(patch_modifier_var);
   patch_modifier_var = patchMod_insert(patch_modifier_var);
   test_passes(testname);
   return 0;
}
