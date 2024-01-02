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
