.. _`sec-dev:PatchObject.h`:

PatchObject.h
#############

.. cpp:namespace:: Dyninst::PatchAPI::dev

.. cpp:class:: PatchObject

  .. cpp:function:: AddrSpace* addrSpace() const
  .. cpp:function:: void setAddrSpace(AddrSpace* as)
  .. cpp:function:: void addFunc(PatchFunction*)
  .. cpp:function:: void removeFunc(PatchFunction*)
  .. cpp:function:: void removeFunc(ParseAPI::Function *)
  .. cpp:function:: void addBlock(PatchBlock*)
  .. cpp:function:: void removeBlock(PatchBlock*)
  .. cpp:function:: void removeBlock(ParseAPI::Block*)
  .. cpp:function:: void addEdge(PatchEdge*)
  .. cpp:function:: void removeEdge(PatchEdge*)
  .. cpp:function:: void removeEdge(ParseAPI::Edge*)
  .. cpp:function:: bool consistency(const AddrSpace *as) const

  .. cpp:member:: protected ParseAPI::CodeObject* co_
  .. cpp:member:: protected Address codeBase_
  .. cpp:member:: protected AddrSpace* addr_space_
  .. cpp:member:: protected FuncMap funcs_
  .. cpp:member:: protected BlockMap blocks_
  .. cpp:member:: protected EdgeMap edges_
  .. cpp:member:: protected CFGMaker* cfg_maker_
  .. cpp:member:: protected PatchCallback *cb_
  .. cpp:member:: protected PatchParseCallback *pcb_

  .. cpp:function:: protected PatchObject(ParseAPI::CodeObject* o, Address a, CFGMaker* cm, PatchCallback *cb = NULL)
  .. cpp:function:: protected PatchObject(const PatchObject* par_obj, Address a, CFGMaker* cm, PatchCallback *cb = NULL)
  .. cpp:function:: protected void copyCFG(PatchObject* par_obj)
  .. cpp:function:: protected bool splitBlock(PatchBlock *first, ParseAPI::Block *second)
  .. cpp:function:: protected void createFuncs()
  .. cpp:function:: protected void createBlocks()
  .. cpp:function:: protected void createEdges()
