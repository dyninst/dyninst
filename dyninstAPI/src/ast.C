#include <stdlib.h>

#include "rtinst/h/rtinst.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"

registerSpace::registerSpace(int count, int *possibles) {
    int i;

    numRegisters = count;
    registers = (registerSlot *) 
	calloc(sizeof(registerSlot), numRegisters);
    for (i=0; i < count; i++) {
	registers[i].number = possibles[i];
	registers[i].inUse = False;
    }
}

reg registerSpace::allocateRegister() {
    int i;
    for (i=0; i < numRegisters; i++) {
	if (!registers[i].inUse) {
	    registers[i].inUse = True;
	    highWaterRegister = (highWaterRegister > i) ? 
		 highWaterRegister : i;
	    return(registers[i].number);
	}
    }
    abort();
    return(-1);
}

void registerSpace::freeRegister(int reg) {
    int i;
    for (i=0; i < numRegisters; i++) {
	if (registers[i].number == reg) {
	    registers[i].inUse = False;
	    return;
	}
    }
}

void registerSpace::resetSpace() {
    int i;
    for (i=0; i < numRegisters; i++) {
	registers[i].inUse = False;
    }
    highWaterRegister = 0;
}

extern registerSpace *regSpace;

int AstNode::generateTramp(process *proc, char *i, int *count)
{
    int ret;
    static AstNode *preamble = new AstNode(trampPreamble, NULL, NULL);
    static AstNode *trailer = new AstNode(trampTrailer, NULL, NULL);

    regSpace->resetSpace();

    preamble->generateCode(proc, regSpace, i, count);
    generateCode(proc, regSpace, i, count);
    ret = trailer->generateCode(proc, regSpace, i, count);

    return(ret);
}

reg AstNode::generateCode(process *proc,registerSpace *rs,char *insn, int *base)
{
    int addr;
    int fromAddr;
    int startInsn;
    reg src1, src2;
    reg src = -1;
    reg dest = -1;

    if (type == opCodeNode) {
        if (op == ifOp) {
	    src = loperand->generateCode(proc, rs, insn, base);
	    startInsn = *base;
	    fromAddr = emit(op, src, (reg) 0, (reg) 0, insn, base);
	    rs->freeRegister(src);
            (void) roperand->generateCode(proc, rs, insn, base);
	    // call emit again now with correct offset.
	    (void) emit(op, src, (reg) 0, (reg) (*base - fromAddr), 
		insn, &startInsn);
            // printf("branch forward %d\n", *base - fromAddr);
	} else if (op == storeOp) {
	    src1 = roperand->generateCode(proc, rs, insn, base);
	    src2 = rs->allocateRegister();
	    addr = (int) loperand->dValue->getInferriorPtr();
	    (void) emit(op, src1, src2, (reg) addr, insn, base);
	    rs->freeRegister(src1);
	    rs->freeRegister(src2);
	} else if (op == trampTrailer) {
	    return(emit(op, 0, 0, 0, insn, base));
	} else {
	    if (loperand) 
		src = loperand->generateCode(proc, rs, insn, base);
	    if (roperand) 
		dest = roperand->generateCode(proc, rs, insn, base);
	    (void) emit(op, src, dest, dest, insn, base);
	    rs->freeRegister(src);
	}
    } else if (type == operandNode) {
	dest = rs->allocateRegister();
	if (oType == Constant) {
	    (void) emit(loadConstOp, (reg) oValue, dest, dest, insn, base);
	} else if (oType == DataPtr) {
	    addr = (int) dValue->getInferriorPtr();
	    (void) emit(loadConstOp, (reg) addr, dest, dest, insn, base);
	} else if (oType == DataValue) {
	    addr = (int) dValue->getInferriorPtr();
	    if (dValue->getType() == timer) {
		(void) emit(loadConstOp, (reg) addr, dest, dest, insn, base);
	    } else {
		(void) emit(loadOp, (reg) addr, dest, dest, insn, base);
	    }
	} else if (oType == Param) {
	    src = rs->allocateRegister();
	    dest = getParameter(src, (int) oValue);
	    if (src != dest) {
		rs->freeRegister(src);
	    }
	}
    } else if (type == callNode) {
	int addr;
	// find func addr.
	addr = findInternalAddress(proc->symbols, callee, False);
	if (!addr) {
	    function *func;
	    func = findFunction(proc->symbols, callee);
	    if (!func) {
		printf("unable to find addr of %s\n", callee);
		abort();
	    }
	    addr = func->addr;
	}

	if (loperand) {
	    src1 = loperand->generateCode(proc, rs, insn, base);
	} else {
	    src1 = -1;
	}

	if (roperand) {
	    src2 = roperand->generateCode(proc, rs, insn, base);
	} else {
	    src2 = -1;
	}
	(void) emit(callOp, src1, src2, addr, insn, base);
    } else if (type == sequenceNode) {
	loperand->generateCode(proc, rs, insn, base);
	return(roperand->generateCode(proc, rs, insn, base));
    }
    return(dest);
}

char *getOpString(opCode op)
{
    switch (op) {
	case plusOp: return("+");
	case minusOp: return("-");
	case timesOp: return("*");
	case divOp: return("/");
	case lessOp: return("<");
	case leOp: return("<=");
	case greaterOp: return(">");
	case geOp: return(">=");
	case eqOp: return("==");
	case neOp: return("!=");
	case loadOp: return("lda");
	case loadConstOp: return("load");
	case storeOp: return("=");
	case ifOp: return("if");
	case trampPreamble: return("preTramp");
	case trampTrailer: return("goto");
	case noOp: return("nop");
	case andOp: return("and");
	case orOp: return("or");
	default: return("ERROR");
    }
}

float AstNode::cost()
{
    float total;

    if (type == operandNode) {
	total = 0.0;
    } else if (type == opCodeNode) {
	total = 0.0;
	if (loperand) total += loperand->cost();
	if (roperand) total += roperand->cost();
    } else if (type == callNode) {
	total = getPrimitiveCost(callee);
	if (loperand) total += loperand->cost();
	if (roperand) total += roperand->cost();
    } else if (type == sequenceNode) {
	total = 0.0;
	if (loperand) total += loperand->cost();
	if (roperand) total += roperand->cost();
    }
    return(total);
}

void AstNode::print()
{
    if (type == operandNode) {
	if (oType == Constant) {
	    printf(" %d", (int) oValue);
	} else if (oType == DataPtr) {
	    printf(" %d", (int) dValue->getInferriorPtr());
	} else if (oType == DataValue) {
	    printf(" @%d", (int) dValue->getInferriorPtr());
	} else if (oType == Param) {
	    printf(" param[%d]", oValue);
	}
    } else if (type == opCodeNode) {
	printf(" (%s", getOpString(op));
	if (loperand) loperand->print();
	if (roperand) roperand->print();
	printf(")");
    } else if (type == callNode) {
	printf(" (%s", callee);
	if (loperand) loperand->print();
	if (roperand) roperand->print();
	printf(")");
    } else if (type == sequenceNode) {
	if (loperand) loperand->print();
	printf(", ");
	if (roperand) roperand->print();
    }
}

AstNode *createPrimitiveCall(char *func, dataReqNode *dataPtr, int param2)
{
    return(new AstNode(func, new AstNode(DataValue, (void *) dataPtr), 
			     new AstNode(Constant, (void *) param2)));
}

AstNode *createIf(AstNode *expression, AstNode *action)
{
    return(new AstNode(ifOp, expression, action));
}

