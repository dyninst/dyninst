.. _`sec:CodeTracker.h`:

CodeTracker.h
#############

Keeps a list of relocated Widgets (compressed where possible) for mapping
addresses and types between original and relocated code.

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: CodeTracker

  .. cpp:type:: Address FunctionEntryID

    I'd like to use a block  as a unique key element, but really can't because blocks can be deleted and recreated.
    Instead, I'm using their entry address - that's the address in BlockForwardsMap.  The ForwardsMap address is a
    straightforward "what's the data at this particular address".

  .. cpp:type:: Address BlockEntryID
  .. cpp:type:: std::list<TrackerElement *> TrackerList
  .. cpp:type:: std::map<Address, RelocatedElements> FwdMapInner
  .. cpp:type:: std::map<FunctionEntryID, FwdMapInner> FwdMapMiddle
  .. cpp:type:: std::map<BlockEntryID, FwdMapMiddle> ForwardMap
  .. cpp:type:: class IntervalTree<Address, TrackerElement *> ReverseMap
  .. cpp:function:: CodeTracker()
  .. cpp:function:: ~CodeTracker()
  .. cpp:function:: static CodeTracker *fork(CodeTracker *parent, AddressSpace *child)
  .. cpp:function:: bool origToReloc(Address origAddr, block_instance *block, func_instance *func, RelocatedElements &relocs) const
  .. cpp:function:: bool relocToOrig(Address relocAddr, RelocInfo &ri) const
  .. cpp:function:: TrackerElement *findByReloc(Address relocAddr) const
  .. cpp:function:: Address lowOrigAddr() const
  .. cpp:function:: Address highOrigAddr() const
  .. cpp:function:: Address lowRelocAddr() const
  .. cpp:function:: Address highRelocAddr() const
  .. cpp:function:: void addTracker(TrackerElement *)
  .. cpp:function:: void createIndices()
  .. cpp:function:: void debug()
  .. cpp:function:: const TrackerList &trackers()
  .. cpp:member:: private ForwardMap origToReloc_

    We make this block specific to handle shared code

  .. cpp:member:: private ReverseMap relocToOrig_
  .. cpp:member:: private TrackerList trackers_


.. cpp:struct:: CodeTracker::RelocatedElements

  .. cpp:member:: Address instruction
  .. cpp:member:: std::map<instPoint*, Address> instrumentation
  .. cpp:member:: Address pad
  .. cpp:function:: RelocatedElements()


.. cpp:struct:: CodeTracker::RelocInfo

  .. cpp:member:: Address orig
  .. cpp:member:: Address reloc
  .. cpp:member:: block_instance *block
  .. cpp:member:: func_instance *func
  .. cpp:member:: baseTramp *bt
  .. cpp:member:: unsigned pad
  .. cpp:function:: RelocInfo()


.. cpp:class:: TrackerElement

  .. cpp:function:: TrackerElement(Address o, block_instance *b, func_instance *f)
  .. cpp:function:: virtual ~TrackerElement()
  .. cpp:function:: virtual Address relocToOrig(Address reloc) const = 0
  .. cpp:function:: virtual Address origToReloc(Address orig) const = 0
  .. cpp:function:: virtual type_t type() const = 0
  .. cpp:function:: Address orig() const
  .. cpp:function:: Address reloc() const
  .. cpp:function:: unsigned size() const
  .. cpp:function:: block_instance *block() const
  .. cpp:function:: func_instance *func() const
  .. cpp:function:: void setReloc(Address reloc)
  .. cpp:function:: void setSize(unsigned size)
  .. cpp:function:: virtual bool mergeable() const
  .. cpp:function:: protected TrackerElement()
  .. cpp:function:: protected TrackerElement(const TrackerElement &)
  .. cpp:member:: protected Address orig_
  .. cpp:member:: protected Address reloc_
  .. cpp:member:: protected unsigned size_
  .. cpp:member:: protected block_instance *block_
  .. cpp:member:: protected func_instance *func_


.. cpp:enum:: TrackerElement::type_t

  .. cpp:enumerator:: original
  .. cpp:enumerator:: emulated
  .. cpp:enumerator:: instrumentation
  .. cpp:enumerator:: padding


.. cpp:function:: std::ostream & operator<<(std::ostream &os, const Dyninst::Relocation::TrackerElement &e)


.. cpp:class:: OriginalTracker : public TrackerElement

  .. cpp:function:: OriginalTracker(Address orig, block_instance *b, func_instance *f)
  .. cpp:function:: virtual ~OriginalTracker()
  .. cpp:function:: virtual Address relocToOrig(Address reloc) const
  .. cpp:function:: virtual Address origToReloc(Address orig) const
  .. cpp:function:: virtual type_t type() const


.. cpp:class:: EmulatorTracker : public TrackerElement

  .. cpp:function:: EmulatorTracker(Address orig, block_instance *b, func_instance *f)
  .. cpp:function:: virtual ~EmulatorTracker()
  .. cpp:function:: virtual Address relocToOrig(Address reloc) const
  .. cpp:function:: virtual Address origToReloc(Address orig) const
  .. cpp:function:: virtual type_t type() const


.. cpp:class:: InstTracker : public TrackerElement

  .. cpp:function:: InstTracker(Address orig, baseTramp *baseT, block_instance *b, func_instance *f)
  .. cpp:function:: virtual ~InstTracker()
  .. cpp:function:: virtual Address relocToOrig(Address reloc) const
  .. cpp:function:: virtual Address origToReloc(Address orig) const
  .. cpp:function:: virtual type_t type() const
  .. cpp:function:: baseTramp *baseT() const
  .. cpp:function:: virtual bool mergeable() const
  .. cpp:member:: private baseTramp *baseT_


.. cpp:class:: PaddingTracker : public TrackerElement

  .. cpp:function:: PaddingTracker(Address orig, unsigned pad, block_instance *b, func_instance *f)
  .. cpp:function:: virtual ~PaddingTracker()
  .. cpp:function:: virtual Address relocToOrig(Address reloc) const
  .. cpp:function:: virtual Address origToReloc(Address orig) const
  .. cpp:function:: virtual type_t type() const
  .. cpp:function:: unsigned pad() const
  .. cpp:function:: virtual bool mergeable() const
  .. cpp:member:: private unsigned pad_


