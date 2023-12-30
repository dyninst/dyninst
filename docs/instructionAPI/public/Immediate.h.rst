Immediate.h
===========

.. cpp:namespace:: Dyninst::InstructionAPI

Immediate Class
---------------

The Immediate class represents an immediate value in an operand.

Since an Immediate represents a constant value, the ``setValue`` and
``clearValue`` interface are disabled on Immediate objects. If an
immediate value is being modified, a new Immediate object should be
created to represent the new value.

.. code-block:: cpp

    virtual bool isUsed(InstructionAST::Ptr findMe) const
    void getChildren(vector<InstructionAST::Ptr> &) const

By definition, an ``Immediate`` has no children.

.. code-block:: cpp

    void getUses(set<InstructionAST::Ptr> &)

By definition, an ``Immediate`` uses no registers.

.. code-block:: cpp
 
    bool isUsed(InstructionAPI::Ptr findMe) const

``isUsed``, when called on an Immediate, will return true if ``findMe``
represents an Immediate with the same value. While this convention may
seem arbitrary, it allows ``isUsed`` to follow a natural rule: an
``InstructionAST`` is used by another ``InstructionAST`` if and only if
the first ``InstructionAST`` is a subtree of the second one.