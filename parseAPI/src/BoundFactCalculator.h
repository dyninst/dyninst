#ifndef BOUND_FACTS_CALCULATOR_H
#define BOUND_FACTS_CALCULATOR_H

#include "ThunkData.h"
#include "BoundFactData.h"
#include "CFG.h"

#include "slicing.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

// To avoid the bound fact calculation from deadlock
#define IN_QUEUE_LIMIT 50



class BoundFactsCalculator {
    BoundFactsType boundFacts;
    ParseAPI::Function *func;
    GraphPtr slice;
    bool firstBlock;
    ReachFact &rf;
    ThunkData &thunks;

    void ThunkBound(BoundFact *curFact, Node::Ptr src, Node::Ptr trg);
    BoundFact* Meet(Node::Ptr curNode);
    void CalcTransferFunction(Node::Ptr curNode, BoundFact *newFact);

public:
    bool CalculateBoundedFacts(); 

    BoundFactsCalculator(ParseAPI::Function *f, GraphPtr s, bool first, ReachFact &r, ThunkData &t): 
        func(f), slice(s), firstBlock(first), rf(r), thunks(t) {} 

    BoundFact *GetBoundFact(Node::Ptr node);
    ~BoundFactsCalculator();
};

#endif
