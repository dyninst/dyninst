#ifndef __DEBUGGER_INTERFACE__
#define __DEBUGGER_INTERFACE__

//  DebuggerInterface exists to abstract debugger-like operations
//  in a platform independent way.  The primary motivation for its
//  creation is that, on linux, ptrace will not work in a framework
//  where it is called from different threads.
//
//  For non-ptrace platforms, this class is essentially just a wrapper
//  for existing functions like read/writeDataSpace, continueProcess, etc.
//
//  For ptrace platforms, all ptrace operations must come from the same
//  thread.  So this class creates a thread to handle all ptrace operations.
//  calls to member functions are logged as "debugger events", and executed
//  on the "ptrace dispatcher thread" asap.

// This class maintains its own mutex/cond pair so that it may sleep
// uninterrupted when there is no ptrace activity.

#include "EventHandler.h"
#include "mailbox.h"
#include "callbacks.h"
#include "util.h"
#include "showerror.h"
#include <stdlib.h>
#if !defined (os_windows)
#include <unistd.h>
#include <sys/types.h>
#endif
#if defined  (arch_ia64)
#include <libunwind.h>
#include <libunwind-ptrace.h>
#define PTRACE_RETURN long
#else
#define PTRACE_RETURN long
#endif

#if defined (os_linux)
#define CREATE_DBI_THREAD createThread(); 
#else
#define CREATE_DBI_THREAD 
#endif
extern unsigned long dbi_thread_id;

typedef enum {
  dbiUndefined,
  dbiForkNewProcess,
  dbiPtrace,
  dbiWriteDataSpace,
  dbiReadDataSpace,
  dbiWaitPid,
  dbiGetFrameRegister,
  dbiSetFrameRegister,
  dbiCreateUnwindAddressSpace,
  dbiDestroyUnwindAddressSpace,
  dbiUPTCreate,
  dbiUPTDestroy,
  dbiInitFrame,
  dbiStepFrame,
  dbiIsSignalFrame,
  dbiWaitpid,
  dbiLastEvent /* placeholder for the end of the list */
               /* make sure to reflect additions to this list in */
               /* dbiEventType2Str */
} DBIEventType;

const char *dbiEventType2str(DBIEventType);

class DBIEvent {
  public:
    DBIEvent(DBIEventType t = dbiUndefined) : type(t) {}
    DBIEventType type;
};

class DebuggerInterface;

#if defined (os_linux)
#define DBI_PLATFORM_TARGET_THREAD TARGET_DBI_THREAD
#else
#define DBI_PLATFORM_TARGET_THREAD TARGET_ANY_THREAD
#endif
extern CallbackCompletionCallback dbi_signal_done;

class DBICallbackBase : public SyncCallback
{
  friend class DebuggerInterface;
  //friend void dbi_signal_done_(CallbackBase *cb);

  public:
  DBICallbackBase(DBIEventType t, eventLock *l) : SyncCallback(true /*synchronous*/, l,
                                                  DBI_PLATFORM_TARGET_THREAD, 
                                                  dbi_signalCompletion), type(t) {}
  virtual ~DBICallbackBase() {}
  static void dbi_signalCompletion(CallbackBase *cb);
  virtual bool waitForCompletion();
  virtual bool execute() {return execute_real();}
  virtual bool execute_real() = 0;
  protected:
  DBIEventType type;
};

class DebuggerInterface : public EventHandler<DBIEvent> {
  friend  void dbi_signal_done_(CallbackBase *);
  friend class DBICallbackBase;

  public:
  DebuggerInterface() : EventHandler<DBIEvent>(&dbilock, "DBI", false),   
                        isReady(false),
                        isBusy(false), evt(dbiUndefined) 
  {
    CREATE_DBI_THREAD
    dbi_thread_id = getThreadID();
    dbi_printf("%s[%d][%s]:  created DBI thread, dbi_thread_id = %lu, -1UL = %lu\n", 
            __FILE__, __LINE__, getThreadStr(getExecThreadID()), dbi_thread_id, -1UL);
  }
  virtual ~DebuggerInterface() {}

  bool forkNewProcess(pdstring file, pdstring dir, pdvector<pdstring> *argv, 
                      pdvector<pdstring> *envp,
                      pdstring inputFile, pdstring outputFile, int &traceLink,
                      int &pid, int & /*tid*/, int & /*procHandle*/,
                      int & /*thrHandle*/, int stdin_fd, int stdout_fd, int stderr_fd);

  PTRACE_RETURN ptrace(int req, pid_t pid, Address addr, Address data, 
                       int word_len = -1, int *ptrace_errno = NULL);

  bool writeDataSpace(pid_t pid, Address addr, int nbytes, 
                      Address data, int address_width, 
                      const char *file, unsigned int line);

  bool readDataSpace(pid_t pid, Address addr, int nbytes, 
                     Address data, int address_width, 
                     const char * file, unsigned int line);

  int waitpidNoBlock(int *status);

#if defined (arch_ia64)
  int getFrameRegister(unw_cursor_t *cp, unw_regnum_t reg, unw_word_t *valp);
  int setFrameRegister(unw_cursor_t *cp, unw_regnum_t reg, unw_word_t val);
  void * createUnwindAddressSpace(unw_accessors_t *ap, int byteorder);
  int destroyUnwindAddressSpace(unw_addr_space *ap);
  void *UPTcreate(pid_t pid);
  void UPTdestroy(void *handle);
  int initFrame(unw_cursor_t *cp, unw_addr_space_t as, void *arg);
  int stepFrameUp(unw_cursor_t *cp);
  bool isSignalFrame(unw_cursor_t *cp);
  int waitpid(pid_t pid, int *status, int options);
  public:
#endif
  bool bulkPtraceWrite(void *inTraced, u_int nbytes, void *inSelf, int pid, int address_width);
  bool bulkPtraceRead(void *inTraced, u_int nbytes, void *inSelf, int pid, int address_width);
  private:
  bool isReady;
  bool isBusy;
  virtual bool waitNextEvent(DBIEvent &ev);
  virtual bool handleEvent(DBIEvent &ev)
    { LOCK_FUNCTION(bool, handleEventLocked, (ev));}
  bool handleEventLocked(DBIEvent &ev);

  void getBusy() {
    dbilock._Lock(FILE__, __LINE__);
    while (isBusy) {
      dbi_printf("%s[%d]:  dbi is busy, waiting for signal\n", FILE__, __LINE__);
      dbilock._WaitForSignal(FILE__, __LINE__);
    }
    isBusy = true;
    dbilock._Unlock(FILE__, __LINE__);
  }
  void releaseBusy() {
    dbilock._Lock(FILE__, __LINE__);
    isBusy = false;
    dbilock._Broadcast(FILE__, __LINE__);
    dbilock._Unlock(FILE__, __LINE__);
  }

  DBIEventType evt;
  eventLock dbilock;

};

#define DBI_PRINT(x) dbi_debug_filename = __FILE__; \
                     dbi_debug_lineno = __LINE__; \
                     dbi_printf x

DebuggerInterface *getDBI();
PTRACE_RETURN DBI_ptrace(int req, pid_t pid, Address addr, Address data, int *ptrace_errno = NULL, int word_len = -1, const char *file = NULL, unsigned int line = 0); 
bool DBI_writeDataSpace(pid_t pid, Address addr, int nelem, Address data, int word_len, const char *file = NULL, unsigned int line = 0); 
bool DBI_readDataSpace(pid_t pid, Address addr, int nelem, Address data, int word_len, const char *file = NULL, unsigned int line = 0); 


//  Helper callback classes for use with mailbox system


class ForkNewProcessCallback : public DBICallbackBase
{
  public:
   ForkNewProcessCallback(eventLock *l) : DBICallbackBase(dbiForkNewProcess,l) {}
   ForkNewProcessCallback(ForkNewProcessCallback &src) : DBICallbackBase(dbiForkNewProcess,
                                                                         src.lock) {}
   virtual ~ForkNewProcessCallback() {}

   CallbackBase *copy() { return new ForkNewProcessCallback(*this);}
   bool execute_real(void);
   bool operator()(pdstring file, pdstring dir, pdvector<pdstring> *argv, 
                   pdvector<pdstring> *envp,
                   pdstring inputFile, pdstring outputFile, int &traceLink,
                   int &pid, int & /*tid*/, int & /*procHandle*/,
                   int & /*thrHandle*/, int stdin_fd, int stdout_fd, int stderr_fd);

   bool getReturnValue() {return ret;}

  private:
   bool ret;
   pdstring *file_;
   pdstring *dir_;
   pdvector<pdstring> *argv_;
   pdvector<pdstring> *envp_;
   pdstring *inputFile_;
   pdstring *outputFile_;
   int *traceLink_;
   int *pid_;
   int *tid_;
   int *procHandle_;
   int *thrHandle_;
   int stdin_fd_;
   int stdout_fd_;
   int stderr_fd_;
};

class PtraceCallback : public DBICallbackBase
{
  public:
   PtraceCallback(eventLock *l) : DBICallbackBase(dbiPtrace, l) {}
   PtraceCallback(PtraceCallback &src) : DBICallbackBase(dbiPtrace,src.lock) {}
   virtual ~PtraceCallback() {}

   CallbackBase *copy() { return new PtraceCallback(*this);}
   bool execute_real(void);
   bool operator()(int req, pid_t pid, Address addr, Address data, 
                   int word_len = -1);

   PTRACE_RETURN getReturnValue() {return ret;}
   int getErrno() {return ptrace_errno;}

  private:
   PTRACE_RETURN ret;
   int req_;
   pid_t pid_;
   Address addr_;
   Address data_;
   int word_len_;
   int ptrace_errno;
};

class WriteDataSpaceCallback : public DBICallbackBase
{
  public:
   WriteDataSpaceCallback(eventLock *l) : DBICallbackBase(dbiWriteDataSpace, l) {}
   WriteDataSpaceCallback(WriteDataSpaceCallback &src) : DBICallbackBase(dbiWriteDataSpace, 
                                                                         src.lock) {}
   virtual ~WriteDataSpaceCallback() {}

   CallbackBase *copy() { return new WriteDataSpaceCallback(*this);}
   bool execute_real(void);
   bool operator()(pid_t pid, Address addr, int nelem, 
                   Address data, int word_len);

   bool getReturnValue() {return ret;}

  private:
   bool ret;
   pid_t pid_;
   Address addr_;
   int nelem_;
   Address data_;
   int word_len_;
};

class ReadDataSpaceCallback : public DBICallbackBase
{
  public:
   ReadDataSpaceCallback(eventLock *l) : DBICallbackBase(dbiReadDataSpace, l) {}
   ReadDataSpaceCallback(ReadDataSpaceCallback &src) : DBICallbackBase(dbiReadDataSpace,
                                                                       src.lock) {}
   virtual ~ReadDataSpaceCallback() {}

   CallbackBase *copy() { return new ReadDataSpaceCallback(*this);}
   bool execute_real(void);
   bool operator()(pid_t pid, Address addr, int nelem, 
                   Address data, int word_len);

   bool getReturnValue() {return ret;}

  private:
   bool ret;
   pid_t pid_;
   Address addr_;
   int nelem_;
   Address data_;
   int word_len_;
};

class WaitPidNoBlockCallback : public DBICallbackBase
{
  public:
   WaitPidNoBlockCallback(eventLock *l) : DBICallbackBase(dbiWaitPid, l) {}
   WaitPidNoBlockCallback(WaitPidNoBlockCallback &src) : DBICallbackBase(dbiWaitPid,
                                                                         src.lock) {}
   virtual ~WaitPidNoBlockCallback() {}

   CallbackBase *copy() { return new WaitPidNoBlockCallback(*this);}
   bool execute_real(void);
   bool operator()(int *status);

   int getReturnValue() {return ret;}

  private:
   int ret;
   int *status_;
};

#if defined (arch_ia64)
class GetFrameRegisterCallback : public DBICallbackBase
{
  public:
   GetFrameRegisterCallback(eventLock *l) : DBICallbackBase(dbiGetFrameRegister, l) {}
   GetFrameRegisterCallback(GetFrameRegisterCallback &src) : 
       DBICallbackBase(dbiGetFrameRegister, src.lock) {}
   virtual ~GetFrameRegisterCallback() {}

   CallbackBase *copy() { return new GetFrameRegisterCallback(*this);}
   bool execute_real(void);
   bool operator()(unw_cursor_t *cp, unw_regnum_t reg, unw_word_t *valp);

   int getReturnValue() {return ret;}

  private:
   int ret;
   unw_cursor_t *cp_;
   unw_regnum_t reg_;
   unw_word_t *valp_;
};

class SetFrameRegisterCallback : public DBICallbackBase
{
  public:
   SetFrameRegisterCallback(eventLock *l) : DBICallbackBase(dbiSetFrameRegister,l) {}
   SetFrameRegisterCallback(SetFrameRegisterCallback &src) : 
      DBICallbackBase(dbiSetFrameRegister, src.lock) {}
   virtual ~SetFrameRegisterCallback() {}

   CallbackBase *copy() { return new SetFrameRegisterCallback(*this);}
   bool execute_real(void);
   bool operator()(unw_cursor_t *cp, unw_regnum_t reg, unw_word_t val);

   int getReturnValue() {return ret;}

  private:
   int ret;
   unw_cursor_t *cp_;
   unw_regnum_t reg_;
   unw_word_t val_;
};

class CreateUnwindAddressSpaceCallback : public DBICallbackBase
{
  public:
   CreateUnwindAddressSpaceCallback(eventLock *l) : 
      DBICallbackBase(dbiCreateUnwindAddressSpace, l) {}
   CreateUnwindAddressSpaceCallback(CreateUnwindAddressSpaceCallback &src) :
      DBICallbackBase(dbiCreateUnwindAddressSpace, src.lock) {}
   virtual ~CreateUnwindAddressSpaceCallback() {}

   CallbackBase *copy() { return new CreateUnwindAddressSpaceCallback(*this);}
   bool execute_real(void);
   bool operator()(unw_accessors_t *ap, int byteorder);

   void * getReturnValue() {return ret;}

  private:
   void *ret;
   unw_accessors_t *ap_;
   int byteorder_;
};

class DestroyUnwindAddressSpaceCallback : public DBICallbackBase
{
  public:
   DestroyUnwindAddressSpaceCallback(eventLock *l) : 
        DBICallbackBase(dbiDestroyUnwindAddressSpace, l) {}
   DestroyUnwindAddressSpaceCallback(DestroyUnwindAddressSpaceCallback &src) :
      DBICallbackBase(dbiDestroyUnwindAddressSpace, src.lock) {}
   virtual ~DestroyUnwindAddressSpaceCallback() {}

   CallbackBase *copy() { return new DestroyUnwindAddressSpaceCallback(*this);}
   bool execute_real(void);
   bool operator()(unw_addr_space *ap);

   int getReturnValue() {return ret;}

  private:
   int ret;
   unw_addr_space *as_;
};

class UPTCreateCallback : public DBICallbackBase
{
  public:
   UPTCreateCallback(eventLock *l) : DBICallbackBase(dbiUPTCreate, l) {}
   UPTCreateCallback(UPTCreateCallback &src) : DBICallbackBase(dbiUPTCreate, src.lock) {}
   virtual ~UPTCreateCallback() {}

   CallbackBase *copy() { return new UPTCreateCallback(*this);}
   bool execute_real(void);
   bool operator()(pid_t pid);

   void *getReturnValue() {return ret;}

  private:
   void *ret;
   pid_t pid_;
};

class UPTDestroyCallback : public DBICallbackBase
{
  public:
   UPTDestroyCallback(eventLock *l) : DBICallbackBase(dbiUPTDestroy, l) {}
   UPTDestroyCallback(UPTDestroyCallback &src) : DBICallbackBase(dbiUPTDestroy, src.lock) {}
   virtual ~UPTDestroyCallback() {}

   CallbackBase *copy() { return new UPTDestroyCallback(*this);}
   bool execute_real(void);
   bool operator()(void *handle);


  private:
   void *handle_;
};

class InitFrameCallback : public DBICallbackBase
{
  public:
   InitFrameCallback(eventLock *l) : DBICallbackBase(dbiInitFrame,l) {}
   InitFrameCallback(InitFrameCallback &src) :
      DBICallbackBase(dbiInitFrame, src.lock) {}
   virtual ~InitFrameCallback() {}

   CallbackBase *copy() { return new InitFrameCallback(*this);}
   bool execute_real(void);
   bool operator()(unw_cursor_t *cp, unw_addr_space_t as, void *arg);

   int getReturnValue() {return ret;}
  private:
   int ret;
   unw_cursor_t *cp_;
   unw_addr_space_t as_;
   void *arg_;
};

class StepFrameUpCallback : public DBICallbackBase
{
  public:
   StepFrameUpCallback(eventLock *l) : DBICallbackBase(dbiStepFrame, l) {}
   StepFrameUpCallback(StepFrameUpCallback &src) :
      DBICallbackBase(dbiStepFrame,src.lock) {}
   virtual ~StepFrameUpCallback() {}

   CallbackBase *copy() { return new StepFrameUpCallback(*this);}
   bool execute_real(void);
   bool operator()(unw_cursor_t *cp);

   int getReturnValue() {return ret;}
  private:
   int ret;
   unw_cursor_t *cp_;
};

class IsSignalFrameCallback : public DBICallbackBase
{
  public:
   IsSignalFrameCallback(eventLock *l) : DBICallbackBase(dbiIsSignalFrame, l) {}
   IsSignalFrameCallback(IsSignalFrameCallback &src) :
      DBICallbackBase(dbiIsSignalFrame, src.lock) {}
   virtual ~IsSignalFrameCallback() {}

   CallbackBase *copy() { return new IsSignalFrameCallback(*this);}
   bool execute_real(void);
   bool operator()(unw_cursor_t *cp);

   bool getReturnValue() {return ret;}
  private:
   bool ret;
   unw_cursor_t *cp_;
};

class WaitpidCallback : public DBICallbackBase
{
  public:
   WaitpidCallback(eventLock *l) : DBICallbackBase(dbiWaitpid, l) {}
   WaitpidCallback(WaitpidCallback &src) :
      DBICallbackBase(dbiWaitpid, src.lock) {}
   virtual ~WaitpidCallback() {}

   CallbackBase *copy() { return new WaitpidCallback(*this);}
   bool execute_real(void);
   bool operator()(pid_t pid, int *status, int options);

   int getReturnValue() {return ret;}
  private:
   int ret;
   pid_t pid_;
   int *status_;
   int options_;
};
#endif // arch_ia64
#endif
