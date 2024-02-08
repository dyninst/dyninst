.. _`sec:StackLocation.h`:

StackLocation.h
###############

.. cpp:type:: Dyninst::IntervalTree<Dyninst::Address, int> ValidPCRange

.. cpp:class:: StackLocation

  .. cpp:function:: StackLocation(Dyninst::StackAnalysis::Height o, int s, StackAccess::StackAccessType t, bool h, ValidPCRange* v = NULL)
  .. cpp:function:: StackLocation(Dyninst::StackAnalysis::Height o, StackAccess::StackAccessType t, Dyninst::MachRegister r, ValidPCRange* v = NULL)
  .. cpp:function:: StackLocation(Dyninst::MachRegister r, int s)
  .. cpp:function:: StackLocation()
  .. cpp:function:: StackAccess::StackAccessType type() const
  .. cpp:function:: void setType(StackAccess::StackAccessType t)
  .. cpp:function:: int size() const
  .. cpp:function:: void setSize(int s)
  .. cpp:function:: bool isNull() const
  .. cpp:function:: bool isStackMemory() const
  .. cpp:function:: Dyninst::StackAnalysis::Height off() const
  .. cpp:function:: void setOff(Dyninst::StackAnalysis::Height o)
  .. cpp:function:: bool isRegisterHeight() const
  .. cpp:function:: bool isRegister() const
  .. cpp:function:: Dyninst::MachRegister reg() const
  .. cpp:function:: void setReg(Dyninst::MachRegister r)
  .. cpp:function:: ValidPCRange* valid() const
  .. cpp:function:: std::string format() const
  .. cpp:function:: bool operator<(const StackLocation& rhs) const
  .. cpp:member:: private StackAccess::StackAccessType _type
  .. cpp:member:: private int _size
  .. cpp:member:: private bool _isStackMemory
  .. cpp:member:: private Dyninst::StackAnalysis::Height _off
  .. cpp:member:: private bool _isRegisterHeight{}
  .. cpp:member:: private bool _isRegister
  .. cpp:member:: private Dyninst::MachRegister _reg
  .. cpp:member:: private bool _isNull
  .. cpp:member:: private ValidPCRange* _valid


.. cpp:struct:: less_StackLocation

  .. cpp:function:: bool operator()(StackLocation* a, StackLocation* b) const

.. cpp:class:: tmpObject

  .. cpp:function:: tmpObject(long o, int s, StackAccess::StackAccessType t, ValidPCRange* v = NULL)
  .. cpp:function:: long offset() const
  .. cpp:function:: int size() const
  .. cpp:function:: StackAccess::StackAccessType type() const
  .. cpp:function:: ValidPCRange* valid() const
  .. cpp:member:: private long _offset
  .. cpp:member:: private int _size
  .. cpp:member:: private StackAccess::StackAccessType _type
  .. cpp:member:: private ValidPCRange* _valid


.. cpp:struct:: less_tmpObject

  .. cpp:function:: bool operator()(tmpObject a, tmpObject b) const
