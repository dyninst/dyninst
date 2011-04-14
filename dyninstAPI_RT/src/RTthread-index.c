/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#define IDX_NONE -1

static DECLARE_TC_LOCK(DYNINST_index_lock);

static int first_free;
static int first_deleted;
static int num_free;
static int num_deleted;

static dyninst_thread_t default_thread_structs[MAX_THREADS];
static int default_thread_hash[THREADS_HASH_SIZE];

DLLEXPORT int DYNINSTthreadCount() { return (DYNINST_max_num_threads - num_free); }

dyntid_t DYNINST_getThreadFromIndex(unsigned index)
{
    return (dyntid_t) DYNINST_thread_structs[index].tid;
}

void DYNINST_initialize_index_list()
{
  static int init_index_done;
  unsigned i;

  if (init_index_done) return;
  init_index_done = 1;

  if (DYNINST_max_num_threads == MAX_THREADS)
     DYNINST_thread_structs = default_thread_structs;
  else
     DYNINST_thread_structs = (dyninst_thread_t *) malloc((DYNINST_max_num_threads+1) * sizeof(dyninst_thread_t));
  assert( DYNINST_thread_structs != NULL );
  memset(DYNINST_thread_structs, 0, (DYNINST_max_num_threads+1) * sizeof(dyninst_thread_t));

  if (DYNINST_max_num_threads == MAX_THREADS) {
     DYNINST_thread_hash_size = THREADS_HASH_SIZE;
     DYNINST_thread_hash = default_thread_hash;
  }
  else {
     DYNINST_thread_hash_size = (int) (DYNINST_max_num_threads * 1.25);
     DYNINST_thread_hash = (int *) malloc(DYNINST_thread_hash_size * sizeof(int));
  }
  assert( DYNINST_thread_hash != NULL );

  for (i=0; i < DYNINST_thread_hash_size; i++)
      DYNINST_thread_hash[i] = IDX_NONE;

  for (i=0; i<DYNINST_max_num_threads-1; i++) {
      DYNINST_thread_structs[i].next_free = i+1;
      DYNINST_thread_structs[i].thread_state = UNALLOC;
  }
  DYNINST_thread_structs[DYNINST_max_num_threads-1].next_free = IDX_NONE;
  DYNINST_thread_structs[DYNINST_max_num_threads-1].thread_state = UNALLOC;
  
  /* We reserve 0 for the 'initial thread' */
  first_free = 1;
  first_deleted = IDX_NONE;
  num_free = DYNINST_max_num_threads;
  num_deleted = 0;
}

/**
 * A guaranteed-if-there index lookup 
 **/
unsigned DYNINSTthreadIndexSLOW(dyntid_t tid)
{
   unsigned hash_id, orig;
   unsigned retval = DYNINST_NOT_IN_HASHTABLE;
   int index, result;
   unsigned long tid_val = (unsigned long) tid;
   result = tc_lock_lock(&DYNINST_index_lock);
   if (result == DYNINST_DEAD_LOCK) {
       rtdebug_printf("%s[%d]:  DEADLOCK HERE tid %lu \n", __FILE__, __LINE__, dyn_pthread_self());
      /* We specifically return DYNINST_max_num_threads so that instrumentation has someplace safe to scribble
         in case of an error. */
       /* DO NOT USE print statements here. That's horribly unsafe if we've instrumented
          the output functions, as we'll infinite recurse and die */
       return DYNINST_max_num_threads;
      }
   /**
    * Search the hash table
    **/
   if (!DYNINST_thread_hash_size) {
      //Uninitialized tramp guard.
      return DYNINST_max_num_threads;
   }

   hash_id = tid_val % DYNINST_thread_hash_size;
   orig = hash_id;
   for (;;) 
   {
      index = DYNINST_thread_hash[hash_id];
      if (index != IDX_NONE && DYNINST_thread_structs[index].tid == tid) {

          if (DYNINST_thread_structs[index].thread_state == LWP_EXITED) {
              /* "Hey, this guy got cleaned up!" */
              DYNINST_thread_hash[hash_id] = IDX_NONE;
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
    assert(first_free != IDX_NONE);
    ret = first_free;
    first_free = DYNINST_thread_structs[first_free].next_free;
    if (first_free != IDX_NONE) {
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
        if (index == IDX_NONE) continue;
        
        if (DYNINST_thread_structs[index].thread_state != LWP_EXITED)
            continue;
            
        /* Okay, this one was deleted. Remove it from the hash... */
        DYNINST_thread_hash[hash_iter] = IDX_NONE;
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

   /* Return an error if this tid already exists.*/
   if( DYNINSTthreadIndexSLOW(tid) != DYNINST_NOT_IN_HASHTABLE ) {
      /* ERROR_HANDLING_BAD */
      return DYNINST_max_num_threads;
      }

   result = tc_lock_lock(&DYNINST_index_lock);
   if (result == DYNINST_DEAD_LOCK) {
       rtdebug_printf("%s[%d]:  DEADLOCK HERE tid %lu \n", __FILE__, __LINE__, dyn_pthread_self());
       fprintf(stderr," %s[%d]:  DEADLOCK HERE tid %lu \n", __FILE__, __LINE__, (unsigned long) dyn_pthread_self());
      /* ERROR_HANDLING_BAD */
      return DYNINST_max_num_threads;
      }

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
           clean_thread_list();
       }
       t = get_free_index();
   }
   else {
       /* This might actually be a reasonable thing to do, although we should
          probably mention that the user needs to recompile Dyninst/Paradyn. */
       retval = DYNINST_max_num_threads;
       goto done;
   }

   /*Initialize the dyninst_thread_t object*/
   DYNINST_thread_structs[t].tid = tid;
   DYNINST_thread_structs[t].thread_state = THREAD_ACTIVE;
   DYNINST_thread_structs[t].next_free = IDX_NONE;
   DYNINST_thread_structs[t].lwp = dyn_lwp_self();
   
   /*Put it in the hash table*/
   hash_id = tid_val % DYNINST_thread_hash_size;
   orig = hash_id;
   while (DYNINST_thread_hash[hash_id] != IDX_NONE)
   {
       hash_id++;
       if (hash_id == DYNINST_thread_hash_size)
           hash_id = 0;
       if (orig == hash_id) {
           /* Isn't this an error case? - bernat */
           /* ERROR_HANDLING_BAD */
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
       rtdebug_printf("%s[%d]:  DEADLOCK HERE tid %lu \n", __FILE__, __LINE__, dyn_pthread_self());
       fprintf(stderr," %s[%d]:  DEADLOCK HERE tid %lu \n", __FILE__, __LINE__, (unsigned long) dyn_pthread_self());
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
      if (index != IDX_NONE && DYNINST_thread_structs[index].tid == tid)
          break;
      hash_id++;
      if (hash_id == DYNINST_thread_hash_size)
          hash_id = 0;
      if (orig == hash_id)
          {
              rtdebug_printf("%s[%d]:  DESTROY FAILURE:  tid not in hash\n", __FILE__, __LINE__);
              retval = -1;
              goto done; /*tid doesn't exist*/
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

#if 0
void DYNINST_print_lists()
{
   unsigned i;
   int index;
   printf("  Free:    ");
   for (i = first_free; i != IDX_NONE; i = threads[i].next_free)
      printf("%u@%u ", threads[i].tid, i);
   printf("\n");

   printf("  Alloced: ");
   for (i = 0; i<threads_hash_size; i++)
   {
      index = threads_hash[i];
      if (index != IDX_NONE)
         printf("%u@%u ", threads[index].tid, i);
   }
   printf("\n");
}
#endif
