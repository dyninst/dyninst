CFGMaker.h
==========

.. cpp:namespace:: Dyninst::patchAPI

CFGMaker
========

**Declared in**: CFGMaker.h

The CFGMaker class is a factory class that constructs the above CFG
structures (PatchFunction, PatchBlock, and PatchEdge). The methods in
this class are used by PatchObject. Programmers can extend
PatchFunction, PatchBlock and PatchEdge by annotating their own data,
and then use this class to instantiate these CFG structures.

.. code-block:: cpp
    
    virtual PatchFunction* makeFunction(ParseAPI::Function* func,
    PatchObject* obj); virtual PatchFunction* copyFunction(PatchFunction*
    func, PatchObject* obj);

    virtual PatchBlock* makeBlock(ParseAPI::Block* blk, PatchObject*
    obj); virtual PatchBlock* copyBlock(PatchBlock* blk, PatchObject*
    obj);

    virtual PatchEdge* makeEdge(ParseAPI::Edge* edge, PatchBlock* src,
    PatchBlock* trg, PatchObject* obj); virtual PatchEdge*
    copyEdge(PatchEdge* edge, PatchObject* obj);

Programmers implement the above virtual methods to instantiate a CFG
structure (either a PatchFunction, a PatchBlock, or a PatchEdge) or to
copy (e.g., when forking a new process).