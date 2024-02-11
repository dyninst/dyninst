.. _`sec:instPoint.h`:

instPoint.h
###########

..
  This class doesn't live in PatchAPI. The namespace is just here to help with Point::Type lookup.
.. cpp:namespace:: Dyninst::PatchAPI

.. cpp:class:: instPoint : public Dyninst::PatchAPI::Point

  We can restrict instrumentation to a particular instance of a block, edge, or instruction by additionally
  specifying a function for context.

  .. cpp:function:: static instPoint *funcEntry(func_instance *)
  .. cpp:function:: static instPoint *funcExit(func_instance *, block_instance *exitPoint)
  .. cpp:function:: static instPoint *blockEntry(func_instance *, block_instance *)
  .. cpp:function:: static instPoint *blockExit(func_instance *, block_instance *)
  .. cpp:function:: static instPoint *edge(func_instance *, edge_instance *)
  .. cpp:function:: static instPoint *preInsn(func_instance *, block_instance *, Dyninst::Address, Dyninst::InstructionAPI::Instruction = Dyninst::InstructionAPI::Instruction(), bool trusted = false)
  .. cpp:function:: static instPoint *postInsn(func_instance *, block_instance *, Dyninst::Address, Dyninst::InstructionAPI::Instruction = Dyninst::InstructionAPI::Instruction(), bool trusted = false)
  .. cpp:function:: static instPoint *preCall(func_instance *, block_instance *)
  .. cpp:function:: static instPoint *postCall(func_instance *, block_instance *)

  .. cpp:function:: static std::pair<instPoint *, instPoint *> getInstpointPair(instPoint *)
  .. cpp:function:: static instPoint *fork(instPoint *parent, AddressSpace *as)
  .. cpp:function:: ~instPoint()

  .. cpp:function:: private instPoint(Point::Type, PatchMgrPtr m, func_instance *)
  .. cpp:function:: private instPoint(Point::Type, PatchMgrPtr, func_instance *, block_instance *)

    Call/exit site

  .. cpp:function:: private instPoint(Point::Type, PatchMgrPtr, block_instance *, func_instance *)

    (possibly func context) block

  .. cpp:function:: private instPoint(Point::Type, PatchMgrPtr, block_instance *, Dyninst::Address, Dyninst::InstructionAPI::Instruction, func_instance *)
  .. cpp:function:: private instPoint(Point::Type, PatchMgrPtr, edge_instance *, func_instance *)

  .. cpp:function:: baseTramp *tramp()
  .. cpp:function:: AddressSpace *proc() const
  .. cpp:function:: func_instance *func() const
  .. cpp:function:: block_instance *block() const
  .. cpp:function:: edge_instance *edge() const
  .. cpp:function:: Dyninst::Address insnAddr() const

    instPoints have two Point::Types of addresses. The first is "instrument before or after the insn at this addr".
    The second is a best guess as to the *next* address that will execute.

  .. cpp:function:: block_instance *block_compat() const

    This is for address tracking... if we're between blocks (e.g., post-call, function exit, or edge instrumentation) and thus aren't
    strongly tied to a block give us the next block that will execute. Unlike block() above, this always works.

  .. cpp:function:: Dyninst::Address addr_compat() const
  .. cpp:function:: bitArray liveRegisters()
  .. cpp:function:: std::string format() const
  .. cpp:function:: virtual Dyninst::PatchAPI::InstancePtr pushBack(Dyninst::PatchAPI::SnippetPtr)
  .. cpp:function:: virtual Dyninst::PatchAPI::InstancePtr pushFront(Dyninst::PatchAPI::SnippetPtr)
  .. cpp:function:: void markModified()
  .. cpp:member:: private bitArray liveRegs_
  .. cpp:function:: private void calcLiveness()
  .. cpp:function:: private static bool checkInsn(block_instance *, Dyninst::InstructionAPI::Instruction &insn, Dyninst::Address a)

    Will fill in insn if it's NULL-equivalent

  .. cpp:member:: private baseTramp *baseTramp_


.. cpp:function:: Dyninst::PatchAPI::InstancePtr getChildInstance(Dyninst::PatchAPI::InstancePtr parentInstance, AddressSpace *childProc)

.. code:: cpp

  #define IPCONV(p) (static_cast<instPoint *>(p))

