.. _`sec-dev:BPatch_snippet.h`:

BPatch_snippet.h
################

.. cpp:namespace:: dev

.. cpp:class:: BPatch_snippet

  .. cpp:function:: BPatch_snippet()

    The default constructor was exposed as public, so we're stuck with it even though that *should* be
    an error. For now, make it a null node (and hope nobody ever tries to generate code).

  .. cpp:function:: BPatch_snippet & operator=(const BPatch_snippet &src)

    Needed to ensure that the reference counts for the asts contained in the snippets is correct.

  .. cpp:function:: BPatch_type * getType()

    Returns the type of the underlying AST.

    DynC internal use only.


.. cpp:function:: AstNodePtr generateArrayRef(const BPatch_snippet &lOperand, const BPatch_snippet &rOperand)

  Construct an Ast expression for an array.


.. cpp:function:: AstNodePtr generateFieldRef(const BPatch_snippet &lOperand, const BPatch_snippet &rOperand)

  Construct an Ast expression for an structure field.


.. cpp:class:: BPatch_variableExpr : public BPatch_snippet
   
  .. cpp:function:: private BPatch_variableExpr(const char* in_name, BPatch_addressSpace* in_addSpace,\
                                                AddressSpace* as, AstNodePtr ast_wrapper_, BPatch_type *type,\
                                                void* in_address)

  .. cpp:function:: private BPatch_variableExpr(BPatch_addressSpace *in_addSpace, AddressSpace *as,\
                                                void *in_address, int in_register, BPatch_type *type,\
                                                BPatch_storageClass storage = BPatch_storageAddr,\
                                                BPatch_point *scp = NULL)

    Constructs a snippet representing a variable of the given type at the given address.

    - ``in_addSpace``: The BPatch_addressSpace that the variable resides in.
    - ``in_address``:   The address of the variable in the inferior's address space.
    - ``in_register``:  The register of the variable in the inferior's address space.
    - ``type``: The type of the variable.
    - ``in_storage``:  Enum of how this variable is stored.

  .. cpp:function:: private BPatch_variableExpr(BPatch_addressSpace *in_addSpace, AddressSpace *as,\
                                                BPatch_localVar *lv, BPatch_type *type, BPatch_point *scp)

    Construct a snippet representing a variable of the given type at the given address.

    - ``in_addSpace``: The BPatch_addressSpace that the variable resides in.
    - ``lv``: The local variable handle
    - ``type``: The type of the variable.

  .. cpp:function:: private BPatch_variableExpr(BPatch_addressSpace *in_addSpace, AddressSpace *ll_addSpace,\
                                                int_variable *iv, BPatch_type *type)

  .. cpp:function:: private BPatch_variableExpr(const char *name, BPatch_addressSpace in_addSpace,\
                                                AddressSpace* ll_addSpace, void* in_address, BPatch_type type)

  .. cpp:function:: static BPatch_variableExpr* makeVariableExpr(BPatch_addressSpace* in_addSpace,\
                                                                  int_variable* v, BPatch_type* type)

  .. cpp:function:: bool BPatch_variableExpr::setType(BPatch_type *newType)

    Sets the variable's type.

  .. cpp:function:: bool BPatch_variableExpr::setSize(int sz)

    Sets the variable's size.


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


.. cpp:var:: static std::set<BPatchStopThreadCallback> *stopThread_cbs=NULL

  Causes us to create only one StopThreadCallback per BPatchStopThreadCallback, though we create a
  function call snippet to DYNINST_stopThread for each individual stopThreadExpr.  It's not necessary
  that we limit StopThreadCallbacks creations like this.

