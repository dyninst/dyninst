/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
#include <stdio.h>
#include <stdlib.h>

#include "dyninstAPI/src/callbacks.h"
#include "dyninstAPI/src/mailbox.h"
#include "dyninstAPI/src/EventHandler.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/signalgenerator.h"
#include "dyninstAPI/h/BPatch_thread.h"
#include "dyninstAPI/h/BPatch_function.h"
#include "dyninstAPI/h/BPatch_point.h"

CallbackManager *callback_manager = NULL;
CallbackManager *getCBManager()
{
   if (callback_manager == NULL)
      callback_manager = new CallbackManager();

   return callback_manager;
}

bool CallbackManager::registerCallback(eventType evt, CallbackBase *cb)
{
   pdvector<CallbackBase *> &cbs_for_type = cbs[evt];
   cbs_for_type.push_back(cb);

   return true;
}

bool CallbackManager::removeCallbacks(eventType evt, pdvector<CallbackBase *> &cbs_out)
{
   if (cbs.defines(evt)) {
      pdvector<CallbackBase *> cbs_for_type = cbs.get_and_remove(evt);
      for (unsigned int i = 0; i < cbs_for_type.size(); ++i) {
         cbs_out.push_back(cbs_for_type[i]); 
      }
      assert(!cbs.defines(evt));
   }
   else  {
      mailbox_printf("%s[%d]:  no callbacks matching %s\n", 
            FILE__, __LINE__, eventType2str(evt));
      return false;
   }

   return true;
}

bool CallbackManager::dispenseCallbacksMatching(eventType evt, pdvector<CallbackBase *> &cbs_out)
{
   if (cbs_out.size()) {
      fprintf(stderr, "%s[%d]:  WARN, dispenseCallbacksMatching (%s) appending to existing callbacks\n", FILE__, __LINE__, eventType2str(evt));
   }

   if (cbs.defines(evt)) {
      pdvector<CallbackBase *> &cbs_for_type = cbs[evt];
      for (unsigned int i = 0; i < cbs_for_type.size(); ++i) {
         CallbackBase *newcb = cbs_for_type[i]->copy(); 
         cbs_out.push_back(newcb); 
      }
   }
   else  {
      mailbox_printf("%s[%d]:  no callbacks matching %s\n", 
            FILE__, __LINE__, eventType2str(evt));
      return false;
   }
   return true;

}

void SyncCallback::signalCompletion(CallbackBase *cb) 
{
   //fprintf(stderr, "%s[%d]:  welcome to singal completion\n", FILE__, __LINE__);
   SyncCallback *scb = (SyncCallback *) cb;
   scb->lock->_Lock(FILE__, __LINE__);
   scb->completion_signalled = true;
   if (scb->sh)
      scb->sh->wait_cb = NULL;
   signal_printf("%s[%d]: SyncCallback, signalling completion, sh = %s\n", FILE__, __LINE__, scb->sh ? scb->sh->getName() : "null");
   mailbox_printf("%s[%d][%s]:  signalling completion of callback\n",
         FILE__, __LINE__, getThreadStr(getExecThreadID()));
   scb->lock->_Broadcast(FILE__, __LINE__);
   scb->lock->_Unlock(FILE__, __LINE__);
}

bool SyncCallback::waitForCompletion() 
{
   //  Assume that we are already locked upon entry
   assert(lock);
   assert(lock->depth());
   assert(lock == global_mutex);
   //  we need to find the signal
   //  handler that has this thread id -- ie, find out if we are running on a 
   //  signal handler thread.  Since we do not have an easy way of getting 
   //  this object, we need to search for it:

   extern pdvector<process *> processVec;
   for (unsigned int i = 0; i < processVec.size(); ++i) {
      if (processVec[i]) {
         if (processVec[i]->status() != deleted && processVec[i]->sh)
            if (NULL != (sh = processVec[i]->sh->findSHWithThreadID(getExecThreadID())))
               break;
      }
   }
   signal_printf("%s[%d]: SyncCallback, waiting for completion, sh = %p\n", FILE__, __LINE__, sh ? sh->getName() : "null");
   if (sh) {
      sh->wait_cb = (CallbackBase *) this;
      sh->sg->pingIfContinueBlocked();
   }

   while (!completion_signalled) {
      if (!lock) {
         fprintf(stderr, "%s[%d]:  LOCK IS GONE!!\n", FILE__, __LINE__);
         return false;
      }
      mailbox_printf("%s[%d][%s]:  waiting for completion of callback\n",
            FILE__, __LINE__, getThreadStr(getExecThreadID()));
      if (0 != lock->_Broadcast(FILE__, __LINE__)) assert(0);
      if (0 != lock->_WaitForSignal(FILE__, __LINE__)) assert(0);
   }
   return true;
}

bool SyncCallback::execute() 
{
   unsigned int need_to_relock = 0;
   lock->_Lock(FILE__, __LINE__);
   while (lock->depth()-1) {
      lock->_Unlock(FILE__, __LINE__);
      need_to_relock++;
   }
   lock->_Unlock(FILE__, __LINE__);

   execute_real();

   while (need_to_relock--) 
      lock->_Lock(FILE__, __LINE__);
   return true;
}

bool SyncCallback::do_it()
{
   bool reset_delete_enabled = false;
   if (synchronous) {
      if (deleteEnabled()) {
         enableDelete(false);
         reset_delete_enabled = true;
      }
   }

   getMailbox()->executeOrRegisterCallback(this);
   if (synchronous) {
      signal_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
      waitForCompletion();
      if (reset_delete_enabled)
         enableDelete();
   }

   return true;
}

ErrorCallback::~ErrorCallback()
{
   //  need to free memory allocated for the arguments 
   if (str) free(str);
}

bool ErrorCallback::execute_real(void) 
{
   cb(sev, num, &str);
   return true;
}

bool ErrorCallback::operator()(BPatchErrorLevel severity, int number,
      const char *error_string)
{
   //  Assume that we are already locked upon entry
   assert(lock->depth());
   assert(error_string);
   str = P_strdup(error_string);
   num = number;
   sev = severity;    

   return do_it();
}

bool ForkCallback::execute_real(void) 
{
   bool is_stopped = par->isVisiblyStopped();
   if (!is_stopped)
   par->markVisiblyStopped(true);
   cb(par, chld);
   if (!is_stopped)
   par->markVisiblyStopped(false);
   return true;
}

bool ForkCallback::operator()(BPatch_thread *parent, BPatch_thread *child)
{
   assert(lock->depth());
   par = parent;
   chld = child;

   return do_it();
}

bool ExecCallback::execute_real(void) 
{
   bool is_stopped = thread->isVisiblyStopped();
   if (!is_stopped)
   thread->markVisiblyStopped(true);
   cb(thread);
   if (!is_stopped)
   thread->markVisiblyStopped(false);
   return true;
}

bool ExecCallback::operator()(BPatch_thread *thr)
{
   assert(lock->depth());
   thread = thr;

   return do_it();
}

bool ExitCallback::execute_real(void) 
{
   bool is_stopped = thread->isVisiblyStopped();
   if (!is_stopped)
   thread->markVisiblyStopped(true);
   cb(thread, type);
   return true;
}

bool ExitCallback::operator()(BPatch_thread *thr, BPatch_exitType exit_type)
{
   assert(lock->depth());
   thread = thr;;
   type = exit_type;

   return do_it();
}

bool SignalCallback::execute_real(void) 
{
   cb(thread, num);
   return true;
}

bool SignalCallback::operator()(BPatch_thread *thr, int sigNum)
{
   assert(lock->depth());
   thread = thr;
   num = sigNum;

   return do_it();
}

bool OneTimeCodeCallback::execute_real(void) 
{
   cb(thread, user_data, return_value);
   return true;
}

bool OneTimeCodeCallback::operator()(BPatch_thread *thr, void *userData, void *returnValue)
{
   assert(lock->depth());
   thread = thr;
   user_data = userData;
   return_value = returnValue;

   return do_it();
}

bool DynLibraryCallback::execute_real(void) 
{
   bool is_stopped = thread->isVisiblyStopped();
   if (!is_stopped)
   thread->markVisiblyStopped(true);
   cb(thread, mod, load_param);
   if (!is_stopped)
   thread->markVisiblyStopped(false);
   return true;
}

bool DynLibraryCallback::operator()(BPatch_thread *thr, BPatch_module *module, bool load)
{
   assert(lock->depth());
   thread = thr;
   mod = module;
   load_param = load;

   return do_it();
}

bool DynamicCallsiteCallback::execute_real(void) 
{
   cb(pt, func);
   return true;
}

bool DynamicCallsiteCallback::operator()(BPatch_point *at_point, 
                                         BPatch_function *called_function)
{
   assert(lock->depth());
   pt = at_point;
   func = called_function;

   return do_it();
}

UserEventCallback::~UserEventCallback()
{
   if (buf) delete [] (int *)buf;
}

bool UserEventCallback::execute_real(void) 
{
   cb(proc, buf, bufsize);
   return true;
}

bool UserEventCallback::operator()(BPatch_process *process, void *buffer, int buffersize)
{
   assert(lock->depth());
   proc = process;
   bufsize = buffersize;
   buf = new int [buffersize];
   memcpy(buf, buffer, buffersize); 

   return do_it();
}


process* SyncCallback::getProcess() const
{
    return NULL;
}
process * StopThreadCallback::getProcess() const
{
    return point->getFunction()->getProc()->lowlevel_process();
}
process * CodeOverwriteCallback::getProcess() const
{
    return proc;
}


StopThreadCallback::StopThreadCallback(BPatchStopThreadCallback callback) : 
    SyncCallback(),
    cb(callback), 
    point(NULL), 
    return_value(NULL) 
{
    enableDelete(false);
}
StopThreadCallback::StopThreadCallback(StopThreadCallback &src) : 
    SyncCallback(),
    cb(src.cb), 
    point(NULL), 
    return_value(NULL) 
{
    enableDelete(false);
}

StopThreadCallback::~StopThreadCallback() 
{
    mal_printf("%s[%d] About to delete stopthread callback.  This is bad\n");
    assert(0);
}

bool StopThreadCallback::execute_real(void) 
{
   cb(point, return_value);
  return true;
}

bool StopThreadCallback::operator()(BPatch_point *atPoint, void *returnValue)
{
  assert(lock->depth());
  point = atPoint;
  return_value = returnValue;

  return do_it();
}

bool SignalHandlerCallback::execute_real(void) 
{
  cb(point, signum, *handlers);
  return true;
}

bool SignalHandlerCallback::handlesSignal(long signum) 
{
  if (NULL == signals) {
    return true;
  } else {
    return signals->contains(signum);
  }
}

bool SignalHandlerCallback::operator()(BPatch_point *at_point, long signal_number, 
                                       BPatch_Vector<Dyninst::Address> *handler_vec)
{
  assert(lock->depth());
  point = at_point;
  signum = signal_number;
  handlers = handler_vec;
  return do_it();
}

bool CodeOverwriteCallback::execute_real(void) 
{
  cb_(fault_instr, v_target);
  return true;
}

bool CodeOverwriteCallback::operator()
(BPatch_point *fault_instr_, Address v_target_, process *proc_)
{
  assert(lock->depth());
  fault_instr = fault_instr_;
  v_target = v_target_;
  proc = proc_;
  return do_it();
}

bool AsyncThreadEventCallback::execute_real(void) 
{
   async_printf("%s[%d][%s]:  welcome to AsyncThreadEventCallback: execute\n", 
         FILE__, __LINE__, getThreadStr(getExecThreadID()));
   cb(proc, thr);

   return true;
}

bool AsyncThreadEventCallback::operator()(BPatch_process *process, BPatch_thread *thread)
{
   assert(lock->depth());
   assert(process);
   assert(thread);

   proc = process;
   thr = thread;
   synchronous = override_to_sync;

   return do_it();
}

InternalThreadExitCallback::~InternalThreadExitCallback()
{
   if (cbs) delete cbs;
}

bool InternalThreadExitCallback::execute_real(void) 
{
   cb(proc, thr, cbs);
   return true;
}

bool InternalThreadExitCallback::operator()(BPatch_process *p, BPatch_thread *t, 
      pdvector<AsyncThreadEventCallback *> *callbacks)
{
   assert(lock->depth());
   proc = p;
   thr = t;
   cbs = callbacks;

   return do_it();
}
