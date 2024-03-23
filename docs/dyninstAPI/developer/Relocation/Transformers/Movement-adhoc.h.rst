.. _`sec:Movement-adhoc.h`:

Movement-adhoc.h
################

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: adhocMovementTransformer : public Transformer

  Identify PC-relative memory accesses and replace them with a dedicated Widget

  .. cpp:type:: private boost::shared_ptr<RelocInsn> RelocInsnPtr
  .. cpp:type:: private boost::shared_ptr<InstructionAPI::Instruction> InsnPtr

  .. cpp:function:: virtual bool process(RelocBlock *, RelocGraph *)
  .. cpp:function:: adhocMovementTransformer(AddressSpace *as)
  .. cpp:function:: virtual ~adhocMovementTransformer()
  .. cpp:function:: private bool isPCDerefCF(WidgetPtr ptr, InstructionAPI::Instruction insn, Address &destPtr)
  .. cpp:function:: private bool isPCRelData(WidgetPtr ptr, InstructionAPI::Instruction insn, Address &target)

    We define this as "uses PC and is not control flow"

  .. cpp:function:: private bool isGetPC(WidgetPtr ptr, InstructionAPI::Instruction insn, Absloc &aloc, Address &thunkAddr)

    Records where PC was stored

  .. cpp:function:: private bool isStackFrameSensitive(Offset& origOffset, signed long& delta,\
                                                       const Accesses* accesses, OffsetVector*& offVec,\
                                                       TMap*& tMap, ParseAPI::Block* block, Address addr)

    Determines if an instruction is stack frame sensitive (i.e. needs to be updated with a new displacement). If so,
    returns in delta the amount by which the displacement needs to be updated.

  .. cpp:member:: private AddressSpace *addrSpace

    Used for finding call targets

  .. cpp:member:: private std::map<Address, std::pair<Offset, signed long> > definitionDeltas

    Map of definition addresses to (origDisp, delta) pairs

