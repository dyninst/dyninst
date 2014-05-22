#ifndef INDIRECT_CONTROL_FLOW_H
#define INDIRECT_CONTROL_FLOW_H

#include "TableGuardData.h"
#include "BoundFactData.h"
#include "CFG.h"
#include "InstructionSource.h"

#include "slicing.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InstructionAPI;

class BoundFactsCalculator {
    BoundFactsType boundFacts;
    GuardSet &guards;    
    ParseAPI::Function *func;
    GraphPtr slice;
    bool firstBlock;
    ReachFact &rf;

    void ConditionalJumpBound(BoundFact *curFact, Node::Ptr src, Node::Ptr trg);  

    BoundFact* Meet(Node::Ptr curNode);
    void CalcTransferFunction(Node::Ptr curNode, BoundFact *newFact);

public:
    bool CalculateBoundedFacts(); 

    BoundFactsCalculator(GuardSet &g, ParseAPI::Function *f, GraphPtr s, bool first, ReachFact &r): 
        guards(g), func(f), slice(s), firstBlock(first), rf(r) {} 

    BoundFact *GetBoundFact(Node::Ptr node);
    ~BoundFactsCalculator();
};


class IndirectControlFlowAnalyzer {
    GuardSet guards;
    // The function and block that contain the indirect jump
    ParseAPI::Function *func;
    ParseAPI::Block *block;

    bool EndWithConditionalJump(ParseAPI::Block * b);
    void GetAllReachableBlock(set<ParseAPI::Block*> &reachable);
    void SaveGuardData(ParseAPI::Block *prev);    
    void FindAllConditionalGuards(); 
    bool IsJumpTable(GraphPtr slice, BoundFactsCalculator &bfc, BoundValue &target);
    bool FillInOutEdges(BoundValue &target, std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges);
    GraphPtr CalcBackwardSlice(ParseAPI::Block *b, 
                               Address addr,
			       string filename);


public:
    bool NewJumpTableAnalysis(std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges);
    IndirectControlFlowAnalyzer(ParseAPI::Function *f, ParseAPI::Block *b): func(f), block(b) {}

};

#endif
