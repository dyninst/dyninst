.. _`sec-dev:BPatch_function.h`:

BPatch_function.h
#################

.. cpp:namespace:: dev

.. cpp:class:: BPatch_function : public BPatch_sourceObj, public Dyninst::AnnotatableSparse

  .. cpp:member:: BPatch_localVarCollection * localVariables
  .. cpp:member:: BPatch_localVarCollection * funcParameters

  .. cpp:function:: BPatch_function(BPatch_addressSpace *_addSpace, func_instance *_func, BPatch_module *mod = NULL)
  .. cpp:function:: BPatch_function(BPatch_addressSpace *_addSpace, func_instance *_func, BPatch_type * _retType, BPatch_module *)
  .. cpp:function:: virtual ~BPatch_function()

  .. cpp:function:: bool hasParamDebugInfo()

    dynC internal use only

  .. cpp:function:: func_instance *lowlevel_func() const
  .. cpp:function:: BPatch_process *getProc() const
  .. cpp:function:: BPatch_addressSpace *getAddSpace() const
  .. cpp:function:: bool getSourceObj(BPatch_Vector<BPatch_sourceObj *> &)

      Return the contained source objects (e.g. statements). This is not currently supported.

  .. cpp:function:: BPatch_sourceObj *getObjParent()

      Return the parent of the function (i.e. the module)

  .. cpp:function:: void setReturnType(BPatch_type * _retType)
  .. cpp:function:: void setModule(BPatch_module *module)
  .. cpp:function:: void removeCFG()
  .. cpp:function:: void getUnresolvedControlTransfers(BPatch_Vector<BPatch_point *> &unresolvedCF)

      Gets unresolved instPoints from func_instance and converts them to BPatch_points, puts them in unresolvedCF

  .. cpp:function:: void getAbruptEndPoints(BPatch_Vector<BPatch_point *> &abruptEnds)

      Gets abrupt end instPoints from func_instance and converts them to BPatch_points, puts them in abruptEnds

  .. cpp:function:: void getCallerPoints(BPatch_Vector<BPatch_point*>& callerPoints)

      This one is interesting - get call points for everywhere that _calls_ us. That we know about.

  .. cpp:function:: void getAllPoints(BPatch_Vector<BPatch_point*>& allPoints)
  .. cpp:function:: void getEntryPoints(BPatch_Vector<BPatch_point *> &entryPoints)
  .. cpp:function:: void getExitPoints(BPatch_Vector<BPatch_point *> &entryPoints)
  .. cpp:function:: void getCallPoints(BPatch_Vector<BPatch_point *> &entryPoints)
  .. cpp:function:: bool setHandlerFaultAddrAddr(Dyninst::Address addr, bool set)

      Sets the address in the structure at which the fault instruction's address is stored if "set" is true.
      Accesses the fault address and  translates it back to an original address if it corresponds to relocated
      code in the Dyninst heap

  .. cpp:function:: bool removeInstrumentation(bool useInsertionSet)

      Removes all instrumentation and relocation from the function and  restores the original version
      Also flushes the runtime library address cache, if present

  .. cpp:function:: bool parseNewEdge(Dyninst::Address source, Dyninst::Address target)

      Update code bytes if necessary (defensive mode), Parse new edge,  Correct missing elements in BPatch-level datastructures

  .. cpp:function:: void relocateFunction()
  .. cpp:function:: bool getSharedFuncs(std::set<BPatch_function*> &funcs)
  .. cpp:function:: void addParam(Dyninst::SymtabAPI::localVar *lvar)

      This function adds a function parameter to the BPatch_function parameter vector.

  .. cpp:function:: void fixupUnknown(BPatch_module *)
  .. cpp:function:: bool containsSharedBlocks()

    This isn't so much for internal use only, but it should remain undocumented for now.

  .. cpp:function:: bool getVariables(BPatch_Vector<BPatch_variableExpr *> &vect)

    This returns false, and should probably not exist. See getVars.

  .. cpp:function:: const char* addName(const char *name, bool isPrimary = true, bool isMangled = false)

      isPrimary: function will now use this name as a primary output name
      isMangled: this is the "mangled" name rather than demangled (pretty)

  .. cpp:function:: bool addMods(std::set<StackMod*>)

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
