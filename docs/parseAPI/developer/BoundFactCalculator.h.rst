.. _`sec:BoundFactCalculator.h`:

BoundFactCalculator.h
#####################

.. cpp:namespace:: Dyninst::ParseAPI

.. code:: c

  // To avoid the bound fact calculation from deadlock
  #define IN_QUEUE_LIMIT 10

.. cpp:class:: BoundFactsCalculator

  .. cpp:function:: bool CalculateBoundedFacts()
  .. cpp:function:: BoundFactsCalculator(ParseAPI::Function *f, GraphPtr s, bool first, \
                    bool oneByteRead, SymbolicExpression &sym)
  .. cpp:function:: BoundFact *GetBoundFactIn(Node::Ptr node)
  .. cpp:function:: BoundFact *GetBoundFactOut(Node::Ptr node)
