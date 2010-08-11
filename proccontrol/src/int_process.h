#if !defined(INT_PROCESS_H_)
#define INT_PROCESS_H_

#include "proccontrol/h/Process.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/h/Event.h"

#include "dynutil/h/dyn_regs.h"
#include "common/h/dthread.h"

#include <vector>
#include <map>
#include <list>
#include <set>
#include <utility>
#include <queue>

using namespace Dyninst;
using namespace ProcControlAPI;

class int_thread;
class int_threadPool;
class handlerpool;
class int_iRPC;

typedef dyn_detail::boost::shared_ptr<int_iRPC> int_iRPC_ptr;

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
   bool post_create();

   bool attach();
   virtual bool plat_attach() = 0;
   bool attachThreads();
   bool post_attach();

  public:
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
   static const char *stateName(State s);
   void initializeProcess(Process::ptr p);

   void setExitCode(int c);
   void setCrashSignal(int s);
   bool getExitCode(int &c);
   bool getCrashSignal(int &s);

   virtual bool plat_individualRegAccess() = 0;

   void addProcStopper(Event::ptr ev);
   Event::ptr removeProcStopper();
   bool hasQueuedProcStoppers() const;

   int getAddressWidth();
   HandlerPool *handlerPool() const;

   bool addBreakpoint(Dyninst::Address addr, int_breakpoint *bp);
   bool rmBreakpoint(Dyninst::Address addr, int_breakpoint *bp);
   installed_breakpoint *getBreakpoint(Dyninst::Address addr);

   Dyninst::Address infMalloc(unsigned long size, bool use_addr = false, Dyninst::Address addr = 0x0);
   bool infFree(Dyninst::Address addr);

   bool readMem(void *local, Dyninst::Address remote, size_t size);
   bool writeMem(void *local, Dyninst::Address remote, size_t size);
   virtual bool plat_readMem(int_thread *thr, void *local, 
                             Dyninst::Address remote, size_t size) = 0;
   virtual bool plat_writeMem(int_thread *thr, void *local, 
                              Dyninst::Address remote, size_t size) = 0;
   
   virtual bool independentLWPControl() = 0;
   static bool isInCB();
   static void setInCB(bool b);

   int_library *getLibraryByName(std::string s) const;
   size_t numLibs() const;
   virtual bool refresh_libraries(std::set<int_library *> &added_libs,
                                  std::set<int_library *> &rmd_libs) = 0;
   virtual bool initLibraryMechanism() = 0;

   bool forceGeneratorBlock() const;
   void setForceGeneratorBlock(bool b);
   bool isForceTerminating() const;
   void markForceTerminating();

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
   bool forceTerminating;
   int exitCode;
   static bool in_callback;
   mem_state::ptr mem;
   std::map<Dyninst::Address, unsigned> exec_mem_cache;
   std::queue<Event::ptr> proc_stoppers;
};

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
 * status:
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
 * states:
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

   Dyninst::THR_ID getTid() const;
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
   bool contWithSignal(int sigOverride = -1);
   virtual bool plat_cont() = 0;
   virtual bool plat_stop() = 0;
   void setPendingUserStop(bool b);
   bool hasPendingUserStop() const;
   void setPendingStop(bool b);
   bool hasPendingStop() const;

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
   bool saveRegsForRPC();
   bool restoreRegsForRPC(bool clear);
   bool hasSavedRPCRegs();
   bool runningInternalRPC() const;
   void incSyncRPCCount();
   void decSyncRPCCount();
   bool hasSyncRPC();
   int_iRPC_ptr nextPostedIRPC() const;
   bool handleNextPostedIRPC(bool block);
   int_iRPC_ptr hasRunningProcStopperRPC() const;

   //Register Management
   bool getAllRegisters(int_registerPool &pool);
   bool getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val);
   bool setAllRegisters(int_registerPool &pool);
   bool setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val);
   virtual bool plat_getAllRegisters(int_registerPool &pool) = 0;
   virtual bool plat_getRegister(Dyninst::MachRegister reg, 
                                 Dyninst::MachRegisterVal &val) = 0;
   virtual bool plat_setAllRegisters(int_registerPool &pool) = 0;
   virtual bool plat_setRegister(Dyninst::MachRegister reg, 
                                 Dyninst::MachRegisterVal val) = 0;

   //Misc
   virtual bool attach() = 0;
   Thread::ptr thread();

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
   rpc_list_t posted_rpcs;
   int_registerPool rpc_regs;
   unsigned sync_rpc_count;
   bool pending_user_stop;
   bool pending_stop;
   int num_locked_stops;
   bool user_single_step;
   bool single_step;
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

class installed_breakpoint
{
   friend class Dyninst::ProcControlAPI::EventBreakpoint;
 private:
   mem_state::ptr memory;
   std::set<int_breakpoint *> bps;
   std::set<Breakpoint::ptr> hl_bps;

   unsigned char buffer[4]; //At least as large as any arch's trap instruction
   int buffer_size;
   bool installed;
   int suspend_count;
   Dyninst::Address addr;
 public:
   installed_breakpoint(mem_state::ptr memory_, Dyninst::Address addr_);
   installed_breakpoint(mem_state::ptr memory_, const installed_breakpoint *ip);
   ~installed_breakpoint();

   bool addBreakpoint(int_process *proc, int_breakpoint *bp);
   bool rmBreakpoint(int_process *proc, int_breakpoint *bp, bool &empty);
   bool install(int_process *proc);
   bool uninstall(int_process *proc);
   bool plat_install(int_process *proc, bool should_save);

   bool suspend(int_process *proc);
   bool resume(int_process *proc);

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
HandlerPool *createDefaultHandlerPool();
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
  static void eventqueue_cb_wrapper();
  
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
