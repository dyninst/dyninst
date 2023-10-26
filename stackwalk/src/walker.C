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

#include "stackwalk/h/walker.h"
#include "stackwalk/h/frame.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/symlookup.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/steppergroup.h"
#include "stackwalk/src/sw.h"
#include "stackwalk/src/libstate.h"
#include <assert.h>
#include "registers/abstract_regs.h"

using namespace Dyninst;
using namespace Dyninst::Stackwalker;
using namespace std;

SymbolReaderFactory *Walker::symrfact = NULL;

void Walker::version(int& major, int& minor, int& maintenance)
{
    major = SW_MAJOR;
    minor = SW_MINOR;
    maintenance = SW_BETA;
}


Walker::Walker(ProcessState *p,
               StepperGroup *grp,
               SymbolLookup *sym,
               bool default_steppers,
               std::string exec_name) :
   proc(NULL),
   lookup(NULL),
   creation_error(false),
   call_count(0)
{
   bool result;
   //Always start with a process object
   assert(p);
   proc = p;
   proc->walker = this;

   sw_printf("[%s:%d] - Creating new Walker with proc=%p, sym=%p, step = %d\n",
             FILE__, __LINE__, (void*)proc, (void*)sym, (int) default_steppers);
   group = grp ? grp : createDefaultStepperGroup();
   if (default_steppers) {
      result = createDefaultSteppers();
      if (!result) {
         sw_printf("[%s:%d] - Error creating default steppers\n",
                   FILE__, __LINE__);
         creation_error = true;
         return;
      }
   }

   lookup = sym ? sym : createDefaultSymLookup(exec_name);
   if (lookup) {
      lookup->walker = this;
   }
   else {
      sw_printf("[%s:%d] - WARNING, no symbol lookup available\n",
                FILE__, __LINE__);
   }
}

Walker* Walker::newWalker(std::string exec_name)
{
  sw_printf("[%s:%d] - Creating new stackwalker on current process\n",
            FILE__, __LINE__);

  ProcessState *newproc = createDefaultProcess(exec_name);
  if (!newproc) {
    sw_printf("[%s:%d] - Error creating default process\n",
	      FILE__, __LINE__);
    return NULL;
  }

  Walker *newwalker = new Walker(newproc, NULL, NULL, true, exec_name);
  if (!newwalker || newwalker->creation_error) {
    sw_printf("[%s:%d] - Error creating new Walker object %p\n",
	      FILE__, __LINE__, (void*)newwalker);
    return NULL;
  }

  sw_printf("[%s:%d] - Successfully created Walker %p\n",
	    FILE__, __LINE__, (void*)newwalker);

  return newwalker;
}

Walker *Walker::newWalker(Dyninst::PID pid,
                          std::string executable)
{
  sw_printf("[%s:%d] - Creating new stackwalker for process %d on %s\n",
            FILE__, __LINE__, (int) pid, executable.c_str());

  ProcessState *newproc = createDefaultProcess(pid, executable);
  if (!newproc) {
    sw_printf("[%s:%d] - Error creating default process\n",
	      FILE__, __LINE__);
    return NULL;
  }

  Walker *newwalker = new Walker(newproc, NULL, NULL, true, executable);
  if (!newwalker || newwalker->creation_error) {
    sw_printf("[%s:%d] - Error creating new Walker object %p\n",
	      FILE__, __LINE__, (void*)newwalker);
    return NULL;
  }

  sw_printf("[%s:%d] - Successfully created Walker %p\n",
	    FILE__, __LINE__, (void*)newwalker);

  return newwalker;
}

Walker *Walker::newWalker(std::string exec_name,
                          const std::vector<std::string> &argv)
{
   sw_printf("[%s:%d] - Creating new stackwalker with process %s\n",
             FILE__, __LINE__, exec_name.c_str());

   ProcessState *newproc = createDefaultProcess(exec_name, argv);
   if (!newproc) {
      sw_printf("[%s:%d] - Error creating default process\n",
                FILE__, __LINE__);
      return NULL;
   }

   Walker *newwalker = new Walker(newproc, NULL, NULL, true, exec_name);
   if (!newwalker || newwalker->creation_error) {
      sw_printf("[%s:%d] - Error creating new Walker object %p\n",
                FILE__, __LINE__, (void*)newwalker);
      return NULL;
   }

   sw_printf("[%s:%d] - Successfully created Walker %p\n",
             FILE__, __LINE__, (void*)newwalker);

   return newwalker;
}

Walker *Walker::newWalker(ProcessState *proc,
                          StepperGroup *grp,
                          SymbolLookup *lookup,
                          bool default_steppers)
{
   if (!proc) {
      sw_printf("[%s:%d] - Error proc parameter to newWalker must not be NULL\n",
                FILE__, __LINE__);
      setLastError(err_badparam, "Tried to create a walker with a NULL " \
                   "ProcessState param");
      return NULL;
   }
   sw_printf("[%s:%d] - Creating custom Walker with proc = %p" \
             "lookup = %p\n", FILE__, __LINE__, (void*)proc, (void*)lookup);

   Walker *newwalker = new Walker(proc, grp, lookup, default_steppers, "");
   if (!newwalker || newwalker->creation_error) {
      sw_printf("[%s:%d] - Error creating new Walker object %p\n",
                FILE__, __LINE__, (void*)newwalker);
      return NULL;
   }

   sw_printf("[%s:%d] - Successfully created Walker %p\n",
             FILE__, __LINE__, (void*)newwalker);

   return newwalker;
}

Walker *Walker::newWalker(Dyninst::PID pid)
{
   return newWalker(pid, "");
}

Walker *Walker::newWalker(Dyninst::ProcControlAPI::Process::ptr proc)
{
  sw_printf("[%s:%d] - Creating new stackwalker for ProcControl process %d\n",
	    FILE__, __LINE__, (int) proc->getPid());

  ProcessState *newproc = createDefaultProcess(proc);
  if (!newproc) {
    sw_printf("[%s:%d] - Error creating default process\n",
	      FILE__, __LINE__);
    return NULL;
  }

  Walker *newwalker = new Walker(newproc, NULL, NULL, true, string());
  if (!newwalker || newwalker->creation_error) {
    sw_printf("[%s:%d] - Error creating new Walker object %p\n",
	      FILE__, __LINE__, (void*)newwalker);
    return NULL;
  }

  sw_printf("[%s:%d] - Successfully created Walker %p\n",
	    FILE__, __LINE__, (void*)newwalker);

  return newwalker;
}

bool Walker::newWalker(const std::vector<Dyninst::PID> &pids,
                       std::vector<Walker *> &walkers_out)
{
   return newWalker(pids, walkers_out, "");
}

bool Walker::newWalker(const std::vector<Dyninst::PID> &pids,
                       std::vector<Walker *> &walkers_out,
                       std::string executable)
{
  sw_printf("[%s:%d] - Creating multiple stackwalkers\n",
	    FILE__, __LINE__);
  unsigned num_errors = 0;

  vector<ProcDebug *> new_dbs;
  bool pd_result = createDefaultProcess(pids, new_dbs);
  if (!pd_result) {
     sw_printf("[%s:%d] - Errors attaching to some processes\n",
               FILE__, __LINE__);
  }

  vector<ProcDebug *>::iterator i;
  for (i = new_dbs.begin(); i != new_dbs.end(); i++) {
     ProcDebug *pd = *i;
     if (!pd) {
        assert(!pd_result);
        walkers_out.push_back(NULL);
        num_errors++;
        continue;
     }

     Walker *newwalker = new Walker((ProcessState *) pd, NULL, NULL, true, executable);
     if (!newwalker || newwalker->creation_error) {
        sw_printf("[%s:%d] - Error creating new Walker object %p\n",
                  FILE__, __LINE__, (void*)newwalker);
        walkers_out.push_back(NULL);
        num_errors++;
        continue;
     }

     sw_printf("[%s:%d] - Successfully created walker for %d\n",
               FILE__, __LINE__, pd->getProcessId());
     walkers_out.push_back(newwalker);
  }

  if (num_errors == pids.size())
     return false;
  return true;
}

Walker::~Walker() {
   if (proc)
      delete proc;
   if (lookup)
      delete lookup;
   delete group;
}

SymbolReaderFactory *Walker::getSymbolReader()
{
   if (Walker::symrfact) {
      return Walker::symrfact;
   }

   SymbolReaderFactory *fact = ProcControlAPI::Process::getDefaultSymbolReader();
   if (fact) {
      Walker::symrfact = fact;
      return fact;
   }

   fact = Stackwalker::getDefaultSymbolReader();
   if (fact) {
      Walker::symrfact = fact;
      return fact;
   }
   return NULL;
}

void Walker::setSymbolReader(SymbolReaderFactory *srf)
{
   symrfact = srf;
   ProcControlAPI::Process::setDefaultSymbolReader(srf);
}

/**
 * What is happening here, you may ask?
 *
 * getInitialFrame returns the active frame on the stack.  However,
 * this is a problem if we want to do a first party stackwalk, because
 * getInitialFrame will return a copy of its own frame and then deconstruct
 * the frame, leaving us with an invalid stack frame.
 *
 * Instead we put the implementation of getInitialFrame into a macro, which
 * means it's inlined into any function that uses it.  Thus, we can do a
 * first party stackwalk by embedding this into the walkStack call, which will
 * get its own frame, then generate a stack frame without destroying the
 * initial frame.
 *
 * This is used in two places, so I figure it's better to use a
 * #define rather than make two copies of this code.  Also, this is only
 * legal to call from a Walker object.
 **/
#define getInitialFrameImpl(frame, thread) \
{ \
  result = true; \
  Dyninst::MachRegister pc_reg, frm_reg, stk_reg; \
  Dyninst::MachRegisterVal pc, sp, fp; \
  location_t loc; \
  if (thread == NULL_THR_ID) { \
    result = proc->getDefaultThread(thread); \
    if (!result) { \
      sw_printf("getDefaultThread returned an error\n"); \
      result = false; \
      goto done_gifi; \
    } \
  } \
  result = proc->getRegValue(Dyninst::ReturnAddr, thread, pc); \
  result = result && proc->getRegValue(Dyninst::StackTop, thread, sp); \
  result = result && proc->getRegValue(Dyninst::FrameBase, thread, fp); \
  if (!result) { \
    sw_printf("Failed to get registers from process\n"); \
    result = false; \
    goto done_gifi; \
  } \
  frame.setRA(pc); \
  frame.setFP(fp); \
  frame.setSP(sp); \
  loc.location = loc_register; \
  loc.val.reg = Dyninst::ReturnAddr; \
  frame.setRALocation(loc); \
  loc.val.reg = Dyninst::StackTop; \
  frame.setSPLocation(loc); \
  loc.val.reg = Dyninst::FrameBase; \
  frame.setFPLocation(loc); \
  frame.setThread(thread); \
  frame.markTopFrame(); \
  done_gifi: ; \
}

bool Walker::walkStack(std::vector<Frame> &stackwalk, THR_ID thread)
{
   bool result;
   Frame initialFrame(this);

   if (thread == NULL_THR_ID) {
      result = proc->getDefaultThread(thread);
      if (!result) {
         sw_printf("[%s:%d] - Couldn't get initial thread on %d\n",
                   FILE__, __LINE__, proc->getProcessId());
         return false;
      }
   }

   result = callPreStackwalk(thread);
   if (!result) {
      sw_printf("[%s:%d] - Call to preStackwalk failed, exiting from stackwalk\n",
                FILE__, __LINE__);
      return false;
   }

   sw_printf("[%s:%d] - Starting stackwalk on thread %d\n",
             FILE__, __LINE__, (int) thread);

   getInitialFrameImpl(initialFrame, thread);
   if (!result) {
      sw_printf("[%s:%d] - Failed to get registers from process on thread %d\n",
                FILE__, __LINE__, (int) thread);
      goto done;
   }

   result = walkStackFromFrame(stackwalk, initialFrame);
   if (!result) {
      sw_printf("[%s:%d] - walkStackFromFrame failed on thread %d\n",
                FILE__, __LINE__, (int) thread);
      goto done;
   }

 done:
   bool postresult = callPostStackwalk(thread);
   if (!postresult) {
      sw_printf("[%s:%d] - Call to postStackwalk failed\n", FILE__, __LINE__);
      return false;
   }
   return result;
}

bool Walker::walkStackFromFrame(std::vector<Frame> &stackwalk,
                                const Frame &frame)
{
   bool result;

   stackwalk.clear();
   stackwalk.push_back(frame);

   sw_printf("[%s:%d] - walkStackFromFrame called with frame at %lx\n",
             FILE__, __LINE__, stackwalk.back().getRA());

   result = callPreStackwalk();
   if (!result) {
      sw_printf("[%s:%d] - Call to preStackwalk failed, exiting from stackwalk\n",
                FILE__, __LINE__);
      return false;
   }


   for (;;) {
     Frame cur_frame(this);
     sw_printf("[%s:%d] - Walking single frame from %lx\n", FILE__, __LINE__,
	       stackwalk.back().getRA());

     result = walkSingleFrame(stackwalk.back(), cur_frame);
     if (!result) {
        if (getLastError() == err_stackbottom) {
           sw_printf("[%s:%d] - Reached bottom of stack\n", FILE__, __LINE__);
           clearLastError();
           result = true;
           goto done;
        }
        sw_printf("[%s:%d] - Error walking through stack frame %s\n",
                  FILE__, __LINE__, getLastErrorMsg());
        result = false;
        goto done;
     }
     stackwalk.back().next_stepper = cur_frame.getStepper();
     size_t cur_capa = stackwalk.capacity();
     stackwalk.push_back(cur_frame);     
     if (cur_capa != stackwalk.capacity()) {
         // If the stackwalk vector reallocates memory,
	 // all prev_frame points become invalid.
	 // So, we need to update them.
         for (size_t i = 1; i < stackwalk.size(); ++i) {
	     stackwalk[i].prev_frame = &(stackwalk[i - 1U]);
	 }
     }
   }

 done:
   bool postresult = callPostStackwalk();
   if (!postresult) {
      sw_printf("[%s:%d] - Call to postStackwalk failed\n", FILE__, __LINE__);
      return false;
   }

   for (std::vector<Frame>::iterator swi = stackwalk.begin();
        swi != stackwalk.end();
        ++swi)
   {
     swi->prev_frame = NULL;
   }

   sw_printf("[%s:%d] - Finished walking callstack from frame, result = %s\n",
             FILE__, __LINE__, result ? "true" : "false");

   return result;
}

bool Walker::walkSingleFrame(const Frame &in, Frame &out)
{
   gcframe_ret_t gcf_result;
   bool result;
   Frame last_frame = in;

   sw_printf("[%s:%d] - Attempting to walk through frame with RA 0x%lx\n",
	     FILE__, __LINE__, last_frame.getRA());

   result = callPreStackwalk();
   if (!result) {
      sw_printf("[%s:%d] - Call to preStackwalk failed, exiting from stackwalk\n",
                FILE__, __LINE__);
      return false;
   }

   if (!group) {
      setLastError(err_nogroup, "Attempt to walk a stack without a StepperGroup");
      return false;
   }

   out.prev_frame = &in;

   FrameStepper *last_stepper = NULL;
   for (;;)
   {
     FrameStepper *cur_stepper = NULL;
     bool res = group->findStepperForAddr(last_frame.getRA(), cur_stepper,
                                          last_stepper);
     if (!res) {
        sw_printf("[%s:%d] - Unable to find a framestepper for %lx\n",
                  FILE__, __LINE__, last_frame.getRA());
        result = false;
        goto done;
     }
     sw_printf("[%s:%d] - Attempting to use stepper %s\n",
               FILE__, __LINE__, cur_stepper->getName());
     gcf_result = cur_stepper->getCallerFrame(in, out);
     if (gcf_result == gcf_success) {
       sw_printf("[%s:%d] - Success using stepper %s on 0x%lx\n",
                 FILE__, __LINE__, cur_stepper->getName(), in.getRA());
       if (!checkValidFrame(in, out)) {
          sw_printf("[%s:%d] - Resulting frame is not valid\n", FILE__, __LINE__);
          result = false;
          goto done;
       }
       sw_printf("[%s:%d] - Returning frame with RA %lx, SP %lx, FP %lx\n",
		 FILE__, __LINE__, out.getRA(), out.getSP(), out.getFP());
       out.setStepper(cur_stepper);
       result = true;
       goto done;
     }
     else if (gcf_result == gcf_not_me) {
       last_stepper = cur_stepper;
       sw_printf("[%s:%d] - Stepper %s declined address 0x%lx\n",
                 FILE__, __LINE__, cur_stepper->getName(), in.getRA());
       continue;
     }
     else if (gcf_result == gcf_stackbottom) {
        sw_printf("[%s:%d] - Stepper %s bottomed out on 0x%lx\n",
                  FILE__, __LINE__, cur_stepper->getName(), in.getRA());
       setLastError(err_stackbottom, "walkSingleFrame reached bottom of stack");
       result = false;
       goto done;
     }
     else if (gcf_result == gcf_error) {
        sw_printf("[%s:%d] - A stepper reported error %p on frame at %lx\n",
                  FILE__, __LINE__, (void*)cur_stepper, in.getRA());
       result = false;
       goto done;
     }
   }

 done:
   out.setThread(in.getThread());
   bool postresult = callPostStackwalk();
   if (!postresult) {
      sw_printf("[%s:%d] - Call to postStackwalk failed\n", FILE__, __LINE__);
      return false;
   }

   return result;
}

bool Walker::getInitialFrame(Frame &frame, THR_ID thread) {
   bool result;
   frame.walker = this;
   result = callPreStackwalk(thread);
   if (!result) {
      sw_printf("[%s:%d] - Call to preStackwalk failed, exiting from stackwalk\n",
                FILE__, __LINE__);
      return false;
   }

   getInitialFrameImpl(frame, thread);
   if (!result) {
      sw_printf("[%s:%d] - getInitialFrameImpl failed on thread %d\n",
                FILE__, __LINE__, (int) thread);
   }
   bool postresult = callPostStackwalk(thread);
   if (!postresult) {
      sw_printf("[%s:%d] - Call to postStackwalk failed\n", FILE__, __LINE__);
      return false;
   }

   return result;
}


bool Walker::getAvailableThreads(std::vector<THR_ID> &threads) const {
   threads.clear();
   bool result = proc->getThreadIds(threads);
   if (dyn_debug_stackwalk) {
      if (!result) {
         sw_printf("[%s:%d] - getThreadIds error\n", FILE__, __LINE__);
      }
      else {
         sw_printf("[%s:%d] - getThreadIds returning %lu values:\t\n",
                   FILE__, __LINE__, threads.size());
         for (unsigned i=0; i<threads.size(); i++) {
            sw_printf("%d ", (int) threads[i]);
         }
         sw_printf("\n ");
      }
   }
   return result;
}

ProcessState *Walker::getProcessState() const {
   return proc;
}

SymbolLookup *Walker::getSymbolLookup() const {
   return lookup;
}

ProcessState *Walker::createDefaultProcess(std::string exec_name)
{
   ProcSelf *pself = new ProcSelf(exec_name);
   pself->initialize();
   return pself;
}

ProcessState *Walker::createDefaultProcess(PID pid, std::string executable)
{
   ProcDebug *pdebug = ProcDebug::newProcDebug(pid, executable);
   return pdebug;
}

ProcessState *Walker::createDefaultProcess(Dyninst::ProcControlAPI::Process::ptr proc)
{
   ProcDebug *pdebug = ProcDebug::newProcDebug(proc);
   return pdebug;
}

bool Walker::createDefaultProcess(const vector<Dyninst::PID> &pids,
                                  vector<ProcDebug *> &pds)
{
   return ProcDebug::newProcDebugSet(pids, pds);
}

ProcessState *Walker::createDefaultProcess(std::string exec_name,
                                           const std::vector<std::string> &argv)
{
   ProcDebug *pdebug = ProcDebug::newProcDebug(exec_name, argv);
   return pdebug;
}

bool Walker::addStepper(FrameStepper *s)
{
   assert(group);
   sw_printf("[%s:%d] - Registering stepper %s with group %p\n",
             FILE__, __LINE__, s->getName(), (void*)group);
   group->registerStepper(s);
   return true;
}

bool Walker::callPreStackwalk(Dyninst::THR_ID tid)
{
   call_count++;
   if (call_count != 1)
      return true;

   return getProcessState()->preStackwalk(tid);
}

bool Walker::callPostStackwalk(Dyninst::THR_ID tid)
{
   call_count--;

   if (call_count != 0)
      return true;

   return getProcessState()->postStackwalk(tid);
}

StepperGroup *Walker::createDefaultStepperGroup()
{
   return new AddrRangeGroup(this);
}

StepperGroup *Walker::getStepperGroup() const
{
   return group;
}

int_walkerSet::int_walkerSet() :
   non_pd_walkers(0)
{
   initProcSet();
}

int_walkerSet::~int_walkerSet()
{
   clearProcSet();
}

pair<set<Walker *>::iterator, bool> int_walkerSet::insert(Walker *w)
{
   ProcDebug *pd = dynamic_cast<ProcDebug *>(w->getProcessState());
   if (!pd) {
      non_pd_walkers++;
   }
   else {
      addToProcSet(pd);
   }

   return walkers.insert(w);
}

void int_walkerSet::erase(set<Walker *>::iterator i)
{
   ProcDebug *pd = dynamic_cast<ProcDebug *>((*i)->getProcessState());
   if (!pd) {
      non_pd_walkers--;
   }
   else {
      eraseFromProcSet(pd);
   }

   walkers.erase(i);
}

WalkerSet *WalkerSet::newWalkerSet()
{
   return new WalkerSet();
}

WalkerSet::WalkerSet() :
   iwalkerset(new int_walkerSet())
{
}

WalkerSet::~WalkerSet()
{
   delete iwalkerset;
}

WalkerSet::iterator WalkerSet::begin() {
   return iwalkerset->walkers.begin();
}

WalkerSet::iterator WalkerSet::end() {
   return iwalkerset->walkers.end();
}

WalkerSet::iterator WalkerSet::find(Walker *w) {
   return iwalkerset->walkers.find(w);
}

WalkerSet::const_iterator WalkerSet::begin() const {
   return ((const int_walkerSet *) iwalkerset)->walkers.begin();
}

WalkerSet::const_iterator WalkerSet::end() const {
   return ((const int_walkerSet *) iwalkerset)->walkers.end();
}

WalkerSet::const_iterator WalkerSet::find(Walker *w) const {
   return ((const int_walkerSet *) iwalkerset)->walkers.find(w);
}

pair<WalkerSet::iterator, bool> WalkerSet::insert(Walker *walker) {
   return iwalkerset->insert(walker);
}

void WalkerSet::erase(WalkerSet::iterator i) {
   iwalkerset->erase(i);
}

bool WalkerSet::empty() const {
   return iwalkerset->walkers.empty();
}

size_t WalkerSet::size() const {
   return iwalkerset->walkers.size();
}

bool WalkerSet::walkStacks(CallTree &tree, bool walk_initial_only) const {
   if (empty()) {
      sw_printf("[%s:%d] - Attempt to walk stacks of empty process set\n", FILE__, __LINE__);
      return false;
   }
   if (!iwalkerset->non_pd_walkers) {
      bool bad_plat = false;
      bool result = iwalkerset->walkStacksProcSet(tree, bad_plat, walk_initial_only);
      if (result) {
         //Success
         return true;
      }
      if (!bad_plat) {
         //Error
         return false;
      }
      sw_printf("[%s:%d] - Platform does not have OS supported unwinding\n", FILE__, __LINE__);
   }

   bool had_error = false;
   for (const_iterator i = begin(); i != end(); i++) {
      vector<THR_ID> threads;
      Walker *walker = *i;
      bool result = walker->getAvailableThreads(threads);
      if (!result) {
         sw_printf("[%s:%d] - Error getting threads for process %d\n", FILE__, __LINE__,
                   walker->getProcessState()->getProcessId());
         had_error = true;
         continue;
      }

      for (vector<THR_ID>::iterator j = threads.begin(); j != threads.end(); j++) {
         std::vector<Frame> swalk;
         THR_ID thr = *j;

         result = walker->walkStack(swalk, thr);
         if (!result && swalk.empty()) {
            sw_printf("[%s:%d] - Error walking stack for %d/%ld\n", FILE__, __LINE__,
                      walker->getProcessState()->getProcessId(), thr);
            had_error = true;
            continue;
         }
         tree.addCallStack(swalk, thr, walker, !result);

         if (walk_initial_only) break;
      }
   }
   return !had_error;
}
