/*
 * This file contains assembly versions of often-called instrumentation
 * code
 */

#include "tc-lock.h"
#include <pthread.h>
#include <stdio.h>

#define CURR_POS_REG register unsigned int curr_pos asm("11")

void DYNINST_initialize_once(void);
extern int DYNINST_ThreadTids[MAX_NUMBER_OF_THREADS];

int DYNINSTthreadPos ()
{
  int pid;
  int curr_pos = DYNINSTthreadPosFAST();
  DYNINST_initialize_once(); /* Is this necessary? */
  pid = DYNINSTthreadSelf();
  fprintf(stderr, "DYNINSTthreadPos for pid %d\n", pid);
  if (pid <= 0) {
    fprintf(stderr, "Bad PID: %d\n", pid);
    abort();
  }
  /* Quick method. Could we get away with a logical AND? */
  if ((curr_pos > 0) && 
      (curr_pos < MAX_NUMBER_OF_THREADS) &&
      (DYNINST_ThreadTids[curr_pos] == pid)) {
    return curr_pos;
  }
  /* Slow method */
  curr_pos = _threadPos(pid, curr_pos);
  DYNINST_ThreadCreate(curr_pos, pid);
  /* Store curr_pos on stack somewhere around here? */
  fprintf(stderr, "DyninstThreadPos: %d\n", curr_pos);
  return curr_pos;
}

int DYNINSTloop()
{
  fprintf(stderr, "Looping\n");
  return 0;
}

int DYNINST_not_deleted()
{
  CURR_POS_REG;
  int ret = (curr_pos != -2);
  fprintf(stderr, "DYNINST_not_deleted: ret %d\n", ret);
  return ret;
}

int DYNINSTthreadDeletePos()
{
  CURR_POS_REG;
  curr_pos = -2;
  return curr_pos;
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

