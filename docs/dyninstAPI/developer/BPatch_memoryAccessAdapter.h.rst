.. _`sec:BPatch_memoryAccessAdapter.h`:

BPatch_memoryAccessAdapter.h
############################


.. cpp:class:: BPatch_memoryAccessAdapter : public Dyninst::InstructionAPI::Visitor

  .. cpp:function:: BPatch_memoryAccessAdapter()
  .. cpp:function:: virtual ~BPatch_memoryAccessAdapter()
  .. cpp:function:: BPatch_memoryAccess* convert(Dyninst::InstructionAPI::Instruction insn, Dyninst::Address current, bool is64)
  .. cpp:function:: virtual void visit(Dyninst::InstructionAPI::BinaryFunction* b)
  .. cpp:function:: virtual void visit(Dyninst::InstructionAPI::Dereference* d)
  .. cpp:function:: virtual void visit(Dyninst::InstructionAPI::RegisterAST* r)
  .. cpp:function:: virtual void visit(Dyninst::InstructionAPI::Immediate* i)
  .. cpp:member:: private unsigned int bytes
  .. cpp:member:: private long imm
  .. cpp:member:: private int ra
  .. cpp:member:: private int rb
  .. cpp:member:: private int sc
  .. cpp:member:: private bool setImm
