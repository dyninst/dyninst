.. _`sec:RelDataWidget.h`:

RelDataWidget.h
###############

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: RelDataWidget : public Widget

  Represents generation for a PC-relative memory loadstore

  Read vs. write doesn't matter now but might in the future.

  .. cpp:type:: boost::shared_ptr<RelDataWidget> Ptr
  .. cpp:function:: virtual bool generate(const codeGen &, const RelocBlock *, CodeBuffer &)
  .. cpp:function:: TrackerElement *tracker(const RelocBlock *t) const
  .. cpp:function:: static Ptr create(InstructionAPI::Instruction insn, Address addr, Address target)
  .. cpp:function:: virtual ~RelDataWidget()
  .. cpp:function:: virtual std::string format() const
  .. cpp:function:: virtual unsigned size() const
  .. cpp:function:: virtual Address addr() const
  .. cpp:function:: private RelDataWidget(InstructionAPI::Instruction insn, Address addr, Address target)
  .. cpp:member:: private InstructionAPI::Instruction insn_
  .. cpp:member:: private Address addr_
  .. cpp:member:: private Address target_


.. cpp:struct:: RelDataPatch : public Patch

  Read vs. write doesn't matter now but might in the future.

  .. cpp:function:: private RelDataPatch(InstructionAPI::Instruction a, Address b, Address o)
  .. cpp:function:: private virtual bool apply(codeGen &gen, CodeBuffer *buffer)
  .. cpp:function:: private virtual unsigned estimate(codeGen &templ)
  .. cpp:function:: private virtual ~RelDataPatch()
  .. cpp:function:: private void setFunc(func_instance *_func)
  .. cpp:function:: private void setBlock(block_instance *_block)
  .. cpp:member:: private InstructionAPI::Instruction orig_insn
  .. cpp:member:: private Address target_addr{}
  .. cpp:member:: private Address orig{}
  .. cpp:member:: private func_instance *func{}
  .. cpp:member:: private block_instance *block{}
