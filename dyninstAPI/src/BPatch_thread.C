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

#define BPATCH_FILE

#include "BPatch_thread.h"
#include "BPatch.h"
#include "BPatch_libInfo.h"
#include "BPatch_function.h"
#include "process.h"
#include "signalgenerator.h"
#include "mailbox.h"
#include "dyn_thread.h"
#include "dyn_lwp.h"
#include "BPatch_libInfo.h"
#include "function.h"
#include "BPatch_statement.h"

#if defined(IBM_BPATCH_COMPAT)
#include <algorithm>
#endif

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

   if (!llthread->walkStack(stackWalk) ) {
     fprintf(stderr, "%s[%d]: ERROR doing stackwalk\n", FILE__, __LINE__);
   }

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
      if (frame.frameType_ != Frame::unset) {
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
	if (iP) {
	  point = proc->findOrCreateBPPoint(NULL, iP);
	}
	if (point) {
	  stack.push_back(BPatch_frame(this,
				       (void*)stackWalk[i].getPC(),
				       (void*)stackWalk[i].getFP(),
				       false,
				       true,
				       point));
	  
	  // And the "top-level function" one.
	  Address origPC = frame.getUninstAddr();
	  stack.push_back(BPatch_frame(this,
				       (void *)origPC,
				       (void *)stackWalk[i].getFP(),
				       false, // not signal handler,
				       false, // not inst.
				       NULL, // No point
				       true)); // Synthesized frame
	}
	else {
	  // No point = internal instrumentation, make it go away.
	  Address origPC = frame.getUninstAddr();
	  stack.push_back(BPatch_frame(this,
				       (void *)origPC,
				       (void *)stackWalk[i].getFP(),
				       false, // not signal handler,
				       false, // not inst.
				       NULL, // No point
				       false)); // Synthesized frame
	  
	}
      }
      else {
         // Not instrumentation, normal case
         Address origPC = frame.getPC();
         stack.push_back(BPatch_frame(this,
                                      (void *)origPC,
                                      (void *)frame.getFP(),
                                      isSignalFrame));
      }
   }
   return true;
}

BPatch_thread *BPatch_thread::createNewThread(BPatch_process *proc, 
                                              int ind, int lwp_id, dynthread_t async_tid)
{
   BPatch_thread *newthr;
   newthr = new BPatch_thread(proc, ind, lwp_id, async_tid);
   return newthr;
}

BPatch_thread::BPatch_thread(BPatch_process *parent, int ind, int lwp_id, dynthread_t async_tid) :
    deleted_callback_made(false)
{
   proc = parent;
   legacy_destructor = true;
   updated = false;
   reported_to_user = false;
   index = (unsigned) -1; // is this safe ??  might want index = -1??
   is_deleted = false;
   doa_tid = (dynthread_t) -1;
   dyn_lwp *lwp = NULL;

   llthread = proc->llproc->getThread(async_tid);
#if defined(os_windows)
  //On Windows the initial LWP has two possible handles, one associated
  // with the thread and one associated with the process.  We use the
  // process handle in Dyninst, so we don't want to use the thread-based
  // lwp_id that got passed in.
  if (!ind)
      lwp = proc->llproc->getInitialLwp();
#endif
   if (llthread && llthread->get_lwp())
      lwp = llthread->get_lwp();
   else if (!lwp)
      lwp = proc->llproc->getLWP(lwp_id);

   if (lwp == NULL) {
     doa = true;
   } 
   else if (lwp->is_dead()) {
     doa = true;
   }
   else doa = false;

   doa_tid = async_tid;
   if (doa) {
      is_deleted = true;
      llthread = NULL;
      return;
   }

   if (!llthread)
       llthread = new dyn_thread(proc->llproc, ind, lwp);

   if (llthread->get_index() == -1 && ind != -1)
       proc->llproc->updateThreadIndex(llthread, ind);

   index = llthread->get_index();

}

BPatch_thread::BPatch_thread(BPatch_process *parent, dyn_thread *dthr) :
    deleted_callback_made(false)
{
   doa = false;
   doa_tid = (dynthread_t) -1;
   is_deleted = false;
   index = 0;
   proc = parent;
   llthread = dthr;
   legacy_destructor = true;
   updated = false;
   reported_to_user = false;
}

void BPatch_thread::updateValues(dynthread_t tid, unsigned long stack_start,
                                 BPatch_function *initial_func, int lwp_id)
{
   dyn_lwp *lwp = NULL;
   if (updated) {
     //fprintf(stderr, "%s[%d]:  thread already updated\n", FILE__, __LINE__);
     return;
   }

#if defined(os_windows)
  //On Windows the initial LWP has two possible handles, one associated
  // with the thread and one associated with the process.  We use the
  // process handle in Dyninst, so we don't want to use the thread-based
  // lwp_id that got passed in.
  if (!index)
      lwp = proc->llproc->getInitialLwp();
#endif
#if !defined(cap_proc_fd)
   if (llthread && llthread->get_lwp())
       lwp = llthread->get_lwp();
#endif
   // For solaris-style /proc we _always_ use the process-grabbed
   // LWP. Thread 1 is created with the representative LWP initially,
   // and then needs to be updated.
   if (!lwp)
       lwp = proc->llproc->getLWP(lwp_id);

   updated = true;
   if (stack_start && !llthread->get_stack_addr())
       llthread->update_stack_addr(stack_start);
   if (lwp && 
       ((llthread->get_lwp() == NULL) ||
        (llthread->get_lwp() == proc->llproc->getRepresentativeLWP())))
       llthread->update_lwp(lwp);

   if (!llthread->get_tid()) {
       if (tid == -1) {
           //Expensive... uses an iRPC to get it from the RT library
           tid = proc->llproc->mapIndexToTid(index);   
       }
       llthread->update_tid(tid);
   }
   
   //If initial_func or stack_start aren't provided then
   // we can update them with a stack walk
   // We delay this to speed initialization -- the main thread
   // never comes in with info, which triggers a.out parsing.

   if (initial_func && !llthread->get_start_func())
   {
      llthread->update_start_func(initial_func->func);
   }
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
   if (doa || is_deleted) {
      return doa_tid/*(dynthread_t) -1*/;
   }
   return llthread->get_tid();
}

int BPatch_thread::getLWPInt()
{
   if (doa || is_deleted) {
      return (int) -1;
   }
   return llthread->get_lwp()->get_lwp_id();
}

BPatch_function *BPatch_thread::getInitialFuncInt()
{
   if (doa || is_deleted) {
      return NULL;
   }
   int_function *ifunc = llthread->get_start_func();

   if (!ifunc && llthread->get_indirect_start_addr())
   {
      //Currently should only be true on IA-64
      process *llproc = getProcess()->llproc;
      Address func_struct = llthread->get_indirect_start_addr();
      Address functionEntry = 0;
      bool readDataSpace = llproc->readDataSpace((void *) func_struct, 
                                                 sizeof(Address), 
                                                 &functionEntry, false);
      if( readDataSpace ) {
         ifunc = llproc->findFuncByAddr(functionEntry);
         llthread->update_start_func(ifunc);
      }
   }

   if (!ifunc) {
       BPatch_Vector<BPatch_frame> stackWalk;

       getCallStackInt(stackWalk);

       unsigned long stack_start = 0;
       BPatch_function *initial_func = NULL;
       
       int pos = stackWalk.size() - 1;

#if defined(DEBUG)
       for (unsigned foo = 0; foo < stackWalk.size(); foo++) {
	 BPatch_function *func = stackWalk[foo].findFunction();
	 fprintf(stderr, "Function at %d is %s\n", foo, func ? func->lowlevel_func()->symTabName().c_str() : "<NULL>");
       }
#endif

       //Consider stack_start as starting at the first
       //function with a stack frame.
       while ((!stack_start || !initial_func) && (pos >= 0)) {
           if (!stack_start) {
               stack_start = (unsigned long) stackWalk[pos].getFP();
           }
           if (!initial_func) {
               BPatch_function *func = stackWalk[pos].findFunction();
               BPatch_module *mod = func ? func->getModule() : NULL;
               if (mod && !mod->isSystemLib())
                   initial_func = func;	       
           }
           pos--;
       }
       
#if defined(os_linux)
       // RH9 once again does it half-right.  The "initial function" by our
       // heuristics is start_thread, but in reality is actually the called
       // function of this frame.
       
       if (initial_func && pos >= 0) {
          // Check if function is in libpthread
          char mname[2048], fname[2048], pfname[2048];
          initial_func->getModule()->getName(mname, 2048);
          initial_func->getName(fname, 2048);
          if (strstr(mname, "libpthread.so")) {
             initial_func = stackWalk[pos].findFunction();
             stack_start = (unsigned long) stackWalk[pos].getFP();
          } else if (!strcmp(fname,"start_thread") &&
                     pos < (int) stackWalk.size() - 2 &&
                     stackWalk[pos+2].findFunction() &&
                     !strcmp(stackWalk[pos+2].findFunction()->getName(pfname, 2048), "clone")) {
             initial_func = stackWalk[pos].findFunction();
             stack_start = (unsigned long) stackWalk[pos].getFP();
          }
       }
#endif

       if (!llthread->get_stack_addr())
           llthread->update_stack_addr(stack_start);
       if (initial_func)
           llthread->update_start_func(initial_func->func);
   }

   // Try again...
   ifunc = llthread->get_start_func();
   ifunc = llthread->map_initial_func(ifunc);

   if (!ifunc) return NULL;

   return proc->findOrCreateBPFunc(ifunc, NULL);
}

unsigned long BPatch_thread::getStackTopAddrInt()
{
   if (doa || is_deleted) {
      return (unsigned long) -1;
   }
   if (llthread->get_stack_addr() == 0) {
       BPatch_Vector<BPatch_frame> stackWalk;

       getCallStackInt(stackWalk);
       unsigned long stack_start = 0;
       BPatch_function *initial_func = NULL;
       
       int pos = stackWalk.size() - 1;
       //Consider stack_start as starting at the first
       //function with a stack frame.
       while ((!stack_start || !initial_func) && (pos >= 0)) {
           if (!stack_start)
               stack_start = (unsigned long) stackWalk[pos].getFP();
           if (!initial_func) {
               initial_func = stackWalk[pos].findFunction();
               if (initial_func) {
                   char fname[2048];
                   initial_func->getName(fname, 2048);
                   //fprintf(stderr, "%s[%d]:  setting initial func to %s\n", FILE__, __LINE__, fname);
               }
           }
           pos--;
       }
       llthread->update_stack_addr(stack_start);
       if (initial_func)
           llthread->update_start_func(initial_func->func);
   }
   
   return llthread->get_stack_addr();
}


void BPatch_thread::removeThreadFromProc() 
{
#if !defined(USE_DEPRECATED_BPATCH_VECTOR)
   // STL vectors don't have item erase. We use iterators instead...
   proc->threads.erase(std::find(proc->threads.begin(),
                                 proc->threads.end(),
                                 this));
#else
   for (unsigned i=0; i<proc->threads.size(); i++) {
      if (proc->threads[i] == this) {
         proc->threads.erase(i);
         break;
      }
   }
#endif    
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
  if (llthread)
    removeThreadFromProc();
  if (legacy_destructor)
  {
    //  ~BPatch_process obtains a lock and does a wait(), so it will fail an assert 
    //  unless we "creatively adjust" the recursive lock depth here.
    BPatch_process *temp = proc;
    proc = NULL;
    if (temp) delete temp;
  }
  else
  {
    if (llthread) {
      dynthread_t thr  = getTid();
      if (thr == 0) {
	fprintf(stderr, "%s[%d]:  about to deleteThread(0)\n", FILE__, __LINE__);
      }
      proc->llproc->deleteThread(thr);
    }
  }
}

void BPatch_thread::deleteThread(bool cleanup) 
{
   removeThreadFromProc();

   dynthread_t thr = getTid();
   if (thr == 0) {
      signal_printf("%s[%d]:  WARN:  skipping deleteThread(0)\n", FILE__, __LINE__);
   }
   else if (cleanup)
     proc->llproc->deleteThread(thr);
   llthread = NULL;
   is_deleted = true;

   //We're intentionally hanging onto thread objects rather than
   // deleting them, in order to allow the user to still have a handle
   // for deleted threads.  If we change our mind on this, uncomment the
   // following two lines:
   //legacy_destructor = true;
   //delete this;
}

/**
 * Paradynd sometimes wants handles to the OS threads for reading timing 
 * information.  Not sure if this should become a part of the public, 
 * supported interface.
 **/
unsigned long BPatch_thread::os_handleInt()
{
   if (doa || is_deleted) {
      return (unsigned long) -1;
   }
#if !defined(os_windows)
    // Don't need this any more; we only needed the /proc/<pid>/usage
    // fd on Solaris, and that's opened by the daemon. Windows... I don't
    // understand enough about the system to say.
    assert(0);
    return -1UL; // keep compiler happy
#else
    return (unsigned long) llthread->get_lwp()->get_fd();
#endif
}

#if 0
// We use the one in BPatch_process, feeding it thread info...

/*
 * BPatch_thread::oneTimeCodeInternal
 *
 * Causes a snippet expression to be evaluated once in the mutatee at the next
 * available opportunity.  Optionally, Dyninst will call a callback function
 * when the snippet has executed in the mutatee, and can wait until the
 * snippet has executed to return.
 *
 * expr		The snippet to evaluate.
 * userData	This value is given to the callback function along with the
 *		return value for the snippet.  Can be used by the caller to
 *		store per-oneTimeCode information.
 * synchronous	True means wait until the snippet has executed, false means
 *		return immediately.
 */
void *BPatch_thread::oneTimeCodeInternal(const BPatch_snippet &expr,
                                         void *userData,
                                         bool synchronous)
{
   bool needToResume = false;

   signal_printf("%s[%d]: UI top of oneTimeCode (threaded)...\n", FILE__, __LINE__);

   while (proc->llproc->sh->isActivelyProcessing()) {
       signal_printf("%s[%d]:  waiting before doing user stop for process %d\n", FILE__, __LINE__, proc->llproc->getPid());
       proc->llproc->sh->waitForEvent(evtAnyEvent);
   }
      
   signal_printf("%s[%d]: oneTimeCode (threaded), handlers quiet, sync %d, statusIsStopped %d\n",
                 FILE__, __LINE__, synchronous, proc->statusIsStopped());

   if (synchronous && !proc->statusIsStopped()) {
       assert(0); // ...
       
      if (!isStopped()) {
         fprintf(stderr, "%s[%d]:  failed to run oneTimeCodeInternal .. status is %s\n", 
                 FILE__, __LINE__, 
                 proc->llproc ? proc->llproc->getStatusAsString().c_str() : "unavailable");
         return NULL;
      }
      needToResume = true;
   }

   OneTimeCodeInfo *info = new OneTimeCodeInfo(synchronous, userData, index);

   proc->llproc->getRpcMgr()->postRPCtoDo(expr.ast,
                                          false, 
                                          BPatch_process::oneTimeCodeCallbackDispatch,
                                          (void *)info,
                                          false,
                                          llthread, NULL); 
    
   if (synchronous) {
      do {
        proc->llproc->getRpcMgr()->launchRPCs(false);
        getMailbox()->executeCallbacks(FILE__, __LINE__);
        if (info->isCompleted()) break;
        proc->llproc->sh->waitForEvent(evtRPCSignal, proc->llproc, NULL /*lwp*/, statusRPCDone);
        getMailbox()->executeCallbacks(FILE__, __LINE__);
      } while (!info->isCompleted() && !isTerminated());
      
      void *ret = info->getReturnValue();
      delete info;

      if (needToResume) {
         proc->continueExecutionInt();
      }
        
      return ret;
   } else {
      proc->llproc->getRpcMgr()->launchRPCs(proc->llproc->status() == running);
      return NULL;
   }
}
#endif

/*
 * BPatch_thread::oneTimeCode
 *
 * Have the mutatee execute specified code expr once.  Wait until done.
 *
 */
void *BPatch_thread::oneTimeCodeInt(const BPatch_snippet &expr, bool *err)
{
  if (is_deleted)
    return NULL;
  return proc->oneTimeCodeInternal(expr, this, NULL, NULL, true, err);
}

/*
 * BPatch_thread::oneTimeCodeAsync
 *
 * Have the mutatee execute specified code expr once.  Don't wait until done.
 *
 */
bool BPatch_thread::oneTimeCodeAsyncInt(const BPatch_snippet &expr, 
                                        void *userData,
                                        BPatchOneTimeCodeCallback cb)
{
   if (proc->statusIsTerminated()) {
      return false;
   }
   if (is_deleted)
     return false;
   proc->oneTimeCodeInternal(expr, this, userData, cb, false, NULL);
   return true;   
}

bool BPatch_thread::isDeadOnArrivalInt() 
{
   return doa;
}

/* This function should be deprecated. */
bool BPatch_thread::getLineAndFile( unsigned long addr, 
                                    unsigned short & lineNo, 
                                    char * fileName, 
                                    int length ) 
{
   BPatch_Vector< BPatch_statement > lines;
   if ( ! getSourceLines( addr, lines ) ) { return false; }
	
   if ( lines.size() > 0 ) {
      lineNo = (unsigned short) lines[0].lineNumber();
      strncpy( fileName, lines[0].fileName(), length );
      return true;
   }
		
return false;
} /* end getLineAndFile() */
    
void BPatch_thread::setDynThread(dyn_thread *thr)
{
  llthread = thr;
  updated = false;
}

bool BPatch_thread::isVisiblyStopped()
{
   return proc->isVisiblyStopped;
}

void BPatch_thread::markVisiblyStopped(bool new_state)
{
   proc->isVisiblyStopped = new_state;
}
