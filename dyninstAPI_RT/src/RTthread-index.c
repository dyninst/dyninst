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

/***********************************************************
 *
 * RTthread-index: INDEX calculation for MTDyninst
 *
 ***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RTthread.h"

#define NONE -1

static DECLARE_TC_LOCK(DYNINST_index_lock);

typedef struct thread_t {
   dyntid_t tid;
   int next;
} thread_t;

static thread_t *threads;
static int *threads_hash;
static unsigned threads_hash_size;

static int first_free;
static int first_deleted;

dyntid_t DYNINST_getThreadFromIndex(int index)
{
   return (dyntid_t) threads[index].tid;
}

void DYNINST_initialize_index_list()
{
  static int init_index_done;
  unsigned i;

  if (init_index_done) return;
  init_index_done = 1;

  threads = (thread_t *) malloc(DYNINST_max_num_threads * sizeof(thread_t));
  memset(threads, 0, DYNINST_max_num_threads * sizeof(thread_t));

  threads_hash_size = (int) (DYNINST_max_num_threads * 1.25);
  threads_hash = (unsigned *) malloc(threads_hash_size * sizeof(unsigned));


  for (i=0; i<threads_hash_size; i++)
     threads_hash[i] = NONE;

  for (i=0; i<DYNINST_max_num_threads-1; i++)
     threads[i].next = i+1;
  threads[DYNINST_max_num_threads-1].next = NONE;

  first_free = 0;
  first_deleted = NONE;
}

/**
 * A guaranteed-if-there index lookup 
 **/
unsigned DYNINSTthreadIndexSLOW(dyntid_t tid)
{
   unsigned hash_id, orig;
   unsigned retval;
   int index, t, result;
   unsigned long tid_val = (unsigned long) tid;

   result = tc_lock_lock(&DYNINST_index_lock);
   if (result == DYNINST_DEAD_LOCK)
      return DYNINST_max_num_threads;
   /**
    * Search the hash table
    **/
   hash_id = tid_val % threads_hash_size;
   orig = hash_id;
   for (;;) 
   {
      index = threads_hash[hash_id];
      if (index != NONE && threads[index].tid == tid)
      {
         retval = index;
         goto done;
      }
      hash_id++;
      if (hash_id == threads_hash_size)
         hash_id = 0;
      if (orig == hash_id)
         break;
   }

   /**
    * If we didn't find a tid it could have been deleted,
    * search the deleted list linearly.  A find here should
    * be rare, since it should mean that a thread called DYNINST_free_index
    * and then DYNINST_index_slow.
    **/
   for (t = first_deleted; t != NONE; t = threads[t].next)
      if (threads[t].tid == tid)
      {
         retval = t;
         goto done;
      }

   retval = DYNINST_max_num_threads; //Couldn't find it

 done:
   tc_lock_unlock(&DYNINST_index_lock);
   return retval;
}
    
unsigned DYNINST_alloc_index(dyntid_t tid)
{
   int result;
   unsigned hash_id, orig;
   unsigned t, retval;
   unsigned long tid_val = (unsigned long) tid;

   //Return an error if this tid already exists.
   if (DYNINSTthreadIndexSLOW(tid) != DYNINST_max_num_threads)
      return DYNINST_max_num_threads;
   
   result = tc_lock_lock(&DYNINST_index_lock);
   if (result == DYNINST_DEAD_LOCK)
      return DYNINST_max_num_threads;

   if (first_free != NONE) //An unallocated free slot exists
   {
      t = first_free;
      first_free = threads[first_free].next;
   }
   else if (first_deleted != NONE) //No un-allocated free slots, use a deleted
   {
      t = first_deleted;
      first_deleted = threads[first_deleted].next;
   }
   else //No threads slots free
   {
      retval = DYNINST_max_num_threads;
      goto done;
   }
   
   //Initialize the thread_t object
   threads[t].tid = tid;
   threads[t].next = NONE;

   //Put it in the hash table
   hash_id = tid_val % threads_hash_size;
   orig = hash_id;
   while (threads_hash[hash_id] != NONE)
   {
      hash_id++;
      if (hash_id == threads_hash_size)
         hash_id = 0;
      if (orig == hash_id)
      {
         retval = DYNINST_max_num_threads;
         goto done;
      }
   }

   threads_hash[hash_id] = t;
   retval = t;
 done:
   tc_lock_unlock(&DYNINST_index_lock);
   return retval;
}

int DYNINST_free_index(dyntid_t tid)
{
   unsigned deleted_end = 0;
   unsigned hash_id, orig;
   int t, index, result, retval;
   unsigned long tid_val = (unsigned long) tid;

   result = tc_lock_lock(&DYNINST_index_lock);
   if (result == DYNINST_DEAD_LOCK)
      return -1;
   
   /**
    * Find this thread in the hash table
    **/
   hash_id = tid_val % threads_hash_size;
   orig = hash_id;
   for (;;)
   {
      index = threads_hash[hash_id];
      if (index != NONE && threads[index].tid == tid)
         break;
      hash_id++;
      if (hash_id == threads_hash_size)
         hash_id = 0;
      if (orig == hash_id)
      {
         retval = -1;
         goto done; //tid doesn't exist
      }
   }

   /**
    * Find the end of the deleted list, and make sure that
    * this thread hasn't already been deleted.
    **/
   for (t = first_deleted; t != NONE; t = threads[t].next)
   {
      if (t == index)
      {
         retval = -1;
         goto done; //double free
      }
      deleted_end = t;
   }


   /**
    * Clean-up
    **/
   //Remove this dude from the hash table
   threads_hash[hash_id] = NONE;

   //Add him to the end of the deleted list
   if (first_deleted == NONE)
      first_deleted = index;
   else
      threads[deleted_end].next = index;
   threads[index].next = NONE;
   
   retval = 0;
 done:    
   tc_lock_unlock(&DYNINST_index_lock);
   return retval;
}

#if 0
void DYNINST_print_lists()
{
   unsigned i;
   int index;
   printf("  Free:    ");
   for (i = first_free; i != NONE; i = threads[i].next)
      printf("%u@%u ", threads[i].tid, i);
   printf("\n");

   printf("  Deleted: ");
   for (i = first_deleted; i != NONE; i = threads[i].next)
      printf("%u@%u ", threads[i].tid, i);
   printf("\n");

   printf("  Alloced: ");
   for (i = 0; i<threads_hash_size; i++)
   {
      index = threads_hash[i];
      if (index != NONE)
         printf("%u@%u ", threads[index].tid, i);
   }
   printf("\n");
}
#endif
