/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/* 
 * $Log: ast.C,v $
 * Revision 1.33  1996/11/12 17:48:34  mjrg
 * Moved the computation of cost to the basetramp in the x86 platform,
 * and changed other platform to keep code consistent.
 * Removed warnings, and made changes for compiling with Visual C++
 *
 * Revision 1.32  1996/11/11 01:45:30  lzheng
 * Moved the instructions which is used to caculate the observed cost
 * from the miniTramps to baseTramp
 *
 * Revision 1.31  1996/10/31 08:36:58  tamches
 * the shm-sampling commit; added noCost param to some fns
 *
 * Revision 1.30  1996/10/04 16:12:38  naim
 * Optimization for code generation (use of immediate operations whenever
 * possible). This first commit is only for the sparc platform. Other platforms
 * should follow soon - naim
 *
 * Revision 1.29  1996/09/13 21:41:57  mjrg
 * Implemented opcode ReturnVal for ast's to get the return value of functions.
 * Added missing calls to free registers in Ast.generateCode and emitFuncCall.
 * Removed architecture dependencies from inst.C.
 * Changed code to allow base tramps of variable size.
 *
 * Revision 1.28  1996/08/21 18:02:35  mjrg
 * Changed the ast nodes generated for timers. This just affects the ast
 * nodes, not the code generated.
 *
 * Revision 1.27  1996/08/20 19:07:30  lzheng
 * Implementation of moving multiple instructions sequence and
 * splitting the instrumentations into two phases.
 *
 * Revision 1.26  1996/08/16 21:18:14  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.25  1996/05/12 05:15:56  tamches
 * aix 4.1 commit
 *
 * Revision 1.24  1996/04/26 20:16:16  lzheng
 * Move part of code in AstNode::generateCode to machine dependent file.
 * (Those code are put into the procedure emitFuncCall)
 *
 * Revision 1.23  1996/04/10 18:00:11  lzheng
 * Added multiple arguments to calls for HPUX by using stack instead of extra
 * registers.
 *
 * Revision 1.22  1996/04/08 21:21:11  lzheng
 * changes for HP generateCode and emitFuncCall
 *
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
#elif defined(hppa1_1_hp_hpux)
#include "inst-hppa.h"
#elif defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
#include "inst-power.h"
#elif defined(i386_unknown_solaris2_5)
#else
#endif

extern registerSpace *regSpace;
extern bool doNotOverflow(int value);

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

reg registerSpace::allocateRegister(char *insn, unsigned &base, bool noCost) 
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
	    emit(saveRegOp, registers[i].number, 0, 0, insn, base, noCost);
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
        if (registers[i].inUse) {
          sprintf(errorLine,"WARNING: register %d is still in use\n",registers[i].number);
          logLine(errorLine);
        }
	registers[i].inUse = false;
	registers[i].mustRestore = false;
	registers[i].needsSaving = registers[i].startsLive;
    }
    highWaterRegister = 0;
}

AstNode &AstNode::operator=(const AstNode &src) {
   if (&src == this)
      return *this; // the usual check for x=x

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

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
   astFlag = src.astFlag;
#endif

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

	// delete roperand, to free up some memory, before reassigning to it
	if (roperand) delete roperand;

	roperand = new AstNode(plusOp, l, r);
	op = storeOp;
    } else if (func == "subCounter") {
	type = opCodeNode;

	// delete roperand, to free up some memory, before reassigning to it
        if (roperand) delete roperand;

	roperand = new AstNode(minusOp, l, r);
	op = storeOp;
     } else {
        // should put some verifying code here
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

	// delete roperand, to free up some memory, before reassigning to it
	if (roperand) delete roperand;

	roperand = new AstNode(plusOp, l); // assumes NULL for right operand
	op = storeOp;
    } else if (func == "subCounter") {
	type = opCodeNode;

	// delete roperand, to free up some memory, before reassigning to it
        if (roperand) delete roperand;

	roperand = new AstNode(minusOp, l); // assumes NULL for right operand
	op = storeOp;
     } else {
        // treat "func" as the name of a fn to call
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
      // should put some checking code here
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
			   int &trampCost, bool noCost) const {
    static AstNode trailer(trampTrailer); // private constructor
    static AstNode preambleTemplate(trampPreamble, 
    	       		           AstNode(Constant, (void *) 0));
        // private constructor; assumes NULL for right child

    trampCost  = preambleTemplate.cost() + cost() + trailer.cost();
    int cycles = trampCost;

#ifdef notdef
    print();
    sprintf(errorLine, "cost of inst point = %d cycles\n", cycles);
    logLine(errorLine);
#endif

    AstNode preamble(trampPreamble, 
                    AstNode(Constant, (void *) cycles));
        // private constructor; assumes NULL for right child

    initTramps(); // needed to initialize regSpace below
                  // shouldn't we do a "delete regSpace" first to avoid
                  // leaking memory?
    regSpace->resetSpace();

    if (type != opCodeNode || op != noOp) {
        reg tmp;
	preamble.generateCode(proc, regSpace, i, count, noCost);
	tmp = generateCode(proc, regSpace, i, count, noCost);
        regSpace->freeRegister(tmp);
	return trailer.generateCode(proc, regSpace, i, count, noCost);
    } else {
	return (unsigned) emit(op, 1, 0, 0, i, count, noCost);
    }
}

bool isPowerOf2(int value, int &result)
{
  if (value==1) {
    result=0;
    return(true);
  }
  if ((value%2)!=0) return(false);
  if (isPowerOf2(value/2,result)) {
    result++;
    return(true);
  }
  else return(false);
}

reg AstNode::generateCode(process *proc,
			  registerSpace *rs,
			  char *insn, 
			  unsigned &base, bool noCost) const {
    unsigned addr;
    unsigned fromAddr;
    unsigned startInsn;
    reg src1, src2;
    reg src = -1;
    reg dest = -1;
    reg right_dest = -1;

    if (type == opCodeNode) {
        if (op == ifOp) {
	    src = loperand->generateCode(proc, rs, insn, base, noCost);
	    startInsn = base;
	    fromAddr = emit(op, src, (reg) 0, (reg) 0, insn, base, noCost);
            rs->freeRegister(src);
            reg tmp = roperand->generateCode(proc, rs, insn, base, noCost);
            rs->freeRegister(tmp);

	    // call emit again now with correct offset.
	    (void) emit(op, src, (reg) 0, (reg) ((int)base - (int)fromAddr), 
			insn, startInsn, noCost);
            // sprintf(errorLine,branch forward %d\n", base - fromAddr);
	} else if (op == storeOp) {
	    src1 = roperand->generateCode(proc, rs, insn, base, noCost);
	    src2 = rs->allocateRegister(insn, base, noCost);
	    addr = loperand->dValue->getInferiorPtr();
	    assert(addr != 0); // check for NULL
	    (void) emit(op, src1, src2, (reg) addr, insn, base, noCost);
	    rs->freeRegister(src1);
	    rs->freeRegister(src2);
	} else if (op == trampTrailer) {
	    return (unsigned) emit(op, 0, 0, 0, insn, base, noCost);

	} else if (op == trampPreamble) {
	    return (unsigned) emit(op, 0, 0, 0, insn, base, noCost);

	} else {
	    AstNode *left = loperand;
	    AstNode *right = roperand;

	    if (left && right) {
              if (left->type == operandNode && left->oType == Constant) {
                if (type == opCodeNode) {
                  if (op == plusOp) {
                    logLine("plusOp exchange\n");
  	            AstNode *temp = right;
	            right = left;
	            left = temp;
	          } else if (op == timesOp) {
                    if (right->type == operandNode) {
                      if (right->oType != Constant) {
                        logLine("timesOp exchange 1\n");
                        AstNode *temp = right;
	                right = left;
	                left = temp;
		      }
                      else {
                        int result;
                        if (!isPowerOf2((int)right->oValue,result) &&
                             isPowerOf2((int)left->oValue,result))
                        {
                          logLine("timesOp exchange 2\n");
                          AstNode *temp = right;
	                  right = left;
	                  left = temp;
		        }
		      }
		    }
		  }
	        }
	      }
	    }

	    if (left)
	      src = left->generateCode(proc, rs, insn, base, noCost);

	    rs->freeRegister(src);
	    dest = rs->allocateRegister(insn, base, noCost);

            if (right && (right->type == operandNode) &&
                (right->oType == Constant) &&
                doNotOverflow((int)right->oValue) &&
                (type == opCodeNode))
	    {
	      emitImm(op, src, (reg)right->oValue, dest, insn, base, noCost);
	    }
	    else {
	      if (right)
		right_dest = right->generateCode(proc, rs, insn, base, noCost);
              rs->freeRegister(right_dest);
	      (void) emit(op, src, right_dest, dest, insn, base, noCost);
	    }
	}
    } else if (type == operandNode) {
	dest = rs->allocateRegister(insn, base, noCost);
	if (oType == Constant) {
	    (void) emit(loadConstOp, (reg) oValue, dest, dest, insn, base, noCost);
	} else if (oType == ConstantPtr) {
	    (void) emit(loadConstOp, (reg) (*(unsigned int *) oValue),
		dest, dest, insn, base, noCost);
	} else if (oType == DataPtr) {
	    addr = dValue->getInferiorPtr();
	    assert(addr != 0); // check for NULL
	    (void) emit(loadConstOp, (reg) addr, dest, dest, insn, base, noCost);
	} else if (oType == DataValue) {
	    addr = dValue->getInferiorPtr();

	    assert(addr != 0); // check for NULL
	    (void) emit(loadOp, (reg) addr, dest, dest, insn, base, noCost);
	} else if (oType == ReturnVal) {
	    rs->freeRegister(dest);
	    src = rs->allocateRegister(insn, base, noCost);
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
	    if (astFlag)
		dest = emit(getSysRetValOp, 0, 0, src, insn, base, noCost);
	    else 
#endif
	        dest = emit(getRetValOp, 0, 0, src, insn, base, noCost);
	    if (src != dest) {
		rs->freeRegister(src);
	    }
	} else if (oType == Param) {
	    rs->freeRegister(dest);
	    src = rs->allocateRegister(insn, base, noCost);
	    // return the actual reg it is in.
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
	    if (astFlag)
		dest = emit(getSysParamOp, (reg)oValue, 0, src, insn, base, noCost);
	    else 
#endif
		dest = emit(getParamOp, (reg)oValue, 0, src, insn, base, noCost);
	    if (src != dest) {
		rs->freeRegister(src);
	    }
	} else if (oType == DataAddr) {
	  addr = (unsigned) oValue;
	  emit(loadOp, (reg) addr, dest, dest, insn, base, noCost);
	}
    } else if (type == callNode) {
	dest = emitFuncCall(callOp, rs, insn, base, operands, callee, proc, noCost);
    } else if (type == sequenceNode) {
	(void) loperand->generateCode(proc, rs, insn, base, noCost);
 	return(roperand->generateCode(proc, rs, insn, base, noCost));
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
        case shiftOp: return("shift");
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
        } else if (op == shiftOp) {
            total += getInsnCost(op);
	} else if (op == trampTrailer) {
	    total = getInsnCost(op);
	} else if (op == trampPreamble) {
	    total = getInsnCost(op);
	} else {
	    if (loperand) 
		total += loperand->cost();
	    if (roperand) 
		total += roperand->cost();
	    total += getInsnCost(op);
	}
    } else if (type == operandNode) {
	if (oType == Constant) {
	    total = getInsnCost(loadConstOp);
	} else if (oType == DataPtr) {
	    total = getInsnCost(loadConstOp);
	} else if (oType == DataValue) {
	    total = getInsnCost(loadOp);
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
   return AstNode(func, AstNode(AstNode::DataValue, (void *)dataPtr),
		        AstNode(AstNode::Constant, (void *)param2)
		  );
}

AstNode createIf(const AstNode &expression, const AstNode &action) {
   return AstNode(ifOp, expression, action);
}

AstNode createCall(const string &func, dataReqNode *dataPtr, const AstNode &ast) {
   return AstNode(func,
		  AstNode(AstNode::DataValue, (void *)dataPtr),
		  ast);
}
