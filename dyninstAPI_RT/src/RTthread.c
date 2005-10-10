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

#include <stdlib.h>
#include <stdio.h>

#include "RTthread.h"
#include "RTcommon.h"
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"

unsigned DYNINST_max_num_threads;
int DYNINST_multithread_capable;
extern unsigned int DYNINSThasInitialized;

static unsigned threadCreate(int tid);

void (*newthr_cb)(int);

/**
 * Translate a tid given by pthread_self into an index.  Called by 
 * basetramps everywhere.
 **/
int DYNINSTthreadIndex()
{
   int tid, curr_index;

   if (!DYNINSThasInitialized) 
   {
      return 0;
   }

   tid = dyn_pthread_self();
   if (tid == -1)
   {
      return 0;
   }

   curr_index = DYNINSTthreadIndexFAST();
   if ((curr_index >= 0) && (curr_index < DYNINST_max_num_threads) &&
       (DYNINST_getThreadFromIndex(curr_index) == tid))
   {
      return curr_index;
   }
   
   curr_index = DYNINSTthreadIndexSLOW(tid);
   if (curr_index == DYNINST_max_num_threads)
   {
      curr_index = threadCreate(tid);
   }

   return curr_index;
}

static tc_lock_t DYNINST_trace_lock;

static int asyncSendThreadEvent(int pid, rtBPatch_asyncEventType type, 
                                void *ev, int ev_size)
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
      fprintf(stderr, "[%s:%d] - Error in libdyninstAPI_RT: trace pipe deadlock\n",
                    __FILE__, __LINE__);
      return -1;
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
   return result;
}

/**
 * Creates a new index for a given tid.
 **/
static unsigned threadCreate(int tid)
{
   int res;
   BPatch_newThreadEventRecord ev;
   unsigned index;

   if (!DYNINSThasInitialized)
   {
      fprintf(stderr, "[%s:%u] - Not initialized\n", __FILE__, __LINE__);
      return DYNINST_max_num_threads;
   }
   
   // Get an index
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
   
   if (newthr_cb)
   {
      newthr_cb(index);
   }

   //Only async for now.  We should parameterize this function to also have a
   // sync option.
   asyncSendThreadEvent(ev.ppid, rtBPatch_threadCreateEvent, &ev, 
                        sizeof(BPatch_newThreadEventRecord));
   return index;
}

/**
 * Called when a thread is destroyed
 **/
void DYNINSTthreadDestroy()
{
   int tid = dyn_pthread_self();
   int index = DYNINSTthreadIndex();
   int pid = dyn_pid_self();
   int lwp = dyn_lwp_self();
   int err;

   err = DYNINST_free_index(tid);
   if (err)
      return;
   
   BPatch_deleteThreadEventRecord rec;

   memset(&rec, 0, sizeof(rec));
   rec.index = index;
   
   asyncSendThreadEvent(pid, rtBPatch_threadDestroyEvent, &rec, 
                        sizeof(BPatch_deleteThreadEventRecord));
}

/**
 * This function's entire purpose in life is to exist in the symbol table.
 * We modify a global variable just to ensure that no compiler ever gets
 * rid of it.
 **/
volatile int DYNINST_dummy_create_var;
void DYNINST_dummy_create()
{
   //   fprintf(stderr, "[%s:%u] - In DYNINST_dummy_create\n", __FILE__, __LINE__);
   //DYNINST_dummy_create_var++;
}

/**
 * Some exported wrapper function for other runtime libraries to use tc_lock
 **/

unsigned dyninst_threadIndex()
{
   return DYNINSTthreadIndex();
}
