#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__
#include "mailbox.h"
#include "rpcMgr.h"
#include "util.h"

inline unsigned eventTypeHash(const eventType &val) 
{
  return (unsigned int) val;
}   

///////////////////////////////////////////////////////////
/////  Callback Manager:  a container for callbacks
///////////////////////////////////////////////////////////

class CallbackManager {

  public:
    CallbackManager() : cbs(eventTypeHash) {}
    ~CallbackManager() {}

    bool registerCallback(eventType evt, CallbackBase *cb);
    bool removeCallbacks(eventType evt, pdvector<CallbackBase *> &_cbs);

    //  CallbackManager::dispenseCallbacksMatching
    //
    //  Makes a copy of any callbacks matching specified event type and
    //  returns them in a vector.  
    bool dispenseCallbacksMatching(eventType evt, pdvector<CallbackBase *> &cbs);

  private:
   dictionary_hash<eventType, pdvector<CallbackBase *> > cbs;
};

CallbackManager *getCBManager();
class SignalHandler;
class SyncCallback : public CallbackBase
{
  public:

  SyncCallback(bool is_synchronous = true, eventLock *l = global_mutex,
               unsigned long target_thread_id_ = TARGET_UI_THREAD,
               CallbackCompletionCallback completion_cb = signalCompletion) :
    CallbackBase(target_thread_id_, completion_cb), synchronous(is_synchronous),
    lock(l), completion_signalled(false), sh(NULL) {}
  SyncCallback(SyncCallback &src) :
    CallbackBase(src.targetThread(), src.getCleanupCallback()),  
    synchronous(src.synchronous), lock(src.lock), completion_signalled(false), sh(NULL) {}
  virtual ~SyncCallback() {}

   void setSynchronous(bool flag = true) {synchronous = flag;}
   static void signalCompletion(CallbackBase *cb); 
   virtual bool execute(void); 
  protected:
   virtual bool execute_real(void) = 0;
   virtual bool waitForCompletion(); 
   bool synchronous;
   eventLock *lock;
   bool completion_signalled;
  private:
   SignalHandler *sh;
};

///////////////////////////////////////////////////////////
/////  Callback classes,  type-safe and know how to call themselves
///////////////////////////////////////////////////////////

class ErrorCallback : public SyncCallback 
{  
  public:
   ErrorCallback(BPatchErrorCallback callback): SyncCallback(false /*synchronous*/),
                                                cb(callback), str(NULL) {}
   ErrorCallback(ErrorCallback &src) : SyncCallback(false),
                                       cb(src.cb), num(-1), str(NULL) {}
   ~ErrorCallback();

   CallbackBase *copy() { return new ErrorCallback(*this);}
   bool execute_real(void); 
   bool operator()(BPatchErrorLevel severity,
                   int number,
                   const char *error_str);  
   BPatchErrorCallback getFunc() {return cb;}
  private:    
   BPatchErrorCallback cb;    
   BPatchErrorLevel sev;    
   int num;    
   char *str;
};

class ForkCallback : public SyncCallback 
{  
  public:
   ForkCallback(BPatchForkCallback callback) : SyncCallback(),
      cb(callback), par(NULL), chld(NULL) {}
   ForkCallback(ForkCallback &src) : SyncCallback(),
      cb(src.cb), par(NULL), chld(NULL) {}
   ~ForkCallback() {}

   CallbackBase *copy() { return new ForkCallback(*this);}
   bool execute_real(void); 
   bool operator()(BPatch_thread *parent,
                   BPatch_thread *child);
   BPatchForkCallback getFunc() {return cb;}
  private:    
   BPatchForkCallback cb;    
   BPatch_thread *par;
   BPatch_thread *chld;
};

class ExecCallback : public SyncCallback 
{  
  public:
   ExecCallback(BPatchExecCallback callback) : SyncCallback(),
      cb(callback), proc(NULL) {}
   ExecCallback(ExecCallback &src) : SyncCallback(),
      cb(src.cb), proc(NULL) {}
   ~ExecCallback() {}

   CallbackBase *copy() { return new ExecCallback(*this);}
   bool execute_real(void); 
   bool operator()(BPatch_thread *process);
   BPatchExecCallback getFunc() {return cb;}
  private:    
   BPatchExecCallback cb;    
   BPatch_thread *proc;
};

class ExitCallback : public SyncCallback 
{  
  public:
   ExitCallback(BPatchExitCallback callback) : SyncCallback(),
      cb(callback), proc(NULL) {}
   ExitCallback(ExitCallback &src) : SyncCallback(),
      cb(src.cb), proc(NULL) {}
   ~ExitCallback() {}

   CallbackBase *copy() { return new ExitCallback(*this);}
   bool execute_real(void); 
   bool operator()(BPatch_thread *process, BPatch_exitType exit_type);
   BPatchExitCallback getFunc() {return cb;}
  private:    
   BPatchExitCallback cb;    
   BPatch_thread *proc;
   BPatch_exitType type;
};

class SignalCallback : public SyncCallback 
{  
  public:
   SignalCallback(BPatchSignalCallback callback) : SyncCallback(),
      cb(callback), proc(NULL) {}
   SignalCallback(SignalCallback &src) : SyncCallback(),
      cb(src.cb), proc(NULL) {}
   ~SignalCallback() {}

   CallbackBase *copy() { return new SignalCallback(*this);}
   bool execute_real(void); 
   bool operator()(BPatch_thread *process, int sigNum);
   BPatchSignalCallback getFunc() {return cb;}
  private:    
   BPatchSignalCallback cb;    
   BPatch_thread *proc;
   int num;
};

class OneTimeCodeCallback : public SyncCallback 
{  
  public:
   OneTimeCodeCallback(BPatchOneTimeCodeCallback callback) : SyncCallback(),
      cb(callback), proc(NULL) {}
   OneTimeCodeCallback(OneTimeCodeCallback &src) : SyncCallback(),
      cb(src.cb), proc(NULL), user_data(NULL), return_value(NULL) {}
   ~OneTimeCodeCallback() {}

   CallbackBase *copy() { return new OneTimeCodeCallback(*this);}
   bool execute_real(void); 
   bool operator()(BPatch_thread *process, void *userData, void *returnValue);
   BPatchOneTimeCodeCallback getFunc() {return cb;}
  private:    
   BPatchOneTimeCodeCallback cb;    
   BPatch_thread *proc;
   void *user_data;
   void *return_value;
};

class DynLibraryCallback : public SyncCallback 
{  
  public:
   DynLibraryCallback(BPatchDynLibraryCallback callback) : SyncCallback(),
      cb(callback), proc(NULL) {}
   DynLibraryCallback(DynLibraryCallback &src) : SyncCallback(),
      cb(src.cb), proc(NULL), mod(NULL) {}
   ~DynLibraryCallback() {}

   CallbackBase *copy() { return new DynLibraryCallback(*this);}
   bool execute_real(void); 
   bool operator()(BPatch_thread *process, BPatch_module *module, bool load);
   BPatchDynLibraryCallback getFunc() {return cb;}
  private:    
   BPatchDynLibraryCallback cb;    
   BPatch_thread *proc;
   BPatch_module *mod;
   bool load_param;
};

class DynamicCallsiteCallback : public SyncCallback 
{  
  public:
   DynamicCallsiteCallback(BPatchDynamicCallSiteCallback c) : SyncCallback(),
      cb(c), pt(NULL), func(NULL) {}
   DynamicCallsiteCallback(DynamicCallsiteCallback &src) : SyncCallback(),
      cb(src.cb), pt(NULL), func(NULL) {}
   ~DynamicCallsiteCallback() {}

   CallbackBase *copy() { return new DynamicCallsiteCallback(*this);}
   bool execute_real(void); 
   bool operator()(BPatch_point *at_point, BPatch_function *called_function);
   BPatchDynamicCallSiteCallback getFunc() {return cb;}
  private:    
   BPatchDynamicCallSiteCallback cb;    
   BPatch_point *pt;
   BPatch_function *func;
};

class UserEventCallback : public SyncCallback 
{  
  public:
   UserEventCallback(BPatchUserEventCallback callback) : SyncCallback(),
      cb(callback), proc(NULL), buf(NULL) {}
   UserEventCallback(UserEventCallback &src) : SyncCallback(),
      cb(src.cb), proc(NULL), buf(NULL), bufsize(-1) {}
   ~UserEventCallback();

   CallbackBase *copy() { return new UserEventCallback(*this);}
   bool execute_real(void); 
   bool operator()(BPatch_process *process, void *buffer, int buffersize);
   BPatchUserEventCallback getFunc() {return cb;}
  private:    
   BPatchUserEventCallback cb;    
   BPatch_process *proc;
   void *buf;
   int bufsize;
};

class AsyncThreadEventCallback : public SyncCallback 
{  
  public:
   AsyncThreadEventCallback(BPatchAsyncThreadEventCallback callback) : 
      SyncCallback(), 
      cb(callback), proc(NULL), thr(NULL) {}
   AsyncThreadEventCallback(AsyncThreadEventCallback &src) : SyncCallback(),
      cb(src.cb), proc(NULL), thr(NULL) {}
   ~AsyncThreadEventCallback() {}

   CallbackBase *copy() { return new AsyncThreadEventCallback(*this);}
   bool execute_real(void); 
   bool operator()(BPatch_process *process, BPatch_thread *thread);
   BPatchAsyncThreadEventCallback getFunc() {return cb;}
  private:    
   BPatchAsyncThreadEventCallback cb;    
   BPatch_process *proc;
   BPatch_thread *thr;
};

typedef void (*internalThreadExitCallback)(BPatch_process *, BPatch_thread *,
                                           pdvector<AsyncThreadEventCallback *> *);

class InternalThreadExitCallback : public SyncCallback 
{  
  public:
   InternalThreadExitCallback(internalThreadExitCallback callback) : 
      SyncCallback(), 
      cb(callback), proc(NULL) {}
   InternalThreadExitCallback(InternalThreadExitCallback &src) : 
      SyncCallback(), 
      cb(src.cb), proc(NULL), thr(NULL), cbs(NULL) {}
   ~InternalThreadExitCallback();

   CallbackBase *copy() { return new InternalThreadExitCallback(*this);}
   bool execute_real(void); 
   bool operator()(BPatch_process *p, BPatch_thread *t, 
                   pdvector<AsyncThreadEventCallback *> *callbacks);
  private:    
   internalThreadExitCallback cb;
   BPatch_process *proc;
   BPatch_thread *thr;
   pdvector<AsyncThreadEventCallback *> *cbs;
};

#ifdef IBM_BPATCH_COMPAT
class ThreadEventCallback : public SyncCallback 
{  
  public:
   ThreadEventCallback(BPatchThreadEventCallback callback) : SyncCallback(),
      cb(callback), thr(NULL) {}
   ThreadEventCallback(ThreadEventCallback &src) : SyncCallback(),
      cb(src.cb), thr(NULL), a1(NULL), a2(NULL) {}
   ~ThreadEventCallback();

   CallbackBase *copy() { return new ThreadEventCallback(*this);}
   bool execute_real(void); 
   bool operator()(BPatch_thread *thread, void *arg1, void *arg2);
   BPatchThreadEventCallback getFunc() {return cb;}
  private:    
   BPatchAsyncThreadEventCallback cb;    
   BPatch_thread *thr;
   void *a1, *a2;
};
#endif

#endif
