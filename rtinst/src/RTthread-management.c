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
  int index,  tid;
  void* stackbase ;
  long  startpc ;
  int   lwpid ;
  void*  resumestate_p ;

  if (DYNINST_ThreadInfo(&stackbase, &tid, &startpc, &lwpid, &resumestate_p)) {
      tid = pthread_self();
      index = DYNINSTthreadIndexSLOW(tid) ;
      if (index == MAX_NUMBER_OF_THREADS)
          index = DYNINST_alloc_index(tid);
      if (index < 0) {
          assert(0 && "Serious error in DYNINST_ThreadUpdate");
      }
      traceRec.ppid   = getpid();
      traceRec.tid    = tid;
      traceRec.index    = index;
      traceRec.lwp = P_lwp_self();
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
  _VirtualTimerStart(&(virtualTimers[index]), THREAD_CREATE) ;
  return index ;
}

/*
 * Report info for a new thread 
 */
void DYNINST_reportNewThread(unsigned index, int tid) 
{
    void* stackbase ;
    long  startpc ;
    int   lwpid ; /* Ignored */
    void*  resumestate_p ;
    extern int pipeOK(void); /* RTindexix.c */
    if (pipeOK())
        if (DYNINST_ThreadInfo(&stackbase, &tid, &startpc, &lwpid, &resumestate_p)) {
            traceThread traceRec ;
            traceRec.ppid   = getpid();
            traceRec.tid    = tid;
            traceRec.index=index ;
            traceRec.lwp = P_lwp_self();
            traceRec.ntids  = 1;
            traceRec.stride = 0;
            traceRec.stack_addr = (unsigned) stackbase;
            traceRec.start_pc = (unsigned) startpc ;
            traceRec.resumestate_p = resumestate_p ;
            traceRec.context = FLAG_SELF ;
            DYNINSTgenerateTraceRecord(0,
                                       TR_THR_CREATE,sizeof(traceRec),
                                       &traceRec,
                                       1,
                                       DYNINSTgetWalltime(),
                                       DYNINSTgetCPUtime());
        }
}

void DYNINST_reportThreadDeletion(unsigned index, int tid) {
  traceThread traceRec;
  rawTime64 process_time, wall_time ;
  traceRec.index = index;
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
  unsigned index = DYNINSTthreadIndexFAST();
  int tid = P_thread_self();
  
  if(tid == 0)  /* sometimes get invalid tid when forking */
     return;

  /* Order: set the INDEX to "awaiting deletion",
     report deletion, then stop the virtual timer. The VT will be
     deleted when it needs to be reused as part of allocating
     a new thread.
  */
  DYNINST_free_index(index, tid);

  DYNINST_reportThreadDeletion(index, tid);

  _VirtualTimerStop(&(virtualTimers[index]));

}

int DYNINSTregister_running_thread(void) {
   char line[120];
   int tid;
   int index;
   virtualTimer *vt;
/*
   sprintf(line, "REGISTER: tid: %d, lwp: %d\n",
           P_thread_self(), P_lwp_self());
   write(2, line, strlen(line));
*/ 
   tid = P_thread_self();
   if(tid == 0) {
       /* the daemon will recognize this failed RPC and handle it accordingly*/
/*
       sprintf(line, "  register, tid: %d, lwp: %d, returning\n",
               tid, P_lwp_self());
       write(2, line, strlen(line));
*/
       return 0;
   }
   index = DYNINST_lookup_index(tid);
   if(index == MAX_NUMBER_OF_THREADS) {
/*
       sprintf(line, "  couldn't find corresponding index, tid: %d, lwp %d\n",
              tid, P_lwp_self());
      write(2, line, strlen(line));
*/
      return 0;
   }
   vt = &(virtualTimers[index]);
/*
   sprintf(line, "  register, tid: %d, index: %d, lwp: %d, addr: %p\n",
           tid, index, P_lwp_self(), vt);
   write(2, line, strlen(line));
*/
   _VirtualTimerDestroy(vt);
   _VirtualTimerStart(vt, 0);
   DYNINST_reportNewThread(index, tid);

   return 1;
}

/* Returns new INDEX */

unsigned DYNINSTthreadCreate(int tid)
{
  unsigned index;
  unsigned lwpid;
  /* This can get run before we have actually initialized the
     library... from the base tramp */

  if (!DYNINST_initialize_done)
    return 0;

  /* Check to see if we already know this thread */
  index = DYNINST_lookup_index(tid);
  if (index < MAX_NUMBER_OF_THREADS)
    return index;
  index = DYNINST_alloc_index(tid);
  lwpid = P_lwp_self();
  /* Set up virtual timers */
  if (&(virtualTimers[index])) {
      _VirtualTimerDestroy(&(virtualTimers[index]));
      _VirtualTimerStart(&(virtualTimers[index]), THREAD_CREATE) ;
  }

  /* Report new thread */
  DYNINST_reportNewThread(index, tid);

  return index;
}

void DYNINST_dummy_create(void)
{
  /* Do nothing... placeholder for pthread_create() */
}

/* Called at context switch in */
void DYNINSTthreadStart() {
  unsigned i;
  unsigned index = DYNINSTthreadIndexFAST() ; /* in mini */
  if (index >= 0) {
    int lwpid = P_lwp_self() ;
    /* Restart the virtual timer */
    _VirtualTimerStart(&(virtualTimers[index]), VIRTUAL_TIMER_START) ;

#if 0
    /* Check to see if there are pending iRPCs to run */
    for (i = 0; i < MAX_PENDING_RPC; i++)
      if (pendingIRPCs[index][i].flag == 1) { /* Ha! We have an RPC! */
	if (pendingIRPCs[index][i].rpc)
	  (*pendingIRPCs[index][i].rpc)();
	pendingIRPCs[index][i].flag = 2;
      }
#endif
  }
}

/* CALLED at thread context switch out */
void DYNINSTthreadStop() {
  int index; /* in mini */
  index = DYNINSTthreadIndexFAST() ; /* in mini */
  if (index >=0) {
    _VirtualTimerStop(&(virtualTimers[index])) ;
    _VirtualTimerFinalize(&(virtualTimers[index])) ;
  }
}



