
/*
 * $Log: ast.h,v $
 * Revision 1.4  1994/09/22 01:44:51  markc
 * Made first arg to AstNode constructor const
 * strdup'd callee in AstNode::AstNode, this is temporary
 * Made first arg to createPrimitiveCall const
 *
 */

//
// Define a AST class for use in generating primitive and pred calls
//
//

extern "C" {
#include <string.h>
#include <stdio.h>
}

class dataReqNode;

typedef enum { sequenceNode, opCodeNode, operandNode, callNode } nodeType;

typedef enum { Constant, DataValue, DataPtr, Param } operandType;

// a register.
typedef int reg;

class registerSlot {
 public:
    int number;         // what register is it
    Boolean inUse;      // free or in use.
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
	AstNode(const char *func, AstNode *l, AstNode *r) {
	    loperand = l;
	    roperand = r;
	    if (!strcmp(func, "setCounter")) {
		type = opCodeNode;
		type = opCodeNode;
		op = storeOp;
            } else if (!strcmp(func, "addCounter")) {
		type = opCodeNode;
		roperand = new AstNode(plusOp, l, r);
		op = storeOp;
	    } else if (!strcmp(func, "subCounter")) {
		type = opCodeNode;
		roperand = new AstNode(minusOp, l, r);
		op = storeOp;
	    } else if (!strcmp(func, "startTimer")) {
		type = callNode;
		roperand = NULL;
		if ((r->type != operandNode) || (r->oType !=  Constant)) {
		    printf("invalid second operand to %s\n", func);
		    loperand = NULL;
		} else {
		    if (r->oValue == 0) {
			callee = strdup("DYNINSTstartWallTimer");
		    } else {
			callee = strdup("DYNINSTstartProcessTimer");
		    }
		    loperand = l;
		}
	    } else if (!strcmp(func, "stopTimer")) {
		type = callNode;
		roperand = NULL;
		if ((r->type != operandNode) || (r->oType !=  Constant)) {
		    printf("invalid second operand to %s\n", func);
		    loperand = NULL;
		} else {
		    loperand = l;
		    if (r->oValue == 0) {
			callee = strdup("DYNINSTstopWallTimer");
		    } else {
			callee = strdup("DYNINSTstopProcessTimer");
		    }
		}
	    } else {
		type = callNode;
		callee = strdup(func);
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
	int generateTramp(process *proc, char *i, caddr_t *base);
	reg generateCode(process *proc, registerSpace *rs, char *i, 
			 caddr_t *base);
	int cost();	// return the # of instruction times in the ast.
	void print();
    private:
	nodeType type;
	opCode op;		// for opCode ndoes
	char *callee;		// for call ndoes
	operandType oType;	// for operand nodes
	void *oValue;		// for operand nodes
	dataReqNode *dValue;	// for operand nodes
	AstNode *loperand;
	AstNode *roperand;
	int firstInsn;
	int lastInsn;
};

AstNode *createPrimitiveCall(const char *func, dataReqNode*, int param2);
AstNode *createIf(AstNode *expression, AstNode *action);
