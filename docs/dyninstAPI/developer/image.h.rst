.. _`sec:image.h`:

image.h
#######

.. cpp:class:: image : public codeRange

  Image class contains information about statically and  dynamically linked code belonging to a process

  .. cpp:function:: static image *parseImage(fileDescriptor &desc, BPatch_hybridMode mode, bool parseGaps)
  .. cpp:function:: static void removeImage(image *img)

    And to get rid of them if we need to re-parse

  .. cpp:function:: image *clone()
  .. cpp:function:: image(fileDescriptor &desc, bool &err, BPatch_hybridMode mode, bool parseGaps)
  .. cpp:function:: void analyzeIfNeeded()
  .. cpp:function:: bool isParsed()
  .. cpp:function:: parse_func *addFunction(Dyninst::Address functionEntryAddr, const char *name = NULL)
  .. cpp:function:: pdmodule *getOrCreateModule(Dyninst::SymtabAPI::Module *mod)

    creates the module if it does not exist

  .. cpp:function:: protected ~image()
  .. cpp:function:: protected int destroy()

    7JAN05: go through the removeImage call!

  .. cpp:function:: pdmodule *findModule(const string &name, bool wildcard = false)

    find the named module

  .. cpp:function:: const std::vector<parse_func *> *findFuncVectorByPretty(const std::string &name)

    Find the vector of functions associated with a (demangled) name Returns internal pointer, so label as const

  .. cpp:function:: const std::vector<parse_func *> *findFuncVectorByMangled(const std::string &name)
  .. cpp:function:: const std::vector<image_variable *> *findVarVectorByPretty(const std::string &name)

    Variables: nearly identical

  .. cpp:function:: const std::vector<image_variable *> *findVarVectorByMangled(const std::string &name)
  .. cpp:function:: std::vector<parse_func *> *findFuncVectorByPretty(functionNameSieve_t bpsieve, void *user_data, std::vector<parse_func *> *found)

    Find the vector of functions determined by a filter function

  .. cpp:function:: std::vector<parse_func *> *findFuncVectorByMangled(functionNameSieve_t bpsieve, void *user_data, std::vector<parse_func *> *found)

  ......

  .. rubric::
    Function lookup (by name or address) routines

  .. warning::
    Overlapping region objects MUST NOT use these routines
  
  .. cpp:function:: parse_func *findFuncByEntry(const Dyninst::Address &entry)

    Find a function that begins at a particular address

  .. cpp:function:: int findFuncs(const Dyninst::Address offset, set<Dyninst::ParseAPI::Function *> &funcs)

    Code sharing allows multiple functions to overlap a given point

  .. cpp:function:: int findBlocksByAddr(const Dyninst::Address offset, set<Dyninst::ParseAPI::Block *> &blocks)

    Find the basic blocks that overlap the given address

  .. cpp:function:: void addTypedPrettyName(parse_func *func, const char *typedName)

    Add an extra pretty name to a known function (needed for handling overloaded functions in paradyn)

  .. cpp:function:: image_variable *createImageVariable(Dyninst::Address offset, std::string name, int size, pdmodule *mod)

    Create an image variable (e.g., malloced variable). Creates the variable and adds to appropriate data structures.

  .. cpp:function:: bool symbolExists(const std::string &)

    Check symbol existence

  .. cpp:function:: void postProcess(const std::string)

      Load .pif file

  .. cpp:function:: string file() const
  .. cpp:function:: string name() const
  .. cpp:function:: string pathname() const
  .. cpp:function:: const fileDescriptor &desc() const
  .. cpp:function:: Dyninst::Address imageOffset() const
  .. cpp:function:: Dyninst::Address dataOffset() const
  .. cpp:function:: Dyninst::Address dataLength() const
  .. cpp:function:: Dyninst::Address imageLength() const
  .. cpp:function:: void setImageLength(Dyninst::Address newlen)
  .. cpp:function:: Dyninst::Address get_address() const

    codeRange interface implementation

  .. cpp:function:: unsigned get_size() const
  .. cpp:function:: Dyninst::SymtabAPI::Symtab *getObject() const
  .. cpp:function:: Dyninst::ParseAPI::CodeObject *codeObject() const
  .. cpp:function:: bool isDyninstRTLib() const
  .. cpp:function:: bool isExecutable() const
  .. cpp:function:: bool isSharedLibrary() const
  .. cpp:function:: bool isSharedObject() const
  .. cpp:function:: bool isRelocatableObj() const
  .. cpp:function:: bool getExecCodeRanges(std::vector<std::pair<Dyninst::Address, Dyninst::Address>> &ranges)
  .. cpp:function:: Dyninst::SymtabAPI::Symbol *symbol_info(const std::string &symbol_name)

    Return symbol table information

  .. cpp:function:: bool findSymByPrefix(const std::string &prefix, std::vector<Dyninst::SymtabAPI::Symbol *> &ret)

    And used for finding inferior heaps.... hacky, but effective.

  .. cpp:function:: const Dyninst::ParseAPI::CodeObject::funclist &getAllFunctions()
  .. cpp:function:: const std::vector<image_variable *> &getAllVariables()

  ......

  .. rubric::
    DEFENSIVE-MODE CODE

  .. cpp:function:: BPatch_hybridMode hybridMode() const
  .. cpp:function:: bool hasNewBlocks() const
  .. cpp:function:: const vector<parse_block *> &getNewBlocks() const
  .. cpp:function:: void clearNewBlocks()
  .. cpp:function:: void register_codeBytesUpdateCB(void *cb_arg0)

    callback that updates our view the binary's raw code bytes

  .. cpp:function:: void *cb_arg0() const
  .. cpp:function:: const std::vector<image_variable *> &getExportedVariables() const
  .. cpp:function:: const std::vector<image_variable *> &getCreatedVariables()
  .. cpp:function:: bool getInferiorHeaps(vector<pair<string, Dyninst::Address>> &codeHeaps, vector<pair<string, Dyninst::Address>> &dataHeaps)
  .. cpp:function:: bool getModules(vector<pdmodule *> &mods)
  .. cpp:function:: int getNextBlockID()
  .. cpp:function:: Dyninst::Address get_main_call_addr() const
  .. cpp:function:: void *getErrFunc() const
  .. cpp:function:: std::unordered_map<Dyninst::Address, std::string> *getPltFuncs()
  .. cpp:function:: void getPltFuncs(std::map<Dyninst::Address, std::string> &out)
  .. cpp:function:: bool updatePltFunc(parse_func *caller_func, Dyninst::Address stub_targ)
  .. cpp:function:: void destroy(Dyninst::ParseAPI::Block *)

    Object deletion (defensive mode)

  .. cpp:function:: void destroy(Dyninst::ParseAPI::Edge *)
  .. cpp:function:: void destroy(Dyninst::ParseAPI::Function *)
  .. cpp:function:: private void findModByAddr(const Dyninst::SymtabAPI::Symbol *lookUp, vector<Dyninst::SymtabAPI::Symbol *> &mods, string &modName, Dyninst::Address &modAddr, const string &defName)
  .. cpp:function:: private int findMain()

    Platform-specific discovery of the "main" function

    FIXME There is a minor but fundamental design flaw that needs to be resolved wrt findMain returning void.

  .. cpp:function:: private bool determineImageType()
  .. cpp:function:: private bool addSymtabVariables()
  .. cpp:function:: private void setModuleLanguages(std::unordered_map<std::string, Dyninst::SymtabAPI::supportedLanguages> *mod_langs)
  .. cpp:function:: private void enterFunctionInTables(parse_func *func)

    We have a _lot_ of lookup types this handles proper entry

  .. cpp:function:: private bool buildFunctionLists(std::vector<parse_func *> &raw_funcs)
  .. cpp:function:: private void analyzeImage()
  .. cpp:function:: private void insertPLTParseFuncMap(const std::string &, parse_func *)

  ......

  .. rubric::
    GAP PARSING SUPPORT
    
  .. cpp:function:: private bool parseGaps()

  ......

  .. cpp:member:: private fileDescriptor desc_

    file descriptor(includes name)

  .. cpp:member:: private string name_

    filename part of file, no slashes

  .. cpp:member:: private string pathname_

    file name with path

  .. cpp:member:: private Dyninst::Address imageOffset_
  .. cpp:member:: private unsigned imageLen_
  .. cpp:member:: private Dyninst::Address dataOffset_
  .. cpp:member:: private unsigned dataLen_
  .. cpp:member:: private std::unordered_map<Dyninst::Address, parse_func *> activelyParsing
  .. cpp:member:: private bool is_libdyninstRT
  .. cpp:member:: private Dyninst::Address main_call_addr_

    address of call to main()

  .. cpp:member:: private Dyninst::SymtabAPI::Symtab *linkedFile

    data from the symbol table

  .. cpp:member:: private Dyninst::SymtabAPI::Archive *archive
  .. cpp:member:: private Dyninst::ParseAPI::CodeObject *obj_
  .. cpp:member:: private Dyninst::ParseAPI::SymtabCodeSource *cs_
  .. cpp:member:: private Dyninst::ParseAPI::SymtabCodeSource::hint_filt *filt
  .. cpp:member:: private DynCFGFactory *img_fact_
  .. cpp:member:: private DynParseCallback *parse_cb_
  .. cpp:member:: private void *cb_arg0_

    argument for mapped_object callback

  .. cpp:member:: private map<Dyninst::SymtabAPI::Module *, pdmodule *> mods_
  .. cpp:member:: private std::vector<image_variable *> everyUniqueVariable
  .. cpp:member:: private std::vector<image_variable *> createdVariables
  .. cpp:member:: private std::vector<image_variable *> exportedVariables
  .. cpp:member:: private std::vector<image_parRegion *> parallelRegions

    This contains all parallel regions on the image These line up with the code generated
    to support OpenMP, UPC, Titanium, ...

  .. cpp:member:: private int nextBlockID_

    unique (by image) numbering of basic blocks

  .. cpp:member:: private dyn_hash_map<string, pdmodule *> modsByFileName
  .. cpp:member:: private std::unordered_map<Dyninst::Address, std::string> *pltFuncs

    "Function" symbol names that are PLT entries or the equivalent FIXME remove

  .. cpp:member:: private std::unordered_map<Dyninst::Address, image_variable *> varsByAddr
  .. cpp:member:: private vector<pair<string, Dyninst::Address>> codeHeaps_
  .. cpp:member:: private vector<pair<string, Dyninst::Address>> dataHeaps_
  .. cpp:member:: private vector<parse_block *> newBlocks_

    new element tracking

  .. cpp:member:: private bool trackNewBlocks_
  .. cpp:member:: private int refCount
  .. cpp:member:: private imageParseState_t parseState_
  .. cpp:member:: private bool parseGaps_
  .. cpp:member:: private BPatch_hybridMode mode_
  .. cpp:member:: private Dyninst::Architecture arch
  .. cpp:member:: private dyn_hash_map<string, parse_func *> plt_parse_funcs


.. cpp:class:: pdmodule

  .. cpp:function:: pdmodule(Dyninst::SymtabAPI::Module *mod, image *e)
  .. cpp:function:: void cleanProcessSpecific(PCProcess *p)
  .. cpp:function:: bool getFunctions(std::vector<parse_func *> &funcs)
  .. cpp:function:: bool findFunction(const std::string &name, std::vector<parse_func *> &found)
  .. cpp:function:: bool getVariables(std::vector<image_variable *> &vars)
  .. cpp:function:: bool findFunctionByMangled(const std::string &name, std::vector<parse_func *> &found)

    We can see more than one function with the same mangledname in the same object, because it's OK for different
    modules in the same object to define the same (local) symbol. However, we can't always determine module information
    which means one of ourmodule classes may contain information about an entire object,and therefore, multiple functons
    with the same mangled name.

  .. cpp:function:: bool findFunctionByPretty(const std::string &name, std::vector<parse_func *> &found)
  .. cpp:function:: void dumpMangled(std::string &prefix) const
  .. cpp:function:: const string &fileName() const
  .. cpp:function:: Dyninst::SymtabAPI::supportedLanguages language() const
  .. cpp:function:: Dyninst::Address addr() const
  .. cpp:function:: bool isShared() const
  .. cpp:function:: Dyninst::SymtabAPI::Module *mod()
  .. cpp:function:: image *imExec() const
  .. cpp:member:: private Dyninst::SymtabAPI::Module *mod_
  .. cpp:member:: private image *exec_


.. cpp:class:: fileDescriptor

  File descriptor information

  .. cpp:function:: fileDescriptor()
  .. cpp:function:: fileDescriptor(string file, Dyninst::Address code, Dyninst::Address data)

    Some platforms have split code and data. If yours is not one of them, hand in the same address for code and data.

  .. cpp:function:: fileDescriptor(string file, Dyninst::Address code, Dyninst::Address data, Dyninst::Address length, void *)

    ctor for non-files

  .. cpp:function:: bool operator==(const fileDescriptor &fd) const
  .. cpp:function:: bool operator!=(const fileDescriptor &fd) const
  .. cpp:function:: bool isSameFile(const fileDescriptor &fd) const

    Not quite the same as above is this the same on-disk file

  .. cpp:function:: const string &file() const
  .. cpp:function:: const string &member() const
  .. cpp:function:: Dyninst::Address code() const
  .. cpp:function:: Dyninst::Address data() const
  .. cpp:function:: int pid() const
  .. cpp:function:: void setLoadAddr(Dyninst::Address a)
  .. cpp:function:: void setCode(Dyninst::Address c)
  .. cpp:function:: void setData(Dyninst::Address d)
  .. cpp:function:: void setMember(string member)
  .. cpp:function:: void setPid(int pid)
  .. cpp:function:: Dyninst::Address length() const

    only for non-files

  .. cpp:function:: void *rawPtr()

      only for non-files

  .. cpp:function:: void setHandles(HANDLE proc, HANDLE file)
  .. cpp:function:: HANDLE procHandle() const
  .. cpp:function:: HANDLE fileHandle() const
  .. cpp:member:: private HANDLE procHandle_
  .. cpp:member:: private HANDLE fileHandle_
  .. cpp:member:: private string file_
  .. cpp:member:: private string member_
  .. cpp:member:: private Dyninst::Address code_
  .. cpp:member:: private Dyninst::Address data_
  .. cpp:member:: private int pid_
  .. cpp:member:: private Dyninst::Address length_

    set only if this is not really a file

  .. cpp:function:: private bool IsEqual(const fileDescriptor &fd) const


.. cpp:class:: image_variable

  .. cpp:function:: image_variable()
  .. cpp:function:: image_variable(Dyninst::SymtabAPI::Variable *var, pdmodule *mod)
  .. cpp:function:: Dyninst::Address getOffset() const
  .. cpp:function:: string symTabName() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter symtab_names_begin() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter symtab_names_end() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter pretty_names_begin() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter pretty_names_end() const
  .. cpp:function:: bool addSymTabName(const std::string &, bool isPrimary = false)
  .. cpp:function:: bool addPrettyName(const std::string &, bool isPrimary = false)
  .. cpp:function:: pdmodule *pdmod() const
  .. cpp:function:: Dyninst::SymtabAPI::Variable *svar() const
  .. cpp:member:: Dyninst::SymtabAPI::Variable *var_{}
  .. cpp:member:: pdmodule *pdmod_{}


.. cpp:enum:: imageParseState_t

  .. cpp:enumerator:: unparsed
  .. cpp:enumerator:: symtab
  .. cpp:enumerator:: analyzing
  .. cpp:enumerator:: analyzed


.. cpp:function:: std::string getModuleName(std::string constraint)
.. cpp:function:: std::string getFunctionName(std::string constraint)
.. cpp:function:: int rawfuncscmp( parse_func*& pdf1, parse_func*& pdf2 )

.. cpp:type:: bool (*functionNameSieve_t)(const char *test,void *data)

.. code:: cpp

  #define RH_SEPERATOR '/'

  /* contents of line number field if line is unknown */
  #define UNKNOWN_LINE  0

  #define TAG_LIB_FUNC  0x1
  #define TAG_IO_OUT  0x2
  #define TAG_IO_IN       0x4
  #define TAG_MSG_SEND  0x8
  #define TAG_MSG_RECV    0x10
  #define TAG_SYNC_FUNC 0x20
  #define TAG_CPU_STATE 0x40  /* does the func block waiting for ext. event */
  #define TAG_MSG_FILT    0x80

  #define DYN_MODULE "DYN_MODULE"
  #define EXTRA_MODULE "EXTRA_MODULE"
  #define USER_MODULE "USER_MODULE"
  #define LIBRARY_MODULE  "LIBRARY_MODULE"

  #define NUMBER_OF_MAIN_POSSIBILITIES 8

.. cpp:var:: extern char main_function_names[NUMBER_OF_MAIN_POSSIBILITIES][20]

.. cpp:function:: int instPointCompare( instPoint*& ip1, instPoint*& ip2 )
.. cpp:function:: int basicBlockCompare( BPatch_basicBlock*& bb1, BPatch_basicBlock*& bb2 )

