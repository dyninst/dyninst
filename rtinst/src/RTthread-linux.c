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
 * RTthread-solaris.c: platform dependent runtime instrumentation functions for threads
 *
 ************************************************************************/


#include "RTthread.h"


#include <pthread.h>
#include <stdio.h>
#include "RTthread.h"

int DYNINSTthreadIndex ()
{
  int tid;
  int curr_index = DYNINSTthreadIndexFAST();
  if (!DYNINST_initialize_done)
    return 0;
  tid = (int)P_thread_self();
  if (tid <= 0) {
     /* P_thread_self isn't returning unexpected values at times after a fork
        so return 0 in this case. */
     /* abort(); */
     return 0;
  }

  /* Quick method. Could we get away with a logical AND? */
  if ((curr_index >= 0) && 
      (curr_index < MAX_NUMBER_OF_THREADS)) {
      if (indexToThreads[curr_index] == tid)
          return curr_index;
  }

  /* Slow method */
  curr_index = DYNINSTthreadIndexSLOW(tid);
  if (curr_index == MAX_NUMBER_OF_THREADS) {
    /* Oh, crud. Really slow */
    curr_index = DYNINSTthreadCreate(tid);
  }
  return curr_index;
}

int tc_lock_init(tc_lock_t *t)
{
  t->mutex = 0;
  t->tid = -1;
  return 0;
}

int tc_lock_unlock(tc_lock_t *t)
{
  t->tid = -1;
  t->mutex = 0;
  return 0;
}
    
int tc_lock_destroy(tc_lock_t *t)
{
  t->tid = -1;
  t->mutex = 0;
  return 0;
}

struct thread_struct {
   char ignoreA[72];
   int lwp;           //   72 bytes into thread structure
   char ignoreB[400];
   int pid;           //   476 bytes into thread structure
   char ignoreC[36];
   void *start_func;  //   516 bytes into thread structure
   char ignoreD[56];
   void *stack_addr;  //   576 bytes into thread structure
};

int DYNINST_ThreadInfo(void** stkbase, int* tidp, long *startpc, int* lwpidp,
                       void** rs_p) {
   struct thread_struct *linuxthr = (struct thread_struct *)P_thread_self();

   if(getpid() != linuxthr->pid) {
      fprintf(stderr, "thread structure has pid of %d but really in pid %d\n",
              linuxthr->pid, getpid());
      return 0;
   }

   if(P_lwp_self() != linuxthr->lwp) {
      fprintf(stderr, "thread structure has lwp of %d but really in lwp %d\n",
              linuxthr->lwp, P_lwp_self());
      return 0;
   }
   
   *stkbase = linuxthr->stack_addr;
   *tidp    = (int)P_thread_self();
   *startpc = (long)linuxthr->start_func;
   *lwpidp  = linuxthr->lwp;
   *rs_p    = linuxthr->stack_addr;
   return 1;
}

