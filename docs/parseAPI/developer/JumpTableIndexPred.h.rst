.. _`sec:JumpTableIndexPred.h`:

JumpTableIndexPred.h
####################

.. cpp:class:: JumpTableIndexPred : public Slicer::Predicates

  .. cpp:member:: bool unknownInstruction
  .. cpp:member:: bool findBound
  .. cpp:member:: StridedInterval bound
  .. cpp:member:: std::set<Assignment::Ptr> currentAssigns

  .. cpp:function:: virtual bool addNodeCallback(AssignmentPtr ap, std::set<ParseAPI::Edge*> &visitedEdges)
  .. cpp:function:: virtual bool modifyCurrentFrame(Slicer::SliceFrame &frame, Graph::Ptr g, Slicer*)
  .. cpp:function:: GraphPtr BuildAnalysisGraph(std::set<ParseAPI::Edge*> &visitedEdges)
  .. cpp:function:: bool IsIndexBounded(GraphPtr slice, BoundFactsCalculator &bfc, StridedInterval &target)
  .. cpp:function:: JumpTableIndexPred(ParseAPI::Function *f, ParseAPI::Block *b, AbsRegion i, SymbolicExpression &sym)
  .. cpp:function:: virtual bool ignoreEdge(ParseAPI::Edge *e)
  .. cpp:function:: virtual int slicingSizeLimitFactor()

.. cpp:class:: TypedSliceEdge: public Dyninst::Edge

  .. cpp:type:: boost::shared_ptr<TypedSliceEdge> Ptr

  .. cpp:function:: static TypedSliceEdge::Ptr create(SliceNode::Ptr source, SliceNode::Ptr target, ParseAPI::EdgeTypeEnum t)
  .. cpp:function:: ParseAPI::EdgeTypeEnum type()


.. code:: c

  // Assume the table contain less than this many entries.
  #define MAX_TABLE_ENTRY 1000000