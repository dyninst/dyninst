.. _`sec:RelocEdge.h`:

RelocEdge.h
###########

.. cpp:namespace:: Dyninst::Relocation

.. cpp:struct:: RelocEdge

  .. cpp:function:: RelocEdge(TargetInt *s, TargetInt *t, edge_instance* e, ParseAPI::EdgeTypeEnum et)
  .. cpp:function:: ~RelocEdge()
  .. cpp:member:: TargetInt *src
  .. cpp:member:: TargetInt *trg
  .. cpp:member:: edge_instance *edge
  .. cpp:member:: ParseAPI::EdgeTypeEnum type


.. cpp:struct:: RelocEdges

  .. cpp:function:: RelocEdges()
  .. cpp:type:: std::list<RelocEdge *>::iterator iterator
  .. cpp:type:: std::list<RelocEdge *>::const_iterator const_iterator
  .. cpp:function:: iterator begin()
  .. cpp:function:: iterator end()
  .. cpp:function:: const_iterator begin() const
  .. cpp:function:: const_iterator end() const
  .. cpp:function:: void push_back(RelocEdge *e)
  .. cpp:function:: void insert(RelocEdge *e)
  .. cpp:function:: RelocEdge *find(ParseAPI::EdgeTypeEnum e)
  .. cpp:function:: void erase(RelocEdge *)
  .. cpp:function:: bool contains(ParseAPI::EdgeTypeEnum e)
  .. cpp:member:: std::list<RelocEdge *> edges
