/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: BPatch_thread.C,v 1.135 2005/10/18 21:35:28 bernat Exp $

#define BPATCH_FILE

#include "BPatch_thread.h"
#include "BPatch.h"
#include "BPatch_libInfo.h"
#include "BPatch_function.h"
#include "process.h"
#include "dyn_thread.h"
#include "dyn_lwp.h"
#include "BPatch_libInfo.h"

/*
 * BPatch_thread::getCallStack
 *
 * Returns information about the frames currently on the thread's stack.
 *
 * stack	The vector to fill with the stack trace information.
 */
bool BPatch_thread::getCallStackInt(BPatch_Vector<BPatch_frame>& stack)
{
   pdvector<Frame> stackWalk;   
   bool was_stopped = false;

   was_stopped = proc->isStopped();
   if (!was_stopped)
      proc->stopExecution();

   llthread->walkStack(stackWalk);

   // The internal representation of a stack walk treats instrumentation
   // as part of the original instrumented function. That is to say, if A() 
   // calls B(), and B() is instrumented, the stack will appear as so:
   // A()
   // instrumentation

   // We want it to look like so:
   // A()
   // B()
   // instrumentation

   // We handle this by adding a synthetic frame to the stack walk whenever
   // we discover an instrumentation frame.

   for (unsigned int i = 0; i < stackWalk.size(); i++) {
      bool isSignalFrame = false;
      bool isInstrumentation = false;
      BPatch_point *point = NULL;

      Frame frame = stackWalk[i];
      frame.calcFrameType();
      if (frame.frameType_ != FRAME_unset) {
         isSignalFrame = frame.isSignalFrame();
         isInstrumentation = frame.isInstrumentation();
      }

      if (isInstrumentation) {
         // This is a bit of a slog, actually. We want to only show
         // bpatch points that exist, not describe internals to the
         // user. So instead of calling findOrCreateBPPoint, we manually
         // poke through the mapping table. If there isn't a point, we
         // skip this instrumentation frame instead
         instPoint *iP = frame.getPoint();
         if (iP && proc->instp_map->defines(iP))
            point = proc->instp_map->get(iP);
         if (point) {
            stack.push_back(BPatch_frame(this,
                                         (void*)stackWalk[i].getPC(),
                                         (void*)stackWalk[i].getFP(),
                                         false,
                                         true,
                                         point));
            // And the "top-level function" one.
            stack.push_back(BPatch_frame(this,
                                         (void *)stackWalk[i].getUninstAddr(),
                                         (void *)stackWalk[i].getFP(),
                                         false, // not signal handler,
                                         false, // not inst.
                                         NULL, // No point
                                         true)); // Synthesized frame
         }
         else {
            // No point = internal instrumentation, make it go away.
            stack.push_back(BPatch_frame(this,
                                         (void *)stackWalk[i].getUninstAddr(),
                                         (void *)stackWalk[i].getFP(),
                                         false, // not signal handler,
                                         false, // not inst.
                                         NULL, // No point
                                         false)); // Synthesized frame
                
         }
      }
      else {
         // Not instrumentation, normal case
         stack.push_back(BPatch_frame(this,
                                      (void *)stackWalk[i].getPC(),
                                      (void *)stackWalk[i].getFP(),
                                      isSignalFrame));
      }
   }
   if (!was_stopped)
      proc->continueExecution();
   return true;
}

BPatch_thread::BPatch_thread(BPatch_process *parent, int ind, int lwp_id) 
{
   proc = parent;
   index = ind;
   dyn_lwp *lwp = proc->llproc->getLWP(lwp_id);
   if (!lwp)
      lwp = new dyn_lwp(lwp_id, proc->llproc);
   llthread = new dyn_thread(proc->llproc, index, lwp);   
   legacy_destructor = true;
   updated = false;
}

BPatch_thread::BPatch_thread(BPatch_process *parent, dyn_thread *dthr)
{
   index = 0;
   proc = parent;
   llthread = dthr;
   legacy_destructor = true;
   updated = false;
}

void BPatch_thread::updateValues(dynthread_t tid, unsigned long stack_start,
                                 BPatch_function *initial_func, int lwp_id)
{
   BPatch_Vector<BPatch_frame> stack;

   dyn_lwp *lwp = proc->llproc->getLWP(lwp_id);

   updated = true;
   llthread->update_stack_addr(stack_start);
   llthread->update_lwp(lwp);

   if (tid == -1)
   {
      //Expensive... uses an iRPC to get it from the RT library
      tid = proc->llproc->mapIndexToTid(index);
   }
   llthread->update_tid(tid);

   //If initial_func or stack_start aren't provided then
   // we can update them with a stack walk
   if (!initial_func || !stack_start)
   {
      getCallStack(stack);

      int pos = stack.size() - 1;
      //Consider stack_start as starting at the first
      //function with a stack frame.
      while ((!stack_start || !initial_func) && (pos >= 0))
      {
         if (!stack_start)
            stack_start = (unsigned long) stack[pos].getFP();
         if (!initial_func)
         {
            initial_func = stack[pos].findFunction();
         }
         pos--;
      }
   }

   if (initial_func)
   {
      llthread->update_start_func(initial_func->func);
   }
   llthread->update_stack_addr(stack_start);
}

unsigned BPatch_thread::getBPatchIDInt()
{
   return index;
}

BPatch_process *BPatch_thread::getProcessInt() 
{
   return proc;
}

dynthread_t BPatch_thread::getTidInt()
{
   return llthread->get_tid();
}

int BPatch_thread::getLWPInt()
{
   return llthread->get_lwp()->get_lwp_id();
}

BPatch_function *BPatch_thread::getInitialFuncInt()
{
   int_function *ifunc = llthread->get_start_func();
   if (!ifunc)
      return NULL;
   return proc->func_map->get(ifunc);
}

unsigned long BPatch_thread::getStackTopAddrInt()
{
   return llthread->get_stack_addr();
}


/**
 * We can't overload destructors, and this function used to defined in
 * the API to delete the process.  We don't know if any users are still
 * depending on that behavior, so any regular call to this destructor
 * will trigger the 'delete proc'
 *    
 * To get the "overloaded" and new destructor, set legacy_destructor
 * to false before calling.  This should be okay since it's only used
 * by internal code (should only be called through BPatch_process).
 **/    
void BPatch_thread::BPatch_thread_dtor()
{
   for (unsigned i=0; i<proc->threads.size(); i++)
      if (proc->threads[i] == this)
      {
         proc->threads.erase(i);
         break;
      }
   if (legacy_destructor)
   {
      delete proc;
   }
   else
   {
      proc->llproc->deleteThread(getTid());
   }
}

/**
 * Paradynd sometimes wants handles to the OS threads for reading timing information.
 * Not sure if this should become a part of the public, supported interface.
 **/
unsigned long BPatch_thread::os_handleInt()
{
#if !defined(os_windows)
    // Don't need this any more; we only needed the /proc/<pid>/usage
    // fd on Solaris, and that's opened by the daemon. Windows... I don't
    // understand enough about the system to say.
    assert(0);
#else
    return (unsigned long) llthread->get_lwp()->get_fd();
#endif
}
