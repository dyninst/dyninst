#ifndef BOUND_FACTS_CALCULATOR_H
#define BOUND_FACTS_CALCULATOR_H

#include "ThunkData.h"
#include "BoundFactData.h"
#include "CFG.h"
#include "SymbolicExpression.h"
#include "slicing.h"

#include <vector>
#include <unordered_set>
#include <unordered_map>


// To avoid the bound fact calculation from deadlock
#define IN_QUEUE_LIMIT 10



class BoundFactsCalculator {
    BoundFactsType boundFactsIn, boundFactsOut;
    Dyninst::ParseAPI::Function *func;
    Dyninst::GraphPtr slice;
    bool firstBlock;
    bool handleOneByteRead;
    SymbolicExpression &se;

    BoundFact* Meet(Dyninst::Node::Ptr curNode);
    void CalcTransferFunction(Dyninst::Node::Ptr curNode, BoundFact *newFact);

    std::unordered_map<Dyninst::Node::Ptr, int, Dyninst::Node::NodePtrHasher> analysisOrder, nodeColor;
    vector<Dyninst::Node::Ptr> reverseOrder;
    int orderStamp{};
    
    void DetermineAnalysisOrder();
    void NaturalDFS(Dyninst::Node::Ptr);
    void ReverseDFS(Dyninst::Node::Ptr);
    bool HasIncomingEdgesFromLowerLevel(int curOrder, std::vector<Dyninst::Node::Ptr>& curNodes);

public:
    bool CalculateBoundedFacts();

    BoundFactsCalculator(Dyninst::ParseAPI::Function *f, Dyninst::GraphPtr s, bool first,
                         bool oneByteRead, SymbolicExpression &sym)
        : func(f), slice(s), firstBlock(first), handleOneByteRead(oneByteRead),
          se(sym) {}

    BoundFact *GetBoundFactIn(Dyninst::Node::Ptr node);
    BoundFact *GetBoundFactOut(Dyninst::Node::Ptr node);
    ~BoundFactsCalculator();
};

#endif
