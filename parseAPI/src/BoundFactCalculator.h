#ifndef BOUND_FACTS_CALCULATOR_H
#define BOUND_FACTS_CALCULATOR_H

#include "ThunkData.h"
#include "BoundFactData.h"
#include "CFG.h"

#include "slicing.h"

#include <unordered_set>
#include <unordered_map>

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

// To avoid the bound fact calculation from deadlock
#define IN_QUEUE_LIMIT 10



class BoundFactsCalculator {
    BoundFactsType boundFacts;
    ParseAPI::Function *func;
    GraphPtr slice;
    bool firstBlock;
    ReachFact &rf;
    ThunkData &thunks;
    Address jumpAddr;

    std::unordered_map<Assignment::Ptr, AST::Ptr, Assignment::AssignmentPtrHasher> expandCache;

    void ThunkBound(BoundFact *curFact, Node::Ptr src, Node::Ptr trg);
    BoundFact* Meet(Node::Ptr curNode);
    void CalcTransferFunction(Node::Ptr curNode, BoundFact *newFact);

    std::unordered_map<Node::Ptr, int, Node::NodePtrHasher> analysisOrder, nodeColor;
    vector<Node::Ptr> reverseOrder;
    int orderStamp;
    
    void DetermineAnalysisOrder();
    void NaturalDFS(Node::Ptr);
    void ReverseDFS(Node::Ptr);
    bool HasIncomingEdgesFromLowerLevel(int curOrder, std::vector<Node::Ptr>& curNodes);

    std::pair<AST::Ptr, bool> ExpandAssignment(Assignment::Ptr);

public:
    bool CalculateBoundedFacts(); 

    BoundFactsCalculator(ParseAPI::Function *f, GraphPtr s, bool first, ReachFact &r, ThunkData &t, Address addr): 
        func(f), slice(s), firstBlock(first), rf(r), thunks(t), jumpAddr(addr) {} 

    BoundFact *GetBoundFact(Node::Ptr node);
    ~BoundFactsCalculator();
};

#endif
