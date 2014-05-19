#ifndef INDIRECT_CONTROL_FLOW_H
#define INDIRECT_CONTROL_FLOW_H

#include "TableGuardData.h"
#include "BoundFactData.h"
#include "CFG.h"
#include "CodeSource.h"

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

    set<Node::Ptr> defined;
   
    void ConditionalJumpBound(BoundFact &curFact, Node::Ptr src, Node::Ptr trg);  

    void Meet(Node::Ptr curNode);

    void CalcTransferFunction(Node::Ptr curNode);

public:
    bool CalculateBoundedFacts(); 

    BoundFactsCalculator(GuardSet &g, ParseAPI::Function *f, GraphPtr s, bool first, ReachFact &r): 
        guards(g), func(f), slice(s), firstBlock(first), rf(r) {} 

    BoundFact &GetBoundFact(Node::Ptr node) {return boundFacts[node.get()];}
};


class IndirectControlFlowAnalyzer {
    GuardSet guards;
    // The function and block that contain the indirect jump
    SymtabCodeSource *sts;
    ParseAPI::Function *func;
    ParseAPI::Block *block;

    bool EndWithConditionalJump(ParseAPI::Block * b);
    void GetAllReachableBlock(set<ParseAPI::Block*> &reachable);
    void SaveGuardData(ParseAPI::Block *prev);    
    void FindAllConditionalGuards(); 
    bool IsJumpTable(GraphPtr slice, BoundFactsCalculator &bfc);
    GraphPtr CalcBackwardSlice(ParseAPI::Block *b, 
                               Address addr,
			       string filename);


public:
    bool NewJumpTableAnalysis();
    IndirectControlFlowAnalyzer(SymtabCodeSource *s, ParseAPI::Function *f, ParseAPI::Block *b): sts(s), func(f), block(b) {}

};

#endif
