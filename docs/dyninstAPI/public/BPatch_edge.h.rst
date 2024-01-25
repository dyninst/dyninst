.. _`sec:BPatch_edge.h`:

BPatch_edge.h
#############

.. cpp:class:: BPatch_edge
   
  The **BPatch_edge** class represents a control flow edge in a
  BPatch_flowGraph.

  .. cpp:function:: BPatch_point *getPoint()

    Return an instrumentation point for this edge. This point can be passed
    to BPatch_process::insertSnippet to instrument the edge.

  .. cpp:enum:: BPatch_edgeType
  .. cpp:enumerator:: BPatch_edgeType::CondJumpTaken
  .. cpp:enumerator:: BPatch_edgeType::CondJumpNottaken
  .. cpp:enumerator:: BPatch_edgeType::UncondJump
  .. cpp:enumerator:: BPatch_edgeType::NonJump

  .. cpp:function:: BPatch_edgeType getType()

    Return a type describing this edge. A CondJumpTaken edge is found after
    a conditional branch, along the edge that is taken when the condition is
    true. A CondJumpNottaken edge follows the path when the condition is not
    taken. UncondJump is used along an edge that flows out of an
    unconditional branch that is always taken. NonJump is an edge that flows
    out of a basic block that does not end in a jump, but falls through into
    the next basic block.

  .. cpp:function:: BPatch_basicBlock *getSource()

    Return the source BPatch_basicBlock that this edge flows from.

  .. cpp:function:: BPatch_basicBlock *getTarget()

    Return the target BPatch_basicBlock that this edge flows to.

  .. cpp:function:: BPatch_flowGraph *getFlowGraph()

    Returns the CFG that contains the edge.
