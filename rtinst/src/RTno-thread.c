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

/************************************************************************
 *
 * RTno-thread.c: platform independent runtime instrumentation functions for non-threads
 *
 ************************************************************************/
#if defined(ONE_THREAD)

#ifdef SHM_SAMPLING
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "kludges.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"

#include <thread.h>
#include <sys/lwp.h>
#include <stdlib.h>
#include "RTthread.h"

int lwp_self() {
  return 0 ;
}

int thr_getspecific(thread_key_t key, void **valuep){
  return 0;
}

int thr_setspecific(thread_key_t key, void *value){
  return 0;
}

int thr_keycreate(thread_key_t *keyp, void (*destructor)(void *value)){
  return 0 ;
}

thread_t thr_self(void) {
  return 1 ;
}

int DYNINSTthreadPos() {
  return 0 ;
}

int DYNINSTthreadPosFAST() {
  return 0 ;
}

int DYNINST_not_deleted(void) {
  return 1 ;
}

int DYNINST_not_deletedTID(unsigned k) {
  return 1 ;
}

int DYNINSTthreadPosTID(int tid) {
  return 0;
}

int DYNINSTloopTID(int tid) {
  return 0;
}

int DYNINST_ThreadUpdate(int flag) {
  traceThread traceRec ;
  int pos = traceRec.pos = 0;
  traceRec.ppid   = getpid();
  traceRec.tid    = 1;
  traceRec.ntids  = 1;
  traceRec.stride = 0;
  traceRec.stack_addr = (unsigned) 0;
  traceRec.resumestate_p = NULL;
  traceRec.start_pc = (unsigned) 0 ;
  traceRec.context = flag ;

  DYNINSTgenerateTraceRecord(0,
        TR_THR_SELF,
        sizeof(traceRec),
        &traceRec,
        1,
        DYNINSTgetWalltime(),
        DYNINSTgetCPUtime());
  fprintf(stderr, "... Degenerated case with only 1 thread...\n");
  return pos ;
}

int DYNINSTthreadCheckRPC(void){
}

/************************************************************************
 * void DYNINSTstartProcessTimer_inferiorRPC(tTimer* timer)
************************************************************************/
void
DYNINSTstartThreadTimer_inferiorRPC(tTimer* timer, int tid) {
    /* WARNING: write() could be instrumented to call this routine, so to avoid
       some serious infinite recursion, avoid calling anything that might directly
       or indirectly call write() in this routine; e.g. printf()!!!!! */

#ifndef SHM_SAMPLING
    /* if "write" is instrumented to start/stop timers, then a timer could be
       (incorrectly) started/stopped every time a sample is written back */

    if (DYNINSTin_sample)
       return;
#endif

/* For shared-mem sampling only: bump protector1, do work, then bump protector2 */
    /* fprintf(stderr, "DYNINSTstartThreadTimer_inferiorRPC(timer = 0x%x)\n", timer); */
#ifdef SHM_SAMPLING
    timer->vtimer = (tTimer*) -1 ; /* to communicate with the daemon, see fastInferiorHeapHKs.C */
    /* fprintf(stderr,"****DYNINSTstartThreadTimer_inferiorRPC(0x%x)\n", timer); */
    assert(timer->protector1 == timer->protector2);
    timer->protector1++;
    /* How about some kind of inline asm that flushes writes when the architecture
       is using some kind of relaxed multiprocessor consistency? */
#endif

    /* Note that among the data vrbles, counter is incremented last; in particular,
       after start has been written.  This avoids a nasty little race condition in
       sampling where count is 1 yet start is undefined (or using an old value) when
       read, which usually leads to a rollback.  --ari */
    if (timer->counter == 0) {
        timer->start     = DYNINSTgetCPUtime();
    }
    timer->counter++;

#ifdef SHM_SAMPLING
    timer->protector2++; /* alternatively, timer->protector2 = timer->protector1 */
    assert(timer->protector1 == timer->protector2);
#else
    timer->normalize = MILLION; /* I think this vrble is obsolete & can be removed */
#endif
}

/************************************************************************
 * void DYNINSTstartProcessTimer(tTimer* timer)
************************************************************************/
void
DYNINSTstartThreadTimer(tTimer* timer) {
    /* WARNING: write() could be instrumented to call this routine, so to avoid
       some serious infinite recursion, avoid calling anything that might directly
       or indirectly call write() in this routine; e.g. printf()!!!!! */

#ifndef SHM_SAMPLING
    /* if "write" is instrumented to start/stop timers, then a timer could be
       (incorrectly) started/stopped every time a sample is written back */

    if (DYNINSTin_sample)
       return;
#endif

/* For shared-mem sampling only: bump protector1, do work, then bump protector2 */
#ifdef SHM_SAMPLING
    timer->vtimer = (tTimer*) -1 ; /* to communicate with the daemon, see fastInferiorHeapHKs.C */
    /* fprintf(stderr,"****DYNINSTstartThreadTimer(0x%x)\n", timer); */
    assert(timer->protector1 == timer->protector2);
    timer->protector1++;
    /* How about some kind of inline asm that flushes writes when the architecture
       is using some kind of relaxed multiprocessor consistency? */
#endif

    /* Note that among the data vrbles, counter is incremented last; in particular,
       after start has been written.  This avoids a nasty little race condition in
       sampling where count is 1 yet start is undefined (or using an old value) when
       read, which usually leads to a rollback.  --ari */
    if (timer->counter == 0) {
        timer->start     = DYNINSTgetCPUtime();
    }
    timer->counter++;

#ifdef SHM_SAMPLING
    timer->protector2++; /* alternatively, timer->protector2 = timer->protector1 */
    assert(timer->protector1 == timer->protector2);
#else
    timer->normalize = MILLION; /* I think this vrble is obsolete & can be removed */
#endif
}


/************************************************************************
 * void DYNINSTstopProcessTimer(tTimer* timer)
************************************************************************/
void
DYNINSTstopThreadTimer(tTimer* timer) {
    /* WARNING: write() could be instrumented to call this routine, so to avoid
       some serious infinite recursion, avoid calling anything that might directly
       or indirectly call write() in this routine; e.g. printf()!!!!! */

#ifndef SHM_SAMPLING
    /* if "write" is instrumented to start/stop timers, then a timer could be
       (incorrectly) started/stopped every time a sample is written back */

    if (DYNINSTin_sample)
       return;
#endif

#ifdef SHM_SAMPLING
    assert(timer->protector1 == timer->protector2);
    timer->protector1++;

    if (timer->counter == 0)
       ; /* a strange condition; shouldn't happen.  Should we make it an assert fail? */
    else {
       if (timer->counter == 1) {
          const rawTime64 now = DYNINSTgetCPUtime();
          timer->total += (now - timer->start);

          if (now < timer->start) {
             fprintf(stderr, "rtinst: cpu timer rollback.\n");
             abort();
          }
       }
       timer->counter--;
    }

    timer->protector2++; /* alternatively, timer->protector2=timer->protector1 */
    assert(timer->protector1 == timer->protector2);
#else
    if (timer->counter == 0)
       ; /* should we make this an assert fail? */
    else if (timer->counter == 1) {
        time64 now = DYNINSTgetCPUtime();

        timer->snapShot = timer->total + (now - timer->start);

        timer->mutex    = 1;
        /*
         * The reason why we do the following line in that way is because
         * a small race condition: If the sampling alarm goes off
         * at this point (before timer->mutex=1), then time will go backwards
         * the next time a sample is take (if the {wall,process} timer has not
         * been restarted).
         *
         */

        timer->total = DYNINSTgetCPUtime() - timer->start + timer->total;

        if (now < timer->start) {
            printf("id=%d, snapShot=%f total=%f, \n start=%f  now=%f\n",
                   timer->id.id, (double)timer->snapShot,
                   (double)timer->total,
                   (double)timer->start, (double)now);
            printf("process timer rollback\n"); fflush(stdout);

            abort();
        }
        timer->counter = 0;
        timer->mutex = 0;
    }
    else {
      timer->counter--;
    }
#endif
}

#endif /* ONE_THREAD */
