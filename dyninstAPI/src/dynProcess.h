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
    friend class PCEventHandler;
	friend class PCEventMuxer;
	friend class HybridAnalysis;

public:
    // The desired state of the process, as indicated by the user
    typedef enum {
        ps_stopped,
        ps_running,
    } processState_t;

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

    bool isBootstrapped() const;
    bool isAttached() const;
    bool isStopped() const;
    bool isForcedTerminating() const { return forcedTerminating_; }
    bool isTerminated() const;
    bool hasExitedNormally() const;
    bool isExecing() const;
    processState_t getDesiredProcessState() const;
    void setDesiredProcessState(processState_t ps);

    bool dumpCore(std::string coreFile);
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

    bool walkStacks(std::vector<std::vector<Frame> > &stackWalks);
    bool getAllActiveFrames(std::vector<Frame> &activeFrames);

    Address inferiorMalloc(unsigned size, inferiorHeapType type=anyHeap,
                           Address near_=0, bool *err=NULL);
    void inferiorMallocConstraints(Address near, Address &lo, Address &hi,
                                   inferiorHeapType /* type */);
    virtual void inferiorFree(Dyninst::Address);
    virtual bool inferiorRealloc(Dyninst::Address, unsigned int);

    bool mappedObjIsDeleted(mapped_object *obj);
    void installInstrRequests(const std::vector<instMapping*> &requests);
    Address getTOCoffsetInfo(Address dest);
    Address getTOCoffsetInfo(func_instance *func);
    bool getOPDFunctionAddr(Address &opdAddr);

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
    BPatch_hybridMode getHybridMode();

// TODO FIXME
#if defined(os_windows)
    std::vector<func_instance *> initial_thread_functions;
    bool setBeingDebuggedFlag(bool debuggerPresent);
#endif

    bool getOverwrittenBlocks
      ( std::map<Address, unsigned char *>& overwrittenPages,
        std::list<std::pair<Address,Address> >& overwrittenRegions,
        std::list<block_instance *> &writtenBBIs);

    mapped_object *createObjectNoFile(Address addr);
    void updateCodeBytes( const std::list<std::pair<Address, Address> > &owRegions);

    bool isRuntimeHeapAddr(Address addr) const;
    bool isExploratoryModeOn() const;

    bool hideDebugger();
    void flushAddressCache_RT(Address start = 0, unsigned size = 0);
    void flushAddressCache_RT(codeRange *range) { 
        flushAddressCache_RT(range->get_address(), range->get_size());
    }

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

    std::vector<func_instance *> pcsToFuncs(std::vector<Frame> stackWalk);

    virtual bool hasBeenBound(const SymtabAPI::relocationEntry &entry, 
			   func_instance *&target_pdf, Address base_addr);


    virtual Address offset() const;
    virtual Address length() const;
    virtual Architecture getArch() const;
    virtual bool multithread_capable(bool ignoreIfMtNotSet = false);
    virtual bool multithread_ready(bool ignoreIfMtNotSet = false);
    virtual bool needsPIC();
    virtual void addTrap(Address from, Address to, codeGen &gen);
    virtual void removeTrap(Address from);

    void debugSuicide();
    bool dumpImage(std::string outFile);

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
        bs_initialized
    } bootstrapState_t;
    
    typedef enum {
        not_cached,
        cached_mt_true,
        cached_mt_false
    } mt_cache_result_t;

    static PCProcess *setupExecedProcess(PCProcess *proc, std::string execPath);

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

    bool bootstrapProcess();
    bool hasReachedBootstrapState(bootstrapState_t state) const;
    void setBootstrapState(bootstrapState_t newState);
    bool createStackwalker();
    bool createStackwalkerSteppers();
    void createInitialThreads();
    bool createInitialMappedObjects();
    bool getExecFileDescriptor(std::string filename,
                               bool waitForTrap, // Should we wait for process init
                               fileDescriptor &desc);
    void findSignalHandler(mapped_object *obj);
    void setMainFunction();
    bool setAOut(fileDescriptor &desc);
    bool hasPassedMain();
    bool insertBreakpointAtMain();
    ProcControlAPI::Breakpoint::ptr getBreakpointAtMain() const;
    bool removeBreakpointAtMain();
    Address getLibcStartMainParam(PCThread *thread);
    bool copyDanglingMemory(PCProcess *parent);
    void invalidateMTCache();

    bool loadRTLib();

    AstNodePtr createUnprotectStackAST();
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

    void addASharedObject(mapped_object *newObj);
    void removeASharedObject(mapped_object *oldObj);

    void addInferiorHeap(mapped_object *obj);
    bool skipHeap(const heapDescriptor &heap); // platform-specific
    bool inferiorMallocDynamic(int size, Address lo, Address hi);

    inferiorHeapType getDynamicHeapType() const; 
    
    bool triggerStopThread(Address pointAddress, int callbackID, void *calculation);
    Address stopThreadCtrlTransfer(instPoint *intPoint, Address target);
    bool generateRequiredPatches(instPoint *callPt, AddrPairSet &);
    void generatePatchBranches(AddrPairSet &);

    void triggerNormalExit(int exitcode);

    static void redirectFds(int stdin_fd, int stdout_fd, int stderr_fd, 
            std::map<int, int> &result);

    static bool setEnvPreload(std::vector<std::string> &envp, std::string fileName);

    bool isInDebugSuicide() const;

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

    void markExited();

    bool setBreakpoint(Address addr);
    void writeDebugDataSpace(void *inTracedProcess, u_int amount,
            const void *inSelf);
    bool launchDebugger();
    bool startDebugger();
    static void initSymtabReader();

    ProcControlAPI::Process::ptr pcProc_;
    PCProcess *parent_;

    std::map<dynthread_t, PCThread *> threadsByTid_;
    PCThread *initialThread_;

    ProcControlAPI::Breakpoint::ptr mainBrkPt_;

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
    bool reportedEvent_;
    int savedPid_;
    Dyninst::Architecture savedArch_;

    BPatch_hybridMode analysisMode_;

    codeRangeTree signalHandlerLocations_;
    std::vector<mapped_object *> deletedObjects_;
    std::vector<heapItem *> dyninstRT_heaps_;
    Address RT_address_cache_addr_;

    Address sync_event_id_addr_;
    Address sync_event_arg1_addr_;
    Address sync_event_arg2_addr_;
    Address sync_event_arg3_addr_;
    Address sync_event_breakpoint_addr_;
    Address rt_trap_func_addr_;
    Address thread_hash_tids;
    Address thread_hash_indices;
    int thread_hash_size;

    PCEventHandler *eventHandler_;
    //Mutex<> eventCountLock_;
    int eventCount_;

    syscallNotification *tracedSyscalls_;

    mt_cache_result_t mt_cache_result_;

    bool isInDebugSuicide_;

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

    Register resultRegister;
    void *returnValue;

    bool runProcWhenDone;
    bool isComplete;
    bool deliverCallbacks;
    void *userData;
	Dyninst::ProcControlAPI::Thread::ptr thread;
    bool synchronous;
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
