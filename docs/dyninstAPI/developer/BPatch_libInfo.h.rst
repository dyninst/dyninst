.. _`sec:BPatch_libInfo.h`:

BPatch_libInfo.h
################


.. cpp:class:: BPatch_libInfo

  .. cpp:member:: std::unordered_map<int, BPatch_process *> procsByPid
  .. cpp:function:: BPatch_libInfo()
  .. cpp:function:: bool registerMonitoredPoint(BPatch_point *point)
  .. cpp:function:: BPatch_point *getMonitoredPoint(Dyninst::Address addr)
  .. cpp:function:: int getStopThreadCallbackID(Dyninst::Address cb)
  .. cpp:member:: protected int stopThreadIDCounter_
  .. cpp:member:: protected std::unordered_map<Dyninst::Address, unsigned> stopThreadCallbacks_
  .. cpp:member:: protected std::unordered_map<Dyninst::Address, BPatch_point *> monitoredPoints_
