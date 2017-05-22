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
#include "Event.h"
#include "SymtabReader.h"

#include <stdio.h>

#include <set>

using namespace std;

Breakpoint::ptr rw_bp;
Breakpoint::ptr r_bp;
Breakpoint::ptr w_bp;
Breakpoint::ptr x_bp;

class pc_hw_breakpointMutator : public ProcControlMutator {
public:
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* pc_hw_breakpoint_factory()
{
   return new pc_hw_breakpointMutator();
}

#define NUM_BPS 8
#define NUM_BREAKPOINT_SPINS 8

typedef struct {
   Address addrs[NUM_BPS];
   bool bp_active[NUM_BPS];
   bool bp_run[NUM_BPS];
   unsigned times_hit[NUM_BPS];
} pchw_proc_data_t;

static Breakpoint::ptr bps[NUM_BPS];
static unsigned int bp_modes[NUM_BPS];
static unsigned int bp_sizes[NUM_BPS];
unsigned expected[NUM_BPS];
static std::map<Process::const_ptr, pchw_proc_data_t> procdata;
static bool had_error = false;

static Process::cb_ret_t on_breakpoint(Event::const_ptr ev)
{
   EventBreakpoint::const_ptr ev_bp = ev->getEventBreakpoint();
   pchw_proc_data_t &pdata = procdata[ev->getProcess()];

   std::vector<Breakpoint::const_ptr> all_bps;
   ev_bp->getBreakpoints(all_bps);
   if (all_bps.size() != 1) {
      logerror("Unexpected size of breakpoints in callback\n");
      had_error = true;
      return Process::cbThreadContinue;
   }
   Breakpoint::const_ptr bp = all_bps[0];
   
   unsigned int bp_num;
   for (bp_num = 0; bp_num < NUM_BPS && bps[bp_num] != bp; bp_num++);
   if (bp_num == NUM_BPS) {
      logerror("Received unexpected BP in callback\n");
      had_error = true;
      return Process::cbThreadContinue;
   }
   
   pdata.times_hit[bp_num]++;
   return Process::cbThreadContinue;
}

test_results_t pc_hw_breakpointMutator::executeTest()
{
   had_error = false;
   std::vector<Process::ptr>::iterator i;
   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue process\n");
         return FAILED;
      }
   }

   EventType event_bp(EventType::Breakpoint);
   Process::registerEventCallback(event_bp, on_breakpoint);
   unsigned expected_hits = (comp->num_threads+1) * NUM_BREAKPOINT_SPINS;

   /**
    * Initialize data structures
    **/
   procdata.clear();
   bp_modes[0] = Breakpoint::BP_R | Breakpoint::BP_W;
   bp_modes[1] = Breakpoint::BP_R;
   bp_modes[2] = Breakpoint::BP_W;
   bp_modes[3] = Breakpoint::BP_X;
   bp_modes[4] = Breakpoint::BP_R | Breakpoint::BP_W | Breakpoint::BP_X;
   bp_modes[5] = Breakpoint::BP_R;
   bp_modes[6] = Breakpoint::BP_W;
   bp_modes[7] = Breakpoint::BP_X;
   bp_sizes[0] = 4;
   bp_sizes[1] = 4;
   bp_sizes[2] = 4;
   bp_sizes[3] = 4;
   bp_sizes[4] = 4;
#if defined(arch_64bit_test)
   bp_sizes[5] = 8;
   bp_sizes[6] = 8;
#else
   bp_sizes[5] = 4;
   bp_sizes[6] = 4;
#endif
   bp_sizes[7] = 1;

   for (unsigned j=0; j<NUM_BPS; j++) {
      bps[j] = Breakpoint::newHardwareBreakpoint(bp_modes[j], bp_sizes[j]);
      if (!bps[j]) {
         logerror("Could not create BP\n");
         Process::removeEventCallback(event_bp);
         return FAILED;
      }
      expected[j] = 0;
      if (bp_modes[j] & Breakpoint::BP_R)
         expected[j] += expected_hits;
      if (bp_modes[j] & Breakpoint::BP_W)
         expected[j] += expected_hits;
      if (bp_modes[j] & Breakpoint::BP_X)
         expected[j] += expected_hits;
   }
   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      pchw_proc_data_t &pdata = procdata[proc];

      //Recv message that mutatee is ready
      for (unsigned j=0; j<NUM_BPS; j++) {
         send_addr addrmsg;
         bool result = comp->recv_message((unsigned char *) &addrmsg, sizeof(send_addr), proc);
         if (!result || addrmsg.code != SENDADDR_CODE) {
            logerror("Failed to recieve address message from process\n");
            Process::removeEventCallback(event_bp);
            return FAILED;
         }

         pdata.addrs[j] = comp->adjustFunctionEntryAddress(proc, addrmsg.addr);
         pdata.bp_active[j] = false;
         pdata.bp_run[j] = false;
         pdata.times_hit[j] = 0;
      }
   }

   for (unsigned j=0; j<NUM_BPS; j++) {
      //Stop all processes
      for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
         Process::ptr proc = *i;
         bool result = proc->stopProc();
         if (!result) {
            logerror("Failed to stop process\n");
            Process::removeEventCallback(event_bp);
            return FAILED;
         }
      }
      
      //Clear space for new HW breakpoints
      for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
         Process::ptr proc = *i;
         pchw_proc_data_t &pdata = procdata[proc];

         while (proc->numHardwareBreakpointsAvail(bp_modes[j]) == 0) {
            bool cleared_something = false;
            for (unsigned k=0; k<j; k++) {
               if (!pdata.bp_active[k])
                  continue;
               assert(pdata.bp_run[k]);
               bool result = proc->rmBreakpoint(pdata.addrs[k], bps[k]);
               if (!result) {
                  logerror("Failed to rmBreakpoint\n");
                  Process::removeEventCallback(event_bp);
                  return FAILED;
               }
               pdata.bp_active[k] = false;
               cleared_something = true;
               break;
            }
            if (!cleared_something) {
               logerror("No space for breakpoint and nothing to clear\n");
               Process::removeEventCallback(event_bp);
               return FAILED;
            }
         }
      }

      //Insert new HW breakpoints
      for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
         Process::ptr proc = *i;
         pchw_proc_data_t &pdata = procdata[proc];

         assert(!pdata.bp_run[j] && !pdata.bp_active[j]);
         if (pdata.times_hit[j]) {
            logerror("Breakpoint %d prematurely hit on process\n", (int) j);
            Process::removeEventCallback(event_bp);
            return FAILED;
         }

         bool result = proc->addBreakpoint(pdata.addrs[j], bps[j]);
         if (!result) {
            logerror("Failed to insert HW breakpoint\n");
            Process::removeEventCallback(event_bp);
            return FAILED;
         }
         pdata.bp_active[j] = true;
      }

      //Signal processes 
      syncloc sync_point;
      sync_point.code = SYNCLOC_CODE;
      bool result = comp->send_broadcast((unsigned char *) &sync_point, sizeof(syncloc));
      if (!result) {
         logerror("Failed to send sync broadcast\n");
         Process::removeEventCallback(event_bp);
         return FAILED;
      }

      //Continue processes
      for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
         Process::ptr proc = *i;
         bool result = proc->continueProc();
         if (!result) {
            logerror("Failed to continue a process\n");
            Process::removeEventCallback(event_bp);
            return FAILED;
         }
      }

      //Recv sync
      syncloc locs[NUM_PARALLEL_PROCS];
      result = comp->recv_broadcast((unsigned char *) locs, sizeof(syncloc));
      if (!result) {
         logerror("Failed to recieve sync broadcast\n");
         Process::removeEventCallback(event_bp);
         return FAILED;
      }
      for (unsigned k=0; k<comp->procs.size(); k++) {
         if (locs[k].code != SYNCLOC_CODE) {
            logerror("Received unexpected message code\n");
            Process::removeEventCallback(event_bp);
            return FAILED;
         }
      }

      //Check results
      if (had_error) {
         logerror("Error in callback\n");
         Process::removeEventCallback(event_bp);
         return FAILED;
      }
      for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
         Process::ptr proc = *i;
         pchw_proc_data_t &pdata = procdata[proc];
         pdata.bp_run[j] = true;
         if (pdata.times_hit[j] != expected[j]) {
            logerror("Breakpoint %u ran unexpected number of times %u != %u\n", j, pdata.times_hit[j], expected[j]);
            Process::removeEventCallback(event_bp);
            return FAILED;
         }
      }
   }

   Process::removeEventCallback(event_bp);
   //Recheck that no BPS got extra executions
   for (unsigned j=0; j<NUM_BPS; j++) {
      for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
         Process::ptr proc = *i;
         pchw_proc_data_t &pdata = procdata[proc];
         if (pdata.times_hit[j] != expected[j]) {
            logerror("Breakpoint re-ran unexpectedly %u != %u\n", pdata.times_hit, expected[j]);
            return FAILED;
         }
      }
   }
     
   return PASSED;
}


