
#ifndef AST_HDR
#define AST_HDR

/*
 * $Log: ast.h,v $
 * Revision 1.6  1995/02/16 08:32:50  markc
 * Changed igen interfaces to use strings/vectors rather than char*/igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.5  1994/11/02  11:00:57  markc
 * Replaced string-handles.
 *
 * Revision 1.4  1994/09/22  01:44:51  markc
 * Made first arg to AstNode constructor const
 * strdup'd callee in AstNode::AstNode, this is temporary
 * Made first arg to createPrimitiveCall const
 *
 */

//
// Define a AST class for use in generating primitive and pred calls
//
//

#include <stdio.h>
#include <strstream.h>

class dataReqNode;

typedef enum { sequenceNode, opCodeNode, operandNode, callNode } nodeType;

typedef enum { Constant, DataValue, DataPtr, Param } operandType;

// a register.
typedef int reg;

class registerSlot {
 public:
    int number;         // what register is it
    bool inUse;      // free or in use.
};

class registerSpace {
    public:
	registerSpace(int count, int *possibles);
	reg allocateRegister();
	void freeRegister(int reg);
	void resetSpace();
    private:
	registerSlot *registers;
	int numRegisters;
	int highWaterRegister;
};

class AstNode {
    public:
	AstNode(const string func, AstNode *l, AstNode *r) {
	    loperand = l;
	    roperand = r;
	    if (func == "setCounter") {
		type = opCodeNode;
		type = opCodeNode;
		op = storeOp;
            } else if (func == "addCounter") {
		type = opCodeNode;
		roperand = new AstNode(plusOp, l, r);
		op = storeOp;
	    } else if (func == "subCounter") {
		type = opCodeNode;
		roperand = new AstNode(minusOp, l, r);
		op = storeOp;
	    } else if (func == "startTimer") {
		type = callNode;
		roperand = NULL;
		if ((r->type != operandNode) || (r->oType !=  Constant)) {
		    ostrstream os(errorLine, 1024, ios::out);
		    os << "invalid second operand to " << func << endl;
		    logLine(errorLine);
		    loperand = NULL;
		} else {
		    if (r->oValue == 0) {
			callee = "DYNINSTstartWallTimer";
		    } else {
			callee = "DYNINSTstartProcessTimer";
		    }
		    loperand = l;
		}
	    } else if (func == "stopTimer") {
		type = callNode;
		roperand = NULL;
		if ((r->type != operandNode) || (r->oType !=  Constant)) {
		    ostrstream os(errorLine, 1024, ios::out);
		    os << "invalid second operand to " << func << endl;
		    loperand = NULL;
		} else {
		    loperand = l;
		    if (r->oValue == 0) {
			callee = "DYNINSTstopWallTimer";
		    } else {
			callee = "DYNINSTstopProcessTimer";
		    }
		}
	    } else {
		type = callNode;
		callee = func;
	    }
	};
	AstNode(operandType ot, void *arg) {
	    type = operandNode;
	    oType = ot;
	    if ((oType == DataPtr) || (oType == DataValue)) {
		dValue = (dataReqNode *) arg;
	    } else {
		oValue = (void *) arg;
	    }

	    loperand = NULL;
	    roperand = NULL;
	};
	AstNode(AstNode *l, AstNode *r) {
	    type = sequenceNode;
	    loperand = l;
	    roperand = r;
	};
	AstNode(opCode ot, AstNode *l, AstNode *r) {
	    type = opCodeNode;
	    op = ot;
	    loperand = l;
	    roperand = r;
	};
	int generateTramp(process *proc, char *i, unsigned &base);
	reg generateCode(process *proc, registerSpace *rs, char *i, 
			 unsigned &base);
	int cost();	// return the # of instruction times in the ast.
	void print();
    private:
	nodeType type;
	opCode op;		// for opCode ndoes
	string callee;		// for call ndoes
	operandType oType;	// for operand nodes
	void *oValue;		// for operand nodes
	dataReqNode *dValue;	// for operand nodes
	AstNode *loperand;
	AstNode *roperand;
	int firstInsn;
	int lastInsn;
};

AstNode *createPrimitiveCall(const string func, dataReqNode*, int param2);
AstNode *createIf(AstNode *expression, AstNode *action);

#endif
