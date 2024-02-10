.. _`sec:hybridAnalysis.h`:

hybridAnalysis.h
################

.. cpp:class:: HybridAnalysis

  There should only be one instance of this class, as for the :cpp:class:`BPatch` class

  .. cpp:function:: HybridAnalysis(BPatch_hybridMode mode, BPatch_process *proc)
  .. cpp:function:: ~HybridAnalysis()
  .. cpp:function:: bool init()

    sets up instrumentation, if there will be any

  .. cpp:function:: bool setMode(BPatch_hybridMode mode)

    returns false if conversion has no effect or is not possible

  .. cpp:function:: const HybridAnalysis::AnalysisStats &getStats()
  .. cpp:function:: HybridAnalysisOW *hybridOW()
  .. cpp:function:: BPatch_process *proc()
  .. cpp:function:: static InternalSignalHandlerCallback getSignalHandlerCB()
  .. cpp:function:: BPatch_module *getRuntimeLib()
  .. cpp:function:: void deleteSynchSnippet(SynchHandle *handle)
  .. cpp:function:: Dyninst::ProcControlAPI::Process::mem_perm getOrigPageRights(Dyninst::Address addr)
  .. cpp:function:: void addReplacedFuncs(std::vector<std::pair<BPatch_function *, BPatch_function *>> &repFs)
  .. cpp:function:: void getCallBlocks(Dyninst::Address retAddr, func_instance *retFunc, block_instance *retBlock, pair<ParseAPI::Block *, Dyninst::Address> &returningCallB, set<ParseAPI::Block *> &callBlocks)
  .. cpp:function:: std::map<BPatch_point *, SynchHandle *> &synchMap_pre()
  .. cpp:function:: std::map<BPatch_point *, SynchHandle *> &synchMap_post()

  ......

  .. rubric::
    Callbacks

  .. cpp:function:: bool isInLoop(Dyninst::Address blockAddr, bool activeOnly)
  .. cpp:function:: void netFuncCB(BPatch_point *point, void *)
  .. cpp:function:: void abruptEndCB(BPatch_point *point, void *)
  .. cpp:function:: void virtualFreeAddrCB(BPatch_point *point, void *)
  .. cpp:function:: void virtualFreeSizeCB(BPatch_point *point, void *)
  .. cpp:function:: void virtualFreeCB(BPatch_point *point, void *)
  .. cpp:function:: void badTransferCB(BPatch_point *point, void *returnValue)
  .. cpp:function:: void signalHandlerEntryCB(BPatch_point *point, Dyninst::Address excRecAddr)
  .. cpp:function:: void signalHandlerEntryCB2(BPatch_point *point, Dyninst::Address excCtxtAddr)
  .. cpp:function:: void signalHandlerCB(BPatch_point *pt, long snum, std::vector<Dyninst::Address> &handlers)
  .. cpp:function:: void signalHandlerExitCB(BPatch_point *point, void *dontcare)
  .. cpp:function:: void synchShadowOrigCB(BPatch_point *point, bool toOrig)
  .. cpp:function:: void overwriteSignalCB(Dyninst::Address faultInsnAddr, Dyninst::Address writeTarget)

  ......

  .. rubric::
    callback registering functions

  .. cpp:function:: bool registerCodeDiscoveryCallback(BPatchCodeDiscoveryCallback cb)
  .. cpp:function:: bool registerSignalHandlerCallback(BPatchSignalHandlerCallback cb)
  .. cpp:function:: bool removeCodeDiscoveryCallback()
  .. cpp:function:: bool removeSignalHandlerCallback()

  ......

  .. rubric::
    instrumentation functions
    
  .. cpp:function:: private bool instrumentModules(bool useInsertionSet)
  .. cpp:function:: private bool instrumentModule(BPatch_module *mod, bool useInsertionSet)
  .. cpp:function:: private bool instrumentFunction(BPatch_function *func, bool useInsertionSet, bool instrumentReturns = false, bool syncShadow = false)
  .. cpp:function:: private bool parseAfterCallAndInstrument(BPatch_point *callPoint, BPatch_function *calledFunc, bool foundByRet)
  .. cpp:function:: private void removeInstrumentation(BPatch_function *func, bool useInsertionSet, bool handlesWereDeleted = false)
  .. cpp:function:: private int saveInstrumentationHandle(BPatch_point *point, BPatchSnippetHandle *handle)
  .. cpp:function:: private bool hasEdge(BPatch_function *func, Dyninst::Address source, Dyninst::Address target)
  .. cpp:function:: private bool processInterModuleEdge(BPatch_point *point, Dyninst::Address target, BPatch_module *targMod)
  .. cpp:function:: private bool canUseCache(BPatch_point *pt)

  ......

  .. rubric::
    parsing

  .. cpp:function:: private void parseNewEdgeInFunction(BPatch_point *sourcePoint, Dyninst::Address target, bool useInsertionSet)
  .. cpp:function:: private bool analyzeNewFunction(BPatch_point *source, Dyninst::Address target, bool doInstrumentation, bool useInsertionSet)
  .. cpp:function:: private bool addIndirectEdgeIfNeeded(BPatch_point *srcPt, Dyninst::Address target)
  .. cpp:function:: private bool getCallAndBranchTargets(block_instance *block, std::vector<Address> &targs)

    utility functions that could go in another class, but that no one else really needs

  .. cpp:function:: private bool getCFTargets(BPatch_point *point, vector<Address> &targets)

  .. cpp:function:: void BPatch_process::overwriteAnalysisUpdate(std::map<Dyninst::Address,unsigned char*>& owPages, std::vector<std::pair<Dyninst::Address,int> >& deadBlocks, std::vector<BPatch_function*>& owFuncs, std::set<BPatch_function *> &monitorFuncs, bool &changedPages, bool &changedCode)

    needs to call removeInstrumentation

  .. cpp:member:: private std::map<Dyninst::Address, ExceptionDetails> handlerFunctions
  .. cpp:member:: private std::map<BPatch_function*, std::map<BPatch_point*, BPatchSnippetHandle*>*>* instrumentedFuncs
  .. cpp:member:: private std::map<BPatch_point *, SynchHandle *> synchMap_pre_

      maps from prePt

  .. cpp:member:: private std::map<BPatch_point *, SynchHandle *> synchMap_post_

    maps from postPt

  .. cpp:member:: private std::set<BPatch_function *> instShadowFuncs_
  .. cpp:member:: private std::set<std::string> skipShadowFuncs_
  .. cpp:member:: private std::map<BPatch_function *, BPatch_function *> replacedFuncs_
  .. cpp:member:: private std::set<BPatch_point *> cachePoints_
  .. cpp:member:: private BPatch_module *sharedlib_runtime
  .. cpp:member:: private BPatch_hybridMode mode_
  .. cpp:member:: private BPatch_process *proc_
  .. cpp:member:: private HybridAnalysisOW *hybridow_
  .. cpp:member:: private AnalysisStats stats_
  .. cpp:member:: private BPatchCodeDiscoveryCallback bpatchCodeDiscoveryCB
  .. cpp:member:: private BPatchSignalHandlerCallback bpatchSignalHandlerCB
  .. cpp:member:: private Dyninst::Address virtualFreeAddr_
  .. cpp:member:: private unsigned virtualFreeSize_


.. cpp:class:: HybridAnalysis::SynchHandle

  .. cpp:function:: SynchHandle(BPatch_point* prePt, BPatchSnippetHandle* preHandle)
  .. cpp:function:: void setPostHandle(BPatch_point* postPt, BPatchSnippetHandle* postHandle)
  .. cpp:member:: BPatch_point *prePt_
  .. cpp:member:: BPatch_point *postPt_
  .. cpp:member:: BPatchSnippetHandle *preHandle_
  .. cpp:member:: BPatchSnippetHandle *postHandle_


.. cpp:struct:: HybridAnalysis::ExceptionDetails

  .. cpp:member:: Dyninst::Address faultPCaddr
  .. cpp:member:: bool isInterrupt


.. cpp:class:: HybridAnalysis::AnalysisStats

  .. cpp:function:: AnalysisStats()
  .. cpp:member:: int exceptions
  .. cpp:member:: int winApiCallbacks
  .. cpp:member:: int unpackCount
  .. cpp:member:: int owCount
  .. cpp:member:: int owBytes
  .. cpp:member:: int owExecFunc
  .. cpp:member:: int owFalseAlarm


.. cpp:struct:: HybridAnalysis::blockcmp

  no possibility of equality, return true if b1 < b2

  .. cpp:function:: bool operator()(const BPatch_basicBlock *b1, const BPatch_basicBlock *b2) const



.. cpp:class:: HybridAnalysisOW

  .. cpp:function:: HybridAnalysisOW(HybridAnalysis *hybrid)
  .. cpp:function:: ~HybridAnalysisOW()
  .. cpp:function:: HybridAnalysis *hybrid()
  .. cpp:function:: BPatch_process *proc()
  .. cpp:function:: HybridAnalysisOW::owLoop *findLoop(Dyninst::Address blockStart)
  .. cpp:function:: bool isInLoop(Dyninst::Address blockAddr, bool activeOnly)
  .. cpp:function:: bool registerCodeOverwriteCallbacks(BPatchCodeOverwriteBeginCallback cbBegin, BPatchCodeOverwriteEndCallback cbEnd)
  .. cpp:function:: bool removeCodeOverwriteCallbacks()
  .. cpp:function:: bool codeChangeCB(std::vector<BPatch_function *> &modfuncs)
  .. cpp:function:: bool hasLoopInstrumentation(bool activeOnly, BPatch_function &func, std::set<HybridAnalysisOW::owLoop *> *loops = NULL)

    overwrite loop functions

  .. cpp:function:: bool getActiveLoops(std::vector<HybridAnalysisOW::owLoop *> &active)
  .. cpp:function:: bool activeOverwritePages(std::set<Dyninst::Address> &pages)
  .. cpp:function:: bool deleteLoop(HybridAnalysisOW::owLoop *loop, bool useInsertionSet, BPatch_point *writePoint = NULL, bool uninstrument = true)

     | 1. Check for changes to the underlying code to see if this is safe to do
     | 2. If the loop is active, check for changes to the underlying data, and
     |    if no changes have occurred, we can just remove the loop instrumentation
     |    and everything will be hunky dory once we re-instate the write
     |    protections for the loop's pages
     | return true if the loop was active


  .. cpp:function:: void overwriteSignalCB(Dyninst::Address faultInsnAddr, Dyninst::Address writeTarget)

    Informs the mutator that an instruction will write to a page that contains analyzed code. This function decides
    where to put the instrumentation that will mark the end of the overwriting phase

  .. cpp:function:: void overwriteAnalysis(BPatch_point *point, void *loopID_)
  .. cpp:function:: static InternalCodeOverwriteCallback getCodeOverwriteCB()
  .. cpp:function:: private bool removeLoop(HybridAnalysisOW::owLoop *loop, bool useInsertionSet, BPatch_point *writePoint = NULL, bool uninstrument = true)

    helper to deleteLoop, does not delete loop or its shadowMap

  .. cpp:function:: private BPatch_basicBlockLoop *getWriteLoop(BPatch_function &func, Dyninst::Address writeAddr, bool allowParentLoop = true)

    gets biggest loop without unresolvedmultiply resolved indirect ctrl flow that it can find

  .. cpp:function:: private BPatch_basicBlockLoop *getParentLoop(BPatch_function &func, Dyninst::Address writeAddr)
  .. cpp:function:: private bool addFuncBlocks(owLoop *loop, std::set<BPatch_function *> &addFuncs, std::set<BPatch_function *> &seenFuncs, std::set<int> &overlappingLoops)

    recursively add all functions that contain calls, return true if the function contains no unresolved control flow and the function returns normally

  .. cpp:function:: private bool setLoopBlocks(owLoop *loop, BPatch_basicBlockLoop *writeLoop, std::set<int> &overlappingLoops)

    if writeLoop is null, return the whole function in the loop. returns true if we were able to identify all code in the loop

  .. cpp:function:: private bool removeOverlappingLoops(owLoop *loop, std::set<int> &overlappingLoops)

    returns true if the loop blocks are a superset of the loop(s) it overlaps

  .. cpp:function:: private void makeShadow_setRights(Dyninst::Address pageAddr, owLoop *loop)

    remove any coverage instrumentation make a shadow page, restore write privileges to the page,

  .. cpp:function:: private bool isRealStore(Dyninst::Address insnAddr, block_instance *blk, BPatch_function *func)
  .. cpp:member:: private HybridAnalysis *hybrid_
  .. cpp:member:: private std::set<owLoop *> loops
  .. cpp:member:: private std::map<Dyninst::Address, int> blockToLoop

    KEVINCOMMENT: makes non-guaranteed assumption that only one loop per block, would it be better to use the last instruction address?

  .. cpp:member:: private std::map<int, owLoop *> idToLoop
  .. cpp:member:: private map<Dyninst::Address, int> writeHits

    number of times a write instruction has hit, used to trigger stackwalks for finding inter-function loops when number of
    hits exceeds a threshold

  .. cpp:member:: private static const int HIT_THRESHOLD = 0
  .. cpp:member:: private BPatchCodeOverwriteBeginCallback bpatchBeginCB
  .. cpp:member:: private BPatchCodeOverwriteEndCallback bpatchEndCB



.. cpp:class:: HybridAnalysisOW::owLoop

  .. cpp:function:: owLoop(HybridAnalysisOW *hybridow, Dyninst::Address writeTarg)
  .. cpp:function:: ~owLoop()
  .. cpp:function:: static int getNextLoopId()
  .. cpp:function:: bool isActive() const
  .. cpp:function:: bool writesOwnPage() const
  .. cpp:function:: bool isRealLoop() const
  .. cpp:function:: int getID() const
  .. cpp:function:: Dyninst::Address getWriteTarget()
  .. cpp:function:: void setWriteTarget(Dyninst::Address targ)
  .. cpp:function:: void setWritesOwnPage(bool wop)
  .. cpp:function:: void setActive(bool act)
  .. cpp:function:: void instrumentOverwriteLoop(Dyninst::Address writeInsnAddr)

     | 1. Gather up all instrumentation sites that need to be monitored:
     |   1a. The edges of all instrumented blocks that leave the block set
     |   1b. Unresolved points in instrumented blocks
     | 2. Instrument exit edges and unresolved points with callbacks to the analysis update routine
     |   2a. Instrument at loop exit edges
     |   2b. Instrument at unresolved edges in the loop

  .. cpp:function:: void instrumentOneWrite(Dyninst::Address writeInsnAddr, std::vector<BPatch_function *> writeFuncs)
  .. cpp:function:: void instrumentLoopWritesWithBoundsCheck()

      | 1. initialize necessary variables
      | 2. create bounds array for all blocks in the loop
      | 3. create the bounds check function call snippet
      | 4. instrument each write point

  .. cpp:member:: std::set<BPatchSnippetHandle *> snippets
  .. cpp:member:: std::map<Dyninst::Address, unsigned char *> shadowMap
  .. cpp:member:: std::set<Dyninst::Address> writeInsns
  .. cpp:member:: std::set<BPatch_basicBlock *, HybridAnalysis::blockcmp> blocks

    loop blocks

  .. cpp:member:: std::set<BPatch_point *> unresExits_

    unresolved control transfers that we treat as exit points

  .. cpp:member:: private Dyninst::Address writeTarget_

    write target, set to 0 if loop has multiple write targets

  .. cpp:member:: private bool activeStatus_

    loop active status

  .. cpp:member:: private bool writesOwnPage_

    loop writes own page

  .. cpp:member:: private bool realLoop_

    real loop if we're instrumenting loop exit edges, not immediately after the write instruction

  .. cpp:member:: private HybridAnalysisOW *hybridow_
  .. cpp:member:: private int loopID_
  .. cpp:member:: private static int IDcounter_

