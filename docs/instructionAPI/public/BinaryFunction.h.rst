.. _`sec:BinaryFunction.h`:

BinaryFunction.h
################

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:class:: BinaryFunction : public Expression

  **Evaluates two Expressions into a single Result**

  The operations of interest are :cpp:class:`addition <addResult>`,
  :cpp:class:`multiplication <multResult>`, :cpp:class:`left shift <leftShiftResult>`,
  :cpp:class:`right arithmetic shift <rightArithmeticShiftResult>`,
  :cpp:class:`bitwise and <andResult>`, :cpp:class:`bitwise or <orResult>`,
  :cpp:class:`right logical shift <rightLogicalShiftResult>`, and
  :cpp:class:`right rotate <rightRotateResult>`\ . These allow an
  :cpp:class:`Expression` to represent all addressing modes on all supported
  architectures.

  .. cpp:function:: BinaryFunction(Expression::Ptr arg1, Expression::Ptr arg2, Result_Type result_type, \
                                   funcT::Ptr func)

    Constructs an :cpp:class:`AST` evaluator using the behavior of ``func`` that takes
    two parameters ``arg1`` and ``arg2`` (preserving order) and returns ``result_type``.

  .. cpp:function:: const Result& eval() const

    Returns the evaluation of the contained expressions.

    This result is cached after its initial calculation; the caching mechanism also
    allows outside information to override the results of the ``BinaryFunction``\ ’s
    internal computation. If the cached result exists, it is guaranteed to be returned
    even if the arguments or the function cannot be evaluated.

  .. cpp:function:: void getChildren(vector<InstructionAST::Ptr>& children) const

    Appends the two contained expressions to ``children``.

  .. cpp:function:: void getUses(set<InstructionAST::Ptr> & uses)

    Appends the union of the use sets of its children to ``uses``.

  .. cpp:function:: bool isUsed(InstructionAST::Ptr expr) const

    Checks if ``expr`` is an argument of either contained argument,
    or if it is in the use set of either argument.

  .. cpp:function:: protected virtual bool isStrictEqual(const InstructionAST& rhs) const

.. cpp:class:: BinaryFunction::funcT

  .. cpp:type:: boost::shared_ptr<funcT> Ptr

  .. cpp:function:: funcT(std::string name)
  .. cpp:function:: virtual Result operator()(const Result& arg1, const Result& arg2) const = 0
  .. cpp:function:: std::string format() const

.. cpp:class:: BinaryFunction::addResult : public funcT
.. cpp:class:: BinaryFunction::multResult : public funcT
.. cpp:class:: BinaryFunction::leftShiftResult : public funcT
.. cpp:class:: BinaryFunction::rightArithmeticShiftResult : public funcT
.. cpp:class:: BinaryFunction::andResult : public funcT
.. cpp:class:: BinaryFunction::orResult : public funcT
.. cpp:class:: BinaryFunction::rightLogicalShiftResult : public funcT
.. cpp:class:: BinaryFunction::rightRotateResult : public funcT
