#ifndef INDIRECT_ANALYZER_H
#define INDIRECT_ANALYZER_H

#include <map>
#include <set>
#include <utility>
#include <vector>
#include "ThunkData.h"
#include "BoundFactData.h"
#include "CFG.h"
#include "slicing.h"
#include "BoundFactCalculator.h"
#include "Instruction.h"
using namespace Dyninst;

// Backward-slicing predicate for resolving a control-transfer target that is
// materialized across multiple instructions (register-materialized call/jump).
// Arch-independent: the slice follows register def-use (unlimited bound,
// inherited) and stops at a memory read -- a genuinely indirect target loaded
// from a table/GOT cannot fold to a constant, and stopping bounds the slice.
// Whatever the arch's SymEval can fold (as exercised by jump-table analysis)
// resolves; everything else stays unresolved (sink).
class MaterializedTargetPred : public Slicer::Predicates {
public:
    virtual bool endAtPoint(AssignmentPtr a) {
        return a->insn().readsMemory();
    }
};

class IndirectControlFlowAnalyzer {
    // The function and block that contain the indirect jump
    ParseAPI::Function *func;
    ParseAPI::Block *block;
    set<ParseAPI::Block*> reachable;
    ThunkData thunks;

    void GetAllReachableBlock();  
    void FindAllThunks();
    void ReadTable(AST::Ptr,
                   AbsRegion,
                   StridedInterval &,
                   int ,
                   bool, 
                   std::set<Address> &,
                   std::vector<std::pair<Address, Dyninst::ParseAPI::EdgeTypeEnum> > &,
                   Address &,
                   Address &,
                   int &,
                   std::map<Address, Address>&);
    int GetMemoryReadSize(Assignment::Ptr loc);
    bool IsZeroExtend(Assignment::Ptr loc);
    bool FindJunkInstruction(Address);


public:
    bool NewJumpTableAnalysis(std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges);
    bool ResolveTargetBySlicing(MaterializedTargetPred& pred, Dyninst::Address& target);
    IndirectControlFlowAnalyzer(ParseAPI::Function *f, ParseAPI::Block *b): func(f), block(b) {}

};

#endif
