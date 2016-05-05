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
#include "SymtabReader.h"

using namespace std;

class pc_groupsMutator : public ProcControlMutator {
private:
   bool error;
   AddressSet::ptr data_loc;
   AddressSet::ptr bp_loc;
   AddressSet::ptr free_loc;
   AddressSet::ptr toc_loc;
   ProcessSet::ptr pset;   
   Breakpoint::ptr bp;
public:
   virtual test_results_t executeTest();
   void waitfor_sync();
   void trigger_sync();
   AddressSet::ptr getAddresses(ProcessSet::ptr pset, bool isFunctionAddress);
#if defined(os_windows_test)
   AddressSet::ptr getFreeAddresses(ProcessSet::ptr pset);
#endif

   bool readMemoryTest(uint64_t value, AddressSet::ptr aset);
   bool writeMemoryTest(uint64_t value, AddressSet::ptr aset);
};

static unsigned int num_bp_events = 0;

extern "C" DLLEXPORT TestMutator* pc_groups_factory()
{
  return new pc_groupsMutator();
}

Process::cb_ret_t on_bp(Event::const_ptr ev) {
   num_bp_events++;
   return Process::cbProcContinue;
}

void pc_groupsMutator::waitfor_sync() {
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

AddressSet::ptr pc_groupsMutator::getAddresses(ProcessSet::ptr pset, bool isFunctionAddress) {
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

      if (isFunctionAddress)
         addr.addr = comp->adjustFunctionEntryAddress(p, addr.addr);
      aset->insert(addr.addr, p);
   }
   return aset;
}

#if defined(os_windows_test)
AddressSet::ptr pc_groupsMutator::getFreeAddresses(ProcessSet::ptr pset) {
  AddressSet::ptr aset = AddressSet::newAddressSet();
   
   for (ProcessSet::iterator i = pset->begin(); i != pset->end(); i++) {
      Process::ptr p = *i;

	  Dyninst::Address result = p->findFreeMemory(sizeof(uint64_t));
      
	  aset->insert(result, p);
   }
   return aset;
}
#endif /* Windows-only code */

bool pc_groupsMutator::writeMemoryTest(uint64_t value, AddressSet::ptr aset)
{
   size_t data_size = sizeof(uint64_t);
   
   //Write through address set
   {
      bool result = pset->writeMemory(aset, &value, data_size);
      if (!result) {
         logerror("Failure writing memory\n");
         return false;
      }
   }

   //non-uniform write
   {
      multimap<Process::const_ptr, ProcessSet::write_t> mem_locs;
      for (AddressSet::iterator i = aset->begin(); i != aset->end(); i++) {
         ProcessSet::write_t w;
         w.buffer = &value;
         w.addr = i->first;
         w.size = data_size;
         w.err = err_none;
         mem_locs.insert(make_pair(i->second, w));
      }
      bool result = pset->writeMemory(mem_locs);
      if (!result) {
         logerror("Failure during non-uniform write\n");
         return false;
      }
   }
   return true;
}

bool pc_groupsMutator::readMemoryTest(uint64_t value, AddressSet::ptr aset)
{
   size_t data_size = sizeof(uint64_t);

   //Read from uniform size data
   {
      multimap<Process::ptr, void *> mem_results;
      bool result = pset->readMemory(aset, mem_results, data_size);
      if (!result) {
         logerror("Failed to read memory\n");
         return false;
      }
      ProcessSet::ptr temp_pset = ProcessSet::newProcessSet();
      for (multimap<Process::ptr, void *>::iterator i = mem_results.begin(); i != mem_results.end(); i++) {
         Process::ptr proc = i->first;
         uint64_t val = *((uint64_t *) i->second);
         
         if (val != value) {
            logerror("Read wrong value from memory: %lu\n", val);
            return false;
         }
         free(i->second);
         temp_pset->insert(proc);
      }
      if (!pset->set_difference(temp_pset)->empty() ||
          !temp_pset->set_difference(pset)->empty()) 
      {
         logerror("Read returned incorrect set\n");
         return false;
      }
   }

   //Aggregation read
   for (unsigned i=0; i<=1; i++) {
      bool do_checksum = (i == 0);
      
      std::map<void *, ProcessSet::ptr> mem_results;
      bool result = pset->readMemory(aset, mem_results, data_size, do_checksum);
      if (!result) {
         logerror("Failed to read memory aggregation\n");
         return false;
      }
      if (mem_results.size() != 1) {
         logerror("Did not properly aggregate memory results\n");
         return false;
      }
      void *data = mem_results.begin()->first;
      ProcessSet::ptr temp_pset = mem_results.begin()->second;

      if (*((uint64_t *) data) != value) {
         logerror("Read wrong value from memory during aggregation read\n");
         return false;
      }
      free(data);
      if (!pset->set_difference(temp_pset)->empty() ||
          !temp_pset->set_difference(pset)->empty()) 
      {
         logerror("Read returned incorrect set durring aggregation read\n");
         return false;
      }
   }

   //Non-uniform read
   {
      multimap<Process::const_ptr, ProcessSet::read_t> mem_results;
      for (AddressSet::iterator i = aset->begin(); i != aset->end(); i++) {
         ProcessSet::read_t r;
         r.buffer = (void *) malloc(data_size);
         r.addr = i->first;
         r.size = data_size;
         r.err = err_none;
         mem_results.insert(make_pair(i->second, r));
      }
      bool result = pset->readMemory(mem_results);
      if (!result) {
         logerror("Failed to read memory in non-uniform read\n");
         return false;
      }
      ProcessSet::ptr temp_pset = ProcessSet::newProcessSet();
      for (multimap<Process::const_ptr, ProcessSet::read_t>::iterator i = mem_results.begin(); i != mem_results.end(); i++) {
         Process::const_ptr p = i->first;
         ProcessSet::read_t &r = i->second;
         
         
         if (*((uint64_t *) r.buffer) != value) {
            logerror("Read wrong value during read\n");
            return false;
         }
         free(r.buffer);
         temp_pset->insert(p);
      }
      if (!pset->set_difference(temp_pset)->empty() ||
          !temp_pset->set_difference(pset)->empty()) 
      {
         logerror("Read returned incorrect set durring aggregation read\n");
         return false;
      }      
   }

   return true;
}

void pc_groupsMutator::trigger_sync() {
   syncloc sync;
   sync.code = SYNCLOC_CODE;
   
   bool result = comp->send_broadcast((unsigned char *) &sync, sizeof(syncloc));
   if (!result) {
      logerror("Failed to send broadcast in group test\n");
      error = true;
   }
}

test_results_t pc_groupsMutator::executeTest()
{
   std::vector<Process::ptr>::iterator i;
   error = false;
   pset = comp->pset;

   bool result = pset->continueProcs();
   if (!result) {
      logerror("Failed to continue procs\n");
      return FAILED;
   }
   
   data_loc = getAddresses(pset, false);
   if (error)
      return FAILED;

   bp_loc = getAddresses(pset, true);
   if (error)
      return FAILED;
#if !defined(os_windows_test)
   free_loc = getAddresses(pset, false);
#endif
   if (error)
      return FAILED;

   waitfor_sync();
   if (error)
      return FAILED;

   result = pset->stopProcs();
   if (!result) {
      logerror("Failed to stop procs\n");
      return FAILED;
   }

   //Read memory expecting 4, write memory with 8, read memory expecting 4
   if (!readMemoryTest(4, data_loc)) {
      return FAILED;
   }
   if (!writeMemoryTest(8, data_loc)) {
      return FAILED;
   }
   if (!readMemoryTest(8, data_loc)) {
      return FAILED;
   }
   
   //Malloc memory at an address, go through read/write test
	// This _has_ to happen before the arbitrary alloc test,
   // otherwise we may allocate the addresses we just tried
   // to check...
#if defined(os_windows_test)
   free_loc = getFreeAddresses(pset);
#endif
   result = pset->mallocMemory(sizeof(uint64_t), free_loc);
   if (!result) {
      logerror("Failed to allocate memory\n");
      return FAILED;
   }
   result = writeMemoryTest(16, free_loc);
   if (!result) {
      logerror("Failed to write to allocated memory\n");
      return FAILED;
   }
   result = readMemoryTest(16, free_loc);
   if (!result) {
      logerror("Failed to read from allocated memory\n");
      return FAILED;
   }
   result = pset->freeMemory(free_loc);
   if (!result) {
      logerror("Failed to free memory in allocated region\n");
      return FAILED;
   }

   //Malloc memory, then go through read/write test again
   AddressSet::ptr mallocd_memory;
   mallocd_memory = pset->mallocMemory(sizeof(uint64_t));
   if (mallocd_memory->size() != pset->size()) {
      logerror("Failed to allocate memory\n");
      return FAILED;
   }
   result = writeMemoryTest(16, mallocd_memory);
   if (!result) {
      logerror("Failed to write to allocated memory\n");
      return FAILED;
   }
   result = readMemoryTest(16, mallocd_memory);
   if (!result) {
      logerror("Failed to read from allocated memory\n");
      return FAILED;
   }
   result = pset->freeMemory(mallocd_memory);
   if (!result) {
      logerror("Failed to free memory in allocated region\n");
      return FAILED;
   }


   //Add a breakpoint
   bp = Breakpoint::newBreakpoint();
   result = pset->addBreakpoint(bp_loc, bp);
   if (!result) {
      logerror("Failed to add breakpoint\n");
      return FAILED;
   }
   Process::registerEventCallback(EventType(EventType::Breakpoint), on_bp);
   num_bp_events = 0;

   trigger_sync();
   if (error)
      return FAILED;

   //Run process through breakpoint
   logerror("Continuing procs for breakpoint test\n");
   result = pset->continueProcs();

   if (!result) {
      logerror("Failed to continue proc\n");
      return FAILED;
   }

   waitfor_sync();
   if (error)
      return FAILED;
   logerror("Stopping procs for breakpoint test\n");
   result = pset->stopProcs();
   if (!result)
      return FAILED;

   unsigned int total_breakpoints;
   total_breakpoints = (comp->num_threads + 1) * comp->num_processes;
   if (num_bp_events != total_breakpoints) {
      logerror("Did not receive correct number of breakpoints %d != %d\n", num_bp_events, total_breakpoints);
      return FAILED;
   }

   //Remove breakpoint
   result = pset->rmBreakpoint(bp_loc, bp);
   if (!result) {
      logerror("Failed to rmBreakpoint\n");
      return FAILED;
   }
   
   //Run process through bp function again, this time nothing should hit
   result = pset->continueProcs();
   if (!result) {
      logerror("Failed to continue process\n");
      return FAILED;
   }
   trigger_sync();
   if (error)
      return FAILED;

   waitfor_sync();
   if (error)
      return FAILED;
   result = pset->stopProcs();
   if (!result)
      return FAILED;

   if (num_bp_events != total_breakpoints) {
      logerror("Breakpoint executed after removal\n");
      return FAILED;
   }

   
   
   //Run to completion.
   trigger_sync();
   if (error)
      return FAILED;
   result = pset->continueProcs();
   if (!result) {
      logerror("Failed final continue\n");
      return FAILED;
   }

   return error ? FAILED : PASSED;
}


