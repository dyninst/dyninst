.. _`sec:IndirectAnalyzer.h`:

IndirectAnalyzer.h
##################

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:class:: IndirectControlFlowAnalyzer

  .. cpp:function:: bool NewJumpTableAnalysis(std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges)
  .. cpp:function:: IndirectControlFlowAnalyzer(ParseAPI::Function *f, ParseAPI::Block *b)
