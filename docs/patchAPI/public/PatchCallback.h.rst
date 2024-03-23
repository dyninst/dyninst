.. _`sec:PatchCallback.h`:

PatchCallback.h
###############

.. cpp:namespace:: Dyninst::PatchAPI

.. cpp:class:: PatchCallback

 **A callback invoked when the CFG changes**

  The PatchAPI CFG layer may change at runtime due to program events
  (e.g., a program loading additional code or overwriting its own code
  with new code). The ``PatchCallback`` interface allows users to specify
  callbacks they wish to occur whenever the PatchAPI CFG changes.

  .. cpp:enum:: edge_type_t

    .. cpp:enumerator:: source
    .. cpp:enumerator:: target

  .. important:: Users should only override the ``_cb`` member functions.

  .. cpp:function:: protected virtual void destroy_cb(PatchBlock* b)

      Invoked immediately before ``b``\ 's destructor is run.

  .. cpp:function:: protected virtual void destroy_cb(PatchEdge* e, PatchObject* o)

      Invoked immediately before ``e``\ 's destructor is run.

  .. cpp:function:: protected virtual void destroy_cb(PatchFunction* f)

      Invoked immediately before ``f``\ 's destructor is run.

  .. cpp:function:: protected virtual void destroy_cb(PatchObject* o)

      Invoked immediately before ``o``\ 's destructor is run.

  .. cpp:function:: protected virtual void create_cb(PatchBlock* b)

      Invoked immediately after ``b``\ 's constructor is run.

  .. cpp:function:: protected virtual void create_cb(PatchEdge* e)

      Invoked immediately after ``e``\ 's constructor is run.

  .. cpp:function:: protected virtual void create_cb(PatchFunction* f)

      Invoked immediately after ``f``\ 's constructor is run.

  .. cpp:function:: protected virtual void create_cb(PatchObject* o)

      Invoked immediately after ``o``\ 's constructor is run.

  .. cpp:function:: protected virtual void split_block_cb(PatchBlock* first, PatchBlock* second)

      Invoked after a block is split into ``first`` and ``second``.

  .. cpp:function:: protected virtual void remove_edge_cb(PatchBlock* b, PatchEdge* e, edge_type_t t)

      Invoked before the edge ``e`` of type ``t`` is removed from the block ``b``.

  .. cpp:function:: protected virtual void add_edge_cb(PatchBlock* b, PatchEdge* e, edge_type_t t)

      Invoked after the edge ``e`` of type ``t`` is added to the block ``b``.

  .. cpp:function:: protected virtual void remove_block_cb(PatchFunction* f, PatchBlock* b)

      Invoked after the block ``b`` is removed from function ``f``.

  .. cpp:function:: protected virtual void add_block_cb(PatchFunction* f, PatchBlock* b)

      Invoked before the block ``b`` is added to function ``f``.

  .. cpp:function:: protected virtual void create_cb(Point* pt)

      Invoked after the point ``pt`` is created.

  .. cpp:function:: protected virtual void destroy_cb(Point* pt)

      Invoked before the point ``pt`` is destroyed.

  .. cpp:function:: protected virtual void change_cb(Point* pt, PatchBlock* first, PatchBlock* second)

      Invoked after a block is split.

      ``pt`` belongs to the block ``first`` and is being moved to the block ``second``.

  .. cpp:function:: void destroy(PatchBlock* b)

      Invokes :cpp:func:`destroy_cb` and destroys the block ``b``.

  .. cpp:function:: void destroy(PatchEdge* e, PatchObject* owner)

      Invokes :cpp:func:`destroy_cb` and destroys the edge ``e`` owned by ``owner``.

  .. cpp:function:: void destroy(PatchFunction* f)

      Invokes :cpp:func:`destroy_cb` and destroys the function ``f``.

  .. cpp:function:: void destroy(PatchObject* o)

      Invokes :cpp:func:`destroy_cb` and destroys the object ``o``.

  .. cpp:function:: void create(PatchBlock* b)

      Invokes :cpp:func:`create_cb` for the block ``b``.

  .. cpp:function:: void create(PatchEdge* e)

      Invokes :cpp:func:`create_cb` for the edge ``e``.

  .. cpp:function:: void create(PatchFunction* f)

      Invokes :cpp:func:`create_cb` for the function ``f``.

  .. cpp:function:: void create(PatchObject* o)

      Invokes :cpp:func:`create_cb` for the object ``o``.

  .. cpp:function:: void split_block(PatchBlock* b1, PatchBlock* b2)

      Invokes :cpp:func:`split_block_cb` when a block is split into ``b1`` and ``b1``.

  .. cpp:function:: void remove_edge(PatchBlock* b, PatchEdge* e, edge_type_t t)

      Invokes :cpp:func:`remove_edge_cb` when the edge ``e`` of type ``t`` is removed from ``b``.

  .. cpp:function:: void add_edge(PatchBlock* b, PatchEdge* e, edge_type_t t)

      Invokes :cpp:func:`add_edge_cb` when the edge ``e`` of type ``t`` is added to ``b``.

  .. cpp:function:: void remove_block(PatchFunction* f, PatchBlock* b)

      Invokes :cpp:func:`remove_block_cb` when the block ``b`` is removed from ``f``.

  .. cpp:function:: void add_block(PatchFunction* f, PatchBlock* b)

      Invokes :cpp:func:`add_block_cb` when the block ``b`` is added to ``f``.

  .. cpp:function:: void destroy(Point* p)

      Invokes :cpp:func:`destroy_cb` when ``p`` is destroyed.

  .. cpp:function:: void create(Point* p)

      Invokes :cpp:func:`destroy_cb` when ``p`` is created.

  .. cpp:function:: void change(Point* p, PatchBlock* first, PatchBlock *second)

      Invokes :cpp:func:`change_cb` when after a block is split.

      ``pt`` belongs to the block ``first`` and is being moved to the block ``second``.

  .. cpp:function:: void batch_begin()

      Starts batching.

  .. cpp:function:: void batch_end()

      Terminates batching.

Batching
========

Instead of invoking a callback for each event when it happens, the user can accumulate *all*
callback invocations and then request their invocation. This is referred to as ``batching``.
:cpp:func:`PatchCallback::batch_begin()` enables batching :cpp:func:`PatchCallback::batch_end()`
terminates batching and invokes all outstanding callbacks. Users can enable and disable batching
at any time. This can be useful for reducing overhead of creating many objects at once. However,
calling ``batch_end`` always invokes all outstanding callbacks. It is not possible to ignore
callback invocations.
