.. _`sec:block.h`:

block.h
#######

This is somewhat mangled, but allows Dyninst to access the iteration predicates of Dyninst without having
to go back and template that code. Just wrap a ParseAPI predicate in a EdgePredicateAdapter and poof you're
using edge_instances instead of ParseAPI edges...

.. cpp:class:: edge_instance : public Dyninst::PatchAPI::PatchEdge

  .. cpp:function:: block_instance *src() const
  .. cpp:function:: block_instance *trg() const
  .. cpp:function:: AddressSpace *proc()
  .. cpp:function:: edge_instance(ParseAPI::Edge *edge, block_instance *src, block_instance *trg)
  .. cpp:function:: edge_instance(const edge_instance *parent, mapped_object *child)
  .. cpp:function:: ~edge_instance()

.. cpp:class:: EdgePredicateAdapter : public ParseAPI::iterator_predicate<edge_instance *, edge_instance *>

  .. cpp:function:: EdgePredicateAdapter()
  .. cpp:function:: EdgePredicateAdapter(ParseAPI::EdgePredicate *intPred)
  .. cpp:function:: virtual ~EdgePredicateAdapter()
  .. cpp:function:: virtual bool pred_impl(edge_instance *const e) const
  .. cpp:member:: private ParseAPI::EdgePredicate *int_

.. cpp:class:: block_instance : public Dyninst::PatchAPI::PatchBlock

  .. cpp:function:: block_instance(ParseAPI::Block *ib, mapped_object *obj)

    We create edges lazily

  .. cpp:function:: block_instance(const block_instance *parent, mapped_object *child)

    We also need to copy edges. Thing is, those blocks may not exist yet. So we wait, and do edges
    after all blocks have been created

  .. cpp:function:: ~block_instance()

    Edges are deleted at the mapped_object layer

  .. cpp:function:: mapped_object *obj() const
  .. cpp:function:: AddressSpace *addrSpace() const
  .. cpp:function:: AddressSpace *proc() const
  .. cpp:function:: template <class OutputIterator> void getFuncs(OutputIterator result)
  .. cpp:function:: void triggerModified()

    KEVINTODO: implement this: remove block from Relocation info caching.

  .. cpp:function:: void setNotAbruptEnd()
  .. cpp:function:: parse_block *llb() const
  .. cpp:function:: void *getPtrToInstruction(Address addr) const
  .. cpp:function:: edge_instance *getTarget()
  .. cpp:function:: edge_instance *getFallthrough()
  .. cpp:function:: block_instance *getFallthroughBlock()

    NULL if not conclusive

  .. cpp:function:: func_instance *callee()
  .. cpp:function:: std::string calleeName()
  .. cpp:member:: bool _ignorePowerPreamble
  .. cpp:function:: int id() const
  .. cpp:function:: func_instance *entryOfFunc() const

  ......

  .. rubric::
    These are convinence wrappers for really expensive lookups, and thus should be avoided.

  .. cpp:function:: bool isFuncExit() const
  .. cpp:function:: Address GetBlockStartingAddress()
  .. cpp:function:: virtual void markModified()
  .. cpp:function:: private void updateCallTarget(func_instance *func)

    Update a sink-typed call edge to have an inter-module target preserving original behavior
    on sink edges only.

  .. cpp:function:: private func_instance *findFunction(ParseAPI::Function *)
  .. cpp:function:: private func_instance *callee(std::string const &)


.. cpp:struct:: BlockInstanceAddrCompare

  .. cpp:function:: private bool operator()(block_instance *const &b1, block_instance *const &b2) const

.. cpp:type:: std::set<block_instance *, BlockInstanceAddrCompare> AddrOrderedBlockSet

