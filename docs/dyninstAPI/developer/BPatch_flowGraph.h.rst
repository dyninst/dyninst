.. _`sec-dev:BPatch_flowGraph.h`:

BPatch_flowGraph.h
##################

.. cpp:namespace:: dev

.. cpp:class:: BPatch_flowGraph : public Dyninst::AnnotatableSparse

  .. cpp:member:: private BPatch_function *func_
  .. cpp:member:: private BPatch_addressSpace *addSpace
  .. cpp:member:: private BPatch_module *mod

  .. cpp:member:: private std::set<BPatch_basicBlockLoop*> *loops

    set of loops contained in control flow graph

  .. cpp:member:: private std::set<BPatch_basicBlock*> allBlocks

    set of all basic blocks that control flow graph has

  .. cpp:member:: private BPatch_loopTreeNode *loopRoot

    root of the tree of loops

  .. cpp:member:: private std::set<BPatch_edge*> backEdges

    set of back edges

  .. cpp:member:: private bool isDominatorInfoReady

    flag that keeps whether dominator info is initialized

  .. cpp:member:: private bool isPostDominatorInfoReady

    flag that keeps whether postdominator info is initialized

  .. cpp:member:: private bool isSourceBlockInfoReady

    flag that keeps whether source block info is initialized


  .. cpp:member:: private std::map<Dyninst::PatchAPI::PatchLoop*, BPatch_basicBlockLoop*> _loop_map

  .. cpp:function:: private bool createBasicBlocks()

    This is the main method to create the basic blocks and the the edges between basic blocks. After
    finding the leaders, for each leader a basic block is created and then the predecessors and
    successors of the basic blocks are inserted to the basic blocks by passing from the function address
    space again, one instruction at a time, and using maps from basic blocks to leaders, and leaders to
    basic blocks. The basic block of which the leader is the start address of the function is assumed to
    be the entry block to control flow graph. This makes only one basic block to be in the entryBlocks
    field of the controlflow grpah. If it is possible to enter a function from many points some
    modification is needed to insert all entry basic blocks to the esrelevant field of the class.

  .. cpp:function:: bool createSourceBlocks()

    Creates the source line blocks of all blocks in CFG.

    This function must be called only after basic blocks have been created by calling createBasicBlocks.
    It computes the source block for each basic block. For now, a source block is represented by the
    starting and ending line numbers in the source block for the basic block. Without calling this method,
    line info is not available

  .. cpp:function:: void fillDominatorInfo()

    Fills the dominator information of each basic block looking at the control flow edges. It uses a
    fixed point calculation to find the immediate dominator of the basic blocks and the set of basic
    blocks that are immediately dominated by this one. Before calling this method all the dominator
    information is going to give incorrect results. So first this function must be called to process
    dominator related fields and methods.

  .. cpp:function:: void fillPostDominatorInfo()

    same as :cpp:func:`fillDominatorInfo`, but for postdominatorimmediate-postdom info

  .. cpp:function:: private void dfsVisitWithTargets(BPatch_basicBlock*,int*)
  .. cpp:function:: private void dfsVisitWithSources(BPatch_basicBlock*,int*)
  .. cpp:function:: private void findAndDeleteUnreachable()
  .. cpp:function:: private static void findBBForBackEdge(BPatch_edge*, std::set<BPatch_basicBlock*>&)
  .. cpp:function:: private void getLoopsByNestingLevel(BPatch_Vector<BPatch_basicBlockLoop*>&, bool outerMostOnly)

    Returns the loop objects that exist in the control flow grap.

  .. cpp:function:: private void dfsPrintLoops(BPatch_loopTreeNode *n)
  .. cpp:function:: private void createLoops()
  .. cpp:function:: private void dump()
  .. cpp:function:: private void findLoopExitInstPoints(BPatch_basicBlockLoop *loop, BPatch_Vector<BPatch_point*> *points)

  .. cpp:function:: BPatch_Vector<BPatch_point*>* findLoopInstPoints(const BPatch_procedureLocation loc, \
                                                                     Patch_basicBlockLoop *loop)

    .. code:: console

      We need to detect and handle following cases:

      (1) If a loop has no entry edge, e.g. the loop head is also
      first basic block of the function, we need to create a loop
      preheader.  we probably want to relocate the function at this
      point.

             ___
            v   |
        f: [_]--/
            |
           [ ]


      (2) If a loop header is shared between two loops then add a new
      nop node N, redirect the back edge of each loop to N and make N
      jump to the header. this transforms the two loops into a single
      loop.

           _              _
      --->[_]<---        [_]<---
      |  _/ \_  |       _/ \_  |
      \-[_] [_]-/      [_] [_] |
                         \_/   |
                         [N]---/


      Also, loop instrumentation works on the control flow as it was
      _originally_ parsed. Function entry/exit instrumentation may
      have modified the this control flow, but this new
      instrumentation is not cognizant of these modifications. This
      instrumentation therefore may clobber any instrumentation that
      was added because it has an inaccurate view of the binary.


      2014-10-14 Xiaozhu:
      Case (2) becomes irrelevant because under the new loop detection
      algorithm, there is going to be only one loop identified.
      Case (1) is still a problem.

  .. cpp:function:: bool getEntryBasicBlock(BPatch_Vector<BPatch_basicBlock*> &blocks)

    Actually, there must be only one entry point to each control flow graph but the definition given API
    specifications say there might be more.
