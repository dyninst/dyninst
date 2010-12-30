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

#if !defined(INT_PROCESS_H_)
#define INT_PROCESS_H_

#include "proccontrol/h/Process.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/h/Event.h"

#include "proccontrol/src/response.h"

#include "dynutil/h/dyn_regs.h"
#include "dynutil/h/SymReader.h"
#include "common/h/dthread.h"

#include <vector>
#include <map>
#include <list>
#include <set>
#include <utility>
#include <queue>
#include <stack>

using namespace Dyninst;
using namespace ProcControlAPI;

class int_thread;
class int_threadPool;
class handlerpool;
class int_iRPC;

typedef dyn_detail::boost::shared_ptr<int_iRPC> int_iRPC_ptr;
typedef std::map<Dyninst::MachRegister, std::pair<unsigned int, unsigned int> > dynreg_to_user_t;

typedef std::list<int_iRPC_ptr> rpc_list_t;

class installed_breakpoint;
class int_library;
class int_process;

class mem_state
{
  public:
   typedef mem_state* ptr;
   mem_state(int_process *proc);
   mem_state(mem_state &m, int_process *proc);
   ~mem_state();

   void addProc(int_process *p);
   void rmProc(int_process *p, bool &should_clean);

   std::set<int_process *> procs;
   std::set<int_library *> libs;
   std::map<Dyninst::Address, installed_breakpoint *> breakpoints;
   std::map<Dyninst::Address, unsigned long> inf_malloced_memory;
};

class int_process
{
   friend class Dyninst::ProcControlAPI::Process;
 protected:
   int_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::map<int,int> f);
   int_process(Dyninst::PID pid_, int_process *p);
 public:
   static int_process *createProcess(Dyninst::PID p, std::string e);
   static int_process *createProcess(std::string e, std::vector<std::string> a, std::map<int,int> f);
   static int_process *createProcess(Dyninst::PID pid_, int_process *p);
   virtual ~int_process();
 protected:
   bool create();
   virtual bool plat_create() = 0;
   virtual bool post_create();

   bool attach();
   virtual bool plat_attach() = 0;
   bool attachThreads();
   virtual bool post_attach();

  public:
   void setContSignal(int sig);
   int getContSignal() const;
   bool continueProcess();
   virtual bool plat_contProcess() = 0;

   bool forked();
  protected:
   virtual bool plat_forked() = 0;
   virtual bool post_forked();

  public:
   bool execed();
  protected:
   virtual bool plat_execed() = 0;
   virtual bool plat_detach() = 0;
   virtual bool plat_terminate(bool &needs_sync) = 0;

   virtual bool needIndividualThreadAttach() = 0;
   virtual bool getThreadLWPs(std::vector<Dyninst::LWP> &lwps);

   static bool multi_attach(std::vector<int_process *> &pids);
   bool waitfor_startup();

   void setPid(Dyninst::PID pid);
   int_thread *findStoppedThread();

  public:
   typedef enum {
      neonatal = 0,
      neonatal_intermediate,
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

   bool detach(bool &should_clean);
   bool terminate(bool &needs_sync);
   void updateSyncState(Event::ptr ev, bool gen);
   virtual Dyninst::Architecture getTargetArch() = 0;
   virtual unsigned getTargetPageSize() = 0;
   virtual Dyninst::Address mallocExecMemory(unsigned size);
   virtual Dyninst::Address plat_mallocExecMemory(Dyninst::Address min, unsigned size) = 0;
   virtual void freeExecMemory(Dyninst::Address addr);

   static bool waitAndHandleEvents(bool block);
   static bool waitAndHandleForProc(bool block, int_process *proc, bool &proc_exited);
   static bool waitForAsyncEvent(response::ptr resp);
   static bool waitForAsyncEvent(std::set<response::ptr> resp);

   static const char *stateName(State s);
   void initializeProcess(Process::ptr p);

   void setExitCode(int c);
   void setCrashSignal(int s);
   bool getExitCode(int &c);
   bool getCrashSignal(int &s);
   bool wasForcedTerminated() const;

   virtual bool plat_individualRegAccess() = 0;

   void addProcStopper(Event::ptr ev);
   Event::ptr removeProcStopper();
   bool hasQueuedProcStoppers() const;

   int getAddressWidth();
   HandlerPool *handlerPool() const;

   bool addBreakpoint(Dyninst::Address addr, int_breakpoint *bp);
   bool rmBreakpoint(Dyninst::Address addr, int_breakpoint *bp, result_response::ptr async_resp);
   installed_breakpoint *getBreakpoint(Dyninst::Address addr);

   virtual unsigned plat_breakpointSize() = 0;
   virtual void plat_breakpointBytes(char *buffer) = 0;

   virtual bool plat_createDeallocationSnippet(Dyninst::Address addr, unsigned long size, void* &buffer, 
                                               unsigned long &buffer_size, unsigned long &start_offset) = 0;
   virtual bool plat_createAllocationSnippet(Dyninst::Address addr, bool use_addr, unsigned long size, 
                                             void* &buffer, unsigned long &buffer_size, 
                                             unsigned long &start_offset) = 0;
   virtual bool plat_collectAllocationResult(int_thread *thr, reg_response::ptr resp) = 0;

   virtual SymbolReaderFactory *plat_defaultSymReader();
   Dyninst::Address infMalloc(unsigned long size, bool use_addr = false, Dyninst::Address addr = 0x0);
   bool infFree(Dyninst::Address addr);

   bool readMem(Dyninst::Address remote, mem_response::ptr result, int_thread *thr = NULL);
   bool writeMem(void *local, Dyninst::Address remote, size_t size, result_response::ptr result, int_thread *thr = NULL);

   virtual bool plat_readMem(int_thread *thr, void *local, 
                             Dyninst::Address remote, size_t size) = 0;
   virtual bool plat_writeMem(int_thread *thr, void *local, 
                              Dyninst::Address remote, size_t size) = 0;

   //For a platform, if plat_needsAsyncIO returns true then the async
   // set of functions need to be implemented.  Currently needsAsyncIO_plat 
   // only returns true for bluegene family.  By default these are otherwise
   // unimplemented.
   virtual bool plat_needsAsyncIO() const;
   virtual bool plat_readMemAsync(int_thread *thr, Dyninst::Address addr, 
                                  mem_response::ptr result);
   virtual bool plat_writeMemAsync(int_thread *thr, void *local, Dyninst::Address addr,
                                   size_t size, result_response::ptr result);
   
   typedef enum {
       NoLWPControl = 0,
       HybridLWPControl, // see below for a description of these modes
       IndependentLWPControl
   } ThreadControlMode;
   static ThreadControlMode getThreadControlMode();
   static bool isInCB();
   static void setInCB(bool b);

   int_library *getLibraryByName(std::string s) const;
   size_t numLibs() const;
   virtual bool refresh_libraries(std::set<int_library *> &added_libs,
                                  std::set<int_library *> &rmd_libs,
                                  std::set<response::ptr> &async_responses) = 0;
   virtual bool initLibraryMechanism() = 0;
   virtual bool plat_isStaticBinary() = 0;

   virtual bool plat_supportLWPEvents() const;
   bool forceGeneratorBlock() const;
   void setForceGeneratorBlock(bool b);

   void setAllowInternalRPCEvents(int_thread *thr);
   EventRPCInternal::ptr getInternalRPCEvent();
   
   std::string getExecutable() const;
   static bool isInCallback();

   static int_process *in_waitHandleProc;
 protected:
   State state;
   Dyninst::PID pid;
   std::string executable;
   std::vector<std::string> argv;
   std::map<int,int> fds;
   Dyninst::Architecture arch;
   int_threadPool *threadpool;
   Process::ptr up_proc;
   HandlerPool *handlerpool;
   LibraryPool libpool;
   bool hasCrashSignal;
   int crashSignal;
   bool hasExitCode;
   bool forceGenerator;
   std::stack<int_thread *> allowInternalRPCEvents;
   bool forcedTermination;
   int exitCode;
   static bool in_callback;
   mem_state::ptr mem;
   std::map<Dyninst::Address, unsigned> exec_mem_cache;
   std::queue<Event::ptr> proc_stoppers;
   int continueSig;
};

/*
 * Thread Control Modes (as defined above)
 *
 * Currently, there are 3 thread control modes: NoLWPControl, HybridLWPControl,
 * and IndependentLWPControl.
 *
 * NoLWPControl is currently unused. This mode implies that no operations can
 * be performed on just a LWP.
 *
 * HybridLWPControl is currently the mode on FreeBSD. This mode implies that
 * operations can be performed on LWPs, but the whole process needs to be
 * stopped before these operations can be performed. Additionally, it implies
 * that threads cannot be continued and stopped; they must be resumed and
 * suspended, and followed by a whole process continue. This means that the
 * plat_suspend, plat_resume, and plat_contProcess functions will be used to
 * implement thread stops and continues.
 *
 * IndependentLWPControl is currently the mode on Linux. This mode implies that
 * operations can be performed on LWPs independent of each other's state.
 */

// For improved readability
bool useHybridLWPControl(int_threadPool *tp);
bool useHybridLWPControl(int_thread *thrd);
bool useHybridLWPControl(int_process *p);
bool useHybridLWPControl();

class int_registerPool
{
 public:
   int_registerPool();
   int_registerPool(const int_registerPool &c);
   ~int_registerPool();
   
   typedef std::map<Dyninst::MachRegister, Dyninst::MachRegisterVal> reg_map_t;
   reg_map_t regs;
   bool full;
   int_thread *thread;

   typedef reg_map_t::iterator iterator;
   typedef reg_map_t::const_iterator const_iterator;
};

class thread_exitstate
{
  public:
   Dyninst::LWP lwp;
   Dyninst::THR_ID thr_id;
   Process::ptr proc_ptr;
};

class proc_exitstate
{
  public:
   Dyninst::PID pid;
   bool exited;
   bool crashed;
   int crash_signal;
   int exit_code;
};

/**
 * ON THREADING STATES:
 *
 * Each thread has four different states, which mostly monitor running/stopped
 * status :
 *   GeneratorState - Thread state as seen by the generator object
 *   HandlerState - Thread state as seen by the handler object
 *   InternalState - Target thread state desired by int_* layer
 *   UserState - Target Thread state as desired by the user
 *
 * The GeneratorState and HandlerState represent an event as it moves through the 
 * system.  For example, an event that stops the thread may first appear
 * in the Generator object, and move the GeneratorState to 'stopped'.  It is then
 * seen by the handler, which moves the HandlerState to 'stopped'.  If the thread is 
 * then continued, the HandlerState and GeneratorStates will go back to 'running'.
 * These are primarily seperated to prevent race conditions with the handler and
 * generators modifying the same variable, since they run in seperate threads.
 * 
 * The InternalState and UserState are used to make policy decisions about whether
 * a thread should be running or stopped.  For example, after handling an event
 * we may check the UserState to see if the user wants us to run/stop a thread.
 * The InternalState usually matches the UserState, but may override it for 
 * stop/run decisions in a few scenarios.  For example, if the user posts a iRPC
 * to a running process (UserState = running) we may have to temporarily stop the
 * process to install the iRPC.  We don't want to change the UserState, since the user
 * still wants the process running, so we set the InternalState to stopped while we
 * set up the iRPC, and then return it to the UserState value when the iRPC is ready.
 *
 * There are a couple of important assertions about the relationship between these thread
 * states :
 *  (GeneratorState == running) implies (HandlerState == running)
 *  (HandlerState == running)  implies (InternalState == running)
 *  (InternalState == stopped)  implies (HandlerState == stopped)
 *  (HandlerState == stopped)  implies (GeneratorState == stopped)
 **/
class int_thread
{
   friend class int_threadPool;
 protected:
   int_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);
   static int_thread *createThreadPlat(int_process *proc, 
                                       Dyninst::THR_ID thr_id, 
                                       Dyninst::LWP lwp_id,
                                       bool initial_thrd);
 public:
   static int_thread *createThread(int_process *proc, 
                                   Dyninst::THR_ID thr_id, 
                                   Dyninst::LWP lwp_id,
                                   bool initial_thrd);
   Process::ptr proc() const;
   int_process *llproc() const;

   Dyninst::LWP getLWP() const;

   typedef enum {
      neonatal,
      neonatal_intermediate,
      running,
      stopped,
      exited,
      errorstate
   } State;

   //State management, see above comment on states
   State getHandlerState() const;
   State getUserState() const;
   State getGeneratorState() const;
   State getInternalState() const;
   bool setHandlerState(State s);
   bool setUserState(State s);
   bool setGeneratorState(State s);
   bool setInternalState(State s);
   void restoreInternalState(bool sync = true);
   void desyncInternalState();

   //Process control
   bool userCont();
   bool userStop();
   bool intStop(bool sync = true);
   bool intCont();

   void setContSignal(int sig);
   int getContSignal();
   bool contWithSignal(int sigOverride = -1);
   virtual bool plat_cont() = 0;
   virtual bool plat_stop() = 0;
   void setPendingUserStop(bool b);
   bool hasPendingUserStop() const;
   void setPendingStop(bool b);
   bool hasPendingStop() const;
   void setResumed(bool b);
   bool isResumed() const;

   // Needed for HybridLWPControl thread control mode
   // These can be no-ops for other modes
   virtual bool plat_suspend() = 0;
   virtual bool plat_resume() = 0;

   //Single-step
   bool singleStepMode() const;
   void setSingleStepMode(bool s);
   bool singleStepUserMode() const;
   void setSingleStepUserMode(bool s);
   bool singleStep() const;   
   void markClearingBreakpoint(installed_breakpoint *bp);
   installed_breakpoint *isClearingBreakpoint();

   //RPC Management
   void addPostedRPC(int_iRPC_ptr rpc_);
   rpc_list_t *getPostedRPCs();
   bool hasPostedRPCs();
   void setRunningRPC(int_iRPC_ptr rpc_);
   void clearRunningRPC();
   int_iRPC_ptr runningRPC() const;
   int_iRPC_ptr writingRPC() const;
   void setWritingRPC(int_iRPC_ptr rpc);
   bool saveRegsForRPC(allreg_response::ptr response);
   bool restoreRegsForRPC(bool clear, result_response::ptr response);
   bool hasSavedRPCRegs();
   bool runningInternalRPC() const;
   void incSyncRPCCount();
   void decSyncRPCCount();
   bool hasSyncRPC();
   int_iRPC_ptr nextPostedIRPC() const;
   int_iRPC_ptr hasRunningProcStopperRPC() const;

   typedef enum {
      hnp_post_async,
      hnp_post_sync
   } hnp_sync_t;
   typedef enum {
      hnp_allow_stop,
      hnp_no_stop
   } hnp_stop_t;

   bool handleNextPostedIRPC(hnp_stop_t allow_stop, bool is_sync);

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

   void updateRegCache(int_registerPool &pool);
   void updateRegCache(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val);

   bool hasPostponedContinue() const;
   void setPostponedContinue(bool b);

   //Misc
   virtual bool attach() = 0;
   Thread::ptr thread();

   //User level thread info
   void setTID(Dyninst::THR_ID tid_);
   virtual bool haveUserThreadInfo() = 0;
   virtual bool getTID(Dyninst::THR_ID &tid) = 0;
   virtual bool getStartFuncAddress(Dyninst::Address &addr) = 0;
   virtual bool getStackBase(Dyninst::Address &addr) = 0;
   virtual bool getStackSize(unsigned long &size) = 0;
   virtual bool getTLSPtr(Dyninst::Address &addr) = 0;
      
   virtual ~int_thread();
   static const char *stateStr(int_thread::State s);
 protected:
   Dyninst::THR_ID tid;
   Dyninst::LWP lwp;
   int_process *proc_;
   Thread::ptr up_thread;
   int continueSig_;
   State handler_state;
   State user_state;
   State generator_state;
   State internal_state;

   int_registerPool cached_regpool;
   Mutex regpool_lock;
   int_iRPC_ptr running_rpc;
   int_iRPC_ptr writing_rpc;
   rpc_list_t posted_rpcs;
   int_registerPool rpc_regs;
   unsigned sync_rpc_count;
   bool pending_user_stop;
   bool pending_stop;
   bool resumed;
   int num_locked_stops;
   bool user_single_step;
   bool single_step;
   bool postponed_continue;
   installed_breakpoint *clearing_breakpoint;


   bool setAnyState(int_thread::State *from, int_thread::State to);

   //Stop/Continue
   typedef enum {
      sc_error,
      sc_success,
      sc_success_pending,
      sc_skip
   } stopcont_ret_t;
   bool stop(bool user_stop, bool sync);
   bool cont(bool user_cont);
   stopcont_ret_t stop(bool user_stop);
   stopcont_ret_t cont(bool user_cont, bool has_proc_lock);
};

class int_threadPool {
   friend class Dyninst::ProcControlAPI::ThreadPool;
   friend class Dyninst::ProcControlAPI::ThreadPool::iterator;
 private:
   std::vector<int_thread *> threads;
   std::vector<Thread::ptr> hl_threads;
   std::map<Dyninst::LWP, int_thread *> thrds_by_lwp;

   int_thread *initial_thread;
   int_process *proc_;
   ThreadPool *up_pool;
 public:
   int_threadPool(int_process *p);
   ~int_threadPool();

   void setInitialThread(int_thread *thrd);
   void addThread(int_thread *thrd);
   void rmThread(int_thread *thrd);
   void restoreInternalState(bool sync);
   void desyncInternalState();
   void clear();

   typedef std::vector<int_thread *>::iterator iterator;
   iterator begin() { return threads.begin(); }
   iterator end() { return threads.end(); }

   unsigned size() const;

   int_process *proc() const;
   ThreadPool *pool() const;

   int_thread *findThreadByLWP(Dyninst::LWP lwp);
   int_thread *initialThread() const;
   bool allStopped();
   
   bool userCont();
   bool userStop();
   bool intStop(bool sync = true);
   bool intCont();
 private:
   bool cont(bool user_cont);
   bool stop(bool user_stop, bool sync);
};

class int_library
{
   friend class Dyninst::ProcControlAPI::LibraryPool;
   friend class Dyninst::ProcControlAPI::LibraryPool::iterator;
   friend class Dyninst::ProcControlAPI::LibraryPool::const_iterator;
  private:
   std::string name;
   Dyninst::Address load_address;
   Dyninst::Address data_load_address;
   bool has_data_load;
   bool marked;
   Library::ptr up_lib;
  public:
   int_library(std::string n, Dyninst::Address load_addr);
   int_library(std::string n, Dyninst::Address load_addr, 
               Dyninst::Address data_load_addr);
   int_library(int_library *l);
   ~int_library();
   std::string getName();
   Dyninst::Address getAddr();
   Dyninst::Address getDataAddr();
   bool hasDataAddr();
   
   void setMark(bool b);
   bool isMarked() const;
   
   Library::ptr getUpPtr() const;
};

class int_breakpoint
{
   friend class installed_breakpoint;
 private:
   Breakpoint::weak_ptr up_bp;
   Dyninst::Address to;
   bool isCtrlTransfer_;
   void *data;
 public:
   int_breakpoint(Breakpoint::ptr up);
   int_breakpoint(Dyninst::Address to, Breakpoint::ptr up);
   ~int_breakpoint();

   bool isCtrlTransfer() const;
   Dyninst::Address toAddr() const;
   Dyninst::Address getAddress(int_process *p) const;
   void *getData() const;
   void setData(void *v);
   Breakpoint::weak_ptr upBreakpoint() const;
};

//At least as large as any arch's trap instruction
#define BP_BUFFER_SIZE 4
class installed_breakpoint
{
   friend class Dyninst::ProcControlAPI::EventBreakpoint;
 private:
   mem_state::ptr memory;
   std::set<int_breakpoint *> bps;
   std::set<Breakpoint::ptr> hl_bps;

   char buffer[BP_BUFFER_SIZE];
   int buffer_size;
   bool prepped;
   bool installed;
   int suspend_count;
   Dyninst::Address addr;

   result_response::ptr write_response;
   mem_response::ptr read_response;

   bool writeBreakpoint(int_process *proc, result_response::ptr write_response);
   bool saveBreakpointData(int_process *proc, mem_response::ptr read_response);
   bool restoreBreakpointData(int_process *proc, result_response::ptr res_resp);


 public:
   installed_breakpoint(mem_state::ptr memory_, Dyninst::Address addr_);
   installed_breakpoint(mem_state::ptr memory_, const installed_breakpoint *ip);
   ~installed_breakpoint();

   //Use these three functions to add a breakpoint
   bool prepBreakpoint(int_process *proc, mem_response::ptr mem_resp);
   bool insertBreakpoint(int_process *proc, result_response::ptr res_resp);
   bool addBreakpoint(int_breakpoint *bp);

   bool rmBreakpoint(int_process *proc, int_breakpoint *bp, bool &empty, result_response::ptr async_resp);
   bool uninstall(int_process *proc, result_response::ptr async_resp);
   bool suspend(int_process *proc, result_response::ptr result_resp);
   bool resume(int_process *proc, result_response::ptr async_resp);

   bool isInstalled() const;
   Dyninst::Address getAddr() const;
};

class int_notify {
   friend int_notify *notify();
   friend EventNotify *Dyninst::ProcControlAPI::evNotify();
 private:
   static int_notify *the_notify;
   EventNotify *up_notify;
   std::set<EventNotify::notify_cb_t> cbs;
   int pipe_in;
   int pipe_out;
   int pipe_count;
   int events_noted;
   void writeToPipe();
   void readFromPipe();
   bool createPipe();
 public:
   int_notify();
   
   void noteEvent();
   void clearEvent();
   void registerCB(EventNotify::notify_cb_t cb);
   void removeCB(EventNotify::notify_cb_t cb);
   bool hasEvents();
   int getPipeIn();
};
int_notify *notify();

extern void setGeneratorThread(long t);
void setHandlerThread(long t);
bool isGeneratorThread();
bool isHandlerThread();
HandlerPool *createDefaultHandlerPool(int_process *p);
HandlerPool *plat_createDefaultHandlerPool(HandlerPool *hpool);

class MTManager {
  friend MTManager *mt();
  friend class MTLock;
private:
  static MTManager *mt_;
  DThread evhandler_thread;
  CondVar pending_event_lock;
  Mutex work_lock;
  bool have_queued_events;
  bool is_running;
  bool should_exit;
  Process::thread_mode_t threadMode;
  
  void evhandler_main();
  static void evhandler_main_wrapper(void *);
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
};

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
         if (MTManager::default_thread_mode == Process::HandlerThreading ||
             MTManager::default_thread_mode == Process::CallbackThreading) {
            mt()->startWork();
         }
         mt()->setThreadMode(MTManager::default_thread_mode, true);
      }
      else if (mt()->handlerThreading()) {
         mt()->startWork();
         if (c == deliver_callbacks && 
             MTManager::default_thread_mode == Process::HandlerThreading && 
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
             MTManager::default_thread_mode == Process::HandlerThreading) 
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

#endif
