.. _`sec-dev:BPatch_snippet.h`:

BPatch_snippet.h
################

.. cpp:namespace:: dev

.. cpp:class:: BPatch_snippet
   
  .. cpp:function:: BPatch_type * getType()

    Returns the type of the underlying AST.

    DynC internal use only.

.. cpp:class:: BPatch_variableExpr : public BPatch_snippet
   
  .. cpp:function:: private BPatch_variableExpr(const char* in_name, BPatch_addressSpace* in_addSpace,\
                                                AddressSpace* as, AstNodePtr ast_wrapper_, BPatch_type *type,\
                                                void* in_address)

    Used to get expressions for the components of a structure Used to get function pointers

  .. cpp:function:: private BPatch_variableExpr(BPatch_addressSpace *in_addSpace, AddressSpace *as,\
                                                void *in_address, int in_register, BPatch_type *type,\
                                                BPatch_storageClass storage = BPatch_storageAddr,\
                                                BPatch_point *scp = NULL)

    Used to get forked copies of variable expressions Used by malloc & malloc_by_type

  .. cpp:function:: private BPatch_variableExpr(BPatch_addressSpace *in_addSpace, AddressSpace *as,\
                                                BPatch_localVar *lv,BPatch_type *type, BPatch_point *scp)

    Used for locals

  .. cpp:function:: private BPatch_variableExpr(BPatch_addressSpace *in_addSpace, AddressSpace *ll_addSpace,\
                                                int_variable *iv, BPatch_type *type)

  .. cpp:function:: private BPatch_variableExpr(const char name, BPatch_addressSpace in_addSpace,\
                                                AddressSpace ll_addSpace, void in_address, BPatch_type type)

      Used by findOrCreateVariable

  .. cpp:function:: static BPatch_variableExpr* makeVariableExpr(BPatch_addressSpace* in_addSpace,\
                                                                  int_variable* v, BPatch_type* type)


.. cpp:class:: BPatch_ifMachineConditionExpr : public BPatch_snippet

  It is possible to have a more general expression, say machineConditionExpr, then have this
  reimplemented as ifExpr(machineConditionExpr, ...), and have an optimization (fast path) for
  that case using the specialized AST that supports this class. Memory instrumentation has no
  need for a standalone machineConditionExpr, so that remains TBD...


.. cpp:class:: BPatch_stopThreadExpr : public BPatch_snippet

  .. cpp:function:: BPatch_stopThreadExpr(const BPatchStopThreadCallback &cb,\
                                          const BPatch_snippet &calculation, const mapped_object &obj,\
                                          bool useCache = false, BPatch_stInterpret interp = BPatch_noInterp)

    For internal use in conjunction with memory emulation and defensive  mode analysis