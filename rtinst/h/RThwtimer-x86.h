/*
 * Copyright (c) 1996-2001 Barton P. Miller
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
/*
 * $Id: RThwtimer-x86.h,v 1.2 2001/01/05 16:34:35 pcroth Exp $
 */
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
