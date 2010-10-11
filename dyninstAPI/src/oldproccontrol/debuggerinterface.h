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
#if !defined(__DEBUGGER_INTERFACE__)
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
#include <stdlib.h>
#include <string>

#include "debug.h"

#if !defined (os_windows)
#include <unistd.h>
#include <sys/types.h>
#endif

#define PTRACE_RETURN long

#if defined (os_linux)
#define CREATE_DBI_THREAD createThread(); 
#define DBI_PLATFORM_TARGET_THREAD TARGET_DBI_THREAD
#else
#define CREATE_DBI_THREAD 
#define DBI_PLATFORM_TARGET_THREAD TARGET_ANY_THREAD
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

extern CallbackCompletionCallback dbi_signal_done;

class DBICallbackBase : public SyncCallback
{
  friend class DebuggerInterface;

  public:

  DBICallbackBase(DBIEventType t, eventLock *l) 
    : SyncCallback(true /*synchronous*/, l, DBI_PLATFORM_TARGET_THREAD, 
                   dbi_signalCompletion), 
      type(t) {}  

  virtual ~DBICallbackBase() {};
  static void dbi_signalCompletion(CallbackBase *cb);
  virtual bool waitForCompletion();
  virtual bool execute();
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
                        isBusy(false), evt(dbiUndefined) {
    CREATE_DBI_THREAD
    dbi_thread_id = getThreadID(); 
    dbi_printf("%s[%d][%s]:  created DBI thread, dbi_thread_id = %lu, -1UL = %lu\n", 
            __FILE__, __LINE__, getThreadStr(getExecThreadID()), 
            dbi_thread_id, (unsigned long)-1L);
  }

  virtual ~DebuggerInterface() {}

  bool forkNewProcess(std::string file, std::string dir, pdvector<std::string> *argv, 
                      pdvector<std::string> *envp,
                      std::string inputFile, std::string outputFile, int &traceLink,
                      int &pid, int stdin_fd, int stdout_fd, int stderr_fd,
                      SignalGenerator *sg = NULL);

  PTRACE_RETURN ptrace(int req, pid_t pid, Address addr, Address data, 
                       int word_len = -1, int *ptrace_errno = NULL);

  bool writeDataSpace(pid_t pid, Address addr, int nbytes, 
                      Address data, int address_width, 
                      const char *file, unsigned int line);

  bool readDataSpace(pid_t pid, Address addr, int nbytes, 
                     Address data, int address_width, 
                     const char * file, unsigned int line);

  int waitpidNoBlock(int *status);

  bool bulkPtraceWrite(void *inTraced, u_int nbytes, void *inSelf, int pid, int address_width);
  bool bulkPtraceRead(void *inTraced, u_int nbytes, void *inSelf, int pid, int address_width);
  private:
  bool isReady;
  bool isBusy;
  virtual bool waitNextEvent(DBIEvent &ev);
  virtual bool handleEvent(DBIEvent &ev)
    { __LOCK; bool ret = handleEventLocked(ev); __UNLOCK; return ret; }
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

// Process creation may be a responsibility of the DBI.
bool forkNewProcess_real(std::string file,
                    std::string dir, pdvector<std::string> *argv,
                    pdvector<std::string> *envp,
                    std::string inputFile, std::string outputFile, int &traceLink,
                    pid_t &pid, int stdin_fd, int stdout_fd, int stderr_fd);

//  Helper callback classes for use with mailbox system


#if defined( os_linux )
// There is a bug in linux kernels between 2.6.9 and 2.6.11.11 (inclusive)
// that leads to a kernel panic under the following conditions:
//
// Process A creates a thread B using clone()
// Process A forks off a child process C
// Thread B attaches to process C using ptrace
//
// At this point, A is C's real_parent but B is C's parent. If A exits
// before B completes, the process death handling routines will try to
// assign all of A's children to B (as B is a peer in A's thread group).
// This will trigger a (spurious) assert in the kernel (see kernel/exit.c,
// choose_new_parent(), lines 525 and 526 in the 2.6.9 source tree).
//
// This test and BUG() combination was removed from linux in revision
// 2.6.11.12, though it is unclear whether the developers realized
// the significance of the problem or were rather just taking out a bogus
// BUG() that they believed could not be triggered or was not indicative
// of a real problem (it is, in fact, perfectly fine in the PTRACE case
// for the newly assigned reaper process to be the same as the current
// parent, and there is code in reparent_thread() to handle it).
//
// Details aside, what this means is that until the 2.6.x kernel series
// is dust and ash, we cannot safely ptrace a process unless we are the
// thread/process/[insert favorite entity here] that created it; otherwise
// we are GUARANTEED to cause a kernel panic unless we are very careful
// about exiting (and I think there is general agreement that people
// should not be able crash their systems by handing Dyninst a SIGKILL).
//
// The workaround is to move fork operations onto the DBI thread for linux,
// as was done until 30.Jan.2006, when the unix fork implementation was
// brought in line with Windows. Hence this callback class.

class ForkNewProcessCallback : public DBICallbackBase
{
  public:
   ForkNewProcessCallback(eventLock *l) : DBICallbackBase(dbiForkNewProcess,l) {}
   ForkNewProcessCallback(ForkNewProcessCallback &src) : DBICallbackBase(dbiForkNewProcess,
                                                                         src.lock) {}
   virtual ~ForkNewProcessCallback() {}

   CallbackBase *copy() { return new ForkNewProcessCallback(*this);}
   bool execute_real(void);
   bool operator()(std::string file, std::string dir, pdvector<std::string> *argv, 
                   pdvector<std::string> *envp,
                   std::string inputFile, std::string outputFile, int &traceLink,
                   pid_t &pid, int stdin_fd, int stdout_fd, int stderr_fd,
                   SignalGenerator *sg = NULL);

   bool getReturnValue() {return ret;}

  private:
   bool ret;
   std::string *file_;
   std::string *dir_;
   pdvector<std::string> *argv_;
   pdvector<std::string> *envp_;
   std::string *inputFile_;
   std::string *outputFile_;
   int *traceLink_;
   int *pid_;
   SignalGenerator *sg_;
   int stdin_fd_;
   int stdout_fd_;
   int stderr_fd_;
};
#endif

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

#endif
