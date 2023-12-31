.. _`sec:Expression.h`:

Expression.h
############

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:class:: Expression : public InstructionAST

  **AST representation of how the value of an operand is computed**

  .. cpp:type:: boost::shared_ptr<Expression> Ptr

    A reference-counted pointer to an expression.

  .. cpp:member:: protected Result userSetValue

  .. cpp:function:: const Result& eval() const

    Evaluates the expression and returns a :cpp:class:`Result` containing its value.

    Returns an undefined ``Result`` on failure. See :ref:`sec:expression-evaluation` for details.

  .. cpp:function:: void setValue(const Result & knownValue)

    Sets the evaluation result for this expression to ``knownValue``.

  .. cpp:function:: void clearValue()

    Sets the contents of this expression to undefined.

    The next time :cpp:func:`eval` is called, it will recalculate the value.

  .. cpp:function:: int size() const

    Returns the size of this expression’s result in **bytes**.

  .. cpp:function:: bool bind(Expression * expr, const Result & value)

    Searches for all instances of ``expr`` and sets the result for each subexpression to ``value``.

    Returns ``true`` if at least one instance of ``expr`` was found. See :ref:`sec:expression-binding`
    for details.

  .. cpp:function:: virtual void apply(Visitor *v)

    Applies ``v`` in a postfix-order traversal of contained expressions (as :cpp:class:`AST`\ s)
    with user-defined actions performed at each node of the tree.

  .. cpp:function:: virtual void getChildren(std::vector<Expression::Ptr> & children) const

     Appends the children of this expression to ``children``.

  .. cpp:function:: protected virtual bool isFlag() const


.. cpp:class:: DummyExpr : public Expression

  .. cpp:function:: protected virtual bool checkRegID(MachRegister, unsigned int = 0, unsigned int = 0) const
  .. cpp:function:: protected virtual bool isStrictEqual(const InstructionAST& rhs) const

Notes
=====
This class extends ``InstructionAST`` by adding the concept of evaluation to the nodes.
Evaluation attempts to determine the :cpp:class:`Result` of the computation that
the :cpp:class:`AST` being evaluated represents. It will fill in results of as many
of the nodes in the tree as possible, and if full evaluation is
possible, it will return the result of the computation performed by the
tree.

Permissible leaf nodes of an expression tree are :cpp:class:`RegisterAST`,
:cpp:class:`Immediate`, and :cpp:class:`TernaryAST` objects. Permissible internal nodes are :class:`BinaryFunction` and
:cpp:class:`Dereference` objects. An expression may represent an immediate value,
the contents of a register, or the contents of memory at a given
address, interpreted as a particular type.

The :cpp:class:`Result`\ s in an expression tree contain a type and a value.
Their values may be an undefined value or an instance of their
associated type. When two results are combined using a
``BinaryFunction``, it specifies the output type.
Sign extension, type promotion, truncation, and all other necessary
conversions are handled automatically based on the input types and the
output type. If both of the results that are combined have defined
values, the combination will also have a defined value. Otherwise, the
combination’s value will be undefined.

A user may specify the result of evaluating a given expression. This
mechanism is designed to allow the user to provide a ``Dereference`` or
``RegisterAST`` with information about the state of memory or registers. It
may additionally be used to change the value of an Immediate or to
specify the result of a ``BinaryFunction``. This mechanism may be used
to support other advanced analyses.

.. _`sec:expression-binding`:

Binding
^^^^^^^

In order to make it more convenient to specify the results of particular
subexpressions, the :cpp:func:`bind` method is provided. ``bind`` allows the
user to specify that a given subexpression has a particular value
everywhere that it appears in an expression. For example, if the state
of certain registers is known at the time an instruction is executed, a
user can ``bind`` those registers to their known values throughout an
expression.

.. _`sec:expression-evaluation`:

Evaluation
^^^^^^^^^^

The evaluation mechanism, as mentioned above, will evaluate as many
sub-expressions of an expression as possible. Any operand that is more
complicated than a single immediate value, however, will depend on
register or memory values. The :cpp:class:`Result`\ s of evaluating each
subexpression are cached automatically using :cpp:func:`setValue`.
The expression then attempts to determine its result based on
the results of its children. If this result can be determined
(most likely because register contents have been filled in via
``setValue`` or ``bind``), it will be returned from ``eval``. If it can
not be determined, a result with an undefined value will be
returned. It does not operate on subexpressions that happen to evaluate to the same value. For example,
if a dereference of ``0xDEADBEEF`` is bound to 0, and a register is bound to ``0xDEADBEEF``,
a deference of that register is not bound to 0.

See the :ref:`Dereference Notes <sec:dereference-notes>` for a detailed example.
