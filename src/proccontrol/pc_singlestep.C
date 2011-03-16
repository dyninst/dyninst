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

using namespace std;

class pc_singlestepMutator : public ProcControlMutator {
public:
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* pc_singlestep_factory()
{
  return new pc_singlestepMutator();
}

#define BP_FUNC 2
#define STOP_FUNC 3
#define NUM_FUNCS 5

struct proc_info {
   Address func[NUM_FUNCS];
   proc_info()
   {
      for (unsigned i=0; i<NUM_FUNCS; i++) {
         func[i] = 0x0;
      }
   }
};

struct thread_info {
   int breakpoint;
   int order;
   int hit_funcs[NUM_FUNCS];
   unsigned steps;
   thread_info() :
      breakpoint(-1),
      order(0),
      steps(0)
   {
      for (unsigned i=0; i<NUM_FUNCS; i++) {
         hit_funcs[i] = -1;
      }
   }
};

std::map<Thread::const_ptr, thread_info> tinfo;
std::map<Process::const_ptr, proc_info> pinfo;
Breakpoint::ptr bp;

static bool myerror;

Process::cb_ret_t on_breakpoint(Event::const_ptr ev)
{
   EventBreakpoint::const_ptr ebp = ev->getEventBreakpoint();
   std::vector<Breakpoint::ptr> bps;
   ebp->getBreakpoints(bps);
   if (bps.size() != 1 && bps[0] != bp) {
      logerror("Got unexpected breakpoing\n");
      myerror = true;
   }
   thread_info &ti = tinfo[ev->getThread()];
   ti.breakpoint = ti.order++;
   return Process::cbProcContinue;
}

Process::cb_ret_t on_singlestep(Event::const_ptr ev)
{
   MachRegister pc = MachRegister::getPC(ev->getProcess()->getArchitecture());
   MachRegisterVal loc;

   bool result = ev->getThread()->getRegister(pc, loc);
   if (!result) {
      logerror("Failed to read PC register\n");
      myerror = true;
      return Process::cbDefault;
   }
   
   if (!ev->getThread()->getSingleStepMode())
   {
      logerror("Single step on thread not in single step mode\n");
      myerror = true;
   }

   proc_info &pi = pinfo[ev->getProcess()];
   thread_info &ti = tinfo[ev->getThread()];

   ti.steps++;
   for (unsigned i = 0; i<NUM_FUNCS; i++) {
      if (pi.func[i] == loc) {
         if (ti.hit_funcs[i] != -1) {
            logerror("Single step was executed twice");
            myerror = true;
         }
         ti.hit_funcs[i] = ti.order++;
         if (i == STOP_FUNC) {
            //Last singlstep point.
            ev->getThread()->setSingleStepMode(false);
         }
      }
   }
   
   return Process::cbThreadContinue;
}

test_results_t pc_singlestepMutator::executeTest()
{
   myerror = false;

   Process::registerEventCallback(EventType::Breakpoint, on_breakpoint);
   Process::registerEventCallback(EventType::SingleStep, on_singlestep);
   tinfo.clear();
   pinfo.clear();
   bp = Breakpoint::newBreakpoint();

   
   std::set<Thread::ptr> singlestep_threads;
   std::set<Thread::ptr> regular_threads;

   for (std::vector<Process::ptr>::iterator i = comp->procs.begin(); 
        i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue process\n");
         myerror = true;
      }

      proc_info &pi = pinfo[proc];
      Address funcs[NUM_FUNCS];
      for (unsigned j=0; j < NUM_FUNCS; j++)
      {
         send_addr addr;
         bool result = comp->recv_message((unsigned char *) &addr, sizeof(send_addr), 
                                          proc);
         if (!result) {
            logerror("Failed to recieve addr message\n");
            myerror = true;
         }
         if (addr.code != SENDADDR_CODE) {
            logerror("Unexpected addr code\n");
            myerror = true;
         }
         pi.func[j] = addr.addr;
      }
      
      result = proc->stopProc();
      if (!result) {
         logerror("Failed to stop process\n");
         myerror = true;
      }

      Dyninst::Address addr = pi.func[BP_FUNC];
      result = proc->addBreakpoint(addr, bp);
      if (!result) {
         logerror("Failed to insert breakpoint\n");
         myerror = true;
      }
      
      syncloc sync_msg;
      sync_msg.code = SYNCLOC_CODE;
      result = comp->send_message((unsigned char *) &sync_msg, sizeof(sync_msg), 
                                       proc);
      if (!result) {
         logerror("Failed to send sync message to process\n");
         myerror = true;
      }
      
      ThreadPool::iterator j;
      int count = 0;
      for (j = proc->threads().begin(); j != proc->threads().end(); j++)
      {
         //Singlestep half of the threads.
         Thread::ptr thrd = *j;
         if (count++ % 2 == 0 || thrd->isInitialThread()) {
            singlestep_threads.insert(thrd);
            thrd->setSingleStepMode(true);
         }
         else {
            regular_threads.insert(thrd);
         }
      }
   }


   for (std::vector<Process::ptr>::iterator i = comp->procs.begin(); 
        i!= comp->procs.end(); i++) {
      Process::ptr proc = *i;
      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue process\n");
         myerror = true;
      }
   }

   syncloc loc[NUM_PARALLEL_PROCS];
   bool result = comp->recv_broadcast((unsigned char *) loc, sizeof(syncloc));
   if (!result) {
      logerror("Failed to recieve sync broadcast\n");
      myerror = true;
   }
   for (unsigned j=0; j<comp->procs.size(); j++) {
      if (loc[j].code != SYNCLOC_CODE) {
         logerror("Recieved unexpected message code\n");
         myerror = true;
      }
   }

   std::set<Thread::ptr>::iterator i;
   for (i = singlestep_threads.begin(); i != singlestep_threads.end(); i++)
   {
      thread_info &ti = tinfo[*i];
      if (ti.steps == 0) {
         logerror("Thread did not recieve any single step events\n");
         myerror = true;
      }
      for (unsigned j = 0; j < NUM_FUNCS; j++) {
         if (j > STOP_FUNC) {
            if (ti.hit_funcs[j] != -1)
            {
               logerror("Stop function was single stepped\n");
               myerror = true;
            }
            continue;
         }
         if (ti.hit_funcs[j] == -1) {
            logerror("Function entry was not singlestepped over\n");
            myerror = true;
         }
         if (j == BP_FUNC) {
            /**
             * The BP_FUNC function and breakpoint are at the same place.
             * We can legally recieve them in any order.
             **/
            if (ti.breakpoint == -1) {
               logerror("Function did not execute breakpoint\n");
               myerror = true;
            }
            if (!((ti.hit_funcs[j] == j && ti.breakpoint == j+1) ||
                  (ti.hit_funcs[j] == j+1 && ti.breakpoint == j)))
            {
               logerror("Breakpoint or function was executed out of order\n");
               myerror = true;
            }
            continue;
         }
         int expected = 0;
         if (j < BP_FUNC) {
            expected = j;
         }
         else if (j > BP_FUNC) {
            expected = j+1;
         }
         if (j != BP_FUNC && ti.hit_funcs[j] != expected) {
            logerror("Function was executed out of order\n");
            myerror = true;
         }
      }
   }
   for (i = regular_threads.begin(); i != regular_threads.end(); i++)
   {
      thread_info &ti = tinfo[*i];
      if (ti.steps != 0) {
         logerror("Regular thread had single steps.\n");
         myerror = true;
      }
      if (ti.breakpoint != 0) {
         logerror("Regular thread did not execute breakpoint\n");
         myerror = true;
      }
      for (unsigned j = 0; j < NUM_FUNCS; j++) {
         if (ti.hit_funcs[j] != -1) {
            logerror("Thread singlestepped over function\n");
            myerror = true;
         }
      }
   }

   Process::removeEventCallback(on_singlestep);
   Process::removeEventCallback(on_breakpoint);

   return myerror ? FAILED : PASSED;
}
