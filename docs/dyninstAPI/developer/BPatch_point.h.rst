.. _`sec-dev:BPatch_point.h`:

BPatch_point.h
##############

.. cpp:namespace:: dev

.. cpp:class:: BPatch_point

  .. cpp:member:: private instPoint *point

    We have a disconnect between how BPatch represents a point (e.g., an instruction) and
    how the internals represent a point (e.g., pre-instruction). We handle this here with
    a secondary instPoint that is defined to be the "after" equivalent.

  .. cpp:function:: private void overrideType(BPatch_procedureLocation loc)

    We often create a point with the arbitrary point type, and later need to override
    it to a specific type (e.g., loop entry)

  .. cpp:member:: private Dyninst::PatchAPI::InstancePtr dynamic_point_monitor_func

    a snippet used in monitoring of dynamic calls maybe we want BPatchSnippetHandle here

  .. cpp:function:: BPatch_edge *edge() const

    Hack to get edge information. DO NOT USE.

  .. cpp:function:: bool isReturnInstruction()
  .. cpp:function:: static BPatch_procedureLocation convertInstPointType_t(int intType)
  .. cpp:function:: instPoint *llpoint()
  .. cpp:function:: Dyninst::Address getCallFallThroughAddr()
  .. cpp:function:: bool patchPostCallArea()

  .. cpp:function:: BPatch_addressSpace * getAddressSpace()

    Added for DynC...
