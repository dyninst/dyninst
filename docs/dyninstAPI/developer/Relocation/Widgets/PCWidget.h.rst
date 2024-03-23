.. _`sec:PCWidget.h`:

PCWidget.h
##########

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: PCWidget : public Widget

  .. cpp:type:: boost::shared_ptr<PCWidget> Ptr
  .. cpp:function:: static Ptr create(InstructionAPI::Instruction insn, Address addr, Absloc a, Address thunk = 0)
  .. cpp:function:: virtual bool generate(const codeGen &, const RelocBlock *, CodeBuffer &)
  .. cpp:function:: TrackerElement *tracker(const RelocBlock *t) const
  .. cpp:function:: virtual ~PCWidget()
  .. cpp:function:: virtual std::string format() const
  .. cpp:function:: virtual unsigned size() const
  .. cpp:function:: virtual Address addr() const
  .. cpp:function:: virtual InstructionAPI::Instruction insn() const
  .. cpp:function:: private PCWidget(InstructionAPI::Instruction insn, Address addr, Absloc &a, Address thunkAddr = 0)
  .. cpp:function:: private bool PCtoReturnAddr(const codeGen &templ, const RelocBlock *, CodeBuffer &)
  .. cpp:function:: private bool PCtoReg(const codeGen &templ, const RelocBlock *, CodeBuffer &)
  .. cpp:member:: private InstructionAPI::Instruction insn_
  .. cpp:member:: private Address addr_
  .. cpp:member:: private Absloc a_
  .. cpp:member:: private Address thunkAddr_


.. cpp:struct:: IPPatch : public Patch

  .. cpp:function:: private IPPatch(Type a, Address b, InstructionAPI::Instruction c, block_instance *d, func_instance *e)
  .. cpp:function:: private IPPatch(Type a, Address b, Register c, Address d, InstructionAPI::Instruction e, block_instance *f, func_instance *g)
  .. cpp:function:: private virtual bool apply(codeGen &gen, CodeBuffer *buf)
  .. cpp:function:: private virtual unsigned estimate(codeGen &templ)
  .. cpp:function:: private virtual ~IPPatch()
  .. cpp:member:: private Type type
  .. cpp:member:: private Address addr
  .. cpp:member:: private Register reg
  .. cpp:member:: private Address thunk
  .. cpp:member:: private InstructionAPI::Instruction insn

    Necessary for live registers

  .. cpp:member:: private block_instance *block
  .. cpp:member:: private func_instance *func


.. cpp:enum:: IPPatch::Type

  .. cpp:enumerator:: Push
  .. cpp:enumerator:: Reg
