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

#include "stackwalk/h/procstate.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/steppergroup.h"

#include "proccontrol/h/PCProcess.h"
#include "proccontrol/h/ProcessSet.h"
#include "proccontrol/h/PlatFeatures.h"
#include "proccontrol/h/PCErrors.h"

#include "dynutil/h/dyn_regs.h"
#include "dynutil/h/SymReader.h"

#include "stackwalk/src/libstate.h"
#include "stackwalk/src/sw.h"
#include "common/h/IntervalTree.h"
#include <vector>

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace Stackwalker;
using namespace std;

class PCLibraryState : public LibraryState {
private:
   ProcDebug *pdebug;
  
  IntervalTree<Address, LibAddrPair> loadedLibs;

public:
   PCLibraryState(ProcessState *pd);
   ~PCLibraryState();

   bool checkLibraryContains(Address addr, Library::ptr lib);
   virtual bool getLibraryAtAddr(Address addr, LibAddrPair &lib);
   virtual bool getLibraries(std::vector<LibAddrPair> &libs);
   virtual void notifyOfUpdate();
   virtual Address getLibTrapAddress();
   virtual bool getAOut(LibAddrPair &ao);
  
  bool updateLibraries();
  bool cacheLibraryRanges(Library::ptr lib);

   void checkForNewLib(Library::ptr lib);
};

ProcDebug::ProcDebug(Process::ptr p) :
   ProcessState(p->getPid()),
   proc(p)
{
}

ProcDebug *ProcDebug::newProcDebug(PID pid, std::string executable)
{
   Process::ptr proc = Process::attachProcess(pid, executable);
   if (!proc) {
      Stackwalker::setLastError(err_proccontrol, ProcControlAPI::getLastErrorMsg());
      sw_printf("[%s:%u] - ProcControl error creating process\n", __FILE__, __LINE__);
      return NULL;
   }
   
   return newProcDebug(proc);
}

ProcDebug *ProcDebug::newProcDebug(Dyninst::ProcControlAPI::Process::ptr proc)
{
   ProcDebug *pd = new ProcDebug(proc);
   pd->library_tracker = new PCLibraryState(pd);

   return pd;
}

bool ProcDebug::newProcDebugSet(const std::vector<PID> &pids,
                                std::vector<ProcDebug *> & out_set)
{
   for (vector<PID>::const_iterator i = pids.begin(); i != pids.end(); i++) {
      ProcDebug *pd = ProcDebug::newProcDebug(*i);
      if (!pd)
         return false;
      out_set.push_back(pd);
   }
   return true;
}

ProcDebug *ProcDebug::newProcDebug(std::string executable, 
                                   const std::vector<std::string> &argv)
{
   Process::ptr proc = Process::createProcess(executable, argv);
   if (!proc) {
      Stackwalker::setLastError(err_proccontrol, ProcControlAPI::getLastErrorMsg());
      sw_printf("[%s:%u] - ProcControl error creating process\n", __FILE__, __LINE__);
      return NULL;
   }

   ProcDebug *pd = new ProcDebug(proc);
   pd->library_tracker = new PCLibraryState(pd);
   
   return pd;
}

ProcDebug::~ProcDebug()
{
   if (library_tracker)
      delete library_tracker;
   library_tracker = NULL;
}

#define CHECK_PROC_LIVE \
   do { \
   if (!proc || proc->isTerminated()) { \
     sw_printf("[%s:%u] - operation on exited process\n", __FILE__, __LINE__); \
     Stackwalker::setLastError(err_procexit, "Process has exited or been detached"); \
     return false; \
   } \
   } while (0)

bool ProcDebug::getRegValue(MachRegister reg, THR_ID thread, 
                            MachRegisterVal &val)
{
   CHECK_PROC_LIVE;
   if (reg == FrameBase) {
      reg = MachRegister::getFramePointer(getArchitecture());
   }
   else if (reg == ReturnAddr) {
      reg = MachRegister::getPC(getArchitecture());
   }
   else if (reg == StackTop) {
      reg = MachRegister::getStackPointer(getArchitecture());
   }
   ThreadPool::iterator thrd_i = proc->threads().find(thread);
   if (thrd_i == proc->threads().end()) {
      sw_printf("[%s:%u] - Invalid thread ID to getRegValue\n", __FILE__, __LINE__);
      Stackwalker::setLastError(err_badparam, "Invalid thread ID\n");
      return false;
   }
   Thread::ptr thrd = *thrd_i;
   bool result = thrd->getRegister(reg, val);
   if (!result) {
      sw_printf("[%s:%u] - ProcControlAPI error reading register\n", __FILE__, __LINE__);
      Stackwalker::setLastError(err_proccontrol, ProcControlAPI::getLastErrorMsg());
   }
   return result;
}

bool ProcDebug::readMem(void *dest, Address source, size_t size)
{
   CHECK_PROC_LIVE;
   bool result = proc->readMemory(dest, source, size);
   if (!result) {
     sw_printf("[%s:%u] - ProcControlAPI error reading memory at 0x%lx\n", __FILE__, __LINE__, source);
      Stackwalker::setLastError(err_proccontrol, ProcControlAPI::getLastErrorMsg());
   }
   return result;
}

bool ProcDebug::getThreadIds(std::vector<THR_ID> &thrds)
{
   CHECK_PROC_LIVE;
   ThreadPool::iterator i = proc->threads().begin();
   for (; i != proc->threads().end(); i++) {
      thrds.push_back((*i)->getLWP());
   }
   return true;
}

bool ProcDebug::getDefaultThread(THR_ID &default_tid)
{
   CHECK_PROC_LIVE;
   default_tid = proc->threads().getInitialThread()->getLWP();
   return true;
}

unsigned ProcDebug::getAddressWidth() 
{
   CHECK_PROC_LIVE;
   return getArchAddressWidth(proc->getArchitecture());
}

bool ProcDebug::preStackwalk(THR_ID tid)
{
   CHECK_PROC_LIVE;
   if (tid == NULL_THR_ID)
      getDefaultThread(tid);
   sw_printf("[%s:%u] - Calling preStackwalk for thread %d\n", __FILE__, __LINE__, tid);
   
   ThreadPool::iterator thread_iter = proc->threads().find(tid);
   if (thread_iter == proc->threads().end()) {
      sw_printf("[%s:%u] - Stackwalk on non-existant thread\n", __FILE__, __LINE__);
      Stackwalker::setLastError(err_badparam, "Invalid thread ID\n");
      return false;     
   }
   Thread::ptr active_thread = *thread_iter;

   if (active_thread->isRunning()) {
      sw_printf("[%s:%u] - Stopping running thread %d\n", __FILE__, __LINE__, tid);
      bool result = active_thread->stopThread();
      if (!result) {
         sw_printf("[%s:%u] - Error stopping thread\n", __FILE__, __LINE__);
         Stackwalker::setLastError(err_proccontrol, "Could not stop thread for stackwalk\n");
         return false;
      }
      needs_resume.insert(active_thread);
   }
   return true;
}   

bool ProcDebug::postStackwalk(THR_ID tid)
{
   CHECK_PROC_LIVE;
   if (tid == NULL_THR_ID)
      getDefaultThread(tid);
   sw_printf("[%s:%u] - Calling postStackwalk for thread %d\n", __FILE__, __LINE__, tid);
   
   ThreadPool::iterator thread_iter = proc->threads().find(tid);
   if (thread_iter == proc->threads().end()) {
      sw_printf("[%s:%u] - Stackwalk on non-existant thread\n", __FILE__, __LINE__);
      Stackwalker::setLastError(err_badparam, "Invalid thread ID\n");
      return false;     
   }
   Thread::ptr active_thread = *thread_iter;
   
   set<Thread::ptr>::iterator i = needs_resume.find(active_thread);
   if (i != needs_resume.end()) {
      sw_printf("[%s:%u] - Resuming thread %d after stackwalk\n", __FILE__, __LINE__, tid);
      bool result = active_thread->continueThread();
      if (!result) {
         sw_printf("[%s:%u] - Error resuming stopped thread %d\n", __FILE__, __LINE__, tid);
         Stackwalker::setLastError(err_proccontrol, ProcControlAPI::getLastErrorMsg());
         return false;
      }
      needs_resume.erase(i);
   }
   return true;
}
  
bool ProcDebug::pause(THR_ID tid)
{
   CHECK_PROC_LIVE;
   if (tid == NULL_THR_ID) {
      sw_printf("[%s:%u] - Stopping process %d\n", __FILE__, __LINE__, proc->getPid());

      bool result = proc->stopProc();
      if (!result) {
         sw_printf("[%s:%u] - Error stopping process %d\n", 
                   __FILE__, __LINE__, proc->getPid());
         Stackwalker::setLastError(err_proccontrol, ProcControlAPI::getLastErrorMsg());
         return false;
      }
      return true;
   }

   ThreadPool::iterator thread_iter = proc->threads().find(tid);
   if (thread_iter == proc->threads().end()) {
      sw_printf("[%s:%u] - stop on non-existant thread\n", __FILE__, __LINE__);
      Stackwalker::setLastError(err_badparam, "Invalid thread ID\n");
      return false;     
   }
   Thread::ptr thread = *thread_iter;
   sw_printf("[%s:%u] - Stopping thread %d\n", __FILE__, __LINE__, tid);

   if (thread->isStopped()) {
      sw_printf("[%s:%u] - Thread %d is already stopped\n", __FILE__, __LINE__, tid);
      return true;
   }

   bool result = thread->stopThread();
   if (!result) {
      sw_printf("[%s:%u] - Error stopping thread %d\n", __FILE__, __LINE__, tid);
      Stackwalker::setLastError(err_proccontrol, ProcControlAPI::getLastErrorMsg());
      return false;
   }
   
   return true;
}

bool ProcDebug::resume(THR_ID tid)
{
   CHECK_PROC_LIVE;
   if (tid == NULL_THR_ID) {
      sw_printf("[%s:%u] - Running process %d\n", __FILE__, __LINE__, proc->getPid());

      bool result = proc->continueProc();
      if (!result) {
         sw_printf("[%s:%u] - Error running process %d\n", 
                   __FILE__, __LINE__, proc->getPid());
         Stackwalker::setLastError(err_proccontrol, ProcControlAPI::getLastErrorMsg());
         return false;
      }
      return true;
   }

   ThreadPool::iterator thread_iter = proc->threads().find(tid);
   if (thread_iter == proc->threads().end()) {
      sw_printf("[%s:%u] - continue on non-existant thread\n", __FILE__, __LINE__);
      Stackwalker::setLastError(err_badparam, "Invalid thread ID\n");
      return false;     
   }
   Thread::ptr thread = *thread_iter;
   sw_printf("[%s:%u] - Running thread %d\n", __FILE__, __LINE__, tid);

   if (thread->isRunning()) {
      sw_printf("[%s:%u] - Thread %d is already running\n", __FILE__, __LINE__, tid);
      return true;
   }

   bool result = thread->continueThread();
   if (!result) {
      sw_printf("[%s:%u] - Error running thread %d\n", __FILE__, __LINE__, tid);
      Stackwalker::setLastError(err_proccontrol, ProcControlAPI::getLastErrorMsg());
      return false;
   }
   
   return true;
}

bool ProcDebug::isTerminated()
{
   return (!proc || proc->isTerminated());
}

bool ProcDebug::detach(bool leave_stopped)
{
   CHECK_PROC_LIVE;   
   bool result = proc->detach(leave_stopped);
   if (!result) {
      sw_printf("[%s:%u] - Error detaching from process %d\n", __FILE__, __LINE__, 
                proc->getPid());
      Stackwalker::setLastError(err_proccontrol, ProcControlAPI::getLastErrorMsg());
      return false;
   }
   return true;
}

int ProcDebug::getNotificationFD()
{
   return -1;
}

std::string ProcDebug::getExecutablePath()
{
   CHECK_PROC_LIVE;
   return proc->libraries().getExecutable()->getName();
}

bool ProcDebug::handleDebugEvent(bool block)
{
   bool result = Process::handleEvents(block);
   if (!result) {
      sw_printf("[%s:%u] - Error handling debug events\n", __FILE__, __LINE__);
      Stackwalker::setLastError(err_proccontrol, ProcControlAPI::getLastErrorMsg());
      return false;
   }
   return true;
}

bool ProcDebug::isFirstParty()
{
   return false;
}

Architecture ProcDebug::getArchitecture()
{
   return proc->getArchitecture();
}

Process::ptr ProcDebug::getProc()
{
   return proc;
}

PCLibraryState::PCLibraryState(ProcessState *pd) :
   LibraryState(pd)
{
   pdebug = static_cast<ProcDebug *>(pd);
}
   
PCLibraryState::~PCLibraryState()
{
}
 
bool PCLibraryState::cacheLibraryRanges(Library::ptr lib)
{
   std::string filename = lib->getName();
   Address base = lib->getLoadAddress();

   SymbolReaderFactory *fact = getDefaultSymbolReader();
   SymReader *reader = fact->openSymbolReader(filename);
   if (!reader) {
      sw_printf("[%s:%u] - Error could not open expected file %s\n", 
                __FILE__, __LINE__, filename.c_str());
      return false;
   }

   int num_segments = reader->numSegments();
   for (int i=0; i<num_segments; i++) {
      SymSegment segment;
      reader->getSegment(i, segment);
      if (segment.type != 1) continue;
      Address segment_start = segment.mem_addr + base;
      Address segment_end = segment_start + segment.mem_size;

    loadedLibs.insert(segment_start, segment_end, LibAddrPair(lib->getName(), lib->getLoadAddress()));
   }
   return true;
}

bool PCLibraryState::checkLibraryContains(Address addr, Library::ptr lib)
{
   std::string filename = lib->getName();
   Address base = lib->getLoadAddress();

   SymbolReaderFactory *fact = Walker::getSymbolReader();
   SymReader *reader = fact->openSymbolReader(filename);
   if (!reader) {
      sw_printf("[%s:%u] - Error could not open expected file %s\n", 
                __FILE__, __LINE__, filename.c_str());
      return false;
   }

   int num_regions = reader->numSegments();
   for (int i=0; i<num_regions; i++) {
      SymSegment region;
      reader->getSegment(i, region);
      Address region_start = region.mem_addr + base;
      Address region_end = region_start + region.mem_size;
      if (region_start <= addr && region_end > addr) 
         return true;
   }
   return false;
}

void PCLibraryState::checkForNewLib(Library::ptr lib)
{
   
   if (lib->getData())
      return;
   sw_printf("[%s:%u] - Detected new library %s at %lx, notifying\n",
             __FILE__, __LINE__, lib->getName().c_str(), lib->getLoadAddress());
   
   lib->setData((void *) 0x1);

   StepperGroup *group = pdebug->getWalker()->getStepperGroup();
   LibAddrPair la(lib->getName(), lib->getLoadAddress());

   cacheLibraryRanges(lib);

   group->newLibraryNotification(&la, library_load);
}
/*
For a given address, 'addr', PCLibraryState::getLibraryAtAddr returns
the name and load address of the library/executable that loaded over
'addr'.

Traditionally when searching for the library that contains an address,
we would sequentially open each library, read its program headers and
see if that library contained the address.  This caused at-scale
performance problems on apps with lots of libraries, as we turned up
opening a lot of unnecessary files.

This function tries to be smarter.  It identifies the libraries that
most likely to contain our address, and then targets opens at them.
We do this by getting the dynamic address for each library (a pointer
given by the link map).  The dynamic address points to the library's
DYNAMIC section, which must be loaded into memory as specified by the
System V ABI.  We expect that the library containing addr will have a
nearby DYNAMIC pointer, and we check the two libraries with a DYNAMIC
pointer above and below addr.  One of these libraries should contain
addr.

If, for some reason, we fail to get a DYNAMIC section then we'll stash
that library away in 'zero_dynamic_libs' and check it when done.
*/ 
bool PCLibraryState::getLibraryAtAddr(Address addr, LibAddrPair &lib)
{
   Process::ptr proc = pdebug->getProc();
   CHECK_PROC_LIVE;

   vector<pair<LibAddrPair, unsigned int> > arch_libs;
   updateLibsArch(arch_libs);
   vector<pair<LibAddrPair, unsigned int> >::iterator j;
   for (j = arch_libs.begin(); j != arch_libs.end(); j++) {
      string name = (*j).first.first;
      Address start = (*j).first.second;
      Address size = (*j).second;
      if (addr >= start && addr < start + size) {
         lib.first = name;
         lib.second = start;
         return true;
      }
   }

   bool ret = loadedLibs.find(addr, lib);
   if (ret) {
     return true;
   }
   // Refresh cache
   updateLibraries();
   ret = loadedLibs.find(addr, lib);
   if (ret) {
     return true;
   }

   return false;
}

bool PCLibraryState::getLibraries(std::vector<LibAddrPair> &libs)
{
   Process::ptr proc = pdebug->getProc();
   CHECK_PROC_LIVE;

   LibraryPool::iterator i;   
   for (i = proc->libraries().begin(); i != proc->libraries().end(); i++)
   {
      checkForNewLib(*i);
      libs.push_back(LibAddrPair((*i)->getName(), (*i)->getLoadAddress()));
   }

   vector<pair<LibAddrPair, unsigned int> > arch_libs;
   vector<pair<LibAddrPair, unsigned int> >::iterator j;
   updateLibsArch(arch_libs);
   for (j = arch_libs.begin(); j != arch_libs.end(); j++) {
      libs.push_back(j->first);
   }

   return true;
}

bool PCLibraryState::updateLibraries()
{
   Process::ptr proc = pdebug->getProc();
   CHECK_PROC_LIVE;


   static bool exec = false;

   if (!exec) {
     cacheLibraryRanges(proc->libraries().getExecutable());
     exec = true;
   }


   LibraryPool::iterator i;   
   for (i = proc->libraries().begin(); i != proc->libraries().end(); i++)
   {
      checkForNewLib(*i);
   }

   vector<pair<LibAddrPair, unsigned int> > arch_libs;
   vector<pair<LibAddrPair, unsigned int> >::iterator j;
   updateLibsArch(arch_libs);

   return true;
}

void PCLibraryState::notifyOfUpdate()
{
}

Address PCLibraryState::getLibTrapAddress()
{
   return 0;
}

bool PCLibraryState::getAOut(LibAddrPair &ao)
{
   Process::ptr proc = pdebug->getProc();
   CHECK_PROC_LIVE;

   Library::ptr lib = proc->libraries().getExecutable();
   if (!lib) {
      sw_printf("[%s:%u] - Could not get executable\n", __FILE__, __LINE__);
      return false;
   }
   ao = LibAddrPair(lib->getName(), lib->getLoadAddress());
   return true;
}

void int_walkerSet::addToProcSet(ProcDebug *pd)
{
   ProcessSet::ptr &pset = *((ProcessSet::ptr *) procset);
   Process::ptr proc = pd->getProc();
   pset->insert(proc);
}

void int_walkerSet::eraseFromProcSet(ProcDebug *pd)
{
   ProcessSet::ptr &pset = *((ProcessSet::ptr *) procset);
   Process::ptr proc = pd->getProc();

   ProcessSet::iterator i = pset->find(proc);
   assert(i != pset->end());
   pset->erase(i);
}

void int_walkerSet::clearProcSet()
{
   ProcessSet::ptr *pset = (ProcessSet::ptr *) procset;
   (*pset)->clear();
   delete pset;
   procset = NULL;
}

void int_walkerSet::initProcSet()
{
   ProcessSet::ptr *p = new ProcessSet::ptr();
   *p = ProcessSet::newProcessSet();
   procset = (void *) p;
}

class StackCallback : public Dyninst::ProcControlAPI::CallStackCallback
{
private:
   CallTree &tree;
   FrameNode *cur;
   Walker *cur_walker;
public:
   StackCallback(CallTree &t);
   virtual ~StackCallback();
   
   virtual bool beginStackWalk(Thread::ptr thr);
   virtual bool addStackFrame(Thread::ptr thr, Dyninst::Address ra, Dyninst::Address sp, Dyninst::Address fp);
   virtual void endStackWalk(Thread::ptr thr);
};

StackCallback::StackCallback(CallTree &t) :
   tree(t),
   cur(NULL)
{
   top_first = true;
}

StackCallback::~StackCallback()
{
}

bool StackCallback::beginStackWalk(Thread::ptr thr)
{
   assert(!cur);
   Process::ptr proc = thr->getProcess();
   ProcessState *pstate = ProcessState::getProcessStateByPid(proc->getPid());
   if (!pstate) {
      sw_printf("[%s:%u] - Error, unknown process state for %d while starting stackwalk\n", 
                __FILE__, __LINE__, proc->getPid());
      return false;
   }

   cur_walker = pstate->getWalker();
   cur = tree.getHead();

   return true;
}

bool StackCallback::addStackFrame(Thread::ptr thr,
                                  Dyninst::Address ra, Dyninst::Address sp, Dyninst::Address fp)
{
   Frame f(cur_walker);
   f.setRA(ra);
   f.setSP(sp);
   f.setFP(fp);
   f.setThread(thr->getLWP());
   
   cur = tree.addFrame(f, cur);
   return true;
}

void StackCallback::endStackWalk(Thread::ptr thr) {
   THR_ID thrd_lwp = thr->getLWP();
   Frame *last_frame = cur->getFrame();
   if (last_frame) {
      last_frame->markTopFrame();
   }
   tree.addThread(thrd_lwp, cur, cur_walker, false);
   cur = NULL;
   cur_walker = NULL;
}

bool int_walkerSet::walkStacksProcSet(CallTree &tree, bool &bad_plat, bool walk_initial_only)
{
   ProcessSet::ptr &pset = *((ProcessSet::ptr *) procset);
   ThreadSet::ptr all_threads = ThreadSet::newThreadSet(pset, walk_initial_only);
   StackCallback cbs(tree);

   if (!all_threads->getCallStackUnwinding()) {
      bad_plat = true;
      return false;
   }
   return all_threads->getCallStackUnwinding()->walkStack(&cbs);
}
