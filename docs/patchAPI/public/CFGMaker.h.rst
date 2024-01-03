.. _`sec:CFGMaker.h`:

CFGMaker.h
##########

.. cpp:namespace:: Dyninst::PatchAPI

.. cpp:class:: CFGMaker

  **A factory for constructing CFGs**

  It can make objects of type :cpp:class:`PatchFunction`, :cpp:class:`PatchBlock`,
  and :cpp:class:`PatchEdge` used by :cpp:class:`PatchObject`.

  .. cpp:function:: CFGMaker()
  .. cpp:function:: virtual PatchFunction* makeFunction(ParseAPI::Function*, PatchObject*)
  .. cpp:function:: virtual PatchFunction* copyFunction(PatchFunction*, PatchObject*)
  .. cpp:function:: virtual PatchBlock* makeBlock(ParseAPI::Block*, PatchObject*)
  .. cpp:function:: virtual PatchBlock* copyBlock(PatchBlock*, PatchObject*)
  .. cpp:function:: virtual PatchEdge* makeEdge(ParseAPI::Edge*, PatchBlock*, PatchBlock*, PatchObject*)
  .. cpp:function:: virtual PatchEdge* copyEdge(PatchEdge*, PatchObject*)
