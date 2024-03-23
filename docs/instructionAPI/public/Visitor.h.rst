.. _`sec:Visitor.h`:

Visitor.h
#########

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:class:: Visitor

  **Performs postfix-order traversal of an AST**

  .. cpp:function:: virtual ~Visitor() = default
  .. cpp:function:: Visitor& operator=(const Visitor&) = default
  .. cpp:function:: virtual void visit(BinaryFunction* b) = 0
  .. cpp:function:: virtual void visit(Immediate* i) = 0
  .. cpp:function:: virtual void visit(RegisterAST* r) = 0
  .. cpp:function:: virtual void visit(Dereference* d) = 0


.. _`sec:visitor-notes`:

Notes
=====

This class specifies the interface users should extend to create their own
visitors to perform specific tasks.

Any state the visitor needs to preserve between nodes must be kept within the user's class.
Visitors are invoked by calling :cpp:func:`Expression::apply`. The individual ``visit`` methods
should not be invoked by user code.
