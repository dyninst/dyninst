.. _`sec:CodeMover.h`:

CodeMover.h
###########

.. cpp:namespace:: Dyninst::Relocation

.. cpp:type:: std::map<std::pair<block_instance *, func_instance *>, Priority> PriorityMap

.. cpp:class:: CodeMover

  .. cpp:type:: boost::shared_ptr<CodeMover> Ptr
  .. cpp:type:: std::set<func_instance *> FuncSet
  .. cpp:type:: std::set<block_instance *> BlockSet
  .. cpp:function:: static Ptr create(CodeTracker*)

    A generic mover of code an instruction, a basic block, or a function. This is the algorithm (fixpoint)
    counterpart of the classes described in relocation.h Input:   A structured description of code in terms
    of an instruction, a    block, a function, or a set of functions  A starting address for the moved code

    Output: a buffer containing the moved code  We take a CodeTracker as a reference parameter so that we
    don't have to copy it on output CodeMovers are designed to be discarded while CodeTrackers survive.

  .. cpp:function:: ~CodeMover()
  .. cpp:function:: bool addFunctions(FuncSet::const_iterator begin, FuncSet::const_iterator end)
  .. cpp:function:: bool transform(Transformer &t)

    Apply the given Transformer to all blocks in the Mover

  .. cpp:function:: bool initialize(const codeGen &genTemplate)

    Does all the once-only work to generate code.

  .. cpp:function:: bool relocate(Address addr)

    Allocates an internal buffer and relocates the code provided to the constructor.

    Returns true for success or false for catastrophic failure. The codeGen parameter allows specification of
    various codeGen-carried information

    We wish to minimize the space required by the relocated code. Since some platforms
    may have varying space requirements for certain instructions (e.g., branches) this
    requires a fixpoint calculation. We start with the original size and increase from
    there.

    Reasons for size increase:

      1) Instrumentation. It tends to use room. Odd.
      2) Transformed instructions. We may need to replace a single instruction with a
         sequence to emulate its original behavior
      3) Variable-sized instructions. If we increase branch displacements we may need
         to increase the corresponding branch instruction sizes.

  .. cpp:function:: bool finalize()
  .. cpp:function:: void disassemble() const
  .. cpp:function:: void extractDefensivePads(AddressSpace *)
  .. cpp:type:: std::map<Address, Address> EntryMap

    Get a map from original addresses to new addresses for all blocks

  .. cpp:function:: const EntryMap &entryMap()
  .. cpp:function:: SpringboardMap &sBoardMap(AddressSpace *as)

    Take the current PriorityMap, digest it, and return a sorted list of where we need patches (from and to).

    Not const so we can add others to it.

  .. cpp:function:: PriorityMap &priorityMap()

    Not const so that Transformers can modify it...

  .. cpp:function:: unsigned size() const

    Get either an estimate (pre-relocation) or actual size

  .. cpp:function:: void *ptr() const
  .. cpp:function:: std::string format() const
  .. cpp:function:: codeGen &gen()
  .. cpp:function:: private CodeMover(CodeTracker *t)
  .. cpp:function:: private void setAddr(Address &addr)

  .. cpp:function:: private template <typename RelocBlockIter> bool addRelocBlocks(RelocBlockIter begin,\
                                                                                   RelocBlockIter end, func_instance *f)

  .. cpp:function:: private bool addRelocBlock(block_instance *block, func_instance *f)
  .. cpp:function:: private void finalizeRelocBlocks()
  .. cpp:member:: private RelocGraph *cfg_
  .. cpp:member:: private Address addr_
  .. cpp:member:: private EntryMap entryMap_
  .. cpp:member:: private PriorityMap priorityMap_
  .. cpp:member:: private SpringboardMap sboardMap_
  .. cpp:member:: private CodeTracker *tracker_
  .. cpp:member:: private CodeBuffer buffer_
  .. cpp:member:: private bool finalized_
