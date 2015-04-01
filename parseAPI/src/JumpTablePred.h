#ifndef JUMP_TABLE_PRED_H
#define JUMP_TABLE_PRED_H

#include "CFG.h"
#include "slicing.h"
#include "Edge.h"
#include "ThunkData.h"
#include "Graph.h"
#include "BoundFactCalculator.h"
using namespace Dyninst;

class JumpTablePred : public Slicer::Predicates {
 
    std::set<Assignment::Ptr> currentAssigns;
    ParseAPI::Function *func;
    ParseAPI::Block *block;
    ReachFact &rf;
    ThunkData &thunks;
    std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges;
    GraphPtr BuildAnalysisGraph();
    bool IsJumpTable(GraphPtr slice, BoundFactsCalculator &bfc, BoundValue &target);
    bool FillInOutEdges(BoundValue &target, std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges);

public:
    virtual bool endAtPoint(AssignmentPtr ap);  
    virtual bool addNodeCallback(AssignmentPtr ap);

    JumpTablePred(ParseAPI::Function *f,
                  ParseAPI::Block *b,
		  ReachFact &r,
		  ThunkData &t,
		  std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& out):
            func(f), block(b), rf(r), thunks(t), outEdges(out) {}
};


class TypedSliceEdge: public Dyninst::Edge {
    ParseAPI::EdgeTypeEnum type_; 
    
    TypedSliceEdge(const SliceNode::Ptr source,
              const SliceNode::Ptr target,
	      ParseAPI::EdgeTypeEnum t) 
	      : Dyninst::Edge(source, target), type_(t) {};
  public:	      
   typedef boost::shared_ptr<TypedSliceEdge> Ptr; 
   static TypedSliceEdge::Ptr create(SliceNode::Ptr source,
                                     SliceNode::Ptr target,
				     ParseAPI::EdgeTypeEnum t) {
	return Ptr(new TypedSliceEdge(source, target, t));       
   }                                                

  public:
    ParseAPI::EdgeTypeEnum type() { return type_;}

};

#endif
