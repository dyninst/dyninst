.. _`sec-dev:Point.h`:

Point.h
#######

.. cpp:namespace:: Dyninst::PatchAPI::dev

.. cpp:class:: Point

  .. cpp:function:: protected bool destroy()

    1. Clear all snippet instances
    2. Detach from PatchMgr object

  .. cpp:function:: protected void initCodeStructure()

    Associate this point with the block(s) and function(s) that contain it

  .. cpp:function:: protected void changeBlock(PatchBlock *newBlock)

  .. cpp:member:: protected InstanceList instanceList_
  .. cpp:member:: protected Dyninst::Address addr_
  .. cpp:member:: protected Type type_
  .. cpp:member:: protected PatchMgrPtr mgr_
  .. cpp:member:: protected PatchBlock* the_block_
  .. cpp:member:: protected PatchEdge* the_edge_
  .. cpp:member:: protected PatchFunction* the_func_
  .. cpp:member:: protected InstructionAPI::Instruction insn_


.. cpp:class:: Location

  .. cpp:function:: Location(PatchFunction *f, PatchBlock *b, Dyninst::Address a, \
                             InstructionAPI::Instruction i, PatchEdge *e, bool u, type_t t)

.. code:: c

  // Used in PointType definition
  #define type_val(seq) (0x00000001u << seq)

.. cpp:enum:: Point::Type

  When extending Type, be sure to increment the type_val argument.

  .. cpp:enumerator:: PreInsn = type_val(0)
  .. cpp:enumerator:: PostInsn = type_val(1)
  .. cpp:enumerator:: BlockEntry = type_val(3)
  .. cpp:enumerator:: BlockExit = type_val(4)
  .. cpp:enumerator:: BlockDuring = type_val(5)
  .. cpp:enumerator:: FuncEntry = type_val(6)
  .. cpp:enumerator:: FuncExit = type_val(7)
  .. cpp:enumerator:: FuncDuring = type_val(8)
  .. cpp:enumerator:: EdgeDuring = type_val(9)
  .. cpp:enumerator:: LoopStart = type_val(10)
  .. cpp:enumerator:: LoopEnd = type_val(11)
  .. cpp:enumerator:: LoopIterStart = type_val(12)
  .. cpp:enumerator:: LoopIterEnd = type_val(13)
  .. cpp:enumerator:: PreCall = type_val(14)
  .. cpp:enumerator:: PostCall = type_val(15)
  .. cpp:enumerator:: OtherPoint = type_val(30)
  .. cpp:enumerator:: None = type_val(31)
  .. cpp:enumerator:: InsnTypes = PreInsn | PostInsn
  .. cpp:enumerator:: BlockTypes = BlockEntry | BlockExit | BlockDuring
  .. cpp:enumerator:: FuncTypes = FuncEntry | FuncExit | FuncDuring
  .. cpp:enumerator:: EdgeTypes = EdgeDuring
  .. cpp:enumerator:: LoopTypes = LoopStart | LoopEnd | LoopIterStart | LoopIterEnd
  .. cpp:enumerator:: CallTypes = PreCall | PostCall

.. cpp:class:: Instance : public boost::enable_shared_from_this<Instance>

  .. cpp:function:: void set_state(SnippetState state)
  .. cpp:function:: void disableRecursiveGuard()
  .. cpp:function:: bool recursiveGuardEnabled() const

  .. cpp:member:: protected Point* point_
  .. cpp:member:: protected SnippetPtr snippet_
  .. cpp:member:: protected SnippetState state_
  .. cpp:member:: protected SnippetType type_
  .. cpp:member:: protected bool guarded_

.. cpp:class:: PointMaker

  .. cpp:member:: PatchMgrPtr mgr_

  .. cpp:function:: void setMgr(PatchMgrPtr mgr)
  .. cpp:function:: protected virtual Point *mkFuncPoint(Point::Type t, PatchMgrPtr m, PatchFunction *)
  .. cpp:function:: protected virtual Point *mkFuncSitePoint(Point::Type t, PatchMgrPtr m, PatchFunction *, PatchBlock *)
  .. cpp:function:: protected virtual Point *mkBlockPoint(Point::Type t, PatchMgrPtr m, PatchBlock *, PatchFunction *context)
  .. cpp:function:: protected virtual Point *mkInsnPoint(Point::Type t, PatchMgrPtr m, PatchBlock *, Dyninst::Address, InstructionAPI::Instruction, PatchFunction *context)
  .. cpp:function:: protected virtual Point *mkEdgePoint(Point::Type t, PatchMgrPtr m, PatchEdge *, PatchFunction *context)

.. cpp:struct:: BlockPoints

  .. cpp:member:: Point *entry
  .. cpp:member:: Point *during
  .. cpp:member:: Point *exit
  .. cpp:member:: InsnPoints preInsn
  .. cpp:member:: InsnPoints postInsn

  .. cpp:function:: bool consistency(const PatchBlock *block, const PatchFunction *func) const

.. cpp:struct:: EdgePoints

  .. cpp:member:: Point* during

  .. cpp:function:: bool consistency(const PatchEdge *edge, const PatchFunction *func) const

.. cpp:struct:: FuncPoints

  .. cpp:member:: Point* entry
  .. cpp:member:: Point* during
  .. cpp:member:: std::map<PatchBlock*, Point*> exits
  .. cpp:member:: std::map<PatchBlock*, Point*> preCalls
  .. cpp:member:: std::map<PatchBlock*, Point*> postCalls

  .. cpp:function:: bool consistency(const PatchFunction *func) const

