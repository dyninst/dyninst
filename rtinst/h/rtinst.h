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

/*
 * $Id: rtinst.h,v 1.46 2001/01/16 22:27:25 schendel Exp $
 * This file contains the extended instrumentation functions that are provided
 *   by the Paradyn run-time instrumentation layer.
 */

#ifndef _RTINST_H
#define _RTINST_H

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"

/* We sometimes include this into assembly files, so guard the struct defs. */
#if !defined(__ASSEMBLER__)

/*typedef void (*instFunc)(void *cdata, int type, char *eventData);*/

/* parameters to an instrumented function */
typedef enum { processTime, wallTime } timerType;

struct sampleIdRec {
    unsigned int id;
    /* formerly an aggregate bit, but that's now obsolete */
};
typedef struct sampleIdRec sampleId;

/* struct endStatsRec in dyninstAPI_RT.h */

struct intCounterRec {
   int64_t value;    /* this field must be first for setValue to work -jkh */
   sampleId id;
  /* seems to be unused, implementing atomic loads and stores for the counters
     should make this method unnecessary anyways - bhs
    unsigned char theSpinner;
    mutex serving 2 purposes:
      (1) so paradynd won't sample while we're in middle of updating and
      (2) so multiple LWPs or threads won't update at the same time */ 
};
typedef struct intCounterRec intCounter;

struct floatCounterRec {
    float value;
    sampleId id;
};
typedef struct floatCounterRec floatCounter;

#ifdef SHM_SAMPLING
struct tTimerRec {
   volatile rawTime64 total;
   volatile rawTime64 start;
   volatile int counter;
   volatile sampleId id; /* can be made obsolete in the near future */
#if defined(MT_THREAD)
   volatile int lwp_id;  /* we need to save the lwp id so paradynd can sync */
   volatile int in_inferiorRPC; /* flag to avoid time going backwards - naim */
   volatile struct tTimerRec  *vtimer; /* position in the threadTable */
#endif

   /* the following 2 vrbles are used to implement consistent sampling.
      Updating by rtinst works as follows: bump protector1, do action, then
      bump protector2.  Shared-memory sampling by paradynd works as follows:
      read protector2, read the 3 vrbles above, read protector1.  If
      the 2 protector values differ then try again, else the sample got
      a good snapshot.  Don't forget to be sure paradynd reads the protector
      vrbles in the _opposite_ order that rtinst writes them!!! */
   volatile int protector1;
   volatile int protector2;
};
#else
struct tTimerRec {
    volatile int 	counter;	/* must be 0 to start; 1 to stop */
    volatile time64	total;
    volatile time64	start;
    volatile time64     lastValue;      /* to check for rollback */
    volatile time64	snapShot;	/* used to get consistant value 
					   during st/stp */
    volatile int	normalize;	/* value to divide total by to 
					   get seconds */
                                        /* always seems to be MILLION; 
                                           can we get rid of this? --ari */
    volatile timerType 	type;
    volatile sampleId 	id;
    volatile char mutex;
    /*volatile char sampled;*/
};
#endif /* SHM_SAMPLING */
typedef struct tTimerRec tTimer;

typedef int traceStream;

void DYNINSTgenerateTraceRecord(traceStream sid, short type, 
			        short length,
                                void *eventData, int flush,
			        rawTime64 wall_time, rawTime64 process_time);
/* see comments in rtinst.C for description of the following */
#define UNASSIGNED_TIMER_LEVEL 0
#define HARDWARE_TIMER_LEVEL 1
#define SOFTWARE_TIMER_LEVEL 2
extern int hintBestCpuTimerLevel;
extern int hintBestWallTimerLevel;

typedef rawTime64 (*timeQueryFuncPtr_t)();
extern timeQueryFuncPtr_t pDYNINSTgetCPUtime;
extern timeQueryFuncPtr_t pDYNINSTgetWalltime;

/* Do not call these directly, but access through the higher level time
   retrieval functions DYNINSTgetCPUtime and DYNINSTgetWalltime. */
extern rawTime64 DYNINSTgetCPUtime_sw(void);
extern rawTime64 DYNINSTgetWalltime_sw(void);

/* The time retrieval functions - implemented as macros to increase
   performance.  These will call the correct software or hardware level time
   retrieval function.  Return type is rawTime64. */
#define DYNINSTgetCPUtime()   (*pDYNINSTgetCPUtime)()
#define DYNINSTgetWalltime() (*pDYNINSTgetWalltime)()

#if defined(MT_THREAD)
extern rawTime64 DYNINSTgetCPUtime_LWP(int lwp_id);
#include "rtinst/src/RTthread.h"
#endif

#endif /*!defined(__ASSEMBLER__)*/

#endif
