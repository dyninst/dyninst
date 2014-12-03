#ifndef BACKWARD_SLICING_H
#define BACKWARD_SLICING_H

#include "CFG.h"
#include "slicing.h"
#include "Edge.h"

#include "TableGuardData.h"

using namespace Dyninst;


/* This BackwardSlicer first call DataflowAPI::Slicer to get
 * the traiditional backward slice, which is program dependence graph.
 * Then it transforms the dependence graph to a control flow graph
 */
class BackwardSlicer {

    ParseAPI::Function *func;
    ParseAPI::Block *block;
    Address addr;

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
		   Address a):
        func(f), block(b), addr(a) {};
};	

class IndirectControlFlowPred : public Slicer::Predicates {
  
public:
    virtual bool endAtPoint(AssignmentPtr ap);  
    virtual bool followCall(ParseAPI::Function * callee, CallStack_t & cs, AbsRegion arg);
    virtual bool addPredecessor(AbsRegion);
};


class TypedSliceEdge: public Dyninst::Edge {
    EdgeTypeEnum type_; 
    
    TypedSliceEdge(const SliceNode::Ptr source,
              const SliceNode::Ptr target,
	      EdgeTypeEnum t) 
	      : Dyninst::Edge(source, target), type_(t) {};
  public:	      
   typedef boost::shared_ptr<TypedSliceEdge> Ptr; 
   static TypedSliceEdge::Ptr create(SliceNode::Ptr source,
                                     SliceNode::Ptr target,
				     EdgeTypeEnum t) {
	return Ptr(new TypedSliceEdge(source, target, t));       
   }                                                

  public:
    EdgeTypeEnum type() { return type_;}

};

#endif
