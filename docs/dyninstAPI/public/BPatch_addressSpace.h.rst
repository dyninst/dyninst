.. _`sec:BPatch_addressSpace.h`:

BPatch_addressSpace.h
#####################

.. cpp:enum:: processType

  .. cpp:enumerator:: TRADITIONAL_PROCESS
  .. cpp:enumerator:: STATIC_EDITOR

.. cpp:type:: boost::shared_ptr<PatchMgr> PatchMgrPtr
.. cpp:type:: boost::shared_ptr<DynAddrSpace> DynAddrSpacePtr
.. cpp:type:: boost::shared_ptr<Instance> InstancePtr
    
.. cpp:class:: BPatch_addressSpace

  Functionality that is common between :cpp:class:`BPatch_process` and :cpp:class:`BPatch_binaryEdit`.

  .. cpp:type:: BPatch_Vector<BPatch_statement>::const_iterator statement_iter
  .. cpp:type:: std::vector<std::pair<unsigned long, unsigned long>>::const_iterator arange_iter
  .. cpp:type:: std::vector<BPatch_register>::iterator register_iter

  .. cpp:member:: protected BPatch_Vector<batchInsertionRecord*>* pendingInsertions
  .. cpp:member:: protected BPatch_image* image
  .. cpp:member:: protected std::vector<BPatch_register> registers_

  .. cpp:function:: BPatch_addressSpace()
  .. cpp:function:: virtual ~BPatch_addressSpace()

  .. cpp:function:: protected static BPatch_function* createBPFuncCB(AddressSpace* p, Dyninst::PatchAPI::PatchFunction* f)

      Triggered by lower-level code and forward calls up to the findOrCreate functions.

  .. cpp:function:: protected static BPatch_point* createBPPointCB(AddressSpace* p, Dyninst::PatchAPI::PatchFunction* f, \
                                                                   Dyninst::PatchAPI::Point* ip, int type)

      Triggered by lower-level code and forward calls up to the findOrCreate functions.

  .. cpp:function:: protected virtual void getAS(std::vector<AddressSpace*>& as) = 0

  .. cpp:function:: BPatch_function* findOrCreateBPFunc(Dyninst::PatchAPI::PatchFunction* ifunc, BPatch_module* bpmod)

  .. cpp:function:: BPatch_point* findOrCreateBPPoint(BPatch_function* bpfunc, Dyninst::PatchAPI::Point* ip, \
                                                      BPatch_procedureLocation pointType)

  .. cpp:function:: BPatch_variableExpr* findOrCreateVariable(int_variable* v, BPatch_type* type = NULL)

  .. cpp:function:: virtual processType getType() = 0

    This function returns a processType that reflects whether this address
    space is a BPatch_process or a BPatch_binaryEdit.

  .. cpp:function:: virtual bool getTerminated() = 0

      Checks if the associated process has been terminated.

  .. cpp:function:: virtual bool getMutationsActive() = 0

  .. cpp:function:: BPatch_module *findModuleByAddr(Dyninst::Address addr)

      Returns the module that contains the address ``addr`` or ``NULL`` if the address is not within a module.

      Does NOT trigger parsing.

  .. cpp:function:: bool findFuncsByRange(Dyninst::Address startAddr, Dyninst::Address endAddr, std::set<BPatch_function*> &funcs)

  ......

  .. rubric::
    Snippet Insertion

  These insert a snippet of code at a given point. If a collection of points is
  supplied, insert the code snippet at each point in the list. The
  optional when argument specifies when the snippet is to be called; a
  value of BPatch_callBefore indicates that the snippet should be inserted
  just before the specified point or points in the code, and a value of
  BPatch_callAfter indicates that it should be inserted just after them.

  The order argument specifies where the snippet is to be inserted
  relative to any other snippets previously inserted at the same point.
  The values BPatch_firstSnippet and BPatch_lastSnippet indicate that the
  snippet should be inserted before or after all snippets, respectively.

  .. cpp:function:: virtual BPatchSnippetHandle* insertSnippet(const BPatch_snippet& expr, BPatch_point& point, \
                                                               BPatch_snippetOrder order = BPatch_firstSnippet)

  .. cpp:function:: virtual BPatchSnippetHandle* insertSnippet(const BPatch_snippet& expr, BPatch_point& point, BPatch_callWhen when, \
                                                               BPatch_snippetOrder order = BPatch_firstSnippet)

  .. cpp:function:: virtual BPatchSnippetHandle* insertSnippet(const BPatch_snippet& expr, const BPatch_Vector<BPatch_point*>& points, \
                                                               BPatch_snippetOrder order = BPatch_firstSnippet)

    Inserts the snippet ``expr`` at each of the instrumentation points in ``points``.

  .. cpp:function:: virtual BPatchSnippetHandle* insertSnippet(const BPatch_snippet& expr, const BPatch_Vector<BPatch_point*>& points, \
                                                               BPatch_callWhen when, BPatch_snippetOrder order = BPatch_firstSnippet)

    Inserts the snippet ``expr`` at each of the instrumentation points in ``points``.

  .. warning::
    It is illegal to use :cpp:enumerator:`BPatch_callAfter` with a :cpp:enumerator:`BPatch_locEntry`
    point. Use :cpp:enumerator:`BPatch_callBefore` when instrumenting entry points, which inserts
    instrumentation before the first instruction in a subroutine. Likewise, it is illegal to use
    :cpp:enumerator:`BPatch_callBefore` with a :cpp:enumerator:`BPatch_exit` point. Use
    :cpp:enumerator:`BPatch_callAfter` with exit points. :cpp:enumerator:`BPatch_callAfter` inserts
    instrumentation at the last instruction in the subroutine.
    :cpp:any:`insertSnippet` will return ``NULL`` when used with an illegal pair of
    points.

  ......

  .. cpp:function:: virtual void beginInsertionSet() = 0

    Normally, a call to insertSnippet immediately injects instrumentation
    into the mutatee. However, users may wish to insert a set of snippets as
    a single batch operation. This provides two benefits: First, Dyninst may
    insert instrumentation in a more efficient manner. Second, multiple
    snippets may be inserted at multiple points as a single operation, with
    either all snippets being inserted successfully or none. This batch
    insertion mode is begun with a call to beginInsertionSet; after this
    call, no snippets are actually inserted until a corresponding call to
    finalizeInsertionSet. Dyninst accumulates all calls to insertSnippet
    during batch mode internally, and the returned BPatchSnippetHandles are
    filled in when finalizeInsertionSet is called.

    Insertion sets are un­necessary when doing static binary
    instrumentation. Dyninst uses an implicit insertion set around all
    instrumentation to a static binary.

  .. cpp:function:: virtual bool finalizeInsertionSet(bool atomic, bool* modified = NULL) = 0

    Inserts all snippets accumulated since a call to beginInsertionSet. If
    the atomic parameter is true, then a failure to insert any snippet
    results in all snippets being removed; effectively, the insertion is
    all-or-nothing. If the atomic parameter is false, then snippets are
    inserted individually. This function also fills in the
    BPatchSnippetHandle structures returned by the insertSnippet calls
    comprising this insertion set. It returns true on success and false if
    there was an error inserting any snippets.

    Insertion sets are unnecessary when doing static binary instrumentation.
    Dyninst uses an implicit insertion set around all instrumentation to a
    static binary.

  .. cpp:function:: bool deleteSnippet(BPatchSnippetHandle* handle)

    Remove the snippet associated with the passed handle. If the handle is
    not defined for the process, then deleteSnippet will return false.

  .. cpp:function:: bool replaceFunctionCall(BPatch_point& point, BPatch_function& newFunc)

    Change the function call at the specified point to the function
    indicated by newFunc. The purpose of this routine is to permit runtime
    steering tools to change the behavior of programs by replacing a call to
    one procedure by a call to another. Point must be a function call point.
    If the change was successful, the return value is true, otherwise false
    will be returned.

   .. warning::
      Care must be used when replacing functions. In particular if the compiler has performed
      inter-procedural register allocation between the original caller/callee pair, the
      replacement may not be safe since the replaced function may clobber registers the compiler
      thought the callee left untouched. Also the signatures of the both the function being
      replaced and the new function must be compatible.

  .. cpp:function:: bool removeFunctionCall(BPatch_point& point)

    Disable the mutatee function call at the specified location. The point
    specified must be a valid call point in the image of the mutatee. The
    purpose of this routine is to permit tools to alter the semantics of a
    program by eliminating procedure calls. The mechanism to achieve the
    removal is platform dependent, but might include branching over the call
    or replacing it with NOPs. This function only removes a function call;
    any parameters to the function will still be evaluated.

  .. cpp:function:: bool replaceFunction(BPatch_function& oldFunc, BPatch_function& newFunc)

    Replace all calls to user function old with calls to new. This is done
    by inserting instrumentation (specifically a BPatch_funcJumpExpr) into
    the beginning of function old such that a non-returning jump is made to
    function new. Returns true upon success, false otherwise.

  .. cpp:function:: bool revertReplaceFunction(BPatch_function& oldFunc)

      Undo the actions of :cpp:func:`replaceFunction`.

  .. cpp:function:: bool wrapFunction(BPatch_function* oldFunc, BPatch_function* newFunc, \
                                      Dyninst::SymtabAPI::Symbol* clone)

    Replaces all calls to function old with calls to function new. Unlike
    replaceFunction above, the old function can still be reached via the
    name specified by the provided symbol sym. Function wrapping allows
    existing code to be extended by new code. Consider the following code
    that implements a fast memory allocator for a particular size of memory
    allocation, but falls back to the original memory allocator (referenced
    by origMalloc) for all others.

    The symbol sym is provided by the user and must exist in the program;
    the easiest way to ensure it is created is to use an undefined function
    as shown above with the definition of origMalloc.

  .. cpp:function:: bool revertWrapFunction(BPatch_function* wrappedFunc)

      Undoes the actions of :cpp:func:`wrapFunction`.

  .. cpp:function:: bool getSourceLines(unsigned long addr, BPatch_Vector<BPatch_statement>& lines)

    This function returns the line information associated with the mutatee
    address, addr. The vector lines contain pairs of filenames and line
    numbers that are associated with addr. In many cases only one filename
    and line number is associated with an address, but certain compiler
    optimizations may lead to multiple filenames and lines at an address.
    This information is only available if the mutatee was compiled with
    debug information.

    This function returns true if it was able to find any line information
    at addr, or false otherwise.

  .. cpp:function:: statement_iter getSourceLines_begin(unsigned long addr)
  .. cpp:function:: statement_iter getSourceLines_end(unsigned long addr)

  .. cpp:function:: bool getAddressRanges(const char* fileName, unsigned int lineNo, \
                                          std::vector<Dyninst::SymtabAPI::AddressRange>& ranges)

    Given a filename and line number, fileName and lineNo, this function
    this function returns the ranges of mutatee addresses that implement the
    code range in the output parameter ranges. In many cases a source code
    line will only have one address range implementing it. However, compiler
    optimizations may transform this into multiple disjoint address ranges.
    This information is only available if the mutatee was compiled with
    debug information.

    This function returns true if it was able to find any line information,
    false otherwise.

  .. cpp:function:: statement_iter getAddressRanges_begin(const char* file, unsigned long line)
  .. cpp:function:: statement_iter getAddressRanges_end(const char* file, unsigned long line)

  .. cpp:function:: BPatch_function* findFunctionByEntry(Dyninst::Address entry)

      Returns the function starting at the given address

  .. cpp:function:: bool findFunctionsByAddr(Dyninst::Address addr, std::vector<BPatch_function*>& funcs)

      Returns the functions containing an address (multiple functions are returned when code is shared)

  .. cpp:function:: BPatch_image* getImage()

      Return a handle to the executable file associated with this BPatch_process object.

  ......

  .. rubric::
    These two functions allocate memory. Memory allocation is from a heap.
    The heap is not necessarily the same heap used by the application. The
    available space in the heap may be limited depending on the
    implementation.
  
  .. cpp:function:: BPatch_variableExpr* malloc(int n, std::string name = std::string(""))

    Allocate ``n`` *bytes** of memory in the thread's address space.

    If a name is specified, Dyninst will assign ``var_name`` to the variable; otherwise,
    it will assign an internal name. The returned memory is persistent and
    will not be released until :cpp:func:`BPatch_process::free` is called or the
    application terminates.


    If otherwise unspecified when binary rewriting, then the allocation
    happens in the original object.

  .. cpp:function:: BPatch_variableExpr* malloc(const BPatch_type& type, std::string name = std::string(""))

    Allocate enough memory in the thread's address space for a variable of the given type.

    If a name is specified, Dyninst will assign ``var_name`` to the variable; otherwise,
    it will assign an internal name. The returned memory is persistent and
    will not be released until :cpp:func:`BPatch_process::free` is called or the
    application terminates.

    Using this version is strongly encouraged because it provides additional
    information to permit better type checking of the passed code.

  .. cpp:function:: bool free(BPatch_variableExpr& ptr)

    Free the memory at ``ptr`` allocated allocated with :cpp:func:`BPatch_process::malloc`.

    The programmer is responsible for verifying that all code that could reference this memory
    will not execute again (either by removing all snippets that refer to
    it, or by analysis of the program). Return true if the free succeeded.

  ......

  .. cpp:function:: BPatch_variableExpr* createVariable(std::string name, Dyninst::Address addr,\
                                                        BPatch_type* type = NULL)

  .. cpp:function:: BPatch_variableExpr* createVariable(Dyninst::Address at_addr, BPatch_type* type, \
                                                        std::string var_name = std::string(""), \
                                                        BPatch_module* in_module = NULL)

    This method creates a new variable at the given address addr in the
    module in_module. If a name is specified, Dyninst will assign var_name
    to the variable; otherwise, it will assign an internal name. The type
    parameter will become the type for the new variable.

    When operating in binary rewriting mode, it is an error for the
    in_module parameter to be NULL; it is necessary to specify the module in
    which the variable will be created. Dyninst will then write the variable
    back out in the file specified by in_module.

  .. cpp:function:: bool getRegisters(std::vector<BPatch_register>& regs)

    This function returns a vector of BPatch_register objects that represent
    registers available to snippet code.

  .. cpp:function:: register_iter getRegisters_begin()
  .. cpp:function:: register_iter getRegisters_end()

  .. cpp:function:: bool createRegister_NP(std::string regName, BPatch_register& reg)
  .. cpp:function:: void allowTraps(bool allowtraps)

  .. cpp:function:: virtual BPatch_object* loadLibrary(const char* libname, bool reload = false) = 0

    For dynamic rewriting, this function loads a dynamically linked library
    into the process’s address space. For static rewriting, this function
    adds a library as a library dependency in the rewritten file. In both
    cases Dyninst creates a new BPatch_module to represent this library.

    The libname parameter identifies the file name of the library to be
    loaded, in the standard way that dynamically linked libraries are
    specified on the operating system on which the API is running. This
    function returns a handle to the loaded library. The reload parameter is
    ignored and only remains for backwards compatibility.

  .. cpp:function:: bool isStaticExecutable()

    Checks if the original file opened is a statically-linked executable.


.. cpp:class:: BPatchSnippetHandle

  .. cpp:member:: private BPatch_addressSpace* addSpace_

    Address Space snippet belongs to

  .. cpp:member:: private std::vector<Dyninst::PatchAPI::InstancePtr> instances_

      low-level mappings for removal

  .. cpp:member:: private BPatch_Vector<BPatch_thread*> catchup_threads

      a list of threads to apply catchup to

  .. cpp:function:: private BPatchSnippetHandle(BPatch_addressSpace* addSpace)
  .. cpp:function:: private void addInstance(Dyninst::PatchAPI::InstancePtr p)
  .. cpp:function:: ~BPatchSnippetHandle()
  .. cpp:function:: bool usesTrap()

      Returns whether the installed miniTramps use traps. Not 100% accurate due to
      internal Dyninst design; we can have multiple instances of instrumentation
      due to function relocation.

  .. cpp:function:: bool isEmpty()
  .. cpp:function:: BPatch_function* getFunc()

      ``mtHandles_`` is not empty. Returns the function that the instrumentation was added to

  .. cpp:function:: BPatch_addressSpace* getAddressSpace()
  .. cpp:function:: BPatch_process* getProcess()
  .. cpp:type:: BPatch_Vector<BPatch_thread*>::iterator thread_iter
  .. cpp:function:: thread_iter getCatchupThreads_begin()
  .. cpp:function:: thread_iter getCatchupThreads_end()
  .. cpp:function:: BPatch_Vector<BPatch_thread*>& getCatchupThreads()

