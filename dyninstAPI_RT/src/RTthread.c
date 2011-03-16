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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "RTthread.h"
#include "RTcommon.h"
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "dyninstAPI_RT/h/dyninstRTExport.h"

extern unsigned DYNINST_max_num_threads;
int DYNINST_multithread_capable;
extern unsigned int DYNINSThasInitialized;

static unsigned threadCreate(dyntid_t tid);

static void (*rt_newthr_cb)(int) = NULL;
void setNewthrCB(void (*cb)(int)) {
    rt_newthr_cb = cb;
}

/**
 * Translate a tid given by pthread_self into an index.  Called by 
 * basetramps everywhere.
 **/

int DYNINSTthreadIndex()
{
   dyntid_t tid;
   unsigned curr_index;

   rtdebug_printf("%s[%d]:  welcome to DYNINSTthreadIndex()\n", __FILE__, __LINE__);
   if (!DYNINSThasInitialized) 
   {
      return 0;
   }
   
   tid = dyn_pthread_self();
   rtdebug_printf("%s[%d]:  DYNINSTthreadIndex(): tid = %lu\n", __FILE__, __LINE__, (unsigned long) tid);
   if (tid == (dyntid_t) DYNINST_SINGLETHREADED) {
      return 0;
   }

   curr_index = DYNINSTthreadIndexFAST();
   if (curr_index < DYNINST_max_num_threads &&
       DYNINST_getThreadFromIndex(curr_index) == tid)
   {
       rtdebug_printf("%s[%d]:  DYNINSTthreadIndex(): index exists already, returning %d\n", __FILE__, __LINE__, curr_index);
       return curr_index;
   }
   
   curr_index = DYNINSTthreadIndexSLOW(tid);
   if ( curr_index == DYNINST_NOT_IN_HASHTABLE )
   {
       rtdebug_printf("%s[%d]:  DYNINSTthreadIndex(): doing threadCreate for %lu\n", __FILE__, __LINE__, tid);
       curr_index = threadCreate(tid);
       rtdebug_printf("%s[%d]:  DYNINSTthreadIndex(): returning index: %d\n",  __FILE__, __LINE__, curr_index);
   }
		
   /* While DYNINST_max_num_threads is also an error return for
      DYNINSTthreadIndexSLOW(), there's not really anything we
      can do about it at the moment, so just return it
      and let the mutatee scribble into the so-allocated memory. */

   rtdebug_printf("%s[%d]:  DYNINSTthreadIndex(): returning index: %d\n",  
                  __FILE__, __LINE__, curr_index);
   return curr_index;
}

extern tc_lock_t DYNINST_trace_lock;

static int asyncSendThreadEvent(int pid, rtBPatch_asyncEventType type, 
                                void *ev, unsigned ev_size)
{
   int result;
   rtBPatch_asyncEventRecord aev;
   aev.pid = pid;
   aev.type = type;
   aev.event_fd = 0;
   aev.size = ev_size;

   result = tc_lock_lock(&DYNINST_trace_lock);
   if (result == DYNINST_DEAD_LOCK)
   {
      fprintf(stderr, "[%s:%d] - Error in libdyninstAPI_RT: trace pipe deadlock in thread %lu\n",
                    __FILE__, __LINE__, (unsigned long) dyn_pthread_self() );
      return DYNINST_TRACEPIPE_ERRVAL;
   }
   
   result = DYNINSTwriteEvent((void *) &aev, sizeof(rtBPatch_asyncEventRecord));
   if (result == -1)
   {
      fprintf(stderr, "%s[%d]:  write error creating thread\n",
              __FILE__, __LINE__);
      goto done;
   }

   result = DYNINSTwriteEvent((void *) ev, ev_size);
   if (result == -1)
   {
      fprintf(stderr, "%s[%d]:  write error creating thread\n",
              __FILE__, __LINE__);
      goto done;
   }
   
   
 done:
   tc_lock_unlock(&DYNINST_trace_lock);
   rtdebug_printf("%s[%d]:  leaving asyncSendThreadEvent: status = %s\n", 
                  __FILE__, __LINE__, result ? "error" : "ok");
   return result;
}

/**
 * Creates a new index for a given tid.
 **/
static unsigned threadCreate(dyntid_t tid)
{
   int res;
   BPatch_newThreadEventRecord ev;
   unsigned index;

   rtdebug_printf("%s[%d]:  welcome to threadCreate\n", 
                 __FILE__, __LINE__);
   if (!DYNINSThasInitialized && !DYNINSTstaticMode)
   {
      return DYNINST_max_num_threads;
   }
   
   /* Get an index */
   index = DYNINST_alloc_index(tid);

   /**
    * Trigger the mutator and mutatee side callbacks.
    **/
   memset(&ev, 0, sizeof(BPatch_newThreadEventRecord));

   ev.ppid = dyn_pid_self();
   ev.tid = tid;
   ev.lwp = dyn_lwp_self();
   ev.index = index;
   
   res = DYNINSTthreadInfo(&ev);
   if (!res)
   {
      return DYNINST_max_num_threads;
   }
   
   if (rt_newthr_cb)
   {
      rt_newthr_cb(index);
   }

   if (!DYNINSTstaticMode)
   {
      /*Only async for now.  We should parameterize this function to also 
        have a sync option.*/
      asyncSendThreadEvent(ev.ppid, rtBPatch_threadCreateEvent, &ev, 
                           sizeof(BPatch_newThreadEventRecord));
      rtdebug_printf("%s[%d]:  leaving threadCreate: index = %d\n", 
                     __FILE__, __LINE__, index);
   }
   return index;
}

/**
 * Called when a thread is destroyed
 **/
void DYNINSTthreadDestroy()
{
   dyntid_t tid = dyn_pthread_self();
   int index = DYNINSTthreadIndex();
   /*   int pid = dyn_pid_self();*/
   int err;
   /*   BPatch_deleteThreadEventRecord rec;*/

  rtdebug_printf("%s[%d]: DESTROY freeing index for thread %lu, index = %d\n", 
                 __FILE__, __LINE__, tid, index);
   err = DYNINST_free_index(tid);
   if (err) {
      rtdebug_printf("%s[%d]:  DYNINST_free_index FAILED\n", __FILE__, __LINE__);
      return;
   }
#if 0
   //  no longer do notification of exits asynchronously
   memset(&rec, 0, sizeof(rec));
   rec.index = index;
#if !defined(os_windows)
   //Windows doesn't need to use the trace pipe for thread events, thread 
   // creation/deletion is handled through the debugging interface.
   asyncSendThreadEvent(pid, rtBPatch_threadDestroyEvent, &rec, 
                        sizeof(BPatch_deleteThreadEventRecord));
#endif
#endif
}

/**
 * This function's entire purpose in life is to exist in the symbol table.
 * We modify a global variable just to ensure that no compiler ever gets
 * rid of it.
 **/
volatile int DYNINST_dummy_create_var;
void DYNINST_dummy_create()
{
   /*   fprintf(stderr, "[%s:%u] - In DYNINST_dummy_create\n", __FILE__, __LINE__);*/
   DYNINST_dummy_create_var++;
}

/**
 * Some exported wrapper function for other runtime libraries to use tc_lock
 **/

unsigned dyninst_threadIndex()
{
   return DYNINSTthreadIndex();
}
