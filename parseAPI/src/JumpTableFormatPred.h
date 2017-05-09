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

    ParseAPI::Function *func;
    ParseAPI::Block *block;
    ReachFact &rf;
    ThunkData &thunks;
    SymbolicExpression &se;

    std::pair<AST::Ptr, bool> ExpandAssignment(Assignment::Ptr);

    Address targetBase;    
    // If tableReadSize == 0, this does not represent a memory access
    // Otherwise, tableReadSize reprenents the number bytes of the access
    int tableReadSize;
    int tableStride;

    // On ARM, the table content is often multiplied by 4 before adding with targetBase
    int tcMultiply;
    bool isInverted;
    bool isSubReadContent;
    bool isZeroExtend;

    bool jumpTableFormat;
    bool unknownInstruction;
    bool findIndex;

    AbsRegion index;
    AST::Ptr jumpTargetExpr;


public:

    JumpTableFormatPred(ParseAPI::Function *f,
                        ParseAPI::Block *b,
			ReachFact &r,
			ThunkData &t,
			SymbolicExpression &sym):
            func(f), block(b), rf(r), thunks(t), se(sym) {
	        targetBase = 0;
		tableReadSize = 0;
		tcMultiply = 1;
		isInverted = false;
		isSubReadContent = false;
		isZeroExtend = false;
		jumpTableFormat = true;
		unknownInstruction = false;
		findIndex = false;
	    }

    virtual bool modifyCurrentFrame(Slicer::SliceFrame &frame, Graph::Ptr g);
    std::string format();
    bool isJumpTableFormat() { return jumpTableFormat; }

};

#endif
