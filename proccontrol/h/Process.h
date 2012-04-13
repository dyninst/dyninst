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

#if !defined(PROCESSPC_H_)
#define PROCESSPC_H_

#include <string>
#include <vector>
#include <map>
#include <set>

#include "dyntypes.h"
#include "dyn_regs.h"
#include "EventType.h"
#include "PCErrors.h"
#include "dynptr.h"

class int_process;
class int_breakpoint;
class proc_exitstate;
class thread_exitstate;
class int_library;
class int_thread;
class int_threadPool;
class int_registerPool;
class rpc_wrapper;
class int_iRPC;
class int_notify;
class HandlerPool;

#define pc_const_cast dyn_const_pointer_cast

namespace Dyninst {

class SymbolReaderFactory;

namespace ProcControlAPI {

class Process;
class ThreadPool;
class Thread;
class EventDecoder;
class EventGenerator;
class EventHandler;
class Event;
class RegisterPool;
class Breakpoint;
class ProcessSet;
class ThreadSet;

class Breakpoint 
{
   friend class ::int_breakpoint;
   friend void dyn_checked_delete<Breakpoint>(Breakpoint *);
   friend void dyn_checked_delete<const Breakpoint>(const Breakpoint *);
 private:
   int_breakpoint *llbreakpoint_;
   Breakpoint();
   ~Breakpoint();
 public:
   int_breakpoint *llbp() const;
   typedef dyn_shared_ptr<Breakpoint> ptr;
   typedef dyn_shared_ptr<const Breakpoint> const_ptr;
   typedef dyn_weak_ptr<Breakpoint> weak_ptr;

   static Breakpoint::ptr newBreakpoint();
   static Breakpoint::ptr newTransferBreakpoint(Dyninst::Address to);

   void *getData() const;
   void setData(void *p) const;

   bool isCtrlTransfer() const;
   Dyninst::Address getToAddress() const;
};

class Library
{
   friend class ::int_library;
   friend void dyn_checked_delete<Library>(Library *);
   friend void dyn_checked_delete<const Library>(const Library *);
 private:
   int_library *lib;
   Library();
   ~Library();
 public:
   typedef dyn_shared_ptr<Library> ptr;
   typedef dyn_shared_ptr<const Library> const_ptr;

   std::string getName() const;
   Dyninst::Address getLoadAddress() const;
   Dyninst::Address getDataLoadAddress() const;
   Dyninst::Address getDynamicAddress() const;
   
   void *getData() const;
   void setData(void *p) const;
};

class LibraryPool
{
   friend class ::int_process;
   friend class Dyninst::ProcControlAPI::Process;
 private:
   int_process *proc;
   LibraryPool();
   ~LibraryPool();
 public:
  class iterator {
      friend class Dyninst::ProcControlAPI::LibraryPool;
   private:
      std::set<int_library *>::iterator int_iter;
   public:
      iterator();
      ~iterator();
      Library::ptr operator*() const;
      bool operator==(const iterator &i) const;
      bool operator!=(const iterator &i) const;
      LibraryPool::iterator operator++();
      LibraryPool::iterator operator++(int);
  };

  class const_iterator {
     friend class Dyninst::ProcControlAPI::LibraryPool;
  private:
     std::set<int_library *>::iterator int_iter;
  public:
     const_iterator();
     ~const_iterator();
     Library::const_ptr operator*() const;
     bool operator==(const const_iterator &i);
     bool operator!=(const const_iterator &i);
     LibraryPool::const_iterator operator++();
     LibraryPool::const_iterator operator++(int);
  };

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;

  size_t size() const;

  Library::ptr getExecutable();
  Library::const_ptr getExecutable() const;

  Library::ptr getLibraryByName(std::string s);
  Library::const_ptr getLibraryByName(std::string s) const;
};

class IRPC
{
   friend class ::int_iRPC;
   friend void dyn_checked_delete<IRPC>(IRPC *);
   friend void dyn_checked_delete<const IRPC>(const IRPC *);
 private:
   rpc_wrapper *wrapper;
   IRPC(rpc_wrapper *wrapper_);
   ~IRPC();
 public:
   typedef dyn_shared_ptr<IRPC> ptr;
   typedef dyn_shared_ptr<const IRPC> const_ptr;
   typedef dyn_weak_ptr<IRPC> weak_ptr;
   
   static IRPC::ptr createIRPC(void *binary_blob, unsigned size, 
                               bool async = false);
   static IRPC::ptr createIRPC(void *binary_blob, unsigned size, 
                               Dyninst::Address addr, bool async = false);
   static IRPC::ptr createIRPC(IRPC::ptr orig);
   static IRPC::ptr createIRPC(IRPC::ptr orig, Address addr);
   
   rpc_wrapper *llrpc() const;

   Dyninst::Address getAddress() const;
   void *getBinaryCode() const;
   unsigned getBinaryCodeSize() const;
   unsigned long getID() const;
   void setStartOffset(unsigned long);
   unsigned long getStartOffset() const;

   // user-defined data retrievable during a callback
   void *getData() const;
   void setData(void *p) const;
};

class Process : public dyn_enable_shared_from_this<Process>
{
 private:
   friend class ::int_process;
   friend class ProcessSet;

   int_process *llproc_;
   proc_exitstate *exitstate_;
   
   Process();
   ~Process();
   friend void dyn_checked_delete<Process>(Process *);
   friend void dyn_checked_delete<const Process>(const Process *);
 public:
   typedef dyn_shared_ptr<Process> ptr;
   typedef dyn_shared_ptr<const Process> const_ptr;
   static void version(int& major, int& minor, int& maintenance);

   //These four functions are not for end-users.  
   int_process *llproc() const { return llproc_; }
   proc_exitstate *exitstate() const { return exitstate_; }
   void setLastError(ProcControlAPI::err_t err_code, const char *err_str) const;
   void clearLastError() const;
   

   /**
    * Threading modes control
    **/
   typedef enum {
     NoThreads,
     GeneratorThreading,
     HandlerThreading,
     CallbackThreading
   } thread_mode_t;
   static bool setThreadingMode(thread_mode_t tm);
   static thread_mode_t getThreadingMode();

   /**
    * Create and attach to new processes
    **/
   static const std::map<int,int> emptyFDs;
   static const std::vector<std::string> emptyEnvp;
   static Process::ptr createProcess(std::string executable,
                                     const std::vector<std::string> &argv,
                                     const std::vector<std::string> &envp = emptyEnvp,
                                     const std::map<int,int> &fds = emptyFDs);
   static Process::ptr attachProcess(Dyninst::PID pid, std::string executable = "");

   /**
    * Event Management
    **/
   enum cb_action_t {
     cbDefault,
     cbThreadContinue,
     cbThreadStop,
     cbProcContinue,
     cbProcStop
   };
   struct cb_ret_t {
      cb_ret_t(cb_action_t p) : parent(p), child(cbDefault) {}
      cb_ret_t(cb_action_t p, cb_action_t c) : parent(p), child(c) {}
      cb_action_t parent;
      cb_action_t child;
   };
   
   //cb_func_t really takes an 'Event::const_ptr' as parameter, but this declaration
   // defines the shared_ptr declaration due to Event::const_ptr not being defined yet.
   typedef cb_ret_t(*cb_func_t)(dyn_shared_ptr<const Event>);

   static bool handleEvents(bool block);
   static bool registerEventCallback(EventType evt, cb_func_t cbfunc);
   static bool removeEventCallback(EventType evt, cb_func_t cbfunc);
   static bool removeEventCallback(EventType evt);
   static bool removeEventCallback(cb_func_t cbfunc);

   // user-defined data retrievable during a callback
   void *getData() const;
   void setData(void *p) const;

   Dyninst::PID getPid() const;

   /**
    * Threads
    **/
   const ThreadPool &threads() const;
   ThreadPool &threads();
   
   /**
    * Test state of process
    **/
   bool isTerminated() const;
   bool isExited() const;
   bool isCrashed() const;
   bool isDetached() const;
   int getExitCode() const;
   int getCrashSignal() const;

   bool hasStoppedThread() const;
   bool hasRunningThread() const;
   bool allThreadsStopped() const;
   bool allThreadsRunning() const;
   bool allThreadsRunningWhenAttached() const;

   /**
    * Queries for machine info
    **/
   Dyninst::Architecture getArchitecture() const;
   Dyninst::OSType getOS() const;

   /**
    * Query what kind of events this process supports
    **/
   bool supportsLWPEvents() const;
   bool supportsUserThreadEvents() const;
   bool supportsFork() const;
   bool supportsExec() const;

   /**
    * Control process
    **/
   bool continueProc();
   bool stopProc();
   bool detach();
   bool terminate();
   bool temporaryDetach();
   bool reAttach();

   /**
    * Memory management
    **/
   Dyninst::Address mallocMemory(size_t size);
   Dyninst::Address mallocMemory(size_t size, Dyninst::Address addr);
   bool freeMemory(Dyninst::Address addr);
   bool writeMemory(Dyninst::Address addr, const void *buffer, size_t size) const;
   bool readMemory(void *buffer, Dyninst::Address addr, size_t size) const;

   bool writeMemoryAsync(Dyninst::Address addr, const void *buffer, size_t size, void *opaque_val = NULL) const;
   bool readMemoryAsync(void *buffer, Dyninst::Address addr, size_t size, void *opaque_val = NULL) const;

   /**
    * Libraries
    **/
   const LibraryPool &libraries() const;
   LibraryPool &libraries();

   /**
    * Breakpoints
    **/
   bool addBreakpoint(Dyninst::Address addr, Breakpoint::ptr bp) const;
   bool rmBreakpoint(Dyninst::Address addr, Breakpoint::ptr bp) const;

   /**
    * IRPC
    **/
   dyn_shared_ptr<Thread> postIRPC(IRPC::ptr irpc) const;
   bool getPostedIRPCs(std::vector<IRPC::ptr> &rpcs) const;

   /**
    * Symbol access
    **/
   SymbolReaderFactory *getDefaultSymbolReader();

   /**
    * Errors that occured on this process
    **/
   ProcControlAPI::err_t getLastError() const;
   const char *getLastErrorMsg() const;
};

class Thread
{
 protected:
   friend class ::int_thread;
   int_thread *llthread_;
   thread_exitstate *exitstate_;

   Thread();
   ~Thread();
   friend void dyn_checked_delete<Thread>(Thread *);
   friend void dyn_checked_delete<const Thread>(const Thread *);

   void setLastError(err_t ec, const char *es) const;
 public:
   typedef dyn_shared_ptr<Thread> ptr;
   typedef dyn_shared_ptr<const Thread> const_ptr;
   int_thread *llthrd() const;

   Dyninst::LWP getLWP() const;
   Process::ptr getProcess();
   Process::const_ptr getProcess() const;

   bool isStopped() const;
   bool isRunning() const;
   bool isLive() const;
   bool isDetached() const;
   bool isInitialThread() const;

   bool stopThread();
   bool continueThread();

   void setSingleStepMode(bool s) const;
   bool getSingleStepMode() const;

   bool getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val) const;
   bool getAllRegisters(RegisterPool &pool) const;
   bool setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val) const;
   bool setAllRegisters(RegisterPool &pool) const;

   /**
    * User level thread info.  Only available after a UserThreadCreate event
    **/
   bool haveUserThreadInfo() const;
   Dyninst::THR_ID getTID() const;
   Dyninst::Address getStartFunction() const;
   Dyninst::Address getStackBase() const;
   unsigned long getStackSize() const;
   Dyninst::Address getTLS() const;

   /**
    * IRPC
    **/
   bool postIRPC(IRPC::ptr irpc) const;
   bool getPostedIRPCs(std::vector<IRPC::ptr> &rpcs) const;
   IRPC::const_ptr getRunningIRPC() const;

   void *getData() const;
   void setData(void *p) const;
};

class ThreadPool
{
 private:
   friend class ::int_threadPool;
   friend class Dyninst::ProcControlAPI::Process;
   int_threadPool *threadpool;
   ThreadPool();
   ~ThreadPool();
 public:
   /**
    * Iterators
    **/
   class iterator {
      friend class Dyninst::ProcControlAPI::ThreadPool;
   private:
      static const int uninitialized_val = -1;
      static const int end_val = -2;
      int_threadPool *curp;
      Thread::ptr curh;
      int curi;
   public:
      iterator();
      ~iterator();
      Thread::ptr operator*() const;
      bool operator==(const iterator &i);
      bool operator!=(const iterator &i);
      ThreadPool::iterator operator++();
      ThreadPool::iterator operator++(int);
   };
   iterator begin();
   iterator end();
   iterator find(Dyninst::LWP lwp);

   class const_iterator {
      friend class Dyninst::ProcControlAPI::ThreadPool;
   private:
      static const int uninitialized_val = -1;
      static const int end_val = -2;
      int_threadPool *curp;
      Thread::ptr curh;
      int curi;
   public:
      const_iterator();
      ~const_iterator();
      Thread::const_ptr operator*() const;
      bool operator==(const const_iterator &i);
      bool operator!=(const const_iterator &i);
      ThreadPool::const_iterator operator++();
      ThreadPool::const_iterator operator++(int);
   };
   const_iterator begin() const;
   const_iterator end() const;
   const_iterator find(Dyninst::LWP lwp) const;

   size_t size() const;
   Process::const_ptr getProcess() const;
   Process::ptr getProcess();
   Thread::const_ptr getInitialThread() const;
   Thread::ptr getInitialThread();
};

class RegisterPool
{ 
   friend class Dyninst::ProcControlAPI::Thread;
   friend class Dyninst::ProcControlAPI::ThreadSet;
 private:
   int_registerPool *llregpool;
 public:
   RegisterPool();
   RegisterPool(const RegisterPool &rp);
   ~RegisterPool();
   
   class iterator {
      friend class Dyninst::ProcControlAPI::RegisterPool;
   private:
      typedef std::map<Dyninst::MachRegister, Dyninst::MachRegisterVal>::iterator int_iter; 
      int_iter i;
      iterator(int_iter i_);
   public:
      iterator();
      ~iterator();
      std::pair<Dyninst::MachRegister, Dyninst::MachRegisterVal> operator*();
      bool operator==(const iterator &i);
      bool operator!=(const iterator &i);
      RegisterPool::iterator operator++();
      RegisterPool::iterator operator++(int);
   };
   iterator begin();
   iterator end();
   iterator find(Dyninst::MachRegister r);

   class const_iterator {
      friend class Dyninst::ProcControlAPI::RegisterPool;
   private:
      typedef std::map<Dyninst::MachRegister, Dyninst::MachRegisterVal>::const_iterator int_iter; 
      int_iter i;
      const_iterator(int_iter i_);
   public:
      const_iterator();
      ~const_iterator();
      std::pair<Dyninst::MachRegister, Dyninst::MachRegisterVal> operator*() const;
      bool operator==(const const_iterator &i);
      bool operator!=(const const_iterator &i);
      RegisterPool::const_iterator operator++();
      RegisterPool::const_iterator operator++(int);
   };
   const_iterator begin() const;
   const_iterator end() const;
   const_iterator find(Dyninst::MachRegister r) const;

   Dyninst::MachRegisterVal& operator[](Dyninst::MachRegister r);
   const Dyninst::MachRegisterVal& operator[](Dyninst::MachRegister r) const;

   size_t size() const;
   Thread::const_ptr getThread() const;
   Thread::ptr getThread();
};

class EventNotify
{
 private:
   friend class ::int_notify;
   int_notify *llnotify;
   EventNotify();
   ~EventNotify();
 public:
   typedef void (*notify_cb_t)();

   int getFD();
   void registerCB(notify_cb_t cb);
   void removeCB(notify_cb_t cb);
};
EventNotify *evNotify();

}
}

#endif
