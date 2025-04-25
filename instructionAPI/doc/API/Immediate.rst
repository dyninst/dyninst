.. _sec:immediate:

Immediate Class
---------------

The Immediate class represents an immediate value in an operand.

Since an Immediate represents a constant value, the and interface are
disabled on Immediate objects. If an immediate value is being modified,
a new Immediate object should be created to represent the new value.

.. code::
  virtual bool isUsed(Expression::Ptr findMe) const

.. code::
  void getChildren(vector<Expression::Ptr> &) const

By definition, an ``Immediate`` has no children.

.. code::
  void getUses(set<Expression::Ptr> &)

By definition, an ``Immediate`` uses no registers.

.. code::
  bool isUsed(Expression::Ptr findMe) const

``isUsed``, when called on an Immediate, will return true if \code{findMe}
represents an Immediate with the same value. While this convention may seem
arbitrary, it allows ``isUsed`` to follow a natural rule: an ``Expression``
is used by another ``Expression`` if and only if the first
``Expression`` is a subtree of the second one.

