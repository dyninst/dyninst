.. _`sec:Springboard.h`:

Springboard.h
#############

Build the branches from previous versions of moved code to the new versions.

.. cpp:namespace:: Dyninst::Relocation

.. cpp:struct:: SpringboardReq

  .. cpp:type:: std::map<func_instance *, Address> Destinations
  .. cpp:member:: Address from
  .. cpp:member:: Priority priority
  .. cpp:member:: func_instance *func
  .. cpp:member:: block_instance *block
  .. cpp:member:: Destinations destinations
  .. cpp:member:: bool checkConflicts
  .. cpp:member:: bool includeRelocatedCopies
  .. cpp:member:: bool fromRelocatedCode
  .. cpp:member:: bool useTrap
  .. cpp:function:: SpringboardReq(const Address from_, const Address to_, const Priority priority_, func_instance *func_, block_instance *block_, bool checkConflicts_, bool includeRelocCopies_, bool fromRelocCode_, bool useTrap_)
  .. cpp:function:: SpringboardReq()
  .. cpp:function:: void addReq(const Address from_, const Address to_, const Priority priority_, func_instance *func_, block_instance *block_, bool checkConflicts_, bool includeRelocCopies_, bool fromRelocCode_, bool useTrap_)

      This mechanism handles overlapping functions, where
      we might see springboards from the same address to
      different targets. In this case only one can win,
      but we want to track the different bbls so that
      we can do the right thing with includeRelocatedCopies.


.. cpp:class:: SpringboardMap

  .. cpp:type:: std::map<Address, SpringboardReq> SpringboardsAtPriority
  .. cpp:type:: std::map<Priority, SpringboardsAtPriority> Springboards
  .. cpp:type:: SpringboardsAtPriority::iterator iterator
  .. cpp:type:: SpringboardsAtPriority::const_iterator const_iterator
  .. cpp:type:: SpringboardsAtPriority::reverse_iterator reverse_iterator
  .. cpp:function:: bool empty() const
  .. cpp:function:: void addFromOrigCode(Address from, Address to, Priority p, func_instance *func, block_instance *bbl)
  .. cpp:function:: void addFromRelocatedCode(Address from, Address to, Priority p)
  .. cpp:function:: void addRaw(Address from, Address to, Priority p, func_instance *func, block_instance *bbl, bool checkConflicts, bool includeRelocatedCopies, bool fromRelocatedCode, bool useTrap)
  .. cpp:function:: iterator begin(Priority p)
  .. cpp:function:: iterator end(Priority p)
  .. cpp:function:: reverse_iterator rbegin(Priority p)
  .. cpp:function:: reverse_iterator rend(Priority p)
  .. cpp:member:: private Springboards sBoardMap_


.. cpp:struct:: SpringboardInfo

  Persistent tracking of things that have already gotten springboards across multiple calls to
  :cpp:func:`AddressSpace::relocateInt()` and thus across multiple SpringboardFoo, CodeTracker, CodeMover objects.

  .. cpp:member:: int val
  .. cpp:member:: func_instance *func
  .. cpp:member:: Priority priority
  .. cpp:function:: SpringboardInfo(int v, func_instance* f)
  .. cpp:function:: SpringboardInfo(int v, func_instance* f, Priority p)

.. cpp:class:: InstalledSpringboards

  .. cpp:type:: boost::shared_ptr<InstalledSpringboards> Ptr
  .. cpp:member:: static const int Allocated
  .. cpp:member:: static const int UnallocatedStart
  .. cpp:function:: InstalledSpringboards()
  .. cpp:function:: template <typename BlockIter> bool addBlocks(func_instance* func, BlockIter begin, BlockIter end)
  .. cpp:function:: bool addFunc(func_instance* f)
  .. cpp:function:: bool conflict(Address start, Address end, bool inRelocatedCode, func_instance* func, Priority p)
  .. cpp:function:: bool conflictInRelocated(Address start, Address end)
  .. cpp:function:: void registerBranch(Address start, Address end, const SpringboardReq::Destinations &dest, bool inRelocatedCode, func_instance* func, Priority p)
  .. cpp:function:: void registerBranchInRelocated(Address start, Address end, func_instance* func, Priority p)
  .. cpp:function:: bool forceTrap(Address a)
  .. cpp:member:: private std::set<Address> relocTraps_

    tracks relocation addresses that need trap-based springboards

  .. cpp:member:: private IntervalTree<Address, SpringboardInfo*> validRanges_

    We don't really care about the payload I just want an "easy to look up" range data structure.  Map this to an int because IntervalTree collapses similar ranges. Punks.

  .. cpp:member:: private IntervalTree<Address, SpringboardInfo*> paddingRanges_

    If we consume NOP-padding between functions to get room for a jump, that padding may not exist in the relocation buffer.  Remember such ranges so we can deal with that in reinstrumentation, if only to force a trap.

  .. cpp:member:: private IntervalTree<Address, SpringboardInfo*> overwrittenRelocatedCode_

    Like the previous, but for branches we put in relocated code. We assume anything marked as "in relocated code" is a valid thing to write to, since relocation size is >= original size. However, we still don't want overlapping branches.

  .. cpp:function:: private void debugRanges()

.. cpp:class:: SpringboardBuilder

  .. cpp:type:: boost::shared_ptr<SpringboardBuilder> Ptr
  .. cpp:type:: std::set<func_instance *> FuncSet
  .. cpp:function:: static Ptr createFunc(FuncSet::const_iterator begin, FuncSet::const_iterator end, AddressSpace *addrSpace)
  .. cpp:function:: bool generate(std::list<codeGen> &springboards, SpringboardMap &input)
  .. cpp:function:: private SpringboardBuilder(AddressSpace *a)
  .. cpp:function:: private bool generateInt(std::list<codeGen> &springboards, SpringboardMap &input, Priority p)
  .. cpp:function:: private generateResult_t generateSpringboard(std::list<codeGen> &gens, const SpringboardReq &p)
  .. cpp:function:: private bool generateMultiSpringboard(std::list<codeGen> &input, const SpringboardReq &p)
  .. cpp:function:: private bool createRelocSpringboards(const SpringboardReq &r, bool useTrap, SpringboardMap &input)

    Find all previous instrumentations and also overwrite them.

  .. cpp:function:: private bool generateReplacements(std::list<codeGen> &input, const SpringboardReq &p, bool useTrap)
  .. cpp:function:: private void addMultiNeeded(const SpringboardReq &p)
  .. cpp:function:: private void generateBranch(Address from, Address to, codeGen &input)
  .. cpp:function:: private void generateTrap(Address from, Address to, codeGen &input)
  .. cpp:function:: private bool conflict(Address start, Address end, bool inRelocatedCode, func_instance* func, Priority p)
  .. cpp:function:: private void registerBranch(Address start, Address end, const SpringboardReq::Destinations &dest, bool inRelocatedCode, func_instance* func, Priority p)
  .. cpp:function:: private bool isLegalShortBranch(Address from, Address to)
  .. cpp:function:: private Address shortBranchBack(Address from)
  .. cpp:member:: private AddressSpace *addrSpace_
  .. cpp:member:: private InstalledSpringboards::Ptr installed_springboards_
  .. cpp:member:: private std::list<SpringboardReq> multis_


.. cpp:enum:: SpringboardBuilder::generateResult_t

  .. cpp:enumerator:: Failed
  .. cpp:enumerator:: MultiNeeded
  .. cpp:enumerator:: Succeeded


.. cpp:enum:: Priority

  .. cpp:enumerator:: MIN_PRIORITY
  .. cpp:enumerator:: RELOC_MIN_PRIORITY
  .. cpp:enumerator:: RelocNotRequired
  .. cpp:enumerator:: RelocSuggested
  .. cpp:enumerator:: RelocRequired
  .. cpp:enumerator:: RelocOffLimits
  .. cpp:enumerator:: RELOC_MAX_PRIORITY
  .. cpp:enumerator:: ORIG_MIN_PRIORITY
  .. cpp:enumerator:: NotRequired

  .. cpp:enumerator:: Suggested

    Currently we put suggested springboards at non-func-entry, non-indirect-jump-target
    block entry. In case the jump table analysis under-approximate the jump targets
    (unlikely), the control flow will goes back to instrumentation at other blocks

  .. cpp:enumerator:: IndirBlockEntry

    Indirect jump target block is very important, but is less important than func entry.
    Control flow can escape instrumentation by indirect jump (jump tables).
    So, we install springboards at all indirect jump targets.
    However, jump table analysis can overapproximate jump targets and
    the bogus jump targets can be function entries. So, we put indirect
    jump target as one priority lower than function entry

  .. cpp:enumerator:: FuncEntry

    FuncEntry represents springboards at function entries.
    This is the highest priority because control flow enters instrumentation at function entry

  .. cpp:enumerator:: ORIG_MAX_PRIORITY
  .. cpp:enumerator:: MAX_PRIORITY
