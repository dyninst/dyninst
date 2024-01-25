.. _`sec:addressSpace.h`:

addressSpace.h
##############

This serves to define an "address space", a set of routines that
code generation and instrumentation rely on to perform their duties.
This was derived from the process class and serves as a parent to that
class and the static_space class for the rewriter.



.. cpp:class:: AddressSpace : public InstructionSource

  | This is a little complex, so let me explain my logic
  | Map from B -> F_c -> F
  | B identifies a call site
  | F_c identifies an (optional) function context for the replacement
  |  ... if F_c is not specified, we use NULL
  | F specifies the replacement callee; if we want to remove the call entirely, also use NULL

  .. cpp:type:: boost::shared_ptr<Dyninst::InstructionAPI::Instruction> InstructionPtr
  .. cpp:type:: Relocation::CodeTracker::RelocInfo RelocInfo
  .. cpp:type:: protected std::list<Relocation::CodeTracker *> CodeTrackers
  .. cpp:type:: protected std::set<func_instance *> FuncSet
  .. cpp:type:: protected std::pair<Address, unsigned> DefensivePad

  .. cpp:member:: trampTrapMappings trapMapping

      Trap address to base tramp address (for trap instrumentation)

  .. cpp:member:: std::set<mapped_object *> runtime_lib
  .. cpp:member:: std::string dyninstRT_name
  .. cpp:member:: protected bool heapInitialized_
  .. cpp:member:: protected bool useTraps_
  .. cpp:member:: protected bool sigILLTrampoline_
  .. cpp:member:: protected inferiorHeap heap_
  .. cpp:member:: protected std::vector<mapped_object *> mapped_objects

      Loaded mapped objects (may be just 1)

  .. cpp:member:: protected int_variable* trampGuardBase_

      Tramp recursion index mapping

  .. cpp:member:: protected AstNodePtr trampGuardAST_
  .. cpp:member:: protected void *up_ptr_
  .. cpp:member:: protected Address costAddr_
  .. cpp:member:: protected CodeTrackers relocatedCode_
  .. cpp:member:: protected std::map<mapped_object *, FuncSet> modifiedFunctions_
  .. cpp:member:: protected Dyninst::Relocation::InstalledSpringboards::Ptr installedSpringboards_
  .. cpp:member:: protected std::map<Address, std::map<func_instance*,std::set<DefensivePad> > > forwardDefensiveMap_
  .. cpp:member:: protected IntervalTree<Address, std::pair<func_instance*,Address> > reverseDefensiveMap_
  .. cpp:member:: protected std::map<baseTramp *, std::set<Address> > instrumentationInstances_

      Tracking instrumentation for fast removal

  .. cpp:member:: protected bool delayRelocation_
  .. cpp:member:: protected std::map<func_instance *, Dyninst::SymtabAPI::Symbol *> wrappedFunctionWorklist_
  .. cpp:member:: protected Dyninst::PatchAPI::PatchMgrPtr mgr_
  .. cpp:member:: protected Dyninst::PatchAPI::Patcher::Ptr patcher_
  .. cpp:member:: private BPatch_function *(*new_func_cb)(AddressSpace *a, Dyninst::PatchAPI::PatchFunction *f)
  .. cpp:member:: private BPatch_point *(*new_instp_cb)(AddressSpace *a, Dyninst::PatchAPI::PatchFunction *f, Dyninst::PatchAPI::Point *ip, int type)

  .. cpp:function:: AddressSpace()
  .. cpp:function:: virtual ~AddressSpace()

  .. cpp:function:: PCProcess *proc()

      Down-converts to a process.

  .. cpp:function:: BinaryEdit *edit()

    Down-converts to an edit.

  .. cpp:function:: virtual bool readDataWord(const void *inOther, u_int amount, void *inSelf, bool showError) = 0
  .. cpp:function:: virtual bool readDataSpace(const void *inOther, u_int amount, void *inSelf, bool showError) = 0
  .. cpp:function:: virtual bool readTextWord(const void *inOther, u_int amount, void *inSelf) = 0
  .. cpp:function:: virtual bool readTextSpace(const void *inOther, u_int amount, void *inSelf) = 0
  .. cpp:function:: virtual bool writeDataWord(void *inOther, u_int amount, const void *inSelf) = 0
  .. cpp:function:: virtual bool writeDataSpace(void *inOther, u_int amount, const void *inSelf) = 0
  .. cpp:function:: virtual bool writeTextWord(void *inOther, u_int amount, const void *inSelf) = 0
  .. cpp:function:: virtual bool writeTextSpace(void *inOther, u_int amount, const void *inSelf) = 0
  .. cpp:function:: Address getTOCoffsetInfo(func_instance *)

  .. rubric:: Memory allocation

  We don't specify how it should be done, only that it is. The model is
  that you ask for an allocation "near" a point, where "near" has an
  internal, platform-specific definition. The allocation mechanism does its
  best to give you what you want, but there are no promises - check the
  address of the returned buffer to be sure.

  .. cpp:function:: virtual Address inferiorMalloc(unsigned size, inferiorHeapType type=anyHeap, Address near = 0, bool *err = NULL) = 0
  .. cpp:function:: virtual void inferiorFree(Address item) = 0
  .. cpp:function:: void inferiorFreeInternal(Address item)
  .. cpp:function:: virtual bool inferiorRealloc(Address item, unsigned newSize) = 0

      And a "constrain" call to free unused memory. This is useful because our instrumentation is incredibly wasteful.

  .. cpp:function:: bool inferiorReallocInternal(Address item, unsigned newSize)
  .. cpp:function:: bool inferiorShrinkBlock(heapItem *h, Address block, unsigned newSize)
  .. cpp:function:: bool inferiorExpandBlock(heapItem *h, Address block, unsigned newSize)
  .. cpp:function:: bool isInferiorAllocated(Address block)

  ......

  .. cpp:function:: virtual void addTrap(Address from, Address to, codeGen &gen) = 0

      Allow the AddressSpace to update any extra bookkeeping for trap-based instrumentation

  .. cpp:function:: virtual void removeTrap(Address from) = 0
  .. cpp:function:: virtual bool getDyninstRTLibName()

  .. rubric:: InstructionSource

  .. cpp:function:: virtual bool isValidAddress(const Address) const
  .. cpp:function:: virtual void *getPtrToInstruction(const Address) const
  .. cpp:function:: virtual void *getPtrToData(const Address a) const

  .. cpp:function:: bool usesDataLoadAddress() const

      OS-specific

  .. cpp:function:: virtual bool isCode(const Address) const
  .. cpp:function:: virtual bool isData(const Address) const
  .. cpp:function:: virtual bool isReadOnly(const Address) const
  .. cpp:function:: virtual Address offset() const = 0
  .. cpp:function:: virtual Address length() const = 0
  .. cpp:function:: virtual Architecture getArch() const = 0

  .. rubric:: Function lookup

  .. cpp:function:: bool findFuncsByAll(const std::string &funcname, std::vector<func_instance *> &res, const std::string &libname = "")
  .. cpp:function:: bool findFuncsByPretty(const std::string &funcname, std::vector<func_instance *> &res, const std::string &libname = "")
  .. cpp:function:: bool findFuncsByMangled(const std::string &funcname, std::vector<func_instance *> &res, const std::string &libname = "")
  .. cpp:function:: bool findVarsByAll(const std::string &varname, std::vector<int_variable *> &res, const std::string &libname = "")
  .. cpp:function:: virtual func_instance *findOnlyOneFunction(const std::string &name, const std::string &libname = "", bool search_rt_lib = true)

      And we often internally want to wrap the above to return one and only one func.

  .. cpp:function:: void getAllFunctions(std::vector<func_instance *> &)

      Returns a vector of all functions defined in the a.out and in the shared objects

  .. cpp:function:: bool findFuncsByAddr(Address addr, std::set<func_instance *> &funcs, bool includeReloc = false)
  .. cpp:function:: func_instance *findOneFuncByAddr(Address addr)

      Use it when you *know* that you want one function, picked arbitrarily, from the possible functions.

  .. cpp:function:: func_instance *findFuncByEntry(Address addr)
  .. cpp:function:: func_instance *findFunction(parse_func *ifunc)

      And a lookup by "internal" function to find clones during fork.

  .. cpp:function:: func_instance *findFuncByEntry(const block_instance *block)

      Fast lookups across all mapped_objects

  ......

  .. rubric:: Symbol information

  .. cpp:function:: bool getSymbolInfo( const std::string &name, int_symbol &ret )

    This will find the named symbol in the image or in a shared object
    Necessary since some things don't show up as a function or variable.
    This gets wrapped with an int_symbol and returned.

  .. cpp:function:: bool findBlocksByAddr(Address addr, std::set<block_instance *> &blocks, bool includeReloc = false)
  .. cpp:function:: block_instance *findBlockByEntry(Address addr)
  .. cpp:function:: block_instance *findBlock(parse_block *iblock)
  .. cpp:function:: edge_instance *findEdge(ParseAPI::Edge *iedge)
  .. cpp:function:: func_instance *findJumpTargetFuncByAddr(Address addr)

      Acts like findFunc, but if it fails, checks if 'addr' is a jump to a function.

  .. cpp:function:: bool sameRegion(Dyninst::Address addr1, Dyninst::Address addr2)

      true if the addrs are in the same object and region within the object

  .. cpp:function:: mapped_module *findModule(const std::string &mod_name, bool wildcard = false)

      Returns the module associated with ``mod_name`` this routine checks both the a.out image and any shared object
      images for this module if check_excluded is true it checks to see if the module is excluded and if it is it
      returns 0.  If check_excluded is false it doesn't check if substring_match is true, the first module whose name
      contains the provided string is returned.

      Wildcard: handles "*" and "?"

  .. cpp:function:: mapped_object *findObject(std::string obj_name, bool wildcard = false) const

      Returns the object associated with ``obj_name`` this routine checks both the a.out image and any shared object
      images for this module if check_excluded is true it checks to see if the module is excluded and if it is it
      returns 0.  If check_excluded is false it doesn't check if substring_match is true, the first object whose name
      contains the provided string is returned.

      Wildcard: handles "*" and "?"

  .. cpp:function:: mapped_object *findObject(Address addr) const
  .. cpp:function:: mapped_object *findObject(fileDescriptor desc) const
  .. cpp:function:: mapped_object *findObject(const ParseAPI::CodeObject *co) const
  .. cpp:function:: mapped_object *getAOut()
  .. cpp:function:: void getAllModules(std::vector<mapped_module *> &)

      Returns all modules defined in the a.out and in the shared objects

  .. cpp:function:: const std::vector<mapped_object *> &mappedObjects()

      Return the list of dynamically linked libs

  .. cpp:function:: virtual bool multithread_capable(bool ignore_if_mt_not_set = false) = 0

      If true is passed for ignore_if_mt_not_set, then an error won't be
      initiated if we're unable to determine if the program is multi-threaded.
      We are unable to determine this if the daemon hasn't yet figured out
      what libraries are linked against the application.  Currently, we
      identify an application as being multi-threaded if it is linked against
      a thread library (eg. libpthreads.so on Linux).  There are cases where we
      are querying whether the app is multi-threaded, but it can't be
      determined yet but it also isn't necessary to know.

  .. cpp:function:: virtual bool multithread_ready(bool ignore_if_mt_not_set = false) = 0

      Do we have the RT-side multithread functions available

  .. rubric:: Process-level instrumentation

  instPoint isn't const; it may get an updated list of instances since we generate them lazily. Shouldn't this be an
  instPoint member function?

  .. cpp:function:: void modifyCall(block_instance *callBlock, func_instance *newCallee, func_instance *context = NULL)
  .. cpp:function:: void revertCall(block_instance *callBlock, func_instance *context = NULL)
  .. cpp:function:: void replaceFunction(func_instance *oldfunc, func_instance *newfunc)
  .. cpp:function:: bool wrapFunction(func_instance *original, func_instance *wrapper, SymtabAPI::Symbol *clone)
  .. cpp:function:: void wrapFunctionPostPatch(func_instance *wrapped, Dyninst::SymtabAPI::Symbol *)
  .. cpp:function:: void revertWrapFunction(func_instance *original)
  .. cpp:function:: void revertReplacedFunction(func_instance *oldfunc)
  .. cpp:function:: void removeCall(block_instance *callBlock, func_instance *context = NULL)
  .. cpp:function:: const func_instance *isFunctionReplacement(func_instance *func) const
  .. cpp:function:: bool getDynamicCallSiteArgs(InstructionAPI::Instruction insn, Address addr, \
                                                std::vector<AstNodePtr> &args)
  .. cpp:function:: virtual bool hasBeenBound(const SymtabAPI::relocationEntry &, func_instance *&, Address)
  .. cpp:function:: virtual bool bindPLTEntry(const SymtabAPI::relocationEntry & entry, Address base_addr, \
                                              func_instance * target_func, Address target_addr)
  .. cpp:function:: int_variable* trampGuardBase(void)
  .. cpp:function:: AstNodePtr trampGuardAST(void)
  .. cpp:function:: Emitter *getEmitter()

      Get the current code generator (or emitter)

  .. cpp:function:: virtual bool needsPIC() = 0

      True if any reference to this address space needs PIC

  .. cpp:function:: bool needsPIC(int_variable *v)

      True if we need PIC to reference ``v`` from this addressSpace.

  .. cpp:function:: bool needsPIC(func_instance *f)

      True if we need PIC to reference ``f`` from this addressSpace.

  .. cpp:function:: bool needsPIC(AddressSpace *s)

      True if we need PIC to reference ``s`` from this addressSpace.

  .. cpp:function:: unsigned getAddressWidth() const

  ......

  .. rubric:: BPatch-level stuff

  Callbacks for higher level code (like BPatch) to learn about new functions and InstPoints.

  .. cpp:function:: BPatch_function *newFunctionCB(Dyninst::PatchAPI::PatchFunction *f)

      Trigger the callbacks from a lower level

  .. cpp:function:: BPatch_point *newInstPointCB(Dyninst::PatchAPI::PatchFunction *f, Dyninst::PatchAPI::Point *pt, int type)

      Trigger the callbacks from a lower level

  .. cpp:function:: void registerFunctionCallback(BPatch_function *(*f)(AddressSpace *p, Dyninst::PatchAPI::PatchFunction *f))

      Register callbacks from the higher level

  .. cpp:function:: void registerInstPointCallback(BPatch_point *(*f)(AddressSpace *p, Dyninst::PatchAPI::PatchFunction *f, \
                                                   Dyninst::PatchAPI::Point *ip, int type))

      Register callbacks from the higher level

  .. cpp:function:: void *up_ptr()

      Anonymous up pointer to the containing process. This is BPatch_process in Dyninst. Currently stored as an void pointer
      in case we do anything with this during the library split.

  .. cpp:function:: void set_up_ptr(void *ptr)

  ......

  .. rubric:: Internal and cleanup

  .. cpp:function:: void deleteAddressSpace()

      Clear things out (e.g., deleteProcess)

  .. cpp:function:: void copyAddressSpace(AddressSpace *parent)

      Fork psuedo-constructor

  .. cpp:function:: Address getObservedCostAddr() const
  .. cpp:function:: void updateObservedCostAddr(Address addr)
  .. cpp:function:: bool canUseTraps()

      Can we use traps if necessary?

  .. cpp:function:: void setUseTraps(bool usetraps)

  ......

  .. rubric:: Relocations

  This is the top interface for the new (experimental) (probably not working) code generation interface.
  The core idea is to feed a set of func_instances (actually, a set of blocks, but functions are convenient)
  into a CodeMover class, let it chew on the code, and spit out a buffer of moved code. We also get a priority
  list of patches; (origAddr, movedAddr) pairs. We then get to decide what we want to do with those patches:
  put in a branch or say to heck with it.

  .. cpp:function:: bool relocate()
  .. cpp:function:: void getRelocAddrs(Address orig, block_instance *block, func_instance *func, std::list<Address> &relocs,\
                                       bool getInstrumentationAddrs) const

      Get the list of addresses an address (in a block) has been relocated to.

  .. cpp:function:: bool getAddrInfo(Address relocAddr, Address &origAddr, std::vector<func_instance *> &origFuncs,\
                                     baseTramp *&baseTramp)
  .. cpp:function:: bool getRelocInfo(Address relocAddr, RelocInfo &relocInfo)

  ......

  .. rubric:: Defensive mode

  .. cpp:function:: bool inEmulatedCode(Address addr)

      Debugging method

  .. cpp:function:: std::map<func_instance*,std::vector<edgeStub> > getStubs(const std::list<block_instance *> &owBBIs,\
                                                                             const std::set<block_instance*> &delBBIs,\
                                                                             const std::list<func_instance*> &deadFuncs)
  .. cpp:function:: void addDefensivePad(block_instance *callBlock, func_instance *callFunc, Address padStart, unsigned size)
  .. cpp:function:: void getPreviousInstrumentationInstances(baseTramp *bt, std::set<Address>::iterator &b,\
                                                             std::set<Address>::iterator &e)
  .. cpp:function:: void addInstrumentationInstance(baseTramp *bt, Address addr)
  .. cpp:function:: void addModifiedFunction(func_instance *func)
  .. cpp:function:: void addModifiedBlock(block_instance *block)
  .. cpp:function:: bool delayRelocation() const

  ......

  .. rubric:: inferior malloc support functions

  .. cpp:function:: protected void inferiorFreeCompact()
  .. cpp:function:: protected int findFreeIndex(unsigned size, int type, Address lo, Address hi)
  .. cpp:function:: protected void addHeap(heapItem *h)
  .. cpp:function:: protected void initializeHeap()
  .. cpp:function:: protected Address inferiorMallocInternal(unsigned size, Address lo, Address hi, inferiorHeapType type)
  .. cpp:function:: protected void inferiorMallocAlign(unsigned &size)
  .. cpp:function:: protected bool transform(Dyninst::Relocation::CodeMoverPtr cm)
  .. cpp:function:: protected Address generateCode(Dyninst::Relocation::CodeMoverPtr cm, Address near)
  .. cpp:function:: protected bool patchCode(Dyninst::Relocation::CodeMoverPtr cm, Dyninst::Relocation::SpringboardBuilderPtr spb)
  .. cpp:function:: protected bool relocateInt(FuncSet::const_iterator begin, FuncSet::const_iterator end, Address near)
  .. cpp:function:: Dyninst::Relocation::InstalledSpringboards::Ptr getInstalledSpringboards()
  .. cpp:function:: Dyninst::PatchAPI::PatchMgrPtr mgr() const
  .. cpp:function:: void setMgr(Dyninst::PatchAPI::PatchMgrPtr m)
  .. cpp:function:: void setPatcher(Dyninst::PatchAPI::Patcher::Ptr p)
  .. cpp:function:: void initPatchAPI()
  .. cpp:function:: void addMappedObject(mapped_object* obj)
  .. cpp:function:: Dyninst::PatchAPI::Patcher::Ptr patcher()
  .. cpp:function:: static bool patch(AddressSpace*)
