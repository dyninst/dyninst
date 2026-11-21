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

class JumpTableIndexPred : public Dyninst::Slicer::Predicates {

    Dyninst::ParseAPI::Function *func;
    Dyninst::ParseAPI::Block *block;
    Dyninst::AbsRegion index;
    SymbolicExpression &se;
    std::vector<AST::Ptr> readAST;
    
    bool MatchReadAST(Dyninst::Assignment::Ptr a);


public:
    bool unknownInstruction;
    bool findBound;
    StridedInterval bound;
    std::set<Dyninst::Assignment::Ptr> currentAssigns;
    virtual bool addNodeCallback(Dyninst::AssignmentPtr ap, std::set<Dyninst::ParseAPI::Edge*> &visitedEdges);
    virtual bool modifyCurrentFrame(Dyninst::Slicer::SliceFrame &frame, Dyninst::Graph::Ptr g, Dyninst::Slicer*);
    Dyninst::GraphPtr BuildAnalysisGraph(std::set<Dyninst::ParseAPI::Edge*> &visitedEdges);
    bool IsIndexBounded(Dyninst::GraphPtr slice, BoundFactsCalculator &bfc, StridedInterval &target);

    JumpTableIndexPred(Dyninst::ParseAPI::Function *f,
                       Dyninst::ParseAPI::Block *b,
		       Dyninst::AbsRegion i,
		       SymbolicExpression &sym)
		          : func(f), 
			    block(b),
			    index(i),
			    se(sym) {
			       unknownInstruction = false;
			       findBound = false;
		      }
    virtual bool ignoreEdge(Dyninst::ParseAPI::Edge *e);
    virtual int slicingSizeLimitFactor() { return 100; }
};


class TypedSliceEdge: public Dyninst::Edge {
    Dyninst::ParseAPI::EdgeTypeEnum type_; 
    
    TypedSliceEdge(const Dyninst::SliceNode::Ptr source,
              const Dyninst::SliceNode::Ptr target,
	      Dyninst::ParseAPI::EdgeTypeEnum t) 
	      : Dyninst::Edge(source, target), type_(t) {}
  public:	      
   typedef boost::shared_ptr<TypedSliceEdge> Ptr; 
   static TypedSliceEdge::Ptr create(Dyninst::SliceNode::Ptr source,
                                     Dyninst::SliceNode::Ptr target,
				     Dyninst::ParseAPI::EdgeTypeEnum t) {
	return Ptr(new TypedSliceEdge(source, target, t));       
   }                                                

  public:
    Dyninst::ParseAPI::EdgeTypeEnum type() { return type_;}

};

#endif
