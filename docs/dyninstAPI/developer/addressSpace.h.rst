.. _`sec:addressSpace.h`:

addressSpace.h
##############

.. important::
  Historically, we only use SIGTRAP as the signal for tramopline.
  However, SIGTRAP is always intercepted by GDB, causing it is
  almost impossible to debug through signal trampolines.
  Here, we add a new environment variable DYNINST_SIGNAL_TRAMPOLINE_SIGILL
  to control whether we use SIGILL as the signal for trampolines.
  In the case of binary rewriting, DYNINST_SIGNAL_TRAMPOLINE_SIGILL should be
  consistently set or unset for rewriting the binary and running the rewritten binaries.

.. cpp:class:: AddressSpace : public InstructionSource

  .. cpp:type:: boost::shared_ptr<Dyninst::InstructionAPI::Instruction> InstructionPtr

  .. cpp:member:: trampTrapMappings trapMapping
  .. cpp:member:: std::set<mapped_object *> runtime_lib
  .. cpp:member:: std::string dyninstRT_name

  .. cpp:function:: PCProcess *proc()
  .. cpp:function:: BinaryEdit *edit()
  .. cpp:function:: virtual bool readDataWord(const void *inOther, u_int amount, void *inSelf, bool showError) = 0
  .. cpp:function:: virtual bool readDataSpace(const void *inOther, u_int amount, void *inSelf, bool showError) = 0
  .. cpp:function:: virtual bool readTextWord(const void *inOther, u_int amount, void *inSelf) = 0
  .. cpp:function:: virtual bool readTextSpace(const void *inOther, u_int amount, void *inSelf) = 0
  .. cpp:function:: virtual bool writeDataWord(void *inOther, u_int amount, const void *inSelf) = 0
  .. cpp:function:: virtual bool writeDataSpace(void *inOther, u_int amount, const void *inSelf) = 0
  .. cpp:function:: virtual bool writeTextWord(void *inOther, u_int amount, const void *inSelf) = 0
  .. cpp:function:: virtual bool writeTextSpace(void *inOther, u_int amount, const void *inSelf) = 0
  .. cpp:function:: Address getTOCoffsetInfo(func_instance *)

    Symtab has this information on a per-Function basis. It's kinda nontrivial to get a Function object out of a
    func_instance; instead we use its entry address which is what all the TOC data structures are written in terms
    of anyway.

  .. cpp:function:: virtual Address inferiorMalloc(unsigned size, inferiorHeapType type = anyHeap, Address near = 0, bool *err = NULL) = 0
  .. cpp:function:: virtual void inferiorFree(Address item) = 0
  .. cpp:function:: void inferiorFreeInternal(Address item)
  .. cpp:function:: virtual bool inferiorRealloc(Address item, unsigned newSize) = 0
  .. cpp:function:: bool inferiorReallocInternal(Address item, unsigned newSize)
  .. cpp:function:: bool inferiorShrinkBlock(heapItem *h, Address block, unsigned newSize)
  .. cpp:function:: bool inferiorExpandBlock(heapItem *h, Address block, unsigned newSize)

    We attempt to find a free block that immediately succeeds this one. If we find such a block we expand this block into
    the next; if this is possible we return true. Otherwise we return false.

  .. cpp:function:: bool isInferiorAllocated(Address block)

    Checks if memory was allocated for a variable starting at address ``block``.

  .. cpp:function:: virtual void addTrap(Address from, Address to, codeGen &gen) = 0
  .. cpp:function:: virtual void removeTrap(Address from) = 0
  .. cpp:function:: virtual bool getDyninstRTLibName()
  .. cpp:function:: virtual bool isValidAddress(const Address) const
  .. cpp:function:: virtual void *getPtrToInstruction(const Address) const

      Get me a pointer to the instruction: the return is a local (mutator-side) store for the mutatee. This may
      duck into the local copy for images, or a relocated function's self copy.

  .. cpp:function:: virtual void *getPtrToData(const Address a) const
  .. cpp:function:: bool usesDataLoadAddress() const
  .. cpp:function:: virtual bool isCode(const Address) const
  .. cpp:function:: virtual bool isData(const Address) const
  .. cpp:function:: virtual bool isReadOnly(const Address) const
  .. cpp:function:: virtual Address offset() const = 0
  .. cpp:function:: virtual Address length() const = 0
  .. cpp:function:: virtual Architecture getArch() const = 0
  .. cpp:function:: bool findFuncsByAll(const std::string &funcname, std::vector<func_instance *> &res, const std::string &libname = "")
  .. cpp:function:: bool findFuncsByPretty(const std::string &funcname, std::vector<func_instance *> &res, const std::string &libname = "")
  .. cpp:function:: bool findFuncsByMangled(const std::string &funcname, std::vector<func_instance *> &res, const std::string &libname = "")
  .. cpp:function:: bool findVarsByAll(const std::string &varname, std::vector<int_variable *> &res, const std::string &libname = "")
  .. cpp:function:: virtual func_instance *findOnlyOneFunction(const std::string &name, const std::string &libname = "", bool search_rt_lib = true)
  .. cpp:function:: bool getSymbolInfo(const std::string &name, int_symbol &ret)

     Returns the named symbol from the image or a shared object

  .. cpp:function:: void getAllFunctions(std::vector<func_instance *> &)

    Returns all functions defined in the a.out and in the shared objects.

  .. cpp:function:: bool findFuncsByAddr(Address addr, std::set<func_instance *> &funcs, bool includeReloc = false)
  .. cpp:function:: bool findBlocksByAddr(Address addr, std::set<block_instance *> &blocks, bool includeReloc = false)
  .. cpp:function:: func_instance *findOneFuncByAddr(Address addr)
  .. cpp:function:: func_instance *findFuncByEntry(Address addr)
  .. cpp:function:: block_instance *findBlockByEntry(Address addr)
  .. cpp:function:: func_instance *findFunction(parse_func *ifunc)
  .. cpp:function:: block_instance *findBlock(parse_block *iblock)
  .. cpp:function:: edge_instance *findEdge(ParseAPI::Edge *iedge)
  .. cpp:function:: func_instance *findFuncByEntry(const block_instance *block)
  .. cpp:function:: func_instance *findJumpTargetFuncByAddr(Address addr)

      Acts like :cpp:func:`findTargetFuncByAddr`, but also finds the function if ``addr`` is an indirect jump to a function.

  .. cpp:function:: bool sameRegion(Dyninst::Address addr1, Dyninst::Address addr2)
  .. cpp:function:: mapped_module *findModule(const std::string &mod_name, bool wildcard = false)

    Returns the module associated with ``mod_name``.

    Checks both the a.out image and any shared object images for this resource.

  .. cpp:function:: mapped_object *findObject(std::string obj_name, bool wildcard = false) const

    Returns the object associated with ``obj_name``

  .. cpp:function:: mapped_object *findObject(Address addr) const
  .. cpp:function:: mapped_object *findObject(fileDescriptor desc) const
  .. cpp:function:: mapped_object *findObject(const ParseAPI::CodeObject *co) const
  .. cpp:function:: mapped_object *getAOut()
  .. cpp:function:: void getAllModules(std::vector<mapped_module *> &)

    Returns a vector of all modules defined in the a.out and in the shared objects.

  .. cpp:function:: const std::vector<mapped_object *> &mappedObjects()
  .. cpp:function:: virtual bool multithread_capable(bool ignore_if_mt_not_set = false) = 0
  .. cpp:function:: virtual bool multithread_ready(bool ignore_if_mt_not_set = false) = 0
  .. cpp:function:: void modifyCall(block_instance *callBlock, func_instance *newCallee, func_instance *context = NULL)
  .. cpp:function:: void revertCall(block_instance *callBlock, func_instance *context = NULL)
  .. cpp:function:: void replaceFunction(func_instance *oldfunc, func_instance *newfunc)
  .. cpp:function:: bool wrapFunction(func_instance *original, func_instance *wrapper, SymtabAPI::Symbol *clone)
  .. cpp:function:: void wrapFunctionPostPatch(func_instance *wrapped, Dyninst::SymtabAPI::Symbol *)
  .. cpp:function:: void revertWrapFunction(func_instance *original)
  .. cpp:function:: void revertReplacedFunction(func_instance *oldfunc)
  .. cpp:function:: void removeCall(block_instance *callBlock, func_instance *context = NULL)
  .. cpp:function:: const func_instance *isFunctionReplacement(func_instance *func) const
  .. cpp:function:: bool getDynamicCallSiteArgs(InstructionAPI::Instruction insn, Address addr, std::vector<AstNodePtr> &args)
  .. cpp:function:: virtual bool hasBeenBound(const SymtabAPI::relocationEntry &, func_instance *&, Address)
  .. cpp:function:: virtual bool bindPLTEntry(const SymtabAPI::relocationEntry & entry, Address base_addr, func_instance* target_func, Address target_addr)
  .. cpp:function:: int_variable *trampGuardBase(void)
  .. cpp:function:: AstNodePtr trampGuardAST(void)
  .. cpp:function:: Emitter *getEmitter()
  .. cpp:function:: virtual bool needsPIC() = 0
  .. cpp:function:: bool needsPIC(int_variable *v)
  .. cpp:function:: bool needsPIC(func_instance *f)
  .. cpp:function:: bool needsPIC(AddressSpace *s)
  .. cpp:function:: unsigned getAddressWidth() const
  .. cpp:function:: BPatch_function *newFunctionCB(Dyninst::PatchAPI::PatchFunction *f)
  .. cpp:function:: BPatch_point *newInstPointCB(Dyninst::PatchAPI::PatchFunction *f, Dyninst::PatchAPI::Point *pt, int type)
  .. cpp:function:: void registerFunctionCallback(BPatch_function *(*f)(AddressSpace *p, Dyninst::PatchAPI::PatchFunction *f))
  .. cpp:function:: void registerInstPointCallback(BPatch_point *(*f)(AddressSpace *p, Dyninst::PatchAPI::PatchFunction *f, Dyninst::PatchAPI::Point *ip, int type))
  .. cpp:function:: void *up_ptr()
  .. cpp:function:: void set_up_ptr(void *ptr)
  .. cpp:function:: void deleteAddressSpace()
  .. cpp:function:: void copyAddressSpace(AddressSpace *parent)

    Fork constructor - and so we can assume a parent "process" rather than "address space".

    Actually, for the sake of abstraction, use an AddressSpace instead of process.

    This is only defined for process->process copy until someone can give a good reason for copying anything else...

  .. cpp:function:: AddressSpace()
  .. cpp:function:: virtual ~AddressSpace()
  .. cpp:function:: Address getObservedCostAddr() const
  .. cpp:function:: void updateObservedCostAddr(Address addr)
  .. cpp:function:: bool canUseTraps()
  .. cpp:function:: void setUseTraps(bool usetraps)
  .. cpp:function:: bool relocate()
  .. cpp:function:: void getRelocAddrs(Address orig, block_instance *block, func_instance *func, std::list<Address> &relocs, bool getInstrumentationAddrs) const
  .. cpp:function:: bool getAddrInfo(Address relocAddr, Address &origAddr, std::vector<func_instance *> &origFuncs, baseTramp *&baseTramp)
  .. cpp:function:: bool getRelocInfo(Address relocAddr, RelocInfo &relocInfo)
  .. cpp:function:: bool inEmulatedCode(Address addr)
  .. cpp:function:: std::map<func_instance *, std::vector<edgeStub>> getStubs(const std::list<block_instance *> &owBBIs,\
                                                                              const std::set<block_instance *> &delBBIs, \
                                                                              const std::list<func_instance *> &deadFuncs)

    Create stub edge set which is all edges such that ``e->trg()`` in ``owBBIs`` and ``e->src()`` not in ``delBBIs``,
    in which case, choose stub from among ``e->src()->sources()``.

  .. cpp:function:: void addDefensivePad(block_instance *callBlock, func_instance *callFunc, Address padStart, unsigned size)
  .. cpp:function:: void getPreviousInstrumentationInstances(baseTramp *bt, std::set<Address>::iterator &b, std::set<Address>::iterator &e)
  .. cpp:function:: void addInstrumentationInstance(baseTramp *bt, Address addr)
  .. cpp:function:: void addModifiedFunction(func_instance *func)
  .. cpp:function:: void addModifiedBlock(block_instance *block)
  .. cpp:function:: bool delayRelocation() const
  .. cpp:function:: protected void inferiorFreeCompact()
  .. cpp:function:: protected int findFreeIndex(unsigned size, int type, Address lo, Address hi)
  .. cpp:function:: protected void addHeap(heapItem *h)
  .. cpp:function:: protected void initializeHeap()
  .. cpp:function:: protected Address inferiorMallocInternal(unsigned size, Address lo, Address hi, inferiorHeapType type)
  .. cpp:function:: protected void inferiorMallocAlign(unsigned &size)

  .. cpp:type:: Relocation::CodeTracker::RelocInfo RelocInfo
  .. cpp:type:: protected std::list<Relocation::CodeTracker *> CodeTrackers
  .. cpp:type:: protected std::set<func_instance *> FuncSet
  .. cpp:type:: protected std::pair<Address, unsigned> DefensivePad
  .. cpp:member:: protected Dyninst::Relocation::InstalledSpringboards::Ptr installedSpringboards_
  .. cpp:member:: protected std::map<Address, std::map<func_instance *, std::set<DefensivePad>>> forwardDefensiveMap_
  .. cpp:member:: protected IntervalTree<Address, std::pair<func_instance *, Address>> reverseDefensiveMap_
  .. cpp:member:: protected std::map<baseTramp *, std::set<Address>> instrumentationInstances_
  .. cpp:member:: protected bool delayRelocation_
  .. cpp:member:: protected std::map<func_instance *, Dyninst::SymtabAPI::Symbol *> wrappedFunctionWorklist_
  .. cpp:member:: protected bool heapInitialized_
  .. cpp:member:: protected bool useTraps_
  .. cpp:member:: protected bool sigILLTrampoline_
  .. cpp:member:: protected inferiorHeap heap_
  .. cpp:member:: protected std::vector<mapped_object *> mapped_objects
  .. cpp:member:: protected int_variable *trampGuardBase_
  .. cpp:member:: protected AstNodePtr trampGuardAST_
  .. cpp:member:: protected void *up_ptr_
  .. cpp:member:: protected Address costAddr_
  .. cpp:member:: protected CodeTrackers relocatedCode_
  .. cpp:member:: protected std::map<mapped_object *, FuncSet> modifiedFunctions_
  .. cpp:member:: protected Dyninst::PatchAPI::PatchMgrPtr mgr_
  .. cpp:member:: protected Dyninst::PatchAPI::Patcher::Ptr patcher_

  .. cpp:function:: protected bool transform(Dyninst::Relocation::CodeMoverPtr cm)
  .. cpp:function:: protected Address generateCode(Dyninst::Relocation::CodeMoverPtr cm, Address near)
  .. cpp:function:: protected bool patchCode(Dyninst::Relocation::CodeMoverPtr cm, Dyninst::Relocation::SpringboardBuilderPtr spb)
  .. cpp:function:: protected bool relocateInt(FuncSet::const_iterator begin, FuncSet::const_iterator end, Address near)
  .. cpp:function:: Dyninst::Relocation::InstalledSpringboards::Ptr getInstalledSpringboards()
  .. cpp:function:: Dyninst::PatchAPI::PatchMgrPtr mgr() const
  .. cpp:function:: void setMgr(Dyninst::PatchAPI::PatchMgrPtr m)
  .. cpp:function:: void setPatcher(Dyninst::PatchAPI::Patcher::Ptr p)
  .. cpp:function:: void initPatchAPI()
  .. cpp:function:: void addMappedObject(mapped_object *obj)
  .. cpp:function:: Dyninst::PatchAPI::Patcher::Ptr patcher()
  .. cpp:function:: static bool patch(AddressSpace *)

  .. cpp:type:: private BPatch_function *(*new_func_cb)(AddressSpace *a, Dyninst::PatchAPI::PatchFunction *f)
  .. cpp:type:: private BPatch_point *(*new_instp_cb)(AddressSpace *a, Dyninst::PatchAPI::PatchFunction *f, Dyninst::PatchAPI::Point *ip, int type)
