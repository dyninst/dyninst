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
  .. cpp:function:: private bool accumulateStackRanges(StackMod* m)
  .. cpp:function:: private bool alignStackRanges(int alignment, StackMod::MOrder order, std::vector<StackMod*>& mods)
  .. cpp:function:: private void findContiguousRange(rangeIterator iter, rangeIterator& end)
  .. cpp:function:: private bool checkStackGrowth(Dyninst::StackAnalysis& sa)
  .. cpp:function:: private void processBlock(Dyninst::StackAnalysis& sa, Dyninst::ParseAPI::Block* block)
  .. cpp:function:: private bool checkAllPaths(Dyninst::StackAnalysis& sa)
  .. cpp:function:: private bool checkAllPathsInternal(Dyninst::ParseAPI::Block* block, std::set<Dyninst::ParseAPI::Block*>& state, std::vector<Dyninst::ParseAPI::Block*>& path, std::set<Dyninst::ParseAPI::Block*>& exitBlocks, Dyninst::StackAnalysis& sa)
  .. cpp:function:: private bool checkPath(std::vector<Dyninst::ParseAPI::Block*>& path)
  .. cpp:function:: private bool findInsertOrRemovePoints(Dyninst::StackAnalysis& sa, StackMod* m, std::vector<BPatch_point*>*& points, long& dispFromRSP)
  .. cpp:function:: private bool checkInsn(Dyninst::ParseAPI::Block* block, Dyninst::Offset off, int loc, Dyninst::StackAnalysis& sa, BPatch_point*& point, long& dispFromRSP)
  .. cpp:function:: private bool findCanaryPoints(std::vector<BPatch_point*>* insertPoints, std::vector<BPatch_point*>* checkPoints)
  .. cpp:function:: private bool areModificationsSafe()
  .. cpp:function:: private bool isAccessSafe(InstructionAPI::Instruction insn, StackAccess *access)
  .. cpp:member:: private bool _unsafeStackGrowth
  .. cpp:member:: private std::map<Dyninst::ParseAPI::Block*, std::vector<Dyninst::StackAnalysis::Height>* > blockHeights
  .. cpp:member:: private rangeMap _stackRanges
  .. cpp:member:: private rangeMap _stackRangesCleanup
