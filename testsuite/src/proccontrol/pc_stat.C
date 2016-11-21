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

#include "PlatFeatures.h"
#include "proccontrol_comp.h"
#include "communication.h"

#include "dyntypes.h"

using namespace std;

class pc_statMutator : public ProcControlMutator {
   bool error;
   ProcessSet::ptr pset;
   AddressSet::ptr stack_addr;
   ThreadSet::ptr all_threads;
   MachRegister stack_pointer;
   bool fakeStackwalk();
   bool takeSample();
   AddressSet::ptr getAddresses(ProcessSet::ptr pset);
public:
   virtual test_results_t executeTest();
   void waitfor_sync();
   void trigger_sync();
};

extern "C" DLLEXPORT TestMutator* pc_stat_factory()
{
  return new pc_statMutator();
}

AddressSet::ptr pc_statMutator::getAddresses(ProcessSet::ptr pset) {
   AddressSet::ptr aset = AddressSet::newAddressSet();
   
   for (ProcessSet::iterator i = pset->begin(); i != pset->end(); i++) {
      Process::ptr p = *i;
      send_addr addr;
      bool result = comp->recv_message((unsigned char *) &addr, sizeof(send_addr), p);
      if (!result) {
         logerror("Failed to recv address\n");
         error = true;
         return AddressSet::ptr();
      }
      if (addr.code != SENDADDR_CODE) {
         logerror("Received bad addr message in group test\n");
         error = true;
         return AddressSet::ptr();
      }
      aset->insert(addr.addr, p);
   }
   return aset;
}

static AddressSet::ptr spin_addrs;

void pc_statMutator::waitfor_sync() {
   syncloc *syncs;
   unsigned int size = sizeof(syncloc) * comp->num_processes;
   syncs = (syncloc *) malloc(size);
   memset(syncs, 0, size);

   bool result = comp->recv_broadcast((unsigned char *) syncs, sizeof(syncloc));
   if (!result) {
      logerror("Failed to recv sync in group test\n");
      error = true;
   }
   
   for (unsigned int i = 0; i < comp->num_processes; i++) {
      if (syncs[i].code != SYNCLOC_CODE) {
         logerror("Received bad syncloc message in group test\n");
         error = true;
      }
   }
   free(syncs);
}

void pc_statMutator::trigger_sync() {
   syncloc sync;
   sync.code = SYNCLOC_CODE;
   
   bool result = comp->send_broadcast((unsigned char *) &sync, sizeof(syncloc));
   if (!result) {
      logerror("Failed to send broadcast in group test\n");
      error = true;
   }
}

bool pc_statMutator::fakeStackwalk()
{
   //Just read a bunch of bytes from the stack and registers to simulate walking
   //  Get all registers, then read memory from stack pointers.
   map<Thread::ptr, RegisterPool> all_registers;
   bool result = all_threads->getAllRegisters(all_registers);
   if (!result) {
      logerror("Failed to read all registers\n");
      return false;
   }
   unsigned int expected_threads = comp->num_processes * (comp->num_threads+1);
   if (all_registers.size() != expected_threads) {
      logerror("Unexpected number of threads %lu != %u\n", (unsigned long) all_registers.size(), expected_threads);
      return false;
   }

   AddressSet::ptr stack_locs = AddressSet::newAddressSet();
   for (map<Thread::ptr, RegisterPool>::iterator i = all_registers.begin(); i != all_registers.end(); i++)
   {
      Thread::ptr thr = i->first;
      Process::ptr proc = thr->getProcess();

      const RegisterPool &rp = i->second;
      RegisterPool::const_iterator j = rp.find(stack_pointer);
      if (j == rp.end()) {
         logerror("Register set did not contain stack pointer\n");
         return false;
      }
      MachRegisterVal val = (*j).second;
      stack_locs->insert(val, proc);
   }

   std::multimap<Process::ptr, void *> read_results;
   result = pset->readMemory(stack_locs, read_results, 8);
   if (!result) {
      logerror("Failed to read memory from process set\n");
      return false;
   }
   if (read_results.size() != expected_threads) {
      logerror("Read wrong number of objects\n");
      return false;
   }
   
   return true;
}

class StackCallbackTest : public CallStackCallback
{
public:
   ThreadSet::ptr begin_set;
   ThreadSet::ptr frame_set;
   ThreadSet::ptr end_set;

   StackCallbackTest() {
      begin_set = ThreadSet::newThreadSet();
      frame_set = ThreadSet::newThreadSet();
      end_set = ThreadSet::newThreadSet();
   }
   
   ~StackCallbackTest() {
   }

   virtual bool beginStackWalk(Thread::ptr thr) {
      begin_set->insert(thr);
      return true;
   }

   virtual bool addStackFrame(Thread::ptr thr, Address ra, Address sp, Address fp) {
      logerror("Called addStackFrame - %lx, %lx, %lx\n", ra, sp, fp);
      frame_set->insert(thr);
      return true;
   }
   
   virtual void endStackWalk(Thread::ptr thr) {
      end_set->insert(thr);
   }
};

bool pc_statMutator::takeSample() 
{
   bool result = pset->stopProcs();
   if (!result) {
      logerror("Failure to stop processes before sample\n");
      return false;
   }

   Process::ptr a_proc = *pset->begin();
   stack_pointer = MachRegister::getStackPointer(a_proc->getArchitecture());


   if (pset->getLibraryTracking()) {
      result = pset->getLibraryTracking()->refreshLibraries();
      if (!result) {
         logerror("Failure refreshing libraries\n");
         return false;
      }
   }

   if (pset->getLWPTracking()) {
      result = pset->getLWPTracking()->refreshLWPs();
      if (!result) {
         logerror("Failure refreshing LWPs\n");
         return false;
      }
   }
      
   
   all_threads = ThreadSet::newThreadSet(pset);
   CallStackUnwindingSet *stkset = all_threads->getCallStackUnwinding();
   if (stkset) {
      StackCallbackTest cb_test;
      result = stkset->walkStack(&cb_test);
      if (!result) {
         logerror("Failue to collect stackwalks\n");
         return false;
      }
      if (!all_threads->set_difference(cb_test.begin_set)->empty() || 
          !cb_test.begin_set->set_difference(all_threads)->empty()) {
         logerror("Begin set does not contain all threads\n");
         return false;
      }
      if (!all_threads->set_difference(cb_test.frame_set)->empty() || 
          !cb_test.frame_set->set_difference(all_threads)->empty()) {
         logerror("Frame set does not contain all threads\n");
         return false;
      }
      if (!all_threads->set_difference(cb_test.end_set)->empty() || 
          !cb_test.end_set->set_difference(all_threads)->empty()) {
         logerror("End set does not contain all threads\n");
         return false;
      }
   }
   else {
      fakeStackwalk();
   }

   uint32_t one = 1;
   result = pset->writeMemory(spin_addrs, &one, sizeof(one));
   if (!result) {
      logerror("Error writing memory to processes\n");
      error = true;
   }

   result = pset->continueProcs();
   if (!result) {
      logerror("Failure to stop processes before sample\n");
      return false;
   }

   return true;
}

test_results_t pc_statMutator::executeTest()
{
   error = false;
   pset = comp->pset;

   spin_addrs = getAddresses(pset);
   if (error || spin_addrs->size() != comp->num_processes) {
      logerror("Error getting addresses from mutatee\n");
      return FAILED;
   }

   for (unsigned i=0; i < 10; i++) {
      waitfor_sync();
      if (error)
         return FAILED;

      bool result = takeSample();
      if (!result) {
         logerror("Sample error\n");
         return FAILED;
      }
   }

   return error ? FAILED : PASSED;
}


