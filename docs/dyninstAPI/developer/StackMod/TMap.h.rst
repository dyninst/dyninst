.. _`sec:TMap.h`:

TMap.h
######

.. cpp:class:: TMap

  .. cpp:function:: TMap()
  .. cpp:function:: std::pair<mapping::iterator,bool> insert(std::pair<StackLocation*,StackLocation*> p)
  .. cpp:function:: mapping::iterator find(StackLocation* l)
  .. cpp:function:: StackLocation* at(StackLocation* l)
  .. cpp:function:: mapping::iterator begin()
  .. cpp:function:: mapping::iterator end()
  .. cpp:function:: mapping::reverse_iterator rbegin()
  .. cpp:function:: mapping::reverse_iterator rend()
  .. cpp:function:: void update(StackLocation* loc, int delta)
  .. cpp:function:: void update(StackLocation* loc, Dyninst::MachRegister reg, int size)
  .. cpp:function:: StackLocation* findInMap(StackLocation* src)
  .. cpp:function:: void print() const
  .. cpp:member:: private mapping _map
