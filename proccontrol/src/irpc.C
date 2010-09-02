#include "proccontrol/h/PCErrors.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/src/irpc.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/h/Mailbox.h"

#include <cstring>
#include <cassert>

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
   async(async_),
   thrd(NULL),
   freeBinaryBlob(false),
   needsDesync(false),
   lock_live(0),
   needs_clean(false)
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
   bool needsProcStop = isProcStopRPC();
   bool isStopped;
   if (state == Prepped)
      return true;
   assert(state == Prepping);
   if (needsProcStop || useHybridLWPControl(thrd) ) {
      isStopped = thrd->llproc()->threadPool()->allStopped();
   }else{
      isStopped = thrd->getInternalState() == int_thread::stopped;
   }
   if (isStopped) {
      pthrd_printf("Marking RPC %lu on %d/%d as prepped\n", id(), thrd->llproc()->getPid(), thrd->getLWP());
      setState(Prepped);
   }
   return isStopped;
}

void int_iRPC::setNeedsDesync(bool b)
{
   needsDesync = b;
}

void int_iRPC::setState(int_iRPC::State s) 
{
   int old_state = (int) state;
   int new_state = (int) s;
   assert(new_state >= old_state);
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

void int_iRPC::markReady()
{
   assert(state == Prepped);
   setState(Ready);
   thrd->setRunningRPC(shared_from_this());
}

void int_iRPC::markRunning()
{
   if (state != Ready) return;
   setState(Running);
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
      result = rpcMgr()->createAllocationSnippet(thrd->llproc(), 
                                                 (Dyninst::Address) 0, 
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
      result = rpcMgr()->createDeallocationSnippet(thrd->llproc(),
                                                   target_allocation->addr,
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

   return true;
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
      setLastError(err_exited, "Attempt to post iRPC to exited process");
      return false;
   }
   //Find the thread with the fewest number of posted/running iRPCs
   int_threadPool *tp = proc->threadPool();
   int min_rpc_count = -1;
   int_thread *selected_thread = NULL;
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
      int_thread *thr = *i;
      assert(thr);
      if (thr->getInternalState() != int_thread::running && thr->getInternalState() != int_thread::stopped) {
         continue;
      }
      int rpc_count = numActiveRPCs(thr);
      if (!findAllocationForRPC(thr, rpc)) {
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
   assert(selected_thread);
   return postRPCToThread(selected_thread, rpc);
}

bool iRPCMgr::postRPCToThread(int_thread *thread, int_iRPC::ptr rpc)
{
   pthrd_printf("Posting iRPC %lu to thread %d\n", rpc->id(), thread->getLWP());
   if (thread->getInternalState() != int_thread::running && 
       thread->getInternalState() != int_thread::stopped) 
   {
      perr_printf("Attempt to post iRPC %lu to non-running thread %d\n", 
                  rpc->id(), thread->getLWP());
      setLastError(err_exited, "Attempt to post iRPC to exited thread");
      return false;
   }

   if (!rpc->isAsync())
     thread->incSyncRPCCount();

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
 
bool int_iRPC::needsToDesync() const {
   return needsDesync;
}

int_iRPC::ptr int_iRPC::newAllocationRPC()
{
   int_iRPC::ptr newrpc = int_iRPC::ptr(new int_iRPC(NULL, 0, false));
   newrpc->setState(int_iRPC::Posted);
   newrpc->setType(int_iRPC::Allocation);
   newrpc->setTargetAllocation(cur_allocation);
   newrpc->thrd = thrd;
   thrd->incSyncRPCCount();
   cur_allocation->creation_irpc = newrpc;
   setShouldSaveData(false); //The memory will just be deallocated
   return newrpc;
}

int_iRPC::ptr int_iRPC::newDeallocationRPC()
{
   int_iRPC::ptr newrpc = int_iRPC::ptr(new int_iRPC(NULL, 0, async));
   newrpc->setState(int_iRPC::Posted);
   newrpc->setType(int_iRPC::Deallocation);
   newrpc->setTargetAllocation(cur_allocation);
   newrpc->thrd = thrd;
   if (!async) {
      thrd->incSyncRPCCount();
   }
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

bool iRPCMgr::prepNextRPC(int_thread *thr, bool sync_prep, bool &user_error)
{
   user_error = false;
   pthrd_printf("Prepping next iRPC for %d/%d, sync_prep = %s\n", thr->llproc()->getPid(), thr->getLWP(),
                sync_prep ? "true" : "false");
   if (thr->runningRPC()) {
      pthrd_printf("Thread is already running iRPC %lu\n", thr->runningRPC()->id());
      return false;
   }
   rpc_list_t *posted = thr->getPostedRPCs();
   if (!posted->size()) {
      pthrd_printf("No posted iRPCs for this thread\n");
      return false;
   }
   int_iRPC::ptr rpc = posted->front();
   if (rpc->getState() == int_iRPC::Prepping) {
      pthrd_printf("Thread is already prepping iRPC");
      return false;
   }

   assert(rpc->getState() == int_iRPC::Posted);   
   rpc->setState(int_iRPC::Prepping);

   pthrd_printf("Prepping iRPC %lu on %d/%d\n", rpc->id(), thr->llproc()->getPid(), 
                  thr->getLWP());
   bool isStopped;
   bool needsProcStop = rpc->isProcStopRPC();

   if (needsProcStop && !useHybridLWPControl(thr) ) {
      //Need to make sure entire process is stopped.
      pthrd_printf("iRPC %lu needs a process stop on %d\n", rpc->id(), 
                   thr->llproc()->getPid());
      isStopped = !checkIfNeedsProcStop(thr->llproc());
      rpc->setNeedsDesync(true);
      thr->llproc()->threadPool()->desyncInternalState();
   }
   else {
      if( useHybridLWPControl(thr) ) {
        pthrd_printf("iRPC %lu needs a process stop on %d\n", rpc->id(),
                thr->llproc()->getPid());
        isStopped = thr->llproc()->threadPool()->allStopped();

        if( needsProcStop && isStopped ) {
            rpc->setNeedsDesync(true);
            thr->llproc()->threadPool()->desyncInternalState();
        }
      }else{
        //Need to stop a single thread.
        pthrd_printf("iRPC %lu needs a thread stop on %d/%d\n", rpc->id(),
                       thr->llproc()->getPid(), thr->getLWP());
        isStopped = (thr->getInternalState() == int_thread::stopped);
      }
   }
      
   if (isStopped) {
      pthrd_printf("Already stopped, marked rpc prepped\n");
      rpc->setState(int_iRPC::Prepped);
      return true;
   }

   bool result;
   if (needsProcStop && !useHybridLWPControl(thr) ) {
      pthrd_printf("Stopping process %d for iRPC setup\n", thr->llproc()->getPid());
      result = stopNeededThreads(thr->llproc(), sync_prep);
   }
   else {
      if( useHybridLWPControl(thr) ) {
          pthrd_printf("Stopping process %d for iRPC setup on %d/%d\n",
                  thr->llproc()->getPid(), thr->llproc()->getPid(),
                  thr->getLWP());

          if (sync_prep) {
              thr->llproc()->setPendingProcStop(true);
          }
          rpc->setNeedsDesync(true);
          thr->llproc()->threadPool()->desyncInternalState();
          result = thr->llproc()->threadPool()->intStop(sync_prep);
          if(sync_prep) {
              thr->llproc()->setPendingProcStop(false);
          }
      }else{
          pthrd_printf("Stopping thread %d/%d for iRPC setup\n",
                       thr->llproc()->getPid(), thr->getLWP());
          rpc->setNeedsDesync(true);
          thr->desyncInternalState();
          result = thr->intStop(sync_prep);
      }
   }
   if (!result) {
      pthrd_printf("Failed to stop process/thread for iRPC\n");
      user_error = true;
      return false;
   }

   if (sync_prep && rpc->getState() == int_iRPC::Prepping) {
      pthrd_printf("Setting iRPC to prepped--stopped process for iRPC\n");
      rpc->setState(int_iRPC::Prepped);
      return true;
   }
   return true;
}
   
bool iRPCMgr::runNextRPC(int_thread *thr, bool block)
{
   pthrd_printf("Running next iRPC for %d/%d\n", thr->llproc()->getPid(), thr->getLWP());
   assert(!thr->runningRPC());
   if (thr->runningRPC()) {
      pthrd_printf("Thread is already running iRPC %lu\n", thr->runningRPC()->id());
      return false;
   }
   
   rpc_list_t *posted = thr->getPostedRPCs();
   if (!posted->size()) {
      pthrd_printf("No posted iRPCs for this thread\n");
      return false;
   }
   int_iRPC::ptr rpc = posted->front();
   posted->pop_front();
   pthrd_printf("Removed iRPC %lu from posted queue on thread %d/%d\n", 
                rpc->id(), thr->llproc()->getPid(), thr->getLWP());

   assert(rpc->getState() == int_iRPC::Prepped);
   assert(thr->getInternalState() == int_thread::stopped);
   pthrd_printf("Setting iRPC %lu, to running\n", rpc->id());

   if (rpc->isMemManagementRPC()) {
      assert(!rpc->allocation());
      //Allocation and deallocation RPCs have their data and address
      // lazily filled in, rather than at post time.  This allows us
      // to delay the creation until we know how big they will be.
      pthrd_printf("Creating allocation for Alloc or Dealloc RPC %lu on %d/%d\n", 
                   rpc->id(), thr->llproc()->getPid(), thr->getLWP());
      bool result = rpc->fillInAllocation();
      assert(result);
   }
   assert(rpc->allocation());

   if (!rpc->thread()->hasSavedRPCRegs()) {
      pthrd_printf("Saving original application registers for %d/%d\n",
                   thr->llproc()->getPid(), thr->getLWP());
      bool result = rpc->thread()->saveRegsForRPC();
      assert(result);
   }
   
   if (rpc->shouldSaveData() && !rpc->allocation()->orig_data) {
      pthrd_printf("Saving original %lu bytes of memory at %lx from application " 
                   "for %d/%d\n", (unsigned long) rpc->allocSize(), rpc->addr(),
                   thr->llproc()->getPid(), thr->getLWP());
      rpc->allocation()->orig_data = (char *) malloc(rpc->allocSize());
      bool result = thr->llproc()->readMem(rpc->allocation()->orig_data,
                                           rpc->addr(),
                                           rpc->allocSize());
      assert(result);
   }

   assert(!rpc->allocSize() || (rpc->binarySize() <= rpc->allocSize()));
   assert(rpc->binaryBlob());
   pthrd_printf("Writing rpc %lu memory to %lx->%lx\n", rpc->id(), rpc->addr(),
                rpc->addr()+rpc->binarySize());
   bool result = thr->llproc()->writeMem(rpc->binaryBlob(), rpc->addr(),
                                         rpc->binarySize());
   if (!result) {
      pthrd_printf("Failed to write IRPC\n");
      return false;
   }

   // Pre-maturely mark ready to help workaround bug_freebsd_change_pc
   rpc->markReady();

   Dyninst::Address newpc_addr = rpc->addr() + rpc->startOffset();
   Dyninst::MachRegister pc = Dyninst::MachRegister::getPC(thr->llproc()->getTargetArch());
   pthrd_printf("Setting %d/%d PC to %lx\n", thr->llproc()->getPid(),
                thr->getLWP(), newpc_addr);
   result = thr->setRegister(pc, newpc_addr);
   assert(result);

   if (rpc->needsToDesync() ) {
      if( !rpc->isProcStopRPC() ) {
          if( useHybridLWPControl(thr) ) {
            rpc->setNeedsDesync(false);
            thr->llproc()->threadPool()->restoreInternalState(block);
          }else{
            rpc->setNeedsDesync(false);
            thr->restoreInternalState(block);
          }
      // TODO this condition is incorrect
      }else if( useHybridLWPControl(thr) && block && !mbox()->size() ) {
          if( !thr->intCont() ) {
              pthrd_printf("Failed to continue thread %d/%d for run RPC\n",
                      thr->llproc()->getPid(), thr->getLWP());
              return false;
          }

          if( !thr->llproc()->plat_contProcess() ) {
              pthrd_printf("Failed to continue thread %d/%d to run RPC\n",
                      thr->llproc()->getPid(), thr->getLWP());
              return false;
          }
      }
   }

   return true;
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

bool iRPCMgr:: handleThreadContinue(int_thread *thr, bool user_cont)
{
   int_iRPC::ptr posted_rpc = thr->nextPostedIRPC();
   if (!posted_rpc)
      return true;

   if (!thr->runningRPC()) {
      //The user may have a posted RPC on a stopped thread,
      // ready this before the thread runs.
      pthrd_printf("Readying an IRPC before continuing\n");
      bool result = thr->handleNextPostedIRPC(user_cont);
      if (!result) {
         perr_printf("Failed to handle IRPC\n");
         setLastError(err_internal, "Failed to prep a ready IRPC\n");
         return false;
      }
   }
   if (posted_rpc->getState() == int_iRPC::Ready)
      posted_rpc->setState(int_iRPC::Running);

   return true;
}

int_iRPC::ptr iRPCMgr::createInfMallocRPC(int_process *proc, unsigned long size, 
                                      bool use_addr, Dyninst::Address addr)
{
   int_iRPC::ptr irpc = int_iRPC::ptr(new int_iRPC(NULL, 0, true, true, 0));
   irpc->setType(int_iRPC::InfMalloc);
   size = roundUpPageSize(proc, size);
   
   void *binary_blob;
   unsigned long binary_size, start_offset;
   bool result = createAllocationSnippet(proc,
                                         addr,
                                         use_addr,
                                         size,
                                         binary_blob,
                                         binary_size,
                                         start_offset);
   irpc->setBinaryBlob(binary_blob);
   irpc->setBinarySize(binary_size);
   irpc->setStartOffset(start_offset);
   irpc->setAllocSize(binary_size);
   
   if (!result)
      return int_iRPC::ptr();
   irpc->allocation()->addr = proc->mallocExecMemory(binary_size);
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
   bool result = createDeallocationSnippet(proc,
                                           addr,
                                           size,
                                           binary_blob,
                                           binary_size,
                                           start_offset);
   irpc->setBinaryBlob(binary_blob);
   irpc->setBinarySize(binary_size);
   irpc->setStartOffset(start_offset);
   irpc->setAllocSize(binary_size);
   
   if (!result)
      return int_iRPC::ptr();
   irpc->allocation()->addr = proc->mallocExecMemory(binary_size);
   return irpc;
}

bool iRPCMgr::checkIfNeedsProcStop(int_process *p)
{
   for (int_threadPool::iterator i = p->threadPool()->begin(); i != p->threadPool()->end(); i++) {
      int_thread *thr = *i;
      if (thr->getInternalState() != int_thread::running)
         continue;
      int_iRPC::ptr running = thr->runningRPC();
      if (running && running->isProcStopRPC())
         continue;
      return true;
   }
   return false;
}

bool iRPCMgr::stopNeededThreads(int_process *p, bool sync)
{
   bool stopped_something = false;
   bool error = false;

   pthrd_printf("Stopping threads for a process-stop RPC\n");
   for (int_threadPool::iterator i = p->threadPool()->begin(); i != p->threadPool()->end(); i++) {
      int_thread *thr = *i;
      if (thr->getInternalState() != int_thread::running)
         continue;
      int_iRPC::ptr running = thr->runningRPC();
      if (running && running->isProcStopRPC())
         continue;
      bool result = thr->intStop(false);
      if (!result) {
         perr_printf("Error stopping thread %d/%d\n", p->getPid(), thr->getLWP());
         error = true;
         continue;
      }
      stopped_something = true;
   }

   if (stopped_something && sync) {
      bool proc_exited;
      bool result = int_process::waitAndHandleForProc(true, p, proc_exited);
      if (proc_exited) {
         pthrd_printf("Process exited during iRPC setup\n");
         return false;
      }
      if (!result) {
         perr_printf("Error handling stop events\n");
         error = true;
      }
   }
   return !error;
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
  return PostCallbackPriority;
}

bool iRPCHandler::handleEvent(Event::ptr ev)
{
   //An RPC has completed, clean-up
   int_thread *thr = ev->getThread()->llthrd();
   EventRPC *event = static_cast<EventRPC *>(ev.get());
   int_iRPC::ptr rpc = event->getllRPC()->rpc;
   iRPCMgr *mgr = rpcMgr();
   bool isLastRPC = !thr->hasPostedRPCs();
   assert(rpc);
   assert(mgr);
   assert(rpc->getState() == int_iRPC::Cleaning);

   pthrd_printf("Handling RPC %lu completion on %d/%d\n", rpc->id(), 
                thr->llproc()->getPid(), thr->getLWP());

   if (!rpc->isAsync()) {
     thr->decSyncRPCCount();
   }

   if (rpc->getType() == int_iRPC::Allocation) {
      Dyninst::Address addr;
      bool err = false;
      bool result = rpcMgr()->collectAllocationResult(thr, addr, err);
      assert(result && !err);
      rpc->targetAllocation()->addr = addr;
      pthrd_printf("Allocation RPC %lu got memory at %lx\n", rpc->id(), addr);
   }

   if (rpc->getType() == int_iRPC::InfMalloc || rpc->getType() == int_iRPC::InfFree) {
      pthrd_printf("Cleaning up inferior malloc RPC\n");
      bool err;
      Dyninst::Address addr;
      bool result = rpcMgr()->collectAllocationResult(thr, addr, err);
      assert(result && !err);
      rpc->setMallocResult(addr);
      thr->llproc()->freeExecMemory(rpc->addr());
   }

   if (rpc->isProcStopRPC() && rpc->needsToDesync()) {
      pthrd_printf("Restoring state for threads stopped by mem management rpc\n");
      thr->llproc()->threadPool()->restoreInternalState(false);
   }

   if (rpc->isMemManagementRPC()) 
   {
      pthrd_printf("Freeing exec memory at %lx\n", rpc->addr());
      thr->llproc()->freeExecMemory(rpc->addr());
   }

   pthrd_printf("Restoring all registers\n");

   // A pending stop will cause the thread to be run, invalidating the saved registers
   bool clear = isLastRPC || thr->hasPendingStop();

   bool result = thr->restoreRegsForRPC(clear);
   assert(result);

   if (rpc->shouldSaveData()) {
      pthrd_printf("Restoring memory to %lx from %p of size %lu\n",
                   rpc->addr(), rpc->allocation()->orig_data,
                   rpc->allocSize());
      assert(rpc->allocation()->orig_data);
      bool result = thr->llproc()->writeMem(rpc->allocation()->orig_data, 
                                            rpc->addr(), 
                                            rpc->allocSize());
      assert(result);
   }

   pthrd_printf("RPC %lu is moving to state finished\n", rpc->id());
   thr->clearRunningRPC();
   rpc->setState(int_iRPC::Finished);

   if (!isLastRPC) {
      pthrd_printf("Moving next RPC to running state\n");
      result = thr->handleNextPostedIRPC(false);
      if (!result) {
         pthrd_printf("Failed to post next iRPC\n");
         return false;
      }
   }

   return true;
}

IRPC::ptr IRPC::createIRPC(void *binary_blob, unsigned size, 
                           bool async)
{
   int_iRPC::ptr irpc = int_iRPC::ptr(new int_iRPC(binary_blob, size, async));
   rpc_wrapper *wrapper = new rpc_wrapper(irpc);   
   IRPC::ptr rpc = IRPC::ptr(new IRPC(wrapper));
   irpc->setIRPC(rpc);
   irpc->copyBinaryBlob(binary_blob, size);
   irpc->setAsync(async);

   return rpc;
}

IRPC::ptr IRPC::createIRPC(void *binary_blob, unsigned size, 
                           Dyninst::Address addr, bool async)
{
   int_iRPC::ptr irpc = int_iRPC::ptr(new int_iRPC(binary_blob, size, async, true, addr));
   rpc_wrapper *wrapper = new rpc_wrapper(irpc);
   IRPC::ptr rpc = IRPC::ptr(new IRPC(wrapper));
   irpc->setIRPC(rpc);
   irpc->copyBinaryBlob(binary_blob, size);
   irpc->setAsync(async);
   return rpc;
}

IRPC::IRPC(rpc_wrapper *wrapper_) :
   wrapper(wrapper_)
{
}

IRPC::~IRPC()
{
  delete wrapper;
}

rpc_wrapper *IRPC::llrpc() const
{
  return wrapper;
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

