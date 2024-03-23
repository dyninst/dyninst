.. _`sec-dev:BPatch_loopTreeNode.h`:

BPatch_loopTreeNode.h
#####################

.. cpp:namespace:: dev

.. cpp:class:: BPatch_loopTreeNode

  .. cpp:var:: private char *hierarchicalName

  name which indicates this loop's relative nesting

  .. cpp:var:: private BPatch_Vector<func_instance *> callees

  A vector of functions called within the body of this loop (and
  not the body of sub loops).
