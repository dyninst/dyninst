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

#define BPATCH_FILE

#include "BPatch_thread.h"
#include "BPatch.h"
#include "BPatch_libInfo.h"
#include "BPatch_function.h"
#include "BPatch_libInfo.h"
#include "function.h"
#include "BPatch_statement.h"

#include "dynThread.h"
#include "dynProcess.h"
#include "debug.h"

/*
 * BPatch_thread::getCallStack
 *
 * Returns information about the frames currently on the thread's stack.
 *
 * stack	The vector to fill with the stack trace information.
 */
bool BPatch_thread::getCallStack(BPatch_Vector<BPatch_frame>& stack)
{
   std::vector<Frame> stackWalk;   

   if (!llthread->walkStack(stackWalk) ) {
     proccontrol_printf("%s[%d]: failed to perform stackwalk on thread %d\n",
             FILE__, __LINE__, llthread->getLWP());
     return false;
   }

   for (unsigned int i = 0; i < stackWalk.size(); i++) {
      bool isSignalFrame = false;
      bool isInstrumentation = false;
      BPatch_point *point = NULL;

      Frame frame = stackWalk[i];
      instPoint *iP = NULL;

      isSignalFrame = frame.isSignalFrame();
      isInstrumentation = frame.isInstrumentation();

      if (isInstrumentation)
      {
        if (NULL != (iP = frame.getPoint()))
        {
           point = proc->findOrCreateBPPoint(NULL, iP, BPatch_point::convertInstPointType_t(iP->type()));
        }

        if (!point)
        {
          isInstrumentation = false; 
        }
      }


      stack.push_back(BPatch_frame(this,
                                   (void *)frame.getPC(),
                                   (void *)frame.getFP(),
                                   isSignalFrame,
                                   isInstrumentation,
                                   point));
   }
   return true;
}

BPatch_thread *BPatch_thread::createNewThread(BPatch_process *proc, PCThread *thr)
{
   BPatch_thread *newthr;
   newthr = new BPatch_thread(proc, thr);
   return newthr;
}

BPatch_thread::BPatch_thread(BPatch_process *parent, PCThread *thr) : madeExitCallback_(false) {
   proc = parent;
   llthread = thr;
}

unsigned BPatch_thread::getBPatchID()
{
    return (unsigned)llthread->getIndex();
}

BPatch_process *BPatch_thread::getProcess() 
{
   return proc;
}

dynthread_t BPatch_thread::getTid()
{
   return llthread->getTid();
}

Dyninst::LWP BPatch_thread::getLWP()
{
   return llthread->getLWP();
}

BPatch_function *BPatch_thread::getInitialFunc() {
   func_instance *ifunc = llthread->getStartFunc();
   if (!ifunc) return NULL;
   return proc->findOrCreateBPFunc(ifunc, NULL);
}

unsigned long BPatch_thread::getStackTopAddr() {
   return llthread->getStackAddr();
}

BPatch_thread::~BPatch_thread()
{
    if( llthread ) {
        delete llthread;
        llthread = NULL;
    }
}

/**
 * Paradynd sometimes wants handles to the OS threads for reading timing 
 * information.  Not sure if this should become a part of the public, 
 * supported interface.
 **/
unsigned long BPatch_thread::os_handle()
{
    return (unsigned long)-1;
}

/*
 * BPatch_thread::oneTimeCode
 *
 * Have the mutatee execute specified code expr once.  Wait until done.
 *
 */
void *BPatch_thread::oneTimeCode(const BPatch_snippet &expr, bool *err) {
    if( !llthread->isLive() ) {
        if ( err ) *err = true;
        return NULL;
    }

    if( !proc->isStopped() ) {
        BPatch_reportError(BPatchWarning, 0,
                           "oneTimeCode failing because process is not stopped");
        if( err ) *err = true;
        return NULL;
    }

    return proc->oneTimeCodeInternal(expr, this, NULL, NULL, true, err, true);
}

/*
 * BPatch_thread::oneTimeCodeAsync
 *
 * Have the mutatee execute specified code expr once.  Don't wait until done.
 *
 */
bool BPatch_thread::oneTimeCodeAsync(const BPatch_snippet &expr, 
                                        void *userData,
                                        BPatchOneTimeCodeCallback cb)
{
   if ( !llthread->isLive() ) return false;
   if (proc->statusIsTerminated()) return false;

   bool err = false;
   proc->oneTimeCodeInternal(expr, this, userData, cb, false, &err, true);

   if( err ) return false;
   return true;
}

bool BPatch_thread::isDeadOnArrival() 
{
   return false;
}

void BPatch_thread::updateThread(PCThread *newThr) {
    if( llthread ) delete llthread;

    llthread = newThr;
}
