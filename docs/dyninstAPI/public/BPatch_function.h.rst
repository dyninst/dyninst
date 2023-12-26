BPatch_function.h
=================

``BPatch_function``
-------------------
.. cpp:namespace:: BPatch_function

.. cpp:class:: BPatch_function
   
   An object of this class represents a function in the application. A
   BPatch_image object (see description below) can be used to retrieve a
   BPatch_function object representing a given function.
   
   .. cpp:function:: std::string getName();
      
   .. cpp:function:: std::string getDemangledName();
      
   .. cpp:function:: std::string getMangledName();
      
   .. cpp:function:: std::string getTypedName();
      
   .. cpp:function:: void getNames(std::vector<std::string> &names);
      
   .. cpp:function:: void getDemangledNames(std::vector<std::string> &names);
      
   .. cpp:function:: void getMangledNames(std::vector<std::string> &names);
      
   .. cpp:function:: void getTypedNames(std::vector<std::string> &names);
      
      Return name(s) of the function. The getName functions return the primary
      name; this is typically the first symbol we encounter while parsing the
      program; getName is an alias for getDemangledName. The getNames
      functions return all known names for the function, including any names
      specified by weak symbols.
      
   .. cpp:function:: bool getAddressRange(Dyninst::Address &start, Dyninst::Address &end)
      
      Returns the bounds of the function; for non-contiguous functions, this
      is the lowest and highest address of code that the function includes.
      
   .. cpp:function:: std::vector<BPatch_localVar *> *getParams()
      
      Return a vector of BPatch_localVar snippets that refer to the parameters
      of this function. The position in the vector corresponds to the position
      in the parameter list (starting from zero). The returned local variables
      can be used to check the types of functions, and can be used in snippet
      expressions.
      
   .. cpp:function:: BPatch_type *getReturnType()
      
      Return the type of the return value for this function.
      
   .. cpp:function:: BPatch_variableExpr *getFunctionRef()
      
      For platforms with complex function pointers (e.g., 64-bit PPC) this
      constructs and returns the appropriate descriptor.
      
   .. cpp:function:: std::vector<BPatch_localVar *> *getVars()
      
      Returns a vector of BPatch_localVar objects that contain the local
      variables in this function. These BPatch_localVars can be used as parts
      of snippets in instrumentation. This function requires debug information
      to be present in the mutatee. If Dyninst was unable to find any local
      variables, this function will return an empty vector. It is up to the
      user to free the vector returned by this function.
      
   .. cpp:function:: bool isInstrumentable()
      
      Return true if the function can be instrumented, and false if it cannot.
      Various conditions can cause a function to be uninstrumentable. For
      example, there exists a platform-specific minimum function size beyond
      which a function cannot be instrumented.
      
   .. cpp:function:: bool isSharedLib()
      
      This function returns true if the function is defined in a shared
      library.
      
   .. cpp:function:: BPatch_module *getModule()
      
      Return the module that contains this function. Depending on whether the
      program was compiled for debugging or the symbol table stripped, this
      information may not be available. This function returns NULL if module
      information was not found.
      
   .. cpp:function:: char *getModuleName(char *name, int maxLen)
      
      Copies the name of the module that contains this function into the
      buffer pointed to by name. Copies at most maxLen characters and returns
      a pointer to name.
      
   .. cpp:enum:: BPatch_procedureLocation
   .. cpp:enumerator:: BPatch_procedureLocation::BPatch_entry
   .. cpp:enumerator:: BPatch_procedureLocation::BPatch_exit
   .. cpp:enumerator:: BPatch_procedureLocation::BPatch_subroutine
   .. cpp:enumerator:: BPatch_procedureLocation::BPatch_locInstruction
   .. cpp:enumerator:: BPatch_procedureLocation::BPatch_locBasicBlockEntry
   .. cpp:enumerator:: BPatch_procedureLocation::BPatch_locLoopEntry
   .. cpp:enumerator:: BPatch_procedureLocation::BPatch_locLoopExit
   .. cpp:enumerator:: BPatch_procedureLocation::BPatch_locLoopStartIter
   .. cpp:enumerator:: BPatch_procedureLocation::BPatch_locLoopStartExit
   .. cpp:enumerator:: BPatch_procedureLocation::BPatch_allLocations
      
   .. cpp:function:: const std::vector<BPatch_point *> *findPoint(const BPatch_procedureLocation loc)
      
      Return the BPatch_point or list of BPatch_points associated with the
      procedure. It is used to select which type of points associated with the
      procedure will be returned. BPatch_entry and BPatch_exit request
      respectively the entry and exit points of the subroutine.
      BPatch_subroutine returns the list of points where the procedure calls
      other procedures. If the lookup fails to locate any points of the
      requested type, NULL is returned.
      
   .. cpp:enum:: BPatch_opCode
   .. cpp:enumerator:: BPatch_opCode::BPatch_opLoad
   .. cpp:enumerator:: BPatch_opCode::BPatch_opStore
   .. cpp:enumerator:: BPatch_opCode::BPatch_opPrefetch
      
   .. cpp:function:: std::vector<BPatch_point *> *findPoint(const std::set<BPatch_opCode>&ops)
      
   .. cpp:function:: std::vector<BPatch_point *> *findPoint(const BPatch_Set<BPatch_opCode>& ops)
      
      Return the vector of BPatch_points corresponding to the set of machine
      instruction types described by the argument. This version is used
      primarily for memory access instrumentation. The BPatch_opCode is an
      enumeration of instruction types that may be requested: BPatch_opLoad,
      BPatch_opStore, and BPatch_opPrefetch. Any combination of these may be
      requested by passing an appropriate argument set containing the desired
      types. The instrumentation points created by this function have
      additional memory access information attached to them. This allows such
      points to be used for memory access specific snippets (e.g. effective
      address). The memory access information attached is described under
      Memory Access classes in section 4.27.1.
      
   .. cpp:function:: BPatch_localVar *findLocalVar(const char *name)
      
      Search the function’s local variable collection for name. This returns a
      pointer to the local variable if a match is found. This function returns
      NULL if it fails to find any variables.
      
   .. cpp:function:: std::vector<BPatch_variableExpr *> *findVariable(const char * name)
      
   .. cpp:function:: bool findVariable(const char *name, std::vector<BPatch_variableExpr> &vars)
      
      Return a set of variables matching name at the scope of this function.
      If no variables match in the local scope, then the global scope will be
      searched for matches. This function returns NULL if it fails to find any
      variables.
      
   .. cpp:function:: BPatch_localVar *findLocalParam(const char *name)
      
      Search the function’s parameters for a given name. A BPatch_localVar *
      pointer is returned if a match is found, and NULL is returned otherwise.
      
   .. cpp:function:: void *getBaseAddr()
      
      Return the starting address of the function in the mutatee’s address
      space.
      
   .. cpp:function:: BPatch_flowGraph *getCFG()
      
      Return the control flow graph for the function, or NULL if this
      information is not available. The BPatch_flowGraph is described in
      section 4.16.
      
   .. cpp:function:: bool findOverlapping(std::vector<BPatch_function *> &funcs)
      
      Determine which functions overlap with the current function (see Section
      2.). Return true if other functions overlap the current function; the
      overlapping functions are added to the funcs vector. Return false if no
      other functions overlap the current function.
      
   .. cpp:function:: bool addMods(std::set<StackMod *> mods)
      
      implemented on x86 and x86-64
      
      Apply stack modifications in mods to the current function; the StackMod
      class is described in section 4.25. Perform error checking, handle stack
      alignment requirements, and generate any modifications required for
      cleanup at function exit. addMods atomically adds all modifications in
      mods; if any mod is found to be unsafe, none of the modifications in
      mods will be applied.
      
      addMods can only be used in binary rewriting mode.
      
      Returns false if the stack modifications are unsafe or if Dyninst is
      unable to perform the analysis required to guarantee safety.