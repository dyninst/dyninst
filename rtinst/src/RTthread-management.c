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

#include "RTthread.h"
#include <sys/types.h>
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include <assert.h>
#include <stdio.h>

/*
 * Updates the info for the 'main' thread
 */
int DYNINST_reportThreadUpdate(int flag) {
  traceThread traceRec ;
  int pos,  tid;
  void* stackbase ;
  long  startpc ;
  int   lwpid ;
  void*  resumestate_p ;
  if (DYNINST_ThreadInfo(&stackbase, &tid, &startpc, &lwpid, &resumestate_p)) {
    pos=DYNINST_lookup_pos(tid) ;
    if (pos == MAX_NUMBER_OF_THREADS)
      pos = DYNINST_alloc_pos(tid);
    if (pos < 0) {
      assert(0 && "Serious error in DYNINST_ThreadUpdate");
    }
    traceRec.ppid   = getpid();
    traceRec.tid    = tid;
    traceRec.pos    = pos;
    traceRec.ntids  = 1;
    traceRec.stride = 0;
    traceRec.stack_addr = (unsigned) stackbase;
    traceRec.start_pc = (unsigned) startpc ;
    traceRec.resumestate_p = resumestate_p ;
    traceRec.context = flag ;
    DYNINSTgenerateTraceRecord(0,
			       TR_THR_SELF,
			       sizeof(traceRec),
			       &traceRec,
			       1,
			       DYNINSTgetWalltime(),
			       DYNINSTgetCPUtime());
  } 

  /* Called by parent thread... start its virtual timer */
  _VirtualTimerStart(&(RTsharedData.virtualTimers[pos]), THREAD_CREATE) ;
  return pos ;
}

/*
 * Report info for a new thread 
 */
void DYNINST_reportNewThread(unsigned pos, int tid) 
{
  void* stackbase ;
  long  startpc ;
  int   lwpid ;
  void*  resumestate_p ;
  extern int pipeOK(void); /* RTposix.c */
  if (pipeOK())
    if (DYNINST_ThreadInfo(&stackbase, &tid, &startpc, &lwpid, &resumestate_p)) {
      traceThread traceRec ;
      traceRec.ppid   = getpid();
      traceRec.tid    = tid;
      traceRec.ntids  = 1;
      traceRec.stride = 0;
      traceRec.stack_addr = (unsigned) stackbase;
      traceRec.start_pc = (unsigned) startpc ;
      traceRec.resumestate_p = resumestate_p ;
      traceRec.pos=pos ;
      traceRec.context = FLAG_SELF ;

      
      DYNINSTgenerateTraceRecord(0,
				 TR_THR_CREATE,sizeof(traceRec),
				 &traceRec,
				 1,
				 DYNINSTgetWalltime(),
				 DYNINSTgetCPUtime());
    }
}

void DYNINST_reportThreadDeletion(unsigned pos, int tid) {
  traceThread traceRec;
  rawTime64 process_time, wall_time ;
  traceRec.pos = pos;
  process_time = DYNINSTgetCPUtime();
  wall_time = DYNINSTgetWalltime();
  traceRec.ppid   = getpid();
  traceRec.tid    = tid;
  traceRec.ntids  = 1;
  traceRec.stride = 0;
  DYNINSTgenerateTraceRecord(0,TR_THR_DELETE,sizeof(traceRec),
                             &traceRec, 1,
                             wall_time,process_time);
}  



void DYNINSTthreadDelete(void) {
  unsigned pos = DYNINSTthreadPosFAST();
  int tid = DYNINSTthreadSelf();

  /* Order: set the POS to "awaiting deletion",
     report deletion, then destroy the timer */
  DYNINST_free_pos(pos, tid);

  DYNINST_reportThreadDeletion(pos, tid);

  DYNINST_VirtualTimerDestroy(&(RTsharedData.virtualTimers[pos]));

}

/* Returns new POS */

unsigned DYNINSTthreadCreate(int tid)
{
  unsigned pos;
  unsigned lwpid;
  /* This can get run before we have actually initialized the
     library... from the base tramp */

  if (!DYNINST_initialize_done)
    return 0;
  /* Check to see if we already know this thread */
  pos = DYNINST_lookup_pos(tid);
  if (pos < MAX_NUMBER_OF_THREADS)
    return pos;
  pos = DYNINST_alloc_pos(tid);
  lwpid = P_lwp_self();
  /* Report new thread */
  DYNINST_reportNewThread(pos, tid);
  /* Store the POS in thread-specific storage */
  /* Shift POS to (1 <-> MAX) to avoid NULL return value */
  P_thread_setspecific(DYNINST_thread_key, (void *)(pos+1)) ;
  /* Set up virtual timers */
  if (&(RTsharedData.virtualTimers[pos])) {
    _VirtualTimerStart(&(RTsharedData.virtualTimers[pos]), THREAD_CREATE) ;
  }
  return pos;
}

void DYNINST_dummy_create(void)
{
  /* Do nothing... placeholder for pthread_create() */
}

/* Called at context switch in */
void DYNINSTthreadStart() {
  unsigned i;
  unsigned pos = DYNINSTthreadPosFAST() ; /* in mini */
  if (pos >= 0) {
    int lwpid = P_lwp_self() ;

    /* Restart the virtual timer */
    _VirtualTimerStart(&(RTsharedData.virtualTimers[pos]), VIRTUAL_TIMER_START) ;

    /* Check to see if there are pending iRPCs to run */
    for (i = 0; i < MAX_PENDING_RPC; i++)
      if (RTsharedData.pendingIRPCs[pos][i].flag == 1) { /* Ha! We have an RPC! */
	if (RTsharedData.pendingIRPCs[pos][i].rpc)
	  (*RTsharedData.pendingIRPCs[pos][i].rpc)();
	RTsharedData.pendingIRPCs[pos][i].flag = 2;
      }
  }
}

/* CALLED at thread context switch out */
void DYNINSTthreadStop() {
  int pos; /* in mini */
  pos = DYNINSTthreadPosFAST() ; /* in mini */
  if (pos >=0) {
    _VirtualTimerStop(&(RTsharedData.virtualTimers[pos])) ;
  }
}



