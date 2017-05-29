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
    bool unknownInstruction;
    bool findIndex;

    AbsRegion index;
    Assignment::Ptr indexLoc;
    bool firstMemoryRead;
    Assignment::Ptr memLoc;
    AST::Ptr jumpTargetExpr;

    set<Address> constAddr;

    JumpTableFormatPred(ParseAPI::Function *f,
                        ParseAPI::Block *b,
			ReachFact &r,
			ThunkData &t,
			SymbolicExpression &sym):
            func(f), block(b), rf(r), thunks(t), se(sym) {
	        jumpTableFormat = true;
		unknownInstruction = false;
		findIndex = false;
		firstMemoryRead = true;
	    }

    virtual bool modifyCurrentFrame(Slicer::SliceFrame &frame, Graph::Ptr g);
    std::string format();
    bool isJumpTableFormat() { return jumpTableFormat && findIndex && jumpTargetExpr;}
    bool findSpillRead(Graph::Ptr g, SliceNode::Ptr &);
    void adjustActiveMap(Slicer::SliceFrame &frame, SliceNode::Ptr);
};

#endif
