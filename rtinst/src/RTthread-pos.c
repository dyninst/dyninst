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

#include "kludges.h"
#include "RTthread.h"

/***********************************************************
 *
 * RTthread-index: INDEX calculation for MT paradyn
 *
 ***********************************************************/

/* Short version: when we get a pthread id, it is rarely in
   a form that is easy to use as an array offset. IDs are rarely
   consecutive numbers. So we convert this thread ID to a paradyn
   ID, called an INDEX. 
*/

unsigned  DYNINST_next_free_index;
unsigned  DYNINST_num_index_free;
DECLARE_TC_LOCK(DYNINST_index_lock);

void DYNINST_initialize_index_list()
{
  unsigned i;
  static int init_index_done = 0;
  if (init_index_done) return;
  tc_lock_init(&DYNINST_index_lock);
  for (i = 0; i < MAX_NUMBER_OF_THREADS; i++)
    RTsharedData.indexToThread[i] = 0;
  /* 0 means a free slot. */
  DYNINST_next_free_index = 0;
  DYNINST_num_index_free = MAX_NUMBER_OF_THREADS;
  init_index_done = 1;

  /* That gets the INDEX->THREAD mapping done, now the other way around */
  DYNINST_indexHash = (unsigned *)calloc(MAX_NUMBER_OF_THREADS, sizeof(unsigned));
  for (i = 0; i < MAX_NUMBER_OF_THREADS; i++)
      DYNINST_indexHash[i] = -1;
}

/* 
   Get the next free INDEX slot, using a linear scan of 
   the array. Question: would a linked list be faster?
*/
unsigned DYNINST_alloc_index(int tid)
{
    unsigned hashed_tid;
    unsigned next_free_index;
    unsigned saw_deleted_index = 0;
    unsigned looped_once = 0;

    if (!DYNINST_num_index_free) return -1;
    if (DYNINST_DEAD_LOCK == tc_lock_lock(&DYNINST_index_lock)) {
        return MAX_NUMBER_OF_THREADS;
    }
    /* We've got the lock, stay here as short a time as indexsible */
    next_free_index = DYNINST_next_free_index;
    
    while (RTsharedData.indexToThread[next_free_index] != 0) {
        if (RTsharedData.indexToThread[next_free_index] == THREAD_AWAITING_DELETION)
            saw_deleted_index = 1;
        next_free_index++;
        if (next_free_index >= MAX_NUMBER_OF_THREADS)
            if (looped_once && !saw_deleted_index) {
                /* Weird... we've gone through the entire array with no luck,
                   and there are no threads being freed */
                tc_lock_unlock(&DYNINST_index_lock);
                return MAX_NUMBER_OF_THREADS;
            }
            else {
                /* There is at least one thread that's going to be freed,
                   so loop until it is*/
                next_free_index -= MAX_NUMBER_OF_THREADS;
                looped_once = 1;
            }
    }
    /* next_free_index is free */
    
    RTsharedData.indexToThread[next_free_index] = tid;
    DYNINST_num_index_free--;
    DYNINST_next_free_index = next_free_index+1;
    
    /* Now that we have the index, store it for this thread */
    /* TODO: better hash function than mod */
    hashed_tid = tid % MAX_NUMBER_OF_THREADS;
    while(DYNINST_indexHash[hashed_tid] != -1) {
        hashed_tid++;
        if (hashed_tid == MAX_NUMBER_OF_THREADS)
            hashed_tid -= MAX_NUMBER_OF_THREADS;
    }
    DYNINST_indexHash[hashed_tid] = next_free_index;
    
    tc_lock_unlock(&DYNINST_index_lock);
    return next_free_index;
}
    
void DYNINST_free_index(unsigned index, int tid)
{
    unsigned hashed_tid;
    if (RTsharedData.indexToThread[index] != tid) {
        return;
    }
    if (DYNINST_DEAD_LOCK == tc_lock_lock(&DYNINST_index_lock)) {
        return;
    }
    /* Don't free immediately -- the daemon needs to clear out the
       variable arrays first */
    RTsharedData.indexToThread[index] = THREAD_AWAITING_DELETION;
    DYNINST_num_index_free++;
    
    hashed_tid = tid % MAX_NUMBER_OF_THREADS;
    while (DYNINST_indexHash[hashed_tid] != index) {
        hashed_tid++;
        if (hashed_tid >= MAX_NUMBER_OF_THREADS)
            hashed_tid -= MAX_NUMBER_OF_THREADS;
    }
    DYNINST_indexHash[hashed_tid] = -1;
    
    tc_lock_unlock(&DYNINST_index_lock);
}

/* Why would someone want to do this? */

unsigned DYNINST_lookup_index(int tid)
{
  /* Readonly... no need to lock */
  unsigned i;
  for (i = 0; i < MAX_NUMBER_OF_THREADS; i++)
    if (RTsharedData.indexToThread[i] == tid)
      return i;
  return MAX_NUMBER_OF_THREADS;
}


/***************************************************************/

/* A guaranteed-if-there index lookup */

unsigned DYNINSTthreadIndexSLOW(int tid)
{
  unsigned index = -1;
  unsigned hashed_tid;
  unsigned orig_tid;
  hashed_tid = tid % MAX_NUMBER_OF_THREADS;
  orig_tid = hashed_tid;

  while(1) {
      index = DYNINST_indexHash[hashed_tid];
      if ((index >= 0) && (RTsharedData.indexToThread[index] == tid)) {
         /* Found it */
          break;
      }
      hashed_tid++;
      if (hashed_tid >= MAX_NUMBER_OF_THREADS)
          hashed_tid -= MAX_NUMBER_OF_THREADS;
      if (hashed_tid == orig_tid) {
          /* Breakout condition */
          return MAX_NUMBER_OF_THREADS;
      }
      
  }

  return index;
}
