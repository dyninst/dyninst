#include <math.h>

//
// matherr is a user defined exception handler for the math library.
// if it returns 0, the matherr library do something about the arithmetic
// exception. If it returns 1, it does not. We use this function here
// to avoid the error messages produced by sqrt(-3). It is only turned on
// just before the call to sqrt(-3), and it is inmediately turned off
// afterwards.
// NOTE: The file "matherr.h" should be included before "main", otherwise
// the customized matherr would not get called.
//

bool matherr_flag=false;

int matherr(struct exception *x) {
  if ((x->type == DOMAIN) && !strcmp(x->name, "sqrt")) {
    if (matherr_flag)
      return(1);
  } 
  return(0);
}

