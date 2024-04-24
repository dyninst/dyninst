.. _`sec:DynObject.h`:

DynObject.h
###########

.. cpp:namespace:: Dyninst::PatchAPI

.. cpp:class:: DynObject : public PatchObject

  .. cpp:function:: static DynObject* create(ParseAPI::CodeObject* co, AddressSpace* as, Address base)
  .. cpp:function:: DynObject(ParseAPI::CodeObject* co, AddressSpace* as, Address base)
  .. cpp:function:: DynObject(const DynObject *par_obj, AddressSpace* child, Address base)
  .. cpp:function:: virtual ~DynObject()
  .. cpp:function:: AddressSpace* as() const
  .. cpp:member:: private AddressSpace* as_

.. cpp:class:: DynCFGMaker : public Dyninst::PatchAPI::CFGMaker

  .. cpp:function:: DynCFGMaker()
  .. cpp:function:: virtual ~DynCFGMaker()
  .. cpp:function:: virtual PatchFunction* makeFunction(ParseAPI::Function*, PatchObject*)
  .. cpp:function:: virtual PatchFunction* copyFunction(PatchFunction*, PatchObject*)
  .. cpp:function:: virtual PatchBlock* makeBlock(ParseAPI::Block*, PatchObject*)
  .. cpp:function:: virtual PatchBlock* copyBlock(PatchBlock*, PatchObject*)
  .. cpp:function:: virtual PatchEdge* makeEdge(ParseAPI::Edge*, PatchBlock*, PatchBlock*, PatchObject*)
  .. cpp:function:: virtual PatchEdge* copyEdge(PatchEdge*, PatchObject*)

.. cpp:type:: boost::shared_ptr<DynCFGMaker> DynCFGMakerPtr
