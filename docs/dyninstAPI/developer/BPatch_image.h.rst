.. _`sec-dev:BPatch_image.h`:

BPatch_image.h
##############

.. cpp:namespace:: dev

.. cpp:class:: BPatch_image

  .. cpp:function:: BPatch_image(BPatch_addressSpace *addSpace)
  .. cpp:function:: BPatch_image()
  .. cpp:function:: BPatch_module *findModule(mapped_module *base)
  .. cpp:function:: BPatch_object *findObject(mapped_object *base)
  .. cpp:function:: virtual ~BPatch_image()
  .. cpp:function:: void getNewCodeRegions(std::vector<BPatch_function*>& newFuncs, std::vector<BPatch_function*>&modFuncs)
  .. cpp:function:: void clearNewCodeRegions()
