.. _`sec:Immediate.h`:

Immediate.h
###########

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:class:: Immediate : public Expression

  **An immediate value in an operand**

  Since an Immediate represents a constant value, :cpp:func:`Expression::setValue` and
  :cpp:func:`Expression::clearValue` are not implemented. If an
  immediate value is being modified, a new object should be
  created to represent the new value.

  .. cpp:function:: static Immediate::Ptr makeImmediate(const Result& val)

    A convenience function to construct an immediate.

  .. cpp:function:: Immediate(const Result& val)

    Constructs an immediate with the value ``val``.

  .. cpp:function:: virtual void getChildren(vector<InstructionAST::Ptr>&) const

      Does nothing because an immediate has no children.

  .. cpp:function:: virtual void getChildren(vector<Expression::Ptr>&) const

      Does nothing because an immediate has no children.

  .. cpp:function:: virtual void getUses(set<InstructionAST::Ptr>&)

    Does nothing because an immediate uses no registers or memory.

  .. cpp:function:: virtual bool isUsed(InstructionAST::Ptr i) const

    Checks if ``i`` represents an Immediate with the same value as this object.

    While this convention may seem arbitrary, it allows ``isUsed`` to follow a natural
    rule: an ``InstructionAST`` is used by another ``InstructionAST`` if and only if
    the first is a subtree of the second one.

  .. cpp:function:: virtual std::string format(formatStyle) const

    Returns a string representation of this expression using the style ``formatStyle``.

  .. cpp:function:: virtual std::string format(Architecture arch, formatStyle) const

    Returns a string representation of this expression using the :cpp:class:`ArchSpecificFormatter`
    associated with ``arch``. ``formatStyle`` is ignored.

  .. cpp:function:: virtual void apply(Visitor* v)

    Applies ``v`` in a postfix-order traversal of contained expressions (as :cpp:class:`AST`\ s)
    with user-defined actions performed at each node of the tree.
