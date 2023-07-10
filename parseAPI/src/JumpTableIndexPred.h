#ifndef JUMP_TABLE_INDEX_PRED_H
#define JUMP_TABLE_INDEX_PRED_H

#include <set>
#include <vector>
#include "CFG.h"
#include "slicing.h"
#include "Edge.h"
#include "ThunkData.h"
#include "Graph.h"
#include "BoundFactCalculator.h"
#include "Absloc.h"
using namespace Dyninst;

class JumpTableIndexPred : public Slicer::Predicates {

    ParseAPI::Function *func;
    ParseAPI::Block *block;
    AbsRegion index;
    SymbolicExpression &se;
    std::vector<AST::Ptr> readAST;
    
    bool MatchReadAST(Assignment::Ptr a);


public:
    bool unknownInstruction;
    bool findBound;
    StridedInterval bound;
    std::set<Assignment::Ptr> currentAssigns;
    virtual bool addNodeCallback(AssignmentPtr ap, std::set<ParseAPI::Edge*> &visitedEdges);
    virtual bool modifyCurrentFrame(Slicer::SliceFrame &frame, Graph::Ptr g, Slicer*);
    GraphPtr BuildAnalysisGraph(std::set<ParseAPI::Edge*> &visitedEdges);
    bool IsIndexBounded(GraphPtr slice, BoundFactsCalculator &bfc, StridedInterval &target);

    JumpTableIndexPred(ParseAPI::Function *f,
                       ParseAPI::Block *b,
		       AbsRegion i,
		       SymbolicExpression &sym)
		          : func(f), 
			    block(b),
			    index(i),
			    se(sym) {
			       unknownInstruction = false;
			       findBound = false;
		      }
    virtual bool ignoreEdge(ParseAPI::Edge *e);
    virtual int slicingSizeLimitFactor() { return 100; }
};


class TypedSliceEdge: public Dyninst::Edge {
    ParseAPI::EdgeTypeEnum type_; 
    
    TypedSliceEdge(const SliceNode::Ptr source,
              const SliceNode::Ptr target,
	      ParseAPI::EdgeTypeEnum t) 
	      : Dyninst::Edge(source, target), type_(t) {}
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
