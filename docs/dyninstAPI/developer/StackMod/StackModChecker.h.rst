.. _`sec:StackModChecker.h`:

StackModChecker.h
#################

.. cpp:class:: StackModChecker

  .. cpp:function:: StackModChecker(BPatch_function* b, func_instance* f)
  .. cpp:function:: ~StackModChecker()
  .. cpp:function:: bool addModsInternal(std::set<StackMod*>)
  .. cpp:member:: private BPatch_function* bfunc
  .. cpp:member:: private func_instance* func
  .. cpp:type:: private map<int, pair<int, StackMod::MType> > rangeMap
  .. cpp:type:: private rangeMap::iterator rangeIterator
  .. cpp:function:: private std::string getName() const

    Returns the name of the function we're checking

  .. cpp:function:: private bool accumulateStackRanges(StackMod* m)

    Accumulates the effects of the current stack modification on the _stackRanges; this gets used to
    determine if additional modification need to be added to preserve alignment requirements on the current platform.

  ......

  .. warning::
    The methods alignStackRanges and findContiguousRange function are fundamentally broken and need to be rewritten.
    There is much wrong here, but it may work in limited circumstances.
 
  .. cpp:function:: private bool alignStackRanges(int alignment, StackMod::MOrder order, std::vector<StackMod*>& mods)

    Conform to alignment requirements of the current platform

  .. cpp:function:: private void findContiguousRange(rangeIterator iter, rangeIterator& end)

  ......

  .. cpp:function:: private bool checkStackGrowth(Dyninst::StackAnalysis& sa)

    This function determines whether the stack frame grows and shrinks multiple
    times throughout the function.  Currently, we do not allow stack
    modifications (INSERT, REMOVE, MOVE) for functions where this occurs,
    because the stack offsets specified by the modifications may be valid for
    multiple address ranges in the function.  In the future, stack modifications
    could be extended to handle these functions.

  .. cpp:function:: private void processBlock(Dyninst::StackAnalysis& sa, Dyninst::ParseAPI::Block* block)

    Cache all SP heights for each block in the function

  .. cpp:function:: private bool checkAllPaths(Dyninst::StackAnalysis& sa)

    Check the stack growth for all paths in the function

  .. cpp:function:: private bool checkAllPathsInternal(Dyninst::ParseAPI::Block* block, std::set<Dyninst::ParseAPI::Block*>& state,\
                                                       std::vector<Dyninst::ParseAPI::Block*>& path,\
                                                       std::set<Dyninst::ParseAPI::Block*>& exitBlocks, Dyninst::StackAnalysis& sa)

    Recursively build all control flow paths through the function; when the end of path is found (at an exit block), check for safe
    stack growth on that path.

  .. cpp:function:: private bool checkPath(std::vector<Dyninst::ParseAPI::Block*>& path)

    Check for unsafe stack growth on the current path

  .. cpp:function:: private bool findInsertOrRemovePoints(Dyninst::StackAnalysis& sa, StackMod* m, std::vector<BPatch_point*>*& points, long& dispFromRSP)
  .. cpp:function:: private bool checkInsn(Dyninst::ParseAPI::Block* block, Dyninst::Offset off, int loc, Dyninst::StackAnalysis& sa, BPatch_point*& point, long& dispFromRSP)
  .. cpp:function:: private bool findCanaryPoints(std::vector<BPatch_point*>* insertPoints, std::vector<BPatch_point*>* checkPoints)

    Locate the libc-provided canary failure function __stack_chk_fail

  .. cpp:function:: private bool areModificationsSafe()
  .. cpp:function:: private bool isAccessSafe(InstructionAPI::Instruction insn, StackAccess *access)

    Check if a particular stack access in instruction insn is safe, given the current set of
    stack modifications. Accesses are unsafe if a stack modification perturbs the access range,
    which can happen by:

      - inserting into the accessed range,
      - removing any part of the accessed range, or
      - moving a portion (but not the whole) accessed range.

  .. cpp:member:: private bool _unsafeStackGrowth
  .. cpp:member:: private std::map<Dyninst::ParseAPI::Block*, std::vector<Dyninst::StackAnalysis::Height>* > blockHeights
  .. cpp:member:: private rangeMap _stackRanges
  .. cpp:member:: private rangeMap _stackRangesCleanup
