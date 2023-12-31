Expression.h
============

.. cpp:namespace:: Dyninst::InstructionAPI

Expression Class
----------------

An ``Expression`` is an AST representation of how the value of an
operand is computed.

The ``Expression`` class extends the ``InstructionAST`` class by adding
the concept of evaluation to the nodes of an ``InstructionAST``.
Evaluation attempts to determine the ``Result`` of the computation that
the AST being evaluated represents. It will fill in results of as many
of the nodes in the tree as possible, and if full evaluation is
possible, it will return the result of the computation performed by the
tree.

Permissible leaf nodes of an ``Expression`` tree are RegisterAST and
Immediate objects. Permissible internal nodes are ``BinaryFunction`` and
Dereference objects. An ``Expression`` may represent an immediate value,
the contents of a register, or the contents of memory at a given
address, interpreted as a particular type.

The ``Result``\ s in an ``Expression`` tree contain a type and a value.
Their values may be an undefined value or an instance of their
associated type. When two ``Result``\ s are combined using a
``BinaryFunction``, the ``BinaryFunction`` specifies the output type.
Sign extension, type promotion, truncation, and all other necessary
conversions are handled automatically based on the input types and the
output type. If both of the ``Result``\ s that are combined have defined
values, the combination will also have a defined value; otherwise, the
combination’s value will be undefined. For more information, see
Section `3.7 <#sec:result>`__, Section `3.10 <#sec:binaryFunction>`__,
and Section `3.11 <#sec:dereference>`__.

A user may specify the result of evaluating a given ``Expression``. This
mechanism is designed to allow the user to provide a Dereference or
RegisterAST with information about the state of memory or registers. It
may additionally be used to change the value of an Immediate or to
specify the result of a ``BinaryFunction``. This mechanism may be used
to support other advanced analyses.

In order to make it more convenient to specify the results of particular
subexpressions, the ``bind`` method is provided. ``bind`` allows the
user to specify that a given subexpression has a particular value
everywhere that it appears in an expression. For example, if the state
of certain registers is known at the time an instruction is executed, a
user can ``bind`` those registers to their known values throughout an
``Expression``.

The evaluation mechanism, as mentioned above, will evaluate as many
sub-expressions of an expression as possible. Any operand that is more
complicated than a single immediate value, however, will depend on
register or memory values. The ``Result``\ s of evaluating each
subexpression are cached automatically using the ``setValue`` mechanism.
The ``Expression`` then attempts to determine its ``Result`` based on
the ``Result``\ s of its children. If this ``Result`` can be determined
(most likely because register contents have been filled in via
``setValue`` or ``bind``), it will be returned from ``eval``; if it can
not be determined, a ``Result`` with an undefined value will be
returned. See Figure 6 for an illustration of this concept; the operand
represented is ``[ EBX + 4 \ast EAX ]``. The contents of ``EBX`` and
``EAX`` have been determined through some outside mechanism, and have
been defined with ``setValue``. The ``eval`` mechanism proceeds to
determine the address being read by the ``Dereference``, since this
information can be determined given the contents of the registers. This
address is available from the Dereference through its child in the tree,
even though calling ``eval`` on the Dereference returns a ``Result``
with an undefined value.

.. code-block:: cpp

    typedef boost::shared_ptr<Expression> Ptr

A type definition for a reference-counted pointer to an ``Expression``.

.. code-block:: cpp

    const Result & eval() const

If the ``Expression`` can be evaluated, returns a ``Result`` containing
its value. Otherwise returns an undefined ``Result``.

.. code-block:: cpp

    const setValue(const Result & knownValue)

Sets the result of ``eval`` for this ``Expression`` to ``knownValue``.

.. code-block:: cpp

    void clearValue()

``clearValue`` sets the contents of this ``Expression`` to undefined.
The next time ``eval`` is called, it will recalculate the value of the
``Expression``.

.. code-block:: cpp

    int size() const

``size`` returns the size of this ``Expression``\ ’s ``Result``, in
bytes.

.. code-block:: cpp
    
    bool bind(Expression * expr, const Result & value)

``bind`` searches for all instances of the Expression ``expr`` within
this Expression, and sets the result of ``eval`` for those
subexpressions to ``value``. ``bind`` returns ``true`` if at least one
instance of ``expr`` was found in this Expression.

``bind`` does not operate on subexpressions that happen to evaluate to
the same value. For example, if a dereference of ``0xDEADBEEF`` is bound
to 0, and a register is bound to ``0xDEADBEEF``, a deference of that
register is not bound to 0.

virtual void apply(Visitor \*)

``apply`` applies a ``Visitor`` to this ``Expression``. Visitors perform
postfix-order traversal of the ASTs represented by an ``Expression``,
with user-defined actions performed at each node of the tree. We present
a thorough discussion with examples in Section `3.6 <#sec:visitor>`__.

virtual void getChildren(std::vector<Expression::Ptr> & children) const

``getChildren`` may be called on an ``Expression`` taking a vector of
``ExpressionPtr``\ s, rather than ``InstructionAST``\ Ptrs. All children
which are ``Expression``\ s will be appended to ``children``.