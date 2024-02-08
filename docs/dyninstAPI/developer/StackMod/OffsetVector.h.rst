.. _`sec:OffsetVector.h`:

OffsetVector.h
##############

.. cpp:class:: OffsetVector

  .. cpp:type:: Dyninst::StackAnalysis::Height K
  .. cpp:type:: Dyninst::StackLocation* V
  .. cpp:function:: OffsetVector()
  .. cpp:function:: Dyninst::IntervalTree<K,V> stack()
  .. cpp:function:: std::map<Dyninst::MachRegister, Dyninst::IntervalTree<K,V> > definedRegs()
  .. cpp:function:: void insert(K lb, K ub, V v, bool isRegHeight)
  .. cpp:function:: void update(K lb, K newUB)
  .. cpp:function:: void update(K lb, V newVal)
  .. cpp:function:: bool erase(K key, bool stack=true)
  .. cpp:function:: bool erase(MachRegister r, V val)
  .. cpp:function:: bool find(K key, V& val) const
  .. cpp:function:: bool find(K key, MachRegister reg, V& val) const
  .. cpp:function:: bool find(K key, K& lb, K& ub, V& val) const
  .. cpp:function:: void addSkip(Dyninst::MachRegister, Dyninst::StackAnalysis::Height, Address, Address)
  .. cpp:function:: bool isSkip(Dyninst::MachRegister, Dyninst::StackAnalysis::Height, Address, Address) const
  .. cpp:function:: void printSkips() const
  .. cpp:function:: void print() const
  .. cpp:member:: private Dyninst::IntervalTree<K,V> _stack
  .. cpp:member:: private std::map<Dyninst::MachRegister, Dyninst::IntervalTree<K,V> > _definedRegs
  .. cpp:member:: private std::map<Dyninst::MachRegister, std::map<Dyninst::StackAnalysis::Height, std::set<std::pair<Dyninst::Address,Dyninst::Address>>*>*> _skipRegPCs
