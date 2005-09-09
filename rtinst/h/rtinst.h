/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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
 * $Id: rtinst.h,v 1.63 2005/09/09 18:05:34 legendre Exp $
 * This file contains the extended instrumentation functions that are provided
 *   by the Paradyn run-time instrumentation layer.
 */

#ifndef _RTINST_H
#define _RTINST_H


/* The rawTime64 type represents time in a native or raw time unit, as in the
   time samples taken from the rtinst library.  We're using this typedef'd
   name in order to differentiate a time sample in a "native" time unit
   (eg. cycles) as opposed to one in our common time unit as in the time
   classes, such as timeStamp and timeLength.

   I thought about whether the rawTime64 type should be signed or unsigned:

   Currently the rawTime64 is a signed 64 bit which leads to a max value less
   than 2^63.  However, this value could be greater than 2^63.  For example,
   the TSC register is a signed 64 bit integer.  So when the TSC reaches
   2^63, we are no longer correctly representing the number.

   The natural solution would be to change the rawTime64 type to an unsigned
   64 bit integer.  The problem with this is that we want our time types to
   represent signed 64 bit integers in order to represent negative times (as
   in time differences and time sent to the front-end being based from some
   hardcoded time, which could be set in the future compared to the run time
   of Paradyn).  Well, one might argue, we can still make it work because our
   time samples are just delta's as determined when the daemon first grabs
   the time samples out of the shm variables and these will never even get
   close to 2^63.  However, there are other places where we are feeding
   rawTime64 samples directly into the time types as a sort of current time
   of a sample from the rtinst library.  These occurrences are for recording
   the current time when a resource is created or for determining the first
   sample time.  So if we'd change the rawTime64 type to unsigned, we'd be
   trying to feed these unsigned 64 bit values into the time classes which
   are signed 64 bit.

   Here is what I propose.  We'll keep the rawTime64 type as a signed 64 bit
   value and we'll just require that this never reaches 2^63.  If it does
   we'll reset the tick value, which seems for the TSC seems to be able to be
   done by resetting the machine.  This shouldn't be all that common.  For
   example, it would take a 3GHz machine 97 years to count up from 0 to 2^63.
*/

#include "common/h/Types.h" 

typedef int64_t rawTime64;

struct intCounterRec {
   int64_t value;  /* this field must be first for setValue to work -jkh */
};
typedef struct intCounterRec intCounter;

struct floatCounterRec {
    float value;
};
typedef struct floatCounterRec floatCounter;

struct tHwCounterRec {
   int64_t value;
   int hwevent;
};
typedef struct tHwCounterRec tHwCounter;

struct tHwTimerRec {
    volatile rawTime64 total;
    volatile rawTime64 start;
    volatile int counter;
    unsigned pos; /* Unused for ST */
    volatile int hwevent;
    
    volatile int protector1;
    volatile int protector2;
};
typedef struct tHwTimerRec tHwTimer;

typedef struct virtualTimerRec {
   volatile rawTime64 total;
   volatile rawTime64 start;
   volatile int counter;
   unsigned lwp; /* lwp this timer belongs to */
   unsigned rt_fd; /* File descriptor, RT-side */
   
   volatile int protector1;
   volatile int protector2;
   
   rawTime64 rt_previous; /* Previous value for RT lib */
   /* Daemon-specific data is in the dyn_lwp object */
   unsigned pad[5];
} virtualTimer;

typedef struct tTimerRec {
  volatile rawTime64 total;
  volatile rawTime64 start;
  volatile int counter;
  unsigned index; /* Unused for ST, but makes the size a power of 2 */

   /* the following 2 vrbles are used to implement consistent sampling.
      Updating by rtinst works as follows: bump protector1, do action, then
      bump protector2.  Shared-memory sampling by paradynd works as follows:
      read protector2, read the 3 vrbles above, read protector1.  If
      the 2 protector values differ then try again, else the sample got
      a good snapshot.  Don't forget to be sure paradynd reads the protector
      vrbles in the _opposite_ order that rtinst writes them!!! */
   volatile int protector1;
   volatile int protector2;
} tTimer;

/* see comments in rtinst.C for description of the following */
#define UNASSIGNED_TIMER_LEVEL 0
#define HARDWARE_TIMER_LEVEL 1
#define SOFTWARE_TIMER_LEVEL 2

extern int hintBestCpuTimerLevel;
extern int hintBestWallTimerLevel;

typedef rawTime64 (*timeQueryFuncPtr_t)(void);
extern timeQueryFuncPtr_t PARADYNgetCPUtime;
extern timeQueryFuncPtr_t PARADYNgetWalltime;

/* Do not call these directly, but access through the higher level time
   retrieval functions DYNINSTgetCPUtime and DYNINSTgetWalltime. */
extern rawTime64 DYNINSTgetCPUtime_sw(void);
extern rawTime64 DYNINSTgetWalltime_sw(void);
extern rawTime64 DYNINSTgetCPUtime_hw(void);
extern rawTime64 DYNINSTgetWalltime_hw(void);

extern timeQueryFuncPtr_t swCpuTimeFPtrInfo;
extern timeQueryFuncPtr_t hwCpuTimeFPtrInfo;
extern timeQueryFuncPtr_t swWallTimeFPtrInfo;
extern timeQueryFuncPtr_t hwWallTimeFPtrInfo;

/* The time retrieval functions - implemented as macros to increase
   performance.  These will call the correct software or hardware level time
   retrieval function.  Return type is rawTime64. */
#define DYNINSTgetCPUtime()   (*PARADYNgetCPUtime)()
#define DYNINSTgetWalltime() (*PARADYNgetWalltime)()

/* 
   Several things are shared between the daemon and application in the
   shared memory segment(s). These are allocated by the daemon for both
   sides, and the addresses (from the app's point of view) are communicated
   in initialization.
*/

/* Address of array of virtual timers */
extern virtualTimer *virtualTimers;
/* Address of observed cost counter */
extern unsigned *RTobserved_cost;

void PARADYNgenerateTraceRecord(short type, short length, void *eventData,
                                rawTime64 wall_time, rawTime64 process_time);


#endif
