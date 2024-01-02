.. _`sec-dev:CodeObject.h`:

CodeObject.h
############

.. cpp:namespace:: Dyninst::ParseAPI::dev

.. cpp:class:: CodeObject

  .. cpp:function:: CFGFactory* fact()

      Returns the CFG factory.

  .. cpp:function:: Address getFreeAddr() const
  .. cpp:function:: ParseData* parse_data()

  .. cpp:function:: void startCallbackBatch()

      Starts a batch of callbacks that have been registered.

  .. cpp:function:: void finishCallbackBatch()

      Completes all callbacks in the current batch.

  .. cpp:function:: void registerCallback(ParseCallback* cb)

      Register a callback ``cb``

  .. cpp:function:: void unregisterCallback(ParseCallback* cb)

      Unregister an existing callback ``cb``

  .. cpp:function:: void finalize()

      Force complete parsing of the CodeObject; parsing operations are
      otherwise completed only as needed to answer queries.

  .. cpp:function:: void destroy(Edge*)

      Destroy the edge listed.

  .. cpp:function:: void destroy(Block*)

      Destroy the code block listed.

  .. cpp:function:: void destroy(Function*)

      Destroy the function listed.



Notes
=====

It is often much more efficient to  batch callbacks and deliver them
all at once than one at a time.  Particularly if we're deleting code,
it's better to get "The following blocks were deleted" than "block 1
was deleted; block 2 lost an edge; block 2 was deleted...". Callback
batching isn't currently used.
