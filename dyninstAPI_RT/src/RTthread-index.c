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
#include <assert.h>
#if !defined(os_windows)
#include <unistd.h>
#endif
#include "RTthread.h"
#include "RTcommon.h"

#define NONE -1

static DECLARE_TC_LOCK(DYNINST_index_lock);
int DYNINST_am_initial_thread(dyntid_t);

static int first_free;
static int first_deleted;
static int num_free;
static int num_deleted;

dyntid_t DYNINST_getThreadFromIndex(int index)
{
    return (dyntid_t) DYNINST_thread_structs[index].tid;
}

void DYNINST_initialize_index_list()
{
  static int init_index_done;
  unsigned i;

  if (init_index_done) return;
  init_index_done = 1;

  DYNINST_thread_structs = (dyninst_thread_t *) malloc(DYNINST_max_num_threads * sizeof(dyninst_thread_t));
  memset(DYNINST_thread_structs, 0, DYNINST_max_num_threads * sizeof(dyninst_thread_t));

  DYNINST_thread_hash_size = (int) (DYNINST_max_num_threads * 1.25);
  DYNINST_thread_hash = (int *) malloc(DYNINST_thread_hash_size * sizeof(int));

  for (i=0; i < DYNINST_thread_hash_size; i++)
      DYNINST_thread_hash[i] = NONE;

  for (i=0; i<DYNINST_max_num_threads-1; i++) {
      DYNINST_thread_structs[i].next_free = i+1;
      DYNINST_thread_structs[i].thread_state = UNALLOC;
  }
  DYNINST_thread_structs[DYNINST_max_num_threads-1].next_free = NONE;
  DYNINST_thread_structs[DYNINST_max_num_threads-1].thread_state = UNALLOC;
  
  /* We reserve 0 for the 'initial thread' */
  first_free = 1;
  first_deleted = NONE;
  num_free = DYNINST_max_num_threads;
  num_deleted = 0;
}

/**
 * A guaranteed-if-there index lookup 
 **/
unsigned DYNINSTthreadIndexSLOW(dyntid_t tid)
{
   unsigned hash_id, orig;
   unsigned retval = DYNINST_max_num_threads;
   int index, result;
   unsigned long tid_val = (unsigned long) tid;

   result = tc_lock_lock(&DYNINST_index_lock);
   if (result == DYNINST_DEAD_LOCK)
      return DYNINST_max_num_threads;
   /**
    * Search the hash table
    **/
   hash_id = tid_val % DYNINST_thread_hash_size;
   orig = hash_id;
   for (;;) 
   {
      index = DYNINST_thread_hash[hash_id];
      if (index != NONE && DYNINST_thread_structs[index].tid == tid) {

          if (DYNINST_thread_structs[index].thread_state == LWP_EXITED) {
              /* "Hey, this guy got cleaned up!" */
              DYNINST_thread_hash[hash_id] = NONE;
              break;
          }
          else if (DYNINST_thread_structs[index].lwp != dyn_lwp_self()) {
              /* We must have exited and recreated the thread before we deleted... */
              /* Copied in effect from free_index... */
              DYNINST_thread_structs[index].thread_state = THREAD_COMPLETE;
              num_deleted++;
              break;
          }
          else {
              retval = index;
              break;
          }
      }
      hash_id++;
      if (hash_id == DYNINST_thread_hash_size)
          hash_id = 0;
      if (orig == hash_id)
          break;
   }

   tc_lock_unlock(&DYNINST_index_lock);
   return retval;
}

static unsigned get_free_index() {
    /* Assert: we are locked */
    unsigned ret = 0;
    assert(first_free != NONE);
    ret = first_free;
    first_free = DYNINST_thread_structs[first_free].next_free;
    if (first_free != NONE) {
        assert(DYNINST_thread_structs[first_free].thread_state == UNALLOC);
    }
    return ret;
}

/* What we have:
   Several entries may have set their thread_state to LWP_EXITED (done by
   the mutator). They are still in the hash table, since up until now thread code
   may have accessed them. We want to remove them from the hash table,
   set the thread_state to UNALLOC, and add it to the free list. */

static void clean_thread_list() {
    /* Assert: we are locked */
    unsigned hash_iter = 0;
    for (hash_iter = 0; hash_iter < DYNINST_thread_hash_size; hash_iter++) {
        unsigned index = DYNINST_thread_hash[hash_iter];
        if (index == NONE) continue;
        
        if (DYNINST_thread_structs[index].thread_state != LWP_EXITED)
            continue;
            
        /* Okay, this one was deleted. Remove it from the hash... */
        DYNINST_thread_hash[hash_iter] = NONE;
        /* Mark it as unallocated... */
        DYNINST_thread_structs[index].tid = 0;
        DYNINST_thread_structs[index].thread_state = UNALLOC;
        /* And add it to the end of the free list*/
        DYNINST_thread_structs[index].next_free = first_free;
        first_free = index;
        num_free++;
        num_deleted--;
    }
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

   if (DYNINST_am_initial_thread(tid)) {
       t = 0;
   }
   else  if (num_free) {
       t = get_free_index();
   }
   else if (num_deleted) {
       clean_thread_list();
       /* We have deleted, but they weren't freed by the mutator yet.... */
       while (num_free == 0) {
           sleep(1);
           clean_thread_list();
       }
       t = get_free_index();
   }
   else {
       retval = DYNINST_max_num_threads;
       goto done;
   }

   //Initialize the dyninst_thread_t object
   DYNINST_thread_structs[t].tid = tid;
   DYNINST_thread_structs[t].thread_state = THREAD_ACTIVE;
   DYNINST_thread_structs[t].next_free = NONE;
   DYNINST_thread_structs[t].lwp = dyn_lwp_self();
   
   //Put it in the hash table
   hash_id = tid_val % DYNINST_thread_hash_size;
   orig = hash_id;
   while (DYNINST_thread_hash[hash_id] != NONE)
   {
       hash_id++;
       if (hash_id == DYNINST_thread_hash_size)
           hash_id = 0;
       if (orig == hash_id) {
           /* Isn't this an error case? - bernat */
           retval = DYNINST_max_num_threads;
           goto done;
       }
   }
   
   DYNINST_thread_hash[hash_id] = t;
   retval = t;
   num_free--;
 done:
   tc_lock_unlock(&DYNINST_index_lock);
   return retval;

}

int DYNINST_free_index(dyntid_t tid)
{
   unsigned hash_id, orig;
   int index, result, retval;
   unsigned long tid_val = (unsigned long) tid;

   result = tc_lock_lock(&DYNINST_index_lock);
   if (result == DYNINST_DEAD_LOCK) {
      rtdebug_printf("%s[%d]:  DEADLOCK HERE\n", __FILE__, __LINE__);
      return -1;
   }

   /**
    * Find this thread in the hash table
    **/
   hash_id = tid_val % DYNINST_thread_hash_size;
   orig = hash_id;
   for (;;)
   {
      index = DYNINST_thread_hash[hash_id];
      if (index != NONE && DYNINST_thread_structs[index].tid == tid)
          break;
      hash_id++;
      if (hash_id == DYNINST_thread_hash_size)
          hash_id = 0;
      if (orig == hash_id)
          {
              rtdebug_printf("%s[%d]:  DESTROY FAILURE:  tid not in hash\n", __FILE__, __LINE__);
              retval = -1;
              goto done; //tid doesn't exist
          }
   }
   /* Mark this entry as disabled */
   DYNINST_thread_structs[index].thread_state = THREAD_COMPLETE;

   num_deleted++;
   retval = 0;
 done:    
   tc_lock_unlock(&DYNINST_index_lock);
   return retval;
}

/* 
   We reserve index 0 for the initial thread. This value varies by
   platform but is always constant for that platform. Wrap that
   platform-ness here. 
*/
int DYNINST_am_initial_thread(dyntid_t tid) {
#if defined(os_aix)
    return (tid == (dyntid_t) 1);
#elif defined(os_linux)
    static dyntid_t already_matched = (dyntid_t) -1; 
    if (dyn_lwp_self() == getpid()) {
        if ((already_matched != (dyntid_t) -1) && 
            (already_matched != tid)) {
            /* This can only happen in 2.4; we don't have lwp_self(),
               or multiple tids share the same lwp. Error case. */
            assert(0);
            return 0;
        }
        already_matched = tid;
        return 1;
    }
    return 0;
#elif defined(os_solaris)
    return (tid == (dyntid_t) 1);
#elif defined(os_windows)
    static int not_first = 0;
    if (not_first) return 0;
    not_first = 1; 
    return 1;
#else
    return 0;
#endif
}
 

#if 0
void DYNINST_print_lists()
{
   unsigned i;
   int index;
   printf("  Free:    ");
   for (i = first_free; i != NONE; i = threads[i].next_free)
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
