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

#if !defined(INT_PROCESS_H_)
#define INT_PROCESS_H_

#include "PCProcess.h"
#include "PCErrors.h"
#include "Event.h"
#include "PlatFeatures.h"

#include "response.h"
#include "memcache.h"

#include "registers/MachRegister.h"
#include "Architecture.h"
#include "common/h/SymReader.h"
#include "common/src/dthread.h"

#include <assert.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <utility>
#include <queue>
#include <stack>

namespace Dyninst {
namespace ProcControlAPI {
class ProcessSet;
class CallStackCallback;
}
}

using namespace Dyninst;
using namespace ProcControlAPI;

class int_thread;
class int_threadPool;
class handlerpool;
class int_iRPC;

typedef std::multimap<Dyninst::Address, Dyninst::ProcControlAPI::Process::ptr> int_addressSet;
typedef std::set<Dyninst::ProcControlAPI::Process::ptr> int_processSet;
typedef std::set<Dyninst::ProcControlAPI::Thread::ptr> int_threadSet;

typedef boost::shared_ptr<int_iRPC> int_iRPC_ptr;
typedef std::map<Dyninst::MachRegister, std::pair<unsigned int, unsigned int> > dynreg_to_user_t;

typedef std::list<int_iRPC_ptr> rpc_list_t;

typedef void* hwbp_key_t;

class bp_instance;
class sw_breakpoint;
class hw_breakpoint;
class int_library;
class int_process;
class emulated_singlestep;

class int_libraryTracking;
class int_LWPTracking;
class int_threadTracking;
class int_followFork;
class int_callStackUnwinding;
class int_multiToolControl;
class int_signalMask;
class int_remoteIO;
class int_memStats;
class int_memUsage;

struct bp_install_state {
   Dyninst::Address addr;
   int_breakpoint *bp;
   sw_breakpoint *ibp;
   bool do_install;
   mem_response::ptr mem_resp;
   result_response::ptr res_resp;
};

/**
 * Data reflecting the contents of a process's memory should be
 * stored in the mem_state object (e.g, breakpoints, libraries
 * inferior mallocs).  That way, if two processes ever share memory
 * (see POSIX vfork), then they can share a mem_state object
 * and changes to one will be reflected in the other.
 **/
class mem_state
{
  public:
   typedef mem_state* ptr;
   mem_state(int_process *proc);
   mem_state(mem_state &m, int_process *proc);
   ~mem_state();

   void addProc(int_process *p);
   void rmProc(int_process *p, bool &should_clean);

   void addLibrary(int_library *lib);
   void rmLibrary(int_library *lib);

   std::set<int_process *> procs;
   std::set<int_library *> libs;
   std::map<Dyninst::Address, sw_breakpoint *> breakpoints;
   std::map<Dyninst::Address, unsigned long> inf_malloced_memory;
};

/**
 * Counters are usually* used to determine when to return from waitAndHandleEvents.
 * Each Counter is associated with int_processes or int_threads, and counts how
 * many of them are in a certain state.  For example, ClearingBPs counts how
 * many threads have int_threads are currently clearing breakpoints.
 *
 * Counters are embedded into their int_process and int_thread objects, and
 * if one of those objects are deleted the counter will automatically be
 * decremented.
 *
 * int_process::waitAndHandleEvents will check most counters when determining
 * whether it should return.  For example, if any threads are ClearingBPs then
 * int_process::waitAndHandleEvents will block until they're done.
 *
 * *=Some of the generator counters such as ForceGeneratorBlock and
 *   GeneratorNonExitedThreads are used in the generator rather than
 *   waitAndHandleEvents.
 **/
class Counter {
  public:
   static const int NumCounterTypes = 13;
   enum CounterType {
      HandlerRunningThreads = 0,
      GeneratorRunningThreads = 1,
      SyncRPCs = 2,
      SyncRPCRunningThreads = 3,
      PendingStops = 4,
      ClearingBPs = 5,
      ProcStopRPCs = 6,
      AsyncEvents = 7,
      ForceGeneratorBlock = 8,
      GeneratorNonExitedThreads = 9,
      StartupTeardownProcesses = 10,
      NeonatalThreads = 11,
      PendingStackwalks = 12
   };

   Counter(CounterType ct_);
   ~Counter();

   void inc();
   void dec();

   bool local() const;
   int localCount() const;
   static bool global(CounterType ct);
   static int globalCount(CounterType ct);
   static int processCount(CounterType ct, int_process* p);

   static const char *getNameForCounter(int counter_type);
  private:
   int local_count;
   CounterType ct;

   void adjust(int val);

   static Mutex<> * locks;
   static int global_counts[NumCounterTypes];
};

/**
 * This class works with the StateTracker (see below) to determine
 * when an Event should be handled.  Suppose you receive a Breakpoint
 * event on Linux, which stopped the thread it occured on but you
 * want the entire process stopped.
 *
 * Override the Event's procStopper() to set a StateTracker describing
 * the desired state of the process (all threads stopped in the BP case)
 * then return false from procStopper when the process reaches that state.
 *
 * PC will move the process towards your desired state, then handle the event
 * when procStopper returns false.  ProcStopEventManager will handle much of this,
 * and it's *StoppedTo functions can be used to see if the process has reached
 * your desired state.
 **/
class ProcStopEventManager {
  private:
   int_process *proc;
   std::set<Event::ptr> held_pstop_events;
  public:
   ProcStopEventManager(int_process *p);
   ~ProcStopEventManager();

   bool prepEvent(Event::ptr ev);
   void checkEvents();

   bool processStoppedTo(int state_id);
   bool threadStoppedTo(int_thread *thr, int state_id);
};

/**
 * This is the central class in PC, representing both a process and
 * and the porting interface needed to move PC to a new platform.
 *
 * Most of the information about a process hangs off of here (except
 * for memory related info, see mem_state above).  This includes
 * PIDs, execution state, environment, etc.
 *
 * There are numerous pure virtual functions below, most beginning
 * with plat_*.  To port PC to a new platform make a new class (ie.,
 * linux_process) that inherits from int_process and fill-in these
 * pure virtual functions.  Then have int_process::createProcess
 * return your new class, but cast back to an int_process.
 *
 * There are existing child classes of int_process that you can
 * use if your system shares certain things in common with other
 * platforms.  For example, several systems (Linux, FreeBSD)
 * use the System V interfaces for library loading.  Thus there
 * exists a sysv_process class that inherits from int_process and
 * fills in the library handling virtual functions of int_process.
 * Other examples are x86_process, which fill things like how
 * to build a breakpoint instruction.
 *
 * By having the new platforms inherit from these you leverage
 * a lot of existing work.  Note that new ports will also have
 * to implement their own decoders and generators.
 **/
class int_process
{
   friend class Dyninst::ProcControlAPI::Process;
   friend class Dyninst::ProcControlAPI::ProcessSet;
 protected:
   int_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
           std::vector<std::string> envp, std::map<int,int> f);
   int_process(Dyninst::PID pid_, int_process *p);
 public:
   static int_process *createProcess(Dyninst::PID p, std::string e);
   static int_process *createProcess(std::string e, std::vector<std::string> a,
           std::vector<std::string> envp, std::map<int,int> f);
   static int_process *createProcess(Dyninst::PID pid_, int_process *p);
   virtual ~int_process();
 protected:
   static bool create(int_processSet *ps);
   virtual bool plat_create() = 0;
   virtual async_ret_t post_create(std::set<response::ptr> &async_responses);

   static bool attach(int_processSet *ps, bool reattach);
   static bool reattach(int_processSet *pset);
   virtual bool plat_attach(bool allStopped, bool &should_sync) = 0;

   bool attachThreads(bool &found_new_threads);
   bool attachThreads();
   virtual bool plat_attachThreadsSync();

   virtual async_ret_t post_attach(bool wasDetached, std::set<response::ptr> &aresps);
   async_ret_t initializeAddressSpace(std::set<response::ptr> &async_responses);

   virtual bool plat_syncRunState() = 0;
   bool syncRunState();

  public:

   typedef enum {
      ct_fork,
      ct_launch,
      ct_attach
   } creationMode_t;
   creationMode_t getCreationMode() const;

   void setContSignal(int sig);
   int getContSignal() const;
   virtual bool forked();

   virtual OSType getOS() const = 0;
  protected:
   virtual bool plat_forked() = 0;
   virtual bool post_forked();

  public:
   bool execed();
   virtual bool plat_detach(result_response::ptr resp, bool leave_stopped) = 0;
   virtual bool plat_detachDone();
  protected:
   virtual bool plat_execed();
   virtual bool plat_terminate(bool &needs_sync) = 0;

   virtual bool needIndividualThreadAttach() = 0;
   virtual bool getThreadLWPs(std::vector<Dyninst::LWP> &lwps);

   virtual void plat_threadAttachDone();
   bool waitfor_startup();

   void setPid(Dyninst::PID pid);
   int_thread *findStoppedThread();

  public:
   virtual bool plat_processGroupContinues();

   typedef enum {
      neonatal = 0,
      neonatal_intermediate,
      detached,
      running,
      exited,
      errorstate
   } State;
   State getState() const;
   void setState(State s);

   Dyninst::PID getPid() const;
   int_threadPool *threadPool() const;

   Process::ptr proc() const;
   mem_state::ptr memory() const;

   err_t getLastError();
   const char *getLastErrorMsg();
   void clearLastError();
   void setLastError(err_t err, const char *err_str);

   void throwDetachEvent(bool temporary, bool leaveStopped);

   virtual bool preTerminate();
   bool terminate(bool &needs_sync);
   void updateSyncState(Event::ptr ev, bool gen);

   virtual void plat_adjustSyncType(Event::ptr, bool) {}
   virtual Dyninst::Architecture getTargetArch() = 0;
   virtual unsigned getTargetPageSize() = 0;
   virtual unsigned plat_getRecommendedReadSize();
   virtual Dyninst::Address mallocExecMemory(unsigned size);
   virtual Dyninst::Address plat_mallocExecMemory(Dyninst::Address min, unsigned size) = 0;
   virtual void freeExecMemory(Dyninst::Address addr);

   static bool waitAndHandleEvents(bool block);
   static bool waitAndHandleForProc(bool block, int_process *proc, bool &proc_exited);
   static bool waitForAsyncEvent(response::ptr resp);
   static bool waitForAsyncEvent(std::set<response::ptr> resp);

   virtual bool plat_waitAndHandleForProc();

   Counter &asyncEventCount();
   Counter &getForceGeneratorBlockCount();
   Counter &getStartupTeardownProcs();

   static const char *stateName(State s);

   void initializeProcess(Process::ptr p);

   virtual void instantiateRPCThread() {}

   void setExitCode(int c);
   void setCrashSignal(int s);
   bool getExitCode(int &c);
   bool getCrashSignal(int &s);
   bool wasForcedTerminated() const;

   virtual bool plat_individualRegAccess() = 0;
   virtual bool plat_individualRegRead(Dyninst::MachRegister reg, int_thread *thr);
   virtual bool plat_individualRegSet();

   int getAddressWidth();
   HandlerPool *handlerPool() const;

   bool addBreakpoint(Dyninst::Address addr, int_breakpoint *bp);
   bool addBreakpoint_phase1(bp_install_state *is);
   bool addBreakpoint_phase2(bp_install_state *is);
   bool addBreakpoint_phase3(bp_install_state *is);

   bool removeBreakpoint(Dyninst::Address addr, int_breakpoint *bp, std::set<response::ptr> &resps);
   bool removeAllBreakpoints();

   sw_breakpoint *getBreakpoint(Dyninst::Address addr);

   virtual unsigned plat_breakpointSize() = 0;
   virtual void plat_breakpointBytes(unsigned char *buffer) = 0;
   virtual bool plat_breakpointAdvancesPC() const = 0;

   virtual bool plat_createDeallocationSnippet(Dyninst::Address addr, unsigned long size, void* &buffer,
                                               unsigned long &buffer_size, unsigned long &start_offset) = 0;
   virtual bool plat_createAllocationSnippet(Dyninst::Address addr, bool use_addr, unsigned long size,
                                             void* &buffer, unsigned long &buffer_size,
                                             unsigned long &start_offset) = 0;
   virtual bool plat_collectAllocationResult(int_thread *thr, reg_response::ptr resp) = 0;
   virtual bool plat_threadOpsNeedProcStop();

   virtual SymbolReaderFactory *plat_defaultSymReader();
   virtual SymbolReaderFactory *getSymReader();
   virtual void setSymReader(SymbolReaderFactory *fact);

   virtual Dyninst::Address direct_infMalloc(unsigned long size, bool use_addr = false, Dyninst::Address addr = 0x0);
   virtual bool direct_infFree(Dyninst::Address addr);

   Address infMalloc(unsigned long size, bool use_addr, Address addr);
   bool infFree(Address addr);
   static bool infMalloc(unsigned long size, int_addressSet *aset, bool use_addr);
   static bool infFree(int_addressSet *aset);

   static std::string plat_canonicalizeFileName(std::string s);

   enum bp_write_t {
      not_bp,
      bp_install,
      bp_clear
   };

   bool readMem(Dyninst::Address remote, mem_response::ptr result, int_thread *thr = NULL);
   bool writeMem(const void *local, Dyninst::Address remote, size_t size, result_response::ptr result, int_thread *thr = NULL, bp_write_t bp_write = not_bp);

   virtual bool plat_readMem(int_thread *thr, void *local,
                             Dyninst::Address remote, size_t size) = 0;
   virtual bool plat_writeMem(int_thread *thr, const void *local,
                              Dyninst::Address remote, size_t size, bp_write_t bp_write) = 0;

   virtual async_ret_t plat_calcTLSAddress(int_thread *thread, int_library *lib, Offset off,
                                           Address &outaddr, std::set<response::ptr> &resps);

   virtual Address plat_findFreeMemory(size_t) { return 0; }

   //For a platform, if plat_needsAsyncIO returns true then the async
   // set of functions need to be implemented. By default these are
   // unimplemented.
   virtual bool plat_needsAsyncIO() const;
   virtual bool plat_readMemAsync(int_thread *thr, Dyninst::Address addr,
                                  mem_response::ptr result);
   virtual bool plat_writeMemAsync(int_thread *thr, const void *local, Dyninst::Address addr,
                                   size_t size, result_response::ptr result, bp_write_t bp_write);

   bool getMemoryAccessRights(Dyninst::Address addr, Process::mem_perm& rights);
   bool setMemoryAccessRights(Dyninst::Address addr, size_t size,
                              Process::mem_perm rights,
                              Process::mem_perm& oldRights);
   // Zuyu FIXME pure virtual function
   virtual bool plat_getMemoryAccessRights(Dyninst::Address addr, Process::mem_perm& rights);
   virtual bool plat_setMemoryAccessRights(Dyninst::Address addr, size_t size,
                                           Process::mem_perm rights,
                                           Process::mem_perm& oldRights);
   virtual bool plat_decodeMemoryRights(Process::mem_perm& rights_internal,
                                        unsigned long rights);
   virtual bool plat_encodeMemoryRights(Process::mem_perm rights_internal,
                                        unsigned long& rights);

   virtual bool findAllocatedRegionAround(Dyninst::Address addr,
                                          Process::MemoryRegion& memRegion);
   virtual bool plat_findAllocatedRegionAround(Dyninst::Address addr,
                                               Process::MemoryRegion& memRegion);

   memCache *getMemCache();

   virtual bool plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates) = 0;
	// Windows-only technically
   virtual void* plat_getDummyThreadHandle() const { return NULL; }

   virtual void noteNewDequeuedEvent(Event::ptr ev);

   static bool isInCB();
   static void setInCB(bool b);

   void throwNopEvent();
   void throwRPCPostEvent();

   virtual bool plat_supportFork();
   virtual bool plat_supportExec();
   virtual bool plat_supportDOTF();

   virtual bool plat_supportThreadEvents();
   virtual bool plat_supportLWPCreate();
   virtual bool plat_supportLWPPreDestroy();
   virtual bool plat_supportLWPPostDestroy();

   virtual bool plat_preHandleEvent();
   virtual bool plat_postHandleEvent();
   virtual bool plat_preAsyncWait();
   virtual bool plat_supportHWBreakpoint();

   virtual bool plat_needsPCSaveBeforeSingleStep();
   virtual async_ret_t plat_needsEmulatedSingleStep(int_thread *thr, std::vector<Dyninst::Address> &result);
   virtual bool plat_convertToBreakpointAddress(Address &, int_thread *) { return true; }
   virtual void plat_getEmulatedSingleStepAsyncs(int_thread *thr, std::set<response::ptr> resps);
   virtual bool plat_needsThreadForMemOps() const { return true; }
   virtual unsigned int plat_getCapabilities();
   virtual Event::ptr plat_throwEventsBeforeContinue(int_thread *thr);

   int_library *getLibraryByName(std::string s) const;
   size_t numLibs() const;
   virtual bool refresh_libraries(std::set<int_library *> &added_libs,
                                  std::set<int_library *> &rmd_libs,
                                  bool &waiting_for_async,
                                  std::set<response::ptr> &async_responses) = 0;

   virtual bool plat_isStaticBinary() = 0;
   virtual int_library *plat_getExecutable() = 0;

   virtual bool plat_supportDirectAllocation() const { return false; }
   bool forceGeneratorBlock() const;
   void setForceGeneratorBlock(bool b);

   std::string getExecutable() const;
   static bool isInCallback();

   static int_process *in_waitHandleProc;
   // TODO: clean up w/enum
   bool wasCreatedViaAttach() const { return createdViaAttach; }
   void wasCreatedViaAttach(bool val) { createdViaAttach = val; }

   // Platform-specific; is this address in what we consider a system lib.
   virtual bool addrInSystemLib(Address /*addr*/) { return false; }

   ProcStopEventManager &getProcStopManager();

   std::map<int, int> &getProcDesyncdStates();

   bool isRunningSilent(); //No callbacks
   void setRunningSilent(bool b);

   virtual ExecFileInfo* plat_getExecutableInfo() const { return NULL; }

   int_libraryTracking *getLibraryTracking();
   int_LWPTracking *getLWPTracking();
   int_threadTracking *getThreadTracking();
   int_followFork *getFollowFork();
   int_multiToolControl *getMultiToolControl();
   int_signalMask *getSignalMask();
   int_memUsage *getMemUsage();
   int_callStackUnwinding *getCallStackUnwinding();
   int_remoteIO *getRemoteIO();
 protected:
   State state;
   Dyninst::PID pid;
   creationMode_t creation_mode;
   std::string executable;
   std::vector<std::string> argv;
   std::vector<std::string> env;
   std::map<int,int> fds;
   Dyninst::Architecture arch;
   int_threadPool *threadpool;
   Process::ptr up_proc;
   HandlerPool *handlerpool;
   LibraryPool libpool;
   bool hasCrashSignal;
   int crashSignal;
   bool hasExitCode;
   bool forcedTermination;
   bool silent_mode;
   int exitCode;
   static bool in_callback;
   mem_state::ptr mem;
   std::map<Dyninst::Address, unsigned> exec_mem_cache;
   int continueSig;
   bool createdViaAttach;
   memCache mem_cache;
   Counter async_event_count;
   Counter force_generator_block_count;
   Counter startupteardown_procs;
   ProcStopEventManager proc_stop_manager;
   std::map<int, int> proc_desyncd_states;
   void *user_data;
   err_t last_error;
   const char *last_error_string;
   SymbolReaderFactory *symbol_reader;
   static SymbolReaderFactory *user_set_symbol_reader;

   //Cached PlatFeature pointers, which are used to avoid slow dynamic casts
   // (they're used frequently in tight loops on BG/Q)
   int_libraryTracking *pLibraryTracking;
   int_LWPTracking *pLWPTracking;
   int_threadTracking *pThreadTracking;
   int_followFork *pFollowFork;
   int_multiToolControl *pMultiToolControl;
   int_signalMask *pSignalMask;
   int_callStackUnwinding *pCallStackUnwinding;
   int_memUsage *pMemUsage;
   int_remoteIO *pRemoteIO;
   bool LibraryTracking_set;
   bool LWPTracking_set;
   bool ThreadTracking_set;
   bool FollowFork_set;
   bool MultiToolControl_set;
   bool SignalMask_set;
   bool CallStackUnwinding_set;
   bool MemUsage_set;
   bool remoteIO_set;
};

struct ProcToIntProc {
   int_process *operator()(const Process::ptr &p) const { return p->llproc(); }
};

/**
 * These processes represent four common models of how to stop/continue threads.
 * If a new platform follows one of these, then inherit them from the appropriate
 * class.
 *
 * indep_lwp_control_process - Each thread/lwp stops and continues independent from
 *  each other one.  (Linux)
 * unified_lwp_control_process - There is no thread-specific control, every thread
 *  stops/runs alongside its peers (BG/P)
 * hybrid_lwp_control_process - All threads in a process are run/stopped when
 *  a thread stops/runs, but threads can be overridden with a suspend state
 *  that can keep them stopped when others run (FreeBSD, Windows, BG/Q).
 **/
class indep_lwp_control_process : virtual public int_process
{
  protected:
   virtual bool plat_syncRunState();
  public:
   indep_lwp_control_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
                             std::vector<std::string> envp, std::map<int,int> f);
   indep_lwp_control_process(Dyninst::PID pid_, int_process *p);
   virtual ~indep_lwp_control_process();
};

class unified_lwp_control_process : virtual public int_process
{
  protected:
   virtual bool plat_syncRunState();
  public:
   unified_lwp_control_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
                               std::vector<std::string> envp, std::map<int,int> f);
   unified_lwp_control_process(Dyninst::PID pid_, int_process *p);
   virtual ~unified_lwp_control_process();

   virtual bool plat_processGroupContinues();
};

class hybrid_lwp_control_process : virtual public int_process
{
  protected:
   virtual bool plat_syncRunState();
   virtual bool plat_suspendThread(int_thread *thr) = 0;
   virtual bool plat_resumeThread(int_thread *thr) = 0;
   bool debugger_stopped;
  public:
   hybrid_lwp_control_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
                              std::vector<std::string> envp, std::map<int,int> f);
   hybrid_lwp_control_process(Dyninst::PID pid_, int_process *p);
   virtual ~hybrid_lwp_control_process();

   virtual bool suspendThread(int_thread *thr);
   virtual bool resumeThread(int_thread *thr);

   virtual void noteNewDequeuedEvent(Event::ptr ev);
   virtual bool plat_debuggerSuspended();

   virtual bool plat_processGroupContinues();
};

/**
 * A collection of registers from a thread.
 **/
class int_registerPool
{
 public:
   int_registerPool();

   typedef std::map<Dyninst::MachRegister, Dyninst::MachRegisterVal> reg_map_t;
   reg_map_t regs;
   bool full;
   int_thread *thread;

   typedef reg_map_t::iterator iterator;
   typedef reg_map_t::const_iterator const_iterator;
};

/**
 * When a thread/process exits we delete the int_process/int_thread objects.  But,
 * we leave the UI handles Process/Thread around until the user gets rid of their
 * last pointer.  There are only a few operations that are legal on an exited process
 * (such as getting the pid or exitcode).  thread_exitstate and proc_exitstate hold that
 * information after a process exits.
 **/
class thread_exitstate
{
  public:
   Dyninst::LWP lwp;
   Dyninst::THR_ID thr_id;
   Process::ptr proc_ptr;
   void *user_data;
};

class proc_exitstate
{
  public:
   Dyninst::PID pid;
   bool exited;
   bool crashed;
   int crash_signal;
   int exit_code;
   err_t last_error;
   const char *last_error_msg;
   void *user_data;

   void setLastError(err_t e_, const char *m) { last_error = e_; last_error_msg = m; }
};

/**
 * int_thread repesents a thread/lwp (PC assumes an M:N model of 1:1).  See the comment
 * above int_process, most of which applies here.
 *
 * An int_process also holds the stopped/running state of a process.  See the StateTracker
 * comment for a longer discussion here.
 **/
class int_thread
{
   friend class Dyninst::ProcControlAPI::Thread;
   friend class int_threadPool;
   friend class ProcStopEventManager;
   friend class hw_breakpoint;
 protected:
   int_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);
   static int_thread *createThreadPlat(int_process *proc,
                                       Dyninst::THR_ID thr_id,
                                       Dyninst::LWP lwp_id,
                                       bool initial_thrd);

public:
   enum attach_status_t {
      as_unknown = 0,          //Threads found by getThreadLWPs come in as_needs_attach, others
      as_created_attached,     // come int as_created_attached.  Up to platforms to interpret this
      as_needs_attach          // however they want.
   };

   static int_thread *createThread(int_process *proc,
                                   Dyninst::THR_ID thr_id,
                                   Dyninst::LWP lwp_id,
                                   bool initial_thrd,
                                   attach_status_t astatus = as_unknown);
   static int_thread *createRPCThread(int_process *p);
   Process::ptr proc() const;
   int_process *llproc() const;

   Dyninst::LWP getLWP() const;
   void changeLWP(Dyninst::LWP new_lwp);

#define RUNNING_STATE(S) (S == int_thread::running || S == int_thread::neonatal_intermediate)
   typedef enum {
      none=0,
      neonatal=1,
      neonatal_intermediate=2,
      running=3,
      stopped=4,
      dontcare=5,
      ditto=6,
      exited=7,
      detached=8,
      errorstate=9
   } State;
   //The order of these is very important.  Lower numbered
   // states take precedence over higher numbered states.
   static const int NumStateIDs = 19;
   static const int NumTargetStateIDs = (NumStateIDs-2); //Handler and Generator states aren't target states

   static const int AsyncStateID            = 0;
   static const int CallbackStateID         = 1;
   static const int PostponedSyscallStateID = 2;
   static const int PendingStopStateID      = 3;
   static const int IRPCStateID             = 4;
   static const int IRPCSetupStateID        = 5;
   static const int IRPCWaitStateID         = 6;
   static const int BreakpointStateID       = 7;
   static const int BreakpointHoldStateID   = 8;
   static const int BreakpointResumeStateID = 9;
   static const int ExitingStateID          = 10;
   static const int InternalStateID         = 11;
   static const int StartupStateID          = 12;
   static const int DetachStateID           = 13;
   static const int UserRPCStateID          = 14;
   static const int ControlAuthorityStateID = 15;
   static const int UserStateID             = 16;
   static const int HandlerStateID          = 17;
   static const int GeneratorStateID        = 18;
   static std::string stateIDToName(int id);

   /**
    * Documentation on StateTracker.  aka How we stop/continue the process:
    *
    * Making a decision on when to stop/continue a thread is complicated,
    * especially when multiple events are happening on multiple threads.
    * We have to consider cases like an iRPC running on one thread, while
    * another thread handles a breakpoint and another thread is being
    * stopped by the user.
    *
    * Each of these events might try to change the stop/running states of
    * other threads, which can lead to conflicts over who's running.
    * We resolve these conflicts by:
    * - Giving each PC subsystem (e.g., iRPCs, breakpoints, user stops, ...) its
    *   variable indicating whether a thread should run.  These are the StateTrackers.
    * - When we acutually decide whether a thread should stop/run, we use a priority-based
    *   projection to reduce the multiple StateTrackers from each subsystem into
    *   a single target state (this happens in int_process::syncRunState).
    *
    * As an example, if you look below you'll see that the IRPC subsystem's
    * StateTrackers are a higher priority than the Breakpoint handling
    * subsystem.  That means that if an IRPC and a breakpoint are happening
    * at the same time, then we'll handle the stop/continues for the IRPC
    * first.  When those are done (e.g., when the iRPC subsystem sets
    * its StateTrackers to the dontcare value), then we'll move on to
    * handle the breakpoints.
    *
    * The UserState is one of the lowest priority StateTrackers--meaning
    * everything else takes precedence.  This state represents the wishes
    * of the user.  You can override the user's wishes (i.e, impose a
    * temporary stop while handling something) by setting a higher priority
    * state, doing your work, then clearing that state.
    *
    * In general, most people will use StateTrackers from Handlers when
    * dealing with an event that might have multiple stages.  In these cases,
    * have the first stage of the event (or the code that starts the events)
    * set the StateTracker to the desired state.  Have the last stage of the
    * event clear the StateTracker.  Events that can be handled with a single
    * stage (e.g, forwarding a signal) don't usually need a StateTracker.
    *
    * The HandlerState and GeneratorState are two special case StateTrackers.
    * Instead of representing a goal state, they represent the actual
    * stop/running state of a thread.  The GenratorState is the state
    * as viewed by PC's generator thread, and similar HandlerState is for
    * PC's handler thread.  These are seperate variables so that we don't
    * get races if they both read/update that variable at once.  Most of the
    * time, you'll want the HandlerState.
    **/
   class StateTracker {
     protected:
      State state;
      int id;
      int sync_level;
      int_thread *up_thr;
     public:
      StateTracker(int_thread *t, int id, int_thread::State initial);

      void desyncState(State ns = int_thread::none);
      void desyncStateProc(State ns = int_thread::none);

      bool setState(State ns = int_thread::none);
      bool setStateProc(State ns = int_thread::none);

      void restoreState();
      void restoreStateProc();
      State getState() const;
      bool isDesynced() const;

      std::string getName() const;
      int getID() const;

      int_thread *debugthr() const { return up_thr; }
   };

   //State management, see above comment on states
   StateTracker &getPostponedSyscallState();
   StateTracker &getExitingState();
   StateTracker &getStartupState();
   StateTracker &getBreakpointState();
   StateTracker &getBreakpointResumeState();
   StateTracker &getBreakpointHoldState();
   StateTracker &getCallbackState();
   StateTracker &getIRPCState();
   StateTracker &getIRPCSetupState();
   StateTracker &getIRPCWaitState();
   StateTracker &getAsyncState();
   StateTracker &getInternalState();
   StateTracker &getDetachState();
   StateTracker &getControlAuthorityState();
   StateTracker &getUserRPCState();
   StateTracker &getUserState();
   StateTracker &getHandlerState();
   StateTracker &getGeneratorState();
   StateTracker &getPendingStopState();

   StateTracker &getStateByID(int id);
   StateTracker &getActiveState();
   static char stateLetter(State s);

   Counter &handlerRunningThreadsCount();
   Counter &generatorRunningThreadsCount();
   Counter &syncRPCCount();
   Counter &runningSyncRPCThreadCount();
   Counter &pendingStopsCount();
   Counter &clearingBPCount();
   Counter &procStopRPCCount();
   Counter &getGeneratorNonExitedThreadCount();
   Counter &neonatalThreadCount();
   Counter &pendingStackwalkCount();

   //Process control
   bool intStop();
   bool intCont();
   async_ret_t handleSingleStepContinue();

   void terminate();

   void setContSignal(int sig);
   int getContSignal();

   virtual bool plat_cont() = 0;
   virtual bool plat_stop() = 0;
   void setPendingStop(bool b);
   bool hasPendingStop() const;

   bool wasRunningWhenAttached() const;
   void setRunningWhenAttached(bool b);
   bool isStopped(int state_id);

   // Is this thread's lifetime only an IRPC and it gets
   // discarded afterwards?
   virtual bool isRPCEphemeral() const { return false; }

   //Single-step
   bool singleStepMode() const;
   void setSingleStepMode(bool s);
   bool singleStepUserMode() const;
   void setSingleStepUserMode(bool s);
   bool singleStep() const;
   void markClearingBreakpoint(bp_instance *bp);
   bp_instance *isClearingBreakpoint();
   void markStoppedOnBP(bp_instance *bp);
   bp_instance *isStoppedOnBP();

    // Syscall tracking
   bool syscallUserMode() const;
   void setSyscallUserMode(bool s);
   bool syscallMode() const;
    bool preSyscall();

   // Emulating single steps with breakpoints
   void addEmulatedSingleStep(emulated_singlestep *es);
   void rmEmulatedSingleStep(emulated_singlestep *es);
   emulated_singlestep *getEmulatedSingleStep();

   //RPC Management
   void addPostedRPC(int_iRPC_ptr rpc_);
   rpc_list_t *getPostedRPCs();
   bool hasPostedRPCs();
   void setRunningRPC(int_iRPC_ptr rpc_);
   void clearRunningRPC();
   int_iRPC_ptr runningRPC() const;
   bool saveRegsForRPC(allreg_response::ptr response);
   bool restoreRegsForRPC(bool clear, result_response::ptr response);
   bool hasSavedRPCRegs();
   void incSyncRPCCount();
   void decSyncRPCCount();
   bool hasSyncRPC();
   int_iRPC_ptr nextPostedIRPC() const;

   int_iRPC_ptr hasRunningProcStopperRPC() const;
   virtual bool notAvailableForRPC() {
		return false;
   }

   //Register Management
   bool getAllRegisters(allreg_response::ptr result);
   bool getRegister(Dyninst::MachRegister reg, reg_response::ptr result);
   bool setAllRegisters(int_registerPool &pool, result_response::ptr result);
   bool setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val, result_response::ptr result);

   virtual bool plat_getAllRegisters(int_registerPool &pool) = 0;
   virtual bool plat_getRegister(Dyninst::MachRegister reg,
                                 Dyninst::MachRegisterVal &val) = 0;
   virtual bool plat_setAllRegisters(int_registerPool &pool) = 0;
   virtual bool plat_setRegister(Dyninst::MachRegister reg,
                                 Dyninst::MachRegisterVal val) = 0;

   virtual bool plat_getAllRegistersAsync(allreg_response::ptr result);
   virtual bool plat_getRegisterAsync(Dyninst::MachRegister reg,
                                      reg_response::ptr result);
   virtual bool plat_setAllRegistersAsync(int_registerPool &pool,
                                          result_response::ptr result);
   virtual bool plat_setRegisterAsync(Dyninst::MachRegister reg,
                                      Dyninst::MachRegisterVal val,
                                      result_response::ptr result);
   virtual bool plat_handle_ghost_thread();
   virtual void plat_terminate();

   void updateRegCache(int_registerPool &pool);
   void updateRegCache(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val);
   void clearRegCache();

   // The exiting property is separate from the main state because an
   // exiting thread can either be running or stopped (depending on the
   // desires of the user).
   bool isExiting() const;
   void setExiting(bool b);
   bool isExitingInGenerator() const;
   void setExitingInGenerator(bool b);

   static void cleanFromHandler(int_thread *thr, bool should_delete);

   //Misc
   virtual bool attach() = 0;
   Thread::ptr thread();

   typedef void(*continue_cb_t)(int_thread *thrd);
   static void addContinueCB(continue_cb_t cb);
   void triggerContinueCBs();

   void throwEventsBeforeContinue();
   virtual bool suppressSanityChecks();

   //User level thread info
   void setTID(Dyninst::THR_ID tid_);
   virtual bool haveUserThreadInfo();
   virtual bool getTID(Dyninst::THR_ID &tid);
   virtual bool getStartFuncAddress(Dyninst::Address &addr);
   virtual bool getStackBase(Dyninst::Address &addr);
   virtual bool getStackSize(unsigned long &size);
   virtual bool getTLSPtr(Dyninst::Address &addr);
   virtual Dyninst::Address getThreadInfoBlockAddr();

   // Windows-only; default implementation is "yes, we're a user thread"
   virtual bool isUser() const { return true; }

   virtual unsigned hwBPAvail(unsigned mode);
   virtual bool rmHWBreakpoint(hw_breakpoint *bp,
                               bool suspend,
                               std::set<response::ptr> &resps,
                               bool &done);
   virtual bool addHWBreakpoint(hw_breakpoint *bp,
                                bool resume,
                                std::set<response::ptr> &resps,
                                bool &done);
   virtual EventBreakpoint::ptr decodeHWBreakpoint(response::ptr &resp,
                                                   bool have_reg = false,
                                                   Dyninst::MachRegisterVal regval = 0);
   virtual bool bpNeedsClear(hw_breakpoint *hwbp);

   virtual ~int_thread();
   static const char *stateStr(int_thread::State s);

   State getTargetState() const;
   void setTargetState(State s);

   void setSuspended(bool b);
   bool isSuspended() const;
   void setLastError(err_t ec, const char *es);

   hw_breakpoint *getHWBreakpoint(Address addr);

 protected:
   Dyninst::THR_ID tid;
   Dyninst::LWP lwp;
   int_process *proc_;
   Thread::ptr up_thread;
   int continueSig_;
   attach_status_t attach_status;

   Counter handler_running_thrd_count;
   Counter generator_running_thrd_count;
   Counter sync_rpc_count;
   Counter sync_rpc_running_thr_count;
   Counter pending_stop;
   Counter clearing_bp_count;
   Counter proc_stop_rpc_count;
   Counter generator_nonexited_thrd_count;
   Counter neonatal_threads;
   Counter pending_stackwalk_count;

   StateTracker postponed_syscall_state;
   StateTracker exiting_state;
   StateTracker startup_state;
   StateTracker pending_stop_state;
   StateTracker callback_state;
   StateTracker breakpoint_state;
   StateTracker breakpoint_hold_state;
   StateTracker breakpoint_resume_state;
   StateTracker irpc_setup_state;
   StateTracker irpc_wait_state;
   StateTracker irpc_state;
   StateTracker async_state;
   StateTracker internal_state;
   StateTracker detach_state;
   StateTracker user_irpc_state;
   StateTracker control_authority_state;
   StateTracker user_state;
   StateTracker handler_state;
   StateTracker generator_state;
   StateTracker *all_states[NumStateIDs];

   State target_state;
   State saved_user_state;

   int_registerPool cached_regpool;
   Mutex<true> regpool_lock;
   int_iRPC_ptr running_rpc;
   int_iRPC_ptr writing_rpc;
   rpc_list_t posted_rpcs;
   int_registerPool rpc_regs;

   bool user_single_step;
   bool single_step;
   bool handler_exiting_state;
   bool generator_exiting_state;
   bool running_when_attached;
   bool suspended;

    bool user_syscall;
    bool next_syscall_is_exit;

   Address stopped_on_breakpoint_addr;
   Address postponed_stopped_on_breakpoint_addr;

   bp_instance *clearing_breakpoint;
   emulated_singlestep *em_singlestep;
   void *user_data;

   std::set<hw_breakpoint *> hw_breakpoints;
   static std::set<continue_cb_t> continue_cbs;
   CallStackUnwinding *unwinder;
 public:
   Address addr_fakeSyscallExitBp;
   bool isSet_fakeSyscallExitBp;
   Breakpoint::ptr BPptr_fakeSyscallExitBp;
};

/**
 * int_threadPool reprents a collection of threads.  Each int_process has one
 * int_threadPool, which has multiple threads.
 **/
class int_threadPool {
   friend class Dyninst::ProcControlAPI::ThreadPool;
   friend class Dyninst::ProcControlAPI::ThreadPool::iterator;
   friend class int_thread;
 private:
   std::vector<int_thread *> threads;
   std::vector<Thread::ptr> hl_threads;
   std::map<Dyninst::LWP, int_thread *> thrds_by_lwp;

   mutable int_thread *initial_thread; // may be updated by side effect on Windows
   int_process *proc_;
   ThreadPool *up_pool;
   bool had_multiple_threads;
 public:
   int_threadPool(int_process *p);
   ~int_threadPool();

   void setInitialThread(int_thread *thrd);
   void addThread(int_thread *thrd);
   void rmThread(int_thread *thrd);
   void noteUpdatedLWP(int_thread *thrd);
   void clear();
   bool hadMultipleThreads() const;

   typedef std::vector<int_thread *>::iterator iterator;
   iterator begin() { return threads.begin(); }
   iterator end() { return threads.end(); }
   bool empty() { return threads.empty(); }

   unsigned size() const;

   int_process *proc() const;
   ThreadPool *pool() const;

   int_thread *findThreadByLWP(Dyninst::LWP lwp);
   int_thread *initialThread() const;
   bool allHandlerStopped();
   bool allStopped(int state_id);

   void saveUserState(Event::ptr ev);
   void restoreUserState();
};

/**
 * Represents a Dynamic Shared Object (aka DSO, aka .dll/.so) loaded by the process.
 * Each DSO has a library name and load address.
 *
 * int_library doesn't hang directly from a process, but from its mem_state object.
 **/
class int_library
{
   friend class Dyninst::ProcControlAPI::LibraryPool;
   friend class Dyninst::ProcControlAPI::LibraryPool::iterator;
   friend class Dyninst::ProcControlAPI::LibraryPool::const_iterator;
   friend class mem_state;
  private:
   std::string name;
   std::string abs_name;
   Dyninst::Address load_address;
   Dyninst::Address data_load_address;
   Dyninst::Address dynamic_address;
   Dyninst::Address sysv_map_address;
   bool has_data_load;
   bool marked;
   void *user_data;
   Library::ptr up_lib;
   bool is_shared_lib;
   mem_state::ptr memory;
  public:
   int_library(std::string n,
               bool shared_lib,
               Dyninst::Address load_addr,
               Dyninst::Address dynamic_load_addr,
               Dyninst::Address data_load_addr = 0,
               bool has_data_load_addr = false);
   int_library(int_library *l);
   ~int_library();
   std::string getName();
   std::string getAbsName();
   Dyninst::Address getAddr();
   Dyninst::Address getDataAddr();
   Dyninst::Address getDynamicAddr();
   bool hasDataAddr();

   void setMark(bool b);
   bool isMarked() const;

   void setUserData(void *d);
   void *getUserData();

   bool isSharedLib() const;

   Library::ptr getUpPtr() const;
   void markAsCleanable();
   void setLoadAddress(Address addr);
   void setDynamicAddress(Address addr);

   Address mapAddress();
   void setMapAddress(Address a);
   void markAOut() { is_shared_lib = false; }

   bool inProcess(int_process *proc);
};

/**
 * There are five (FIVE!) classes related to breakpoints:
 *
 * Breakpoint - The user interface to breakpoints.  The user will always
 *  use this when handling breakpoints.
 *
 * int_breakpoint - The internal handle for a breakpoint.  A Breakpoint
 *  will always have one int_breakpoint.  However, internal breakpoints
 *  (ie, the breakpoint used in System V library loading) don't necessary
 *  have the UI interface object of Breakpoint.
 *
 *  int_breakpoint's aren't process specific (so they can be copied easily)
 *  upon fork.  A single int_breakpoint can be inserted into multiple
 *  processes, and multiple times into one int_process.
 *
 *  An int_breakpoint can have properties, like a control transfer
 *  int_breakpoint will transfer control when it executes, a
 *  onetime breakpoint will clean itself after being hit, and
 *  a thread-specific breakpoint will only trigger if hit by
 *  certain threads.
 *
 *  If internal code wants to keep a handle to a breakpoint, then
 *  it should use int_breakpoint.
 *
 * bp_instance - Each int_breakpoint/process/address triple is
 *  represented by a bp_instance.  This reprents an actual
 *  low-level breakpoint at some location.  Unless you're
 *  writing low-level BP code you can ignore this class.
 *
 *  bp_instance is an abstract class, implemented by sw_breakpoint
 *  and hw_breakpoint.
 *
 * sw_breakpoint is a type of bp_instance, as implemented by a
 *  trap instruction.  A certain sequence of bytes is written
 *  into the process at a code location, which throws a SIGTRAP
 *  (or similar) when executed.
 *
 * hw_breakpoint is a type of bp_instance, as implemented by
 *  hardware debug registers.  These are usually used to implement
 *  things like watchpoints in debuggers.  They are usually
 *  thread-specific and can set to trigger when code executes
 *  or data is read or written.
 **/
class int_breakpoint
{
   friend class sw_breakpoint;
   friend class hw_breakpoint;
 private:
   Breakpoint::weak_ptr up_bp;
   Dyninst::Address to;
   bool isCtrlTransfer_;
   void *data;

   bool hw;
   unsigned int hw_perms;
   unsigned int hw_size;
   bool onetime_bp;
   bool onetime_bp_hit;
   bool procstopper;
   bool suppress_callbacks;
   bool offset_transfer;
   std::set<Thread::const_ptr> thread_specific;
 public:
   int_breakpoint(Breakpoint::ptr up);
   int_breakpoint(Dyninst::Address to, Breakpoint::ptr up, bool off);
   int_breakpoint(unsigned int hw_prems_, unsigned int hw_size_, Breakpoint::ptr up);
   ~int_breakpoint();

   bool isCtrlTransfer() const;
   Dyninst::Address toAddr() const;
   Dyninst::Address getAddress(int_process *p) const;
   void *getData() const;
   void setData(void *v);
   void setOneTimeBreakpoint(bool b);
   void markOneTimeHit();
   bool isOneTimeBreakpoint() const;
   bool isOneTimeBreakpointHit() const;

   void setThreadSpecific(Thread::const_ptr p);
   bool isThreadSpecific() const;
   bool isThreadSpecificTo(Thread::const_ptr p) const;

   void setProcessStopper(bool b);
   bool isProcessStopper() const;

   void setSuppressCallbacks(bool);
   bool suppressCallbacks(void) const;

   bool isHW() const;
   unsigned getHWSize() const;
   unsigned getHWPerms() const;

   bool isOffsetTransfer() const;
   Breakpoint::weak_ptr upBreakpoint() const;
};

class bp_instance
{
   friend class Dyninst::ProcControlAPI::EventBreakpoint;
  protected:
   std::set<int_breakpoint *> bps;
   std::set<Breakpoint::ptr> hl_bps;
   Dyninst::Address addr;
   bool installed;
   int suspend_count;

   sw_breakpoint *swbp;
   hw_breakpoint *hwbp;

   bool suspend_common();
   bool resume_common();
  public:
   virtual bool checkBreakpoint(int_breakpoint *bp, int_process *proc);
   virtual bool rmBreakpoint(int_process *proc, int_breakpoint *bp,
                             bool &empty, std::set<response::ptr> &resps);
   virtual async_ret_t uninstall(int_process *proc, std::set<response::ptr> &resps) = 0;

   Address getAddr() const;

   bp_instance(Address addr);
   bp_instance(const bp_instance *ip);

   typedef std::set<int_breakpoint *>::iterator iterator;
   iterator begin();
   iterator end();

   bool containsIntBreakpoint(int_breakpoint *bp);
   int_breakpoint *getCtrlTransferBP(int_thread *thread);

   bool isInstalled() const;
   virtual bool needsClear() = 0;

   virtual async_ret_t suspend(int_process *proc, std::set<response::ptr> &resps) = 0;
   virtual async_ret_t resume(int_process *proc, std::set<response::ptr> &resps) = 0;

   sw_breakpoint *swBP();
   hw_breakpoint *hwBP();
   virtual ~bp_instance();
};


//At least as large as any arch's trap instruction
#define BP_BUFFER_SIZE 8
//Long breakpoints can be used to artifically increase the size of the BP write,
// which fools the BG breakpoint interception code that looks for 4 byte writes.
#define BP_LONG_SIZE 4
class sw_breakpoint : public bp_instance
{
   friend class Dyninst::ProcControlAPI::EventBreakpoint;
   friend class int_process;
 private:
   mem_state::ptr memory;

   char buffer[BP_BUFFER_SIZE];
   int buffer_size;
   bool prepped;
   bool long_breakpoint;

   result_response::ptr write_response;
   mem_response::ptr read_response;

   bool writeBreakpoint(int_process *proc, result_response::ptr write_response);
   bool saveBreakpointData(int_process *proc, mem_response::ptr read_response);
   bool restoreBreakpointData(int_process *proc, result_response::ptr res_resp);

   sw_breakpoint(mem_state::ptr memory_, Dyninst::Address addr_);
 public:
   sw_breakpoint(mem_state::ptr memory_, const sw_breakpoint *ip);
   ~sw_breakpoint();

   static sw_breakpoint *create(int_process *proc, int_breakpoint *bp, Dyninst::Address addr_);
   //Use these three functions to add a breakpoint
   bool prepBreakpoint(int_process *proc, mem_response::ptr mem_resp);
   bool insertBreakpoint(int_process *proc, result_response::ptr res_resp);
   bool addToIntBreakpoint(int_breakpoint *bp, int_process *proc);

   virtual async_ret_t uninstall(int_process *proc, std::set<response::ptr> &resps);
   virtual async_ret_t suspend(int_process *proc, std::set<response::ptr> &resps);
   virtual async_ret_t resume(int_process *proc, std::set<response::ptr> &resps);

   unsigned getNumIntBreakpoints() const;
   virtual bool needsClear();
};

class hw_breakpoint : public bp_instance {
  friend class Dyninst::ProcControlAPI::EventBreakpoint;
private:
  unsigned int hw_perms;
  unsigned int hw_size;
  bool proc_wide;
  int_thread *thr;
  bool error;

  hw_breakpoint(int_thread *thr, unsigned mode, unsigned size,
                bool pwide, Dyninst::Address addr);
public:
  virtual async_ret_t uninstall(int_process *proc, std::set<response::ptr> &resps);


  static hw_breakpoint *create(int_process *proc, int_breakpoint *bp, Dyninst::Address addr_);
  ~hw_breakpoint();

  bool install(bool &done, std::set<response::ptr> &resps);
  unsigned int getPerms() const;
  unsigned int getSize() const;
  bool procWide() const;
  int_thread *getThread() const;
  virtual bool needsClear();

  virtual async_ret_t suspend(int_process *proc, std::set<response::ptr> &resps);
  virtual async_ret_t resume(int_process *proc, std::set<response::ptr> &resps);
};

/**
 * On PPC64 certain synchronization instructions can mis-behave if we
 * try to single-step across them.  This class recognizes these situations
 * and replaces a single-step operation with a breakpoint insertion/run over
 * the offending code.
 **/
class emulated_singlestep {
   // Breakpoints that are added and removed in a group to emulate
   // a single step with breakpoints
  private:
   bool saved_user_single_step;
   bool saved_single_step;
   int_breakpoint *bp;
   int_thread *thr;
   std::set<Address> addrs;

  public:
   emulated_singlestep(int_thread *thr);
   ~emulated_singlestep();

   bool containsBreakpoint(Address addr) const;
   async_ret_t add(Address addr);
   async_ret_t clear();
   void restoreSSMode();

   std::set<response::ptr> clear_resps;
};

#if defined(arch_aarch64)
/**
 * For aarch64 linux: 3.17.4-302.fc21.aarch64
 * there is a kernel bug on ptrace.
 * On the fast path, during the mid-syscall process, if the PTRACE
 * flags are changed, before exiting the syscall, it never checks flags
 * again. Hence, the syscall_exit_stop signal is not generated as on
 * other platform as we expected. The solution to this is that we insert
 * a breakpoint on the syscall's next instruction. And recognize it as
 * the exit stop. And check return values of syscall.
 */
/*
class syscall_exit_breakpoints{
  public:
    syscall_exit_breakpoints();
    ~syscall_exit_breakpoints();

    bool contains_breakpoints(Address addr) const;
    bool is_enabled() const;
    bool add(Address addr);

  private:
    bool enabled;
    std::set<Address> addrs;
    int_breakpoint *bp;
};
*/
#endif

struct clearError {
   void operator()(Process::ptr p) {
      p->clearLastError();
   }

   template <class T>
   void operator()(const std::pair<T, Process::const_ptr> &v) {
      v.second->clearLastError();
   }
   template <class T>
   void operator()(const std::pair<Process::const_ptr, T> &v) {
      v.first->clearLastError();
   }
   template <class T>
   void operator()(const std::pair<T, Process::ptr> &v) {
      v.second->clearLastError();
   }
   template <class T>
   void operator()(const std::pair<Process::ptr, T> &v) {
      v.first->clearLastError();
   }
};

struct setError {
private:
   err_t err;
   const char *err_str;
public:
   setError(err_t e, const char *s) { err = e; err_str = s; }
   void operator()(Process::ptr p) {
      p->setLastError(err, err_str);
   }
   void operator()(const std::pair<Address, Process::ptr> &v) {
      v.second->setLastError(err, err_str);
   }
};

/**
 * The notify class is the internal interface to th UI Notify class.
 * It is used to signal the user (via platform-specific interfaces)
 * that an event is ready to be handled.
 **/
class int_notify {
#if defined(os_windows)
	class windows_details
	{
		HANDLE evt;
	public:
		windows_details() : evt(INVALID_HANDLE_VALUE)
		{
		}
		typedef HANDLE wait_object_t;
		void noteEvent()
		{
			::ReleaseSemaphore(evt, 1, NULL);
		}
		void clearEvent()
		{
			// No-op with semaphores
			//::ResetEvent(evt);
		}

		bool createInternals()
		{
			evt = ::CreateSemaphore(NULL, 0, 1000, NULL);
			return evt  != INVALID_HANDLE_VALUE;
		}
		bool internalsValid()
		{
			return evt != INVALID_HANDLE_VALUE;
		}
		wait_object_t getWaitObject()
		{
			return evt;
		}
	};
	typedef windows_details details_t;
#else
	class unix_details
	{
		friend class int_notify;
		int pipe_in;
		int pipe_out;
		void writeToPipe();
		void readFromPipe();
	public:
		unix_details();
		typedef int wait_object_t;
		void noteEvent();
		void clearEvent();
		bool createInternals();
		bool internalsValid();
		wait_object_t getWaitObject();
	};
	typedef unix_details details_t;
#endif

	typedef details_t::wait_object_t wait_object_t;
   friend int_notify *notify();
   friend EventNotify *Dyninst::ProcControlAPI::evNotify();
 private:
   static int_notify *the_notify;
   EventNotify up_notify;
   std::set<EventNotify::notify_cb_t> cbs;
   int events_noted;
   details_t my_internals;
 public:
   int_notify();
   void noteEvent();
   void clearEvent();

   void registerCB(EventNotify::notify_cb_t cb);
   void removeCB(EventNotify::notify_cb_t cb);
   bool hasEvents();
   details_t::wait_object_t getWaitable();
};
int_notify *notify();

extern void setGeneratorThread(long t);
void setHandlerThread(long t);
bool isGeneratorThread();
bool isHandlerThread();
bool isUserThread();
HandlerPool *createDefaultHandlerPool(int_process *p);
HandlerPool *plat_createDefaultHandlerPool(HandlerPool *hpool);

class MTManager {
  friend MTManager *mt();
  friend class MTLock;
private:
  static MTManager *mt_;
  DThread evhandler_thread;
  CondVar<> pending_event_lock;
  Mutex < true > work_lock;
  bool have_queued_events;
  bool is_running;
  bool should_exit;
  Process::thread_mode_t threadMode;

  void evhandler_main();
#if defined(os_windows)
  static unsigned long WINAPI evhandler_main_wrapper(void *);
#else
  static void evhandler_main_wrapper(void *);
#endif
  void eventqueue_cb();

public:
  static const Process::thread_mode_t default_thread_mode = Process::HandlerThreading;
  MTManager();
  ~MTManager();

  void run();
  void stop();

  void startWork();
  void endWork();

  bool handlerThreading();
  Process::thread_mode_t getThreadMode();
  bool setThreadMode(Process::thread_mode_t tm, bool init = false);

  static void eventqueue_cb_wrapper();
};

inline MTManager *mt() {
   return MTManager::mt_;
}

class MTLock
{
 private:
   bool should_unlock;
 public:
   typedef enum {allow_init} initialize;
   typedef enum {allow_generator} generator;
   typedef enum {nocb, deliver_callbacks} callbacks;
   MTLock(initialize, callbacks c = nocb)
   {
      should_unlock = true;
      if (!MTManager::mt_) {
         MTManager::mt_ = new MTManager();
         if ((MTManager::default_thread_mode == Process::HandlerThreading) ||
             (MTManager::default_thread_mode == Process::CallbackThreading)) {
            mt()->startWork();
         }
         mt()->setThreadMode(MTManager::default_thread_mode, true);
      }
      else if (mt()->handlerThreading()) {
         mt()->startWork();
         if ((c == deliver_callbacks) &&
             (MTManager::default_thread_mode == Process::HandlerThreading) &&
             notify()->hasEvents())
         {
            pthrd_printf("MTLock triggered event handling\n");
            int_process::waitAndHandleEvents(false);
            pthrd_printf("MTLock triggered event handling finished\n");
         }
      }
   }

   MTLock(generator)
   {
      if (isGeneratorThread()) {
         should_unlock = false;
         return;
      }
      should_unlock = true;
      if (mt()->handlerThreading()) {
         mt()->startWork();
      }
   }

   MTLock(callbacks)
   {
      assert(!isGeneratorThread());
      should_unlock = true;
      if (mt()->handlerThreading()) {
         mt()->startWork();
         if (notify()->hasEvents() &&
             (MTManager::default_thread_mode == Process::HandlerThreading) )
         {
            pthrd_printf("MTLock triggered event handling\n");
            int_process::waitAndHandleEvents(false);
            pthrd_printf("MTLock triggered event handling finished\n");
         }
      }
   }

   MTLock()
   {
      assert(!isGeneratorThread());
      should_unlock = true;
      if (mt()->handlerThreading())
         mt()->startWork();
   }

   ~MTLock() {
      if (should_unlock && mt()->handlerThreading())
         mt()->endWork();
   }
};

// A class to stop the various threads that have been started when
// the library is deinitialized
class int_cleanup {
    public:
        ~int_cleanup();
};

#define PROC_EXIT_TEST(STR, RET)                      \
   if (!llproc_) {                                    \
     perr_printf(STR " on exited process\n");         \
     setLastError(err_exited, "Process is exited");   \
     return RET;                                      \
   }

#define PROC_DETACH_TEST(STR, RET)                       \
   if (llproc_->getState() == int_process::detached) {   \
     perr_printf(STR " on detached process\n");          \
     setLastError(err_detached, "Process is detached");  \
     return RET;                                         \
   }

#define PROC_CB_TEST(STR, RET)                                          \
   if (int_process::isInCB()) {                                         \
     perr_printf(STR " while in callback\n");                           \
     setLastError(err_incallback, "Cannot do operation from callback"); \
     return RET;                                                        \
   }

#define PROC_EXIT_DETACH_TEST(STR, RET)         \
   PROC_EXIT_TEST(STR, RET)                     \
   PROC_DETACH_TEST(STR, RET)

#define PROC_EXIT_DETACH_CB_TEST(STR, RET)      \
   PROC_EXIT_TEST(STR, RET)                     \
   PROC_DETACH_TEST(STR, RET)                   \
   PROC_CB_TEST(STR, RET)

#define THREAD_EXIT_TEST(STR, RET)                    \
   if (!llthread_) {                                  \
     perr_printf(STR " on exited thread\n");          \
     setLastError(err_exited, "Thread is exited");    \
     return RET;                                      \
   }                                                  \
   if (!llthread_->llproc()) {                        \
     perr_printf(STR " on exited process\n");         \
     setLastError(err_exited, "Process is exited");   \
     return RET;                                      \
   }

#define THREAD_DETACH_TEST(STR, RET)                                    \
   if (llthread_->llproc()->getState() == int_process::detached) {      \
     perr_printf(STR " on detached process\n");                         \
     setLastError(err_detached, "Process is detached");                 \
     return RET;                                                        \
   }                                                                    \

#define THREAD_STOP_TEST(STR, RET)                                     \
   if (llthread_->getUserState().getState() != int_thread::stopped) {  \
      setLastError(err_notstopped, "Thread not stopped");              \
      perr_printf(STR " on running thread %d\n", llthread_->getLWP()); \
      return RET;                                                      \
   }

#define THREAD_EXIT_DETACH_TEST(STR, RET)         \
   THREAD_EXIT_TEST(STR, RET)                     \
   THREAD_DETACH_TEST(STR, RET)

#define THREAD_EXIT_DETACH_CB_TEST(STR, RET)      \
   THREAD_EXIT_TEST(STR, RET)                     \
   THREAD_DETACH_TEST(STR, RET)                   \
   PROC_CB_TEST(STR, RET)

#define THREAD_EXIT_DETACH_STOP_TEST(STR, RET)    \
   THREAD_EXIT_TEST(STR, RET)                     \
   THREAD_DETACH_TEST(STR, RET)                   \
   THREAD_STOP_TEST(STR, RET)

#define PTR_EXIT_TEST(P, STR, RET)                       \
   if (!P || !P->llproc()) {                             \
      perr_printf(STR " on exited process\n");           \
      P->setLastError(err_exited, "Process is exited");  \
      return RET;                                        \
   }

#define TRUTH_TEST(P, STR, RET)                                 \
   if (!(P)) {                                                  \
      perr_printf(STR " parameter is invalid\n");               \
      setLastError(err_badparam, STR " paramter is invalid\n"); \
      return RET;                                               \
   }

#endif
