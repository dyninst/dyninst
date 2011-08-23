#include <stdio.h>
#include <assert.h>

int orig_wrapped(int);

int wrapper(int i) {
   fprintf(stderr, "Wrapper entry: arg %d\n", i);
   i = orig_wrapped(i+1);
   fprintf(stderr, "Wrapper exit: arg %d\n", i);
   return i+1;
}
