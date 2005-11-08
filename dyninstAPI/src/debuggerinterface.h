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
} DBIEventType;

//char *dbiEvent2Str(DBIEventType);

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

class DBICallbackBase : public CallbackBase
{
  friend class DebuggerInterface;
  friend void dbi_signal_done_(CallbackBase *cb);

  public:
  DBICallbackBase(DBIEventType t) : CallbackBase(DBI_PLATFORM_TARGET_THREAD, dbi_signal_done),
                                done_flag(false), type(t) {}
  ~DBICallbackBase() {}

  protected:
  bool done_flag;
  DBIEventType type;
  bool DBILock();
  bool DBIUnlock();
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
  void waitForCompletion(DBICallbackBase *cb, unsigned long target_thread_id =-1UL) {
    //fprintf(stderr, "%s[%d][%s]:  DBI callback sleeping until completion\n", 
     //       __FILE__, __LINE__, getThreadStr(getExecThreadID()));
    dbilock._Lock(__FILE__, __LINE__);
    evt = cb->type;
    if (isRunning()) {
      dbilock._Broadcast(__FILE__, __LINE__);
      dbilock._WaitForSignal(__FILE__, __LINE__);
      while (!cb->done_flag) {
        fprintf(stderr, "%s[%d][%s]:  weird!\n", FILE__, __LINE__, getThreadStr(getExecThreadID()));
        dbilock._WaitForSignal(__FILE__, __LINE__);
      }
      dbilock._Unlock(__FILE__, __LINE__);
    }else {
      assert (cb->done_flag);
      dbilock._Unlock(__FILE__, __LINE__);
    }
    //fprintf(stderr, "%s[%d][%s]:  DBI callback complete, waking\n", __FILE__, __LINE__, 
    //        getThreadStr(getExecThreadID()));
  }
  bool bulkPtraceWrite(void *inTraced, u_int nbytes, void *inSelf, int pid, int address_width);
  bool bulkPtraceRead(void *inTraced, u_int nbytes, void *inSelf, int pid, int address_width);
  private:
  bool isReady;
  bool isBusy;
  virtual bool waitNextEvent(DBIEvent &ev);
  virtual bool handleEvent(DBIEvent &ev)
    { LOCK_FUNCTION(bool, handleEventLocked, (ev));}
  bool handleEventLocked(DBIEvent &ev);

  void signalDone(DBICallbackBase *cb) {
    dbi_printf("%s[%d][%s]:  signalling DBI callback done\n", FILE__, __LINE__, 
           getThreadStr(getExecThreadID()));
    dbilock._Lock(FILE__, __LINE__);
    cb->done_flag = true;
    dbilock._Broadcast(FILE__, __LINE__);
    dbilock._Unlock(FILE__, __LINE__);
    dbi_printf("%s[%d][%s]:  signalled DBI callback done\n", FILE__, __LINE__, 
           getThreadStr(getExecThreadID()));
  }
  void getBusy() {
    do {
      dbilock._Lock(FILE__, __LINE__);
      if (isBusy) {
        fprintf(stderr, "%s[%d][%s]:  dbi is busy, waiting for signal\n", __FILE__, __LINE__, getThreadStr(getExecThreadID()));
        dbilock._WaitForSignal(FILE__, __LINE__);
        dbilock._Unlock(FILE__, __LINE__);
      }
      else {
        isBusy = true;
        dbilock._Unlock(FILE__, __LINE__);
        break;
      }
    } while (1);
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
   ForkNewProcessCallback() : DBICallbackBase(dbiForkNewProcess) {}
   ForkNewProcessCallback(ForkNewProcessCallback &) : DBICallbackBase(dbiForkNewProcess) {}
   ~ForkNewProcessCallback() {}

   CallbackBase *copy() { return new ForkNewProcessCallback(*this);}
   bool execute(void);
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
   PtraceCallback() : DBICallbackBase(dbiPtrace) {}
   PtraceCallback(PtraceCallback &) : DBICallbackBase(dbiPtrace) {}
   ~PtraceCallback() {}

   CallbackBase *copy() { return new PtraceCallback(*this);}
   bool execute(void);
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
   WriteDataSpaceCallback() : DBICallbackBase(dbiWriteDataSpace) {}
   WriteDataSpaceCallback(WriteDataSpaceCallback &) : DBICallbackBase(dbiWriteDataSpace) {}
   ~WriteDataSpaceCallback() {}

   CallbackBase *copy() { return new WriteDataSpaceCallback(*this);}
   bool execute(void);
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
   ReadDataSpaceCallback() : DBICallbackBase(dbiReadDataSpace) {}
   ReadDataSpaceCallback(ReadDataSpaceCallback &) : DBICallbackBase(dbiReadDataSpace) {}
   ~ReadDataSpaceCallback() {}

   CallbackBase *copy() { return new ReadDataSpaceCallback(*this);}
   bool execute(void);
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
   WaitPidNoBlockCallback() : DBICallbackBase(dbiWaitPid) {}
   WaitPidNoBlockCallback(WaitPidNoBlockCallback &) : DBICallbackBase(dbiWaitPid) {}
   ~WaitPidNoBlockCallback() {}

   CallbackBase *copy() { return new WaitPidNoBlockCallback(*this);}
   bool execute(void);
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
   GetFrameRegisterCallback() : DBICallbackBase(dbiGetFrameRegister) {}
   GetFrameRegisterCallback(GetFrameRegisterCallback &) : DBICallbackBase(dbiGetFrameRegister) {}
   ~GetFrameRegisterCallback() {}

   CallbackBase *copy() { return new GetFrameRegisterCallback(*this);}
   bool execute(void);
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
   SetFrameRegisterCallback() : DBICallbackBase(dbiSetFrameRegister) {}
   SetFrameRegisterCallback(SetFrameRegisterCallback &) : DBICallbackBase(dbiSetFrameRegister) {}
   ~SetFrameRegisterCallback() {}

   CallbackBase *copy() { return new SetFrameRegisterCallback(*this);}
   bool execute(void);
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
   CreateUnwindAddressSpaceCallback() : DBICallbackBase(dbiCreateUnwindAddressSpace) {}
   CreateUnwindAddressSpaceCallback(CreateUnwindAddressSpaceCallback &) :
      DBICallbackBase(dbiCreateUnwindAddressSpace) {}
   ~CreateUnwindAddressSpaceCallback() {}

   CallbackBase *copy() { return new CreateUnwindAddressSpaceCallback(*this);}
   bool execute(void);
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
   DestroyUnwindAddressSpaceCallback() : DBICallbackBase(dbiDestroyUnwindAddressSpace) {}
   DestroyUnwindAddressSpaceCallback(DestroyUnwindAddressSpaceCallback &) :
      DBICallbackBase(dbiDestroyUnwindAddressSpace) {}
   ~DestroyUnwindAddressSpaceCallback() {}

   CallbackBase *copy() { return new DestroyUnwindAddressSpaceCallback(*this);}
   bool execute(void);
   bool operator()(unw_addr_space *ap);

   int getReturnValue() {return ret;}

  private:
   int ret;
   unw_addr_space *as_;
};

class UPTCreateCallback : public DBICallbackBase
{
  public:
   UPTCreateCallback() : DBICallbackBase(dbiUPTCreate) {}
   UPTCreateCallback(UPTCreateCallback &) : DBICallbackBase(dbiUPTCreate) {}
   ~UPTCreateCallback() {}

   CallbackBase *copy() { return new UPTCreateCallback(*this);}
   bool execute(void);
   bool operator()(pid_t pid);

   void *getReturnValue() {return ret;}

  private:
   void *ret;
   pid_t pid_;
};

class UPTDestroyCallback : public DBICallbackBase
{
  public:
   UPTDestroyCallback() : DBICallbackBase(dbiUPTDestroy) {}
   UPTDestroyCallback(UPTDestroyCallback &) : DBICallbackBase(dbiUPTDestroy) {}
   ~UPTDestroyCallback() {}

   CallbackBase *copy() { return new UPTDestroyCallback(*this);}
   bool execute(void);
   bool operator()(void *handle);


  private:
   void *handle_;
};

class InitFrameCallback : public DBICallbackBase
{
  public:
   InitFrameCallback() : DBICallbackBase(dbiInitFrame) {}
   InitFrameCallback(InitFrameCallback &) :
      DBICallbackBase(dbiInitFrame) {}
   ~InitFrameCallback() {}

   CallbackBase *copy() { return new InitFrameCallback(*this);}
   bool execute(void);
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
   StepFrameUpCallback() : DBICallbackBase(dbiStepFrame) {}
   StepFrameUpCallback(StepFrameUpCallback &) :
      DBICallbackBase(dbiStepFrame) {}
   ~StepFrameUpCallback() {}

   CallbackBase *copy() { return new StepFrameUpCallback(*this);}
   bool execute(void);
   bool operator()(unw_cursor_t *cp);

   int getReturnValue() {return ret;}
  private:
   int ret;
   unw_cursor_t *cp_;
};

class IsSignalFrameCallback : public DBICallbackBase
{
  public:
   IsSignalFrameCallback() : DBICallbackBase(dbiIsSignalFrame) {}
   IsSignalFrameCallback(IsSignalFrameCallback &) :
      DBICallbackBase(dbiIsSignalFrame) {}
   ~IsSignalFrameCallback() {}

   CallbackBase *copy() { return new IsSignalFrameCallback(*this);}
   bool execute(void);
   bool operator()(unw_cursor_t *cp);

   bool getReturnValue() {return ret;}
  private:
   bool ret;
   unw_cursor_t *cp_;
};

class WaitpidCallback : public DBICallbackBase
{
  public:
   WaitpidCallback() : DBICallbackBase(dbiWaitpid) {}
   WaitpidCallback(IsSignalFrameCallback &) :
      DBICallbackBase(dbiWaitpid) {}
   ~WaitpidCallback() {}

   CallbackBase *copy() { return new WaitpidCallback(*this);}
   bool execute(void);
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
