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

class pc_temp_detachMutator : public ProcControlMutator {
public:
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* pc_temp_detach_factory()
{
  return new pc_temp_detachMutator();
}

static bool event_source_error = false;
static bool not_expecting_event = true;
Process::cb_ret_t on_event_source_event(Event::const_ptr ev)
{
   if( not_expecting_event ) {
       //We should not see any of the signals if the detach worked
       logerror("Error. Received event source event\n");
       event_source_error = true;
   }

   return Process::cbThreadContinue;
}

#if !defined(os_windows_test)
static EventType::Code eventSourceEventType = EventType::Signal;
#else
// TODO specify event created by the event source on Windows
static EventType::Code eventSourceEventType = EventType::Unset;
#endif

// Basic test for temporary detach 
// 
// The mutator temporarily detaches from the mutatee and the
// mutatee generates events periodically. The mutator re-attaches
// when the mutatee has finished generating events and the mutator
// reads some state.
test_results_t pc_temp_detachMutator::executeTest()
{
   std::vector<Process::ptr>::iterator i;
   bool error = false;

   Process::registerEventCallback(eventSourceEventType, on_event_source_event);

   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      bool result = proc->continueProc();
      if (!result) {
          logerror("Failed to continue processes\n");
          error = true;
          continue;
      }

      result = proc->temporaryDetach();
      if (!result) {
         logerror("Failed to temporarily detach from processes\n");
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

   // Receive address of event source counter
   send_addr addrs[NUM_PARALLEL_PROCS];
   unsigned j = 0;
   for(i = comp->procs.begin(); i != comp->procs.end(); ++i, ++j ) {
       Process::ptr proc = *i;
       result = comp->recv_message((unsigned char *) &addrs[j], sizeof(send_addr), proc);
       if( !result ) {
           logerror("Failed to receive sync broadcast\n");
           return FAILED;
       }
   }

   // Wait for mutatees to signal attach
   syncloc sync_points[NUM_PARALLEL_PROCS];
   result = comp->recv_broadcast((unsigned char *) sync_points, sizeof(syncloc));
   if (!result) {
      logerror("Failed to receive sync broadcast\n");
      return FAILED;
   }

   j = 0;
   for (i = comp->procs.begin(); i != comp->procs.end(); i++, j++) {
      if (sync_points[j].code != SYNCLOC_CODE) {
         logerror("Recieved unexpected sync message\n");
         return FAILED;
      }
   }

   // Check that we didn't receive any events while detached
   if( comp->poll_for_events() ) {
       logerror("Events were received while detached\n");
       error = true;
   }

   // The event source could still be delivering events after we re-attach, this is okay
   not_expecting_event = false;

   // Re-attach to all the processes and read the value of the event counter
   j = 0;
   for(i = comp->procs.begin(); i != comp->procs.end(); ++i, ++j ) {
       Process::ptr proc = *i;
       do {
           if( !proc->reAttach() ) {
               logerror("Failed to re-attach to processes\n");
               error = true;
               break;
           }

           uint64_t eventCount = 0;
           if( !proc->readMemory(&eventCount, (Dyninst::Address)addrs[j].addr, sizeof(eventCount)) ) {
               logerror("Failed to read event counter from process memory\n");
               error = true;
               break;
           }

           if( eventCount == 0 ) {
               logerror("Mutatee did not generate any events, detach maybe failed?\n");
               error = true;
               break;
           }

           if( !proc->continueProc() ) {
               logerror("Failed to continue process\n");
               error = true;
               break;
           }
       }while(0);
   }

   // Tell the mutatees its okay to exit
   result = comp->send_broadcast((unsigned char *) &sync_point, sizeof(syncloc));
   if (!result) {
      logerror("Failed to send sync broadcast\n");
      return FAILED;
   }

   if (event_source_error) error = true;
   Process::removeEventCallback(eventSourceEventType);

   return error ? FAILED : PASSED;
}
