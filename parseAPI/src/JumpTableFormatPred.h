#ifndef JUMP_TABLE_FORMAT_PRED_H
#define JUMP_TABLE_FORMAT_PRED_H

#include "CFG.h"
#include "slicing.h"
#include "Edge.h"
#include "ThunkData.h"
#include "Graph.h"
#include "SymbolicExpression.h"
//#include "BoundFactCalculator.h"
using namespace Dyninst;

class JumpTableFormatPred : public Slicer::Predicates {
public:
    ParseAPI::Function *func;
    ParseAPI::Block *block;
    ReachFact &rf;
    ThunkData &thunks;
    SymbolicExpression &se;

    bool jumpTableFormat;
    bool findIndex;
    bool findTableBase;

    AbsRegion index;
    Assignment::Ptr indexLoc;
    bool firstMemoryRead;
    Assignment::Ptr memLoc;
    AST::Ptr jumpTargetExpr;

    set<Address> constAddr;
    dyn_hash_map<Assignment::Ptr, std::pair<AST::Ptr, AST::Ptr>, Assignment::AssignmentPtrHasher> aliases;

    // On ppc 64, r2 is reserved for storing the address of the global offset table 
    Address toc_address;

    virtual bool modifyCurrentFrame(Slicer::SliceFrame &frame, Graph::Ptr g, Slicer*);
    std::string format();
    bool isJumpTableFormat() { return jumpTableFormat && findIndex && findTableBase && memLoc;}
    bool findRead(Graph::Ptr g, SliceNode::Ptr &);
    bool adjustSliceFrame(Slicer::SliceFrame &frame, SliceNode::Ptr, Slicer*);
    bool isTOCRead(Slicer::SliceFrame &frame, SliceNode::Ptr);
    void FindTOC();
    JumpTableFormatPred(ParseAPI::Function *f,
                        ParseAPI::Block *b,
			ReachFact &r,
			ThunkData &t,
			SymbolicExpression &sym);
    virtual bool ignoreEdge(ParseAPI::Edge *e);
    virtual int slicingSizeLimitFactor() { return 100; }
    //virtual bool endAtPoint(AssignmentPtr) override;

};

#endif
