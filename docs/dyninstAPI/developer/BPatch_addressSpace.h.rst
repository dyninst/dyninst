.. _`sec-dev:BPatch_addressSpace.h`:

BPatch_addressSpace.h
#####################

.. cpp:namespace:: dev

.. cpp:class:: BPatch_addressSpace

  .. cpp:function:: BPatch_module* findModuleByAddr(Dyninst::Address addr)

      Doesn't cause parsing

  .. cpp:function:: bool findFuncsByRange(Dyninst::Address startAddr, Dyninst::Address endAddr, \
                                          std::set<BPatch_function*>& funcs)
