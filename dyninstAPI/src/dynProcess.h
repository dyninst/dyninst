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

#ifndef DYNPROCESS_H
#define DYNPROCESS_H
/*
 * pcProcess.h
 *
 * A class that encapsulates a ProcControlAPI Process for the rest of Dyninst.
 */

#include <assert.h>
#include <list>
#include <stddef.h>
#include <utility>
#include <vector>
#include <string>
#include <map>
#include <set>

#include "addressSpace.h"
#include "inst.h"
#include "codeRange.h"
#include "infHeap.h"
#include "ast.h"
#include "syscallNotification.h"
#include "baseTramp.h"

#include "Symtab.h"

#include "symtabAPI/h/SymtabReader.h"
#include "PCProcess.h"
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/symlookup.h"
#include "pcEventHandler.h"
#include "frame.h"

#define RPC_LEAVE_AS_IS 0
#define RPC_RUN_WHEN_DONE 1
#define RPC_STOP_WHEN_DONE 2

class DynSymReaderFactory;
class PCEventMuxer;

class PCProcess : public AddressSpace {
    // Why PCEventHandler is a friend
    // 
    // PCProcess needs two interfaces: one that the rest of Dyninst sees and
    // one that can be used to update the state of the PCProcess during event
    // handling.
    //
    // The argument for having two different interfaces is that it will keep
    // process control internals from bleeding out into the rest of Dyninst.
    // This allows changes to the internals to have relatively low impact on the
    // rest of Dyninst
    friend class PCEventHandler;
	friend class PCEventMuxer; // Needed for some state queries that are protected
	friend class HybridAnalysis;

public:
    // The desired state of the process, as indicated by the user
    typedef enum {
        ps_stopped,
        ps_running,
    } processState_t;

    // Process creation and control
    static PCProcess *createProcess(const std::string file, std::vector<std::string> &argv,
                                    BPatch_hybridMode analysisMode,
                                    std::vector<std::string> &envp,
                                    const std::string dir, int stdin_fd, int stdout_fd,
                                    int stderr_fd);

    static PCProcess *attachProcess(const std::string &progpath, int pid,
                                    BPatch_hybridMode analysisMode);
    ~PCProcess();

    static std::string createExecPath(const std::string &file, const std::string &dir);
    virtual bool getDyninstRTLibName();

    bool continueProcess();
    bool stopProcess();
    bool terminateProcess();
    bool detachProcess(bool cont);

    // Process status
    bool isBootstrapped() const; // true if Dyninst has finished it's initialization for the process
    bool isAttached() const; // true if ok to operate on the process
    bool isStopped() const; // true if the process is stopped
    bool isForcedTerminating() const { return forcedTerminating_; } // true if we're in the middle of a forced termination
    bool isTerminated() const; // true if the process is terminated ( either via normal exit or crash )
    bool hasExitedNormally() const; // true if the process has exited (via a normal exit)
    bool isExecing() const; // true if the process is in the middle of an exec
    processState_t getDesiredProcessState() const;
    void setDesiredProcessState(processState_t ps);

    // Memory access
    bool dumpCore(std::string coreFile); // platform-specific
    bool writeDataSpace(void *inTracedProcess,
                        u_int amount, const void *inSelf);
    bool writeDataWord(void *inTracedProcess,
                       u_int amount, const void *inSelf);
    bool readDataSpace(const void *inTracedProcess, u_int amount,
                       void *inSelf, bool displayErrMsg);
    bool readDataWord(const void *inTracedProcess, u_int amount,
                      void *inSelf, bool displayErrMsg);
    bool writeTextSpace(void *inTracedProcess, u_int amount, const void *inSelf);
    bool writeTextWord(void *inTracedProcess, u_int amount, const void *inSelf);
    bool readTextSpace(const void *inTracedProcess, u_int amount,
                       void *inSelf);
    bool readTextWord(const void *inTracedProcess, u_int amount,
                      void *inSelf);

    unsigned getMemoryPageSize() const;

    typedef ProcControlAPI::Process::mem_perm PCMemPerm;
    bool getMemoryAccessRights(Address start,  PCMemPerm& rights);
    bool setMemoryAccessRights(Address start,  size_t size, PCMemPerm  rights);
    void changeMemoryProtections(Address addr, size_t size, PCMemPerm  rights,
                                 bool setShadow);

public:

    // Process properties and fields
    PCThread *getInitialThread() const;
    PCThread *getThread(dynthread_t tid) const;
    void getThreads(std::vector<PCThread* > &threads) const;
    void addThread(PCThread *thread);
    bool removeThread(dynthread_t tid);

    int getPid() const;

    bool wasRunningWhenAttached() const;
    bool wasCreatedViaAttach() const;
    bool wasCreatedViaFork() const;
    PCEventHandler *getPCEventHandler() const;
    int incrementThreadIndex();

    // Stackwalking
    bool walkStacks(std::vector<std::vector<Frame> > &stackWalks);
    bool getAllActiveFrames(std::vector<Frame> &activeFrames);

    // Inferior Malloc
    Address inferiorMalloc(unsigned size, inferiorHeapType type=anyHeap,
                           Address near_=0, bool *err=NULL);
    void inferiorMallocConstraints(Address near, Address &lo, Address &hi,
                                   inferiorHeapType /* type */); // platform-specific
    virtual void inferiorFree(Dyninst::Address);
    virtual bool inferiorRealloc(Dyninst::Address, unsigned int);

    // Instrumentation support
    bool mappedObjIsDeleted(mapped_object *obj);
    void installInstrRequests(const std::vector<instMapping*> &requests);
    Address getTOCoffsetInfo(Address dest); // platform-specific
    Address getTOCoffsetInfo(func_instance *func); // platform-specific
    bool getOPDFunctionAddr(Address &opdAddr); // architecture-specific

    // iRPC interface
    bool postIRPC(AstNodePtr action,
                 void *userData,
                 bool runProcessWhenDone,
                 PCThread *thread,
                 bool synchronous,
                 void **result,
                 bool userRPC,
                 bool isMemAlloc = false,
                 Address addr = 0);
	bool postIRPC(void* buffer, 
		int size, 
		void* userData, 
		bool runProcessWhenDone, 
		PCThread* thread, 
		bool synchronous, 
		void** result, 
		bool userRPC, 
		bool isMemAlloc = false, 
		Address addr = 0);
private:
        bool postIRPC_internal(void *buffer,
                               unsigned size,
                               unsigned breakOffset,
                               Register resultReg,
                               Address addr,
                               void *userData,
                               bool runProcessWhenDone,
                               PCThread *thread,
                               bool synchronous,
                               bool userRPC,
                               bool isMemAlloc,
                               void **result);
                               

public:
    /////////////////////////////////////////////
    // Begin Exploratory and Defensive mode stuff
    /////////////////////////////////////////////
    BPatch_hybridMode getHybridMode();

// TODO FIXME
#if defined(os_windows)
    std::vector<func_instance *> initial_thread_functions;
    bool setBeingDebuggedFlag(bool debuggerPresent);
#endif

    // code overwrites
    bool getOverwrittenBlocks
      ( std::map<Address, unsigned char *>& overwrittenPages,//input
        std::list<std::pair<Address,Address> >& overwrittenRegions,//output
        std::list<block_instance *> &writtenBBIs);//output

    // synch modified mapped objects with current memory contents
    mapped_object *createObjectNoFile(Address addr);
    void updateCodeBytes( const std::list<std::pair<Address, Address> > &owRegions);

    bool isRuntimeHeapAddr(Address addr) const;
    bool isExploratoryModeOn() const;

    bool hideDebugger(); // platform-specific
    void flushAddressCache_RT(Address start = 0, unsigned size = 0);
    void flushAddressCache_RT(codeRange *range) { 
        flushAddressCache_RT(range->get_address(), range->get_size());
    }

    // Active instrumentation tracking
    typedef std::pair<Address, Address> AddrPair;
    typedef std::set<AddrPair> AddrPairSet;
    typedef std::set<Address> AddrSet;

    struct ActiveDefensivePad {
        Address activePC;
        Address padStart;
        block_instance *callBlock;
        block_instance *ftBlock;
        ActiveDefensivePad(Address a, Address b, block_instance *c, block_instance *d)
            : activePC(a), padStart(b), callBlock(c), ftBlock(d) {}
    };
    typedef std::list<ActiveDefensivePad> ADPList;

    bool patchPostCallArea(instPoint *point);
    func_instance *findActiveFuncByAddr(Address addr);

    /////////////////////////////////////////////
    // End Exploratory and Defensive mode stuff
    /////////////////////////////////////////////

    // No function is pushed onto return vector if address can't be resolved
    // to a function
    std::vector<func_instance *> pcsToFuncs(std::vector<Frame> stackWalk);

    // architecture-specific
    virtual bool hasBeenBound(const SymtabAPI::relocationEntry &entry, 
			   func_instance *&target_pdf, Address base_addr);

    // AddressSpace implementations //
    virtual Address offset() const;
    virtual Address length() const;
    virtual Architecture getArch() const;
    virtual bool multithread_capable(bool ignoreIfMtNotSet = false); // platform-specific
    virtual bool multithread_ready(bool ignoreIfMtNotSet = false);
    virtual bool needsPIC();
    virtual void addTrap(Address from, Address to, codeGen &gen);
    virtual void removeTrap(Address from);

    // Miscellaneuous
    void debugSuicide();
    bool dumpImage(std::string outFile);

    // Stackwalking internals
    bool walkStack(std::vector<Frame> &stackWalk, PCThread *thread);
    bool getActiveFrame(Frame &frame, PCThread *thread);

    void addSignalHandler(Address, unsigned);
    bool isInSignalHandler(Address addr);

    bool bindPLTEntry(const SymtabAPI::relocationEntry &,
                      Address,
                      func_instance *, 
                      Address);
    bool supportsUserThreadEvents(); 

protected:
    typedef enum {
        bs_attached,
        bs_readyToLoadRTLib,
        bs_loadedRTLib,
        bs_initialized // RT library has been loaded
    } bootstrapState_t;
    
    typedef enum {
        not_cached,
        cached_mt_true,
        cached_mt_false
    } mt_cache_result_t;

    static PCProcess *setupExecedProcess(PCProcess *proc, std::string execPath);

    // Process create/exec constructor
    PCProcess(ProcControlAPI::Process::ptr pcProc, std::string file,
            BPatch_hybridMode analysisMode)
        : pcProc_(pcProc),
          parent_(NULL),
          initialThread_(NULL), 
          file_(file), 
          attached_(true),
          execing_(false),
          exiting_(false),
       forcedTerminating_(false),
          runningWhenAttached_(false), 
          createdViaAttach_(false),
          processState_(ps_stopped),
          bootstrapState_(bs_attached),
          main_function_(NULL),
          curThreadIndex_(0),
          reportedEvent_(false),
          savedPid_(pcProc->getPid()),
          savedArch_(pcProc->getArchitecture()),
          analysisMode_(analysisMode), 
          RT_address_cache_addr_(0),
          sync_event_id_addr_(0),
          sync_event_arg1_addr_(0),
          sync_event_arg2_addr_(0),
          sync_event_arg3_addr_(0),
          sync_event_breakpoint_addr_(0),
          rt_trap_func_addr_(0),
       thread_hash_tids(0),
       thread_hash_indices(0),
       thread_hash_size(0),
          eventHandler_(NULL),
          eventCount_(0),
          tracedSyscalls_(NULL),
          mt_cache_result_(not_cached),
          isInDebugSuicide_(false),
          irpcTramp_(NULL),
          inEventHandling_(false),
          stackwalker_(NULL)
    {
        irpcTramp_ = baseTramp::createForIRPC(this);
    }

    // Process attach constructor
    PCProcess(ProcControlAPI::Process::ptr pcProc, BPatch_hybridMode analysisMode)
        : pcProc_(pcProc),
          parent_(NULL),
          initialThread_(NULL), 
          attached_(true), 
          execing_(false),
          exiting_(false),
       forcedTerminating_(false),
          runningWhenAttached_(false), 
          createdViaAttach_(true),
          processState_(ps_stopped),
          bootstrapState_(bs_attached), 
          main_function_(NULL),
          curThreadIndex_(0),
          reportedEvent_(false),
          savedPid_(pcProc->getPid()),
          savedArch_(pcProc->getArchitecture()),
          analysisMode_(analysisMode), 
          RT_address_cache_addr_(0),
          sync_event_id_addr_(0),
          sync_event_arg1_addr_(0),
          sync_event_arg2_addr_(0),
          sync_event_arg3_addr_(0),
          sync_event_breakpoint_addr_(0),
          rt_trap_func_addr_(0),
       thread_hash_tids(0),
       thread_hash_indices(0),
       thread_hash_size(0),
          eventHandler_(NULL),
          eventCount_(0),
          tracedSyscalls_(NULL),
          mt_cache_result_(not_cached),
          isInDebugSuicide_(false),
          irpcTramp_(NULL),
          inEventHandling_(false),
          stackwalker_(NULL)
    {
        irpcTramp_ = baseTramp::createForIRPC(this);
    }

    static PCProcess *setupForkedProcess(PCProcess *parent, ProcControlAPI::Process::ptr pcProc);

    // Process fork constructor
    PCProcess(PCProcess *parent, ProcControlAPI::Process::ptr pcProc)
        : pcProc_(pcProc),
          parent_(parent),
          initialThread_(NULL), // filled in during bootstrap
          file_(parent->file_),
          attached_(true), 
          execing_(false),
          exiting_(false),
       forcedTerminating_(false),
          runningWhenAttached_(false), 
          createdViaAttach_(false),
          processState_(ps_stopped),
          bootstrapState_(bs_attached), 
          main_function_(parent->main_function_),
          curThreadIndex_(0), // threads are created from ProcControl threads
          reportedEvent_(false),
          savedPid_(pcProc->getPid()),
          savedArch_(pcProc->getArchitecture()),
          analysisMode_(parent->analysisMode_), 
          RT_address_cache_addr_(parent->RT_address_cache_addr_),
          sync_event_id_addr_(parent->sync_event_id_addr_),
          sync_event_arg1_addr_(parent->sync_event_arg1_addr_),
          sync_event_arg2_addr_(parent->sync_event_arg2_addr_),
          sync_event_arg3_addr_(parent->sync_event_arg3_addr_),
          sync_event_breakpoint_addr_(parent->sync_event_breakpoint_addr_),
          rt_trap_func_addr_(parent->rt_trap_func_addr_),
       thread_hash_tids(parent->thread_hash_tids),
       thread_hash_indices(parent->thread_hash_indices),
       thread_hash_size(parent->thread_hash_size),
          eventHandler_(parent->eventHandler_),
          eventCount_(0),
          tracedSyscalls_(NULL), // filled after construction
          mt_cache_result_(parent->mt_cache_result_),
          isInDebugSuicide_(parent->isInDebugSuicide_),
          inEventHandling_(false),
          stackwalker_(NULL)
    {
        irpcTramp_ = baseTramp::createForIRPC(this);
    }

    // bootstrapping
    bool bootstrapProcess();
    bool hasReachedBootstrapState(bootstrapState_t state) const;
    void setBootstrapState(bootstrapState_t newState);
    bool createStackwalker();
    bool createStackwalkerSteppers(); // platform-specific
    void createInitialThreads();
    bool createInitialMappedObjects();
    bool getExecFileDescriptor(std::string filename,
                               bool waitForTrap, // Should we wait for process init
                               fileDescriptor &desc);
    void findSignalHandler(mapped_object *obj);
    void setMainFunction();
    bool setAOut(fileDescriptor &desc);
    bool hasPassedMain(); // OS-specific
    bool insertBreakpointAtMain();
    ProcControlAPI::Breakpoint::ptr getBreakpointAtMain() const;
    bool removeBreakpointAtMain();
    Address getLibcStartMainParam(PCThread *thread); // architecture-specific
    bool copyDanglingMemory(PCProcess *parent);
    void invalidateMTCache();

    // RT library management
    bool loadRTLib();

    AstNodePtr createUnprotectStackAST(); // architecture-specific
    bool setRTLibInitParams();
    bool instrumentMTFuncs();
    bool extractBootstrapStruct(DYNINST_bootstrapStruct *bs_record);
    bool iRPCDyninstInit();


    Address getRTEventBreakpointAddr();
    Address getRTEventIdAddr();
    Address getRTEventArg1Addr();
    Address getRTEventArg2Addr();
    Address getRTEventArg3Addr();
    Address getRTTrapFuncAddr();

    // Shared library managment
    void addASharedObject(mapped_object *newObj);
    void removeASharedObject(mapped_object *oldObj);

    // Inferior heap management
    void addInferiorHeap(mapped_object *obj);
    bool skipHeap(const heapDescriptor &heap); // platform-specific
    bool inferiorMallocDynamic(int size, Address lo, Address hi);

    // platform-specific
    inferiorHeapType getDynamicHeapType() const; 
    
    // Hybrid Mode
    bool triggerStopThread(Address pointAddress, int callbackID, void *calculation);
    Address stopThreadCtrlTransfer(instPoint *intPoint, Address target);
    bool generateRequiredPatches(instPoint *callPt, AddrPairSet &);
    void generatePatchBranches(AddrPairSet &);

    // Event Handling
    void triggerNormalExit(int exitcode);

    // Misc

    // platform-specific, populates the passed map, if the file descriptors differ
    static void redirectFds(int stdin_fd, int stdout_fd, int stderr_fd, 
            std::map<int, int> &result);

    // platform-specific, sets LD_PRELOAD with RT library 
    static bool setEnvPreload(std::vector<std::string> &envp, std::string fileName);

    bool isInDebugSuicide() const;

    // Event handling support
    void setReportingEvent(bool b);
    bool hasReportedEvent() const;
    void setExecing(bool b);
    bool isInEventHandling() const;
    void setInEventHandling(bool b);
    void setExiting(bool b);
    bool isExiting() const;
    bool hasPendingEvents();
    bool hasRunningSyncRPC() const;
	void addSyncRPCThread(Dyninst::ProcControlAPI::Thread::ptr thr);
    void removeSyncRPCThread(Dyninst::ProcControlAPI::Thread::ptr thr);
    bool continueSyncRPCThreads();

    // ProcControl doesn't keep around a process's information after it exits.
    // However, we allow a Dyninst user to query certain information out of
    // an exited process. Just make sure no operations are attempted on the
    // ProcControl process
    void markExited();

    // Debugging
    bool setBreakpoint(Address addr);
    void writeDebugDataSpace(void *inTracedProcess, u_int amount,
            const void *inSelf);
    bool launchDebugger();
    bool startDebugger(); // platform-specific
    static void initSymtabReader();

    // Fields //

    // Underlying ProcControl process
    ProcControlAPI::Process::ptr pcProc_;
    PCProcess *parent_;

    // Corresponding threads
    std::map<dynthread_t, PCThread *> threadsByTid_;
    PCThread *initialThread_;

    ProcControlAPI::Breakpoint::ptr mainBrkPt_;

    // Properties
    std::string file_;
    bool attached_;
    bool execing_;
    bool exiting_;
    bool forcedTerminating_;
    bool runningWhenAttached_;
    bool createdViaAttach_;
    processState_t processState_;
    bootstrapState_t bootstrapState_;
    func_instance *main_function_;
    int curThreadIndex_;
    // true when Dyninst has reported an event to ProcControlAPI for this process
    bool reportedEvent_; // indicates the process should remain stopped
    int savedPid_; // ProcControl doesn't keep around Process objects after exit
    Dyninst::Architecture savedArch_;

    // Hybrid Analysis
    BPatch_hybridMode analysisMode_;

    // Active instrumentation tracking
    codeRangeTree signalHandlerLocations_;
    std::vector<mapped_object *> deletedObjects_;
    std::vector<heapItem *> dyninstRT_heaps_;
    Address RT_address_cache_addr_;

    // Addresses of variables in RT library
    Address sync_event_id_addr_;
    Address sync_event_arg1_addr_;
    Address sync_event_arg2_addr_;
    Address sync_event_arg3_addr_;
    Address sync_event_breakpoint_addr_;
    Address rt_trap_func_addr_;
    Address thread_hash_tids;
    Address thread_hash_indices;
    int thread_hash_size;

    // The same PCEventHandler held by the BPatch layer
    PCEventHandler *eventHandler_;
    //Mutex<> eventCountLock_;
    int eventCount_;

    syscallNotification *tracedSyscalls_;

    mt_cache_result_t mt_cache_result_;

    bool isInDebugSuicide_; // Single stepping is only valid in this context

    // Misc.
    baseTramp *irpcTramp_;
    bool inEventHandling_;
	std::set<Dyninst::ProcControlAPI::Thread::ptr> syncRPCThreads_;
    Dyninst::Stackwalker::Walker *stackwalker_;
    static Dyninst::SymtabAPI::SymtabReaderFactory *symReaderFactory_;
    std::map<Address, ProcControlAPI::Breakpoint::ptr> installedCtrlBrkpts;
};

class inferiorRPCinProgress : public codeRange {
public:
    inferiorRPCinProgress() :
        rpc(ProcControlAPI::IRPC::ptr()),
        rpcStartAddr(0),
        rpcCompletionAddr(0),
        resultRegister(Null_Register),
        returnValue(NULL),
        runProcWhenDone(false),
        isComplete(false),
        deliverCallbacks(false),
        userData(NULL),
		thread(Dyninst::ProcControlAPI::Thread::ptr()),
        synchronous(false),
        memoryAllocated(false)
    {}

    virtual Address get_address() const { return rpc->getAddress(); }
    virtual unsigned get_size() const { return (rpcCompletionAddr - rpc->getAddress())+1; }
    virtual void *getPtrToInstruction(Address /*addr*/) const { assert(0); return NULL; }

    ProcControlAPI::IRPC::ptr rpc;
    Address rpcStartAddr;
    Address rpcCompletionAddr;

    Register resultRegister; // register that contains the return value
    void *returnValue;

    bool runProcWhenDone;
    bool isComplete;
    bool deliverCallbacks;
    void *userData;
	Dyninst::ProcControlAPI::Thread::ptr thread;
    bool synchronous; // caller is responsible for cleaning up this object
    bool memoryAllocated;
};

class signal_handler_location : public codeRange {
public:
    signal_handler_location(Address addr, unsigned size) :
        addr_(addr), size_(size) {}

    Address get_address() const {
        return addr_;
    }
    unsigned get_size() const {
        return size_;
    }

private:
    Address addr_;
    unsigned size_;
};

class StackwalkSymLookup : public Dyninst::Stackwalker::SymbolLookup {
  private:
    PCProcess *proc_;

  public:
    StackwalkSymLookup(PCProcess *pc);
    virtual bool lookupAtAddr(Dyninst::Address addr,
                              std::string &out_name,
                              void* &out_value);
    virtual ~StackwalkSymLookup();
};

class StackwalkInstrumentationHelper : public Dyninst::Stackwalker::DyninstDynamicHelper {
  private:
    PCProcess *proc_;

  public:
    StackwalkInstrumentationHelper(PCProcess *pc);
    virtual bool isInstrumentation(Dyninst::Address ra, Dyninst::Address *orig_ra,
                                   unsigned *stack_height, bool *aligned, bool *entryExit);
    virtual ~StackwalkInstrumentationHelper();
};

class DynFrameHelper : public Dyninst::Stackwalker::FrameFuncHelper {
  private:
    PCProcess *proc_;

  public:
    DynFrameHelper(PCProcess *pc);
    virtual Dyninst::Stackwalker::FrameFuncHelper::alloc_frame_t allocatesFrame(Address addr);
    virtual ~DynFrameHelper();
};

class DynWandererHelper : public Dyninst::Stackwalker::WandererHelper {
  private:
    PCProcess *proc_;

  public:
    DynWandererHelper(PCProcess *pc);
    virtual bool isPrevInstrACall(Address addr, Address &target);
    virtual Dyninst::Stackwalker::WandererHelper::pc_state isPCInFunc(Address func_entry, Address pc);
    virtual bool requireExactMatch();
    virtual ~DynWandererHelper();
};

#endif
