.. _`sec:InsnWidget.h`:

InsnWidget.h
############

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: InsnWidget : public Widget

  .. cpp:type:: boost::shared_ptr<InsnWidget> Ptr
  .. cpp:function:: virtual bool generate(const codeGen &, const RelocBlock *, CodeBuffer &)
  .. cpp:function:: TrackerElement *tracker(const RelocBlock *trace) const
  .. cpp:function:: static Ptr create(InstructionAPI::Instruction insn, Address addr)
  .. cpp:function:: virtual ~InsnWidget()
  .. cpp:function:: virtual std::string format() const
  .. cpp:function:: virtual InstructionAPI::Instruction insn() const
  .. cpp:function:: virtual Address addr() const
  .. cpp:function:: virtual unsigned size() const
  .. cpp:function:: private InsnWidget(InstructionAPI::Instruction insn, Address addr)
  .. cpp:member:: private InstructionAPI::Instruction insn_

    Pointer to the instruction we represent

  .. cpp:member:: private Address addr_

    Original address of this instruction

