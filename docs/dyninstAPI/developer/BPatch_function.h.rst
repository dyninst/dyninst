.. _`sec-dev:BPatch_function.h`:

BPatch_function.h
#################

.. cpp:namespace:: dev

.. cpp:class:: BPatch_function : public BPatch_sourceObj, public Dyninst::AnnotatableSparse

  .. cpp:member:: BPatch_localVarCollection * localVariables
  .. cpp:member:: BPatch_localVarCollection * funcParameters

  .. cpp:function:: bool hasParamDebugInfo()

    dynC internal use only

  .. cpp:function:: func_instance *lowlevel_func() const
  .. cpp:function:: BPatch_process *getProc() const
  .. cpp:function:: BPatch_addressSpace *getAddSpace() const
  .. cpp:function:: BPatch_function(BPatch_addressSpace *_addSpace, func_instance *_func, BPatch_module *mod = NULL)
  .. cpp:function:: BPatch_function(BPatch_addressSpace *_addSpace, func_instance *_func, BPatch_type * _retType, BPatch_module *)
  .. cpp:function:: bool getSourceObj(BPatch_Vector<BPatch_sourceObj *> &)
  .. cpp:function:: BPatch_sourceObj *getObjParent()
  .. cpp:function:: void setReturnType(BPatch_type * _retType)
  .. cpp:function:: void setModule(BPatch_module *module)
  .. cpp:function:: void removeCFG()
  .. cpp:function:: void getUnresolvedControlTransfers(BPatch_Vector<BPatch_point *> &unresolvedCF)
  .. cpp:function:: void getAbruptEndPoints(BPatch_Vector<BPatch_point *> &abruptEnds)
  .. cpp:function:: void getCallerPoints(BPatch_Vector<BPatch_point*>& callerPoints)
  .. cpp:function:: void getAllPoints(BPatch_Vector<BPatch_point*>& allPoints)
  .. cpp:function:: void getEntryPoints(BPatch_Vector<BPatch_point *> &entryPoints)
  .. cpp:function:: void getExitPoints(BPatch_Vector<BPatch_point *> &entryPoints)
  .. cpp:function:: void getCallPoints(BPatch_Vector<BPatch_point *> &entryPoints)
  .. cpp:function:: bool setHandlerFaultAddrAddr(Dyninst::Address addr, bool set)
  .. cpp:function:: bool removeInstrumentation(bool useInsertionSet)
  .. cpp:function:: bool parseNewEdge(Dyninst::Address source, Dyninst::Address target)
  .. cpp:function:: void relocateFunction()
  .. cpp:function:: bool getSharedFuncs(std::set<BPatch_function*> &funcs)
  .. cpp:function:: void addParam(Dyninst::SymtabAPI::localVar *lvar)
  .. cpp:function:: void fixupUnknown(BPatch_module *)
  .. cpp:function:: bool containsSharedBlocks()

    This isn't so much for internal use only, but it should remain undocumented for now.
