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

#include "kludges.h"
#include "RTthread.h"

/***********************************************************
 *
 * RTthread-pos: POS calculation for MT paradyn
 *
 ***********************************************************/

/* Short version: when we get a pthread id, it is rarely in
   a form that is easy to use as an array offset. IDs are rarely
   consecutive numbers. So we convert this thread ID to a paradyn
   ID, called a POS. 
*/

unsigned  DYNINST_next_free_pos;
unsigned  DYNINST_num_pos_free;
DECLARE_TC_LOCK(DYNINST_pos_lock);

void DYNINST_initialize_pos_list()
{
  unsigned i;
  static int init_pos_done = 0;
  if (init_pos_done) return;
  tc_lock_init(&DYNINST_pos_lock);
  for (i = 0; i < MAX_NUMBER_OF_THREADS; i++)
    RTsharedData.posToThread[i] = 0;
  /* 0 means a free slot. */
  DYNINST_next_free_pos = 0;
  DYNINST_num_pos_free = MAX_NUMBER_OF_THREADS;
  init_pos_done = 1;
}

/* 
   Get the next free POS slot, using a linear scan of 
   the array. Question: would a linked list be faster?
*/
unsigned DYNINST_alloc_pos(int tid)
{
  unsigned next_free_pos;
  unsigned saw_deleted_pos = 0;
  unsigned looped_once = 0;
  if (!DYNINST_num_pos_free) return -1;
  if (DYNINST_DEAD_LOCK == tc_lock_lock(&DYNINST_pos_lock)) {
    fprintf(stderr, "Attempting to re-acquire lock for POS table\n");
    return MAX_NUMBER_OF_THREADS;
  }
  /* We've got the lock, stay here as short a time as possible */
  next_free_pos = DYNINST_next_free_pos;
  while (RTsharedData.posToThread[next_free_pos] != 0) {
    if (RTsharedData.posToThread[next_free_pos] == THREAD_AWAITING_DELETION)
      saw_deleted_pos = 1;
    next_free_pos++;
    if (next_free_pos >= MAX_NUMBER_OF_THREADS)
      if (looped_once && !saw_deleted_pos) {
	/* Weird... we've gone through the entire array with no luck,
	   and there are no threads being freed */
	tc_lock_unlock(&DYNINST_pos_lock);
	return MAX_NUMBER_OF_THREADS;
      }
      else {
	/* There is at least one thread that's going to be freed,
	   so loop until it is*/
	next_free_pos -= MAX_NUMBER_OF_THREADS;
	looped_once = 1;
      }
  }
  /* next_free_pos is free */
  fprintf(stderr, "Returning slot %d, at addr 0x%x, 0x%x\n",
	  next_free_pos, &(RTsharedData.posToThread[next_free_pos]),
	  RTsharedData.posToThread);
  RTsharedData.posToThread[next_free_pos] = tid;
  DYNINST_num_pos_free--;
  DYNINST_next_free_pos = next_free_pos+1;
  tc_lock_unlock(&DYNINST_pos_lock);
  return next_free_pos;
}
    
void DYNINST_free_pos(unsigned pos, int tid)
{
  if (RTsharedData.posToThread[pos] != tid) {
    fprintf(stderr, "POS and tid don't match in free (%u != %d)\n", pos, tid);
    return;
  }
  if (DYNINST_DEAD_LOCK == tc_lock_lock(&DYNINST_pos_lock)) {
    fprintf(stderr, "Attempting to re-acquire lock for POS table\n");
    return;
  }
  /* Don't free immediately -- the daemon needs to clear out the
     variable arrays first */
  RTsharedData.posToThread[pos] = THREAD_AWAITING_DELETION;
  DYNINST_num_pos_free++;
  tc_lock_unlock(&DYNINST_pos_lock);
}

/* Why would someone want to do this? */

unsigned DYNINST_lookup_pos(int tid)
{
  /* Readonly... no need to lock */
  unsigned i;
  for (i = 0; i < MAX_NUMBER_OF_THREADS; i++)
    if (RTsharedData.posToThread[i] == tid)
      return i;
  return MAX_NUMBER_OF_THREADS;
}


/***************************************************************/

/*
  Why -1? Because POS starts at 0, but NULL is a valid
  return value (means not found). So we shift the range
  to 1-MAX_NUMBER_OF_THREADS
*/
unsigned DYNINSTthreadPosSLOW(tid)
{
  unsigned pos;
  pos = (unsigned)P_thread_getspecific(DYNINST_thread_key);
  if (pos == 0)
    return MAX_NUMBER_OF_THREADS;
  return pos-1;
}
