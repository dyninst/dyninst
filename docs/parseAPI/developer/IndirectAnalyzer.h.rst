.. _`sec:IndirectAnalyzer.h`:

IndirectAnalyzer.h
##################

.. cpp:class:: IndirectControlFlowAnalyzer

  .. cpp:member:: private ParseAPI::Function *func

    The function and block that contain the indirect jump

  .. cpp:member:: private ParseAPI::Block *block
  .. cpp:member:: private set<ParseAPI::Block *> reachable
  .. cpp:member:: private ThunkData thunks
  .. cpp:function:: private void GetAllReachableBlock()

    Find all blocks that reach the block containing the indirect jump

  .. cpp:function:: private void FindAllThunks()

  .. cpp:function:: private void ReadTable(AST::Ptr, AbsRegion, StridedInterval &,\
                                           int, bool, std::set<Address> &,\
                                           std::vector<std::pair<Address, Dyninst::ParseAPI::EdgeTypeEnum>> &,\
                                           Address &, Address &, int &, std::map<Address, Address> &)

  .. cpp:function:: private int GetMemoryReadSize(Assignment::Ptr loc)
  .. cpp:function:: private bool IsZeroExtend(Assignment::Ptr loc)
  .. cpp:function:: private bool FindJunkInstruction(Address)

  .. cpp:function:: bool NewJumpTableAnalysis(std::vector<std::pair< Address,\
                                              Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges)

    Insert edges found into the edges vector, and return true if the vector is non-empty

  .. cpp:function:: IndirectControlFlowAnalyzer(ParseAPI::Function *f, ParseAPI::Block *b)
