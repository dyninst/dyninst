.. _`sec:BPatch_function.h`:

BPatch_function.h
#################

.. cpp:class:: BPatch_function : public BPatch_sourceObj, public Dyninst::AnnotatableSparse
   
  **A function in a binary**

  .. cpp:function:: std::string getName()

      The same as :cpp:func:`getDemangledName`.

  .. cpp:function:: std::string getMangledName()

      Returns mangled name of the function.

  .. cpp:function:: std::string getDemangledName()

      Returns the demangled name fo the function.

  .. cpp:function:: std::string getTypedName()

      Returns demangled name of function (with type string)

  .. cpp:function:: bool getNames(std::vector<std::string> &names)

      Returns in ``names`` all names of the function (including weak symbols).

  .. cpp:function:: bool getDemangledNames(std::vector<std::string> &names)

      Returns in ``names`` all demangled names of the function (including weak symbols).

  .. cpp:function:: bool getMangledNames(std::vector<std::string> &names)

      Returns in ``names`` all mangled names of the function (including weak symbols).

  .. cpp:function:: bool getTypedNames(std::vector<std::string> &names)

      Returns in ``names`` all demangled names of the function (including weak symbols) with
      an additional representation of the type.

  .. cpp:function:: void* getBaseAddr()

      Return the starting address of the function in the mutatee’s address space.

  .. cpp:function:: BPatch_type* getReturnType()

      Returns type of this function

  .. cpp:function:: BPatch_module* getModule()

      Returns the module that contains this function.

      Depending on whether the program was compiled for debugging or the symbol table stripped,
      this information may not be available.

      Returns ``NULL`` if no module was not found.

  .. cpp:function:: BPatch_Vector<BPatch_localVar*>* getParams()

      Returns the formal parameters of this function.

      The position in the returned vector corresponds to the position
      in the parameter list (starting from zero).

  .. cpp:function:: BPatch_Vector<BPatch_localVar*>* getVars()

      Returns the local variables in this function.

      Requires debug information in the mutatee.

  .. cpp:function:: BPatch_Vector<BPatch_point*>* findPoint(const BPatch_procedureLocation loc)

      Returns instrumentation points with the type ``loc``.

  .. cpp:function:: BPatch_Vector<BPatch_point*>* findPoint(const BPatch_Set<BPatch_opCode>& ops)

      Returns instrumentation points corresponding to the set of machine instruction types in ``ops``.

      This version is used primarily for memory access instrumentation. Any combination of opcode
      types may passed. The instrumentation points created by this function have
      additional memory access information attached to them. This allows such
      points to be used for memory access specific snippets (e.g. effective
      address).

  .. cpp:function:: BPatch_Vector<BPatch_point*>* findPoint(const std::set<BPatch_opCode>& ops)

      Returns instrumentation points corresponding to the set of opcodes in ``ops``.

  .. cpp:function:: BPatch_point* findPoint(Dyninst::Address addr)

      Returns the point at the address ``addr``, if it exists.

  .. cpp:function:: BPatch_localVar* findLocalVar(const char* name)

      Returns the local variable with name ``name``, if it exists.

  .. cpp:function:: BPatch_localVar* findLocalParam(const char* name)

      Returns the formal parameter with name ``name``, if it exists.

  .. cpp:function:: BPatch_Vector<BPatch_variableExpr*>* findVariable(const char *name)

      Returns all local variables in this function or at global scope with name ``name``, if any exist.

  .. cpp:function:: char* getModuleName(char *name, int maxLen)

      Returns name of module this function belongs to

  .. cpp:function:: bool isInstrumentable()

      Checks if the function is instrumentable.

      Various conditions can cause a function to be uninstrumentable. For
      example, there exists a platform-specific minimum function size beyond
      which a function cannot be instrumented.

  .. cpp:function:: bool isSharedLib()

      Checks if this function lives in a shared library

  .. cpp:function:: BPatch_flowGraph* getCFG()

      Returns the control flow graph for the function

  .. cpp:function:: operator Dyninst::ParseAPI::Function *() const

      Returns the underlying ParseAPI function.

  .. cpp:function:: operator Dyninst::PatchAPI::PatchFunction *() const

      Returns the underlying PatchAPI function.

  .. cpp:function:: bool getAddressRange(void* &start, void* &end)

      Returns in ``start`` and ``end`` the bounds of the function.

      For non-contiguous functions, this is the lowest and highest address of code that the
      function includes.

  .. cpp:function:: bool getAddressRange(Dyninst::Address &start, Dyninst::Address &end)

      Returns in ``start`` and ``end`` the bounds of the function.

      For non-contiguous functions, this is the lowest and highest address of code that the
      function includes.

  .. cpp:function:: unsigned int getFootprint()

  .. cpp:function:: BPatch_variableExpr *getFunctionRef()

      For platforms with complex function pointers (e.g., 64-bit PPC) this
      constructs and returns the appropriate descriptor.

  .. cpp:function:: bool findOverlapping(BPatch_Vector<BPatch_function *> &funcs)

      Returns in ``funcs`` the functions that overlap with the current function.

      Returns ``true`` if any were found.

  ......

  .. rubric:: Deprecated methods

  .. cpp:function:: char* getName(char *s, int len)
  .. cpp:function:: char* getMangledName(char *s, int len)
  .. cpp:function:: char* getTypedName(char *s, int len)
  .. cpp:function:: bool getNames(BPatch_Vector<const char *> &names)
  .. cpp:function:: bool getMangledNames(BPatch_Vector<const char *> &names)

