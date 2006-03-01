#include <stdio.h>
#include <stdlib.h>

#include "dyninstAPI/src/callbacks.h"
#include "dyninstAPI/src/mailbox.h"
#include "dyninstAPI/src/EventHandler.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_thread.h"

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
    if (sh)
      sh->wait_cb = (CallbackBase *) this;

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
  cb(sev, num, (const char **)&str);
  return true;
}

bool ErrorCallback::operator()(BPatchErrorLevel severity, int number,
                          const char *error_string)
{
  //  Assume that we are already locked upon entry
  assert(lock->depth());

  str = strdup(error_string);
  num = number;
  sev = severity;    

  return do_it();
}

bool ForkCallback::execute_real(void) 
{
  cb(par, chld);
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
  cb(thread);
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
  cb(thread, mod, load_param);
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

bool AsyncThreadEventCallback::execute_real(void) 
{
  async_printf("%s[%d][%s]:  welcome to AsyncThreadEventCallback: execute\n", 
          FILE__, __LINE__, getThreadStr(getExecThreadID()));
  cb(proc, thr);

  //After executing a thread delete callback, destroy the thread
  if (thr->llthread && thr->llthread->is_exited() && !thr->is_deleted)
     proc->deleteBPThread(thr);

  return true;
}

bool AsyncThreadEventCallback::operator()(BPatch_process *process, BPatch_thread *thread)
{
  assert(lock->depth());
  assert(process);
  assert(thread);

  proc = process;
  thr = thread;

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

#ifdef IBM_BPATCH_COMPAT
bool DPCLProcessEventCallback::execute_real(void) 
{
  cb(proc, a1, a2);
  return true;
}

bool DPCLProcessEventCallback::operator()(BPatch_process *process, void *arg1, void *arg2)
{
  assert(lock->depth());
  proc = process;
  a1 = arg1;
  a2 = arg2;

  return do_it();
}

DPCLProcessEventCallback::~DPCLProcessEventCallback() {}


bool DPCLThreadEventCallback::execute_real(void) 
{
  cb(thr, a1, a2);
  return true;
}

bool DPCLThreadEventCallback::operator()(BPatch_thread *thread, void *arg1, void *arg2)
{
  assert(lock->depth());
  thr = thread;
  a1 = arg1;
  a2 = arg2;

  return do_it();
}

DPCLThreadEventCallback::~DPCLThreadEventCallback() {}

#endif
