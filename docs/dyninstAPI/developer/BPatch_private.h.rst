.. _`sec:BPatch_private.h`:

BPatch_private.h
################

.. cpp:struct:: batchInsertionRecord

  .. cpp:member:: BPatch_thread *thread_

    Thread-specific instruction

  .. cpp:member:: std::vector<BPatch_point *> points_

    For delayed insertion vector because there is a vector-insert technique

  .. cpp:member:: std::vector<callWhen> when_

    This has to be vectorized to handle the multiple-point insertion + edges.

  .. cpp:member:: callOrder order_

  .. cpp:member:: BPatch_snippet snip

      Make a copy so that the user doesn't have to.

  .. cpp:member:: BPatchSnippetHandle *handle_

      handle to fill in

  .. cpp:member:: bool trampRecursive_
