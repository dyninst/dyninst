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

class IndirectControlFlowAnalyzer {
    // The function and block that contain the indirect jump
    Dyninst::ParseAPI::Function *func;
    Dyninst::ParseAPI::Block *block;
    set<Dyninst::ParseAPI::Block*> reachable;
    ThunkData thunks;

    void GetAllReachableBlock();  
    void FindAllThunks();
    void ReadTable(AST::Ptr,
                   Dyninst::AbsRegion,
                   StridedInterval &,
                   int ,
                   bool, 
                   std::set<Dyninst::Address> &,
                   std::vector<std::pair<Dyninst::Address, Dyninst::ParseAPI::EdgeTypeEnum> > &,
                   Dyninst::Address &,
                   Dyninst::Address &,
                   int &,
                   std::map<Dyninst::Address, Dyninst::Address>&);
    int GetMemoryReadSize(Dyninst::Assignment::Ptr loc);
    bool IsZeroExtend(Dyninst::Assignment::Ptr loc);
    bool FindJunkInstruction(Dyninst::Address);


public:
    bool NewJumpTableAnalysis(std::vector<std::pair< Dyninst::Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges);
    bool ResolveCallTargetBySlicing(Dyninst::Address& target);
    IndirectControlFlowAnalyzer(Dyninst::ParseAPI::Function *f, Dyninst::ParseAPI::Block *b): func(f), block(b) {}

};

#endif
