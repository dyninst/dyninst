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

#include "proccontrol_comp.h"
#include "communication.h"

#include <cstdio>

using namespace std;

class pc_irpcMutator : public ProcControlMutator {
public:
  virtual test_results_t executeTest();
   void runIRPCs();
   void initialMessageExchange();
   bool finalMessageExchange();

   bool setStateForRPC();
   bool postNextRPC();
   bool waitForRPCCompletion();

};

extern "C" DLLEXPORT TestMutator* pc_irpc_factory()
{
  return new pc_irpcMutator();
}

static bool myerror;

struct rpc_data_t {
   IRPC::ptr rpc;
   Thread::const_ptr thread;
   Address malloced_addr;
   bool posted;
   bool completed;
   bool assigned;

   rpc_data_t() :
      malloced_addr(0),
      posted(false),
      completed(false),
      assigned(false)
   {
   }
};

struct proc_info_t {
   Dyninst::Address val;
   Dyninst::Address irpc_calltarg;
   Dyninst::Address irpc_tocval;
   Dyninst::Address busywait;
   std::vector<rpc_data_t *> rpcs;
   proc_info_t() : val(0), irpc_calltarg(0) {}
   void clear() {
      for (unsigned i=0; i<rpcs.size(); i++) {
         delete rpcs[i];
      }
      rpcs.clear();
   }
};

static std::map<Process::ptr, proc_info_t> pinfo;

#define NUM_IRPCS 4

struct thread_info_t {
   std::vector<rpc_data_t *> rpcs;
   int cur;
   thread_info_t() :
      cur(0)
   {
   }
};

static std::map<Thread::const_ptr, thread_info_t> tinfo;
static std::map<IRPC::const_ptr, rpc_data_t *> rpc_to_data;

#include "pc_irpc_asm.h"

static bool has_pending_irpcs()
{
   for (std::map<Process::ptr, proc_info_t>::iterator i = pinfo.begin();
        i != pinfo.end(); i++)
   {
      proc_info_t &p = i->second;
      for (vector<rpc_data_t *>::iterator j = p.rpcs.begin(); j != p.rpcs.end(); j++)
      {
         rpc_data_t *rpcdata = *j;
         if (rpcdata->posted && !rpcdata->completed) {
            return true;
         }
      }
   }
   return false;
}

static bool all_irpcs_completed()
{
   for (std::map<Process::ptr, proc_info_t>::iterator i = pinfo.begin();
        i != pinfo.end(); i++)
   {
      proc_info_t &p = i->second;
      for (vector<rpc_data_t *>::iterator j = p.rpcs.begin(); j != p.rpcs.end(); j++)
      {
         if (!(*j)->completed) {
            return false;
         }
      }
   }
   return true;
}

enum allocation_mode_t {
   manual_allocate = 0,
   auto_allocate = 1
};

enum post_time_t {
   post_sequential = 0,
   post_all_once = 1,
   post_from_callback = 2
};

enum post_to_t {
   post_to_proc = 0,
   post_to_thread = 1
};

enum rpc_sync_t {
   rpc_use_sync = 0,
   rpc_use_async = 1,
   rpc_use_postsync = 2
};

enum thread_start_t {
   rpc_start_stopped = 0,
   rpc_start_running = 1
};

allocation_mode_t allocation_mode;
post_time_t post_time;
post_to_t post_to;
rpc_sync_t rpc_sync;
thread_start_t thread_start;

#define STR_CASE(S) case S: return #S

const char *am_str() {
   switch (allocation_mode) {
      STR_CASE(manual_allocate);
      STR_CASE(auto_allocate);
   }
   return NULL;
}

const char *pti_str() {
   switch (post_time) {
      STR_CASE(post_sequential);
      STR_CASE(post_all_once);
      STR_CASE(post_from_callback);
   }
   return NULL;
}

const char *pto_str() {
   switch (post_to) {
      STR_CASE(post_to_proc);
      STR_CASE(post_to_thread);
   }
   return NULL;
}

const char *rs_str() {
   switch (rpc_sync) {
      STR_CASE(rpc_use_sync);
      STR_CASE(rpc_use_async);
      STR_CASE(rpc_use_postsync);
   }
   return NULL;
}

const char *ts_str() {
   switch (thread_start) {
      STR_CASE(rpc_start_stopped);
      STR_CASE(rpc_start_running);
   }
   return NULL;
}

static bool post_irpc(Thread::const_ptr thr)
{
   Process::const_ptr proc = thr->getProcess();
   Process::ptr proc_nc = Process::ptr();

   //bad hack to remove const from process
   for (std::map<Process::ptr, proc_info_t>::iterator i = pinfo.begin();
        i != pinfo.end(); i++) {
      if (proc == i->first) {
         proc_nc = i->first;
         break;
      }
   }
   assert(proc_nc);

   proc_info_t &p = pinfo[proc_nc];

   //for (vector<rpc_data_t *>::iterator j = p.rpcs.begin(); j != p.rpcs.end(); j++)
   for (unsigned j = 0; j < p.rpcs.size(); j++)
   {
      rpc_data_t *rpcdata = p.rpcs[j];
      if (rpcdata->posted)
         continue;
      rpcdata->posted = true;

      Thread::const_ptr result_thread;
      if (post_to == post_to_proc) {
         if(rpc_sync == rpc_use_postsync) {
            if(!proc_nc->runIRPCSync(rpcdata->rpc))
            {
               logerror("Failed to post sync rpc to process\n");
               myerror = true;
               return false;
            }
            //We'll record all these too the initial thread, though we're actually
            // telling ProcControlAPI to run it anywhere.
            result_thread = proc->threads().getInitialThread();
         }
         else {
	         bool result = proc->postIRPC(rpcdata->rpc);
            if (!result) {
               logerror("Failed to post rpc to process\n");
               myerror = true;
               return false;
            }
            result_thread = proc->threads().getInitialThread();
         }
      }
      else if (post_to == post_to_thread) {
         bool result = thr->postIRPC(rpcdata->rpc);
         if (!result) {
            logerror("Failed to post rpc to thread\n");
            myerror = true;
            return false;
         }
         result_thread = thr;
      }
      thread_info_t &t = tinfo[result_thread];
      if (rpcdata->assigned) {
         if (result_thread && rpcdata->thread != result_thread) {
            logerror("postIRPC and callback disagree on RPC's thread\n");
            myerror = true;
            return false;
         }
         if (!rpcdata->completed) {
            logerror("IRPC ran callback, but was not marked completed\n");
            myerror = true;
            return false;
         }
      }
      else {
         rpcdata->assigned = true;
         rpcdata->thread = result_thread;
         t.rpcs.push_back(rpcdata);
         if (rpcdata->completed) {
            logerror("IRPC was completed but not assigned\n");
            myerror = true;
            return false;
         }
      }

      return true;
   }
   return false;
}


void pc_irpcMutator::runIRPCs() {
   unsigned char *buffer;
   unsigned buffer_size;
   tinfo.clear();
   rpc_to_data.clear();

   /**
    * Stop processes
    **/
   if (thread_start == rpc_start_stopped)
   {
      for (vector<Process::ptr>::iterator i = comp->procs.begin();
           i != comp->procs.end(); i++)
      {
         Process::ptr proc = *i;
         bool result = proc->stopProc();
         if (!result) {
            logerror("Failed to stop process\n");
            myerror = true;
            return;
         }
      }
   }

   /**
    * Create the IRPC objects.
    **/
   bool async = (rpc_sync == rpc_use_async);
   for (vector<Process::ptr>::iterator i = comp->procs.begin();
        i != comp->procs.end(); i++)
   {
	   Process::ptr proc = *i;
      proc_info_t &p = pinfo[proc];
      p.clear();
      unsigned long start_offset;
      createBuffer(proc, pinfo[proc].irpc_calltarg, pinfo[proc].irpc_tocval, buffer, buffer_size, start_offset);

      for (ThreadPool::iterator j = proc->threads().begin();
           j != proc->threads().end(); j++)
      {
         Thread::ptr thr = *j;
         thread_info_t &t = tinfo[thr];
         if(!thr->isUser())
         {
            continue;
         }

         for (unsigned k = 0; k < NUM_IRPCS; k++)
         {
            IRPC::ptr irpc;
            rpc_data_t *rpc_data = new rpc_data_t();
            if (allocation_mode == manual_allocate) {
                pthrd_printf("Mutator: start mallocMemory(size)...\n");
               Dyninst::Address addr = proc->mallocMemory(buffer_size+1);
               assert(addr);
                pthrd_printf("Mutator: start createIRPC...\n");
               irpc = IRPC::createIRPC((void *) buffer, buffer_size, addr, async);
               irpc->setStartOffset(start_offset);
               rpc_data->malloced_addr = addr;
            }
            else if (allocation_mode == auto_allocate) {
               irpc = IRPC::createIRPC((void *) buffer, buffer_size, async);
               irpc->setStartOffset(start_offset);
            }
            rpc_data->rpc = irpc;
            p.rpcs.push_back(rpc_data);
            rpc_to_data[irpc] = rpc_data;
         }
      }
      free(buffer);
   }

   /**
    * Post IRPCs
    **/
   for (std::map<Thread::const_ptr, thread_info_t>::iterator i = tinfo.begin();
        i != tinfo.end(); i++)
   {
      Thread::const_ptr thr = i->first;
      Process::const_ptr proc = thr->getProcess();
      thread_info_t &t = i->second;

	  if(!thr->isUser())
	  {
		  continue;
	  }
      int num_to_post_now = (post_time == post_all_once) ? NUM_IRPCS : 1;
      for (unsigned j=0; j<num_to_post_now; j++)
      {
         post_irpc(thr);
      }
   }

   /**
    * Wait for completion
    **/
   bool done = false;

   while (!myerror) {
      while (has_pending_irpcs()) {
         /**
          * Continue all stopped threads
          **/
         if (thread_start == rpc_start_stopped) {
            bool continued_something = true;
            while (continued_something)
            {
               continued_something = false;
               for (vector<Process::ptr>::iterator i = comp->procs.begin();
                    i != comp->procs.end(); i++)
               {
                  Process::ptr proc = *i;
                  for (ThreadPool::iterator j = proc->threads().begin();
                       j != proc->threads().end(); j++)
                  {
                     Thread::ptr thr = *j;
                     if (!thr->isStopped())
                        continue;
                     bool result = thr->continueThread();
                     if (!result)
                     {
                        logerror("Failure continuing threads\n");
                        myerror = true;
                     }
                     continued_something = true;
                  }
               }
            }
         }
         if (all_irpcs_completed() || !has_pending_irpcs()) {
            break;
         }
         /**
          * Block
          **/
         bool result = comp->block_for_events();
         if (!result) {
            logerror("Failed to block for events\n");
            myerror = true;
            return;
         }
         if (rpc_sync == rpc_use_sync && has_pending_irpcs())
         {
            //Don't consider this an error if thread_start == rpc_start_stoppe.
            // block_for_events returned because we stopped all threads, even though there were
            // sync RPCs pending.
            if (thread_start != rpc_start_stopped)
            {
               logerror("handleEvents returned with sync rpcs pending\n");
               myerror = true;
               return;
            }
         }
      }
      if (all_irpcs_completed()) {
         break;
      }
      /**
       * Post next batch, if necessary
       **/
      if (post_time == post_sequential) {
         for (std::map<Thread::const_ptr, thread_info_t>::iterator i = tinfo.begin();
				  i != tinfo.end(); i++)
         {
				Thread::const_ptr thr = i->first;
				post_irpc(thr);
			 }
      }
   }

   /**
    * Free memory
    **/
   if (allocation_mode == manual_allocate) {
      for (std::map<Process::ptr, proc_info_t>::iterator i = pinfo.begin();
           i != pinfo.end(); i++) {
         proc_info_t &p = i->second;
         Process::ptr proc = i->first;
         for (vector<rpc_data_t *>::iterator j = p.rpcs.begin(); j != p.rpcs.end(); j++)
         {
            Dyninst::Address addr = (*j)->malloced_addr;
            bool result = proc->freeMemory(addr);
            if (!result) {
               logerror("Problems freeing memory\n");
               myerror = true;
            }
         }
      }
   }

   /**
    * Check results from target process
    **/
   for (vector<Process::ptr>::iterator i = comp->procs.begin();
        i != comp->procs.end(); i++)
   {
      Process::ptr proc = *i;
      proc_info_t &p = pinfo[proc];
      /**
       * Stop process
       **/
      proc->stopProc();
      uint32_t val;
      bool result = proc->readMemory(&val, p.val, sizeof(uint32_t));
      if (!result) {
         logerror("Failure reading from process memory\n");
         myerror = true;
      }
      if (val != (comp->num_threads+1) * NUM_IRPCS) {
         logerror("val = %d, expected = %d\n", val, (comp->num_threads+1)*NUM_IRPCS);
         logerror("IRPCS did not update val\n");
         myerror = true;
      }

      val = 0;
      result = proc->writeMemory(p.val, &val, sizeof(uint32_t));
      if (!result) {
         logerror("Failure writing to process memory\n");
         myerror = true;
      }

      /**
       * Continue process
       **/
      result = proc->continueProc();
      if (!result) {
         logerror("Failure continuing process\n");
         myerror = true;
      }

      p.clear();
   }
}

Process::cb_ret_t on_irpc(Event::const_ptr ev)
{
   IRPC::const_ptr irpc = ev->getEventRPC()->getIRPC();
   std::map<IRPC::const_ptr, rpc_data_t *>::iterator i = rpc_to_data.find(irpc);
   if (i == rpc_to_data.end()) {
      logerror("Got unrecognized IRPC");
      myerror = true;
      return Process::cbDefault;
   }
   rpc_data_t *rpcdata = i->second;
   Process::const_ptr proc = ev->getProcess();
   Thread::const_ptr lookup_thread;
   if (post_to == post_to_proc) {
      lookup_thread = proc->threads().getInitialThread();
   }
   else {
      lookup_thread = ev->getThread();
   }
   thread_info_t &t = tinfo[lookup_thread];
   if (rpcdata->assigned) {
	   if (post_to == post_to_thread && rpcdata->thread && rpcdata->thread != ev->getThread()) {
         logerror("callback and postIRPC disagree on RPC's thread\n");
         myerror = true;
         return Process::cbDefault;
      }
   }
   else {
      //Darn race, we finished the rpc before we put it into the thread's RPC
      // vector.  We'll fill it in now and check the result in post.
      rpcdata->assigned = true;
      rpcdata->thread = lookup_thread;
      t.rpcs.push_back(rpcdata);
   }
   if (rpcdata->completed) {
      logerror("Got already completed IRPC in callback\n");
      myerror = true;
      return Process::cbDefault;
   }

   // Check whether the thread's registers can be read or not
   MachRegister pcReg = MachRegister::getPC(ev->getProcess()->getArchitecture());
   MachRegisterVal pcVal;
   if (!ev->getThread()->getRegister(pcReg, pcVal)) {
       logerror("Failed to retrieve PC in iRPC callback\n");
       myerror = true;
   }

   int &cur = t.cur;

   assert(cur < t.rpcs.size());
   if (t.rpcs[cur] != rpcdata) {
      if (post_to != post_to_proc)
      {
         //post_to_proc scatters the rpcs across a multithreaded process.  No guarentee on order
         logerror("RPC ran out of order\n");
         myerror = true;
      }
   }
   if (!rpcdata->posted) {
      logerror("Unposted RPC ran\n");
      myerror = true;
   }
   rpcdata->completed = true;
   cur++;

   if (post_time == post_from_callback) {
      post_irpc(ev->getThread());
   }

   if (thread_start == rpc_start_stopped) {
      return Process::cbThreadStop;
   }
   return Process::cbThreadContinue;
}

void pc_irpcMutator::initialMessageExchange()
{
	myerror = false;
   pinfo.clear();

   Process::registerEventCallback(EventType::RPC, on_irpc);

   for (std::vector<Process::ptr>::iterator i = comp->procs.begin();
        i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue process\n");
         myerror = true;
      }

      proc_info_t p;
      send_addr addr;
      result = comp->recv_message((unsigned char *) &addr, sizeof(send_addr),
                                  proc);
      if (!result) {
         logerror("Failed to recieve addr message\n");
         myerror = true;
      }
      if (addr.code != SENDADDR_CODE) {
         logerror("Unexpected addr code\n");
         myerror = true;
      }
      p.irpc_calltarg = addr.addr;

      result = comp->recv_message((unsigned char *) &addr, sizeof(send_addr),
              proc);
      if(!result) {
          logerror("Failed to receive addr message\n");
          myerror = true;
      }
      if( addr.code != SENDADDR_CODE ) {
          logerror("Unexpected addr code\n");
          myerror = true;
      }
      p.irpc_tocval = addr.addr;

      result = comp->recv_message((unsigned char *) &addr, sizeof(send_addr),
                                  proc);
      if (!result) {
         logerror("Failed to recieve addr message\n");
         myerror = true;
      }
      if (addr.code != SENDADDR_CODE) {
         logerror("Unexpected addr code\n");
         myerror = true;
      }
      p.val = addr.addr;

      result = comp->recv_message((unsigned char *) &addr, sizeof(send_addr),
                                  proc);
      if (!result) {
         logerror("Failed to recieve busywait addr message\n");
         myerror = true;
      }
      if (addr.code != SENDADDR_CODE) {
         logerror("Unexpected addr code\n");
         myerror = true;
      }
      p.busywait = addr.addr;

      pinfo[proc] = p;
   }
}


test_results_t pc_irpcMutator::executeTest()
{
   initialMessageExchange();
  if (myerror) {
     char buffer[256];
     snprintf(buffer, 256, "Errored in initial setup\n");
     logerror(buffer);
     finalMessageExchange();
     return FAILED;
  }

   //Change these values for debugging
   unsigned allocation_mode_start = 0;
   unsigned post_time_start = 0;
   unsigned post_to_start = 0;
   unsigned rpc_sync_start = 0;
   unsigned thread_start_start = 0;

   unsigned allocation_mode_end = 1;
   unsigned post_time_end = 2;
   unsigned post_to_end = 1;
   unsigned rpc_sync_end = 2;
   unsigned thread_start_end = 1;

   for (unsigned a = allocation_mode_start; a <= allocation_mode_end; a++)
      for (unsigned b = post_time_start; b <= post_time_end; b++)
         for (unsigned c = post_to_start; c <= post_to_end; c++)
            for (unsigned d = rpc_sync_start; d <= rpc_sync_end ; d++)
               for (unsigned e = thread_start_start; e <= thread_start_end; e++)
               {
                  allocation_mode = (allocation_mode_t) a;
                  post_time = (post_time_t) b;
                  post_to = (post_to_t) c;
                  rpc_sync = (rpc_sync_t) d;
                  thread_start = (thread_start_t) e;

#if defined(os_windows_test)
                  if(rpc_sync == rpc_use_sync && post_time != post_sequential) continue;
                  // Windows does not support thread-specific RPCs due to the
                  //  "create a thread" mechanism for dealing with all threads in syscall.
                  if (post_to == post_to_thread) continue;
#endif
                  if (post_time == post_from_callback && rpc_sync == rpc_use_postsync)
                     continue; //Incompatible, postSyncRPC cannot be run from a callback.

                  logerror("Running: allocation_mode=%s post_time=%s post_to=%s "
                           "rpc_sync=%s thread_start=%s\n", am_str(), pti_str(), pto_str(),
                           rs_str(), ts_str());
                  assert(!myerror);
                  runIRPCs();
                  if (myerror) {
                     char buffer[256];
                     snprintf(buffer, 256, "Errored on: allocation_mode=%s post_time=%s post_to=%s "
                              "rpc_sync=%s thread_start=%s\n", am_str(), pti_str(), pto_str(),
                              rs_str(), ts_str());
                     logerror(buffer);
                     goto done;
                  }
               }
  done:
   if (!finalMessageExchange()) {
      logerror("Failed to send sync broadcast\n");
      return FAILED;
   }

   return myerror ? FAILED : PASSED;
}

bool pc_irpcMutator::finalMessageExchange()
{
   Process::removeEventCallback(EventType::RPC);

	for (vector<Process::ptr>::iterator i = comp->procs.begin();
		i != comp->procs.end(); i++)
	{
		int done = 1;
		(*i)->writeMemory(pinfo[*i].busywait, &done, sizeof(int));
	}
   syncloc sync_point;
   sync_point.code = SYNCLOC_CODE;
   return comp->send_broadcast((unsigned char *) &sync_point, sizeof(syncloc));
}
