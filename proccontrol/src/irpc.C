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

#include "PCErrors.h"
#include "Event.h"

#include "irpc.h"
#include "response.h"
#include "int_process.h"
#include "int_event.h"
#include "int_handler.h"
#include "Mailbox.h"
#include "procpool.h"

// CLEANUP
#if defined(os_windows)
#include "windows_process.h"
#endif

#include <cstring>
#include <cassert>
#include <iostream>

using namespace std;
unsigned long int_iRPC::next_id;

int_iRPC::int_iRPC(void *binary_blob_,
                   unsigned long binary_size_,
                   bool async_,
                   bool alreadyAllocated,
                   Dyninst::Address addr) :
   state(Unassigned),
   type(User),
   binary_blob(binary_blob_),
   binary_size(binary_size_),
   start_offset(0),
   thrd(NULL),
   inffree_target(0),
   async(async_),
   freeBinaryBlob(false),
   restore_internal(false),
   counted_sync(false),
   malloc_result(0),
   restore_at_end(int_thread::none),
   directFree_(false),
   user_data(NULL)
{
   my_id = next_id++;
   if (alreadyAllocated) {
      cur_allocation = iRPCAllocation::ptr(new iRPCAllocation());
      cur_allocation->addr = addr;
      cur_allocation->size = binary_size;
   }
}

int_iRPC::~int_iRPC()
{
   if (freeBinaryBlob) {
      free(binary_blob);
      binary_blob = NULL;
   }
}

bool int_iRPC::isRPCPrepped()
{
   if (state == Prepped)
      return true;

   assert(state == Prepping);
	// Ephemeral thread doesn't exist, so it doesn't need to stop.
   if(thrd->isRPCEphemeral())
	   return true;
   if (isProcStopRPC()) {
      int_threadPool *pool = thrd->llproc()->threadPool();
      for (int_threadPool::iterator i = pool->begin(); i != pool->end(); i++) {
         if (!(*i)->isStopped(int_thread::IRPCSetupStateID) &&
             (*i)->getActiveState().getID() != int_thread::IRPCStateID)
         {
            return false;
         }
      }
      return true;
   }
   else {
      return thrd->isStopped(int_thread::IRPCSetupStateID);
   }
}

void int_iRPC::setState(int_iRPC::State s)
{
   int old_state = (int) state;
   int new_state = (int) s;
   if(new_state < old_state)
   {
     assert(!"Illegal state reversion");
     return;
   }
   state = s;
}

void int_iRPC::setType(int_iRPC::Type t)
{
   type = t;
}

void int_iRPC::setBinaryBlob(void *b)
{
   binary_blob = b;
}

void int_iRPC::setBinarySize(unsigned long s)
{
   binary_size = s;
}

void int_iRPC::copyBinaryBlob(void *b, unsigned long s)
{
   if(binary_blob && freeBinaryBlob) free(binary_blob);
   binary_blob = malloc(s);
   assert(binary_blob);
   binary_size = s;
   memcpy(binary_blob, b, s);
   freeBinaryBlob = true;
}

void int_iRPC::setStartOffset(unsigned long s)
{
   start_offset = s;
}

void int_iRPC::setAsync(bool a)
{
   async = a;
}

void int_iRPC::setThread(int_thread *t)
{
   thrd = t;
}

void int_iRPC::setIRPC(IRPC::weak_ptr o)
{
   hl_irpc = o;
}

void int_iRPC::setAllocSize(unsigned long s)
{
  cur_allocation->size = s;
}

void int_iRPC::setAllocation(iRPCAllocation::ptr a)
{
  cur_allocation = a;
  a->ref_count++;
}

void int_iRPC::setShouldSaveData(bool b)
{
  cur_allocation->needs_datasave = b;
}

void int_iRPC::setTargetAllocation(iRPCAllocation::ptr a)
{
  target_allocation = a;
}

static unsigned long roundUpPageSize(int_process *proc, unsigned long val)
{
  unsigned long pgsize = proc->getTargetPageSize();
  Dyninst::Address addr_mask = ~(pgsize-1);
  Dyninst::Address aligned_addr = val & addr_mask;
  if (aligned_addr < val)
    aligned_addr += pgsize;
  assert(aligned_addr >= val);
  return aligned_addr;
}

bool int_iRPC::fillInAllocation()
{
   bool result = false;
   assert(isMemManagementRPC());
   assert(target_allocation);
   assert(target_allocation->size);

   if (type == Allocation) {
      //We'll do all allocations in page size increments, round up the
      // size to the nearest page size.
      unsigned long aligned_size = roundUpPageSize(thread()->llproc(), target_allocation->size);
      pthrd_printf("Rounding target allocation size up from %lu to %lu\n",
                   target_allocation->size, aligned_size);
      target_allocation->size = aligned_size;

      //Fill in the snippet with a call to something like mmap for the
      // approiate amount of memory.
      pthrd_printf("Setting up allocation snippet %lu to allocate memory of "
                   "size %lu\n", id(), target_allocation->size);
      result = thrd->llproc()->plat_createAllocationSnippet((Dyninst::Address) 0,
                                                            false,
                                                            target_allocation->size,
                                                            binary_blob,
                                                            binary_size,
                                                            start_offset);
   }
   else if (type == Deallocation) {
      //Set up a snippet with a call to something like munmap for the
      // address and size.
      pthrd_printf("Setting up deallocation snippet %lu to clear memory %lx of "
                   "size %lu\n", id(), target_allocation->addr,
                   target_allocation->size);
      result = thrd->llproc()->plat_createDeallocationSnippet(target_allocation->addr,
                                                              target_allocation->size,
                                                              binary_blob,
                                                              binary_size,
                                                              start_offset);
   }
   assert(result);

   /**
    * We have a chicken and egg problem.  We'd like to run a snippet to allocate or
    * deallocate memory, but we don't have any memory to run it out of.
    *
    * We'll ask the platform specifics for a piece of executable memory to run the
    * initial allocation snippet out of.  As an example platform, on linux we'll
    * go searching through /proc/PID/maps for a big enough piece of executable memory
    * and temporarily convert that into a call to mmap, the results of which will
    * be used by the real RPCs.
    **/
   if (!cur_allocation) {
      setAllocation(iRPCAllocation::ptr(new iRPCAllocation()));
   }
   cur_allocation->size = binary_size;
   cur_allocation->addr = thrd->llproc()->mallocExecMemory(binary_size);
   setShouldSaveData(true);

   return true;
}

bool int_iRPC::countedSync()
{
   return counted_sync;
}

iRPCMgr *rpcMgr()
{
  static iRPCMgr rpcmgr;;
  return &rpcmgr;
}

iRPCMgr::iRPCMgr()
{
}

iRPCMgr::~iRPCMgr()
{
}

iRPCAllocation::ptr iRPCMgr::findAllocationForRPC(int_thread *thread, int_iRPC::ptr rpc)
{
   if (rpc->allocation())
      return rpc->allocation();

   rpc_list_t *posted = thread->getPostedRPCs();
   //See if an pending allocation in the queue can be used.
   for (rpc_list_t::iterator i = posted->begin(); i != posted->end(); i++) {
      int_iRPC::ptr cur = *i;
      if (cur->getType() == int_iRPC::Allocation) {
         iRPCAllocation::ptr allocation = cur->targetAllocation();
         assert(allocation);
         return allocation;
      }
   }

   //See if a finished running but uncleaned allocation can be used.
   // (We can search the list of posted iRPCs and the running iRPC
   // for references to an allocation that already ran.
   for (rpc_list_t::iterator i = posted->begin(); i != posted->end(); i++) {
      int_iRPC::ptr cur = *i;
      if (cur->getType() == int_iRPC::User && cur->allocSize() >= rpc->binarySize() &&
          !cur->userAllocated()) {
         return cur->allocation();
      }
   }
   int_iRPC::ptr running = thread->runningRPC();
   if (running &&
       running->getType() == int_iRPC::User &&
       running->allocSize() >= rpc->binarySize() &&
       !running->userAllocated())
   {
      return running->allocation();
   }
   return iRPCAllocation::ptr();
}

bool iRPCMgr::postRPCToProc(int_process *proc, int_iRPC::ptr rpc)
{
   pthrd_printf("Posting iRPC %lu to process %d, selecting a thread...\n",
                rpc->id(), proc->getPid());
   if (proc->getState() != int_process::running) {
      perr_printf("Attempt to post iRPC %lu to non-running process %d\n",
                  rpc->id(), proc->getPid());
      proc->setLastError(err_exited, "Attempt to post iRPC to exited process");
      return false;
   }
   //Find the thread with the fewest number of posted/running iRPCs
   int_threadPool *tp = proc->threadPool();
   int min_rpc_count = -1;
   int_thread *selected_thread = NULL;
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
      int_thread *thr = *i;
      assert(thr);
      if (thr->getGeneratorState().getState() != int_thread::running &&
          thr->getGeneratorState().getState() != int_thread::stopped)
      {
         continue;
      }
      if(thr->notAvailableForRPC()) {
         pthrd_printf("Skipping thread that is marked as system\n");
         continue;
      }

      // Don't post RPCs to threads that are in the middle of exiting
      if(thr->isExiting()) continue;


      int rpc_count = numActiveRPCs(thr);
      if (!proc->plat_supportDirectAllocation() && !findAllocationForRPC(thr, rpc)) {
         //We'll need to run an allocation and deallocation on this thread.
         // two more iRPCs.
         rpc_count += 2;
      }
      pthrd_printf("Thread %d has %d running/posted iRPCs\n", thr->getLWP(), rpc_count);
      if (rpc_count < min_rpc_count || min_rpc_count == -1) {
         selected_thread = thr;
         min_rpc_count = rpc_count;
      }
   }

   selected_thread = createThreadForRPC(proc, selected_thread);
   if(!selected_thread)
   {
     pthrd_printf("No thread available for iRPC %lu, aborting\n", rpc->id());
     return false;

   }
   pthrd_printf("Selected thread %d for iRPC %lu\n", selected_thread->getLWP(), rpc->id());


   assert(selected_thread);
   return postRPCToThread(selected_thread, rpc);
}


bool iRPCMgr::postRPCToThread(int_thread *thread, int_iRPC::ptr rpc)
{
   pthrd_printf("Posting iRPC %lu to thread %d\n", rpc->id(), thread->getLWP());

   if(thread->notAvailableForRPC()) {
      cerr << "Skipping thread that is marked as system - in thread-specific RPC" << endl;
      return false;
  }

   if (thread->getGeneratorState().getState() != int_thread::running &&
       thread->getGeneratorState().getState() != int_thread::stopped)
   {
		pthrd_printf("Internal state unhappy, ret false\n");
	   perr_printf("Attempt to post iRPC %lu to non-running thread %d\n",
                  rpc->id(), thread->getLWP());
      thread->setLastError(err_exited, "Attempt to post iRPC to exited thread");
      return false;
   }

   if( thread->isExiting() ) {
		pthrd_printf("Thread exiting, ret false\n");
       perr_printf("Attempt to post IRPC %lu to exiting thread %d\n",
               rpc->id(), thread->getLWP());
       thread->setLastError(err_exited, "Attempt to post iRPC to exiting thread");
       return false;
   }

   if (!rpc->isAsync()) {
     thread->incSyncRPCCount();
     rpc->counted_sync = true;
   }

   rpc_list_t *cur_list = thread->getPostedRPCs();
   rpc->setThread(thread);

   /**
    * There are three types of iRPCs.  User, allocation, and deallocation.
    * If an iRPC is supposed to allocate itself, then we'll post an extra
    * allocation (e.g, call mmap) and deallocation (e.g, call munmap) iRPC around it.
    * If there are several iRPCs and already some allocation/deallocation iRPCs
    * in the queue, then we'll hijack those allocations and possibly change their
    * memory size.
    *
    * If an allocation routine has already been run, but isn't big enough, we'll
    * have to put in new allocation iRPCs.
    *
    * So, if the queue looks like:
    *  Allocation(128) User1 User2 User3 Deallocation
    * and we want to add a User4 iRPC of size 256, we'd change the queue to:
    *  Allocation(256) User1 User2 User3 User4 Deallocation
    **/
   iRPCAllocation::ptr allocation;
   if (rpc->userAllocated()) {
      //The iRPC already has memory allocated, probably by the user,
      // no need for extra iRPCs.
      rpc->setShouldSaveData(true);
      cur_list->push_back(rpc);
      pthrd_printf("RPC %lu already has allocated memory, added to end\n", rpc->id());
      goto done;
   }
   if(thread->llproc()->plat_supportDirectAllocation())
   {
	   rpc->setDirectFree(true);
	   Address buffer = thread->llproc()->direct_infMalloc(rpc->binarySize());
		// FIXME: fail to post rather than asserting
	   assert(buffer);
	   rpc->setAllocation(iRPCAllocation::ptr(new iRPCAllocation()));
		rpc->allocation()->addr = buffer;
	   rpc->setAllocSize(rpc->binarySize());
	   cur_list->push_back(rpc);
		goto done;
   }

   if (rpc->isInternalRPC()) {
     cur_list->push_front(rpc);
     pthrd_printf("RPC %lu is internal and cuts in line\n", rpc->id());
     goto done;
   }
   allocation = findAllocationForRPC(thread, rpc);
   if (allocation) {
      rpc->setAllocation(allocation);
      //We have an allocation that works, add the this rpc to the end and move
      // the deallocation after it to the end of the list.
      bool found_dealloc = false;
      rpc_list_t::iterator i;
      int_iRPC::ptr deletion_rpc;
      for (i = cur_list->begin(); i != cur_list->end(); i++)
      {
         if (*i == rpc->deletionRPC()) {
            deletion_rpc = rpc->deletionRPC();
            cur_list->erase(i);
            found_dealloc = true;
            break;
         }
      }
      assert(found_dealloc);
      cur_list->push_back(rpc);
      cur_list->push_back(deletion_rpc);
      if (rpc->binarySize() > rpc->allocSize()) {
         pthrd_printf("Resized existing allocation from %lu to %lu to fit iRPC %lu\n",
                      rpc->allocSize(), rpc->binarySize(), rpc->id());
         rpc->setAllocSize(rpc->binarySize());
      }
      else {
         pthrd_printf("iRPC %lu fits in existing allocation\n", rpc->id());
      }
      goto done;
   }

   //We need to create new allocation and deallocation iRPCs around
   // this iRPC.
   rpc->setAllocation(iRPCAllocation::ptr(new iRPCAllocation()));
   rpc->setAllocSize(rpc->binarySize());
   cur_list->push_back(rpc->newAllocationRPC());
   cur_list->push_back(rpc);
   cur_list->push_back(rpc->newDeallocationRPC());
   pthrd_printf("Created new allocation and deallocation of size %lu to fit iRPC\n",
                rpc->binarySize());

 done:
   rpc->setState(int_iRPC::Posted);
   if (dyninst_debug_proccontrol) {
      pthrd_printf("Posted iRPC list for %d:\n", thread->getLWP());
      for (rpc_list_t::iterator i = cur_list->begin(); i != cur_list->end(); i++) {
         int_iRPC::ptr cur_rpc = *i;
         switch (cur_rpc->getType()) {
            case int_iRPC::NoType:
               assert(0);
               break;
            case int_iRPC::Allocation:
               pclean_printf("\tA-%lu(%lu) ", cur_rpc->id(),
                             cur_rpc->targetAllocation()->size);
               break;
            case int_iRPC::Deallocation:
               pclean_printf("\tD-%lu(%lu), ", cur_rpc->id(),
                             cur_rpc->targetAllocation()->size);
               break;
            case int_iRPC::User:
               pclean_printf("\tU-%lu(%lu), ", cur_rpc->id(), cur_rpc->binarySize());
               break;
            case int_iRPC::InfMalloc:
               pclean_printf("\tM-%lu", cur_rpc->id());
               break;
            case int_iRPC::InfFree:
               pclean_printf("\tF-%lu", cur_rpc->id());
               break;
         }
         pclean_printf("\n");
      }
   }

   return true;
}

Dyninst::Address int_iRPC::infMallocResult()
{
  return malloc_result;
}

void int_iRPC::setMallocResult(Dyninst::Address addr)
{
  malloc_result = addr;
}

Address int_iRPC::getInfFreeTarget()
{
   return inffree_target;
}

rpc_wrapper *int_iRPC::getWrapperForDecode()
{
   IRPC::ptr up = hl_irpc.lock();
   if (up == IRPC::ptr()) {
      return new rpc_wrapper(shared_from_this());
   }
   else {
      return up->llrpc();
   }
}

unsigned long int_iRPC::id() const {
   return my_id;
}

int_iRPC::State int_iRPC::getState() const {
   return state;
}

int_iRPC::Type int_iRPC::getType() const {
   return type;
}

void *int_iRPC::binaryBlob() const {
   return binary_blob;
}

unsigned long int_iRPC::binarySize() const {
   return binary_size;
}

unsigned long int_iRPC::startOffset() const {
   return start_offset;
}

bool int_iRPC::isAsync() const {
   return async;
}

int_thread *int_iRPC::thread() const {
   return thrd;
}

IRPC::weak_ptr int_iRPC::getIRPC() const {
   return hl_irpc;
}

iRPCAllocation::ptr int_iRPC::allocation() const {
   return cur_allocation;
}

iRPCAllocation::ptr int_iRPC::targetAllocation() const {
   return target_allocation;
}

bool int_iRPC::isProcStopRPC() const {
   return isInternalRPC() || isMemManagementRPC();
}

bool int_iRPC::isInternalRPC() const {
   return type == InfMalloc || type == InfFree;
}

unsigned long int_iRPC::allocSize() const {
   assert(cur_allocation);
   return cur_allocation->size;
}

Dyninst::Address int_iRPC::addr() const {
   if (!cur_allocation) return 0x0;
   return cur_allocation->addr;
}

bool int_iRPC::hasSavedRegs() const {
   assert(cur_allocation);
   return cur_allocation->have_saved_regs;
}

bool int_iRPC::userAllocated() const {
   if (!cur_allocation) return false;
   return cur_allocation->creation_irpc.expired() && cur_allocation->deletion_irpc.expired();
}

bool int_iRPC::shouldSaveData() const {
   assert(cur_allocation);
   return cur_allocation->needs_datasave;
}

bool int_iRPC::isMemManagementRPC() const {
   return (type == Allocation || type == Deallocation);
}

int_iRPC::ptr int_iRPC::allocationRPC() const {
   assert(cur_allocation);
   return cur_allocation->creation_irpc.lock();
}

int_iRPC::ptr int_iRPC::deletionRPC() const {
   assert(cur_allocation);
   return cur_allocation->deletion_irpc.lock();
}

int_iRPC::ptr int_iRPC::newAllocationRPC()
{
   int_iRPC::ptr newrpc = int_iRPC::ptr(new int_iRPC(NULL, 0, false));
   newrpc->setState(int_iRPC::Posted);
   newrpc->setType(int_iRPC::Allocation);
   newrpc->setTargetAllocation(cur_allocation);
   newrpc->thrd = thrd;
   cur_allocation->creation_irpc = newrpc;
   return newrpc;
}

int_iRPC::ptr int_iRPC::newDeallocationRPC()
{
   int_iRPC::ptr newrpc = int_iRPC::ptr(new int_iRPC(NULL, 0, async));
   newrpc->setState(int_iRPC::Posted);
   newrpc->setType(int_iRPC::Deallocation);
   newrpc->setTargetAllocation(cur_allocation);
   newrpc->thrd = thrd;
   cur_allocation->deletion_irpc = newrpc;
   return newrpc;
}

#define STR_CASE(s) case s: return #s
const char *int_iRPC::getStrType() const
{
   switch (type) {
      STR_CASE(NoType);
      STR_CASE(Allocation);
      STR_CASE(Deallocation);
      STR_CASE(User);
      STR_CASE(InfMalloc);
      STR_CASE(InfFree);
   }
   return NULL;
}

const char *int_iRPC::getStrState() const
{
   switch (state) {
      STR_CASE(Unassigned);
      STR_CASE(Posted);
      STR_CASE(Prepping);
      STR_CASE(Prepped);
      STR_CASE(Saving);
      STR_CASE(Saved);
      STR_CASE(Writing);
      STR_CASE(Ready);
      STR_CASE(Running);
      STR_CASE(Cleaning);
      STR_CASE(Finished);
   }
   return NULL;
}

unsigned iRPCMgr::numActiveRPCs(int_thread *thr)
{
   int rpc_count = 0;
   rpc_count += thr->getPostedRPCs()->size();
   if (thr->runningRPC())
      rpc_count++;
   return rpc_count;
}

bool int_iRPC::saveRPCState()
{
   pthrd_printf("Saving state for iRPC %lu\n", id());
   assert(getState() == int_iRPC::Prepped);
   setState(Saving);

   if (isMemManagementRPC() && !allocation()) {
      //Allocation and deallocation RPCs have their data and address
      // lazily filled in, rather than at post time.  This allows us
      // to delay the creation until we know how big they will be.
      pthrd_printf("Creating allocation for Alloc or Dealloc RPC %lu on %d/%d\n",
                   id(), thrd->llproc()->getPid(), thrd->getLWP());
      bool result = fillInAllocation();
      assert(result);
      if(!result) return false;

   }
   assert(allocation());

   if (!thread()->hasSavedRPCRegs() && !regsave_result) {
      regsave_result = allreg_response::createAllRegResponse();
      pthrd_printf("Saving original application registers for %d/%d\n",
                   thrd->llproc()->getPid(), thrd->getLWP());
      bool result = thread()->saveRegsForRPC(regsave_result);
      assert(result);
      if(!result) return false;

   }

   if (shouldSaveData() && !allocation()->orig_data)
   {
      //Need to backup the original memory that was at this RPC
      assert(!memsave_result);
      pthrd_printf("Saving original %lu bytes of memory at %lx from application "
                   "for %d/%d\n", (unsigned long) allocSize(), addr(),
                   thrd->llproc()->getPid(), thrd->getLWP());
      allocation()->orig_data = (char *) malloc(allocSize());
      memsave_result = mem_response::createMemResponse((char *) allocation()->orig_data, allocSize());
      bool result = thrd->llproc()->readMem(addr(), memsave_result, (thrd->isRPCEphemeral() ? thrd : NULL));
      assert(result);
      if(!result) return false;

   }

   return true;
}

bool int_iRPC::checkRPCFinishedSave()
{
   assert(getState() == Saving);
   if (memsave_result && !memsave_result->isReady())
      return false;
   if (regsave_result && !regsave_result->isReady())
      return false;

   memsave_result = mem_response::ptr();
   regsave_result = allreg_response::ptr();

   return true;
}

bool int_iRPC::writeToProc()
{
   assert(getState() == Saved);
   setState(Writing);

   int_thread *thr = thread();
   pthrd_printf("Writing rpc %lu memory to %lx->%lx\n", id(), addr(),
                addr()+binarySize());


   if (!rpcwrite_result) {
      rpcwrite_result = result_response::createResultResponse();
	  bool result = thr->llproc()->writeMem(binaryBlob(), addr(), binarySize(), rpcwrite_result, (thr->isRPCEphemeral() ? thr : NULL));
      if (!result) {
         pthrd_printf("Failed to write IRPC\n");
         return false;
      }
   }

   if (!pcset_result) {
      pcset_result = result_response::createResultResponse();

      Dyninst::Address newpc_addr = addr() + startOffset();
      Dyninst::MachRegister pc = Dyninst::MachRegister::getPC(thr->llproc()->getTargetArch());
      pthrd_printf("IRPC: Setting %d/%d PC to %lx\n", thr->llproc()->getPid(),
                   thr->getLWP(), newpc_addr);
      bool result = thr->setRegister(pc, newpc_addr, pcset_result);
      if (!result) {
         pthrd_printf("Failed to set PC register for IRPC\n");
         return false;
      }
   }
   return true;
}

bool int_iRPC::checkRPCFinishedWrite()
{
   assert(rpcwrite_result);
   assert(pcset_result);

   if (!rpcwrite_result->isReady() || rpcwrite_result->hasError())
      return false;
   if (!pcset_result->isReady() || pcset_result->hasError())
      return false;

   rpcwrite_result = result_response::ptr();
   pcset_result = result_response::ptr();

   return true;
}

bool int_iRPC::runIRPC()
{
   pthrd_printf("Moving next iRPC for %d/%d to running\n",
                thrd->llproc()->getPid(), thrd->getLWP());

   //Remove rpc from thread list
   rpc_list_t *posted = thrd->getPostedRPCs();
   assert(shared_from_this() == posted->front());
   posted->pop_front();

   //Set in running state
   thrd->setRunningRPC(shared_from_this());
   setState(Running);

   assert(allocation());
   assert(!allocSize() || (binarySize() <= allocSize()));
   assert(binaryBlob());

   if (thrd->isRPCEphemeral()) {
      thrd->getUserRPCState().desyncState(int_thread::running);
	   // Create it if necessary
	   thrd->llproc()->instantiateRPCThread();
	   pthrd_printf("\tInstantiated iRPC thread\n");
	   // And the threadpool needs to know about us so that we will ContinueDebugEvent. Ugh.
	   if(thrd->llproc()->threadPool()->findThreadByLWP(thrd->getLWP()) == NULL)
	   {
		   thrd->llproc()->threadPool()->addThread(thrd);
		   ProcPool()->addThread(thrd->llproc(), thrd);
	   }
   }


   if (isProcStopRPC()) {
      thrd->getIRPCWaitState().desyncStateProc(int_thread::stopped);
      thrd->getIRPCState().desyncState(int_thread::running);
      thrd->getIRPCSetupState().restoreStateProc();
   }
   else if (thrd->llproc()->plat_threadOpsNeedProcStop()) {
      thrd->getIRPCSetupState().restoreStateProc();
   }
   else {
      thrd->getIRPCSetupState().restoreState();
   }

   return true;
}

void int_iRPC::getPendingResponses(std::set<response::ptr> &resps)
{
   if (memsave_result) {
      resps.insert(memsave_result);
   }
   if (rpcwrite_result) {
      resps.insert(rpcwrite_result);
   }
   if (regsave_result) {
      resps.insert(regsave_result);
   }
   if (pcset_result) {
      resps.insert(pcset_result);
   }
}

void int_iRPC::syncAsyncResponses(bool is_sync)
{
   set<response::ptr> resps;
   getPendingResponses(resps);
   if (resps.empty())
      return;

   if (is_sync) {
      int_process::waitForAsyncEvent(resps);
      return;
   }
}

bool int_iRPC::needsToRestoreInternal() const
{
   return restore_internal;
}

void int_iRPC::setRestoreInternal(bool b)
{
   restore_internal = b;
}

int_thread::State int_iRPC::getRestoreToState() const
{
   return restore_at_end;
}

void int_iRPC::setRestoreToState(int_thread::State s)
{
   restore_at_end = s;
}

bool iRPCMgr::isRPCTrap(int_thread *thr, Dyninst::Address addr)
{
   bool result;
   int_iRPC::ptr rpc;
   Dyninst::Address start, end;
   unsigned long size;
   rpc = thr->runningRPC();
   if (!rpc) {
      pthrd_printf("%d/%d is not running any iRPCs, trap is not RPC completion\n",
                   thr->llproc()->getPid(), thr->getLWP());
      result = false;
      goto done;

   }

   start = rpc->addr();
   size = rpc->allocSize();
   end = start + size;
   if (addr >= start && addr < start+size) {
      pthrd_printf("%d/%d trap at %lx lies between %lx and %lx, is iRPC %lu trap\n",
                   thr->llproc()->getPid(), thr->getLWP(), addr, start, end, rpc->id());
      rpc->setState(int_iRPC::Cleaning);
      result = true;
      goto done;
   }

   pthrd_printf("%d/%d trap at %lx outside %lx and %lx, not iRPC %lu trap\n",
                thr->llproc()->getPid(), thr->getLWP(), addr, start, end, rpc->id());
   result = false;

 done:
   return result;
}

int_iRPC::ptr iRPCMgr::createInfMallocRPC(int_process *proc, unsigned long size,
                                      bool use_addr, Dyninst::Address addr)
{
   int_iRPC::ptr irpc = int_iRPC::ptr(new int_iRPC(NULL, 0, true, true, 0));
   irpc->setType(int_iRPC::InfMalloc);
   size = roundUpPageSize(proc, size);

   void *binary_blob;
   unsigned long binary_size, start_offset;
   bool result = proc->plat_createAllocationSnippet(addr, use_addr, size, binary_blob,
                                               binary_size, start_offset);
   irpc->setBinaryBlob(binary_blob);
   irpc->setBinarySize(binary_size);
   irpc->setStartOffset(start_offset);
   irpc->setAllocSize(binary_size);


   if (!result)
      return int_iRPC::ptr();
   irpc->allocation()->addr = proc->mallocExecMemory(binary_size);
   irpc->setShouldSaveData(true);
   return irpc;
}

int_iRPC::ptr iRPCMgr::createInfFreeRPC(int_process *proc, unsigned long size,
                                    Dyninst::Address addr)
{
   int_iRPC::ptr irpc = int_iRPC::ptr(new int_iRPC(NULL, 0, true, true, 0));
   irpc->setType(int_iRPC::InfFree);
   size = roundUpPageSize(proc, size);

   void *binary_blob;
   unsigned long binary_size, start_offset;
   bool result = proc->plat_createDeallocationSnippet(addr, size, binary_blob, binary_size, start_offset);
   irpc->setBinaryBlob(binary_blob);
   irpc->setBinarySize(binary_size);
   irpc->setStartOffset(start_offset);
   irpc->setAllocSize(binary_size);
   irpc->inffree_target = addr;

   if (!result)
      return int_iRPC::ptr();
   irpc->allocation()->addr = proc->mallocExecMemory(binary_size);
   irpc->setShouldSaveData(true);
   return irpc;
}

iRPCHandler::iRPCHandler() :
   Handler(std::string("RPC Handler"))
{
}

iRPCHandler::~iRPCHandler()
{
}

void iRPCHandler::getEventTypesHandled(std::vector<EventType> &etypes)
{
  etypes.push_back(EventType(EventType::None, EventType::RPC));
}

int iRPCHandler::getPriority() const
{
	// This *must* be after callbacks, so that the user can read any return result they need...
   return PostCallbackPriority;
}

Handler::handler_ret_t iRPCHandler::handleEvent(Event::ptr ev)
{
   //An RPC has completed, clean-up
   int_thread *thr = ev->getThread()->llthrd();
   int_process *proc = ev->getProcess()->llproc();
   EventRPC *event = static_cast<EventRPC *>(ev.get());
   int_eventRPC *ievent = event->getInternal();
   int_iRPC::ptr rpc = event->getllRPC()->rpc;
   iRPCMgr *mgr = rpcMgr();
   bool isLastRPC = !thr->hasPostedRPCs();
   assert(rpc);
   assert(mgr);
   assert(rpc->getState() == int_iRPC::Cleaning);
   // Is this a temporary thread created just for this RPC?
   bool ephemeral = thr->isRPCEphemeral();

   pthrd_printf("Handling RPC %lu completion on %d/%d\n", rpc->id(),
                proc->getPid(), thr->getLWP());

   if ((rpc->getType() == int_iRPC::InfMalloc || rpc->getType() == int_iRPC::Allocation) &&
       !ievent->alloc_regresult)
   {
      //Post a request for the return value of the allocation
      pthrd_printf("Cleaning up allocation RPC\n");
      ievent->alloc_regresult = reg_response::createRegResponse();
      bool result = proc->plat_collectAllocationResult(thr, ievent->alloc_regresult);
      if( (long)ievent->alloc_regresult->getResult() == 0){
            perr_printf("InfMalloc failed\n");
            return ret_error;
      }
      if(!result) return ret_error;

      proc->handlerPool()->notifyOfPendingAsyncs(ievent->alloc_regresult, ev);
   }

   if (rpc->shouldSaveData() && !ievent->memrestore_response) {
      //Post a request to restore the memory used by the RPC
      pthrd_printf("Restoring memory to %lx from %p of size %lu\n",
                   rpc->addr(), rpc->allocation()->orig_data, rpc->allocSize());
      assert(rpc->allocation()->orig_data);
      ievent->memrestore_response = result_response::createResultResponse();
      bool result = proc->writeMem(rpc->allocation()->orig_data,
                                            rpc->addr(),
                                            rpc->allocSize(),
                                            ievent->memrestore_response);
      assert(result);
      if(!result) return ret_error;
   }
   if (rpc->directFree()) {
	   assert(rpc->addr());
	   thr->llproc()->direct_infFree(rpc->addr());
   }
   if (ephemeral) {
      // Don't restore registers; instead, kill the thread if there
      // aren't any other pending iRPCs for it.
      // We count as an active iRPC...
      assert(mgr->numActiveRPCs(thr) > 0);
      if (mgr->numActiveRPCs(thr) == 1) {
         pthrd_printf("Terminating RPC thread %d\n",
                      thr->getLWP());
         thr->terminate();
         // CLEANUP on aisle 1
#if defined(os_windows)
         windows_process *winproc = dynamic_cast<windows_process *>(thr->llproc());
         if (winproc) {
            pthrd_printf("Destroying RPC thread %d\n",
                         thr->getLWP());
            winproc->destroyRPCThread();
         }
#endif
      } else {
         pthrd_printf("RPC thread %d has %u active RPCs, parking thread\n",
                      thr->getLWP(), mgr->numActiveRPCs(thr));
         // don't do an extra desync here, it's handled by throwEventsBeforeContinue()
      }
   }
   else if (!ievent->regrestore_response &&
            (!ievent->alloc_regresult || ievent->alloc_regresult->isReady()))
   {
      //Restore the registers--need to wait for above getreg first
      ievent->regrestore_response = result_response::createResultResponse();
      pthrd_printf("Restoring all registers\n");
      bool result = thr->restoreRegsForRPC(isLastRPC, ievent->regrestore_response);
      assert(result);
      if(!result) return ret_error;
   }

   set<response::ptr> async_pending;
   ievent->getPendingAsyncs(async_pending);
   if (!async_pending.empty()) {
      proc->handlerPool()->notifyOfPendingAsyncs(async_pending, ev);
      return ret_async;
   }

   if (ievent->alloc_regresult) {
      //Collect and use the return value of the allocation
      assert(ievent->alloc_regresult->isReady());

      Address addr = ievent->alloc_regresult->getResult();
      pthrd_printf("Allocation RPC %lu returned memory at %lx\n", rpc->id(), addr);
      if (rpc->getType() == int_iRPC::Allocation) {
         rpc->targetAllocation()->addr = addr;
      }
      else if (rpc->getType() == int_iRPC::InfMalloc) {
         rpc->setMallocResult(addr);
      }
   }

   if (rpc->isProcStopRPC()) {
      pthrd_printf("Restoring RPC state after procstop completion\n");
      thr->getIRPCWaitState().restoreStateProc();
      thr->getIRPCState().restoreState();
   }

   if (rpc->isMemManagementRPC() || rpc->isInternalRPC())
   {
      //Free memory associated with RPC for future use
      pthrd_printf("Freeing exec memory at %lx\n", rpc->addr());
      proc->freeExecMemory(rpc->addr());
   }


   pthrd_printf("RPC %lu is moving to state finished\n", rpc->id());
   thr->clearRunningRPC();
   rpc->setState(int_iRPC::Finished);

   if (rpc->countedSync()) {
     thr->decSyncRPCCount();
   }

   if (rpc->needsToRestoreInternal()) {
      rpc->thread()->getInternalState().restoreState();
   }
   if(mgr->numActiveRPCs(thr) == 0) {
      if (rpc->thread()->getUserRPCState().isDesynced())
         rpc->thread()->getUserRPCState().restoreState();
   } else {
	   thr->throwEventsBeforeContinue();
   }

   return ret_success;
}

iRPCPreCallbackHandler::iRPCPreCallbackHandler() :
   Handler("iRPC PreCallback Handler")
{
}

iRPCPreCallbackHandler::~iRPCPreCallbackHandler()
{
}

Handler::handler_ret_t iRPCPreCallbackHandler::handleEvent(Event::ptr ev)
{
   EventRPC *event = static_cast<EventRPC *>(ev.get());
   int_iRPC::ptr rpc = event->getllRPC()->rpc;

   int_thread::State newstate = rpc->getRestoreToState();
   if (newstate == int_thread::none)
      return ret_success;

   int_thread *thr = ev->getThread()->llthrd();
   thr->getUserState().setState(newstate);
   return ret_success;
}

void iRPCPreCallbackHandler::getEventTypesHandled(std::vector<EventType> &etypes)
{
  etypes.push_back(EventType(EventType::None, EventType::RPC));
}

iRPCLaunchHandler::iRPCLaunchHandler() :
   Handler("iRPC Launch Handler")
{
}

iRPCLaunchHandler::~iRPCLaunchHandler()
{
}

void iRPCLaunchHandler::getEventTypesHandled(std::vector<EventType> &etypes)
{
  etypes.push_back(EventType(EventType::None, EventType::RPCLaunch));
}

Handler::handler_ret_t iRPCLaunchHandler::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thr = ev->getThread()->llthrd();

   int_iRPC::ptr posted_rpc = thr->nextPostedIRPC();
   if (!posted_rpc || thr->runningRPC())
      return Handler::ret_success;

   assert(posted_rpc->getState() != int_iRPC::Posted);

   pthrd_printf("Handling next posted irpc %lu on %d/%d of type %s in state %s\n",
                posted_rpc->id(), proc->getPid(), thr->getLWP(),
                posted_rpc->getStrType(), posted_rpc->getStrState());

   /**
    * Check if we've successfully made it to the stopped state.
    *  (There's no longer any work to be done to move from prepping to prepped--
    *  it all happens in the procstop logic.)
    **/
   if (posted_rpc->getState() == int_iRPC::Prepping) {
      pthrd_printf("Marking RPC %lu on %d/%d as prepped\n", posted_rpc->id(), proc->getPid(), thr->getLWP());
      assert(posted_rpc->isRPCPrepped());
      posted_rpc->setState(int_iRPC::Prepped);
   }

   std::set<response::ptr> async_resps;
   /**
    * Save registers and memory
    **/
   if (posted_rpc->getState() == int_iRPC::Prepped) {
      pthrd_printf("Saving RPC state on %d/%d\n", proc->getPid(), thr->getLWP());
      bool result = posted_rpc->saveRPCState();
      if (!result) {
         pthrd_printf("Failed to save RPC state on %d/%d\n", proc->getPid(), thr->getLWP());
         return Handler::ret_error;
      }
   }

   //Wait for async
   if (posted_rpc->getState() == int_iRPC::Saving) {
      if (!posted_rpc->checkRPCFinishedSave()) {
         pthrd_printf("Async, RPC has not finished save\n");
         posted_rpc->getPendingResponses(async_resps);
         proc->handlerPool()->notifyOfPendingAsyncs(async_resps, ev);
         return Handler::ret_async;
      }
      else {
         posted_rpc->setState(int_iRPC::Saved);
      }
   }

   /**
    * Write PC register and memory
    **/
   if (posted_rpc->getState() == int_iRPC::Saved) {
      pthrd_printf("Writing RPC on %d\n", proc->getPid());
      bool result = posted_rpc->writeToProc();
      if (!result) {
         pthrd_printf("Failed to write RPC on %d\n", proc->getPid());
         return Handler::ret_error;
      }
   }
   //Wait for async
   if (posted_rpc->getState() == int_iRPC::Writing) {
      if (!posted_rpc->checkRPCFinishedWrite()) {
         pthrd_printf("Async, RPC has not finished memory write\n");
         posted_rpc->getPendingResponses(async_resps);
         proc->handlerPool()->notifyOfPendingAsyncs(async_resps, ev);
         return Handler::ret_async;
      }
      else {
         posted_rpc->setState(int_iRPC::Ready);
      }
   }

   if (posted_rpc->getState() == int_iRPC::Ready) {
      posted_rpc->runIRPC();
   }

   return Handler::ret_success;
}

IRPC::ptr IRPC::createIRPC(void *binary_blob, unsigned size,
                           bool non_blocking)
{
   int_iRPC::ptr irpc = int_iRPC::ptr(new int_iRPC(binary_blob, size, non_blocking));
   rpc_wrapper *wrapper = new rpc_wrapper(irpc);
   IRPC::ptr rpc = IRPC::ptr(new IRPC(wrapper));
   irpc->setIRPC(rpc);
   irpc->copyBinaryBlob(binary_blob, size);
   irpc->setAsync(non_blocking);

   return rpc;
}

IRPC::ptr IRPC::createIRPC(void *binary_blob, unsigned size,
                           Dyninst::Address addr, bool non_blocking)
{
   int_iRPC::ptr irpc = int_iRPC::ptr(new int_iRPC(binary_blob, size, non_blocking, true, addr));
   rpc_wrapper *wrapper = new rpc_wrapper(irpc);
   IRPC::ptr rpc = IRPC::ptr(new IRPC(wrapper));
   irpc->setIRPC(rpc);
   irpc->copyBinaryBlob(binary_blob, size);
   irpc->setAsync(non_blocking);
   return rpc;
}

IRPC::ptr IRPC::createIRPC(IRPC::ptr o)
{
   int_iRPC::ptr orig = o->llrpc()->rpc;
   int_iRPC::ptr irpc;
   if (orig->cur_allocation)
      irpc = int_iRPC::ptr(new int_iRPC(orig->binary_blob, orig->binary_size, orig->async,
                                        true, orig->addr()));
   else
      irpc = int_iRPC::ptr(new int_iRPC(orig->binary_blob, orig->binary_size, orig->async,
                                        false, 0));

   rpc_wrapper *wrapper = new rpc_wrapper(irpc);
   IRPC::ptr rpc = IRPC::ptr(new IRPC(wrapper));
   irpc->setIRPC(rpc);
   irpc->copyBinaryBlob(orig->binary_blob, orig->binary_size);
   return rpc;
}

IRPC::ptr IRPC::createIRPC(IRPC::ptr o, Address addr)
{
   int_iRPC::ptr orig = o->llrpc()->rpc;
   int_iRPC::ptr irpc;
   irpc = int_iRPC::ptr(new int_iRPC(orig->binary_blob, orig->binary_size, orig->async,
                                     true, addr));
   rpc_wrapper *wrapper = new rpc_wrapper(irpc);
   IRPC::ptr rpc = IRPC::ptr(new IRPC(wrapper));
   irpc->setIRPC(rpc);
   irpc->copyBinaryBlob(orig->binary_blob, orig->binary_size);
   return rpc;
}

IRPC::IRPC(rpc_wrapper *wrapper_) :
   wrapper(wrapper_)
{
}

IRPC::~IRPC()
{
  delete wrapper;
  wrapper = NULL;
}

rpc_wrapper *IRPC::llrpc() const
{
  return wrapper;
}

void *IRPC::getData() const
{
   return wrapper->rpc->user_data;
}

void IRPC::setData(void *p) const
{
   wrapper->rpc->user_data = p;
}

Dyninst::Address IRPC::getAddress() const
{
  return wrapper->rpc->addr();
}

void *IRPC::getBinaryCode() const
{
  return wrapper->rpc->binaryBlob();
}

unsigned IRPC::getBinaryCodeSize() const
{
  return wrapper->rpc->binarySize();
}

unsigned long IRPC::getID() const
{
   return wrapper->rpc->id();
}

void IRPC::setStartOffset(unsigned long o)
{
   wrapper->rpc->setStartOffset(o);
}

unsigned long IRPC::getStartOffset() const
{
   return wrapper->rpc->startOffset();
}

bool IRPC::isBlocking() const
{
   return !wrapper->rpc->isAsync();
}

#if !defined(os_windows)
int_thread* iRPCMgr::createThreadForRPC(int_process*, int_thread *candidate)
{
   return candidate;
}
#endif

IRPC::State IRPC::state() const
{
	// Up-map from the underlying state
	switch (wrapper->rpc->getState()) {
		case int_iRPC::Unassigned:
			return Created;
		case int_iRPC::Posted:
			return Posted;
		case int_iRPC::Prepping:
		case int_iRPC::Prepped:
		case int_iRPC::Saving:
		case int_iRPC::Saved:
		case int_iRPC::Writing:
		case int_iRPC::Ready:
		case int_iRPC::Running:
			return Running;
		case int_iRPC::Cleaning:
		case int_iRPC::Finished:
			return Done;
		default:
			return Error;
	}
}

bool IRPC::continueStoppedIRPC()
{
   MTLock lock_this_func;

   int_iRPC::ptr rpc = wrapper->rpc;
   int_thread *thrd = rpc->thread();
   IRPC::State cur_state = state();
   if (cur_state != Running && cur_state != Posted) {
      perr_printf("Tried to continueStoppedIRPC on RPC %lu with invalid state %d\n", rpc->id(), cur_state);
      if (thrd)
         thrd->setLastError(err_nothrd, "RPC is not assigned to a thread\n");
      else
         globalSetLastError(err_nothrd, "RPC is not assigned to a thread\n");
      return false;
   }

   assert(thrd);
   int_thread::StateTracker &userstate = thrd->getUserState();
   if (RUNNING_STATE(userstate.getState())) {
      perr_printf("Tried to continue already running thread %d/%d\n", thrd->llproc()->getPid(), thrd->getLWP());
      thrd->setLastError(err_notstopped, "Thread is not stopped\n");
      return false;
   }

   bool result = userstate.setState(int_thread::running);
   if (!result) {
      pthrd_printf("Failed to continue thread %d/%d\n", thrd->llproc()->getPid(), thrd->getLWP());
      thrd->setLastError(err_internal, "Could not continue thread associated with iRPC\n");
      return false;
   }
   thrd->llproc()->throwNopEvent();
   return true;
}
