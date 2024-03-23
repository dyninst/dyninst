.. _`sec:BPatch_edge.h`:

BPatch_edge.h
#############

.. cpp:class:: BPatch_edge
   
  **A an edge in a control flow graph**

  .. cpp:function:: BPatch_edge(edge_instance *e, BPatch_flowGraph *fg)

  .. cpp:function:: ~BPatch_edge()

  .. cpp:function:: void dump()

    Print internal data

  .. cpp:function:: BPatch_basicBlock * getSource()

    Return the source BPatch_basicBlock that this edge flows from.

  .. cpp:function:: BPatch_basicBlock * getTarget()

    Return the target BPatch_basicBlock that this edge flows to.

  .. cpp:function:: BPatch_point * getPoint()

    Return an instrumentation point for this edge. This point can be passed
    to BPatch_process::insertSnippet to instrument the edge.

  .. cpp:function:: BPatch_edgeType getType()

    Return a type describing this edge. A CondJumpTaken edge is found after
    a conditional branch, along the edge that is taken when the condition is
    true. A CondJumpNottaken edge follows the path when the condition is not
    taken. UncondJump is used along an edge that flows out of an
    unconditional branch that is always taken. NonJump is an edge that flows
    out of a basic block that does not end in a jump, but falls through into
    the next basic block.

  .. cpp:function:: BPatch_flowGraph * getFlowGraph()

    Returns the CFG that contains the edge.


.. cpp:enum:: BPatch_edgeType

  .. cpp:enumerator:: CondJumpTaken
  .. cpp:enumerator:: CondJumpNottaken
  .. cpp:enumerator:: UncondJump
  .. cpp:enumerator:: NonJump

  .. note:: This ignores indirect jumps
