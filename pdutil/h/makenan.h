#ifndef makenan_H
#define makenan_H

#include <math.h>
#include <assert.h>

// There is no standard macro to create a NaN valued float
static float f_paradyn_nan = 0;
static bool nan_created = false;
extern bool matherr_flag;

inline float make_Nan() {
    if(!nan_created){
	matherr_flag = true;
        f_paradyn_nan = sqrt(-3); 
	matherr_flag = false;
	nan_created = true;
    }
    assert(isnan(f_paradyn_nan));
    return f_paradyn_nan;
}
#define PARADYN_NaN make_Nan()

#endif
