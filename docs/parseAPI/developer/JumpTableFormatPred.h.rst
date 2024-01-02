.. _`sec:JumpTableFormatPred.h`:

JumpTableFormatPred.h
#####################

.. cpp:namespace:: Dyninst::parseAPI

.. cpp:class:: JumpTableFormatPred : public Slicer::Predicates

  .. cpp:function:: JumpTableFormatPred(ParseAPI::Function *f, ParseAPI::Block *b, ReachFact &r, ThunkData &t, SymbolicExpression &sym)

  .. cpp:member:: ParseAPI::Function* func
  .. cpp:member:: ParseAPI::Block* block
  .. cpp:member:: ReachFact& rf
  .. cpp:member:: ThunkData& thunks
  .. cpp:member:: SymbolicExpression& se
  .. cpp:member:: bool jumpTableFormat
  .. cpp:member:: bool findIndex
  .. cpp:member:: bool findTableBase
  .. cpp:member:: AbsRegion index
  .. cpp:member:: Assignment::Ptr indexLoc
  .. cpp:member:: bool firstMemoryRead
  .. cpp:member:: Assignment::Ptr memLoc
  .. cpp:member:: AST::Ptr jumpTargetExpr
  .. cpp:member:: set<Address> constAddr
  .. cpp:member:: dyn_hash_map<Assignment::Ptr, std::pair<AST::Ptr, AST::Ptr>, Assignment::AssignmentPtrHasher> aliases
  .. cpp:member:: Address toc_address

      On ppc 64, r2 is reserved for storing the address of the global offset table

  .. cpp:function:: virtual bool modifyCurrentFrame(Slicer::SliceFrame &frame, Graph::Ptr g, Slicer*)
  .. cpp:function:: std::string format()
  .. cpp:function:: bool isJumpTableFormat()
  .. cpp:function:: bool findRead(Graph::Ptr g, SliceNode::Ptr &)
  .. cpp:function:: bool adjustSliceFrame(Slicer::SliceFrame &frame, SliceNode::Ptr, Slicer*)
  .. cpp:function:: bool isTOCRead(Slicer::SliceFrame &frame, SliceNode::Ptr)
  .. cpp:function:: void FindTOC()
  .. cpp:function:: virtual bool ignoreEdge(ParseAPI::Edge *e)
  .. cpp:function:: virtual int slicingSizeLimitFactor()
