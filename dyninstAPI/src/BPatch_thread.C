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
#include "BPatch_libInfo.h"
#include "function.h"
#include "BPatch_statement.h"

#include "pcThread.h"
#include "pcProcess.h"
#include "debug.h"

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
            bblInstance *bbi = proc->lowlevel_process()->
                findOrigByAddr(origPC)->is_basicBlockInstance();
            if (bbi && 0 != bbi->version()) {
                origPC = bbi->equivAddr(0, origPC);
            }
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
            bblInstance *bbi = proc->lowlevel_process()->
                findOrigByAddr(origPC)->is_basicBlockInstance();
            if (bbi && 0 != bbi->version()) {
                origPC = bbi->equivAddr(0, origPC);
            }
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
         bblInstance *bbi = proc->lowlevel_process()->
             findOrigByAddr(frame.getPC())->is_basicBlockInstance();
         if (bbi && 0 != bbi->version()) {
             origPC = bbi->equivAddr(0, origPC);
         }
         stack.push_back(BPatch_frame(this,
                                      (void *)origPC,
                                      (void *)frame.getFP(),
                                      isSignalFrame));
      }
   }
   return true;
}

BPatch_thread *BPatch_thread::createNewThread(BPatch_process *proc, PCThread *thr)
{
   BPatch_thread *newthr;
   newthr = new BPatch_thread(proc, thr);
   return newthr;
}

BPatch_thread::BPatch_thread(BPatch_process *parent, PCThread *thr) :
    deleted_callback_made(false)
{
   proc = parent;
   legacy_destructor = true;
   is_deleted = false;
   deleted_tid = (dynthread_t) -1;
   llthread = thr;
}

unsigned BPatch_thread::getBPatchIDInt()
{
    return (unsigned)llthread->getIndex();
}

BPatch_process *BPatch_thread::getProcessInt() 
{
   return proc;
}

dynthread_t BPatch_thread::getTidInt()
{
   if (is_deleted) {
      return deleted_tid;
   }
   return llthread->getTid();
}

int BPatch_thread::getLWPInt()
{
   if (is_deleted) {
      return (int) -1;
   }
   return llthread->getLWP();
}

BPatch_function *BPatch_thread::getInitialFuncInt()
{
   if (is_deleted) {
      return NULL;
   }
   int_function *ifunc = llthread->getStartFunc();

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

       if (!llthread->getStackAddr())
           llthread->updateStackAddr(stack_start);
       if (initial_func)
           llthread->updateStartFunc(initial_func->func);
   }

   // Try again...
   ifunc = llthread->getStartFunc();
   ifunc = llthread->mapInitialFunc(ifunc);

   if (!ifunc) return NULL;

   return proc->findOrCreateBPFunc(ifunc, NULL);
}

unsigned long BPatch_thread::getStackTopAddrInt()
{
   if (is_deleted) {
      return (unsigned long) -1;
   }
   if (llthread->getStackAddr() == 0) {
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
       llthread->updateStackAddr(stack_start);
       if (initial_func)
           llthread->updateStartFunc(initial_func->func);
   }
   
   return llthread->getStackAddr();
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

   proc->llproc->deleteThread(llthread->getTid());

   is_deleted = true;
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
      delete llthread;
      llthread = NULL;
    }
  }
}

/**
 * Paradynd sometimes wants handles to the OS threads for reading timing 
 * information.  Not sure if this should become a part of the public, 
 * supported interface.
 **/
unsigned long BPatch_thread::os_handleInt()
{
    return (unsigned long)-1;
}

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
   return true; // XXX what if oneTimeCodeInternal fails
}

bool BPatch_thread::isDeadOnArrivalInt() 
{
   return false;
}
