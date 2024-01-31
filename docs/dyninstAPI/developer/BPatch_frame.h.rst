.. _`sec-dev:BPatch_frame.h`:

BPatch_frame.h
##############

.. cpp:namespace:: dev

.. cpp:class:: BPatch_frame

  .. cpp:member:: private bool isSynthFrame

    BPatch defines that a trampoline is effectively a "function call" and puts an extra tramp on the stack. Various people (frex, Paradyn) really don't want to see this frame. To make life simpler for everyone, we add a "only call if you know what you're doing" flag.

  .. cpp:member:: private BPatch_point *point_

    This is _so_ much easier than looking it up later. If we're in instrumentation, stash the point
