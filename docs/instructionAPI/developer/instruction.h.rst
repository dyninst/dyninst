.. _`sec-dev:Instruction.h`:

Instruction.h
#############

.. cpp:namespace:: Dyninst::InstructionAPI::dev

.. cpp:class:: Instruction

  .. cpp:function:: private void updateSize(const unsigned int new_size)
  .. cpp:function:: private void decodeOperands() const
  .. cpp:function:: private void addSuccessor(Expression::Ptr e, bool isCall, bool isIndirect, bool isConditional, bool isFallthrough, bool isImplicit = false) const
  .. cpp:function:: private void appendOperand(Expression::Ptr e, bool isRead, bool isWritten, bool isImplicit = false, bool trueP = false, bool falseP = false) const
  .. cpp:function:: private void copyRaw(size_t size, const unsigned char* raw)
  .. cpp:function:: private Expression::Ptr makeReturnExpression() const
  .. cpp:member:: private mutable std::list<Operand> m_Operands
  .. cpp:member:: private mutable Operation m_InsnOp
  .. cpp:member:: private bool m_Valid
  .. cpp:member:: private raw_insn_T m_RawInsn
  .. cpp:member:: private unsigned int m_size
  .. cpp:member:: private Architecture arch_decoded_from
  .. cpp:member:: private mutable std::list<CFT> m_Successors
  .. cpp:member:: private static int numInsnsAllocated
  .. cpp:member:: private ArchSpecificFormatter* formatter

      A non-owning pointer to a singleton object

