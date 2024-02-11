.. _`sec:mapped_object.h`:

mapped_object.h
###############

A class for link map information about a shared object that is mmapped
by the dynamic linker into the applications address space at runtime.

.. cpp:class:: mapped_object : public codeRange, public Dyninst::PatchAPI::DynObject

  **A file in memory**

  It will be a collection of modules (basically, .o's) that are referred to as a unit and loaded as a unit.
  The big reason for this is 1) per-process specialization and 2) a way to reduce memory to create objects
  for all functions ahead of time is wasteful and expensive. So basically, the mapped_object "wins" if it
  can return useful information without having to allocate memory.

  .. cpp:function:: private mapped_object()
  .. cpp:function:: private mapped_object(fileDescriptor fileDesc, image *img, AddressSpace *proc, BPatch_hybridMode mode = BPatch_normalMode)
  .. cpp:function:: static mapped_object *createMappedObject(fileDescriptor &desc, AddressSpace *p, BPatch_hybridMode m = BPatch_normalMode, bool parseGaps = true)

    We need a way to check for errors hence a "get" method

  .. cpp:function:: static mapped_object *createMappedObject(ProcControlAPI::Library::const_ptr lib, AddressSpace *p, BPatch_hybridMode m = BPatch_normalMode, bool parseGaps = true)
  .. cpp:function:: mapped_object(const mapped_object *par_obj, AddressSpace *child)

    Copy constructor: for forks

  .. cpp:function:: ~mapped_object()

    Will delete all func_instances which were originally part of this object including any that were relocated
    (we can always follow the "I was relocated" pointer).

  .. cpp:function:: bool analyze()
  .. cpp:function:: bool isAnalyzed()
  .. cpp:function:: const fileDescriptor &getFileDesc() const
  .. cpp:function:: const string &fullName() const

    Full name, including path

  .. cpp:function:: string fileName() const
  .. cpp:function:: Dyninst::Address codeAbs() const
  .. cpp:function:: Dyninst::Address codeBase() const
  .. cpp:function:: Dyninst::Address imageOffset() const
  .. cpp:function:: unsigned imageSize() const
  .. cpp:function:: unsigned memoryEnd()

    largest allocated memory address + 1

  .. cpp:function:: bool isCode(Dyninst::Address addr) const

    32-bit math safe!

  .. cpp:function:: bool isData(Dyninst::Address addr) const
  .. cpp:function:: Dyninst::Address getBaseAddress() const
  .. cpp:function:: Dyninst::Address dataAbs() const
  .. cpp:function:: Dyninst::Address dataBase() const
  .. cpp:function:: Dyninst::Address dataOffset() const
  .. cpp:function:: unsigned dataSize() const
  .. cpp:function:: Dyninst::Address getTOCBaseAddress() const
  .. cpp:function:: void setTOCBaseAddress(Dyninst::Address addr)
  .. cpp:function:: image *parse_img() const
  .. cpp:function:: bool isSharedLib() const
  .. cpp:function:: bool isStaticExec() const
  .. cpp:function:: static bool isSystemLib(const std::string &name)
  .. cpp:function:: bool isMemoryImg() const
  .. cpp:function:: void setMemoryImg()
  .. cpp:function:: const std::string debugString() const

    Return an appropriate identification string for debug purposes. Will eventually be required by a debug base class.

  .. cpp:function:: Dyninst::Address get_address() const

    Used for codeRange ONLY! DON'T USE THIS! BAD USER!

  .. cpp:function:: void *get_local_ptr() const
  .. cpp:function:: unsigned get_size() const
  .. cpp:function:: AddressSpace *proc() const
  .. cpp:function:: mapped_module *findModule(string m_name, bool wildcard = false)
  .. cpp:function:: mapped_module *findModule(pdmodule *mod)
  .. cpp:function:: mapped_module *getDefaultModule()
  .. cpp:function:: func_instance *findFuncByEntry(const Dyninst::Address addr)
  .. cpp:function:: func_instance *findFuncByEntry(const block_instance *blk)
  .. cpp:function:: bool getInfHeapList(std::vector<heapDescriptor> &infHeaps)
  .. cpp:function:: void getInferiorHeaps(vector<pair<string, Dyninst::Address>> &infHeaps)
  .. cpp:function:: bool findFuncsByAddr(const Dyninst::Address addr, std::set<func_instance *> &funcs)
  .. cpp:function:: bool findBlocksByAddr(const Dyninst::Address addr, std::set<block_instance *> &blocks)
  .. cpp:function:: block_instance *findBlockByEntry(const Dyninst::Address addr)
  .. cpp:function:: block_instance *findOneBlockByAddr(const Dyninst::Address addr)
  .. cpp:function:: void *getPtrToInstruction(Dyninst::Address addr) const

    codeRange method

  .. cpp:function:: void *getPtrToData(Dyninst::Address addr) const
  .. cpp:function:: bool getAllFunctions(std::vector<func_instance *> &funcs)

    Try to avoid using these if you can, since they'll trigger parsing and allocation.

  .. cpp:function:: bool getAllVariables(std::vector<int_variable *> &vars)
  .. cpp:function:: const std::vector<mapped_module *> &getModules()
  .. cpp:function:: BPatch_hybridMode hybridMode()

  ......

  .. rubric::
    Exploratory and defensive mode

  .. cpp:function:: void enableDefensiveMode(bool on = true)
  .. cpp:function:: bool isExploratoryModeOn()
  .. cpp:function:: bool parseNewEdges(const std::vector<edgeStub> &sources)
  .. cpp:function:: bool parseNewFunctions(std::vector<Dyninst::Address> &funcEntryAddrs)
  .. cpp:function:: bool updateCodeBytesIfNeeded(Dyninst::Address entryAddr)

    ret true if was needed

  .. cpp:function:: void updateCodeBytes(const std::list<std::pair<Dyninst::Address, Dyninst::Address>> &owRanges)
  .. cpp:function:: void setCodeBytesUpdated(bool)
  .. cpp:function:: void addProtectedPage(Dyninst::Address pageAddr)

    adds to :cpp:member:`protPages_`.

  .. cpp:function:: void removeProtectedPage(Dyninst::Address pageAddr)
  .. cpp:function:: void removeEmptyPages()
  .. cpp:function:: void remove(func_instance *func)
  .. cpp:function:: void remove(instPoint *p)
  .. cpp:function:: void splitBlock(block_instance *first, block_instance *second)
  .. cpp:function:: bool findBlocksByRange(Dyninst::Address startAddr, Dyninst::Address endAddr, std::list<block_instance *> &pageBlocks)
  .. cpp:function:: void findFuncsByRange(Dyninst::Address startAddr, Dyninst::Address endAddr, std::set<func_instance *> &pageFuncs)
  .. cpp:function:: void addEmulInsn(Dyninst::Address insnAddr, Register effective_addr)
  .. cpp:function:: bool isEmulInsn(Dyninst::Address insnAddr)
  .. cpp:function:: Register getEmulInsnReg(Dyninst::Address insnAddr)
  .. cpp:function:: void setEmulInsnVal(Dyninst::Address insnAddr, void *val)
  .. cpp:function:: int codeByteUpdates()
  .. cpp:function:: void replacePLTStub(Dyninst::SymtabAPI::Symbol *PLTsym, func_instance *func, Dyninst::Address newAddr)
  .. cpp:function:: private void updateCodeBytes(Dyninst::SymtabAPI::Region *reg)

    helper functions

  .. cpp:function:: private bool isUpdateNeeded(Dyninst::Address entryAddr)
  .. cpp:function:: private bool isExpansionNeeded(Dyninst::Address entryAddr)
  .. cpp:function:: private void expandCodeBytes(Dyninst::SymtabAPI::Region *reg)
  .. cpp:function:: bool getSymbolInfo(const std::string &n, int_symbol &sym)

  ......

  .. cpp:function:: const std::vector<func_instance *> *findFuncVectorByPretty(const std::string &funcname)

    All name lookup functions are vectorized, because you can have multiple overlapping names for all sorts of
    reasons. Demangled"pretty": easy overlap (overloaded funcs, etc.). Mangled: multiple modules with static/private
    functions and we've lost the module name.

  .. cpp:function:: const std::vector<func_instance *> *findFuncVectorByMangled(const std::string &funcname)
  .. cpp:function:: bool findFuncsByAddr(std::vector<func_instance *> &funcs)
  .. cpp:function:: bool findBlocksByAddr(std::vector<block_instance *> &blocks)
  .. cpp:function:: const std::vector<int_variable *> *findVarVectorByPretty(const std::string &varname)
  .. cpp:function:: const std::vector<int_variable *> *findVarVectorByMangled(const std::string &varname)
  .. cpp:function:: const int_variable *getVariable(const std::string &varname)
  .. cpp:function:: void setDirty()

    this marks the shared object as dirty, mutated so it needs saved back to disk

  .. cpp:function:: bool isDirty()
  .. cpp:function:: func_instance *findFunction(ParseAPI::Function *img_func)
  .. cpp:function:: int_variable *findVariable(image_variable *img_var)
  .. cpp:function:: block_instance *findBlock(ParseAPI::Block *)
  .. cpp:function:: edge_instance *findEdge(ParseAPI::Edge *, block_instance *src = NULL, block_instance *trg = NULL)

    If we already know the source or target hand them in for efficiency

  .. cpp:function:: func_instance *findGlobalConstructorFunc(const std::string &ctorHandler)

    These methods should be invoked to find the global constructor and destructor functions in stripped, static binaries

  .. cpp:function:: func_instance *findGlobalDestructorFunc(const std::string &dtorHandler)
  .. cpp:function:: std::string getCalleeName(block_instance *)

    We store callee names at the mapped_object level for efficiency

  .. cpp:function:: void setCalleeName(block_instance *, std::string name)
  .. cpp:function:: void setCallee(const block_instance *, func_instance *)
  .. cpp:function:: func_instance *getCallee(const block_instance *) const
  .. cpp:function:: void destroy(PatchAPI::PatchFunction *f)
  .. cpp:function:: void destroy(PatchAPI::PatchBlock *b)
  .. cpp:member:: private fileDescriptor desc_

      full file descriptor

  .. cpp:member:: private string fullName_

      full file name of the shared object

  .. cpp:member:: private string fileName_

      name of shared object as it should be identified in mdl, e.g. as used for "exclude"

  .. cpp:member:: private Dyninst::Address dataBase_

    Where the data starts...

  .. cpp:member:: private Dyninst::Address tocBase
  .. cpp:function:: private void set_short_name()
  .. cpp:member:: private std::vector<mapped_module *> everyModule
  .. cpp:type:: private std::unordered_map<std::string, std::vector<func_instance *> *> func_index_t
  .. cpp:type:: private std::unordered_map<std::string, std::vector<int_variable *> *> var_index_t
  .. cpp:member:: private std::unordered_map<const image_variable *, int_variable *> everyUniqueVariable
  .. cpp:member:: private func_index_t allFunctionsByMangledName
  .. cpp:member:: private func_index_t allFunctionsByPrettyName
  .. cpp:member:: private var_index_t allVarsByMangledName
  .. cpp:member:: private var_index_t allVarsByPrettyName
  .. cpp:member:: private codeRangeTree codeRangesByAddr_
  .. cpp:function:: private void addFunction(func_instance *func)
  .. cpp:function:: private void addVariable(int_variable *var)
  .. cpp:function:: private void addFunctionName(func_instance *func, const std::string newName, func_index_t &index)

      Add a name after-the-fact

  .. cpp:member:: private bool dirty_

      marks the shared object as dirty

  .. cpp:member:: private bool dirtyCalled_

      see comment for :cpp:func:`setDirtyCalled`.

  .. cpp:member:: private image *image_

      pointer to image if processed is true

  .. cpp:member:: private bool dlopenUsed

     mark this shared object as opened by dlopen

  .. cpp:member:: private AddressSpace *proc_

      Parent process

  .. cpp:member:: private bool analyzed_

      Prevent multiple adds

  .. cpp:member:: private BPatch_hybridMode analysisMode_
  .. cpp:member:: private map<Dyninst::Address, WriteableStatus> protPages_
  .. cpp:member:: private std::set<Dyninst::SymtabAPI::Region *> expansionCheckedRegions_
  .. cpp:member:: private bool pagesUpdated_
  .. cpp:member:: private int codeByteUpdates_
  .. cpp:type:: private std::map<Dyninst::Address, std::pair<Register, void *>> EmulInsnMap
  .. cpp:member:: private EmulInsnMap emulInsns_
  .. cpp:member:: private Dyninst::Address memEnd_

      size of object in memory

  .. cpp:function:: private mapped_module *getOrCreateForkedModule(mapped_module *mod)
  .. cpp:member:: private bool memoryImg_
  .. cpp:member:: private std::map<block_instance *, std::string> calleeNames_
  .. cpp:member:: private std::map<const block_instance *, func_instance *> callees_


.. cpp:enum:: mapped_object::nameType_t

  .. cpp:enumerator:: mangledName=1
  .. cpp:enumerator:: prettyName=2
  .. cpp:enumerator:: typedName=4

.. cpp:enum:: mapped_object::WriteableStatus

  .. cpp:enumerator:: PROTECTED
  .. cpp:enumerator:: REPROTECTED
  .. cpp:enumerator:: UNPROTECTED


.. cpp:class:: mappedObjData : public codeRange

  Aggravation: a mapped object might very well occupy multiple "ranges".

  .. cpp:function:: mappedObjData(mapped_object *obj_)
  .. cpp:function:: Dyninst::Address get_address() const
  .. cpp:function:: unsigned get_size() const
  .. cpp:member:: mapped_object *obj


.. cpp:class:: int_symbol

  .. cpp:function:: int_symbol(Dyninst::SymtabAPI::Symbol *sym, Dyninst::Address base)
  .. cpp:function:: int_symbol()
  .. cpp:function:: Dyninst::Address getAddr() const
  .. cpp:function:: unsigned getSize() const
  .. cpp:function:: string symTabName() const
  .. cpp:function:: string prettyName() const
  .. cpp:function:: string typedName() const
  .. cpp:function:: const Dyninst::SymtabAPI::Symbol *sym() const
  .. cpp:member:: private Dyninst::Address addr_
  .. cpp:member:: private const Dyninst::SymtabAPI::Symbol *sym_

.. cpp:class:: int_variable

  .. cpp:function:: private int_variable()
  .. cpp:function:: int_variable(image_variable *var, Dyninst::Address base, mapped_module *mod)
  .. cpp:function:: int_variable(int_variable *parVar, mapped_module *child)
  .. cpp:function:: Dyninst::Address getAddress() const
  .. cpp:function:: string symTabName() const

    Can variables have multiple names?

  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter pretty_names_begin() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter pretty_names_end() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter symtab_names_begin() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter symtab_names_end() const
  .. cpp:function:: mapped_module *mod() const
  .. cpp:function:: const image_variable *ivar() const
  .. cpp:member:: Dyninst::Address addr_{}
  .. cpp:member:: unsigned size_{}
  .. cpp:member:: image_variable *ivar_{}
  .. cpp:member:: mapped_module *mod_{}


.. cpp:struct:: edgeStub

  .. cpp:function:: edgeStub(block_instance *s, Dyninst::Address t, EdgeTypeEnum y)
  .. cpp:function:: edgeStub(block_instance *s, Dyninst::Address t, EdgeTypeEnum y, bool b)
  .. cpp:member:: block_instance *src
  .. cpp:member:: Dyninst::Address trg
  .. cpp:member:: EdgeTypeEnum type
  .. cpp:member:: bool checked{}



.. code:: cpp

  #define   SHAREDOBJECT_NOCHANGE 0
  #define   SHAREDOBJECT_ADDED  1
  #define   SHAREDOBJECT_REMOVED  2

