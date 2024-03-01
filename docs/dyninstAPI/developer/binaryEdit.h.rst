.. _`sec:binaryEdit.h`:

binaryEdit.h
############


.. cpp:class:: BinaryEdit : public AddressSpace

  Reading and writing get somewhat interesting. We are building a false address space - that of the
  "inferior" binary we're editing.  However, that address space doesn't exist - and so we must overlay
  it on ours. In the dynamic case this is easy, of course. So what we have is a mapping. An "Address"
  that we get (inOther), can map to one of the following:

  - An area created with inferiorMalloc
  - A section of the binary that is original
  - A section of the binary that was modified

  .. cpp:function:: bool readDataSpace(const void *inOther, u_int amount, void *inSelf, bool showError)
  .. cpp:function:: bool readTextSpace(const void *inOther, u_int amount, void *inSelf)
  .. cpp:function:: bool writeDataSpace(void *inOther, u_int amount, const void *inSelf)
  .. cpp:function:: bool writeTextSpace(void *inOther, u_int amount, const void *inSelf)
  .. cpp:function:: bool readDataWord(const void *inOther, u_int amount, void *inSelf, bool showError)
  .. cpp:function:: bool readTextWord(const void *inOther, u_int amount, void *inSelf)
  .. cpp:function:: bool writeDataWord(void *inOther, u_int amount, const void *inSelf)
  .. cpp:function:: bool writeTextWord(void *inOther, u_int amount, const void *inSelf)
  .. cpp:function:: Address inferiorMalloc(unsigned size, inferiorHeapType type = anyHeap, Address near = 0, bool *err = NULL)

    Memory allocation We don't specify how it should be done, only that it is. The model is that you ask
    for an allocation "near" a point, where "near" has an internal, platform-specific definition. The
    allocation mechanism does its best to give you what you want, but there are no promises - check the
    address of the returned buffer to be sure.

  .. cpp:function:: virtual void inferiorFree(Address item)
  .. cpp:function:: virtual bool inferiorRealloc(Address item, unsigned newSize)
  .. cpp:function:: Address offset() const
  .. cpp:function:: Address length() const
  .. cpp:function:: Architecture getArch() const

    .. note:: presumably all of the objects in the BinaryEdit collection must be the same architecture.

  .. cpp:function:: bool multithread_capable(bool ignore_if_mt_not_set = false)

    If true is passed for ignore_if_mt_not_set, then an error won't be initiated if we're unable to
    determine if the program is multi-threaded. We are unable to determine this if the daemon hasn't
    yet figured out what libraries are linked against the application.  Currently, we identify an
    application as being multi-threaded if it is linked against a thread library (eg. libpthreads.so on
    Linux).  There are cases where we are querying whether the app is multi-threaded, but it can't be
    determined yet but it also isn't necessary to know.

  .. cpp:function:: bool multithread_ready(bool ignore_if_mt_not_set = false)

    Do we have the RT-side multithread functions available

  .. cpp:function:: virtual bool hasBeenBound(const SymtabAPI::relocationEntry &, func_instance *&, Address)

  .. cpp:function:: bool needsPIC()

    Should be easy if the process isn't _executing_ where we're deleting...

  .. cpp:function:: BinaryEdit()
  .. cpp:function:: ~BinaryEdit()

    .. note:: We do not own the objects contained in ``newDyninstSyms_``, ``rtlib``, or ``siblings``, so do not delete them.


  .. cpp:function:: static BinaryEdit *openFile(const std::string &file, Dyninst::PatchAPI::PatchMgrPtr mgr = Dyninst::PatchAPI::PatchMgrPtr(), Dyninst::PatchAPI::Patcher::Ptr patch = Dyninst::PatchAPI::Patcher::Ptr(), const std::string &member = "")

    And the "open" factory method.

  .. cpp:function:: bool writeFile(const std::string &newFileName)
  .. cpp:function:: virtual func_instance *findOnlyOneFunction(const std::string &name, const std::string &libname = "", bool search_rt_lib = true)
  .. cpp:function:: bool openSharedLibrary(const std::string &file, bool openDependencies = true)

    open a shared library and (optionally) all its dependencies

  .. cpp:function:: void addDependentRelocation(Address to, SymtabAPI::Symbol *referring)

    add a shared library relocation

  .. cpp:function:: Address getDependentRelocationAddr(SymtabAPI::Symbol *referring)

    search for a shared library relocation

  .. cpp:function:: void addLibraryPrereq(std::string libname)

    Add a library prerequisite

  .. cpp:function:: void setupRTLibrary(std::vector<BinaryEdit *> &r)
  .. cpp:function:: std::vector<BinaryEdit *> &rtLibrary()
  .. cpp:function:: bool getAllDependencies(std::map<std::string, BinaryEdit *> &deps)
  .. cpp:function:: void markDirty()
  .. cpp:function:: bool isDirty()
  .. cpp:function:: mapped_object *getMappedObject()
  .. cpp:function:: void setMultiThreadCapable(bool b)
  .. cpp:function:: void addSibling(BinaryEdit *)
  .. cpp:function:: std::vector<BinaryEdit *> &getSiblings()
  .. cpp:function:: bool replaceTrapHandler()

    Find all calls to sigaction equivalents and replace with calls to ``dyn_<sigaction_equivalent_name>``.

  .. cpp:function:: bool usedATrap()

    Here's the story. We may need to install a trap handler for instrumentation to work in the rewritten
    binary. This doesn't play nicely with trap handlers that the binary itself registers. So we're going
    to replace every call to sigaction in the binary with a call to our wrapper. This wrapper 1) ignores
    attempts to register a SIGTRAP 2) Passes everything else through to sigaction It's called
    "dyn_sigaction".

  .. cpp:function:: bool isMultiThreadCapable()
  .. cpp:function:: mapped_object *openResolvedLibraryName(std::string filename, std::map<std::string, BinaryEdit *> &allOpened)
  .. cpp:function:: bool writing()
  .. cpp:function:: void addDyninstSymbol(SymtabAPI::Symbol *sym)
  .. cpp:function:: virtual void addTrap(Address from, Address to, codeGen &gen)
  .. cpp:function:: virtual void removeTrap(Address from)
  .. cpp:function:: static bool getResolvedLibraryPath(const std::string &filename, std::vector<std::string> &paths)
  .. cpp:member:: private Address highWaterMark_
  .. cpp:member:: private Address lowWaterMark_
  .. cpp:member:: private bool isDirty_
  .. cpp:function:: private static bool getStatFileDescriptor(const std::string &file, fileDescriptor &desc)
  .. cpp:function:: private bool inferiorMallocStatic(unsigned size)
  .. cpp:function:: private Address maxAllocedAddr()
  .. cpp:function:: private bool createMemoryBackingStore(mapped_object *obj)
  .. cpp:function:: private bool initialize()

    Initialization. For now we're skipping threads, since we can't get the functions we need. However,
    we kinda need the recursion guard. This is an integer (one per thread, for now - 1) that  begins
    initialized to 1.

  .. cpp:function:: private void makeInitAndFiniIfNeeded()
  .. cpp:function:: private bool archSpecificMultithreadCapable()
  .. cpp:function:: private bool doStaticBinarySpecialCases()

    Function specific to rewritting static binaries

  .. cpp:member:: private codeRangeTree memoryTracker_
  .. cpp:function:: private mapped_object *addSharedObject(const std::string *fullPath)
  .. cpp:member:: private std::vector<depRelocation *> dependentRelocations
  .. cpp:function:: private void buildDyninstSymbols(std::vector<SymtabAPI::Symbol *> &newSyms,\
                                                     SymtabAPI::Region *newSec, SymtabAPI::Module *newMod)

    Build a list of symbols describing instrumentation and relocated functions.  To keep this list
    (somewhat) short, we're doing one symbol per extent of  instrumentation + relocation for a
    particular function.  New: do this for one mapped object.

  .. cpp:member:: private mapped_object *mobj

    `mobj` is only a view. The actual object is owned by AddressSpace::mapped_objects

  .. cpp:member:: private std::vector<BinaryEdit *> rtlib
  .. cpp:member:: private std::vector<BinaryEdit *> siblings
  .. cpp:member:: private bool multithread_capable_
  .. cpp:member:: private bool writing_
  .. cpp:member:: private std::vector<SymtabAPI::Symbol *> newDyninstSyms_

    Symbols that other people (e.g., functions) want us to add


.. cpp:class:: depRelocation

  .. cpp:function:: depRelocation(Address a, SymtabAPI::Symbol *r)
  .. cpp:function:: Address getAddress() const
  .. cpp:function:: SymtabAPI::Symbol *getReferring() const
  .. cpp:member:: private Address to
  .. cpp:member:: private SymtabAPI::Symbol *referring

.. cpp:class:: memoryTracker : public codeRange

  .. cpp:function:: memoryTracker(Address a, unsigned s)
  .. cpp:function:: memoryTracker(Address a, unsigned s, void *b)
  .. cpp:function:: ~memoryTracker() = default
  .. cpp:function:: memoryTracker(memoryTracker const &) = delete
  .. cpp:function:: memoryTracker &operator=(memoryTracker const &) = delete
  .. cpp:function:: memoryTracker(memoryTracker &&) = default
  .. cpp:function:: memoryTracker &operator=(memoryTracker &&) = default
  .. cpp:function:: Address get_address() const
  .. cpp:function:: unsigned get_size() const
  .. cpp:function:: void *get_local_ptr() const
  .. cpp:function:: void realloc(unsigned newsize)
  .. cpp:member:: bool alloced{false}
  .. cpp:member:: bool dirty{false}
  .. cpp:member:: private Address a_
  .. cpp:member:: private unsigned s_
  .. cpp:member:: private std::unique_ptr<char[]> b_
