
/* 
 * $Log: ast.C,v $
 * Revision 1.21  1996/03/25 20:20:39  tamches
 * the reduce-mem-leaks-in-paradynd commit
 *
 * Revision 1.20  1996/03/20 17:02:36  mjrg
 * Added multiple arguments to calls.
 * Instrument pvm_send instead of pvm_recv to get tags.
 *
 * Revision 1.19  1995/12/19 01:04:44  hollings
 * Moved the implementation of registerSpace::readOnlyRegister to processor
 *   specific files (since it is).
 * Fixed a bug in Power relOps cases.
 *
 * Revision 1.18  1995/09/26  20:33:02  naim
 * Adding error messages using function showErrorCallback for paradynd
 *
 * Revision 1.17  1995/08/24  15:03:44  hollings
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
#include "showerror.h"
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

AstNode &AstNode::operator=(const AstNode &src) {
   if (&src == this)
      return *this;

   // clean up self before overwriting self; i.e., release memory
   // currently in use so it doesn't become leaked.
   if (loperand)
      delete loperand;
   if (roperand)
      delete roperand;

   type = src.type;
   if (type == opCodeNode)
      op = src.op; // defined only for operand nodes

   if (type == callNode) {
      callee = src.callee; // defined only for call nodes
      operands = src.operands; // defined only for call nodes
   }

   if (type == operandNode) {
      oType = src.oType;
      if (oType == DataPtr || oType == DataValue)
         dValue = src.dValue;
      else
         oValue = src.oValue;
   }

   loperand = src.loperand ? new AstNode(*src.loperand) : NULL;
   roperand = src.roperand ? new AstNode(*src.roperand) : NULL;

   firstInsn = src.firstInsn;
   lastInsn = src.lastInsn;

   return *this;
}

AstNode::AstNode() {
   // used in mdl.C
   loperand = roperand = NULL;
   // "operands" is left as an empty vector

   //assert(0);
}

AstNode::AstNode(const string &func, const AstNode &l, const AstNode &r) {
    loperand = new AstNode(l);
    roperand = new AstNode(r);

    if (func == "setCounter") {
	type = opCodeNode;
	op = storeOp;
    } else if (func == "addCounter") {
        type = opCodeNode;

	// should we "delete roperand", to free up some memory, before reassigning
        // to roperand? --ari
	if (roperand) delete roperand;

	roperand = new AstNode(plusOp, l, r);
	op = storeOp;
    } else if (func == "subCounter") {
	type = opCodeNode;

	// should we "delete roperand", to free up some memory, before reassigning
        // to roperand? --ari
        if (roperand) delete roperand;

	roperand = new AstNode(minusOp, l, r);
	op = storeOp;
     } else {
        type = callNode;
	callee = func;
	operands += l;
	operands += r;
     }
}

AstNode::AstNode(const string &func, const AstNode &l) {
    loperand = new AstNode(l);
    roperand = NULL;

    if (func == "setCounter") {
	type = opCodeNode;
	op = storeOp;
    } else if (func == "addCounter") {
        type = opCodeNode;

	// should we "delete roperand", to free up some memory, before reassigning
        // to roperand? --ari
	if (roperand) delete roperand;

	roperand = new AstNode(plusOp, l); // assumes NULL for right operand
	op = storeOp;
    } else if (func == "subCounter") {
	type = opCodeNode;

	// should we "delete roperand", to free up some memory, before reassigning
        // to roperand? --ari
        if (roperand) delete roperand;

	roperand = new AstNode(minusOp, l); // assumes NULL for right operand
	op = storeOp;
     } else {
        type = callNode;
	callee = func;
	operands += l;
     }
}

AstNode::AstNode(const string &func, vector<AstNode> &ast_args) {
   if (func == "setCounter" || func == "addCounter" || func == "subCounter") {
      loperand = new AstNode(ast_args[0]);
      roperand = new AstNode(ast_args[1]);
   } else {
      operands = ast_args;
      loperand = roperand = NULL; // ???
   }

   if (func == "setCounter") {
      type = opCodeNode;
      op = storeOp;
   } else if (func == "addCounter") {
      type = opCodeNode;
      if (roperand) delete roperand;
      roperand = new AstNode(plusOp, *loperand, *roperand);
      op = storeOp;
   } else if (func == "subCounter") {
      type = opCodeNode;
      if (roperand) delete roperand;
      roperand = new AstNode(minusOp, *loperand, *roperand);
      op = storeOp;
   } else {
      type = callNode;
      callee = func;
   }
}

AstNode::AstNode(operandType ot, void *arg) {
    type = operandNode;
    oType = ot;
    if (oType == DataPtr || oType == DataValue)
	dValue = (dataReqNode *) arg;
    else
	oValue = (void *) arg;

    loperand = roperand = NULL;
};

AstNode::AstNode(const AstNode &l, const AstNode &r) {
   type = sequenceNode;
   loperand = new AstNode(l);
   roperand = new AstNode(r);
};

AstNode::AstNode(opCode ot) {
   // a private constructor
   type = opCodeNode;
   op = ot;
   loperand = roperand = NULL;
}

AstNode::AstNode(opCode ot, const AstNode &l) {
   // a private constructor
   type = opCodeNode;
   op = ot;
   loperand = new AstNode(l);
   roperand = NULL;
}

AstNode::AstNode(opCode ot, const AstNode &l, const AstNode &r) {
   type = opCodeNode;
   op = ot;
   loperand = new AstNode(l);
   roperand = new AstNode(r);
};

AstNode::AstNode(const AstNode &src) {
   type = src.type;

   if (type == opCodeNode)
      op = src.op; // defined only for operand nodes

   if (type == callNode) {
      callee = src.callee; // defined only for call nodes
      operands = src.operands; // defined only for call nodes 
   }

   if (type == operandNode) {
      oType = src.oType;
      if (oType == DataPtr || oType == DataValue)
         dValue = src.dValue;
      else
	 oValue = src.oValue;
   }

   loperand = src.loperand ? new AstNode(*src.loperand) : NULL;
   roperand = src.roperand ? new AstNode(*src.roperand) : NULL;

   firstInsn = src.firstInsn;
   lastInsn = src.lastInsn;
}

AstNode::~AstNode() {
   if (loperand != NULL)
      delete loperand;

   if (roperand != NULL)
      delete roperand;
}

int AstNode::generateTramp(process *proc, char *i, 
			   unsigned &count, 
			   int trampCost) const {
    int cycles;

    //static AstNode *trailer = new AstNode(trampTrailer, NULL, NULL);
    static AstNode trailer(trampTrailer); // private constructor
    // used to estimate cost.
    static AstNode preambleTemplate(trampPreamble, 
				    AstNode(Constant, (void *) 0));
       // private constructor; assumes NULL for right child
    
    cycles = preambleTemplate.cost() + cost() + trailer.cost() + trampCost;

#ifdef notdef
    print();
    sprintf(errorLine, "cost of inst point = %d cycles\n", cycles);
    logLine(errorLine);
#endif

    AstNode preamble(trampPreamble, 
		     AstNode(Constant, (void *) cycles));
       // private constructor; assumes NULL for right child

    regSpace->resetSpace();

    if (type != opCodeNode || op != noOp) {
	preamble.generateCode(proc, regSpace, i, count);
	generateCode(proc, regSpace, i, count);
	return trailer.generateCode(proc, regSpace, i, count);
    } else {
	return((unsigned) emit(op, 1, 0, 0, i, count));
    }
}

reg AstNode::generateCode(process *proc,
			  registerSpace *rs,
			  char *insn, 
			  unsigned &base) const {
    unsigned addr;
    unsigned fromAddr;
    unsigned startInsn;
    reg src1, src2;
    reg src = -1;
    reg dest = -1;
    reg right_dest = -1;
    vector <reg> srcs;

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
		logLine("Internal error: unable to find addr of DYNINSTobsCostLow\n");
		showErrorCallback(79, "");
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
	    dest = emit(getParamOp, (reg)oValue, 0, src, insn, base);
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
	    os << "Internal error: unable to find addr of " << callee << endl;
	    logLine(errorLine);
	    showErrorCallback(80, (const char *) errorLine);
	    P_abort();
	  }
	  addr = func->addr();
	}

	for (unsigned u = 0; u < operands.size(); u++)
	  srcs += operands[u].generateCode(proc, rs, insn, base);
	dest = emitFuncCall(callOp, srcs, (int) addr, insn, base);
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

int AstNode::cost() const {
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
	    total = getInsnCost(getParamOp);
	}
    } else if (type == callNode) {
	total = getPrimitiveCost(callee);
	for (unsigned u = 0; u < operands.size(); u++)
	  total += operands[u].cost();
    } else if (type == sequenceNode) {
	total += loperand->cost();
	total += roperand->cost();
    }
    return(total);
}

void AstNode::print() const {
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
	for (unsigned u = 0; u < operands.size(); u++)
	  operands[u].print();
	logLine(")");
    } else if (type == sequenceNode) {
	if (loperand) loperand->print();
	logLine(",");
	if (roperand) roperand->print();
    }
}

AstNode createPrimitiveCall(const string &func, dataReqNode *dataPtr, int param2) {
   return AstNode(func, AstNode(DataValue, (void *)dataPtr),
		        AstNode(Constant, (void *)param2)
		  );
}

AstNode createIf(const AstNode &expression, const AstNode &action) {
   return AstNode(ifOp, expression, action);
}

AstNode createCall(const string &func, dataReqNode *dataPtr, const AstNode &ast) {
   return AstNode(func,
		  AstNode(DataValue, (void *)dataPtr),
		  ast);
}
