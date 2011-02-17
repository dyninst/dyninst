/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
#include <assert.h>

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
   
   sw_printf("[%s:%u] - Creating new Walker with proc=%p, sym=%p, step = %d\n",
             __FILE__, __LINE__, proc, sym, (int) default_steppers);
   group = grp ? grp : createDefaultStepperGroup();
   if (default_steppers) {
      result = createDefaultSteppers();
      if (!result) {
         sw_printf("[%s:%u] - Error creating default steppers\n",
                   __FILE__, __LINE__);
         creation_error = true;
         return;
      }
   }

   lookup = sym ? sym : createDefaultSymLookup(exec_name);
   if (lookup) {
      lookup->walker = this;
   }
   else {
      sw_printf("[%s:%u] - WARNING, no symbol lookup available\n",
                __FILE__, __LINE__);
   }
}

Walker* Walker::newWalker(std::string exec_name) 
{
  sw_printf("[%s:%u] - Creating new stackwalker on current process\n",
            __FILE__, __LINE__);
   
  ProcessState *newproc = createDefaultProcess(exec_name);
  if (!newproc) {
    sw_printf("[%s:%u] - Error creating default process\n",
	      __FILE__, __LINE__);
    return NULL;
  }
  
  Walker *newwalker = new Walker(newproc, NULL, NULL, true, exec_name);
  if (!newwalker || newwalker->creation_error) {
    sw_printf("[%s:%u] - Error creating new Walker object %p\n",
	      __FILE__, __LINE__, newwalker);
    return NULL;
  }
  
  sw_printf("[%s:%u] - Successfully created Walker %p\n", 
	    __FILE__, __LINE__, newwalker);
  
  return newwalker;
}

Walker *Walker::newWalker(Dyninst::PID pid,
                          std::string executable)
{
  sw_printf("[%s:%u] - Creating new stackwalker for process %d on %s\n",
            __FILE__, __LINE__, (int) pid, executable.c_str());
  
  ProcessState *newproc = createDefaultProcess(pid, executable);
  if (!newproc) {
    sw_printf("[%s:%u] - Error creating default process\n",
	      __FILE__, __LINE__);
    return NULL;
  }

  Walker *newwalker = new Walker(newproc, NULL, NULL, true, executable);
  if (!newwalker || newwalker->creation_error) {
    sw_printf("[%s:%u] - Error creating new Walker object %p\n",
	      __FILE__, __LINE__, newwalker);
    return NULL;
  }
  
  sw_printf("[%s:%u] - Successfully created Walker %p\n", 
	    __FILE__, __LINE__, newwalker);
  
  return newwalker;
}

Walker *Walker::newWalker(std::string exec_name, 
                          const std::vector<std::string> &argv)
{
   sw_printf("[%s:%u] - Creating new stackwalker with process %s\n",
             __FILE__, __LINE__, exec_name.c_str());

   ProcessState *newproc = createDefaultProcess(exec_name, argv);
   if (!newproc) {
      sw_printf("[%s:%u] - Error creating default process\n",
                __FILE__, __LINE__);
      return NULL;
   }

   Walker *newwalker = new Walker(newproc, NULL, NULL, true, exec_name);
   if (!newwalker || newwalker->creation_error) {
      sw_printf("[%s:%u] - Error creating new Walker object %p\n",
                __FILE__, __LINE__, newwalker);
      return NULL;
   }

   sw_printf("[%s:%u] - Successfully created Walker %p\n", 
             __FILE__, __LINE__, newwalker);

   return newwalker;
}

Walker *Walker::newWalker(ProcessState *proc, 
                          StepperGroup *grp,
                          SymbolLookup *lookup,
                          bool default_steppers)
{
   if (!proc) {
      sw_printf("[%s:%u] - Error proc parameter to newWalker must not be NULL\n",
                __FILE__, __LINE__);
      setLastError(err_badparam, "Tried to create a walker with a NULL " \
                   "ProcessState param");
      return NULL;
   }
   sw_printf("[%s:%u] - Creating custom Walker with proc = %p" \
             "lookup = %p\n", __FILE__, __LINE__, proc, lookup);
  
   Walker *newwalker = new Walker(proc, grp, lookup, default_steppers, "");
   if (!newwalker || newwalker->creation_error) {
      sw_printf("[%s:%u] - Error creating new Walker object %p\n",
                __FILE__, __LINE__, newwalker);
      return NULL;
   }
  
   sw_printf("[%s:%u] - Successfully created Walker %p\n", 
             __FILE__, __LINE__, newwalker);
  
   return newwalker;
}

Walker *Walker::newWalker(Dyninst::PID pid)
{
   return newWalker(pid, "");
}

Walker *Walker::newWalker(Dyninst::ProcControlAPI::Process::ptr proc)
{
  sw_printf("[%s:%u] - Creating new stackwalker for ProcControl process %d\n",
	    __FILE__, __LINE__, (int) proc->getPid());
  
  ProcessState *newproc = createDefaultProcess(proc);
  if (!newproc) {
    sw_printf("[%s:%u] - Error creating default process\n",
	      __FILE__, __LINE__);
    return NULL;
  }

  Walker *newwalker = new Walker(newproc, NULL, NULL, true, string());
  if (!newwalker || newwalker->creation_error) {
    sw_printf("[%s:%u] - Error creating new Walker object %p\n",
	      __FILE__, __LINE__, newwalker);
    return NULL;
  }
  
  sw_printf("[%s:%u] - Successfully created Walker %p\n", 
	    __FILE__, __LINE__, newwalker);
  
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
  sw_printf("[%s:%u] - Creating multiple stackwalkers\n",
	    __FILE__, __LINE__);
  unsigned num_errors = 0;
  
  vector<ProcDebug *> new_dbs;
  bool pd_result = createDefaultProcess(pids, new_dbs);
  if (!pd_result) {
     sw_printf("[%s:%u] - Errors attaching to some processes\n",
               __FILE__, __LINE__);
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
        sw_printf("[%s:%u] - Error creating new Walker object %p\n",
                  __FILE__, __LINE__, newwalker);
        walkers_out.push_back(NULL);
        num_errors++;
        continue;
     }

     sw_printf("[%s:%u] - Successfully created walker for %d\n",
               __FILE__, __LINE__, pd->getProcessId());
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
   //TODO: Stepper cleanup
}

SymbolReaderFactory *Walker::getSymbolReader()
{
   return symrfact;
}

void Walker::setSymbolReader(SymbolReaderFactory *srf)
{
   symrfact = srf;
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
  if (thread == NULL_THR_ID) { \
    result = proc->getDefaultThread(thread); \
    if (!result) { \
      sw_printf("getDefaultThread returned an error\n"); \
      result = false; \
      goto done_gifi; \
    } \
  } \
  result = proc->getRegValue(Dyninst::ReturnAddr, thread, pc); \
  result = !result || proc->getRegValue(Dyninst::StackTop, thread, sp); \
  result = !result || proc->getRegValue(Dyninst::FrameBase, thread, fp); \
  if (!result) { \
    sw_printf("Failed to get registers from process\n"); \
    result = false; \
    goto done_gifi; \
  } \
  frame.setRA(pc); \
  frame.setFP(fp); \
  frame.setSP(sp); \
  frame.setThread(thread); \
  done_gifi: ; \
}

bool Walker::walkStack(std::vector<Frame> &stackwalk, THR_ID thread)
{
   bool result;
   Frame initialFrame(this);

   if (thread == NULL_THR_ID) {
      result = proc->getDefaultThread(thread);
      if (!result) {
         sw_printf("[%s:%u] - Couldn't get initial thread on %d\n",
                   __FILE__, __LINE__, proc->getProcessId());
         return false;
      }
   }

   result = callPreStackwalk(thread);
   if (!result) {
      sw_printf("[%s:%u] - Call to preStackwalk failed, exiting from stackwalk\n",
                __FILE__, __LINE__);
      return false;
   }

   sw_printf("[%s:%u] - Starting stackwalk on thread %d\n",
             __FILE__, __LINE__, (int) thread);

   getInitialFrameImpl(initialFrame, thread);
   if (!result) {
      sw_printf("[%s:%u] - Failed to get registers from process\n",
                __FILE__, __LINE__, (int) thread);
      goto done;
   }                       

   result = walkStackFromFrame(stackwalk, initialFrame);
   if (!result) {
      sw_printf("[%s:%u] - walkStackFromFrame failed\n",
                __FILE__, __LINE__, (int) thread);
      goto done;
   }

 done:
   bool postresult = callPostStackwalk(thread);
   if (!postresult) {
      sw_printf("[%s:%u] - Call to postStackwalk failed\n", __FILE__, __LINE__);
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

   sw_printf("[%s:%u] - walkStackFromFrame called with frame at %lx\n",
             __FILE__, __LINE__, stackwalk.back().getRA());

   result = callPreStackwalk();
   if (!result) {
      sw_printf("[%s:%u] - Call to preStackwalk failed, exiting from stackwalk\n",
                __FILE__, __LINE__);
      return false;
   }


   for (;;) {
     Frame cur_frame(this);
     sw_printf("[%s:%u] - Walking single frame from %lx\n", __FILE__, __LINE__, 
	       stackwalk.back().getRA());

     result = walkSingleFrame(stackwalk.back(), cur_frame);
     if (!result) {
        if (getLastError() == err_stackbottom) {
           sw_printf("[%s:%u] - Reached bottom of stack\n", __FILE__, __LINE__);
           clearLastError();
           result = true;
           goto done;
        }
        sw_printf("[%s:%u] - Error walking through stack frame %s\n", 
                  __FILE__, __LINE__, getLastErrorMsg());
        result = false;
        goto done;
     }
     stackwalk.back().next_stepper = cur_frame.getStepper();
     stackwalk.push_back(cur_frame);
   }       

 done:
   bool postresult = callPostStackwalk();
   if (!postresult) {
      sw_printf("[%s:%u] - Call to postStackwalk failed\n", __FILE__, __LINE__);
      return false;
   }

   for (std::vector<Frame>::iterator swi = stackwalk.begin();
        swi != stackwalk.end();
        ++swi)
   {
     swi->prev_frame = NULL;
   }

   sw_printf("[%s:%u] - Finished walking callstack from frame, result = %s\n",
             __FILE__, __LINE__, result ? "true" : "false");

   return result;
}

bool Walker::walkSingleFrame(const Frame &in, Frame &out)
{
   gcframe_ret_t gcf_result;
   bool result;
   Frame last_frame = in;

   sw_printf("[%s:%u] - Attempting to walk through frame with RA 0x%lx\n",
	     __FILE__, __LINE__, last_frame.getRA());

   result = callPreStackwalk();
   if (!result) {
      sw_printf("[%s:%u] - Call to preStackwalk failed, exiting from stackwalk\n",
                __FILE__, __LINE__);
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
        sw_printf("[%s:%u] - Unable to find a framestepper for %lx\n",
                  __FILE__, __LINE__, last_frame.getRA());
        result = false;
        goto done;
     }
     sw_printf("[%s:%u] - Attempting to use stepper %s\n", 
               __FILE__, __LINE__, cur_stepper->getName());
     gcf_result = cur_stepper->getCallerFrame(in, out);
     if (gcf_result == gcf_success) {
       sw_printf("[%s:%u] - Success using stepper %s on 0x%lx\n", 
                 __FILE__, __LINE__, cur_stepper->getName(), in.getRA());
       if (!checkValidFrame(in, out)) {
          sw_printf("[%s:%u] - Resulting frame is not valid\n", __FILE__, __LINE__);
          result = false;
          goto done;
       }
       sw_printf("[%s:%u] - Returning frame with RA %lx, SP %lx, FP %lx\n",
		 __FILE__, __LINE__, out.getRA(), out.getSP(), out.getFP());
       out.setStepper(cur_stepper);
       result = true;
       goto done;
     }
     else if (gcf_result == gcf_not_me) {
       last_stepper = cur_stepper;
       sw_printf("[%s:%u] - Stepper %s declined address 0x%lx\n", 
                 __FILE__, __LINE__, cur_stepper->getName(), in.getRA());
       continue; 
     }
     else if (gcf_result == gcf_stackbottom) {
        sw_printf("[%s:%u] - Stepper %s bottomed out on 0x%lx\n", 
                  __FILE__, __LINE__, cur_stepper->getName(), in.getRA());
       setLastError(err_stackbottom, "walkSingleFrame reached bottom of stack");
       result = false;
       goto done;
     }
     else if (gcf_result == gcf_error) {
        sw_printf("[%s:%u] - A stepper reported error %d on frame at %lx\n", 
                  __FILE__, __LINE__, cur_stepper, in.getRA());
       result = false;
       goto done;
     }
   }

 done:
   out.setThread(in.getThread());
   bool postresult = callPostStackwalk();
   if (!postresult) {
      sw_printf("[%s:%u] - Call to postStackwalk failed\n", __FILE__, __LINE__);
      return false;
   }

   return result;
}

bool Walker::getInitialFrame(Frame &frame, THR_ID thread) {
   bool result;
   result = callPreStackwalk(thread);
   if (!result) {
      sw_printf("[%s:%u] - Call to preStackwalk failed, exiting from stackwalk\n",
                __FILE__, __LINE__);
      return false;
   }

   getInitialFrameImpl(frame, thread);
   if (!result) {
      sw_printf("[%s:%u] - getInitialFrameImpl failed\n",
                __FILE__, __LINE__, (int) thread);
   }
   bool postresult = callPostStackwalk(thread);
   if (!postresult) {
      sw_printf("[%s:%u] - Call to postStackwalk failed\n", __FILE__, __LINE__);
      return false;
   }

   return result;
}


bool Walker::getAvailableThreads(std::vector<THR_ID> &threads) const {
   threads.clear();
   bool result = proc->getThreadIds(threads);
   if (dyn_debug_stackwalk) {
      if (!result) {
         sw_printf("[%s:%u] - getThreadIds error\n", __FILE__, __LINE__);
      }
      else {
         sw_printf("[%s:%u] - getThreadIds returning %d values:\t\n", 
                   __FILE__, __LINE__, threads.size());
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
   sw_printf("[%s:%u] - Registering stepper %s with group %p\n",
             __FILE__, __LINE__, s->getName(), group);
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
