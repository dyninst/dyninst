.. _`sec:BPatch_flowGraph.h`:

BPatch_flowGraph.h
##################

.. cpp:class:: BPatch_flowGraph
   
  The **BPatch_flowGraph** class represents the control flow graph of a
  function. It provides methods for discovering the basic blocks and loops
  within the function (using which a caller can navigate the graph). A
  BPatch_flowGraph object can be obtained by calling the getCFG method of
  a BPatch_function object.

  .. cpp:function:: bool containsDynamicCallsites()

    Return true if the control flow graph contains any dynamic call sites
    (e.g., calls through a function pointer).

  .. cpp:function:: void getAllBasicBlocks(std::set<BPatch_basicBlock*>&)

  .. cpp:function:: void getAllBasicBlocks(BPatch_Set<BPatch_basicBlock*>&)

    Fill the given set with pointers to all basic blocks in the control flow
    graph. BPatch_basicBlock is described in section 4.17.

  .. cpp:function:: void getEntryBasicBlock(std::vector<BPatch_basicBlock*>&)

    Fill the given vector with pointers to all basic blocks that are entry
    points to the function. BPatch_basicBlock is described in section 4.17.

  .. cpp:function:: void getExitBasicBlock(std::vector<BPatch_basicBlock*>&)

    Fill the given vector with pointers to all basic blocks that are exit
    points of the function. BPatch_basicBlock is described in section 4.17.

  .. cpp:function:: void getLoops(std::vector<BPatch_basicBlockLoop*>&)

    Fill the given vector with a list of all natural (single entry) loops in
    the control flow graph.

  .. cpp:function:: void getOuterLoops(std::vector<BPatch_basicBlockLoop*>&)

    Fill the given vector with a list of all natural (single entry) outer
    loops in the control flow graph.

  .. cpp:function:: BPatch_loopTreeNode *getLoopTree()

    Return the root node of the tree of loops in this flow graph.

  .. cpp:enum:: BPatch_procedureLocation
  .. cpp:enumerator:: BPatch_procedureLocation::BPatch_locLoopEntry
  .. cpp:enumerator:: BPatch_procedureLocation::BPatch_locLoopExit
  .. cpp:enumerator:: BPatch_procedureLocation::BPatch_locLoopStartIter
  .. cpp:enumerator:: BPatch_procedureLocation::BPatch_locLoopEndIter

  .. cpp:function:: std::vector<BPatch_point*> *findLoopInstPoints( \
       const BPatch_procedureLocation loc, BPatch_basicBlockLoop *loop);

    Find instrumentation points for the given loop that correspond to the
    given location: loop entry, loop exit, the start of a loop iteration and
    the end of a loop iteration. BPatch_locLoopEntry and BPatch_locLoopExit
    instrumentation points respectively execute once before the first
    iteration of a loop and after the last iteration.
    BPatch_locLoopStartIter and BPatch_locLoopEndIter respectively execute
    at the beginning and end of each loop iteration.

  .. cpp:function:: BPatch_basicBlock* findBlockByAddr(Dyninst::Address addr);

    Find the basic block within this flow graph that contains addr. Returns
    NULL on failure. This method is inefficient but guaranteed to succeed if
    addr is present in any block in this CFG.

    .. note::
       Dyninst is not always able to generate a correct flow graph
       in the presence of indirect jumps. If a function has a case statement or
       indirect jump instructions, the targets of the jumps are found by
       searching instruction patterns (peep-hole). The instruction patterns
       generated are compiler specific and the control flow graph analyses
       include only the ones we have seen. During the control flow graph
       generation, if a pattern that is not handled is used for case statement
       or multi-jump instructions in the function address space, the generated
       control flow graph may not be complete.
