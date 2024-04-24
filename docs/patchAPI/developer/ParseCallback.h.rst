.. _`sec-dev:ParseCallback.h`:

ParseCallback.h
###############

.. cpp:namespace:: Dyninst::PatchAPI

.. cpp:class:: PatchParseCallback : public ParseAPI::ParseCallback

  **A wrapper around a ParseAPI::ParseCallback**

  .. cpp:function:: PatchParseCallback(PatchObject *obj)

      Creates a patch wrapper around the object ``obj``.

  .. cpp:function:: protected virtual void split_block_cb(ParseAPI::Block*, ParseAPI::Block*)
  .. cpp:function:: protected virtual void destroy_cb(ParseAPI::Block*)
  .. cpp:function:: protected virtual void destroy_cb(ParseAPI::Edge*)
  .. cpp:function:: protected virtual void destroy_cb(ParseAPI::Function*)
  .. cpp:function:: protected virtual void modify_edge_cb(ParseAPI::Edge*, ParseAPI::Block*, edge_type_t)
  .. cpp:function:: protected virtual void remove_edge_cb(ParseAPI::Block*, ParseAPI::Edge*, edge_type_t)
  .. cpp:function:: protected virtual void add_edge_cb(ParseAPI::Block*, ParseAPI::Edge*, edge_type_t)
  .. cpp:function:: protected virtual void remove_block_cb(ParseAPI::Function*, ParseAPI::Block*)
  .. cpp:function:: protected virtual void add_block_cb(ParseAPI::Function*, ParseAPI::Block*)

    Adds blocks lazily, basically does nothing unless block and function have already been created, in
    which case it adds the block to the function.

  .. cpp:function:: protected virtual bool absAddr(Address absolute, Address& loadAddr, ParseAPI::CodeObject*& containerObject)

    Returns the load address of the code object containing an absolute address.


Notes
=====

PatchAPI uses the ParseAPI CFG to construct its own CFG, which
means that we need to be notified of any changes in the underlying
CFG. These changes can come from a number of sources, including
the PatchAPI modification interface or a self-modifying binary.

The PatchAPI modification chain looks like the following:

  1) User requests PatchAPI to modify the CFG;
  2) PatchAPI makes the corresponding request to ParseAPI;
  3) ParseAPI modifies its CFG, triggering "modified CFG" callbacks;
  4) PatchAPI hooks these callbacks and updates its structures.

This is much easier than PatchAPI modifying them a priori, because
it allows for self-modifying code (which skips step 2) to work
with the exact same chain of events.
