.. _`sec-dev:BPatch_basicBlockLoop.h`:

BPatch_basicBlockLoop.h
#######################

.. cpp:namespace:: dev

.. cpp:class:: BPatch_basicBlockLoop
   
  .. cpp:member:: private std::set<BPatch_edge*> backEdges
  .. cpp:member:: private std::set<BPatch_basicBlock*> entries
  .. cpp:member:: private BPatch_flowGraph *flowGraph

    the flow graph this loop is part of

  .. cpp:member:: private std::set<BPatch_basicBlockLoop*> containedLoops

    set of loops that are contained (nested) in this loop.

  .. cpp:member:: private std::set<BPatch_basicBlock*> basicBlocks

    the basic blocks in the loop

  .. cpp:function:: private void addBackEdges(std::vector<BPatch_edge*> &edges)

    this func is only invoked by BPatch_flowGraph::createLoops

  .. cpp:function:: std::set<BPatch_variableExpr*>* BPatch_basicBlockLoop::getLoopIterators()

      we did not implement this method yet. It needs some deeper
      analysis and some sort of uniform dataflow framework. It is a method
      that returns the iterator of the loop. To find it we have to do
      invariant code analysis which needs reaching definition information
      live variable analysis etc. Since these algorithms are not
      machine independent and needs more inner level machine dependent
      functions and we do not need at this moment for our project we did not
      implement the function. It returns NULL for now.

  .. cpp:function:: private BPatch_basicBlockLoop(BPatch_flowGraph *, Dyninst::PatchAPI::PatchLoop*)

  .. cpp:function:: private bool getLoops(BPatch_Vector<BPatch_basicBlockLoop*>&, bool outerMostOnly) const

    get either contained or outer loops, determined by outerMostOnly
