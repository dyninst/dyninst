/*
 * This file contains assembly versions of often-called instrumentation
 * code
 */

#include <pthread.h>
#include <stdio.h>
#include "RTthread.h"

int DYNINSTthreadPos ()
{
  int tid;
  int curr_pos = DYNINSTthreadPosFAST();
  if (!DYNINST_initialize_done)
    return 0;
  tid = P_thread_self();
  if (tid <= 0) {
    abort();
  }

  fprintf(stderr, "DYNINSTthreadPos on thread %d\n", tid);
  
  /* Quick method. Could we get away with a logical AND? */
  /*
  if ((curr_pos >= 0) && 
      (curr_pos < MAX_NUMBER_OF_THREADS)) {
    if (RTsharedData.posToThread[curr_pos] == tid)
      return curr_pos;
  }
  */
  /* Slow method */
  curr_pos = DYNINSTthreadPosSLOW(tid);
  fprintf(stderr, "slow method returned %d\n", curr_pos);
  if (curr_pos == MAX_NUMBER_OF_THREADS) {
    /* Oh, crud. Really slow */
    curr_pos = DYNINSTthreadCreate(tid);
  }
  return curr_pos;
}

unsigned DYNINSTthreadContext()
{
  return 0;
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

