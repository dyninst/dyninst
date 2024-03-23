.. _`sec:dynProcess.h`:

dynProcess.h
############


This diagrams the startup flow of messages between the mutator and mutatee. Entry points
for create and attach process are both given.

.. code:: console

        Mutator           Signal              Mutatee
    Create:
        Fork/Exec
                        <-- Trap              Halted in exec (handled by ProcControlAPI)
        Install trap in main
                        <-- Trap              Halted in main
     Attach: (also paused, not in main)
        Install call to dlopen/
        LoadLibrary
                        <-- Trap              In library load
        Set parameters in library
                        <-- Trap              Finished loading
        Restore code and leave paused
        Finalize library
          If finalizing fails, init via iRPC

In all cases, the process is left paused at the entry of main
(create) or where it was (attach). No permanent instrumentation
is inserted.


.. cpp:class:: PCProcess : public AddressSpace

  **Encapsulates a ProcControlAPI Process**

  .. cpp:function:: static PCProcess *createProcess(const std::string file, std::vector<std::string> &argv, BPatch_hybridMode analysisMode, std::vector<std::string> &envp, const std::string dir, int stdin_fd, int stdout_fd, int stderr_fd)

    Process creation and control

  .. cpp:function:: static PCProcess *attachProcess(const std::string &progpath, int pid, BPatch_hybridMode analysisMode)
  .. cpp:function:: ~PCProcess()
  .. cpp:function:: static std::string createExecPath(const std::string &file, const std::string &dir)
  .. cpp:function:: virtual bool getDyninstRTLibName()
  .. cpp:function:: bool continueProcess()
  .. cpp:function:: bool stopProcess()
  .. cpp:function:: bool terminateProcess()
  .. cpp:function:: bool detachProcess(bool cont)
  .. cpp:function:: bool isBootstrapped() const

    true if Dyninst has finished it's initialization for the process

  .. cpp:function:: bool isAttached() const

    true if ok to operate on the process

  .. cpp:function:: bool isStopped() const

    true if the process is stopped

  .. cpp:function:: bool isForcedTerminating() const

    true if we're in the middle of a forced termination

  .. cpp:function:: bool isTerminated() const

    true if the process is terminated (either via normal exit or crash)

  .. cpp:function:: bool hasExitedNormally() const

    true if the process has exited(via a normal exit)

  .. cpp:function:: bool isExecing() const

    true if the process is in the middle of an exec

  .. cpp:function:: processState_t getDesiredProcessState() const
  .. cpp:function:: void setDesiredProcessState(processState_t ps)
  .. cpp:function:: bool dumpCore(std::string coreFile)

    platform-specific

  .. cpp:function:: bool writeDataSpace(void *inTracedProcess, u_int amount, const void *inSelf)
  .. cpp:function:: bool writeDataWord(void *inTracedProcess, u_int amount, const void *inSelf)
  .. cpp:function:: bool readDataSpace(const void *inTracedProcess, u_int amount, void *inSelf, bool displayErrMsg)
  .. cpp:function:: bool readDataWord(const void *inTracedProcess, u_int amount, void *inSelf, bool displayErrMsg)
  .. cpp:function:: bool writeTextSpace(void *inTracedProcess, u_int amount, const void *inSelf)
  .. cpp:function:: bool writeTextWord(void *inTracedProcess, u_int amount, const void *inSelf)
  .. cpp:function:: bool readTextSpace(const void *inTracedProcess, u_int amount, void *inSelf)
  .. cpp:function:: bool readTextWord(const void *inTracedProcess, u_int amount, void *inSelf)
  .. cpp:function:: unsigned getMemoryPageSize() const
  .. cpp:type:: ProcControlAPI::Process::mem_perm PCMemPerm
  .. cpp:function:: bool getMemoryAccessRights(Address start, PCMemPerm &rights)
  .. cpp:function:: bool setMemoryAccessRights(Address start, size_t size, PCMemPerm rights)
  .. cpp:function:: void changeMemoryProtections(Address addr, size_t size, PCMemPerm rights, bool setShadow)
  .. cpp:function:: PCThread *getInitialThread() const

  ......

  .. rubric::
    Process properties and fields

  .. cpp:function:: PCThread *getThread(dynthread_t tid) const
  .. cpp:function:: void getThreads(std::vector<PCThread *> &threads) const
  .. cpp:function:: void addThread(PCThread *thread)
  .. cpp:function:: bool removeThread(dynthread_t tid)
  .. cpp:function:: int getPid() const
  .. cpp:function:: bool wasRunningWhenAttached() const
  .. cpp:function:: bool wasCreatedViaAttach() const
  .. cpp:function:: bool wasCreatedViaFork() const
  .. cpp:function:: PCEventHandler *getPCEventHandler() const
  .. cpp:function:: int incrementThreadIndex()
  .. cpp:function:: bool walkStacks(std::vector<std::vector<Frame>> &stackWalks)

  ......

  .. rubric::
    Stackwalking

  .. cpp:function:: bool getAllActiveFrames(std::vector<Frame> &activeFrames)

    Return a vector (possibly with one object) of active frames in the process

  .. cpp:function:: Address inferiorMalloc(unsigned size, inferiorHeapType type = anyHeap, Address near_ = 0, bool *err = NULL)

  ......

  .. rubric::
    Inferior Malloc

  .. cpp:function:: void inferiorMallocConstraints(Address near, Address &lo, Address &hi, inferiorHeapType type)

    platform-specific

  .. cpp:function:: virtual void inferiorFree(Dyninst::Address)
  .. cpp:function:: virtual bool inferiorRealloc(Dyninst::Address, unsigned int)
  .. cpp:function:: bool mappedObjIsDeleted(mapped_object *obj)

  ......

  .. rubric::
    Instrumentation support

  .. cpp:function:: void installInstrRequests(const std::vector<instMapping *> &requests)

    A copy of the BPatch-level instrumentation installer

  .. cpp:function:: Address getTOCoffsetInfo(Address dest)

      platform-specific

  .. cpp:function:: Address getTOCoffsetInfo(func_instance *func)

      platform-specific

  .. cpp:function:: bool getOPDFunctionAddr(Address &opdAddr)

      architecture-specific

  .. cpp:function:: bool postIRPC(AstNodePtr action, void *userData, bool runProcessWhenDone, PCThread *thread, bool synchronous, void **result, bool userRPC, bool isMemAlloc = false, Address addr = 0)

  ......

  .. rubric::
    iRPC interface

  .. cpp:function:: bool postIRPC(void *buffer, int size, void *userData, bool runProcessWhenDone, PCThread *thread, bool synchronous, void **result, bool userRPC, bool isMemAlloc = false, Address addr = 0)
  .. cpp:function:: private bool postIRPC_internal(void *buffer, unsigned size, unsigned breakOffset, Register resultReg, Address addr, void *userData, bool runProcessWhenDone, PCThread *thread, bool synchronous, bool userRPC, bool isMemAlloc, void **result)

  ......

  .. rubric::
    Exploratory and Defensive mode stuff

  .. cpp:function:: BPatch_hybridMode getHybridMode()

  .. cpp:function:: bool getOverwrittenBlocks(std::map<Address, unsigned char *> &overwrittenPages,\
                                              std::list<std::pair<Address, Address>> &overwrittenRegions,\
                                              std::list<block_instance *> &writtenBBIs)

    Checks if blocks were overwritten.

    Initializes overwritten blocks and ranges by contrasting shadow pages with current memory contents

      - reads shadow pages in from memory
      - constructs overwritten region list
      - constructs overwrittn basic block list
      - determines if the last of the blocks has an abrupt end, in which case it marks it as overwritten

  .. cpp:function:: mapped_object *createObjectNoFile(Address addr)

    synch modified mapped objects with current memory contents

  .. cpp:function:: void updateCodeBytes(const std::list<std::pair<Address, Address>> &owRegions)

    Distribute the work to mapped_objects

  .. cpp:function:: bool isRuntimeHeapAddr(Address addr) const
  .. cpp:function:: bool isExploratoryModeOn() const
  .. cpp:function:: bool hideDebugger()

    platform-specific

  .. cpp:function:: void flushAddressCache_RT(Address start = 0, unsigned size = 0)

    Will flush addresses of all addresses in the specified range, if the range is null, flush all
    addresses from the cache.  Also flush  rt-lib heap addrs that correspond to the range.

  .. cpp:function:: void flushAddressCache_RT(codeRange *range)

  ......

  .. rubric::
    Active instrumentation tracking

  .. cpp:type:: std::pair<Address, Address> AddrPair
  .. cpp:type:: std::set<AddrPair> AddrPairSet
  .. cpp:type:: std::set<Address> AddrSet
  .. cpp:type:: std::list<ActiveDefensivePad> ADPList
  .. cpp:function:: bool patchPostCallArea(instPoint *point)
  .. cpp:function:: func_instance *findActiveFuncByAddr(Address addr)

    Given an address that's on the call stack, find the function that's actively executing that
    address.  This makes most sense for finding the address that's triggered a context switch back to
    Dyninst, either through instrumentation or a signal

  ......

  .. cpp:function:: std::vector<func_instance *> pcsToFuncs(std::vector<Frame> stackWalk)
  .. cpp:function:: virtual bool hasBeenBound(const SymtabAPI::relocationEntry &entry, func_instance *&target_pdf, Address base_addr)

    Architecture-specific check if the runtime linker has bound the function symbol corresponding to the
    relocation entry in at the address specified by entry and base_addr.

    If it has been bound, then the callee function is returned in "target_pdf", else it returns false.

    .. warning:: Only implemented on x86

  ......

  .. rubric::
    AddressSpace implementations
      
  .. cpp:function:: virtual Address offset() const
  .. cpp:function:: virtual Address length() const
  .. cpp:function:: virtual Architecture getArch() const
  .. cpp:function:: virtual bool multithread_capable(bool ignoreIfMtNotSet = false)

    If true is passed for ignore_if_mt_not_set, then an error won't be initiated if we're unable to
    determine if the program is multi-threaded. We are unable to determine this if the daemon hasn't yet
    figured out what libraries are linked against the application. Currently, we identify an
    application as being multi-threaded if it is linked against a thread library (eg. libpthreads.so on
    Linux). There are cases where we are querying whether the app is multi-threaded, but it can't be
    determined yet but it also isn't necessary to know.

  .. cpp:function:: virtual bool multithread_ready(bool ignoreIfMtNotSet = false)
  .. cpp:function:: virtual bool needsPIC()
  .. cpp:function:: virtual void addTrap(Address from, Address to, codeGen &gen)
  .. cpp:function:: virtual void removeTrap(Address from)

  ......

  .. rubric::
    Miscellaneous
    
  .. cpp:function:: void debugSuicide()

    debugSuicide is a kind of alternate debugging continueProc.  It runs the process until terminated in
    single step mode, printing each instruction as it executes.

  .. cpp:function:: bool dumpImage(std::string outFile)
  .. cpp:function:: bool walkStack(std::vector<Frame> &stackWalk, PCThread *thread)

  ......

  .. rubric::
    Stackwalking internals

  .. cpp:function:: bool getActiveFrame(Frame &frame, PCThread *thread)
  .. cpp:function:: void addSignalHandler(Address, unsigned)
  .. cpp:function:: bool isInSignalHandler(Address addr)
  .. cpp:function:: bool bindPLTEntry(const SymtabAPI::relocationEntry &, Address, func_instance *, Address)
  .. cpp:function:: bool supportsUserThreadEvents()
  .. cpp:function:: protected static PCProcess *setupExecedProcess(PCProcess *proc, std::string execPath)
  .. cpp:function:: protected PCProcess(ProcControlAPI::Process::ptr pcProc, std::string file, BPatch_hybridMode analysisMode)

    Process createexec constructor

  .. cpp:function:: protected PCProcess(ProcControlAPI::Process::ptr pcProc, BPatch_hybridMode analysisMode)

    Process attach constructor

  .. cpp:function:: protected static PCProcess *setupForkedProcess(PCProcess *parent, ProcControlAPI::Process::ptr pcProc)
  .. cpp:function:: protected PCProcess(PCProcess *parent, ProcControlAPI::Process::ptr pcProc)

    Process fork constructor

  .. cpp:function:: protected bool bootstrapProcess()

  ......

  .. rubric::
    Bootstrapping

  .. cpp:function:: protected bool hasReachedBootstrapState(bootstrapState_t state) const
  .. cpp:function:: protected void setBootstrapState(bootstrapState_t newState)
  .. cpp:function:: protected bool createStackwalker()
  .. cpp:function:: protected bool createStackwalkerSteppers()

    platform-specific

  .. cpp:function:: protected void createInitialThreads()
  .. cpp:function:: protected bool createInitialMappedObjects()
  .. cpp:function:: protected bool getExecFileDescriptor(std::string filename, bool waitForTrap, fileDescriptor &desc)
  .. cpp:function:: protected void findSignalHandler(mapped_object *obj)

    We keep a vector of all signal handler locations

  .. cpp:function:: protected void setMainFunction()

    ``NUMBER_OF_MAIN_POSSIBILITIES`` is defined in image.h

  .. cpp:function:: protected bool setAOut(fileDescriptor &desc)
  .. cpp:function:: protected bool hasPassedMain()

      OS-specific

  .. cpp:function:: protected bool insertBreakpointAtMain()
  .. cpp:function:: protected ProcControlAPI::Breakpoint::ptr getBreakpointAtMain() const
  .. cpp:function:: protected bool removeBreakpointAtMain()
  .. cpp:function:: protected Address getLibcStartMainParam(PCThread *thread)

      architecture-specific

  .. cpp:function:: protected bool copyDanglingMemory(PCProcess *parent)
  .. cpp:function:: protected void invalidateMTCache()
  .. cpp:function:: protected bool loadRTLib()

  ......

  .. rubric::
    RT library management

  .. cpp:function:: protected AstNodePtr createUnprotectStackAST()

    architecture-specific

  .. cpp:function:: protected bool setRTLibInitParams()

    Set up the parameters for DYNINSTinit in the RT lib

  .. cpp:function:: protected bool instrumentMTFuncs()
  .. cpp:function:: protected bool extractBootstrapStruct(DYNINST_bootstrapStruct *bs_record)
  .. cpp:function:: protected bool iRPCDyninstInit()
  .. cpp:function:: protected Address getRTEventBreakpointAddr()
  .. cpp:function:: protected Address getRTEventIdAddr()
  .. cpp:function:: protected Address getRTEventArg1Addr()
  .. cpp:function:: protected Address getRTEventArg2Addr()
  .. cpp:function:: protected Address getRTEventArg3Addr()
  .. cpp:function:: protected Address getRTTrapFuncAddr()

  ......

  .. rubric::
    Shared library managment

  .. cpp:function:: protected void addASharedObject(mapped_object *newObj)

    creates an image, creates new resources for a new shared object adds it to the collection of
    mapped_objects

  .. cpp:function:: protected void removeASharedObject(mapped_object *oldObj)

  ......

  .. rubric::
    Inferior heap management

  .. cpp:function:: protected void addInferiorHeap(mapped_object *obj)

    Given an image, add all static heaps inside it (DYNINSTstaticHeap...) to the buffer pool.

  .. cpp:function:: protected bool skipHeap(const heapDescriptor &heap)

    platform-specific

  .. cpp:function:: protected bool inferiorMallocDynamic(int size, Address lo, Address hi)
  .. cpp:function:: protected inferiorHeapType getDynamicHeapType() const

    platform-specific

  ......

  .. rubric::
    Hybrid Mode
  
  .. cpp:function:: protected bool triggerStopThread(Address pointAddress, int callbackID, void *calculation)

    - Need three pieces of information:
      - The instrumentation point that triggered the stopThread event (pointAddress)
      - The ID of the callback function given at the registration of the stopThread snippet
      - The result of the snippet calculation that was given by the user, if the point is a return instruction, read the return address
    - If the calculation is an address that is meant to be interpreted, do that
    - Invoke the callback

  .. cpp:function:: protected Address stopThreadCtrlTransfer(instPoint *intPoint, Address target)

    If calculation is a relocated address, translate it to the original addr.

    - case 1: The point is at a return instruction
    - case 2: The point is a control transfer into the runtime library Mark returning functions as returning Save the targets of indirect control transfers (not regular returns)


  .. cpp:function:: protected bool generateRequiredPatches(instPoint *callPt, AddrPairSet &)
  .. cpp:function:: protected void generatePatchBranches(AddrPairSet &)

  ......

  .. rubric::
    Event Handling
    
  .. cpp:function:: protected void triggerNormalExit(int exitcode)

  ......

  .. rubric::
    Misc

  .. cpp:function:: protected static void redirectFds(int stdin_fd, int stdout_fd, int stderr_fd, std::map<int, int> &result)
  .. cpp:function:: protected static bool setEnvPreload(std::vector<std::string> &envp, std::string fileName)

    platform-specific, sets LD_PRELOAD with RT library

  .. cpp:function:: protected bool isInDebugSuicide() const

  ......

  .. rubric::
    Event handling support

  .. cpp:function:: protected void setReportingEvent(bool b)
  .. cpp:function:: protected bool hasReportedEvent() const
  .. cpp:function:: protected void setExecing(bool b)
  .. cpp:function:: protected bool isInEventHandling() const
  .. cpp:function:: protected void setInEventHandling(bool b)
  .. cpp:function:: protected void setExiting(bool b)
  .. cpp:function:: protected bool isExiting() const
  .. cpp:function:: protected bool hasPendingEvents()
  .. cpp:function:: protected bool hasRunningSyncRPC() const
  .. cpp:function:: protected void addSyncRPCThread(Dyninst::ProcControlAPI::Thread::ptr thr)
  .. cpp:function:: protected void removeSyncRPCThread(Dyninst::ProcControlAPI::Thread::ptr thr)
  .. cpp:function:: protected bool continueSyncRPCThreads()
  .. cpp:function:: protected void markExited()

    ProcControl doesn't keep around a process's information after it exits. However, we allow a Dyninst user
    to query certain information out of an exited process. Just make sure no operations are attempted on the
    ProcControl process

  ......

  .. rubric::
    Debugging
    
  .. cpp:function:: protected bool setBreakpoint(Address addr)
  .. cpp:function:: protected void writeDebugDataSpace(void *inTracedProcess, u_int amount, const void *inSelf)
  .. cpp:function:: protected bool launchDebugger()
  .. cpp:function:: protected bool startDebugger()

    platform-specific

  .. cpp:function:: protected static void initSymtabReader()
  .. cpp:member:: protected ProcControlAPI::Process::ptr pcProc_

    Underlying ProcControl process

  .. cpp:member:: protected PCProcess *parent_
  .. cpp:member:: protected std::map<dynthread_t, PCThread *> threadsByTid_

    Corresponding threads

  .. cpp:member:: protected PCThread *initialThread_
  .. cpp:member:: protected ProcControlAPI::Breakpoint::ptr mainBrkPt_
  .. cpp:member:: protected std::string file_
  .. cpp:member:: protected bool attached_
  .. cpp:member:: protected bool execing_
  .. cpp:member:: protected bool exiting_
  .. cpp:member:: protected bool forcedTerminating_
  .. cpp:member:: protected bool runningWhenAttached_
  .. cpp:member:: protected bool createdViaAttach_
  .. cpp:member:: protected processState_t processState_
  .. cpp:member:: protected bootstrapState_t bootstrapState_
  .. cpp:member:: protected func_instance *main_function_
  .. cpp:member:: protected int curThreadIndex_
  .. cpp:member:: protected bool reportedEvent_

      indicates the process should remain stopped
      true when Dyninst has reported an event to ProcControlAPI for this process

  .. cpp:member:: protected int savedPid_

      ProcControl doesn't keep around Process objects after exit

  .. cpp:member:: protected Dyninst::Architecture savedArch_
  .. cpp:member:: protected BPatch_hybridMode analysisMode_

    Hybrid Analysis

  .. cpp:member:: protected codeRangeTree signalHandlerLocations_

    Active instrumentation tracking

  .. cpp:member:: protected std::vector<mapped_object *> deletedObjects_
  .. cpp:member:: protected std::vector<heapItem *> dyninstRT_heaps_
  .. cpp:member:: protected Address RT_address_cache_addr_

  ......

  .. rubric::
    Addresses of variables in RT library

  .. cpp:member:: protected Address sync_event_id_addr_
  .. cpp:member:: protected Address sync_event_arg1_addr_
  .. cpp:member:: protected Address sync_event_arg2_addr_
  .. cpp:member:: protected Address sync_event_arg3_addr_
  .. cpp:member:: protected Address sync_event_breakpoint_addr_
  .. cpp:member:: protected Address rt_trap_func_addr_
  .. cpp:member:: protected Address thread_hash_tids
  .. cpp:member:: protected Address thread_hash_indices
  .. cpp:member:: protected int thread_hash_size
  .. cpp:member:: protected PCEventHandler *eventHandler_

    The same PCEventHandler held by the BPatch layer

  .. cpp:member:: protected int eventCount_
  .. cpp:member:: protected syscallNotification *tracedSyscalls_
  .. cpp:member:: protected mt_cache_result_t mt_cache_result_
  .. cpp:member:: protected bool isInDebugSuicide_

    Single stepping is only valid in this context

  .. cpp:member:: protected baseTramp *irpcTramp_
  .. cpp:member:: protected bool inEventHandling_
  .. cpp:member:: protected std::set<Dyninst::ProcControlAPI::Thread::ptr> syncRPCThreads_
  .. cpp:member:: protected Dyninst::Stackwalker::Walker *stackwalker_
  .. cpp:member:: protected static Dyninst::SymtabAPI::SymtabReaderFactory *symReaderFactory_
  .. cpp:member:: protected std::map<Address, ProcControlAPI::Breakpoint::ptr> installedCtrlBrkpts


.. cpp:var:: static const unsigned MAX_THREADS = 32

  Should match ``MAX_THREADS`` in RTcommon.c

.. code:: c

  #define HEAP_DYN_BUF_SIZE (0x100000)

.. cpp:var:: static const Address ADDRESS_LO = ((Address)0x10000);
.. cpp:var:: static const Address ADDRESS_HI = ((Address)~((Address)0));


.. cpp:enum:: processState_t

  The desired state of the process, as indicated by the user

  .. cpp:enumerator:: ps_stopped
  .. cpp:enumerator:: ps_running


.. cpp:struct:: PCProcess::ActiveDefensivePad

  .. cpp:member:: Address activePC
  .. cpp:member:: Address padStart
  .. cpp:member:: block_instance *callBlock
  .. cpp:member:: block_instance *ftBlock
  .. cpp:function:: ActiveDefensivePad(Address a, Address b, block_instance *c, block_instance *d)


.. cpp:enum:: PCProcess::bootstrapState_t

  .. cpp:enumerator:: bs_attached
  .. cpp:enumerator:: bs_readyToLoadRTLib
  .. cpp:enumerator:: bs_loadedRTLib
  .. cpp:enumerator:: bs_initialized

    RT library has been loaded

.. cpp:enum:: PCProcess::mt_cache_result_t

  .. cpp:enumerator:: not_cached
  .. cpp:enumerator:: cached_mt_true
  .. cpp:enumerator:: cached_mt_false


.. cpp:class:: inferiorRPCinProgress : public codeRange

  .. cpp:function:: inferiorRPCinProgress()
  .. cpp:function:: virtual Address get_address() const
  .. cpp:function:: virtual unsigned get_size() const
  .. cpp:function:: virtual void *getPtrToInstruction(Address addr) const
  .. cpp:member:: ProcControlAPI::IRPC::ptr rpc
  .. cpp:member:: Address rpcStartAddr
  .. cpp:member:: Address rpcCompletionAddr
  .. cpp:member:: Register resultRegister

    register that contains the return value

  .. cpp:member:: void *returnValue
  .. cpp:member:: bool runProcWhenDone
  .. cpp:member:: bool isComplete
  .. cpp:member:: bool deliverCallbacks
  .. cpp:member:: void *userData
  .. cpp:member:: Dyninst::ProcControlAPI::Thread::ptr thread
  .. cpp:member:: bool synchronous

    caller is responsible for cleaning up this object

  .. cpp:member:: bool memoryAllocated

.. cpp:class:: signal_handler_location : public codeRange

  .. cpp:function:: signal_handler_location(Address addr, unsigned size)
  .. cpp:function:: Address get_address() const
  .. cpp:function:: unsigned get_size() const
  .. cpp:member:: private Address addr_
  .. cpp:member:: private unsigned size_

.. cpp:class:: StackwalkSymLookup : public Dyninst::Stackwalker::SymbolLookup

  .. cpp:member:: private PCProcess *proc_
  .. cpp:function:: StackwalkSymLookup(PCProcess *pc)
  .. cpp:function:: virtual bool lookupAtAddr(Dyninst::Address addr, std::string &out_name, void *&out_value)
  .. cpp:function:: virtual ~StackwalkSymLookup()

.. cpp:class:: StackwalkInstrumentationHelper : public Dyninst::Stackwalker::DyninstDynamicHelper

  .. cpp:member:: private PCProcess *proc_
  .. cpp:function:: StackwalkInstrumentationHelper(PCProcess *pc)
  .. cpp:function:: virtual bool isInstrumentation(Dyninst::Address ra, Dyninst::Address *orig_ra, unsigned *stack_height,\
                                                   bool *aligned, bool *entryExit)

    Checks if code at address ``ra`` is instrumentation.

    If in frameless instrumentation, ``orig_ra`` will contain the original address of the instruction
    or ``0``. The height of the stack is returned in ``stack_height``. ``entryExit`` will be set to
    ``true`` if this frame is entry/exit instrumentation.

  .. cpp:function:: virtual ~StackwalkInstrumentationHelper()

.. cpp:class:: DynFrameHelper : public Dyninst::Stackwalker::FrameFuncHelper

  .. cpp:member:: private PCProcess *proc_
  .. cpp:function:: DynFrameHelper(PCProcess *pc)
  .. cpp:function:: virtual Dyninst::Stackwalker::FrameFuncHelper::alloc_frame_t allocatesFrame(Address addr)
  .. cpp:function:: virtual ~DynFrameHelper()

.. cpp:class:: DynWandererHelper : public Dyninst::Stackwalker::WandererHelper

  .. cpp:member:: private PCProcess *proc_
  .. cpp:function:: DynWandererHelper(PCProcess *pc)
  .. cpp:function:: virtual bool isPrevInstrACall(Address addr, Address &target)
  .. cpp:function:: virtual Dyninst::Stackwalker::WandererHelper::pc_state isPCInFunc(Address func_entry, Address pc)
  .. cpp:function:: virtual bool requireExactMatch()
  .. cpp:function:: virtual ~DynWandererHelper()


Notes
*****

Why PCEventHandler is a friend

  PCProcess needs two interfaces: one that the rest of Dyninst sees and
  one that can be used to update the state of the PCProcess during event
  handling.

  The argument for having two different interfaces is that it will keep
  process control internals from bleeding out into the rest of Dyninst.
  This allows changes to the internals to have relatively low impact on the
  rest of Dyninst

Why PCEventMuxer is a friend

  Needed for some state queries that are protected
