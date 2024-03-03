.. _`sec:dyninstAPI:function.h`:

function.h
##########


.. cpp:class:: func_instance : public patchTarget, public Dyninst::PatchAPI::PatchFunction

  .. cpp:function:: func_instance(parse_func *f, Dyninst::Address baseAddr, mapped_module *mod)

    Almost everythcing gets filled in later.

  .. cpp:function:: func_instance(const func_instance *parent, mapped_module *child_mod)
  .. cpp:function:: ~func_instance()

    We don't delete blocks, since they're shared between functions. We *do* delete context instPoints,
    though Except that should get taken care of normally since the structures are static.

  ......

  .. rubric::
    Passthrough functions.To minimize wasted memory (since there will be many copies of this function) we make
    most methods passthroughs to the original parsed version.
    
  .. cpp:function:: std::string symTabName() const
  .. cpp:function:: std::string prettyName() const
  .. cpp:function:: std::string typedName() const
  .. cpp:function:: std::string name() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter symtab_names_begin() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter symtab_names_end() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter pretty_names_begin() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter pretty_names_end() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter typed_names_begin() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter typed_names_end() const

  .....

  .. rubric::
    Debuggering functions

  .. cpp:function:: void debugPrint() const

    warning: doesn't (and can't) force initialization of lazily-built
    data structures because this function is declared to be constant

  .. cpp:function:: void addSymTabName(const std::string name, bool isPrimary = false)

    Add to mapped_object if a "new" name (true return from internal).

  .. cpp:function:: void addPrettyName(const std::string name, bool isPrimary = false)
  .. cpp:function:: Dyninst::Address getPtrAddress() const
  .. cpp:function:: parse_func *ifunc() const

    Not defined here so we don't have to play header file magic Not const we can add names via the Dyninst layer

  .. cpp:function:: mapped_module *mod() const
  .. cpp:function:: mapped_object *obj() const
  .. cpp:function:: AddressSpace *proc() const
  .. cpp:function:: std::string format() const

  .....

  .. rubric::
    CFG and other function body methods

  .. cpp:type:: AddrOrderedBlockSet BlockSet
  .. cpp:function:: block_instance *entryBlock()

  .. rubric::
    Defensive mode stuff

  .. cpp:function:: const BlockSet &unresolvedCF()

    Blocks that have a sink target, essentially

  .. cpp:function:: const BlockSet &abruptEnds()

    Blocks where we provisionally stopped

  .. cpp:function:: block_instance *setNewEntry(block_instance *def, std::set<block_instance *> &deadBlocks)

    The original entry block is gone, we choose a new entry block from the function, whichever non-dead
    block we can find that has no intraprocedural incoming edges. If there's no obvious block to
    choose, we stick with the default block.

  .. cpp:function:: bool isSignalHandler()

    kevin signal-handler information

  .. cpp:function:: Dyninst::Address getHandlerFaultAddr()
  .. cpp:function:: Dyninst::Address getHandlerFaultAddrAddr()
  .. cpp:function:: void setHandlerFaultAddr(Dyninst::Address fa)
  .. cpp:function:: void setHandlerFaultAddrAddr(Dyninst::Address faa, bool set)

    Sets the address in the structure at which the fault instruction's address is stored if "set" is
    true. Accesses the fault address and translates it back to an original address if it corresponds to
    relocated code in the Dyninst heap

  .. cpp:function:: void triggerModified()
  .. cpp:function:: block_instance *getBlockByEntry(const Dyninst::Address addr)
  .. cpp:function:: bool getBlocks(const Dyninst::Address addr, std::set<block_instance *> &blks)

    Return in ``blks`` all blocks that have an instruction starting at ``addr``.

    If there are none, return all blocks containing ``addr``.

  .. cpp:function:: block_instance *getBlock(const Dyninst::Address addr)

    Get the block with an instruction that starts at addr

  .. cpp:function:: Offset addrToOffset(const Dyninst::Address addr) const
  .. cpp:function:: bool hasNoStackFrame() const
  .. cpp:function:: bool savesFramePointer() const
  .. cpp:function:: func_instance *getNoPowerPreambleFunc()
  .. cpp:function:: void setNoPowerPreambleFunc(func_instance *f)
  .. cpp:function:: func_instance *getPowerPreambleFunc()
  .. cpp:function:: void setPowerPreambleFunc(func_instance *f)
  .. cpp:function:: func_instance *findCallee(block_instance *callBlock)

    Legacy/inter-module calls. Arguably should be an interprocedural edge, but I expect that would break all manner of things

  .. cpp:function:: bool isInstrumentable()
  .. cpp:function:: Dyninst::Address get_address() const
  .. cpp:function:: unsigned get_size() const
  .. cpp:function:: unsigned footprint()

    not const, calls ifunc()->extents()

  .. cpp:function:: std::string get_name() const
  .. cpp:function:: bool setReturnValue(int val)

    Replaces the function with a 'return val' statement.  currently needed only on Linuxx86  Defined in inst-x86.C

  ......

  .. rubric::
    Code overlapping
    
  .. cpp:function:: bool getSharingFuncs(block_instance *b, std::set<func_instance *> &funcs)

    Get all functions that "share" the block. Actually, the block_instance will not be shared (they are per
    function), but the underlying parse_block records the sharing status. So dodge through to the image layer
    and find out that info. Returns true if such functions exist.

    Dig down to the low-level block of b, find the low-level functions that share it, and map up to
    int-level functions and add them to the funcs list.

  .. cpp:function:: bool getSharingFuncs(std::set<func_instance *> &funcs)

    The same, but for any function that overlaps with any of our basic blocks.

    Find sharing functions via checking all basic blocks. We might be able to check only exit points;
    but we definitely need to check _all_ exits so for now we're checking everything.

    OPTIMIZATION: we're not checking all blocks, only an exit point this _should_ work :) but needs to change
    if we ever do flow-sensitive parsing

  .. cpp:function:: bool getOverlappingFuncs(std::set<func_instance *> &funcs)

    Slower version of the above that also finds functions that occupy the same address range, even if they do not
    share blocks - this can be caused by overlapping but disjoint assembly sequences

  .. cpp:function:: bool getOverlappingFuncs(block_instance *b, std::set<func_instance *> &funcs)

  ......

  .. rubric::
    Misc

  .. cpp:function:: const std::vector<int_parRegion *> &parRegions()
  .. cpp:function:: bool containsSharedBlocks() const
  .. cpp:function:: unsigned getNumDynamicCalls()
  .. cpp:function:: template <class OutputIterator> void getCallerBlocks(OutputIterator result)

    Fill the <callers> vector with pointers to the statically-determined list of functions that call this function.

  .. cpp:function:: template <class OutputIterator> void getCallerFuncs(OutputIterator result)
  .. cpp:function:: bool getLiveCallerBlocks(const std::set<block_instance *> &deadBlocks,\
                                             const std::list<func_instance *> &deadFuncs, std::map<Dyninst::Address,\
                                             vector<block_instance *>> &output_stubs)

    Get caller blocks that aren't in deadBlocks

  .. cpp:function:: bool savesReturnAddr() const
  .. cpp:function:: callType func_instance::getCallingConvention()

    Calling convention for this function

  .. cpp:function:: int getParamSize()
  .. cpp:function:: void setParamSize(int s)
  .. cpp:function:: void getReachableBlocks(const std::set<block_instance *> &exceptBlocks, const std::list<block_instance *> &seedBlocks, std::set<block_instance *> &reachBlocks)
  .. cpp:function:: bool consistency() const

    - Check for ``1:1`` block relationship in the block list and block map
    - Check that all instPoints are in the correct block.

  .. cpp:function:: instPoint *funcEntryPoint(bool create)

    Wrappers for patchapi findPoints to find a single instPoint

  .. cpp:function:: instPoint *funcExitPoint(block_instance *blk, bool create)
  .. cpp:function:: instPoint *preCallPoint(block_instance *blk, bool create)
  .. cpp:function:: instPoint *postCallPoint(block_instance *blk, bool create)
  .. cpp:function:: instPoint *blockEntryPoint(block_instance *blk, bool create)
  .. cpp:function:: instPoint *blockExitPoint(block_instance *b, bool create)
  .. cpp:function:: instPoint *preInsnPoint(block_instance *b, Dyninst::Address a, Dyninst::InstructionAPI::Instruction insn, bool trusted, bool create)
  .. cpp:function:: instPoint *postInsnPoint(block_instance *b, Dyninst::Address a, Dyninst::InstructionAPI::Instruction insn, bool trusted, bool create)
  .. cpp:function:: instPoint *edgePoint(edge_instance *eg, bool create)
  .. cpp:type:: std::vector<instPoint *> Points

    Wrappers for patchapi findPoints to find all instPoints w certain type

  .. cpp:function:: void funcExitPoints(Points *)
  .. cpp:function:: void callPoints(Points *)
  .. cpp:function:: void blockInsnPoints(block_instance *, Points *)
  .. cpp:function:: void edgePoints(Points *)

  ......

  .. rubric::
    Function wrapping

  .. cpp:function:: bool addSymbolsForCopy()
  .. cpp:function:: bool updateRelocationsToSym(Dyninst::SymtabAPI::Symbol *oldsym, Dyninst::SymtabAPI::Symbol *newsym)
  .. cpp:function:: Dyninst::SymtabAPI::Symbol *getWrapperSymbol()
  .. cpp:function:: Dyninst::SymtabAPI::Symbol *getRelocSymbol()
  .. cpp:function:: void createWrapperSymbol(Dyninst::Address entry, std::string name)
  .. cpp:function:: static void destroy(func_instance *f)
  .. cpp:function:: void removeBlock(block_instance *block)
  .. cpp:function:: void split_block_cb(block_instance *b1, block_instance *b2)
  .. cpp:function:: void add_block_cb(block_instance *block)
  .. cpp:function:: virtual void markModified()

  ......

  .. rubric::
    Stack modification

  .. cpp:function:: void addParam(Dyninst::SymtabAPI::localVar *p)
  .. cpp:function:: void addVar(Dyninst::SymtabAPI::localVar *v)
  .. cpp:function:: std::set<Dyninst::SymtabAPI::localVar *> getParams() const
  .. cpp:function:: std::set<Dyninst::SymtabAPI::localVar *> getVars() const
  .. cpp:function:: void setStackMod(bool b)
  .. cpp:function:: bool hasStackMod() const
  .. cpp:function:: void addMod(StackMod *m, TMap *tMap)
  .. cpp:function:: void removeMod(StackMod *m)
  .. cpp:function:: std::set<StackMod *> *getMods() const
  .. cpp:function:: void printMods() const
  .. cpp:function:: Accesses *getAccesses(Dyninst::Address addr)
  .. cpp:function:: void setCanary(bool b)
  .. cpp:function:: bool hasCanary()
  .. cpp:function:: bool hasRandomize()
  .. cpp:function:: bool hasOffsetVector() const
  .. cpp:function:: bool hasValidOffsetVector() const
  .. cpp:function:: bool createOffsetVector()
  .. cpp:function:: OffsetVector *getOffsetVector() const
  .. cpp:function:: TMap *getTMap() const
  .. cpp:function:: void replaceTMap(TMap *newTMap)
  .. cpp:function:: std::map<Dyninst::Address, StackAccess *> *getDefinitionMap()
  .. cpp:function:: bool randomize(TMap *tMap, bool seeded = false, int seed = -1)
  .. cpp:function:: void freeStackMod()
  .. cpp:function:: bool operator<(func_instance &rhs)
  .. cpp:function:: private void removeAbruptEnd(const block_instance *)

    helper func for block_instance::setNotAbruptEnd(), do not call directly

  .. cpp:member:: private Dyninst::Address ptrAddr_

    Absolute address of the function descriptor, if exists

  .. cpp:member:: private mapped_module *mod_

    This is really a dodge translate a list of parse_funcs to int_funcs

  ......

  .. rubric::
    CFG and function body Defensive mode
    
  .. cpp:member:: private BlockSet unresolvedCF_
  .. cpp:member:: private BlockSet abruptEnds_
  .. cpp:member:: private size_t prevBlocksAbruptEnds_

    num func blocks when calculated

  .. cpp:member:: private Dyninst::Address handlerFaultAddr_

    if this is a signal handler, ``faultAddr_`` is
    set to -1, or to the address of the fault
    that last caused the handler to be invoked.

  .. cpp:member:: private Dyninst::Address handlerFaultAddrAddr_

  ......

  .. rubric::
    Parallel Regions

  .. cpp:member:: private std::vector<int_parRegion *> parallelRegions_

    pointer to the parallel regions

  .. cpp:function:: private void addblock_instance(block_instance *instance)
  .. cpp:member:: private callType callingConv
  .. cpp:member:: private int paramSize
  .. cpp:member:: private Dyninst::SymtabAPI::Symbol *wrapperSym_

  ......

  .. rubric::
    Stack modification

  .. cpp:function:: private bool createOffsetVector_Symbols()
  .. cpp:function:: private bool createOffsetVector_Analysis(Dyninst::ParseAPI::Function *func, Dyninst::ParseAPI::Block *block, Dyninst::InstructionAPI::Instruction insn, Dyninst::Address addr)
  .. cpp:function:: private bool addToOffsetVector(StackAnalysis::Height off, int size, StackAccess::StackAccessType type, bool isRegisterHeight, ValidPCRange *valid, MachRegister reg = MachRegister())
  .. cpp:function:: private void createTMap_internal(StackMod *mod, StackLocation *loc, TMap *tMap)
  .. cpp:function:: private void createTMap_internal(StackMod *mod, TMap *tMap)
  .. cpp:member:: private std::set<Dyninst::SymtabAPI::localVar *> _params
  .. cpp:member:: private std::set<Dyninst::SymtabAPI::localVar *> _vars
  .. cpp:member:: private bool _hasDebugSymbols
  .. cpp:member:: private bool _hasStackMod
  .. cpp:member:: private std::set<StackMod *> *_modifications
  .. cpp:member:: private bool _seeded
  .. cpp:member:: private int _seed
  .. cpp:member:: private bool _randomizeStackFrame
  .. cpp:member:: private bool _hasCanary
  .. cpp:member:: private bool _processedOffsetVector
  .. cpp:member:: private bool _validOffsetVector
  .. cpp:member:: private OffsetVector *_offVec
  .. cpp:member:: private set<tmpObject, less_tmpObject> *_tmpObjects
  .. cpp:member:: private TMap *_tMap

    Records transformations to known stack locations and stack pointers due to stack modifications.

  .. cpp:member:: private std::map<Dyninst::Address, Accesses *> *_accessMap

    Records known accesses to stack locations (so we can determine how to modify the accesses for stack modifications).

  .. cpp:member:: private std::map<Dyninst::Address, StackAccess *> *_definitionMap

    Records stack pointer definitions that need to be modified for stack modifications.

  .. cpp:member:: private func_instance *_noPowerPreambleFunc
  .. cpp:member:: private func_instance *_powerPreambleFunc


.. cpp:enum:: callType

  .. cpp:enumerator:: unknown_call
  .. cpp:enumerator:: cdecl_call
  .. cpp:enumerator:: stdcall_call
  .. cpp:enumerator:: fastcall_call
  .. cpp:enumerator:: thiscall_call

