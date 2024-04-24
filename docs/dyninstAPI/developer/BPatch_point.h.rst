.. _`sec-dev:BPatch_point.h`:

BPatch_point.h
##############

.. cpp:namespace:: dev

.. cpp:class:: BPatch_point

  .. cpp:member:: private instPoint *point

    We have a disconnect between how BPatch represents a point (e.g., an instruction) and
    how the internals represent a point (e.g., pre-instruction). We handle this here with
    a secondary instPoint that is defined to be the "after" equivalent.

  .. cpp:member:: private Dyninst::PatchAPI::InstancePtr dynamic_point_monitor_func

    a snippet used in monitoring of dynamic calls maybe we want BPatchSnippetHandle here

  .. cpp:member:: private BPatch_addressSpace *addSpace
  .. cpp:member:: private AddressSpace *lladdSpace
  .. cpp:member:: private BPatch_function *func
  .. cpp:member:: private BPatch_basicBlockLoop *loop
  .. cpp:member:: private instPoint *secondaryPoint
  .. cpp:member:: private BPatch_procedureLocation pointType
  .. cpp:member:: private BPatch_memoryAccess *memacc
  .. cpp:member:: private BPatch_edge *edge_
  .. cpp:member:: private BPatch_Vector<BPatchSnippetHandle *> preSnippets
  .. cpp:member:: private BPatch_Vector<BPatchSnippetHandle *> postSnippets
  .. cpp:member:: private BPatch_Vector<BPatchSnippetHandle *> allSnippets

  .. cpp:function:: BPatch_edge *edge() const

    Hack to get edge information. DO NOT USE.

  .. cpp:function:: bool isReturnInstruction()
  .. cpp:function:: static BPatch_procedureLocation convertInstPointType_t(int intType)
  .. cpp:function:: instPoint *llpoint()
  .. cpp:function:: Dyninst::Address getCallFallThroughAddr()
  .. cpp:function:: bool patchPostCallArea()

  .. cpp:function:: BPatch_addressSpace * getAddressSpace()

    Added for DynC...

  .. cpp:function:: int getDisplacedInstructions(int maxSize, void *insns)

    Does nothing.

  .. cpp:function:: private BPatch_point(BPatch_addressSpace *_addSpace, BPatch_function *_func, instPoint *_point, instPoint *_secondary, BPatch_procedureLocation _pointType, AddressSpace *as)
  .. cpp:function:: private BPatch_point(BPatch_addressSpace *_addSpace, BPatch_function *_func, BPatch_edge *_edge, instPoint *_point, AddressSpace *as)

  .. cpp:function:: private void setLoop(BPatch_basicBlockLoop *l)

    For a ``BPatch_point`` representing a loop instrumentation site, set the loop that it represents.

    We currently can use a single BPatch_point to represent multiple loops. This is a problem, since we
    would really like the points to label a unique loop. On the other hand, then multiple points would
    share the same physical address.. not good.

  .. cpp:function:: private void overrideType(BPatch_procedureLocation loc)

    We often create a point with the arbitrary point type, and later need to override
    it to a specific type (e.g., loop entry)


  .. cpp:function:: private instPoint *getPoint() const
  .. cpp:function:: private instPoint *getPoint(BPatch_callWhen when) const
  .. cpp:function:: private void recordSnippet(BPatch_callWhen, BPatch_snippetOrder, BPatchSnippetHandle *)
  .. cpp:function:: private void attachMemAcc(BPatch_memoryAccess *memacc)
  .. cpp:function:: private AddressSpace *getAS()

  .. cpp:function:: private bool removeSnippet(BPatchSnippetHandle *handle)

    Removes snippet from data structures, doesn't remove the instrumentation.

    It is invoked by :cpp:func:`BPatch_addressSpace::deleteSnippet`.
