BPatch_image.h
==============

``BPatch_image``
----------------
.. cpp:namespace:: BPatch_image

.. cpp:class:: BPatch_image
   
   This class defines a program image (the executable associated with a
   process). The only way to get a handle to a BPatch_image is via the
   BPatch_process member function getImage.
   
   .. cpp:function:: const BPatch_point *createInstPointAtAddr (caddr_t address)
      
      This function has been removed because it is not safe to use. Instead,
      use findPoints:
      
   .. cpp:function:: bool findPoints(Dyninst::Address addr, std::vector<BPatch_point *> &points);
      
      Returns a vector of BPatch_points that correspond with the provided
      address, one per function that includes an instruction at that address.
      There will be one element if there is not overlapping code.
      
   .. cpp:function:: std::vector<BPatch_variableExpr *> *getGlobalVariables()
      
      Return a vector of global variables that are defined in this image.
      
   .. cpp:function:: BPatch_process *getProcess()
      
      Returns the BPatch_process associated with this image.
      
   .. cpp:function:: char *getProgramFileName(char *name, unsigned int len)
      
      Fills provided buffer name with the program’s file name up to len
      characters. The filename may include path information.
      
   .. cpp:function:: bool getSourceObj(std::vector<BPatch_sourceObj *> &sources)
      
      Fill sources with the source objects (see section 4.6) that belong to
      this image. If there are no source objects, the function returns false.
      Otherwise, it returns true.
      
   .. cpp:function:: std::vector<BPatch_function *> *getProcedures(bool incUninstrumentable = false)
      
      Return a vector of the functions in the image. If the
      incUninstrumentable flag is set, the returned table of procedures will
      include uninstrumentable functions. The default behavior is to omit
      these functions.
      
   .. cpp:function:: void getObjects(std::vector<BPatch_object *> &objs)
      
      Fill in a vector of objects in the image.
      
   .. cpp:function:: std::vector<BPatch_module *> *getModules()
      
      Return a vector of the modules in the image.
      
   .. cpp:function:: bool getVariables(std::vector<BPatch_variableExpr *> &vars)
      
      Fills vars with the global variables defined in this image. If there are
      no variable, the function returns false. Otherwise, it returns true.
      
   .. cpp:function:: std::vector<BPatch_function*> *findFunction( \
         const char *name, \
         std::vector<BPatch_function*> &funcs, \
         bool showError = true, \
         bool regex_case_sensitive = true, \
         bool incUninstrumentable = false)
      
      Return a vector of BPatch_functions corresponding to name, or NULL if
      the function does not exist. If name contains a POSIX-extended regular
      expression, and dont_use_regex is false, a regular expression search
      will be performed on function names and matching BPatch_functions
      returned. If showError is true, then Dyninst will report an error via
      the BPatch::registerErrorCallback if no function is found.
      
      If the incUninstrumentable flag is set, the returned table of procedures
      will include uninstrumentable functions. The default behavior is to omit
      these functions.
      
      .. note::
      
         If name is not found to match any demangled function names in
         the module, the search is repeated as if name is a mangled function
         name. If this second search succeeds, functions with mangled names
         matching name are returned instead.
      
   .. cpp:function:: std::vector<BPatch_function*> *findFunction( \
         std::vector<BPatch_function*> &funcs, \
         BPatchFunctionNameSieve bpsieve,\ 
         void *sieve_data = NULL,\ 
         int showError = 0,\ 
         bool incUninstrumentable = false)
      
      Return a vector of BPatch_functions according to the generalized
      user-specified filter function bpsieve. This permits users to easily
      build sets of functions according to their own specific criteria.
      Internally, for each BPatch_function f in the image, this method makes a
      call to bpsieve(f.getName(), sieve_data). The user-specified function
      bpsieve is responsible for taking the name argument and determining if
      it belongs in the output vector, possibly by using extra user-provided
      information stored in sieve_data. If the name argument matches the
      desired criteria, bpsieve should return true. If it does not, bpsieve
      should return false.
      
      The function bpsieve should be defined in accordance with the typedef:
      
   .. cpp:type:: bool (*BPatchFunctionNameSieve) (const char *name, void* sieve_data);
      
      If the incUninstrumentable flag is set, the returned table of procedures
      will include uninstrumentable functions. The default behavior is to omit
      these functions.
      
   .. cpp:function:: bool findFunction(Dyninst::Address addr, std::vector<BPatch_function *>&funcs)
      
      Find all functions that have code at the given address, addr. Dyninst
      supports functions that share code, so this method may return more than
      one BPatch_function. These functions are returned via the funcs output
      parameter. This function returns true if it finds any functions, false
      otherwise.
      
   .. cpp:function:: BPatch_variableExpr *findVariable(const char *name, bool showError = true)
      
   .. cpp:function:: BPatch_variableExpr *findVariable(BPatch_point &scope, const char *name)
      
      second form of this method is not implemented on Windows.
      
      Performs a lookup and returns a handle to the named variable. The first
      form of the function looks up only variables of global scope, and the
      second form uses the passed BPatch_point as the scope of the variable.
      The returned BPatch_variableExpr can be used to create references (uses)
      of the variable in subsequent snippets. The scoping rules used will be
      those of the source language. If the image was not compiled with
      debugging symbols, this function will fail even if the variable is
      defined in the passed scope.
      
   .. cpp:function:: BPatch_type *findType(const char *name)
      
      Performs a lookup and returns a handle to the named type. The handle can
      be used as an argument to BPatch_addressSpace::malloc to create new
      variables of the corresponding type.
      
   .. cpp:function:: BPatch_module *findModule(const char *name, bool substring_match = false)
      
      Returns a module named name if present in the image. If the match fails,
      NULL is returned. If substring_match is true, the first module that has
      name as a substring of its name is returned (e.g. to find
      libpthread.so.1, search for libpthread with substring_match set to
      true).
      
   .. cpp:function:: bool getSourceLines(unsigned long addr, std::vector<BPatch_statement> & lines)
      
      Given an address addr, this function returns a vector of pairs of
      filenames and line numbers at that address. This function is an alias
      for BPatch_­process::getSourceLines (see section 4.4).
      
   .. cpp:function:: bool getAddressRanges( const char * fileName, unsigned int lineNo, \
         std::vector< std::pair< unsigned long, unsigned long > > & ranges )
      
      Given a file name and line number, fileName and lineNo, this function
      returns a list of address ranges that this source line was compiled
      into. This function is an alias for BPatch_process::getAddressRanges
      (see section 4.4).
      
   .. cpp:function:: bool parseNewFunctions(std::vector<BPatch_module*> &newModules, \
            const std::vector<Dyninst::Address> &funcEntryAddrs)
      
      This function takes as input a list of function entry points indicated
      by the funcEntryAddrs vector, which are used to seed parsing in whatever
      modules they are found. All affected modules are placed in the
      newModules vector, which includes any existing modules in which new
      functions are found, as well as modules corresponding to new regions of
      the binary, for which new BPatch_modules are created. The return value
      is true in the event that at least one previously unknown function was
      identified, or false otherwise.