.. _`sec:PatchModifier.h`:

PatchModifier.h
###############

.. cpp:namespace:: Dyninst::PatchAPI

.. cpp:class:: PatchModifier

  .. cpp:function:: static bool redirect(PatchEdge *edge, PatchBlock *target)

      Set the target of the existing edge, ``edge``, to ``target``.

  .. cpp:function:: static PatchBlock* split(PatchBlock* b, Address addr, bool trust = false, Address newlast = (Address)-1)

      Split the block, ``b``, at the address ``addr``.

      We double-check whether the address is a valid instruction boundary unless trust is true.

  .. cpp:function:: static bool remove(std::vector<PatchBlock *> &blocks, bool force = false)

      Remove the blocks, ``blocks`` from the CFG.

      The block must be unreachable (that is, have no in-edges) unless ``force`` is ``true``.

  .. cpp:function:: static bool remove(PatchFunction* f)

      Removes the function ``f`` and all of its blocks.

  .. cpp:function:: static InsertedCode::Ptr insert(PatchObject* o, SnippetPtr snip, Point* point)

      Inserts the snippet ``snip`` into the object ``o`` at point ``point``.

      Returns the inserted code block.

  .. cpp:function:: static InsertedCode::Ptr insert(PatchObject* o, void* start, unsigned size)

      Inserts the ``size`` bytes from ``start`` into the first free address in object ``o``.

      Returns the inserted code block.

.. cpp:class:: InsertedCode

  .. cpp:type:: boost::shared_ptr<InsertedCode> Ptr

  .. cpp:function:: InsertedCode()
  .. cpp:function:: PatchBlock* entry()
  .. cpp:function:: const std::vector<PatchEdge*> &exits()
  .. cpp:function:: const std::set<PatchBlock*> &blocks()
