.. _`sec-dev:PatchCFG.h`:

PatchCFG.h
##########

.. cpp:namespace:: Dyninst::PatchAPI::dev

.. cpp:class:: PatchFunction

  .. cpp:function:: bool findInsnPoints(Point::Type type, PatchBlock *block, InsnPoints::const_iterator &start, InsnPoints::const_iterator &end)

      Fast access to a range of instruction points

  .. cpp:function:: Point *findPoint(Location loc, Point::Type type, bool create = true)

  .. cpp:function:: bool verifyExit(PatchBlock *block)

      Moved to source code because we treat non-returning calls as exits.

  .. cpp:function:: bool verifyCall(PatchBlock *block)

      Moved to source code because we treat non-returning calls as exits.

  .. cpp:function:: bool verifyExitConst(const PatchBlock *block) const

      Does not build the exit/call sets (returns true if set is empty)

  .. cpp:function:: bool verifyCallConst(const PatchBlock *block) const

      Does not build the exit/call sets (returns true if set is empty)

  .. cpp:function:: void remove(Point *p)

      Removes the patch function at point ``p``.

  .. cpp:function:: bool consistency() const

  .. cpp:function:: protected void removeBlock(PatchBlock *)
  .. cpp:function:: protected void addBlock(PatchBlock *)
  .. cpp:function:: protected void splitBlock(PatchBlock *first, PatchBlock *second)
  .. cpp:function:: protected void destroyPoints()

    Destroy points for this block and then each containing function's context specific points for the
    block.

  .. cpp:function:: protected void destroyBlockPoints(PatchBlock *block)
  .. cpp:function:: protected void invalidateBlocks()
  .. cpp:function:: protected void getLoopsByNestingLevel(vector<PatchLoop*>& lbb, bool outerMostOnly)
  .. cpp:function:: protected void createLoops()
  .. cpp:function:: protected void createLoopHierarchy()
  .. cpp:function:: protected void fillDominatorInfo()
  .. cpp:function:: protected void fillPostDominatorInfo()

  .. cpp:member:: protected ParseAPI::Function *func_
  .. cpp:member:: protected PatchObject* obj_
  .. cpp:member:: protected Address addr_
  .. cpp:member:: protected Blockset all_blocks_
  .. cpp:member:: protected Blockset call_blocks_
  .. cpp:member:: protected Blockset return_blocks_
  .. cpp:member:: protected Blockset exit_blocks_
  .. cpp:member:: protected FuncPoints points_
  .. cpp:member:: protected std::map<PatchBlock *, BlockPoints> blockPoints_
  .. cpp:member:: protected std::map<PatchEdge *, EdgePoints> edgePoints_
  .. cpp:member:: protected bool _loop_analyzed

      ``true`` if loops in the function have been found and stored in :cpp:member:`_loops`.

  .. cpp:member:: protected std::set<PatchLoop*> _loops
  .. cpp:member:: protected map<ParseAPI::Loop*, PatchLoop*> _loop_map
  .. cpp:member:: protected PatchLoopTreeNode *_loop_root

      ``NULL`` if the tree structure has not be calculated.

  .. cpp:member:: protected bool isDominatorInfoReady
  .. cpp:member:: protected bool isPostDominatorInfoReady
  .. cpp:member:: protected std::map<PatchBlock*, std::set<PatchBlock*>*> immediateDominates

      Set of basic blocks that this basic block dominates immediately.

  .. cpp:member:: protected std::map<PatchBlock*, PatchBlock*> immediateDominator

      Basic block which is the immediate dominator of the basic block.

  .. cpp:member:: protected std::map<PatchBlock*, std::set<PatchBlock*>*> immediatePostDominates

      Same as previous two fields, but for postdominator tree.

  .. cpp:member:: protected std::map<PatchBlock*, PatchBlock*> immediatePostDominator

.. cpp:class:: PatchLoop

  .. cpp:member:: private std::set<PatchBlock*> entries

      The entries of the loop.

  .. cpp:member:: private PatchFunction* func

      The function this loop is part of.

  .. cpp:member:: private std::set<PatchLoop*> containedLoops

      The set of loops that are contained (nested) in this loop.

  .. cpp:member:: private std::set<PatchBlock*> basicBlocks

      The basic blocks in the loop.

  .. cpp:function:: bool getLoops(vector<PatchLoop*>&, bool outerMostOnly) const

      Get either contained or outer loops, determined by outerMostOnly.

.. cpp:class:: PatchLoopTreeNode

  .. cpp:member:: private char *hierarchicalName

      Name indicating this loop's relative nesting.

  .. cpp:member:: private vector<PatchFunction*> callees

      Functions called within the body of this loop (and not the body of sub loops).

.. cpp:class:: PatchEdge

  .. cpp:function:: PatchCallback* cb() const

      Returns a Patchcallback object associated with this PatchEdge.

  .. cpp:function:: void remove(Point* p)

  .. cpp:function:: bool consistency() const

  .. cpp:member:: protected ParseAPI::Edge *edge_
  .. cpp:member:: protected PatchBlock *src_
  .. cpp:member:: protected PatchBlock *trg_
  .. cpp:member:: protected EdgePoints points_

.. cpp:class:: PatchBlock

  .. cpp:function:: Point *findPoint(Location loc, Point::Type type, bool create = true)
  .. cpp:function:: PatchCallback* cb() const

      Returns the PatchCallback object associated with this PatchBlock.

Notes
=====

The PatchLoopTreeNode class provides a tree interface to a collection of
instances of class PatchLoop contained in a function. The structure of
the tree follows the nesting relationship of the loops in a function.
Each PatchLoopTreeNode contains a pointer to a loop (represented by
PatchLoop), and a set of sub-loops (represented by other
PatchLoopTreeNode objects). The ``loop`` field at the root node is
always ``NULL`` since a function may contain multiple outer loops. The
``loop`` field is never ``NULL`` at any other node since it always
corresponds to a real loop. Therefore, the outer most loops in the
function are contained in the vector of ``children`` of the root.

Each instance of PatchLoopTreeNode is given a name that indicates its
position in the hierarchy of loops. The name of each outermost loop
takes the form of ``loop_x``, where ``x`` is an integer from 1 to n,
where n is the number of outer loops in the function. Each sub-loop has
the name of its parent, followed by a ``.y``, where ``y`` is 1 to m,
where m is the number of sub-loops under the outer loop. For example,
consider the following C function:

.. code-block:: cpp
    
   void foo() {
     int x, y, z, i;
     for (x=0 x<10 x++) {
       for (y = 0 y<10 y++)
         ...
       for (z = 0 z<10 z++)
         ...
     }
     for (i = 0 i<10 i++) {
        ...
     }
   }

The ``foo`` function will have a root PatchLoopTreeNode, containing a
NULL loop entry and two PatchLoopTreeNode children representing the
functions outermost loops. These children would have names ``loop_1``
and ``loop_2``, respectively representing the ``x`` and ``i`` loops.
``loop_2`` has no children. ``loop_1`` has two child PatchLoopTreeNode
objects, named ``loop_1.1`` and ``loop_1.2``, respectively representing
the ``y`` and ``z`` loops.
