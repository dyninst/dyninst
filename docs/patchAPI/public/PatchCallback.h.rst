PatchCallback.h
===============

.. cpp:namespace:: Dyninst::patchAPI

PatchCallback
=============

**Declared in**: PatchCallback.h

The PatchAPI CFG layer may change at runtime due to program events
(e.g., a program loading additional code or overwriting its own code
with new code). The ``PatchCallback`` interface allows users to specify
callbacks they wish to occur whenever the PatchAPI CFG changes.

.. code-block:: cpp
    
    virtual void destroy_cb(PatchBlock *); virtual void
    destroy_cb(PatchEdge *); virtual void destroy_cb(PatchFunction *);
    virtual void destroy_cb(PatchObject *);

Programmers implement the above virtual methods to handle the event of
destroying a PatchBlock, a PatchEdge, a PatchFunction, or a PatchObject
respectively. All the above methods will be called before corresponding
object destructors are called.

.. code-block:: cpp
    
    virtual void create_cb(PatchBlock *); virtual void create_cb(PatchEdge
    *); virtual void create_cb(PatchFunction *); virtual void
    create_cb(PatchObject *);

Programmers implement the above virtual methods to handle the event of
creating a PatchBlock, a PatchEdge, a PatchFunction, or a PatchObject
respectively. All the above methods will be called after the objects are
created.

.. code-block:: cpp
    
    virtual void split_block_cb(PatchBlock *first, PatchBlock *second);

Programmers implement the above virtual method to handle the event of
splitting a PatchBlock as a result of a new edge being discovered. The
above method will be called after the block is split.

.. code-block:: cpp
    
    virtual void remove_edge_cb(PatchBlock *, PatchEdge *, edge_type_t);
    virtual void add_edge_cb(PatchBlock *, PatchEdge *, edge_type_t);

Programmers implement the above virtual methods to handle the events of
removing or adding an PatchEdge respectively. The method remove_edge_cb
will be called before the event triggers, while the method add_edge_cb
will be called after the event triggers.

.. code-block:: cpp
    
    virtual void remove_block_cb(PatchFunction *, PatchBlock *); virtual
    void add_block_cb(PatchFunction *, PatchBlock *);

Programmers implement the above virtual methods to handle the events of
removing or adding a PatchBlock respectively. The method remove_block_cb
will be called before the event triggers, while the method add_block_cb
will be called after the event triggers.

.. code-block:: cpp
    
    virtual void create_cb(Point *pt); virtual void destroy_cb(Point *pt);

Programmers implement the create_cb method above, which will be called
after the Point *pt* is created. And, programmers implement the
destroy_cb method, which will be called before the point *pt* is
deleted.

.. code-block:: cpp
    
    virtual void change_cb(Point *pt, PatchBlock *first, PatchBlock *second);

Programmers implement this method, which is to be invoked after a block
is split. The provided Point belonged to the first block and is being
moved to the second.