.. _`sec-dev:BPatch_basicBlock.h`:

BPatch_basicBlock.h
###################

.. cpp:namespace:: dev

.. cpp:class:: BPatch_basicBlock

  .. cpp:member:: private block_instance *iblock

    the internal basic block structure

  .. cpp:member:: private BPatch_flowGraph *flowGraph

    the flow graph that contains this basic block

  .. cpp:member:: private std::set<BPatch_basicBlock*>* immediateDominates

    set of basic blocks that this basicblock dominates immediately

  .. cpp:member:: private BPatch_basicBlock *immediateDominator

    basic block which is the immediate dominator of the basic block

  .. cpp:member:: private std::set<BPatch_basicBlock*> *immediatePostDominates

    same as previous two fields, but for postdominator tree

  .. cpp:member:: private BPatch_basicBlock *immediatePostDominator
  .. cpp:member:: private BPatch_Vector<BPatch_sourceBlock*> *sourceBlocks

    the source block(source lines) that basic block corresponds

  .. cpp:member:: private BPatch_Vector<BPatch_instruction*> *instructions

    the instructions within this block

  .. cpp:member:: private std::set<BPatch_edge*> incomingEdges

    the incoming edges

  .. cpp:member:: private std::set<BPatch_edge*> outgoingEdges

    the outgoing edges

  .. cpp:function:: protected BPatch_basicBlock(block_instance *ib, BPatch_flowGraph *fg)

    constructor of class

  .. cpp:function:: protected BPatch_Vector<BPatch_point*>* findPointByPredicate(insnPredicate& f)

  .. cpp:function:: block_instance *lowlevel_block()

    Internal functions. Don't use these unless you know what you're doing.

  .. cpp:function:: void setlowlevel_block(block_instance *b)
  .. cpp:function:: void getAllPoints(std::vector<BPatch_point*>& allPoints)

