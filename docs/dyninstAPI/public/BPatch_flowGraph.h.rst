.. _`sec:BPatch_flowGraph.h`:

BPatch_flowGraph.h
##################

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

.. cpp:class:: BPatch_flowGraph : public Dyninst::AnnotatableSparse
   
  **The control flow graph of a function**

  It provides methods for discovering the basic blocks and loops
  within the function (using which a caller can navigate the graph). A
  BPatch_flowGraph object can be obtained by calling the getCFG method of
  a BPatch_function object.

  .. cpp:function:: BPatch_addressSpace *getAddSpace() const
  .. cpp:function:: AddressSpace *getllAddSpace() const
  .. cpp:function:: BPatch_function *getFunction() const
  .. cpp:function:: BPatch_module *getModule() const
  .. cpp:function:: BPatch_basicBlock *findBlock(block_instance *b)
  .. cpp:function:: BPatch_edge *findEdge(edge_instance *e)
  .. cpp:function:: void invalidate()

      invoked when additional parsing takes place

  .. cpp:function:: ~BPatch_flowGraph()

  .. cpp:function:: bool getAllBasicBlocks(BPatch_Set<BPatch_basicBlock*> &blocks)

    returns the set of all basic blocks in the CFG

  .. cpp:function:: bool getAllBasicBlocks(std::set<BPatch_basicBlock *> &blocks)

  returns the set of all basic blocks in the CFG

  .. cpp:function:: bool getEntryBasicBlock(BPatch_Vector<BPatch_basicBlock*> &blocks)

    Returns the basic blocks that are entry points in the control flow graph.

  .. cpp:function:: bool getExitBasicBlock(BPatch_Vector<BPatch_basicBlock*> &blocks)

    Returns the basic blocks that are the exit basic blocks from the control flow graph.
    That is, those are the basic blocks that contains the instruction for returning from
    the function.

  .. cpp:function:: BPatch_basicBlock *findBlockByAddr(Dyninst::Address addr)

    Find the basic block within this flow graph that contains addr. Returns
    NULL on failure. This method is inefficient but guaranteed to succeed if
    addr is present in any block in this CFG.

    .. warning: this method is slow!

  .. cpp:function:: bool getLoops(BPatch_Vector<BPatch_basicBlockLoop*> &loops)

    Returns in ``loops`` the natural (single entry) loops in the control flow graph.

  .. cpp:function:: bool getOuterLoops(BPatch_Vector<BPatch_basicBlockLoop*> &loops)

    Returns in ``loops`` the natural (single entry) outer loops in the control flow graph.

  .. cpp:function:: BPatch_loopTreeNode * getLoopTree()

    Return the root node of the tree of loops in this flow graph.

  .. cpp:function:: bool containsDynamicCallsites()

    Checks if the control flow graph contains any dynamic call sites (e.g., calls through a function pointer).

  .. cpp:function:: void printLoops()

    for debugging, print loops with line numbers to stderr

  .. cpp:function:: BPatch_basicBlockLoop * findLoop(const char *name)
  .. cpp:function:: bool isValid()
  .. cpp:function:: BPatch_Vector<BPatch_point*>* findLoopInstPoints(const BPatch_procedureLocation loc, \
                                                                     Patch_basicBlockLoop *loop)

    find instrumentation points specified by loc, add to points

  .. cpp:function:: std::vector<BPatch_point*> *findLoopInstPoints(const BPatch_procedureLocation loc, \
                                                                   BPatch_basicBlockLoop *loop)

    Find instrumentation points for the given loop that correspond to the
    given location: loop entry, loop exit, the start of a loop iteration and
    the end of a loop iteration. BPatch_locLoopEntry and BPatch_locLoopExit
    instrumentation points respectively execute once before the first
    iteration of a loop and after the last iteration.
    BPatch_locLoopStartIter and BPatch_locLoopEndIter respectively execute
    at the beginning and end of each loop iteration.
