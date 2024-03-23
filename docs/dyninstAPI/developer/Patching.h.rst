.. _`sec:Patching.h`:

Patching.h
##########


.. cpp:class:: DynPatchCallback : public Dyninst::PatchAPI::PatchCallback

  .. cpp:function:: DynPatchCallback()
  .. cpp:function:: ~DynPatchCallback()
  .. cpp:function:: protected virtual void split_block_cb(Dyninst::PatchAPI::PatchBlock *, Dyninst::PatchAPI::PatchBlock *)
  .. cpp:function:: protected virtual void add_block_cb(Dyninst::PatchAPI::PatchFunction *, Dyninst::PatchAPI::PatchBlock *)
  .. cpp:function:: protected virtual void destroy_cb(Dyninst::PatchAPI::Point *)

    really remove, not destroy

  .. cpp:function:: protected virtual void destroy_cb(Dyninst::PatchAPI::PatchBlock *)
  .. cpp:function:: protected virtual void destroy_cb(Dyninst::PatchAPI::PatchEdge *, Dyninst::PatchAPI::PatchObject *owner)
  .. cpp:function:: protected virtual void destroy_cb(Dyninst::PatchAPI::PatchFunction *)
  .. cpp:function:: protected virtual void destroy_cb(Dyninst::PatchAPI::PatchObject *)

    .. caution:: Not implemented

  .. cpp:function:: protected virtual void remove_block_cb(Dyninst::PatchAPI::PatchFunction *, Dyninst::PatchAPI::PatchBlock *)
