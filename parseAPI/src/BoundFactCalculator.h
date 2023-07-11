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

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

// To avoid the bound fact calculation from deadlock
#define IN_QUEUE_LIMIT 10



class BoundFactsCalculator {
    BoundFactsType boundFactsIn, boundFactsOut;
    ParseAPI::Function *func;
    GraphPtr slice;
    bool firstBlock;
    bool handleOneByteRead;
    SymbolicExpression &se;

    BoundFact* Meet(Node::Ptr curNode);
    void CalcTransferFunction(Node::Ptr curNode, BoundFact *newFact);

    std::unordered_map<Node::Ptr, int, Node::NodePtrHasher> analysisOrder, nodeColor;
    vector<Node::Ptr> reverseOrder;
    int orderStamp{};
    
    void DetermineAnalysisOrder();
    void NaturalDFS(Node::Ptr);
    void ReverseDFS(Node::Ptr);
    bool HasIncomingEdgesFromLowerLevel(int curOrder, std::vector<Node::Ptr>& curNodes);

public:
    bool CalculateBoundedFacts();

    BoundFactsCalculator(ParseAPI::Function *f, GraphPtr s, bool first,
                         bool oneByteRead, SymbolicExpression &sym)
        : func(f), slice(s), firstBlock(first), handleOneByteRead(oneByteRead),
          se(sym) {}

    BoundFact *GetBoundFactIn(Node::Ptr node);
    BoundFact *GetBoundFactOut(Node::Ptr node);
    ~BoundFactsCalculator();
};

#endif
