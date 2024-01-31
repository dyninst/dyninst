.. _`sec-dev:BPatch_flowGraph.h`:

BPatch_flowGraph.h
##################

.. cpp:namespace:: dev

.. cpp:class:: BPatch_flowGraph : public Dyninst::AnnotatableSparse

  .. cpp:member:: std::set<BPatch_basicBlockLoop*> *loops

    set of loops contained in control flow graph

  .. cpp:member:: std::set<BPatch_basicBlock*> allBlocks

    set of all basic blocks that control flow graph has

  .. cpp:member:: BPatch_loopTreeNode *loopRoot

    root of the tree of loops

  .. cpp:member:: std::set<BPatch_edge*> backEdges

    set of back edges

  .. cpp:member:: bool isDominatorInfoReady

    flag that keeps whether dominator info is initialized

  .. cpp:member:: bool isPostDominatorInfoReady

    flag that keeps whether postdominator info is initialized

  .. cpp:member:: bool isSourceBlockInfoReady

    flag that keeps whether source block info is initialized

