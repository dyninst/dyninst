/*
 * This file contains assembly versions of often-called instrumentation
 * code
 */

#include <pthread.h>
#include <stdio.h>
#include "RTthread.h"

int DYNINSTthreadIndex ()
{
  int tid;
  int curr_index = DYNINSTthreadIndexFAST();
  if (!DYNINST_initialize_done)
    return 0;
  tid = P_thread_self();
  if (tid <= 0) {
     /* P_thread_self isn't returning unexpected values at times after a fork
        so return 0 in this case. */
     /* abort(); */
     return 0;
  }

  /* Quick method. Could we get away with a logical AND? */
  if ((curr_index >= 0) && 
      (curr_index < MAX_NUMBER_OF_THREADS)) {
      if (RTsharedData.indexToThread[curr_index] == tid)
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

