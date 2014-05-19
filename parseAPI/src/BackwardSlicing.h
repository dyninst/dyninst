#ifndef BACKWARD_SLICING_H
#define BACKWARD_SLICING_H

#include "CFG.h"
#include "slicing.h"

#include "TableGuardData.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;


/* This BackwardSlicer first call DataflowAPI::Slicer to get
 * the traiditional backward slice, then it prune the slice 
 * according to the table guard, and finally transforms the
 * slice graph from a data dependence graph to a control flow
 * graph
 */
class BackwardSlicer {

    ParseAPI::Function *func;
    ParseAPI::Block *block;
    Address addr;
    GuardSet &guards;
    ReachFact &rf;

    int AdjustGraphEntryAndExit(GraphPtr gp);
    void DeleteUnreachableNodes(GraphPtr gp);
    bool PassThroughGuards(SliceNode::Ptr src, SliceNode::Ptr trg);
    void DeleteDataDependence(GraphPtr gp);
    GraphPtr TransformGraph(GraphPtr gp);
    GraphPtr TransformToCFG(GraphPtr gp);

public:

    GraphPtr CalculateBackwardSlicing();
    BackwardSlicer(ParseAPI::Function *f,
                   ParseAPI::Block *b,
		   Address a,
		   GuardSet &g,
		   ReachFact &r):
        func(f), block(b), addr(a), guards(g), rf(r) {};
};	

class IndirectControlFlowPred : public Slicer::Predicates {
    GuardSet &guards;
public:
    virtual bool endAtPoint(AssignmentPtr ap);  
    virtual bool followCall(ParseAPI::Function * callee, CallStack_t & cs, AbsRegion arg);
    IndirectControlFlowPred (GuardSet &g): guards(g) {};
};


#endif
