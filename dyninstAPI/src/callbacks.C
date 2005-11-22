#include <stdio.h>
#include <stdlib.h>

#include "callbacks.h"
#include "mailbox.h"
#include "EventHandler.h"
#include "util.h"
#include "showerror.h"

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
     signal_printf("%s[%d]: SyncCallback, signalling completion, sh = %p\n", FILE__, __LINE__, scb->sh);
     mailbox_printf("%s[%d][%s]:  signalling completion of callback\n",
             FILE__, __LINE__, getThreadStr(getExecThreadID()));
     scb->lock->_Broadcast(FILE__, __LINE__);
     scb->lock->_Unlock(FILE__, __LINE__);
}

bool SyncCallback::waitForCompletion() 
{
    //  Assume that we are already locked upon entry
    assert(lock->depth());
    sh = getSH()->findSHWithThreadID(getExecThreadID());
    signal_printf("%s[%d]: SyncCallback, waiting for completion, sh = %p\n", FILE__, __LINE__, sh);
    if (sh)
      sh->wait_cb = (CallbackBase *) this;

    while (!completion_signalled) {
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
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    //signal_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }

  return true;
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
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    signal_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  return true;
}

bool ExecCallback::execute_real(void) 
{
  cb(proc);
  return true;
}

bool ExecCallback::operator()(BPatch_thread *process)
{
  assert(lock->depth());
  proc = process;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    signal_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  return true;
}

bool ExitCallback::execute_real(void) 
{
  cb(proc, type);
  return true;
}

bool ExitCallback::operator()(BPatch_thread *process, BPatch_exitType exit_type)
{
  assert(lock->depth());
  proc = process;
  type = exit_type;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    signal_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  return true;
}

bool SignalCallback::execute_real(void) 
{
  cb(proc, num);
  return true;
}

bool SignalCallback::operator()(BPatch_thread *process, int sigNum)
{
  assert(lock->depth());
  proc = process;
  num = sigNum;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    signal_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  return true;
}

bool OneTimeCodeCallback::execute_real(void) 
{
  cb(proc, user_data, return_value);
  return true;
}

bool OneTimeCodeCallback::operator()(BPatch_thread *process, void *userData, void *returnValue)
{
  assert(lock->depth());
  proc = process;
  user_data = userData;
  return_value = returnValue;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    signal_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  return true;
}

bool DynLibraryCallback::execute_real(void) 
{
  cb(proc, mod, load_param);
  return true;
}

bool DynLibraryCallback::operator()(BPatch_thread *process, BPatch_module *module, bool load)
{
  assert(lock->depth());
  proc = process;
  mod = module;
  load_param = load;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    signal_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  return true;
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
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    signal_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  return true;
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
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    signal_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  return true;
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
  proc = process;
  thr = thread;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    signal_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  return true;
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
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    signal_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  return true;
}

#ifdef IBM_BPATCH_COMPAT
bool ThreadEventCallback::execute_real(void) 
{
  cb(thr, a1, a2);
  return true;
}

bool ThreadEventCallback::operator()(BPatch_thread *thread, void *arg1, void *arg2)
{
  assert(lock->depth());
  thr = thread;
  a1 = arg1;
  a2 = arg2;
  getMailbox()->executeOrRegisterCallback(this);
  if (synchronous) {
    signal_printf("%s[%d]:  waiting for completion of callback\n", FILE__, __LINE__);
    waitForCompletion();
  }
  return true;
}
#endif
