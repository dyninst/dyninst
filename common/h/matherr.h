#ifndef matherr_H
#define matherr_H
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
extern bool matherr_flag;
extern int matherr(struct exception *x);

#endif
