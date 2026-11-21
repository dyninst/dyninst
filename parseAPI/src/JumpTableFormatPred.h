#ifndef JUMP_TABLE_FORMAT_PRED_H
#define JUMP_TABLE_FORMAT_PRED_H

#include <string>
#include <utility>
#include "CFG.h"
#include "slicing.h"
#include "Edge.h"
#include "ThunkData.h"
#include "Graph.h"
#include "SymbolicExpression.h"
//#include "BoundFactCalculator.h"

class JumpTableFormatPred : public Dyninst::Slicer::Predicates {
public:
    Dyninst::ParseAPI::Function *func;
    Dyninst::ParseAPI::Block *block;
    ReachFact &rf;
    ThunkData &thunks;
    SymbolicExpression &se;

    bool jumpTableFormat;
    bool findIndex;
    bool findTableBase;

    Dyninst::AbsRegion index;
    Dyninst::Assignment::Ptr indexLoc;
    bool firstMemoryRead;
    Dyninst::Assignment::Ptr memLoc;
    AST::Ptr jumpTargetExpr;

    set<Dyninst::Address> constAddr;
    dyn_hash_map<Dyninst::Assignment::Ptr, std::pair<AST::Ptr, AST::Ptr>, Dyninst::Assignment::AssignmentPtrHasher> aliases;

    // On ppc 64, r2 is reserved for storing the address of the global offset table 
    Dyninst::Address toc_address;

    virtual bool modifyCurrentFrame(Dyninst::Slicer::SliceFrame &frame, Dyninst::Graph::Ptr g, Dyninst::Slicer*);
    std::string format();
    bool isJumpTableFormat() { return jumpTableFormat && findIndex && findTableBase && memLoc;}
    bool findRead(Dyninst::Graph::Ptr g, Dyninst::SliceNode::Ptr &);
    bool adjustSliceFrame(Dyninst::Slicer::SliceFrame &frame, Dyninst::SliceNode::Ptr, Dyninst::Slicer*);
    bool isTOCRead(Dyninst::Slicer::SliceFrame &frame, Dyninst::SliceNode::Ptr);
    void FindTOC();
    JumpTableFormatPred(Dyninst::ParseAPI::Function *f,
                        Dyninst::ParseAPI::Block *b,
			ReachFact &r,
			ThunkData &t,
			SymbolicExpression &sym);
    virtual bool ignoreEdge(Dyninst::ParseAPI::Edge *e);
    virtual int slicingSizeLimitFactor() { return 100; }
    //virtual bool endAtPoint(AssignmentPtr) override;

};

#endif
