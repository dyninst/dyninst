.. _`sec-dev:Graph.h`:

Graph.h
#######

.. cpp:namespace:: Dyninst::dev

.. cpp:class:: Graph

  .. cpp:function:: protected void adjustEntryAndExitNodes()

  .. cpp:member:: protected NodeSet nodes_

    We also need to point to all Nodes to keep them alive we can't
    pervasively use shared_ptr within the graph because we're likely
    to have cycles.

  .. cpp:member:: protected NodeMap nodesByAddr_

  .. cpp:member:: protected NodeSet entryNodes_

    These may be overridden by derived classes, don't assume they exists.

  .. cpp:member:: protected NodeSet exitNodes_

    These may be overridden by derived classes, don't assume they exists.
