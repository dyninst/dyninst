#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include "rtinst/h/RThwtimer-x86.h"

void int_handler(int errno);

static volatile sig_atomic_t jumpok = 0;
static sigjmp_buf jmpbuf;

void int_handler(int errno)  {
  /* removes warning */  errno = errno;
  if(jumpok == 0)  return;
  siglongjmp(jmpbuf, 1);
}

// this matches the function in RThwtimer-x86.h, used for non-optimizing
// rtinst library in which case the extern inline function is ignored 
rawTime64 getTSC(void) {
  volatile hrtime_union val;
  rdtsc(val.p[0], val.p[1]);
  return val.t;
}

int isTSCAvail(void)  {
  struct sigaction act, oldact;
  rawTime64 v;
  int retVal=0;
  act.sa_handler = int_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  if(sigaction(SIGSEGV, &act, &oldact) < 0)  {
    perror("isTSCAvail: Error setting up SIGSEGV handler");
    exit(1);
   }

  if(sigsetjmp(jmpbuf, 1)==0)  {
    jumpok = 1;
    v = getTSC();
    retVal=1;
  } else {
    retVal=0;
  }
  if(sigaction(SIGSEGV, &oldact, NULL) < 0) {
    perror("isTSCAvail: Error resetting the SIGSEGV handler");
    exit(1);
  }
  return retVal;
}

#ifdef HRTIME
#include <unistd.h>

int isLibhrtimeAvail(struct hrtime_struct **hr_cpu_link, int pid) {
  int error;
  
  if(! isTSCAvail()) {
    return 0;
  }
  
  error = hrtime_init();
  if (error < 0) {
    return 0;
  }

  error = get_hrtime_struct(pid, hr_cpu_link);
  if (error < 0) {
    return 0;
  }
  return 1;
}

// this matches the function in RThwtimer-x86.h, used for non-optimizing
// rtinst library in which case the extern inline function is ignored 
rawTime64 hrtimeGetVtime(struct hrtime_struct *hr_cpu_link) {
  hrtime_t current;
  get_hrvtime(hr_cpu_link, &current);
  return (rawTime64)(current);
}

#endif

