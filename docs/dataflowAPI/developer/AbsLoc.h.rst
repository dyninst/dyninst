.. _`sec-dev:absloc.h`:

Absloc.h
########

.. cpp:namespace:: Dyninst::dev


.. cpp:class:: AbsRegion

  .. cpp:member:: private Absloc::Type type_

    Type is for "we're on the stack but we don't know where". Effectively, it's a wildcard.

  .. cpp:member:: private Absloc absloc_

    For specific knowledge.

  .. cpp:member:: private AST::Ptr generator_

    And the AST that gave rise to this Absloc. We use this as a generating function (if present and not overridden)

  .. cpp:member:: private size_t size_

    Size in *bits*.

  .. cpp:function:: void addInput(const AbsRegion &reg)

    Internally used method add a dependence on a new abstract region.

    If this is a new region we'll add it to the dependence list. Otherwise we'll join the
    provided input set to the known inputs.

  .. cpp:function:: void addInputs(const std::vector<AbsRegion> &regions)

     Same as :cpp:func:`addInput`.

  .. cpp:member:: private InstructionAPI::Instruction insn_
  .. cpp:member:: private Address addr_
  .. cpp:member:: private ParseAPI::Function *func_
  .. cpp:member:: private ParseAPI::Block *block_
  .. cpp:member:: private std::vector<AbsRegion> inputs_
  .. cpp:member:: private AbsRegion out_


.. cpp:struct:: AssignmentPtrValueComp

  Compare assignments by value. note this is a fast comparison--it checks output and address only.

  .. cpp:function:: bool operator()(const Assignment::Ptr& a, const Assignment::Ptr& b) const
