.. _`sec:StackModWidget.h`:

StackModWidget.h
################

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: StackModWidget : public Widget

  .. cpp:type:: boost::shared_ptr<StackModWidget> Ptr
  .. cpp:function:: virtual bool generate(const codeGen &, const RelocBlock *, CodeBuffer &)
  .. cpp:function:: TrackerElement *tracker(const RelocBlock *t) const
  .. cpp:function:: static Ptr create(InstructionAPI::Instruction insn, Address addr, signed long newDisp, Architecture arch)
  .. cpp:function:: virtual ~StackModWidget()
  .. cpp:function:: virtual std::string format() const
  .. cpp:function:: virtual unsigned size() const
  .. cpp:function:: virtual Address addr() const
  .. cpp:function:: private StackModWidget(InstructionAPI::Instruction insn, Address addr, signed long newDisp, Architecture arch)
  .. cpp:member:: private InstructionAPI::Instruction insn_
  .. cpp:member:: private Address addr_
  .. cpp:member:: private signed long newDisp_
  .. cpp:member:: private Architecture arch_


.. cpp:struct:: StackModPatch : public Patch

  .. cpp:function:: private StackModPatch(InstructionAPI::Instruction a, signed long d, Architecture ar, Address ad)
  .. cpp:function:: private virtual bool apply(codeGen &gen, CodeBuffer *buffer)
  .. cpp:function:: private virtual unsigned estimate(codeGen &templ)
  .. cpp:function:: private virtual ~StackModPatch()
  .. cpp:member:: private InstructionAPI::Instruction orig_insn
  .. cpp:member:: private signed long newDisp
  .. cpp:member:: private Architecture arch
  .. cpp:member:: private Address addr
