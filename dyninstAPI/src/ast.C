#include <stdlib.h>

#include "rtinst/h/rtinst.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"
#include "util.h"

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

int AstNode::generateTramp(process *proc, char *i, caddr_t *count)
{
    int ret;
    int cycles;
    AstNode *preamble;
    static AstNode *trailer = new AstNode(trampTrailer, NULL, NULL);
    // used to estimate cost.
    static AstNode *preambleTemplate = new AstNode(trampPreamble, 
	new AstNode(Constant, (void *) 0), NULL);
    
    
    //
    // argument to the preamble is the cost of this tramp
    //    WARNING: WE assume the machine specific part will add the
    //       cost of its preamble and trailer to this cost.
    // 
    cycles = preambleTemplate->cost() + cost() + trailer->cost();

    preamble = new AstNode(trampPreamble, 
	new AstNode(Constant, (void *) cycles), NULL);

    regSpace->resetSpace();

    preamble->generateCode(proc, regSpace, i, count);
    generateCode(proc, regSpace, i, count);
    ret = trailer->generateCode(proc, regSpace, i, count);

    delete(preamble);
    return(ret);
}

reg AstNode::generateCode(process *proc,
			  registerSpace *rs,
			  char *insn, 
			  caddr_t *base)
{
    caddr_t addr;
    caddr_t fromAddr;
    caddr_t startInsn;
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
            // sprintf(errorLine,branch forward %d\n", *base - fromAddr);
	} else if (op == storeOp) {
	    src1 = roperand->generateCode(proc, rs, insn, base);
	    src2 = rs->allocateRegister();
	    addr = loperand->dValue->getInferriorPtr();
	    (void) emit(op, src1, src2, (reg) addr, insn, base);
	    rs->freeRegister(src1);
	    rs->freeRegister(src2);
	} else if (op == trampTrailer) {
	    return((unsigned) emit(op, 0, 0, 0, insn, base));
	} else if (op == trampPreamble) {
	    int cost;

	    // loperand is a constant AST node with the cost.
	    cost = (int) loperand->oValue;
	    return((unsigned) emit(op, cost, 0, 0, insn, base));
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
	    addr = dValue->getInferriorPtr();
	    (void) emit(loadConstOp, (reg) addr, dest, dest, insn, base);
	} else if (oType == DataValue) {
	    addr = dValue->getInferriorPtr();
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
	caddr_t addr;
	// find func addr.
	addr = findInternalAddress(proc->symbols, callee, False);
	if (!addr) {
	    function *func;
	    func = findFunction(proc->symbols, callee);
	    if (!func) {
		sprintf(errorLine, "unable to find addr of %s\n", callee);
		logLine(errorLine);
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
	(void) emit(callOp, src1, src2, (int) addr, insn, base);
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

int AstNode::cost()
{
    int total = 0;
    int getInsnCost(opCode t);

    if (type == opCodeNode) {
        if (op == ifOp) {
	    total += loperand->cost();
	    total += getInsnCost(op);
	    total += roperand->cost();
	} else if (op == storeOp) {
	    total += roperand->cost();
	    total += getInsnCost(op);
	} else if (op == trampTrailer) {
	    total = getInsnCost(op);
	} else if (op == trampPreamble) {
	    total = getInsnCost(op);
	} else {
	    if (loperand) 
		total += loperand->cost();
	    if (roperand) 
		total += loperand->cost();
	    total += getInsnCost(op);
	}
    } else if (type == operandNode) {
	if (oType == Constant) {
	    total = getInsnCost(loadConstOp);
	} else if (oType == DataPtr) {
	    total = getInsnCost(loadConstOp);
	} else if (oType == DataValue) {
	    if (dValue->getType() == timer) {
		total = getInsnCost(loadConstOp);
	    } else {
		total = getInsnCost(loadOp);
	    }
	} else if (oType == Param) {
	    // for SPARC its always in a register.
	    total = 0;
	}
    } else if (type == callNode) {
	total = getPrimitiveCost(callee);
	if (loperand) total += loperand->cost();
	if (roperand) total += roperand->cost();
    } else if (type == sequenceNode) {
	total += loperand->cost();
	total += roperand->cost();
    }
    return(total);
}

void AstNode::print()
{
    if (type == operandNode) {
	if (oType == Constant) {
	    sprintf(errorLine, " %d", (int) oValue);
	    logLine(errorLine);
	} else if (oType == DataPtr) {
	    sprintf(errorLine, " %d", (int) dValue->getInferriorPtr());
	    logLine(errorLine);
	} else if (oType == DataValue) {
	    sprintf(errorLine, "@%d", (int) dValue->getInferriorPtr());
	    logLine(errorLine);
	} else if (oType == Param) {
	    sprintf(errorLine, "param[%d]", oValue);
	    logLine(errorLine);
	}
    } else if (type == opCodeNode) {
	sprintf(errorLine, "(%s", getOpString(op));
	logLine(errorLine);
	if (loperand) loperand->print();
	if (roperand) roperand->print();
	logLine(")");
    } else if (type == callNode) {
	sprintf(errorLine, "(%s", callee);
	logLine(errorLine);
	if (loperand) loperand->print();
	if (roperand) roperand->print();
	logLine(")");
    } else if (type == sequenceNode) {
	if (loperand) loperand->print();
	logLine(",");
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

