/*
 * Copyright (c) 1996-2010 Barton P. Miller
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

#ifndef PCPROCESS_H
#define PCPROCESS_H
/*
 * pcProcess.h
 *
 * A class that encapsulates a ProcControlAPI Process for the rest of Dyninst.
 */

#include <string>
#include <map>

#include "addressSpace.h"
#include "pcThread.h"
#include "pcEventHandler.h"
#include "BPatch_hybridAnalysis.h"
#include "inst.h"
#include "codeRange.h"
#include "infHeap.h"
#include "ast.h"
#include "syscallNotification.h"
#include "os.h"

#include "Symtab.h"

#include "proccontrol/h/Process.h"
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"

#define RPC_LEAVE_AS_IS 0
#define RPC_RUN_WHEN_DONE 1
#define RPC_STOP_WHEN_DONE 2

typedef enum { vsys_unknown, vsys_unused, vsys_notfound, vsys_found } syscallStatus_t;

class multiTramp;
class bblInstance;

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

public:
    // The desired state of the process, as indicated by the user
    typedef enum {
        ps_stopped,
        ps_running,
    } processState_t;

    // Process creation and control
    static PCProcess *createProcess(const std::string file, pdvector<std::string> *argv,
                                    BPatch_hybridMode analysisMode,
                                    pdvector<std::string> *envp,
                                    const std::string dir, int stdin_fd, int stdout_fd,
                                    int stderr_fd, PCEventHandler *eventHandler);

    static PCProcess *attachProcess(const std::string &progpath, int pid,
                                    BPatch_hybridMode analysisMode, 
                                    PCEventHandler *eventHandler);
    ~PCProcess();

    static std::string createExecPath(const std::string &file, const std::string &dir);

    bool continueProcess(int contSignal = 0);
    bool stopProcess();
    bool terminateProcess();
    bool detachProcess(bool cont);

    // Process status
    bool isBootstrapped() const; // true if Dyninst has finished it's initialization for the process
    bool isAttached() const; // true if ok to operate on the process
    bool isStopped() const; // true if the process is stopped
    bool isTerminated() const; // true if the process is terminated
    bool hasExited() const; // true if the process has exited
    bool isExecing() const; // true if the process is in the middle of an exec
    processState_t getDesiredProcessState() const;
    void setDesiredProcessState(processState_t ps);

    // Memory access
    bool dumpCore(const std::string coreFile); // platform-specific
    bool writeDebugDataSpace(void *inTracedProcess, u_int amount,
                             const void *inSelf);
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

    // Process properties and fields
    PCThread *getInitialThread() const;
    PCThread *getThread(dynthread_t tid) const;
    void getThreads(std::vector<PCThread* > &threads) const;
    void addThread(PCThread *thread);
    void removeThread(dynthread_t tid);

    int getPid() const;
    unsigned getAddressWidth() const;
    bool wasRunningWhenAttached() const;
    bool wasCreatedViaAttach() const;
    bool wasCreatedViaFork() const;
    PCEventHandler *getPCEventHandler() const;
    int incrementThreadIndex();

    // Stackwalking
    bool walkStacks(pdvector<pdvector<Frame> > &stackWalks);
    bool getAllActiveFrames(pdvector<Frame> &activeFrames);

    // Inferior Malloc
    Address inferiorMalloc(unsigned size, inferiorHeapType type=anyHeap,
                           Address near_=0, bool *err=NULL);
    void inferiorMallocConstraints(Address near, Address &lo, Address &hi,
                                   inferiorHeapType /* type */); // platform-specific
    virtual void inferiorFree(Dyninst::Address);
    virtual bool inferiorRealloc(Dyninst::Address, unsigned int);

    // Instrumentation support
    virtual void deleteGeneratedCode(generatedCodeObject *del);
    bool mappedObjIsDeleted(mapped_object *obj);
    void installInstrRequests(const pdvector<instMapping*> &requests);
    bool uninstallMutations();
    bool reinstallMutations();

    // iRPC interface
    bool postIRPC(AstNodePtr action,
                 void *userData,
                 bool runProcessWhenDone,
                 PCThread *thread,
                 bool synchronous,
                 void **result,
                 bool deliverCallbacks,
                 Address addr = 0);

    // Hybrid Mode
    BPatch_hybridMode getHybridMode();
    bool setMemoryAccessRights(Address start, Address size, int rights);

    // code overwrites
    bool getOverwrittenBlocks(std::map<Address, unsigned char *>& overwrittenPages,//input
                              std::map<Address,Address>& overwrittenRegions,//output
                              std::set<bblInstance *> &writtenBBIs); //output
    bool getDeadCodeFuncs(std::set<bblInstance *> &deadBlocks, // input
                          std::set<int_function*> &affectedFuncs, //output
                          std::set<int_function*> &deadFuncs); //output
    unsigned getMemoryPageSize() const;


    // synch modified mapped objects with current memory contents
    mapped_object *createObjectNoFile(Address addr);
    void updateMappedFile(std::map<Dyninst::Address,unsigned char*>& owPages,
                          std::map<Address,Address> owRegions);

    bool isRuntimeHeapAddr(Address addr) const;
    bool isExploratoryModeOn() const;

    bool hideDebugger(); // platform-specific

    // Active instrumentation tracking
    int_function *findActiveFuncByAddr(Address addr);
    void getActiveMultiMap(std::map<Address, multiTramp *> &map);
    void updateActiveMultis();
    void addActiveMulti(multiTramp *multi);
    void fixupActiveStackTargets();
    void invalidateActiveMultis() { isAMcacheValid_ = false; }

    // No function is pushed onto return vector if address can't be resolved
    // to a function
    pdvector<int_function *> pcsToFuncs(pdvector<Frame> stackWalk);

    // architecture-specific
    virtual bool hasBeenBound(const SymtabAPI::relocationEntry &entry, 
			   int_function *&target_pdf, Address base_addr);

    // AddressSpace implementations //
    virtual Address offset() const;
    virtual Address length() const;
    virtual Architecture getArch() const;
    virtual bool multithread_capable(bool ignoreIfMtNotSet = false); // platform-specific
    virtual bool multithread_ready(bool ignoreIfMtNotSet = false);
    virtual bool needsPIC();

    // Miscellaneuous
    void debugSuicide();
    bool dumpImage(std::string outFile);

    Address setAOutLoadAddress(fileDescriptor &desc); // platform-specific

    // Syscall tracing (all architecture specific)
    bool getSysCallParameters(const ProcControlAPI::RegisterPool &regs, long *params, int numparams);
    int getSysCallNumber(const ProcControlAPI::RegisterPool &regs);
    long getSysCallReturnValue(const ProcControlAPI::RegisterPool &regs);
    Address getSysCallProgramCounter(const ProcControlAPI::RegisterPool &regs);
    bool isMmapSysCall(int callnum);
    Offset getMmapLength(int, const ProcControlAPI::RegisterPool &regs);

    // Stackwalking internals
    bool walkStackFromFrame(Frame currentFrame, // Where to start walking
            pdvector<Frame> &stackWalk); // return parameter
    Frame preStackWalkInit(Frame startFrame);

    Address getVsyscallText() { return vsyscall_text_; }
    void setVsyscallText(Address addr) { vsyscall_text_ = addr; }
    Address getVsyscallStart() { return vsyscall_start_; }
    Dyninst::SymtabAPI::Symtab *getVsyscallObject() { return vsyscall_obj_; }
    void setVsyscallObject(SymtabAPI::Symtab *obj) { vsyscall_obj_ = obj; }
    void setVsyscallRange(Address start, Address end) 
        { vsyscall_start_ = start; vsyscall_end_ = end; }
    syscallStatus_t getVsyscallStatus() { return vsys_status_; }
    void setVsyscallStatus(syscallStatus_t s) { vsys_status_ = s; }
    Address getVsyscallEnd() { return vsyscall_end_; }

    void addSignalHandler(Address, unsigned);
    bool isInSignalHandler(Address addr);
    bool readAuxvInfo();

protected:
    typedef enum {
        bs_attached,
        bs_readyToLoadRTLib,
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
            BPatch_hybridMode analysisMode, PCEventHandler *eventHandler)
        : pcProc_(pcProc),
          parent_(NULL),
          initialThread_(NULL), 
          file_(file), 
          attached_(true),
          execing_(false),
          runningWhenAttached_(false), 
          createdViaAttach_(false),
          processState_(ps_stopped),
          bootstrapState_(bs_attached),
          main_function_(NULL),
          curThreadIndex_(0),
          reportedEvent_(false),
          savedPid_(pcProc->getPid()),
          analysisMode_(analysisMode), 
          memoryPageSize_(0),
          isAMcacheValid_(false),
          sync_event_id_addr_(0),
          sync_event_arg1_addr_(0),
          sync_event_arg2_addr_(0),
          sync_event_arg3_addr_(0),
          sync_event_breakpoint_addr_(0),
          eventHandler_(eventHandler),
          tracedSyscalls_(NULL),
          rtLibLoadHeap_(0),
          mt_cache_result_(not_cached),
          isInDebugSuicide_(false),
          vsyscall_text_(0),
          vsyscall_start_(0),
          vsyscall_obj_(NULL),
          vsyscall_end_(0),
          vsys_status_(vsys_unknown),
          auxv_parser_(NULL)
    {}

    // Process attach constructor
    PCProcess(ProcControlAPI::Process::ptr pcProc, BPatch_hybridMode analysisMode,
            PCEventHandler *eventHandler)
        : pcProc_(pcProc),
          parent_(NULL),
          initialThread_(NULL), 
          attached_(true), 
          execing_(false),
          runningWhenAttached_(false), 
          createdViaAttach_(true),
          processState_(ps_stopped),
          bootstrapState_(bs_attached), 
          main_function_(NULL),
          curThreadIndex_(0),
          reportedEvent_(false),
          savedPid_(pcProc->getPid()),
          analysisMode_(analysisMode), 
          memoryPageSize_(0),
          isAMcacheValid_(false),
          sync_event_id_addr_(0),
          sync_event_arg1_addr_(0),
          sync_event_arg2_addr_(0),
          sync_event_arg3_addr_(0),
          sync_event_breakpoint_addr_(0),
          eventHandler_(eventHandler),
          tracedSyscalls_(NULL),
          rtLibLoadHeap_(0),
          mt_cache_result_(not_cached),
          isInDebugSuicide_(false),
          vsyscall_text_(0),
          vsyscall_start_(0),
          vsyscall_obj_(NULL),
          vsyscall_end_(0),
          vsys_status_(vsys_unknown),
          auxv_parser_(NULL)
    {}

    static PCProcess *setupForkedProcess(PCProcess *parent, ProcControlAPI::Process::ptr pcProc);

    // Process fork constructor
    PCProcess(PCProcess *parent, ProcControlAPI::Process::ptr pcProc)
        : pcProc_(pcProc),
          parent_(parent),
          initialThread_(NULL), // filled in during bootstrap
          file_(parent->file_),
          attached_(true), 
          execing_(false),
          runningWhenAttached_(false), 
          createdViaAttach_(false),
          processState_(ps_stopped),
          bootstrapState_(bs_attached), 
          main_function_(parent->main_function_),
          curThreadIndex_(0), // threads are created from ProcControl threads
          reportedEvent_(false),
          savedPid_(pcProc->getPid()),
          analysisMode_(parent->analysisMode_), 
          memoryPageSize_(parent->memoryPageSize_),
          isAMcacheValid_(parent->isAMcacheValid_),
          sync_event_id_addr_(parent->sync_event_id_addr_),
          sync_event_arg1_addr_(parent->sync_event_arg1_addr_),
          sync_event_arg2_addr_(parent->sync_event_arg2_addr_),
          sync_event_arg3_addr_(parent->sync_event_arg3_addr_),
          sync_event_breakpoint_addr_(parent->sync_event_breakpoint_addr_),
          eventHandler_(parent->eventHandler_),
          tracedSyscalls_(NULL),
          rtLibLoadHeap_(parent->rtLibLoadHeap_),
          mt_cache_result_(parent->mt_cache_result_),
          isInDebugSuicide_(parent->isInDebugSuicide_),
          vsyscall_text_(parent->vsyscall_text_),
          vsyscall_start_(parent->vsyscall_start_),
          vsyscall_obj_(parent->vsyscall_obj_),
          vsyscall_end_(parent->vsyscall_end_),
          vsys_status_(parent->vsys_status_),
          auxv_parser_(parent->auxv_parser_)
    {
    }

    // bootstrapping
    bool bootstrapProcess();
    bool hasReachedBootstrapState(bootstrapState_t state) const;
    void setBootstrapState(bootstrapState_t newState);
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

    // RT library management
    bool loadRTLib();
    AstNodePtr createLoadRTAST(); // architecture-specific
    AstNodePtr createUnprotectStackAST(); // architecture-specific
    bool setRTLibInitParams();
    bool instrumentMTFuncs();
    bool initTrampGuard();
    Address findFunctionToHijack(); // OS-specific
    bool postRTLoadCleanup(); // architecture-specific
    bool extractBootstrapStruct(DYNINST_bootstrapStruct *bs_record);
    bool iRPCDyninstInit();

    Address getRTEventBreakpointAddr() const { return sync_event_breakpoint_addr_; }
    Address getRTEventIdAddr() const { return sync_event_id_addr_; }
    Address getRTEventArg1Addr() const { return sync_event_arg1_addr_; }
    Address getRTEventArg2Addr() const { return sync_event_arg2_addr_; }
    Address getRTEventArg3Addr() const { return sync_event_arg3_addr_; }
    void setRTEventBreakpointAddr(Address addr) { sync_event_breakpoint_addr_ = addr; }
    void setRTEventIdAddr(Address addr) { sync_event_id_addr_ = addr; }
    void setRTEventArg1Addr(Address addr) { sync_event_arg1_addr_ = addr; }
    void setRTEventArg2Addr(Address addr) { sync_event_arg2_addr_ = addr; }
    void setRTEventArg3Addr(Address addr) { sync_event_arg3_addr_ = addr; }

    // Shared library managment
    void addASharedObject(mapped_object *newObj);
    void removeASharedObject(mapped_object *oldObj);
    bool usesDataLoadAddress() const; // OS-specific

    // Inferior heap management
    void addInferiorHeap(mapped_object *obj);
    bool skipHeap(const heapDescriptor &heap); // platform-specific
    bool inferiorMallocDynamic(int size, Address lo, Address hi);

    // platform-specific (TODO AIX is dataHeap, everything else is anyHeap)
    inferiorHeapType getDynamicHeapType() const; 
    
    // garbage collection instrumentation
    void gcInstrumentation();
    void gcInstrumentation(pdvector<pdvector<Frame> > &stackWalks);

    // Hybrid Mode
    bool triggerStopThread(Address pointAddress, int callbackID, void *calculation);
    Address stopThreadCtrlTransfer(instPoint *intPoint, Address target);

    // Event Handling
    void triggerNormalExit(int exitcode);

    // TODO this is temporary until ProcControl on Linux gives valid thread id's
    PCThread *getThreadByLWP(Dyninst::LWP lwp);

    // Misc

    // platform-specific, true if the OS says the process is running
    static bool getOSRunningState(int pid); 
    // platform-specific, needed for capability with previous versions of Dyninst that didn't
    // use ProcControlAPI
    static int getDefaultTermSignal(); 
    
    bool isInDebugSuicide() const;
    void setReportingEvent(bool b) { reportedEvent_ = b; }
    bool hasReportedEvent() const { return reportedEvent_; }
    void setExecing(bool b) { execing_ = b; }

    // ProcControl doesn't keep around a process's information after it exits.
    // However, we allow a Dyninst user to query certain information out of
    // an exited process
    void procControlProcExited() { pcProc_ = ProcControlAPI::Process::ptr(); }

    // Debugging
    bool setBreakpoint(Address addr);

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
    bool runningWhenAttached_;
    bool createdViaAttach_;
    processState_t processState_;
    bootstrapState_t bootstrapState_;
    int_function *main_function_;
    int curThreadIndex_;
    // true when Dyninst has reported an event to ProcControlAPI for this process
    bool reportedEvent_; // indicates the process should remain stopped
    int savedPid_; // ProcControl doesn't keep around Process objects after exit

    // Hybrid Analysis
    BPatch_hybridMode analysisMode_;
    int memoryPageSize_;

    // Active instrumentation tracking
    bool isAMcacheValid_;
    std::set<multiTramp *> activeMultis_;
    std::map<bblInstance *, Address> activeBBIs_;
    std::map<int_function *, std::set<Address> *> am_funcRelocs_;

    codeRangeTree signalHandlerLocations_;
    pdvector<mapped_object *> deletedObjects_;
    std::vector<heapItem *> dyninstRT_heaps_;
    pdvector<generatedCodeObject *> pendingGCInstrumentation_;

    // Misc.
    
    // Addresses of variables in RT library
    Address sync_event_id_addr_;
    Address sync_event_arg1_addr_;
    Address sync_event_arg2_addr_;
    Address sync_event_arg3_addr_;
    Address sync_event_breakpoint_addr_;

    // The same PCEventHandler held by the BPatch layer
    PCEventHandler *eventHandler_;

    syscallNotification *tracedSyscalls_;

    // TODO remove when inferiorMalloc machinery uses ProcControlAPI 
    // instead of the RT library
    Address rtLibLoadHeap_;

    mt_cache_result_t mt_cache_result_;

    bool isInDebugSuicide_; // Single stepping is only valid in this context

    // Stackwalking properties
    Address vsyscall_text_;
    Address vsyscall_start_;
    SymtabAPI::Symtab *vsyscall_obj_;
    Address vsyscall_end_;
    syscallStatus_t vsys_status_;
    AuxvParser *auxv_parser_;
};

class inferiorRPCinProgress : public codeRange {
public:
    inferiorRPCinProgress() :
        rpc(ProcControlAPI::IRPC::ptr()),
        rpcStartAddr(0),
        rpcCompletionAddr(0),
        resultRegister(REG_NULL),
        returnValue(NULL),
        runProcWhenDone(false),
        isComplete(false),
        deliverCallbacks(false),
        userData(NULL),
        thr(ProcControlAPI::Thread::ptr()),
        synchronous(false) {}

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
    ProcControlAPI::Thread::ptr thr;
    bool synchronous; // caller is responsible for cleaning up this object
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

#endif
