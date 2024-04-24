.. _`sec:BPatch_image.h`:

BPatch_image.h
##############

.. cpp:class:: BPatch_image: public BPatch_sourceObj
  
  **A program image (the executable associated with a process)**

  The only way to get a handle to a BPatch_image is via :cpp:func:`BPatch_process::getImage`.

  .. cpp:type:: bool (*BPatchFunctionNameSieve) (const char *name, void* sieve_data)
  .. cpp:type:: std::vector<std::pair<unsigned long, unsigned long> >::iterator arange_iter
  .. cpp:type:: BPatch_Vector<BPatch_statement>::iterator statement_iter

  .. cpp:function:: BPatch_thread* getThr()

    Return the BPatch_thread associated with this image

  .. cpp:function:: BPatch_addressSpace* getAddressSpace()

    Return the BPatch_addressSpace associated with this image

  .. cpp:function:: BPatch_process* getProcess()

    Return the BPatch_process associated with this image

  .. cpp:function:: bool getSourceObj(BPatch_Vector<BPatch_sourceObj*> &sources)

    Fill a vector with children source objects (modules)

  .. cpp:function:: BPatch_sourceObj* getObjParent()

    Return the parent of this image (always NULL since this is the top level)

  .. cpp:function:: bool getVariables(BPatch_Vector<BPatch_variableExpr*> &vars)

    Returns the global variables defined in this image

  .. cpp:function:: BPatch_Vector<BPatch_function*>* getProcedures(bool incUninstrumentable = false)

    Returns a list of all procedures in the image upon success

  .. cpp:function:: bool getProcedures(BPatch_Vector<BPatch_function*> &procs, bool incUninstrumentable = false)

    Returns a list of all procedures in the image

  .. cpp:function:: BPatch_Vector<BPatch_parRegion*>* getParRegions(bool incUninstrumentable = false)

    Returns a list of all parallel regions in the image

  .. cpp:function:: BPatch_Vector<BPatch_module*>* getModules()

    Returns a vector of all modules in this image

  .. cpp:function:: void getObjects(std::vector<BPatch_object*> &objs)

    Returns a vector of all objects in this image

  .. cpp:function:: bool getModules(BPatch_Vector<BPatch_module*> &mods)

    Returns a vector of all modules in this image

  .. cpp:function:: BPatch_module* findModule(const char *name, bool substring_match = false)

    Returns a module matching <name> if present in image.

    If ``substring_match`` is ``true``, the first module that has ``name`` as a substring of its name
    is returned. For example, to find "libpthread.so.1", search for "libpthread"  with ``substring_match``
    set to ``true``.

  .. cpp:function:: BPatch_Vector<BPatch_variableExpr*>* getGlobalVariables()

    Returns the global variables defined in this image

  .. cpp:function:: BPatch_Vector<BPatch_function*>* findFunction(const char* name, BPatch_Vector<BPatch_function*> &funcs,\
                                                                  bool showError=true, bool regex_case_sensitive=true, \
                                                                  bool incUninstrumentable = false)

    Returns a vector of functions matching ``name``.

    If ``name`` contains a POSIX-extended regular expression and ``dont_use_regex`` is ``false``, a regular expression search
    is performed. If ``showError`` is ``true``, then errors are reported by :cpp:func:`BPatch::registerErrorCallback` if no
    function is found. If ``incUninstrumentable`` is ``true``, the returned table of procedures include uninstrumentable
    functions. The default behavior is to omit these functions.

    .. note::
       If name is not found to match any demangled function names in
       the module, the search is repeated as if name is a mangled function
       name. If this second search succeeds, functions with mangled names
       matching name are returned instead.

  .. cpp:function:: BPatch_Vector<BPatch_function*>* findFunction(BPatch_Vector<BPatch_function*> &funcs,\
                                                                  BPatchFunctionNameSieve bpsieve, void *user_data=NULL,\
                                                                  int showError=0, bool incUninstrumentable = false)

    Returns a vector of functions matching criterion specified by user defined callback function bpsieve.

  .. cpp:function:: BPatch_function* findFunction(unsigned long addr)

    Returns a function at a specified address

  .. cpp:function:: bool findFunction(Dyninst::Address addr, BPatch_Vector<BPatch_function*> &funcs)

    Returns a function at a specified address

  .. cpp:function:: BPatch_variableExpr* findVariable(const char *name, bool showError=true)

    Returns global variable matching ``name`` in the image in the global scope.

  .. cpp:function:: BPatch_variableExpr* findVariable(BPatch_point &scp, const char *nm, bool showError=true)

    Returns local variable matching name ``nm`` in function scope ``scp``.

    Not implemented on Windows.

  .. cpp:function:: BPatch_type* findType(const char *name)

    Returns a BPatch_type corresponding to <name>, if exists, NULL if not found

  .. cpp:function:: bool findPoints(Dyninst::Address addr, std::vector<BPatch_point*> &points)

    Returns a vector of BPatch_points that correspond with the provided address, one  per function that includes an instruction at that address. Will have one element  if there is not overlapping code.

  .. cpp:function:: bool getAddressRanges(const char* fileName, unsigned int lineNo, std::vector<Dyninst::SymtabAPI::AddressRange>& ranges)

    Given a file name and line number, fileName and lineNo, this function
    returns a list of address ranges that this source line was compiled into.

  .. cpp:function:: bool getSourceLines( unsigned long addr, BPatch_Vector<BPatch_statement> & lines )

    Returns the source statements in ``lines`` that cover the address ``addr``.

  .. cpp:function:: arange_iter getAddressRanges_begin(const char* fileName, unsigned int lineNo)
  .. cpp:function:: arange_iter getAddressRanges_end(const char* fileName, unsigned int lineNo)
  .. cpp:function:: statement_iter getSourceLines_begin(unsigned long addr)
  .. cpp:function:: statement_iter getSourceLines_end(unsigned long addr)

  .. cpp:function:: char* getProgramName(char *name, unsigned int len)

    Fills provided buffer <name> with the program's name, up to <len> chars

  .. cpp:function:: char* getProgramFileName(char *name, unsigned int len)

    Fills provided buffer <name> with the program's file name,   which may include path information.

  .. cpp:function:: bool parseNewFunctions(BPatch_Vector<BPatch_module*> &affectedModules, const BPatch_Vector<Dyninst::Address> &funcEntryAddrs)

    This function uses function entry addresses to find and parse new functions using our control-flow traversal parsing.

    funcEntryAddrs: this is a vector of function start addresses that seed the control-flow-traversal parsing.  If they lie in
    an existing module they are parsed in that module, otherwise a new module is created.  In both cases the modules are added to
    affectedModules

    affectedModules: BPatch_modules will be added to this vector if no existing modules bounded the specified function entry points.
    Unfortunately, new modules will also sometimes have to be created for dynamically created code in memory that does not map to the
    file version of the binary.

    Return value: This value is true if a new module was created or if new code was parsed in an existing module

  .. cpp:function:: bool readString(Dyninst::Address addr, std::string &str, unsigned size_limit = 0)

    Reads a string from the target process

  .. cpp:function:: bool readString(BPatch_variableExpr *expr, std::string &str, unsigned size_limit = 0)
