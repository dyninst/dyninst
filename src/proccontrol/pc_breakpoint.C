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
#include "PCProcess.h"
#include "SymtabReader.h"
#include "Event.h"

#include <stdio.h>

#include <set>

using namespace std;

class pc_breakpointMutator : public ProcControlMutator {
public:
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* pc_breakpoint_factory()
{
   return new pc_breakpointMutator();
}

#define NUM_BREAKPOINTS 4
#define NUM_BREAKPOINT_SPINS 16

Dyninst::Address bp_addrs[NUM_PARALLEL_PROCS][NUM_BREAKPOINTS];
Breakpoint::ptr bps[NUM_PARALLEL_PROCS][NUM_BREAKPOINTS];
std::pair<unsigned, unsigned> indexes[NUM_PARALLEL_PROCS*NUM_BREAKPOINTS];
unsigned cur_index;
std::map<Thread::const_ptr, unsigned> hit_counts;
unsigned num_breakpoints_hit;
bool haserror = false;
unsigned my_num_processes;

Process::cb_ret_t on_breakpoint(Event::const_ptr ev)
{
   num_breakpoints_hit++;
   EventBreakpoint::const_ptr evbp = ev->getEventBreakpoint();
   if (!evbp) {
      logerror("Error, recieved event that wasn't a breakpoint\n");
      haserror = true;
      return Process::cbProcContinue;
   }

   Dyninst::Address addr = evbp->getAddress();
   std::vector<Breakpoint::const_ptr> evbps;
   evbp->getBreakpoints(evbps);
   if (evbps.size() != 1) {
      logerror("Unexpected number of breakpoint objects\n");
      haserror = true;
      return Process::cbProcContinue;
   }
   Breakpoint::const_ptr bp = evbps[0];

   std::pair<unsigned, unsigned> *index = (std::pair<unsigned, unsigned> *) bp->getData();
   if (!index) {
      logerror("Breakpoint does not have associated data\n");
      haserror = true;
      return Process::cbProcContinue;
   }
   if (index->first >= my_num_processes) {
      logerror("Invalid proc index\n");
      haserror = true;
      return Process::cbProcContinue;
   }
   if (index->second >= NUM_BREAKPOINTS) {
      logerror("Invalid breakpoint index\n");
      haserror = true;
      return Process::cbProcContinue;
   }
  
   if (bps[index->first][index->second] != bp) {
      logerror("Unexpected breakpoint pointer for point\n");
      haserror = true;
      return Process::cbProcContinue;
   }
  
   if (bp_addrs[index->first][index->second] != addr) {
      logerror("Address did not match expected breakpoint\n");
      haserror = true;
      return Process::cbProcContinue;
   }

   Thread::const_ptr cur_thread = ev->getThread();
   std::map<Thread::const_ptr, unsigned>::iterator i = hit_counts.find(cur_thread);
   if (i == hit_counts.end()) {
      hit_counts[cur_thread] = 1;
   }
   else {
      hit_counts[cur_thread]++;
   }
   //logerror("mutator breakpoint callback, hit_counts[0x%lx] = %d\n", cur_thread, (hit_counts[cur_thread]));

   return Process::cbProcContinue;
}

test_results_t pc_breakpointMutator::executeTest()
{
   haserror = false;
   cur_index = 0;
   num_breakpoints_hit = 0;
   hit_counts.clear();
   memset(indexes, 0, sizeof(indexes));
   memset(bp_addrs, 0, sizeof(bp_addrs));
   my_num_processes = comp->num_processes;

   for (unsigned i=0; i<my_num_processes; i++) {
      for (unsigned j=0; j<NUM_BREAKPOINTS; j++) {
         bps[i][j] = Breakpoint::ptr();
      }
   }

   std::vector<Process::ptr>::iterator i;
   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue process\n");
         return FAILED;
      }
   }

   unsigned j;
   for (i = comp->procs.begin(), j = 0; i != comp->procs.end(); i++, j++) {
      Process::ptr proc = *i;
      send_addr addrmsg;

      for (unsigned k = 0; k < NUM_BREAKPOINTS; k++) {
         bool result = comp->recv_message((unsigned char *) &addrmsg, sizeof(send_addr), proc);
         if (!result) {
            logerror("Failed to recieve address message from process\n");
            return FAILED;
         }
         if (addrmsg.code != SENDADDR_CODE) {
            logerror("Recieved unexpected message instead of address message\n");
            return FAILED;
         }
       
         bp_addrs[j][k] = comp->adjustFunctionEntryAddress(proc, addrmsg.addr);
      }  
     
      bool result = proc->stopProc();
      if (!result) {
         logerror("Failed to stop process\n");
         return FAILED;
      }

      for (unsigned k = 0; k < NUM_BREAKPOINTS; k++) {

         bps[j][k] = Breakpoint::newBreakpoint();
         indexes[cur_index].first = j;
         indexes[cur_index].second = k;
         bps[j][k]->setData(indexes+cur_index);
         cur_index++;
         bool result = proc->addBreakpoint(bp_addrs[j][k], bps[j][k]);
         if (!result) {
            logerror("Failed to insert breakpoint\n");
            return FAILED;
         }
      }
   }

   EventType event_bp(EventType::Breakpoint);
   Process::registerEventCallback(event_bp, on_breakpoint);
   
   syncloc sync_point;
   sync_point.code = SYNCLOC_CODE;
   bool result = comp->send_broadcast((unsigned char *) &sync_point, sizeof(syncloc));
   if (!result) {
      logerror("Failed to send sync broadcast\n");
      return FAILED;
   }

   for (i = comp->procs.begin(), j = 0; i != comp->procs.end(); i++, j++) {
      Process::ptr proc = *i;
      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue a process\n");
         return FAILED;
      }
   }

   int total_breakpoints;
   if (comp->num_threads)
      total_breakpoints = NUM_BREAKPOINTS * NUM_BREAKPOINT_SPINS * comp->num_processes * comp->num_threads;
   else
      total_breakpoints = NUM_BREAKPOINTS * NUM_BREAKPOINT_SPINS * comp->num_processes;

   while (num_breakpoints_hit < total_breakpoints) {
      bool result = comp->block_for_events();
      if (!result) {
         logerror("Failed to handle events\n");
         return FAILED;
      }
	  logerror("%d/%d breakpoints hit OK\n", num_breakpoints_hit, total_breakpoints);
   }

   std::map<Thread::const_ptr, unsigned>::iterator l;
   for (l = hit_counts.begin(); l != hit_counts.end(); l++) {
      if (l->second != NUM_BREAKPOINT_SPINS * NUM_BREAKPOINTS) {
         logerror("Unexpected number of breakpoints hit on thread\n");
         return FAILED;
      }
   }
     
   return haserror ? FAILED : PASSED;
}


