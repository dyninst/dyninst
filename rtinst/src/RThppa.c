/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */





/************************************************************************
 * RThppa.c: timer functions for hpux
************************************************************************/





/************************************************************************
 * header files.
************************************************************************/

#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>

#include "kludges.h"
#include "rtinst/h/rtinst.h"

#ifndef hppa1_1_hp_hpux
#error This file should be compiled for HP only
#endif

#include <sys/pstat.h>
#include <sys/unistd.h>
#define _PROTOTYPES




/************************************************************************
 * void DYNINSTos_init(void)
 *
 * null initialization for generic operating system.
************************************************************************/

void
DYNINSTos_init(int,int) {
}





/************************************************************************
 * time64 DYNINSTgetCPUtime(void)
 *
 * get the total CPU time used for "an" LWP of the monitored process.
 * this functions needs to be rewritten if a per-thread CPU time is
 * required.  time for a specific LWP can be obtained via the "/proc"
 * filesystem.
 * return value is in usec units.
************************************************************************/

/* 
 * The compilng warning could be eliminated if we use cc instead
 * of gcc. Change it if you think that's good. --ling
 *
 * The cpu time I got in this way is not good. Absolutely 
 * needs improvement 
 */

time64
DYNINSTgetCPUtime(void) {
    time64 now;
    static time64 previous=0;
    struct pst_status pst;
    int target = (int)getpid();

  try_again:
    if (pstat_getproc(&pst, sizeof(pst), (size_t)0, target) != -1) {
      now = (time64)pst.pst_cptickstotal + (time64)pst.pst_cpticks;
      now *= (time64)10000;
      if (now<previous) {
        goto try_again;
      }
      previous=now;
      return(now);
    }
    else {
      perror("pstat_getproc");
      abort();
  }
}





/************************************************************************
 * time64 DYNINSTgetWalltime(void)
 *
 * get the total walltime used by the monitored process.
 * return value is in usec units.
************************************************************************/

time64
DYNINSTgetWalltime(void) {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == -1) {
        perror("gettimeofday");
        abort();
    }
    return ((time64)tv.tv_sec*(time64)1000000 + (time64)tv.tv_usec);
}



/************************************************************************
 *
 *  StartProcessTimer, StartWallTimer, StopProcessTimer,
 *  StopWallTimer to be called from the miniTrampoline.
 *
************************************************************************/


extern DYNINSTstartProcessTimer(tTimer *timer);
extern DYNINSTstartWallTimer(tTimer* timer);
extern DYNINSTstopProcessTimer(tTimer* timer);
extern DYNINSTstopWallTimer(tTimer* timer);

void
DYNINSTstartProcessTimer_hpux(tTimer* timer) {
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

    DYNINSTstartProcessTimer(timer); 
}




void
DYNINSTstartWallTimer_hpux(tTimer* timer) {
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

    DYNINSTstartWallTimer(timer);
}


void
DYNINSTstopProcessTimer_hpux(tTimer* timer) {
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

    DYNINSTstopProcessTimer(timer);
}



void
DYNINSTstopWallTimer_hpux(tTimer* timer) {
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

    DYNINSTstopWallTimer(timer);
}
