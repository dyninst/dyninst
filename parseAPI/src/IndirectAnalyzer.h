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
using namespace Dyninst;

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
    IndirectControlFlowAnalyzer(ParseAPI::Function *f, ParseAPI::Block *b): func(f), block(b) {}

};

#endif
