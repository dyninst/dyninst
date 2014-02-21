/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "RTthread.h"
#include "RTcommon.h"
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "dyninstAPI_RT/h/dyninstRTExport.h"

extern unsigned DYNINST_max_num_threads;
int DYNINST_multithread_capable;
extern unsigned int DYNINSThasInitialized;

static void (*rt_newthr_cb)(int) = NULL;
void setNewthrCB(void (*cb)(int)) {
    rt_newthr_cb = cb;
}

#define IDX_NONE -1

/* I split these in half to make mutator-side updates easier */
DLLEXPORT dyntid_t *DYNINST_thread_hash_tids;
DLLEXPORT long *DYNINST_thread_hash_indices;
DLLEXPORT unsigned DYNINST_thread_hash_size;

static DECLARE_TC_LOCK(DYNINST_index_lock);

static int num_free;

static dyntid_t default_thread_hash_tids[THREADS_HASH_SIZE];
static long default_thread_hash_indices[THREADS_HASH_SIZE];

DLLEXPORT int DYNINSTthreadCount() { return (DYNINST_max_num_threads - num_free); }

void DYNINST_initialize_index_list()
{
  static int init_index_done = 0;
  unsigned i;

  if (init_index_done) return;
  init_index_done = 1;
  if(DYNINST_max_num_threads == 0) DYNINST_max_num_threads = MAX_THREADS;
  if (DYNINST_max_num_threads == MAX_THREADS) {
     DYNINST_thread_hash_size = THREADS_HASH_SIZE;
     DYNINST_thread_hash_indices = default_thread_hash_indices;
     DYNINST_thread_hash_tids = default_thread_hash_tids;
  }
  else {
     DYNINST_thread_hash_size = (int) (DYNINST_max_num_threads * 1.25);
     DYNINST_thread_hash_indices =  
         malloc(DYNINST_thread_hash_size * sizeof(long));
	DYNINST_thread_hash_tids = 
		malloc(DYNINST_thread_hash_size * sizeof(dyntid_t));
  }
  assert( DYNINST_thread_hash_tids != NULL );
  assert( DYNINST_thread_hash_indices != NULL );

  for (i=0; i < DYNINST_thread_hash_size; i++)
      DYNINST_thread_hash_indices[i] = IDX_NONE;

  num_free = DYNINST_max_num_threads;
}

/**
 * A guaranteed-if-there index lookup 
 **/
unsigned DYNINSTthreadIndexSLOW(dyntid_t tid) {
    unsigned hash_id, orig;
    unsigned retval = DYNINST_NOT_IN_HASHTABLE;
    int index, result;
    unsigned long tid_val = (unsigned long) tid;
    rtdebug_printf("%s[%d]: getting dyninst index lock\n", __FILE__, __LINE__);
    result = tc_lock_lock(&DYNINST_index_lock);
    if (result == DYNINST_DEAD_LOCK) {
        rtdebug_printf("%s[%d]:  DEADLOCK HERE tid %lu \n", __FILE__, __LINE__,
                       dyn_pthread_self());
        /* We specifically return DYNINST_max_num_threads so that instrumentation
         * has someplace safe to scribble in case of an error. */

        /* DO NOT USE print statements here. That's horribly unsafe if we've instrumented
           the output functions, as we'll infinite recurse and die */
        return DYNINST_max_num_threads;
    }
    rtdebug_printf("%s[%d]: got dyninst index lock\n", __FILE__, __LINE__);

    /**
     * Search the hash table
     **/
    if (!DYNINST_thread_hash_size) {
        //Uninitialized tramp guard.
        tc_lock_unlock(&DYNINST_index_lock);
        return DYNINST_max_num_threads;
    }

    hash_id = tid_val % DYNINST_thread_hash_size;
    orig = hash_id;
    for (;;) {
        index = DYNINST_thread_hash_indices[hash_id];
        if (index != IDX_NONE && DYNINST_thread_hash_tids[hash_id] == tid) {
            retval = index;
            break;
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

/*
 * Invoked by the mutator on thread creation
 */
unsigned long DYNINSTregisterThread(dyntid_t tid, unsigned index) {
    unsigned hash_id, orig;
    unsigned long tid_val = (unsigned long) tid;

    unsigned long retval = (unsigned long)dyn_pthread_self();
    assert(retval != 0 );
    rtdebug_printf("%s[%d]: Begin DYNINSTregisterThread, tid %lu\n", __FILE__, __LINE__,
            dyn_pthread_self());

    if( tid_val != retval ) {
        tid_val = retval;
    }

    if( tc_lock_lock(&DYNINST_index_lock) == DYNINST_DEAD_LOCK ) {
       rtdebug_printf("%s[%d]:  DEADLOCK HERE tid %lu \n", __FILE__, __LINE__, 
               dyn_pthread_self());
        return 0;
    }

    hash_id = tid_val % DYNINST_thread_hash_size;
    orig = hash_id;
    while(DYNINST_thread_hash_indices[hash_id] != IDX_NONE) {
        hash_id++;
        if( hash_id == DYNINST_thread_hash_size ) hash_id = 0;
        if( orig == hash_id ) {
            retval = 0;
            break;
        }
    }

    if( retval ) {
        DYNINST_thread_hash_indices[hash_id] = index;
        DYNINST_thread_hash_tids[hash_id] = tid;
        num_free--;
        rtdebug_printf("%s[%d]: created mapping for thread (index = %lu, tid = 0x%lx)\n",
                __FILE__, __LINE__, index, tid);
    }

    tc_lock_unlock(&DYNINST_index_lock);
    return retval;
}

/*
 * Invoked by the mutator on thread exit
 */
int DYNINSTunregisterThread(dyntid_t tid) {
    unsigned hash_id, orig;
    unsigned tid_val = (unsigned long) tid;

    int retval = 1;
    rtdebug_printf("%s[%d]: Begin DYNINSTunregisterThread, tid %lu\n", __FILE__, __LINE__,
            dyn_pthread_self());

    if( tc_lock_lock(&DYNINST_index_lock) == DYNINST_DEAD_LOCK ) {
        rtdebug_printf("%s[%d]: DEADLOCK HERE tid %lu\n", __FILE__, __LINE__,
                dyn_pthread_self());
        return 0;
    }

    hash_id = tid_val % DYNINST_thread_hash_size;
    orig = hash_id;
    while(DYNINST_thread_hash_tids[hash_id] != tid) {
        hash_id++;
        if( hash_id == DYNINST_thread_hash_size ) hash_id = 0;
        if( orig == hash_id ) {
            retval = 0;
            break;
        }
    }

    if( retval ) {
        rtdebug_printf("%s[%d]: removed mapping for thread (index = %lu, tid = 0x%lx)\n",
                __FILE__, __LINE__, DYNINST_thread_hash_indices[hash_id], tid);
        DYNINST_thread_hash_indices[hash_id] = IDX_NONE;
        num_free++;
    }

    tc_lock_unlock(&DYNINST_index_lock);
    return retval;
}

int global = 1;

/**
 * Translate a tid given by pthread_self into an index.  Called by 
 * basetramps everywhere.
 **/
int DYNINSTthreadIndex() {
    dyntid_t tid;
    unsigned curr_index;

    rtdebug_printf("%s[%d]:  welcome to DYNINSTthreadIndex()\n", __FILE__, __LINE__);
    if (!DYNINSThasInitialized) {
      rtdebug_printf("%s[%d]: dyninst not initialized, ret false\n", __FILE__, __LINE__);
      return 0;
    }

    tid = (dyntid_t) ((unsigned long)dyn_pthread_self());
    rtdebug_printf("%s[%d]:  DYNINSTthreadIndex(): tid = %lu\n", __FILE__, __LINE__,
                   (unsigned long) tid);
    if (tid == (dyntid_t) DYNINST_SINGLETHREADED) return 0;
    rtdebug_printf("%s[%d]: calling thread index slow (modified)\n", __FILE__, __LINE__);
    curr_index = DYNINSTthreadIndexSLOW(tid);
    rtdebug_printf("%s[%d]: back from thread index slow\n", __FILE__, __LINE__);
    /* While DYNINST_max_num_threads is an error return for
       DYNINSTthreadIndexSLOW(), there's not really anything we
       can do about it at the moment, so just return it
       and let the mutatee scribble into the so-allocated memory. */
    if ( curr_index == DYNINST_NOT_IN_HASHTABLE ) {
        rtdebug_printf("%s[%d]:  DYNINSTthreadIndex(): failed to find index for %lu\n",
                __FILE__, __LINE__, tid);
        curr_index = DYNINST_max_num_threads;
    }

    rtdebug_printf("%s[%d]:  DYNINSTthreadIndex(): returning index: %d\n",
                   __FILE__, __LINE__, curr_index);
    return curr_index;
}

/**
 * Some exported wrapper function for other runtime libraries to use tc_lock
 **/
unsigned dyninst_threadIndex() {
   return DYNINSTthreadIndex();
}
