BinaryFunction.h
================

.. cpp:namespace:: Dyninst::instructionAPI

BinaryFunction Class
--------------------

A ``BinaryFunction`` object represents a function that can combine two
``Expression``\ s and produce another ``ValueComputation``.

For the purposes of representing a single operand of an instruction, the
``BinaryFunction``\ s of interest are addition and multiplication of
integer values; this allows an ``Expression`` to represent all
addressing modes on the architectures currently supported by the
Instruction API.

.. code-block:: cpp

    BinaryFunction(Expression::Ptr arg1, Expression::Ptr arg2, Result_Type result_type, funcT:Ptr func)

The constructor for a ``BinaryFunction`` may take a reference-counted
pointer or a plain C++ pointer to each of the child ``Expression``\ s
that represent its arguments. Since the reference-counted implementation
requires explicit construction, we provide overloads for all four
combinations of plain and reference-counted pointers. Note that
regardless of which constructor is used, the pointers ``arg1`` and
``arg2`` become owned by the ``BinaryFunction`` being constructed, and
should not be deleted. They will be cleaned up when the
``BinaryFunction`` object is destroyed.

The ``func`` parameter is a binary functor on two ``Result``\ s. It
should be derived from ``funcT``. ``addResult`` and ``multResult``,
which respectively add and multiply two ``Result``\ s, are provided as
part of the InstructionAPI, as they are necessary for representing
address calculations. Other ``funcTs`` may be implemented by the user if
desired. ``funcTs`` have names associated with them for output and
debugging purposes. The addition and multiplication functors provided
with the Instruction API are named "+" and "*", respectively.

.. code-block:: cpp

    const Result & eval () const

The ``BinaryFunction`` version of ``eval`` allows the ``eval`` mechanism
to handle complex addressing modes. Like all of the ``ValueComputation``
implementations, a ``BinaryFunction``\ ’s ``eval`` will return the
result of evaluating the expression it represents if possible, or an
empty ``Result`` otherwise. A ``BinaryFunction`` may have arguments that
can be evaluated, or arguments that cannot. Additionally, it may have a
real function pointer, or it may have a null function pointer. If the
arguments can be evaluated and the function pointer is real, a result
other than an empty ``Result`` is guaranteed to be returned. This result
is cached after its initial calculation; the caching mechanism also
allows outside information to override the results of the
``BinaryFunction``\ ’s internal computation. If the cached result
exists, it is guaranteed to be returned even if the arguments or the
function are not evaluable.

.. code-block:: cpp

    void getChildren (vector< InstructionAST::Ptr > & children) const

The children of a ``BinaryFunction`` are its two arguments. Appends the
children of this BinaryFunction to ``children``.

.. code-block:: cpp

    void getUses (set< InstructionAST::Ptr > & uses)

The use set of a ``BinaryFunction`` is the union of the use sets of its
children. Appends the use set of this ``BinaryFunction`` to ``uses``.

.. code-block:: cpp

    bool isUsed (InstructionAST::Ptr findMe) const

``isUsed`` returns ``true`` if ``findMe`` is an argument of this
``BinaryFunction``, or if it is in the use set of either argument.