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
 * RTthread-linux.c: platform dependent runtime instrumentation functions for threads
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
  if (tid == -1) {
     /* P_thread_self isn't returning unexpected values at times after a fork
        so return 0 in this case. */
     /* abort(); */
     fprintf(stderr, "Returning cause less than 0\n");
     return 0;
  }

  /* Quick method. Could we get away with a logical AND? */
  if ((curr_index >= 0) && 
      (curr_index < MAX_NUMBER_OF_THREADS)) {
      if (indexToThreads[curr_index] == tid)
      {
          return curr_index;
      }
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

/**
 * This structure tells us where to look in pthread's pthread_t for
 * the lwp, pid, start_func, and stack_addr fields (in that order).
 **/
#define GET_POS(buf, type, x, y) *((type *) ((buf)+pos_in_pthreadt[(x)][(y)]))
#define POS_ENTRIES 3
#define LWP_POS 0
#define PID_POS 1
#define FUNC_POS 2
#define STCK_POS 3
static unsigned pos_in_pthreadt[POS_ENTRIES][4] = { { 72, 476, 516, 576 },
                                                    { 72, 76, 516, 84 },
                                                    { 72, 476, 516, 80 } };

int DYNINST_ThreadInfo(void** stkbase, int* tidp, long *startpc, int* lwpidp,
                       void** rs_p) 
{
  static int err_printed = 0;
  pid_t pid;
  int i, lwp;
  char *buffer;
  pthread_t me;

  me = P_thread_self();
  buffer = (char *) me;
  pid = getpid();
  lwp = P_lwp_self();

  for (i = 0; i < POS_ENTRIES; i++)
  {    
    if (GET_POS(buffer, pid_t, i, PID_POS) == pid &&
	GET_POS(buffer, int, i, LWP_POS) == lwp)
    {
      *stkbase = *rs_p =  GET_POS(buffer, void *, i, STCK_POS);
      *startpc = GET_POS(buffer, long, i, FUNC_POS);
      *lwpidp = lwp;
      *tidp = (int) me;
      return 1;
    }
  }
  
  if (!err_printed)
  {
    //If you get this error, then the pos_in_pthreadt structure above
    //needs a new entry filled in.  Running the commented out program
    //that follows this function can help you collect the necessary data.
    fprintf(stderr, "[%s:%d] Unable to parse pthread_t structure for this"
	    " version of pthreads.  Making a best guess effort.\n", 
	    __FILE__, __LINE__);
    err_printed = 1;
  }
   
  *rs_p = *stkbase = 0x0; //unknown
  *startpc = 0x0; //unknown
  *tidp    = me;
  *lwpidp  = lwp;

  return 1;
}


/*
//Small program for finding the correct values to fill in pos_in_pthreadt
// above
#include <pthread.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#define gettid() syscall(SYS_gettid)

pthread_attr_t attr;

void *foo(void *f) {
  pid_t pid, tid;
  unsigned stack_addr;
  unsigned best_stack = 0xffffffff;
  int best_stack_pos = 0;
  void *start_func;
  int *p;
  int i = 0;
  pid = getpid();
  tid = gettid();
  start_func = foo;
  //x86 only.  
  asm("movl %%ebp,%0" : "=r" (stack_addr));
  p = (int *) pthread_self();
  while (i < 1000)
  {
    if (*p == (unsigned) pid)
      printf("pid @ %d\n", i);
    if (*p == (unsigned) tid)
      printf("lwp @ %d\n", i);
    if (*p > stack_addr && *p < best_stack)
    {
	best_stack = *p;
	best_stack_pos = i;
    }
    if (*p == (unsigned) start_func)
      printf("func @ %d\n", i);
    i += sizeof(int);
    p++;
  }  
  printf("stack @ %d\n", best_stack_pos);
  return NULL;
}

int main(int argc, char *argv[])
{
  pthread_t t;
  void *result;
  pthread_attr_init(&attr);
  pthread_create(&t, &attr, foo, NULL);
  pthread_join(t, &result);
  return 0;
}
*/
