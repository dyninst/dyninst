.. _`sec:RelocGraph.h`:

RelocGraph.h
############

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: RelocGraph

  .. cpp:type:: std::map<func_instance *, RelocBlock *> SubMap
  .. cpp:type:: std::map<Address, SubMap> InstanceMap
  .. cpp:type:: std::map<std::pair<block_instance *, func_instance *>, RelocBlock *> Map
  .. cpp:type:: std::vector<RelocEdge *> Edges
  .. cpp:function:: RelocGraph()
  .. cpp:function:: ~RelocGraph()
  .. cpp:function:: RelocBlock *begin()
  .. cpp:function:: RelocBlock *end()
  .. cpp:function:: void addRelocBlock(RelocBlock *)

    For initial construction

  .. cpp:function:: void addRelocBlockBefore(RelocBlock *cur, RelocBlock *add)
  .. cpp:function:: void addRelocBlockAfter(RelocBlock *cur, RelocBlock *add)
  .. cpp:function:: RelocEdge *makeEdge(TargetInt *, TargetInt *, edge_instance* e, ParseAPI::EdgeTypeEnum et)
  .. cpp:function:: bool removeEdge(RelocEdge *)
  .. cpp:function:: void removeSource(RelocEdge *)
  .. cpp:function:: void removeTarget(RelocEdge *)
  .. cpp:member:: RelocBlock *head
  .. cpp:member:: RelocBlock *tail
  .. cpp:member:: unsigned size
  .. cpp:member:: Edges edges

    If we keep a master list here and just keep pointers everywhere else, memory management gets a hell of a  lot easier...

  .. cpp:member:: Map springboards
  .. cpp:member:: InstanceMap reloc
  .. cpp:function:: RelocBlock *find(block_instance *, func_instance *) const
  .. cpp:function:: bool setSpringboard(block_instance *from, func_instance *func, RelocBlock *to)
  .. cpp:function:: RelocBlock *findSpringboard(block_instance *from, func_instance *to) const
  .. cpp:function:: void link(RelocBlock *s, RelocBlock *t)

    Should this go here? Well, it's a transformation on RelocBlocks...

  .. cpp:function:: bool interpose(RelocEdge *e, RelocBlock *n)
  .. cpp:function:: bool changeTarget(RelocEdge *e, TargetInt *n)
  .. cpp:function:: bool changeSource(RelocEdge *e, TargetInt *n)
  .. cpp:function:: bool changeType(RelocEdge *e, ParseAPI::EdgeTypeEnum t)
  .. cpp:function:: template <class Predicate> void applyPredicate(Predicate &p, RelocEdges *e, RelocEdges &results)
  .. cpp:function:: RelocBlock *split(RelocBlock *c, WidgetList::iterator where)

    bool addTarget(RelocBlock s, RelocBlock t, ParseAPI::EdgeTypeEnum type)bool remove(RelocEdge e)

  .. cpp:function:: template <class Predicate> bool interpose(Predicate &p, RelocEdges *e, RelocBlock *t)
  .. cpp:function:: template <class Predicate, class Dest> bool changeTargets(Predicate &p, RelocEdges *e, Dest n)
  .. cpp:function:: template <class Predicate, class Source> bool changeSources(Predicate &p, RelocEdges *e, Source n)
  .. cpp:function:: template <class Predicate> bool removeEdge(Predicate &p, RelocEdges *e)
  .. cpp:function:: template <class Predicate> bool changeType(Predicate &p, RelocEdges *e, ParseAPI::EdgeTypeEnum t)


.. cpp:struct:: RelocGraph::InterproceduralPredicate

  .. cpp:function:: bool operator()(RelocEdge *e)

.. cpp:struct:: RelocGraph::IntraproceduralPredicate

  .. cpp:function:: bool operator()(RelocEdge *e)


.. cpp:struct:: Predicates


.. cpp:struct:: Predicates::Interprocedural

  .. cpp:function:: bool operator()(RelocEdge *e)


.. cpp:struct:: Predicates::Intraprocedural

  .. cpp:function:: bool operator()(RelocEdge *e)


.. cpp:struct:: Predicates::Fallthrough

  .. cpp:function:: bool operator()(RelocEdge *e)


.. cpp:struct:: Predicates::CallFallthrough

  .. cpp:function:: bool operator()(RelocEdge *e)


.. cpp:struct:: Predicates::NonCall

  .. cpp:function:: bool operator()(RelocEdge *e)


.. cpp:struct:: Predicates::Call

  .. cpp:function:: bool operator()(RelocEdge *e)


.. cpp:struct:: Predicates::Edge

  .. cpp:function:: Edge(edge_instance *e)
  .. cpp:function:: bool operator()(RelocEdge *e)
  .. cpp:member:: edge_instance *e_


.. cpp:struct:: Predicates::Type

  .. cpp:function:: Type(ParseAPI::EdgeTypeEnum t)
  .. cpp:function:: bool operator()(RelocEdge *e)
  .. cpp:member:: ParseAPI::EdgeTypeEnum t_

