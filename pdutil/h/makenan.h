#ifndef makenan_H
#define makenan_H

#include <math.h>
#include <assert.h>

inline float make_Nan() {
    float  nan_temp = sqrt(-3); 
    assert(isnan(nan_temp));
    return nan_temp;
}
#define PARADYN_NaN make_Nan();

#endif
