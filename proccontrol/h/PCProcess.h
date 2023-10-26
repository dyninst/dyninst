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

#if !defined(PROCESSPC_H_)
#define PROCESSPC_H_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <stddef.h>
#include <utility>

#include "dyntypes.h"
#include "Architecture.h"
#include "registers/MachRegister.h"
#include "EventType.h"
#include "util.h"
#include "PCErrors.h"
#include "boost/checked_delete.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "boost/version.hpp"

#define CHECKED_DELETE_NOEXCEPT BOOST_NOEXCEPT



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
class MTLock;

#define PC_VERSION_8_0_0
#define PC_VERSION_8_1_0
#define PC_VERSION_8_2_0

#define pc_const_cast boost::const_pointer_cast

namespace Dyninst {

class SymbolReaderFactory;

namespace ProcControlAPI {

   extern PC_EXPORT bool is_restricted_ptrace;
 

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
class LibraryTracking;
class ThreadTracking;
class LWPTracking;
class CallStackUnwinding;
class FollowFork;
class SignalMask;
class RemoteIO;
class MemoryUsage;

class ExecFileInfo;

class PC_EXPORT Breakpoint 
{
   friend class ::int_breakpoint;
   friend void boost::checked_delete<Breakpoint>(Breakpoint *) CHECKED_DELETE_NOEXCEPT;
   friend void boost::checked_delete<const Breakpoint>(const Breakpoint *) CHECKED_DELETE_NOEXCEPT;
 private:
   int_breakpoint *llbreakpoint_;
   Breakpoint();
   ~Breakpoint();
 public:
   static const int BP_X = 1;
   static const int BP_W = 2;
   static const int BP_R = 4;

   int_breakpoint *llbp() const;
   typedef boost::shared_ptr<Breakpoint> ptr;
   typedef boost::shared_ptr<const Breakpoint> const_ptr;
   typedef boost::weak_ptr<Breakpoint> weak_ptr;

   static Breakpoint::ptr newBreakpoint();
   static Breakpoint::ptr newTransferBreakpoint(Dyninst::Address to);
   static Breakpoint::ptr newTransferOffsetBreakpoint(signed long shift);
   static Breakpoint::ptr newHardwareBreakpoint(unsigned int mode, unsigned int size);

   void *getData() const;
   void setData(void *p) const;

   bool isCtrlTransfer() const;
   Dyninst::Address getToAddress() const;

   void setSuppressCallbacks(bool);
   bool suppressCallbacks() const;
};

class PC_EXPORT Library
{
   friend class ::int_library;
   friend void boost::checked_delete<Library>(Library *) CHECKED_DELETE_NOEXCEPT;
   friend void boost::checked_delete<const Library>(const Library *) CHECKED_DELETE_NOEXCEPT;
 private:
   int_library *lib;
   Library();
   ~Library();
 public:
   typedef boost::shared_ptr<Library> ptr;
   typedef boost::shared_ptr<const Library> const_ptr;

   std::string getName() const;
   std::string getAbsoluteName() const;
   Dyninst::Address getLoadAddress() const;
   Dyninst::Address getDataLoadAddress() const;
   Dyninst::Address getDynamicAddress() const;
   int_library *debug() const { return lib; }
   bool isSharedLib() const;

   void *getData() const;
   void setData(void *p) const;
};

class PC_EXPORT LibraryPool
{
   friend class ::int_process;
   friend class Dyninst::ProcControlAPI::Process;
 private:
   int_process *proc;
   LibraryPool();
   ~LibraryPool();
 public:
   class PC_EXPORT iterator  {
      friend class Dyninst::ProcControlAPI::LibraryPool;
   private:
      std::set<int_library *>::iterator int_iter;
   public:
      iterator();
      Library::ptr operator*() const;
      bool operator==(const iterator &i) const;
      bool operator!=(const iterator &i) const;
      LibraryPool::iterator operator++();
      LibraryPool::iterator operator++(int);

      typedef Library::ptr value_type;
      typedef int difference_type;
      typedef Library::ptr *pointer;
      typedef Library::ptr &reference;
      typedef std::forward_iterator_tag iterator_category;
  };

   class PC_EXPORT const_iterator  {
     friend class Dyninst::ProcControlAPI::LibraryPool;
  private:
     std::set<int_library *>::iterator int_iter;
  public:
     const_iterator();
     Library::const_ptr operator*() const;
     bool operator==(const const_iterator &i) const;
     bool operator!=(const const_iterator &i) const;
     LibraryPool::const_iterator operator++();
     LibraryPool::const_iterator operator++(int);

      typedef Library::const_ptr value_type;
      typedef int difference_type;
      typedef Library::const_ptr *pointer;
      typedef Library::const_ptr &reference;
      typedef std::forward_iterator_tag iterator_category;
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

  iterator find(Library::ptr lib);
  const_iterator find(Library::ptr lib) const;

};

class PC_EXPORT IRPC
{
   friend class ::int_iRPC;
   friend void boost::checked_delete<IRPC>(IRPC *) CHECKED_DELETE_NOEXCEPT;
   friend void boost::checked_delete<const IRPC>(const IRPC *) CHECKED_DELETE_NOEXCEPT;
 private:
   rpc_wrapper *wrapper;
   IRPC(rpc_wrapper *wrapper_);
   ~IRPC();
 public:
	 typedef enum {
	  Error = 0,
	  Created = 1,
	  Posted = 2,
	  Running = 3,
	  // Callback = 4,
	  Done = 5 } State;

   typedef boost::shared_ptr<IRPC> ptr;
   typedef boost::shared_ptr<const IRPC> const_ptr;
   typedef boost::weak_ptr<IRPC> weak_ptr;
   
   static IRPC::ptr createIRPC(void *binary_blob, unsigned size, 
                               bool non_blocking = false);
   static IRPC::ptr createIRPC(void *binary_blob, unsigned size,
                               Dyninst::Address addr, bool non_blocking = false);
   static IRPC::ptr createIRPC(IRPC::ptr orig);
   static IRPC::ptr createIRPC(IRPC::ptr orig, Address addr);
   
   rpc_wrapper *llrpc() const;

   Dyninst::Address getAddress() const;
   void *getBinaryCode() const;
   unsigned getBinaryCodeSize() const;
   unsigned long getID() const;
   void setStartOffset(unsigned long);
   unsigned long getStartOffset() const;
   bool isBlocking() const;

   // user-defined data retrievable during a callback
   void *getData() const;
   void setData(void *p) const;

   State state() const;

   // Continues the thread this RPC is running on.
   // Useful if you don't know the thread assigned to an IRPC
   bool continueStoppedIRPC();
};

class PC_EXPORT Process : public boost::enable_shared_from_this<Process>
{
 private:
   friend class ::int_process;
   friend class ProcessSet;

   int_process *llproc_;
   proc_exitstate *exitstate_;
   
   Process();
   ~Process();
   friend void boost::checked_delete<Process>(Process *) CHECKED_DELETE_NOEXCEPT;
   friend void boost::checked_delete<const Process>(const Process *) CHECKED_DELETE_NOEXCEPT;
 public:
   typedef boost::shared_ptr<Process> ptr;
   typedef boost::shared_ptr<const Process> const_ptr;
   typedef boost::weak_ptr<Process> weak_ptr;
   typedef boost::weak_ptr<const Process> const_weak_ptr;

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
   typedef cb_ret_t(*cb_func_t)(boost::shared_ptr<const Event>);

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
    * What capabilities do we have on this process
    **/
   static const unsigned int pc_read      = (1<<0);
   static const unsigned int pc_write     = (1<<1);
   static const unsigned int pc_irpc      = (1<<2);   
   static const unsigned int pc_control   = (1<<3);
   unsigned int getCapabilities() const;
   
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
   bool detach(bool leaveStopped = false);
   bool terminate();
   bool temporaryDetach();
   bool reAttach();

   /**
    * Memory management
    **/
   class PC_EXPORT mem_perm {
       bool read;
       bool write;
       bool execute;

       int permVal() const;

   public:
       mem_perm() : read(false), write(false), execute(false) {} 
       mem_perm(const mem_perm& p) : read(p.read), write(p.write),
                                     execute(p.execute) {}
       mem_perm(bool r, bool w, bool x) : read(r), write(w), execute(x) {} 

       bool getR() const { return read;    }
       bool getW() const { return write;   }
       bool getX() const { return execute; }

       bool isNone() const { return !read && !write && !execute; }
       bool isR()    const { return  read && !write && !execute; }
       bool isX()    const { return !read && !write &&  execute; }
       bool isRW()   const { return  read &&  write && !execute; }
       bool isRX()   const { return  read && !write &&  execute; }
       bool isRWX()  const { return  read &&  write &&  execute; }

       mem_perm& setR() { read    = true;  return *this; }
       mem_perm& setW() { write   = true;  return *this; }
       mem_perm& setX() { execute = true;  return *this; }

       mem_perm& clrR() { read    = false; return *this; }
       mem_perm& clrW() { write   = false; return *this; }
       mem_perm& clrX() { execute = false; return *this; }

       bool operator< (const mem_perm& p) const;
       bool operator==(const mem_perm& p) const;
       bool operator!=(const mem_perm& p) const;

       std::string getPermName() const;
   };

   unsigned getMemoryPageSize() const;

   Dyninst::Address mallocMemory(size_t size);
   Dyninst::Address mallocMemory(size_t size, Dyninst::Address addr);
   bool freeMemory(Dyninst::Address addr);
   bool writeMemory(Dyninst::Address addr, const void *buffer, size_t size) const;
   bool readMemory(void *buffer, Dyninst::Address addr, size_t size) const;

   bool writeMemoryAsync(Dyninst::Address addr, const void *buffer, size_t size, void *opaque_val = NULL) const;
   bool readMemoryAsync(void *buffer, Dyninst::Address addr, size_t size, void *opaque_val = NULL) const;

   /** 
    * Currently Windows-only, needed for the test infrastructure but possibly useful elsewhere 
    **/
   Dyninst::Address findFreeMemory(size_t size);

   bool getMemoryAccessRights(Dyninst::Address addr, mem_perm& rights);
   bool setMemoryAccessRights(Dyninst::Address addr, size_t size,
                              mem_perm rights, mem_perm& oldRights);

   // MemoryRegion.first = startAddr, MemoryRegion.second = endAddr
   typedef std::pair<Dyninst::Address, Dyninst::Address> MemoryRegion;
   bool findAllocatedRegionAround(Dyninst::Address addr, MemoryRegion& memRegion);

   /**
    * Libraries
    **/
   const LibraryPool &libraries() const;
   LibraryPool &libraries();

   // Cause a library to be loaded into the process (via black magic)
   bool addLibrary(std::string libname);

   /**
    * Breakpoints
    **/
   bool addBreakpoint(Dyninst::Address addr, Breakpoint::ptr bp) const;
   bool rmBreakpoint(Dyninst::Address addr, Breakpoint::ptr bp) const;
   unsigned numHardwareBreakpointsAvail(unsigned mode);

   /**
    * Post IRPC.  Use continueProc/continueThread to run it,
    * and handleEvents to wait for a blocking IRPC to complete
    **/
   bool postIRPC(IRPC::ptr irpc) const;
   bool getPostedIRPCs(std::vector<IRPC::ptr> &rpcs) const;

   /**
    * Post, run, and wait for an IRPC to complete
    **/
	bool runIRPCSync(IRPC::ptr irpc);

   /**
    * Post and run an IRPC; user must wait for completion. 
    **/
	bool runIRPCAsync(IRPC::ptr irpc);

   /**
    * Symbol access
    **/
   void setSymbolReader(SymbolReaderFactory *reader) const;
   SymbolReaderFactory *getSymbolReader() const;
   static SymbolReaderFactory *getDefaultSymbolReader();
   static void setDefaultSymbolReader(SymbolReaderFactory *reader);

   /**
    * Perform specific operations.  Interface objects will only be returned
    * on appropriately supported platforms, others will return NULL.
    **/
   LibraryTracking *getLibraryTracking();
   ThreadTracking *getThreadTracking();
   LWPTracking *getLWPTracking();
   FollowFork *getFollowFork();
   SignalMask *getSignalMask();
   RemoteIO *getRemoteIO();
   MemoryUsage *getMemoryUsage();
   const LibraryTracking *getLibraryTracking() const;
   const ThreadTracking *getThreadTracking() const;
   const LWPTracking *getLWPTracking() const;
   const FollowFork *getFollowFork() const;
   const SignalMask *getSignalMask() const;
   const RemoteIO *getRemoteIO() const;
   const MemoryUsage *getMemoryUsage() const;

   /**
    * Errors that occured on this process
    **/
   ProcControlAPI::err_t getLastError() const;
   const char *getLastErrorMsg() const;

   /**
    * Executable info
    **/
	ExecFileInfo* getExecutableInfo() const;
};

class PC_EXPORT Thread : public boost::enable_shared_from_this<Thread>
{
 protected:
   friend class ::int_thread;
   int_thread *llthread_;
   thread_exitstate *exitstate_;

   Thread();
   ~Thread();
   friend void boost::checked_delete<Thread>(Thread *) CHECKED_DELETE_NOEXCEPT;
   friend void boost::checked_delete<const Thread>(const Thread *) CHECKED_DELETE_NOEXCEPT;

 public:
   typedef boost::shared_ptr<Thread> ptr;
   typedef boost::shared_ptr<const Thread> const_ptr;
   typedef boost::weak_ptr<Thread> weak_ptr;
   typedef boost::weak_ptr<const Thread> const_weak_ptr;
   int_thread *llthrd() const;
   void setLastError(err_t ec, const char *es) const;

   Dyninst::LWP getLWP() const;
   Process::ptr getProcess();
   Process::const_ptr getProcess() const;

   bool isStopped() const;
   bool isRunning() const;
   bool isLive() const;
   bool isDetached() const;
   bool isInitialThread() const;

   // Added for Windows. Windows creates internal threads which are bound to 
   // the process but are used for OS-level work. We hide these from the user,
   // but need to represent them in ProcControlAPI. 
   bool isUser() const; 

   bool stopThread();
   bool continueThread();

   bool setSingleStepMode(bool s) const;
   bool getSingleStepMode() const;

   bool setSyscallMode(bool s) const;
   bool getSyscallMode() const;

   bool getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val) const;
   bool getAllRegisters(RegisterPool &pool) const;
   bool setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val) const;
   bool setAllRegisters(RegisterPool &pool) const;
   bool getAllRegistersAsync(RegisterPool &pool, void *opaque_val = NULL) const;
   bool setAllRegistersAsync(RegisterPool &pool, void *opaque_val = NULL) const;

   bool readThreadLocalMemory(void *buffer, Library::const_ptr lib, Dyninst::Offset tls_symbol_offset, size_t size) const;
   bool writeThreadLocalMemory(Library::const_ptr lib, Dyninst::Offset tls_symbol_offset, const void *buffer, size_t size) const;
   bool getThreadLocalAddress(Library::const_ptr lib, Dyninst::Offset tls_offset, Dyninst::Address &result_addr) const;

   /**
    * User level thread info.  Only available after a UserThreadCreate event
    **/
   bool haveUserThreadInfo() const;
   Dyninst::THR_ID getTID() const;
   Dyninst::Address getStartFunction() const;
   Dyninst::Address getStackBase() const;
   unsigned long getStackSize() const;
   Dyninst::Address getTLS() const;
   Dyninst::Address getThreadInfoBlockAddr() const;

   /**
    * IRPC
    **/
   bool postIRPC(IRPC::ptr irpc) const;
   bool runIRPCSync(IRPC::ptr irpc);
   bool runIRPCAsync(IRPC::ptr irpc);

   bool getPostedIRPCs(std::vector<IRPC::ptr> &rpcs) const;
   IRPC::const_ptr getRunningIRPC() const;

   /**
    * Returns a stack unwinder on supported platforms.
    * Returns NULL on unsupported platforms
    **/
   CallStackUnwinding *getCallStackUnwinding();

   void *getData() const;
   void setData(void *p) const;
};

class PC_EXPORT ThreadPool
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
   class PC_EXPORT iterator {
      friend class Dyninst::ProcControlAPI::ThreadPool;
   private:
      static const int uninitialized_val = -1;
      static const int end_val = -2;
      int_threadPool *curp;
      Thread::ptr curh;
      int curi;
   public:
      iterator();
      Thread::ptr operator*() const;
      bool operator==(const iterator &i) const;
      bool operator!=(const iterator &i) const;
      ThreadPool::iterator operator++();
      ThreadPool::iterator operator++(int);
   };
   iterator begin();
   iterator end();
   iterator find(Dyninst::LWP lwp);

   class PC_EXPORT const_iterator {
      friend class Dyninst::ProcControlAPI::ThreadPool;
   private:
      static const int uninitialized_val = -1;
      static const int end_val = -2;
      int_threadPool *curp;
      Thread::ptr curh;
      int curi;
   public:
      const_iterator();
      Thread::const_ptr operator*() const;
      bool operator==(const const_iterator &i) const;
      bool operator!=(const const_iterator &i) const;
      ThreadPool::const_iterator operator++();
      ThreadPool::const_iterator operator++(int);

	  typedef Thread::const_ptr value_type;
	  typedef int difference_type;
	  typedef Thread::const_ptr *pointer;
	  typedef Thread::const_ptr &reference;
	  typedef std::forward_iterator_tag iterator_category;
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

class PC_EXPORT RegisterPool
{ 
   friend class Dyninst::ProcControlAPI::Thread;
   friend class Dyninst::ProcControlAPI::ThreadSet;
 private:
   int_registerPool *llregpool;
 public:
   RegisterPool();
   RegisterPool(const RegisterPool &rp);
   ~RegisterPool();
   
   class PC_EXPORT iterator {
      friend class Dyninst::ProcControlAPI::RegisterPool;
   private:
      typedef std::map<Dyninst::MachRegister, Dyninst::MachRegisterVal>::iterator int_iter; 
      int_iter i;
      iterator(int_iter i_);
   public:
      iterator();
      std::pair<Dyninst::MachRegister, Dyninst::MachRegisterVal> operator*();
      bool operator==(const iterator &i) const;
      bool operator!=(const iterator &i) const;
      RegisterPool::iterator operator++();
      RegisterPool::iterator operator++(int);
   };
   iterator begin();
   iterator end();
   iterator find(Dyninst::MachRegister r);

   class PC_EXPORT const_iterator {
      friend class Dyninst::ProcControlAPI::RegisterPool;
   private:
      typedef std::map<Dyninst::MachRegister, Dyninst::MachRegisterVal>::const_iterator int_iter; 
      int_iter i;
      const_iterator(int_iter i_);
   public:
      const_iterator();
      std::pair<Dyninst::MachRegister, Dyninst::MachRegisterVal> operator*() const;
      bool operator==(const const_iterator &i) const;
      bool operator!=(const const_iterator &i) const;
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

class PC_EXPORT EventNotify
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
PC_EXPORT EventNotify *evNotify();

class PC_EXPORT ExecFileInfo
{
  public:
	void* fileHandle;
	void* processHandle;
	Address fileBase;
};


}
}

#endif
