PatchObject.h
=============

.. cpp:namespace:: Dyninst::patchAPI

PatchObject
===========

**Declared in**: PatchObject.h

The PatchObject class is a wrapper of ParseAPI’s CodeObject class
(has-a), which represents an individual binary code object, such as an
executable or a library.

.. code-block:: cpp
    
    static PatchObject* create(ParseAPI::CodeObject* co, Address base,
    CFGMaker* cm = NULL, PatchCallback *cb = NULL);

Creates an instance of PatchObject, which has *co* as its on-disk
representation (ParseAPI::CodeObject), and *base* as the base address
where this object is loaded in the memory. For binary rewriting, base
should be 0. The *cm* and *cb* parameters are for registering plugins.
If *cm* or *cb* is NULL, then we use the default implementation of
CFGMaker or PatchCallback.

.. code-block:: cpp
    
    static PatchObject* clone(PatchObject* par_obj, Address base,
    CFGMaker* cm = NULL, PatchCallback *cb = NULL);

Returns a new object that is copied from the specified object *par_obj*
at the loaded address *base* in the memory. For binary rewriting, base
should be 0. The *cm* and *cb* parameters are for registering plugins.
If *cm* or *cb* is NULL, then we use the default implementation of
CFGMaker or PatchCallback.

.. code-block:: cpp
    
    Address codeBase();

Returns the base address where this object is loaded in memory.

.. code-block:: cpp
    
    PatchFunction *getFunc(ParseAPI::Function *func, bool create = true);

Returns an instance of PatchFunction in this object, based on the *func*
parameter. PatchAPI creates a PatchFunction on-demand, so if there is
not any PatchFunction created for the ParseAPI function *func*, and the
*create* parameter is false, then no any instance of PatchFunction will
be created.

It returns NULL in two cases. First, the function *func* is not in this
PatchObject. Second, the PatchFunction is not yet created and the
*create* is false. Otherwise, it returns a PatchFunction.

.. code-block:: cpp
    
    template <class Iter> void funcs(Iter iter);

Outputs all instances of PatchFunction in this PatchObject to the STL
inserter *iter*.

.. code-block:: cpp
    
    PatchBlock *getBlock(ParseAPI::Block* blk, bool create = true);

Returns an instance of PatchBlock in this object, based on the *blk*
parameter. PatchAPI creates a PatchBlock on-demand, so if there is not
any PatchBlock created for the ParseAPI block *blk*, and the *create*
parameter is false, then no any instance of PatchBlock will be created.

It returns NULL in two cases. First, the ParseAPI block *blk* is not in
this PatchObject. Second, the PatchBlock is not yet created and the
*create* is false. Otherwise, it returns a PatchBlock.

.. code-block:: cpp
    
    template <class Iter> void blocks(Iter iter);

Outputs all instances of PatchBlock in this object to the STL inserter
*iter*.

.. code-block:: cpp
    
     PatchEdge *getEdge(ParseAPI::Edge* edge, PatchBlock* src,
     PatchBlock* trg, bool create = true);

Returns an instance of PatchEdge in this object, according to the
parameters ParseAPI::Edge *edge*, source PatchBlock *src*, and target
PatchBlock *trg*. PatchAPI creates a PatchEdge on-demand, so if there is
not any PatchEdge created for the ParseAPI *edge*, and the *create*
parameter is false, then no any instance of PatchEdge will be created.

It returns NULL in two cases. First, the ParseAPI *edge* is not in this
PatchObject. Second, the PatchEdge is not yet created and the *create*
is false. Otherwise, it returns a PatchEdge.

.. code-block:: cpp
    
    template <class Iter> void edges(Iter iter);

Outputs all instances of PatchEdge in this object to the STL inserter
*iter*.

.. code-block:: cpp
    
   PatchCallback *cb() const;

Returns the PatchCallback object associated with this PatchObject.

.. _sec-3.2.9: