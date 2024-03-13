.. _`sec:LoopAnalyzer.h`:

LoopAnalyzer.h
##############

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:class:: LoopAnalyzer

  **Finds loops in a function**

  | Implement WMZC algorithm to detect both natural loops  and irreducible loops
  | "A New Algorithm for Identifying Loops in Decompilation" by Tao Wei, Jian Mao, Wei Zou and Yu Chen

  .. cpp:function:: bool analyzeLoops()
  .. cpp:function:: void createLoopHierarchy()

      Creates the tree of loops/callees for this flow graph.

  .. cpp:function:: LoopAnalyzer (const Function *f)

  .. cpp:member:: private const Function *func
  .. cpp:member:: private std::map<Block*, set<Block*> > loop_tree
  .. cpp:member:: private std::map<Block*, Loop*> loops
  .. cpp:member:: private std::map<Block*, Block*> header
  .. cpp:member:: private std::map<Block*, int> DFSP_pos
  .. cpp:member:: private std::set<Block*> visited
  .. cpp:function:: private Block* WMZC_DFS(Block* b0, int pos)
  .. cpp:function:: private void WMZC_TagHead(Block* b, Block* h)
  .. cpp:function:: private void FillMoreBackEdges(Loop *loop)
  .. cpp:function:: private void dfsCreateLoopHierarchy(LoopTreeNode * parent, vector<Loop *> &loops, std::string level)
  .. cpp:function:: private static void findBBForBackEdge(Edge*, std::set<Block*>&)
  .. cpp:function:: private bool dfsInsertCalleeIntoLoopHierarchy(LoopTreeNode *node, Function *func, unsigned long addr)

    Try to insert func into the appropriate spot in the loop tree based on
    address ranges. if succesful return true, return false otherwise.

  .. cpp:function:: private void insertCalleeIntoLoopHierarchy(Function * func, unsigned long addr)
  .. cpp:function:: private void createLoops(Block* cur)

    Recursively build the basic blocks in a loop and the contained loops in a loop
