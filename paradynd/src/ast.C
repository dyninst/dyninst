
/* 
 * $Log: ast.C,v $
 * Revision 1.17  1995/08/24 15:03:44  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.16  1995/07/11  20:57:29  jcargill
 * Changed sparc-specific ifdefs to include sparc_tmc_cmost7_3
 *
 * Revision 1.15  1995/05/30  05:04:55  krisna
 * upgrade from solaris-2.3 to solaris-2.4.
 * architecture-os based include protection of header files.
 * removed architecture-os dependencies in generic sources.
 * changed ST_* symbol names to PDST_* (to avoid conflict on HPUX)
 *
 * Revision 1.14  1995/05/18  10:29:19  markc
 * Prevent read-only registers from being deallocated.
 * Return register for procedure calls to allow values to be returned
 *
 * Revision 1.13  1995/03/10  19:29:12  hollings
 * Added code to include base tramp cost in first mini-tramp.
 *
 * Revision 1.12  1995/02/16  08:52:49  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.11  1995/02/16  08:32:48  markc
 * Changed igen interfaces to use strings/vectors rather than charigen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.10  1995/01/30  17:32:07  jcargill
 * changes for gcc-2.6.3; intCounter was both a typedef and an enum constant
 *
 * Revision 1.9  1994/11/02  19:01:22  hollings
 * Made the observed cost model use a normal variable rather than a reserved
 * register.
 *
 * Revision 1.8  1994/11/02  11:00:33  markc
 * Replaced string handles.
 * Attempted to use one type for addresses vs. caddr_t, int, unsigned.
 *
 * Revision 1.7  1994/09/22  01:33:59  markc
 * Cast args for printf
 * changed calloc to new
 * getOpstring now returns const char*
 * createPrimitiveCall takes const char*
 *
 */

#include "rtinst/h/rtinst.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"
#include "util.h"
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4) || defined(sparc_tmc_cmost7_3)
#include "inst-sparc.h"
#else
#if defined(hppa1_1_hp_hpux)
#include "inst-hppa.h"
#endif
#endif

extern registerSpace *regSpace;

registerSpace::registerSpace(int deadCount, int *dead, int liveCount, int *live)
{
    int i;

    numRegisters = deadCount + liveCount;
    registers = new registerSlot[numRegisters];

    // load dead ones
    for (i=0; i < deadCount; i++) {
	registers[i].number = dead[i];
	registers[i].inUse = false;
	registers[i].mustRestore = false;
	registers[i].needsSaving = false;
	registers[i].startsLive = false;
    }

    // load live ones;
    for (i=0; i < liveCount; i++) {
	registers[i+deadCount].number = live[i];
	registers[i+deadCount].inUse = false;
	registers[i+deadCount].mustRestore = false;
	registers[i+deadCount].needsSaving = true;
	registers[i+deadCount].startsLive = true;
    }
}

// Certain registers (i0-i7 on a SPARC) may be available to be read
// as an operand, but cannot be written.  
// THIS IS SPARC SPECIFIC
bool registerSpace::readOnlyRegister(reg reg_number) {
  if ((reg_number < 16) || (reg_number > 23)) 
    return true;
  else
    return false;
}


reg registerSpace::allocateRegister(char *insn, unsigned &base) 
{
    int i;
    for (i=0; i < numRegisters; i++) {
	if (!registers[i].inUse && !registers[i].needsSaving) {
	    registers[i].inUse = true;
	    highWaterRegister = (highWaterRegister > i) ? 
		 highWaterRegister : i;
	    return(registers[i].number);
	}
    }

    // now consider ones that need saving
    for (i=0; i < numRegisters; i++) {
	if (!registers[i].inUse) {
	    emit(saveRegOp, registers[i].number, 0, 0, insn, base);
	    registers[i].inUse = true;
	    registers[i].mustRestore = true;
	    // prevent general spill (func call) from saving this register.
	    registers[i].needsSaving = false;
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
	    registers[i].inUse = false;
	    return;
	}
    }
}

bool registerSpace::isFreeRegister(int reg) {
    int i;
    for (i=0; i < numRegisters; i++) {
	if ((registers[i].number == reg) &&
	    (registers[i].inUse == true)) {
	    return false;
	}
    }
    return true;
}

void registerSpace::resetSpace() {
    int i;
    for (i=0; i < numRegisters; i++) {
	registers[i].inUse = false;
	registers[i].mustRestore = false;
	registers[i].needsSaving = registers[i].startsLive;
    }
    highWaterRegister = 0;
}

int AstNode::generateTramp(process *proc, char *i, 
			   unsigned &count, 
			   int trampCost)
{
    int ret;
    int cycles;
    AstNode *preamble;
    static AstNode *trailer = new AstNode(trampTrailer, NULL, NULL);
    // used to estimate cost.
    static AstNode *preambleTemplate = new AstNode(trampPreamble, 
	new AstNode(Constant, (void *) 0), NULL);
    
    cycles = preambleTemplate->cost() + cost() + trailer->cost() + trampCost;

#ifdef notdef
    print();
    sprintf(errorLine, "cost of inst point = %d cycles\n", cycles);
    logLine(errorLine);
#endif

    preamble = new AstNode(trampPreamble, 
	new AstNode(Constant, (void *) cycles), NULL);

    regSpace->resetSpace();

    if ((type != opCodeNode) || (op != noOp)) {
	preamble->generateCode(proc, regSpace, i, count);
	generateCode(proc, regSpace, i, count);
	ret = trailer->generateCode(proc, regSpace, i, count);
    } else {
	return((unsigned) emit(op, 1, 0, 0, i, count));
    }

    delete(preamble);
    return(ret);
}

reg AstNode::generateCode(process *proc,
			  registerSpace *rs,
			  char *insn, 
			  unsigned &base)
{
    unsigned addr;
    unsigned fromAddr;
    unsigned startInsn;
    reg src1, src2;
    reg src = -1;
    reg dest = -1;
    reg right_dest = -1;

    if (type == opCodeNode) {
        if (op == ifOp) {
	    src = loperand->generateCode(proc, rs, insn, base);
	    startInsn = base;
	    fromAddr = emit(op, src, (reg) 0, (reg) 0, insn, base);
	    rs->freeRegister(src);
            (void) roperand->generateCode(proc, rs, insn, base);
	    // call emit again now with correct offset.
	    (void) emit(op, src, (reg) 0, (reg) ((int)base - (int)fromAddr), 
		insn, startInsn);
            // sprintf(errorLine,branch forward %d\n", base - fromAddr);
	} else if (op == storeOp) {
	    src1 = roperand->generateCode(proc, rs, insn, base);
	    src2 = rs->allocateRegister(insn, base);
	    addr = loperand->dValue->getInferiorPtr();
	    (void) emit(op, src1, src2, (reg) addr, insn, base);
	    rs->freeRegister(src1);
	    rs->freeRegister(src2);
	} else if (op == trampTrailer) {
	    return((unsigned) emit(op, 0, 0, 0, insn, base));
	} else if (op == trampPreamble) {
	    int cost;
	    bool err;
	    int costAddr;

	    // loperand is a constant AST node with the cost.
	    cost = (int) loperand->oValue;
	    costAddr = (proc->symbols)->findInternalAddress("DYNINSTobsCostLow",
		true, err);
	    if (err) {
		logLine("unable to find addr of DYNINSTobsCostLow\n");
		P_abort();
	    }
	    return((unsigned) emit(op, cost, 0, costAddr, insn, base));
	} else {
	    if (loperand) 
		src = loperand->generateCode(proc, rs, insn, base);
	    if (roperand)
	        right_dest = roperand->generateCode(proc, rs, insn, base);

	    // the left operand source register can be reused here
	    rs->freeRegister(src);

	    if ((right_dest != -1) && rs->readOnlyRegister(right_dest))
	      dest = rs->allocateRegister(insn, base);
	    else
	      dest = right_dest;

	    (void) emit(op, src, right_dest, dest, insn, base);
	}
    } else if (type == operandNode) {
	dest = rs->allocateRegister(insn, base);
	if (oType == Constant) {
	    (void) emit(loadConstOp, (reg) oValue, dest, dest, insn, base);
	} else if (oType == ConstantPtr) {
	    (void) emit(loadConstOp, (reg) (*(unsigned int *) oValue),
		dest, dest, insn, base);
	} else if (oType == DataPtr) {
	    addr = dValue->getInferiorPtr();
	    (void) emit(loadConstOp, (reg) addr, dest, dest, insn, base);
	} else if (oType == DataValue) {
	    addr = dValue->getInferiorPtr();
	    // Why a special case for TIMER?, shouldn't this be using DataPtr
	    if (dValue->getType() == TIMER) {
		(void) emit(loadConstOp, (reg) addr, dest, dest, insn, base);
	    } else {
		(void) emit(loadOp, (reg) addr, dest, dest, insn, base);
	    }
	} else if (oType == Param) {
	    src = rs->allocateRegister(insn, base);
	    // return the actual reg it is in.
	    dest = emit(getParamOp, oValue, 0, src, insn, base);
	    if (src != dest) {
		rs->freeRegister(src);
	    }
	} else if (oType == DataAddr) {
	  addr = (unsigned) oValue;
	  emit(loadOp, (reg) addr, dest, dest, insn, base);
	}
    } else if (type == callNode) {
	unsigned addr;
	// find func addr.
	bool err;
	addr = (proc->symbols)->findInternalAddress(callee, false, err);
	if (err) {
	  pdFunction *func = (proc->symbols)->findOneFunction(callee);
	  if (!func) {
	    ostrstream os(errorLine, 1024, ios::out);
	    os << "unable to find addr of " << callee << endl;
	    logLine(errorLine);
	    P_abort();
	  }
	  addr = func->addr();
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
	dest = emit(callOp, src1, src2, (int) addr, insn, base);
    } else if (type == sequenceNode) {
	loperand->generateCode(proc, rs, insn, base);
	return(roperand->generateCode(proc, rs, insn, base));
    }
    return(dest);
}

string getOpString(opCode op)
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
	    if (dValue->getType() == TIMER) {
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
	    sprintf(errorLine, " %d", (int) dValue->getInferiorPtr());
	    logLine(errorLine);
	} else if (oType == DataValue) {
	    sprintf(errorLine, "@%d", (int) dValue->getInferiorPtr());
	    logLine(errorLine);
	} else if (oType == Param) {
	    sprintf(errorLine, "param[%d]", (int) oValue);
	    logLine(errorLine);
	}
    } else if (type == opCodeNode) {
        ostrstream os(errorLine, 1024, ios::out);
	os << "(" << getOpString(op) << ends;
	logLine(errorLine);
	if (loperand) loperand->print();
	if (roperand) roperand->print();
	logLine(")");
    } else if (type == callNode) {
        ostrstream os(errorLine, 1024, ios::out);
	os << "(" << callee << ends;
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

AstNode *createPrimitiveCall(const string func, dataReqNode *dataPtr, int param2)
{
    return(new AstNode(func, new AstNode(DataValue, (void *) dataPtr), 
			     new AstNode(Constant, (void *) param2)));
}

AstNode *createIf(AstNode *expression, AstNode *action)
{
    return(new AstNode(ifOp, expression, action));
}

AstNode *createCall(const string func, dataReqNode *dataPtr, AstNode *ast) {
  return (new AstNode(func, new AstNode(DataValue, (void*) dataPtr), ast));
}
