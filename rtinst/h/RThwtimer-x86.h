
#ifndef __RTHWTIMER_X86__
#define __RTHWTIMER_X86__

#include "common/h/Types.h"

#ifdef HRTIME
#include "hrtime.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
  int isTSCAvail(void);
  rawTime64 getTSC(void);
#ifdef HRTIME
  int isLibhrtimeAvail(struct hrtime_struct **hr_cpu_link, int pid);
  rawTime64 hrtimeGetVtime(struct hrtime_struct *hr_cpu_link);
#endif
#ifdef __cplusplus
}
#endif

typedef union {
  rawTime64 t;
  unsigned long p[2];
} hrtime_union;

#define rdtsc(low,high) \
     __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))


extern inline rawTime64 getTSC(void) {
  volatile hrtime_union val;
  rdtsc(val.p[0], val.p[1]);
  return val.t;
}

#ifdef HRTIME
extern inline rawTime64 hrtimeGetVtime(struct hrtime_struct *hr_cpu_link) {
  hrtime_t current;
  get_hrvtime(hr_cpu_link, &current);
  return (rawTime64)(current);
}
#endif

#endif
