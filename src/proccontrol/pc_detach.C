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

class pc_detachMutator : public ProcControlMutator {
public:
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* pc_detach_factory()
{
  return new pc_detachMutator();
}

static bool signal_error = false;
Process::cb_ret_t on_signal(Event::const_ptr ev)
{
   //We should not see any of the signals if the detach worked
   logerror("Error. Recieved signal\n");
   signal_error = true;
   return Process::cbThreadContinue;
}

test_results_t pc_detachMutator::executeTest()
{
	logerror("Begin detachMutator::executeTest\n");
   std::vector<Process::ptr>::iterator i;
   bool error = false;
   comp->curgroup_self_cleaning = true;
   
   Process::registerEventCallback(EventType::Signal, on_signal);

   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
 	  logerror("Continuing process %d\n", proc->getPid());
      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue process\n");
         error = true;
      }
   }

   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
 	  logerror("Detaching from process %d\n", proc->getPid());
      bool result = proc->detach();
      if (!result) {
         logerror("Failed to detach from processes\n");
         error = true;
      }
   }

   syncloc sync_point;
   sync_point.code = SYNCLOC_CODE;
   bool result = comp->send_broadcast((unsigned char *) &sync_point, sizeof(syncloc));
   if (!result) {
      logerror("Failed to send sync broadcast\n");
      return FAILED;
   }
   logerror("Sent sync broadcast, waiting for response\n");

   syncloc sync_points[NUM_PARALLEL_PROCS];
   result = comp->recv_broadcast((unsigned char *) sync_points, sizeof(syncloc));
   if (!result) {
      logerror("Failed to recieve sync broadcast\n");
      return FAILED;
   }

   unsigned j = 0;
   for (i = comp->procs.begin(); i != comp->procs.end(); i++, j++) {
      if (sync_points[j].code != SYNCLOC_CODE) {
	logerror("Received unexpected sync message: 0x%lx instead of 0x%lx\n", sync_points[j].code, SYNCLOC_CODE);
         return FAILED;
      }
   }

   if (signal_error)
      error = true;
   Process::removeEventCallback(EventType::Signal);

   return error ? FAILED : PASSED;
}


