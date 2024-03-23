.. _`sec:BPatch_module.h`:

BPatch_module.h
###############

.. cpp:class:: BPatch_module: public BPatch_sourceObj

  **A program module**

  A BPatch_module represents a source file in either an executable or a shared library. A
  default module is created in each executable to hold any objects that it cannot match to
  a source file.

  .. cpp:function:: char * getName(char *buffer, int length)

    Returns file name associated with module

  .. cpp:function:: char * getFullName(char *buffer, int length)

    Returns full path name of module, when available

  .. cpp:function:: const char * libraryName()

    Returns name if library, if this module is a shared object

  .. cpp:function:: BPatch_object * getObject()

    Returns BPatch_object containing this file

  .. cpp:function:: size_t getAddressWidth()

    Return the size (in bytes) of a pointer in this module. On 32-bit
    systems this function will return 4, and on 64-bit systems this function
    will return 8.

  .. cpp:function:: bool getVariables(BPatch_Vector<BPatch_variableExpr *> &vars)

    Fills a vector with the global variables that are specified in this module

  .. cpp:function:: BPatch_variableExpr* findVariable(const char* name)

    Find and return a global variable (NULL if not found)

  .. cpp:function:: BPatch_Vector<BPatch_function*>* getProcedures(bool incUninstrumentable = false)

    Returns a vector of all functions in this module

  .. cpp:function:: bool getProcedures(BPatch_Vector<BPatch_function*> &procs, bool incUninstrumentable = false)

  .. cpp:function:: BPatch_Vector<BPatch_function*>* findFunction(const char *name, BPatch_Vector<BPatch_function *> &funcs,\
                                                                  bool notify_on_failure =true, bool regex_case_sensitive =true,\
                                                                  bool incUninstrumentable =false, bool dont_use_regex = false)

    Returns a vector of BPatch_function matching specified <name>

    If name contains a POSIX-extended regular expression, a regex search will be performed on function names, and
    matching BPatch_functions returned. If the incUninstrumentable flag is set, the returned table of procedures
    will include uninstrumentable functions. The default behavior is to omit these functions.

    .. note::
    
      If name is not found to match any demangled function names in the module, the search is repeated as if name is
      a mangled function name. If this second search succeeds, functions with mangled names matching name are returned
      instead.

  .. cpp:function:: BPatch_function * findFunctionByEntry(Dyninst::Address entry)

    Returns the function starting at the given address

  .. cpp:function:: BPatch_Vector<BPatch_function*>* findFunctionByAddress(void* addr, BPatch_Vector<BPatch_function*>& funcs,\
                                                                            bool notify_on_failure = true,\
                                                                            bool incUninstrumentable = false)

    Return a vector of BPatch_functions that contains addr, or NULL if the function does not exist.

    If the incUninstrumentable flag is set, the returned table of procedures will include uninstrumentable functions.
    The default behavior is to omit these functions.

  .. cpp:function:: BPatch_typeCollection *getModuleTypes()

    get the module types member (instead of directly accessing)

  .. cpp:function:: BPatch_function * findFunctionByMangled(const char * mangled_name, bool incUninstrumentable=false)

    Returns a function, if it exits, that matches the provided mangled name

    If the incUninstrumentable flag is set, the functions searched will include uninstrumentable functions.
    The default behavior is to omit these functions.

  .. cpp:function:: bool findPoints(Dyninst::Address addr, std::vector<BPatch_point *> &points)

    Returns a vector of BPatch_points that correspond with the provided address, one per function that includes an
    instruction at that address. Will have one element  if there is not overlapping code.

  .. cpp:function:: bool isSharedLib()

    Returns true if this module represents a shared library

  .. cpp:function:: bool getAddressRanges(char* fileName, unsigned int lineNo, std::vector<Dyninst::SymtabAPI::AddressRange>& ranges)

    Returns in ``ranges`` the addresses covering the source code in file ``fileName`` and line number ``lineNo``.

    In many cases a source code line will only have one address range implementing it. However, compiler optimizations may turn
    this into multiple, disjoint address ranges. This information is only available if the mutatee was compiled with debug information.

    This function may be more efficient than the :cpp:func:`BPatch_process::getAddressRanges` as that version will cause
    Dyninst to parse line information for all modules in a process.

    Returns ``true`` if any line information was found.

  .. cpp:function:: bool getSourceLines(unsigned long addr, BPatch_Vector<BPatch_statement> &lines)

    Returns in ``lines`` the line information associated with the mutatee address ``addr``.

    In many cases only one filename and line number is associated with an address, but certain compiler optimizations
    may lead to multiple filenames and lines at an address. This information is only available if the mutatee was compiled
    with debug information.

    This function may be more efficient than :cpp:func:`BPatch_process::getSourceLines` as that version will cause Dyninst
    to parse line information for all modules in a process.

    Returns ``true`` any line information was found.

  .. cpp:function:: bool getStatements(BPatch_Vector<BPatch_statement> &statements)

    Fill supplied vector with all BPatch_statements from this module

  .. cpp:function:: void* getBaseAddr()

    Return the base address of the module.

    This address is defined as the start of the first function in the module.

  .. cpp:function:: Dyninst::Address getLoadAddr()

  .. cpp:function:: unsigned long getSize()

    Returns the size of the module defined as the end of the last function minus the start of the first function.

  .. cpp:function:: bool isValid()

  .. cpp:function:: BPatch_hybridMode getHybridMode()

    Return the mutator’s analysis mode for the mutatee.

  .. cpp:function:: void enableDefensiveMode(bool on)



.. cpp:namespace-push:: Dyninst::SymtabAPI

.. cpp:function:: Module *convert(const BPatch_module *)

.. cpp:namespace-pop::
 
.. cpp:var:: BPatch_builtInTypeCollection * builtInTypes
