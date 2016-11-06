#ifndef INDIRECT_ANALYZER_H
#define INDIRECT_ANALYZER_H

#include "ThunkData.h"
#include "BoundFactData.h"
#include "CFG.h"
#include "slicing.h"
#include "BoundFactCalculator.h"
using namespace Dyninst;

class IndirectControlFlowAnalyzer {
  // The function and block that contain the indirect jump
  ParseAPI::Function *func;
  ParseAPI::Block *block;
  set<ParseAPI::Block *> reachable;
  ThunkData thunks;

  void GetAllReachableBlock();
  void FindAllThunks();
  bool IsJumpTable(GraphPtr slice, BoundFactsCalculator &bfc,
                   BoundValue &target);
  bool FillInOutEdges(
      BoundValue &target,
      std::vector<std::pair<Address, Dyninst::ParseAPI::EdgeTypeEnum> >
          &outEdges);
  GraphPtr CalcBackwardSlice(ParseAPI::Block *b, Address addr, string filename);

 public:
  bool NewJumpTableAnalysis(
      std::vector<std::pair<Address, Dyninst::ParseAPI::EdgeTypeEnum> >
          &outEdges);
  IndirectControlFlowAnalyzer(ParseAPI::Function *f, ParseAPI::Block *b)
      : func(f), block(b) {}
};

#endif
