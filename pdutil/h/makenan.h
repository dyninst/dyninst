#ifndef makenan_H
#define makenan_H

#include <math.h>
#include <assert.h>

extern float f_paradyn_nan;
extern bool nan_created;
extern bool matherr_flag;

// There is no standard macro to create a NaN valued float
extern int matherr(struct exception *x);
extern float make_Nan();
#define PARADYN_NaN make_Nan()

#endif
