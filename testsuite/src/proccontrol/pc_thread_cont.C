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
#include "proccontrol_comp.h"
#include "communication.h"
#include "Process.h"
#include "Event.h"


#include <set>

using namespace std;

class pc_thread_contMutator : public ProcControlMutator {
public:
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* pc_thread_cont_factory()
{
  return new pc_thread_contMutator();
}

static std::set<Thread::const_ptr> expected_exits;
static std::set<Thread::const_ptr> expected_pre_exits;

static bool cb_error = false;

Process::cb_ret_t on_thread_exit(Event::const_ptr ev)
{
   if (ev->getEventType().code() != EventType::ThreadDestroy) {
      logerror("Got unexpected event type\n");
      cb_error = true;
      return Process::cbDefault;
   }

   if (ev->getEventType().time() == EventType::Pre) {
      set<Thread::const_ptr>::iterator i = expected_pre_exits.find(ev->getThread());
      if (i == expected_pre_exits.end()) {
         logerror("Got pre-exit for unknown thread\n");
         cb_error = true;
         return Process::cbDefault;         
      }
      expected_pre_exits.erase(i);
      return Process::cbDefault;
   }
   else if (ev->getEventType().time() == EventType::Post) {
      set<Thread::const_ptr>::iterator i = expected_pre_exits.find(ev->getThread());
      if (i != expected_pre_exits.end()) {
         logerror("Got exit without pre-exit\n");
         cb_error = true;
      }
      set<Thread::const_ptr>::iterator j = expected_exits.find(ev->getThread());
      if (j == expected_exits.end()) {
         logerror("Got exit for unknown thread\n");
         cb_error = true;
         return Process::cbDefault;
      }
      expected_exits.erase(j);
      return Process::cbDefault;
   }
   else {
      logerror("Got unexpected event time\n");
      cb_error = true;
      return Process::cbDefault;
   }
}

/**
 * After stopping all threads, continue each thread, one at a time, until
 * exit.
 **/
test_results_t pc_thread_contMutator::executeTest()
{
   std::vector<Process::ptr>::iterator i;
   bool error = false;
   cb_error = false;

   EventType et(EventType::ThreadDestroy);
   Process::registerEventCallback(et, on_thread_exit);

   if( comp->num_threads > 0 ) {
       // Continue the whole process
       for(i = comp->procs.begin(); i != comp->procs.end(); ++i) {
           if( !(*i)->continueProc() ) {
               logerror("Error continuing whole process\n");
               error = true;
           }

           if( (*i)->threads().size() != comp->num_threads+1 ) {
               logerror("Unexpected size of thread pool\n");
               error = true;
           }
       }

       // Wait for all threads to reach the barrier
       handshake stop_barrier[NUM_PARALLEL_PROCS];
       if( !comp->recv_broadcast((unsigned char *)stop_barrier, sizeof(handshake)) ) {
           logerror("Failed to receive sync broadcast\n");
           error = true;
       }
       for(unsigned j = 0; j < comp->procs.size(); ++j) {
           if( stop_barrier[j].code != HANDSHAKE_CODE ) {
               logerror("Received unexpected message code\n");
               error = true;
           }
       }

       for(i = comp->procs.begin(); i != comp->procs.end(); ++i) {
           // Stop the whole process
           if( !(*i)->stopProc() ) {
               logerror("Error stopping whole process\n");
               error = true;
           }

           // Continue the initial thread
           if( !(*i)->threads().getInitialThread()->continueThread() ) {
               logerror("Error continuing initial thread\n");
               error = true;
           }

           // Tell the mutatee that the process has stopped
           handshake stopped;
           stopped.code = HANDSHAKE_CODE;
           if( !comp->send_message((unsigned char *)&stopped, sizeof(handshake), (*i)) ) {
               logerror("Error sending handshake to mutatee\n");
               error = true;
           }
       }

       // Wait for the initial thread to unlock all the threads
       if( !comp->recv_broadcast((unsigned char *)stop_barrier, sizeof(handshake)) ) {
           logerror("Failed to receive sync broadcast\n");
           error = true;
       }

       // Continue a single thread and wait for it to exit
       for (unsigned j=0; j < comp->num_threads; j++) {
          expected_exits.clear();

          for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
             Process::ptr proc = *i;
             ThreadPool::iterator k = proc->threads().begin();
             if (k == proc->threads().end()) {
                logerror("No threads in process list\n");
                error = true;
                continue;
             }
             if (!(*k)->isInitialThread()) {
                logerror("Initial thread wasn't first in iterator\n");
                error = true;
             }
             k++;
             if (k == proc->threads().end()) {
                logerror("Not enough threads left in list\n");
                error = true;
                continue;
             }
             Thread::ptr thrd = *k;
             if (thrd->isInitialThread()) {
                logerror("Multiple initial threads or broken iterator\n");
                error = true;
             }
             
             expected_exits.insert(thrd);
             expected_pre_exits.insert(thrd);
             bool result = thrd->continueThread();
             if (!result) {
                logerror("Failed to continue thread\n");
                error = true;
             }
          }
          
          while (!expected_exits.empty())
             comp->block_for_events();
       }
   }else{
       for(i = comp->procs.begin(); i != comp->procs.end(); ++i) {
           if( !(*i)->threads().getInitialThread()->continueThread() ) {
               logerror("Error continuing initial thread\n");
               error = true;
           }
       }
   }

   allow_exit ae;
   ae.code = ALLOWEXIT_CODE;
   bool result = comp->send_broadcast((unsigned char *) &ae, sizeof(allow_exit));
   if (!result) {
      logerror("Could send exit broadcast to mutatee\n");
      error = true;
   }

   return (error || cb_error) ? FAILED : PASSED;
}


