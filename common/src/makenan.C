#include "util/h/makenan.h"
#include "util/h/headers.h"

float f_paradyn_nan = 0.0;
bool nan_created = false;
bool matherr_flag = false;

float make_Nan() {
    if(!nan_created){
    matherr_flag = true;
    double temp = -3.0;
    f_paradyn_nan = (float)sqrt(temp);
    matherr_flag = false;
    nan_created = true;
    }
    assert(isnan(f_paradyn_nan));
    return f_paradyn_nan;
}


int matherr(struct exception *x) {
  if ((x->type == DOMAIN) && !P_strcmp(x->name, "sqrt")) {
      if (matherr_flag)
	    return(1);
  }
  return(0);
}


