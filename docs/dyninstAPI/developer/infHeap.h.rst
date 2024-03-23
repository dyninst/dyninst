.. _`sec:infHeap.h`:

infHeap.h
#########

.. cpp:class:: inferiorHeap

  .. cpp:function:: void clear()
  .. cpp:function:: inferiorHeap()
  .. cpp:function:: inferiorHeap(const inferiorHeap &src)
  .. cpp:function:: inferiorHeap &operator=(const inferiorHeap &src)
  .. cpp:member:: std::unordered_map<Dyninst::Address, heapItem *> heapActive

    active part of heap

  .. cpp:member:: std::vector<heapItem *> heapFree

    free block of data inferior heap

  .. cpp:member:: std::vector<disabledItem> disabledList

    items waiting to be freed.

  .. cpp:member:: int disabledListTotalMem

    total size of item waiting to free

  .. cpp:member:: int totalFreeMemAvailable

      total free memory in the heap

  .. cpp:member:: int freed

      total reclaimed(over time)

  .. cpp:member:: std::vector<heapItem *> bufferPool

      distributed heap segments -- csserra


.. cpp:class:: heapItem

  .. cpp:function:: heapItem()
  .. cpp:function:: heapItem(Dyninst::Address a, int n, inferiorHeapType t, bool d = true, heapStatus s = HEAPfree)
  .. cpp:function:: heapItem(const heapItem *h)
  .. cpp:function:: heapItem(const heapItem &h)
  .. cpp:function:: heapItem &operator=(const heapItem &src)
  .. cpp:function:: void setBuffer(void *b)
  .. cpp:member:: Dyninst::Address addr
  .. cpp:member:: unsigned length
  .. cpp:member:: inferiorHeapType type
  .. cpp:member:: bool dynamic

    part of a dynamically allocated segment?

  .. cpp:member:: heapStatus status
  .. cpp:member:: void *buffer

    For local...


.. cpp:class:: disabledItem

  disabledItem: an item on the heap that we are trying to free.
  "pointsToCheck" corresponds to predecessor code blocks (i.e. prior minitrampbasetramp code)

  .. cpp:function:: disabledItem() noexcept
  .. cpp:function:: disabledItem(heapItem *h, const std::vector<addrVecType> &preds)
  .. cpp:function:: disabledItem(const disabledItem &src)
  .. cpp:function:: disabledItem &operator=(const disabledItem &src)
  .. cpp:function:: ~disabledItem()
  .. cpp:member:: heapItem block

    inferior heap block

  .. cpp:member:: std::vector<addrVecType> pointsToCheck

    list of addresses to check against PCs

  .. cpp:function:: Dyninst::Address getPointer() const
  .. cpp:function:: inferiorHeapType getHeapType() const
  .. cpp:function:: const std::vector<addrVecType> &getPointsToCheck() const
  .. cpp:function:: std::vector<addrVecType> &getPointsToCheck()

.. cpp:class:: heapDescriptor

  Dyninst heap class This needs a better name. Contains a name, and address, and a size. Any ideas?

  .. cpp:function:: heapDescriptor(const std::string name, Dyninst::Address addr, unsigned int size, const inferiorHeapType type)
  .. cpp:function:: heapDescriptor()
  .. cpp:function:: const std::string &name() const
  .. cpp:function:: const Dyninst::Address &addr() const
  .. cpp:function:: const unsigned &size() const
  .. cpp:function:: const inferiorHeapType &type() const
  .. cpp:member:: private std::string name_
  .. cpp:member:: private Dyninst::Address addr_
  .. cpp:member:: private unsigned size_
  .. cpp:member:: private inferiorHeapType type_


.. cpp:enum:: heapStatus

  .. cpp:enumerator:: HEAPfree
  .. cpp:enumerator:: HEAPallocated

.. cpp:enum:: inferiorHeapType

  Bit pattern

  .. cpp:enumerator:: textHeap=0x01
  .. cpp:enumerator:: dataHeap=0x02
  .. cpp:enumerator:: uncopiedHeap=0x04

    not copied on fork

  .. cpp:enumerator:: anyHeap=0x7

    OR of the previous three

  .. cpp:enumerator:: lowmemHeap=0x1000


.. cpp:type:: std::vector<Dyninst::Address> addrVecType

