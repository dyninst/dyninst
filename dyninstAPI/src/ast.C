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
 * Revision 1.37  1997/02/21 20:13:16  naim
 * Moving files from paradynd to dyninstAPI + moving references to dataReqNode
 * out of the ast class. The is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.36  1997/01/27 19:40:36  naim
 * Part of the base instrumentation for supporting multithreaded applications
 * (vectors of counter/timers) implemented for all current platforms +
 * different bug fixes - naim
 *
 * Revision 1.35  1996/11/14 14:42:52  naim
 * Minor fix to my previous commit - naim
 *
 * Revision 1.34  1996/11/14 14:26:57  naim
 * Changing AstNodes back to pointers to improve performance - naim
 *
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
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/dyninstP.h"
#include "paradynd/src/metric.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "paradynd/src/showerror.h"

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
#include "dyninstAPI/src/inst-sparc.h"
#elif defined(hppa1_1_hp_hpux)
#include "dyninstAPI/src/inst-hppa.h"
#elif defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
#include "dyninstAPI/src/inst-power.h"
#elif defined(i386_unknown_solaris2_5)
#include "dyninstAPI/src/inst-x86.h"
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
#if defined(MT_THREAD)
        if (registers[i+deadCount].number == REG_MT) {
          registers[i+deadCount].inUse = true;
          registers[i+deadCount].needsSaving = true;
        }
#endif
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
#if !defined(rs6000_ibm_aix4_1)
            // MT_AIX: we are not saving registers on demand on the power
            // architecture. Instead, we save/restore registers in the base
            // trampoline - naim
 	    emit(saveRegOp, registers[i].number, 0, 0, insn, base, noCost);
#endif
	    registers[i].inUse = true;
#if !defined(rs6000_ibm_aix4_1)
            // MT_AIX
	    registers[i].mustRestore = true;
#endif
	    // prevent general spill (func call) from saving this register.
	    registers[i].needsSaving = false;
	    highWaterRegister = (highWaterRegister > i) ? 
		 highWaterRegister : i;
	    return(registers[i].number);
	}
    }

    logLine("==> WARNING! run out of registers...\n");
    abort();
    return(-1);
}

bool registerSpace::is_keep_register(reg k)
{
  for (unsigned i=0;i<keep_list.size();i++) {
    if (keep_list[i]==k) return(true);
  }
  return(false);
}

void registerSpace::keep_register(reg k)
{
  if (!is_keep_register(k)) {
    keep_list += k;
#if defined(ASTDEBUG)
    sprintf(errorLine,"==> keeping register %d, size is %d <==\n",k,keep_list.size());
    logLine(errorLine);
#endif
  }
}

void registerSpace::unkeep_register(reg k) {
  unsigned ksize = keep_list.size();
  for (unsigned i=0;i<ksize;i++) {
    if (keep_list[i]==k) {
      keep_list[i]=keep_list[ksize-1];
      ksize--;
      keep_list.resize(ksize);
      freeRegister(k);
#if defined(ASTDEBUG)
      sprintf(errorLine,"==> un-keeping register %d, size is %d <==\n",k,keep_list.size());
      logLine(errorLine);
#endif
      break;
    }
  }
}

void registerSpace::freeRegister(int reg) {
    int i;
    if (is_keep_register(reg)) return;
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
        if (registers[i].inUse && (registers[i].number != REG_MT)) {
          sprintf(errorLine,"WARNING: register %d is still in use\n",registers[i].number);
          logLine(errorLine);
        }
	registers[i].inUse = false;
	registers[i].mustRestore = false;
	registers[i].needsSaving = registers[i].startsLive;
#if defined(MT_THREAD)
        if (registers[i].number == REG_MT) {
          registers[i].inUse = true;
          registers[i].needsSaving = true;
        }
#endif
    }
    highWaterRegister = 0;
}

//
// How to use AstNodes:
//
// In order to avoid memory leaks, it is important to define and delete
// AstNodes properly. The general rules are the following:
//
// 1.- Any AstNode defined locally, should be destroyed at the end of that
//     procedure. The only exception occurs when we are returning a pointer
//     to the AstNode as a result of the function (i.e. we need to keep the
//     value alive).
// 2.- Every time we assign an AstNode to another, we have to use the
//     "assignAst" function. This function will update the reference count
//     of the AstNode being assigned and it will return a pointer to it. If
//     we are creating a new AstNode (e.g. AstNode *t1 = new AstNode(...))
//     then it is not necessary to use assign, because the constructor will
//     automatically increment the reference count for us.
// 3.- "removeAst" is the procedure to be used everytime we want to delete
//     an AstNode. In general, if an AstNode is re-used several times, it
//     should be enough to delete the root of the DAG to delete all nodes.
//     However, there are exceptions like this one:
//     AstNode *t1, *t2, *t3;
//     t1 = AstNode(...);   rc-t1=1
//     t2 = AstNode(...);   rc-t2=1
//     t3 = AstNode(t1,t2); rc-t1=2, rc-t2=2, rc-t3=1
//     if we say:
//     removeAst(t3);
//     it will delete t3, but not t1 or t2 (because the rc will be 1 for both
//     of them). Therefore, we need to add the following:
//     removeAst(t1); removeAst(t2);
//     We only delete AstNodes when the reference count is 0.
//

AstNode &AstNode::operator=(const AstNode &src) {
   logLine("Calling AstNode COPY constructor\n");
   if (&src == this)
      return *this; // the usual check for x=x

   // clean up self before overwriting self; i.e., release memory
   // currently in use so it doesn't become leaked.
   if (loperand) {
      if (src.loperand) {
        if (loperand!=src.loperand) {
          removeAst(loperand);
        }
      } else {
        removeAst(loperand);
      }
   }
   if (roperand) {
      if (src.roperand) {
        if (roperand!=src.roperand) {
          removeAst(roperand);
        }
      } else {
        removeAst(roperand);
      }
   }
   referenceCount = src.referenceCount;
   referenceCount++;

   type = src.type;
   if (type == opCodeNode)
      op = src.op; // defined only for operand nodes

   if (type == callNode) {
      callee = src.callee; // defined only for call nodes
      for (unsigned i=0;i<src.operands.size();i++) 
        operands += assignAst(src.operands[i]);
   }

   if (type == operandNode) {
      oType = src.oType;
      oValue = src.oValue;
   }

   loperand = assignAst(src.loperand);
   roperand = assignAst(src.roperand);

   firstInsn = src.firstInsn;
   lastInsn = src.lastInsn;

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
   astFlag = src.astFlag;
#endif

   return *this;
}

#if defined(ASTDEBUG)
static int ASTcount=0;

void ASTcounter()
{
  ASTcount++;
  sprintf(errorLine,"AstNode CONSTRUCTOR - ASTcount is %d\n",ASTcount);
  logLine(errorLine);
}

void ASTcounterNP()
{
  ASTcount++;
}
#endif

AstNode::AstNode() {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
   // used in mdl.C
   type = opCodeNode;
   op = noOp;
   loperand = roperand = NULL;
   referenceCount = 1;
   useCount = 0;
   kept_register = -1;
   // "operands" is left as an empty vector
}

AstNode::AstNode(const string &func, AstNode *l, AstNode *r) {
#if defined(ASTDEBUG)
    ASTcounter();
#endif
    referenceCount = 1;
    useCount = 0;
    kept_register = -1;
    type = callNode;
    callee = func;
    if (l) operands += assignAst(l);
    if (r) operands += assignAst(r);
}

AstNode::AstNode(const string &func, AstNode *l) {
#if defined(ASTDEBUG)
    ASTcounter();
#endif
    referenceCount = 1;
    useCount = 0;
    kept_register = -1;
    loperand = assignAst(l);
    roperand = NULL;
    type = callNode;
    callee = func;
    if (l) operands += assignAst(l);
}

AstNode::AstNode(const string &func, vector<AstNode *> &ast_args) {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
   referenceCount = 1;
   useCount = 0;
   kept_register = -1;
   for (unsigned i=0;i<ast_args.size();i++) 
     if (ast_args[i]) operands += assignAst(ast_args[i]);
   loperand = roperand = NULL;
   type = callNode;
   callee = func;
}

AstNode::AstNode(operandType ot, void *arg) {
#if defined(ASTDEBUG)
    ASTcounterNP();
#endif
    referenceCount = 1;
    useCount = 0;
    kept_register = -1;
    type = operandNode;
    oType = ot;
    oValue = (void *) arg;
    loperand = roperand = NULL;
};

AstNode::AstNode(operandType ot, AstNode *l) {
#if defined(ASTDEBUG)
    ASTcounter();
#endif
    referenceCount = 1;
    useCount = 0;
    kept_register = -1;
    type = operandNode;
    oType = ot;
    oValue = NULL;
    roperand = NULL;
    loperand = assignAst(l);
};

AstNode::AstNode(AstNode *l, AstNode *r) {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
   referenceCount = 1;
   useCount = 0;
   kept_register = -1;
   type = sequenceNode;
   loperand = assignAst(l);
   roperand = assignAst(r);
};

AstNode::AstNode(opCode ot) {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
   // a private constructor
   referenceCount = 1;
   useCount = 0;
   kept_register = -1;
   type = opCodeNode;
   op = ot;
   loperand = roperand = NULL;
}

AstNode::AstNode(opCode ot, AstNode *l) {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
   // a private constructor
   referenceCount = 1;
   useCount = 0;
   kept_register = -1;
   type = opCodeNode;
   op = ot;
   loperand = assignAst(l);
   roperand = NULL;
}

AstNode::AstNode(opCode ot, AstNode *l, AstNode *r) {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
   referenceCount = 1;
   useCount = 0;
   kept_register = -1;
   type = opCodeNode;
   op = ot;
   loperand = assignAst(l);
   roperand = assignAst(r);
};

AstNode::AstNode(AstNode *src) {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
   referenceCount = 1;
   useCount = 0;
   kept_register = -1;

   type = src->type;   
   if (type == opCodeNode)
      op = src->op;             // defined only for operand nodes

   if (type == callNode) {
      callee = src->callee;     // defined only for call nodes
      for (unsigned i=0;i<src->operands.size();i++) {
        if (src->operands[i]) operands += assignAst(src->operands[i]);
      }
   }

   if (type == operandNode) {
      oType = src->oType;
      oValue = src->oValue;
   }

   loperand = assignAst(src->loperand);
   roperand = assignAst(src->roperand);
   firstInsn = src->firstInsn;
   lastInsn = src->lastInsn;
}

void AstNode::printRC()
{
    sprintf(errorLine,"RC referenceCount=%d\n",referenceCount);
    logLine(errorLine);
    if (loperand) {
      logLine("RC loperand\n");
      loperand->printRC();
    }
    if (roperand) {
      logLine("RC roperand\n");
      roperand->printRC();
    }
}

AstNode::~AstNode() {
#if defined(ASTDEBUG)
  ASTcount--;
  sprintf(errorLine,"AstNode DESTRUCTOR - ASTcount is %d\n",ASTcount);
  logLine(errorLine);
#endif
  if (loperand) {
    removeAst(loperand);
  }
  if (roperand) {
    removeAst(roperand);
  }
  if (type==callNode) {
    for (unsigned i=0;i<operands.size();i++) {
      removeAst(operands[i]);
    }
  }
}

//
// Increments/decrements the reference counter for the operands of a call 
// node. If "flag" is true, it increments the counter. Otherwise, it 
// decrements it.
//
void AstNode::updateOperandsRC(bool flag)
{
  if (type==callNode) {
    for (unsigned i=0;i<operands.size();i++) {
      if (operands[i]) {
        if (flag) operands[i]->referenceCount++;
        else operands[i]->referenceCount--;
      }
    }
  }
}

//
// This procedure should be use every time we assign an AstNode pointer,
// because it increments the reference counter.
//
AstNode *assignAst(AstNode *src) {
  if (src) {
    src->referenceCount++;
    src->updateOperandsRC(true);
  }
  return(src);
}

//
// Decrements the reference count for "ast". If it is "0", it calls the 
// AstNode destructor.
//
void removeAst(AstNode *&ast) {
  if (ast) {
    assert(ast->referenceCount>0);
    ast->referenceCount--;
    if (ast->referenceCount==0) {
      delete ast;
      ast=NULL;
    } else {
      ast->updateOperandsRC(false);
    }
  }
}

//
// This procedure decrements the reference count for "ast" until it is 0.
//
void terminateAst(AstNode *&ast) {
  while (ast) {
    removeAst(ast);
  }
}

int AstNode::generateTramp(process *proc, char *i, 
			   unsigned &count, 
			   int &trampCost, bool noCost) {
    static AstNode *trailer=NULL;
    if (!trailer) trailer = new AstNode(trampTrailer); // private constructor
                                                       // used to estimate cost
    static AstNode *preambleTemplate=NULL;
    if (!preambleTemplate) {
      AstNode *tmp1 = new AstNode(Constant, (void *) 0);
      preambleTemplate = new AstNode(trampPreamble, tmp1);
      removeAst(tmp1);
    }
    // private constructor; assumes NULL for right child
    
    trampCost = preambleTemplate->cost() + cost() + trailer->cost();
    int cycles = trampCost;

    AstNode *preamble, *tmp2;
    tmp2 = new AstNode(Constant, (void *) cycles);
    preamble = new AstNode(trampPreamble, tmp2); 
    removeAst(tmp2);
    // private constructor; assumes NULL for right child

    initTramps(); // needed to initialize regSpace below
                  // shouldn't we do a "delete regSpace" first to avoid
                  // leaking memory?

    regSpace->resetSpace();
    reg return_reg;
    if (type != opCodeNode || op != noOp) {
        reg tmp;
	preamble->generateCode(proc, regSpace, i, count, noCost);
        removeAst(preamble);
	tmp = generateCode(proc, regSpace, i, count, noCost);
        regSpace->freeRegister(tmp);
        return_reg = trailer->generateCode(proc, regSpace, i, count, noCost);
    } else {
        removeAst(preamble);
        return_reg = (unsigned) emit(op, 1, 0, 0, i, count, noCost);
    }
    regSpace->resetSpace();
    return(return_reg);
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

void AstNode::setUseCount(void)
{
  useCount++;
  if (useCount>1) return;
  kept_register=-1;
  if (loperand) loperand->setUseCount();
  if (roperand) roperand->setUseCount();
}

void AstNode::cleanUseCount(void)
{
  useCount=0;
  kept_register=-1;
  if (loperand) loperand->cleanUseCount();
  if (roperand) roperand->cleanUseCount();
}

void AstNode::printUseCount(void)
{
  static int i=0;
  i++;
  sprintf(errorLine,"(%d)=>useCount is %d\n",i,useCount);
  logLine(errorLine);
  if (loperand) {
    sprintf(errorLine,"(%d)=>loperand\n",i);
    logLine(errorLine);
    loperand->printUseCount();
  }
  if (roperand) {
    sprintf(errorLine,"(%d)=>roperand\n",i);
    logLine(errorLine);
    roperand->printUseCount();  
  }
}

//
// This procedure generates code for an AST DAG. If there is a sub-graph
// being shared between more than 1 node, then the code is generated only
// once for this sub-graph and the register where the return value of the
// sub-graph is stored, is kept allocated until the last node sharing the
// sub-graph has used it (freeing it afterwards). A count called "useCount"
// is used to determine whether a particular node or sub-graph is being
// shared. At the end of the call to generate code, this count must be 0
// for every node. Another important issue to notice is that we have to make
// sure that if a node is not calling generate code recursively for either
// its left or right operands, we then need to make sure that we update the
// "useCount" for these nodes (otherwise we might be keeping registers
// allocated without reason). 
//
// This code was modified in order to set the proper "useCount" for every
// node in the DAG before calling the original generateCode procedure (now
// generateCode_phase2). This means that we are traversing the DAG twice,
// but with the advantage of potencially generating more efficient code.
//
// Note: a complex Ast DAG might require more registers than the ones 
// currently available. In order to fix this problem, we will need to 
// implement a "virtual" register allocator - naim 11/06/96
//
reg AstNode::generateCode(process *proc,
			  registerSpace *rs,
			  char *insn, 
			  unsigned &base, bool noCost) {
  cleanUseCount();
  setUseCount();
  reg tmp=generateCode_phase2(proc,rs,insn,base,noCost);
  return(tmp);
}

reg AstNode::generateCode_phase2(process *proc,
			         registerSpace *rs,
			         char *insn, 
			         unsigned &base, bool noCost) {
    unsigned addr;
    unsigned fromAddr;
    unsigned startInsn;
    reg src1, src2;
    reg src = -1;
    reg dest = -1;
    reg right_dest = -1;

    useCount--;
    if (kept_register>=0) {
#if defined(ASTDEBUG)
      sprintf(errorLine,"==> Returning register %d <==\n",kept_register);
      logLine(errorLine);
#endif
      if (useCount==0) {
        rs->unkeep_register(kept_register);
        reg tmp=kept_register;
        kept_register=-1;
        return(tmp);
      }
      return(kept_register);
    }

    if (type == opCodeNode) {
        if (op == ifOp) {
            // This ast cannot be shared because it doesn't return a register
	    src = loperand->generateCode_phase2(proc, rs, insn, base, noCost);
	    startInsn = base;
	    fromAddr = emit(op, src, (reg) 0, (reg) 0, insn, base, noCost);
            rs->freeRegister(src);
            reg tmp = roperand->generateCode_phase2(proc, rs, insn, base, noCost);
            rs->freeRegister(tmp);

	    // call emit again now with correct offset.
	    (void) emit(op, src, (reg) 0, (reg) ((int)base - (int)fromAddr), 
			insn, startInsn, noCost);
            // sprintf(errorLine,branch forward %d\n", base - fromAddr);
	} else if (op == storeOp) {
            // This ast cannot be shared because it doesn't return a register
            // Check loperand because we are not generating code for it on
            // this node.
            loperand->useCount--;
            if (loperand->useCount==0 && loperand->kept_register>=0) {
              rs->unkeep_register(loperand->kept_register);
              loperand->kept_register=-1;
	    }
	    src1 = roperand->generateCode_phase2(proc, rs, insn, base, noCost);
	    src2 = rs->allocateRegister(insn, base, noCost);
	    addr = (unsigned) loperand->oValue;
	    assert(addr != 0); // check for NULL
	    (void) emit(op, src1, src2, (reg) addr, insn, base, noCost);
	    rs->freeRegister(src1);
	    rs->freeRegister(src2);
	} else if (op == storeIndirOp) {
	    src1 = roperand->generateCode_phase2(proc, rs, insn, base, noCost);
            dest = loperand->generateCode_phase2(proc, rs, insn, base, noCost);
            (void) emit(op, src1, 0, dest, insn, base, noCost);          
	    rs->freeRegister(src1);
            rs->freeRegister(dest);
	} else if (op == trampTrailer) {
            // This ast cannot be shared because it doesn't return a register
	    return((unsigned) emit(op, 0, 0, 0, insn, base, noCost));
	} else if (op == trampPreamble) {
            // This ast cannot be shared because it doesn't return a register
#ifdef i386_unknown_solaris2_5
	    // loperand is a constant AST node with the cost, in cycles.
	    int cost = noCost ? 0 : (int) loperand->oValue;
            int costAddr = 0; // for now... (won't change if noCost is set)
            loperand->useCount--;

#ifndef SHM_SAMPLING
	    bool err;
	    costAddr = proc->findInternalAddress("DYNINSTobsCostLow", true, err);
	    if (err) {
//		logLine("Internal error: unable to find addr of DYNINSTobsCostLow\n");
		showErrorCallback(79, "");
		P_abort();
	    }
#else
	    if (!noCost)
	       costAddr = (int)proc->getObsCostLowAddrInApplicSpace();
#endif
	    return (unsigned) emit(op, 0, 0, 0, insn, base, noCost);
#endif
	} else {
	    AstNode *left = assignAst(loperand);
	    AstNode *right = assignAst(roperand);

	    if (left && right) {
              if (left->type == operandNode && left->oType == Constant) {
                if (type == opCodeNode) {
                  if (op == plusOp) {
  	            AstNode *temp = assignAst(right);
	            right = assignAst(left);
	            left = assignAst(temp);
                    removeAst(temp);
	          } else if (op == timesOp) {
                    if (right->type == operandNode) {
                      if (right->oType != Constant) {
                        AstNode *temp = assignAst(right);
	                right = assignAst(left);
	                left = assignAst(temp);
                        removeAst(temp);
		      }
                      else {
                        int result;
                        if (!isPowerOf2((int)right->oValue,result) &&
                             isPowerOf2((int)left->oValue,result))
                        {
                          AstNode *temp = assignAst(right);
	                  right = assignAst(left);
	                  left = assignAst(temp);
                          removeAst(temp);
		        }
		      }
		    }
		  }
	        }
	      }
	    }

	    if (left)
	      src = left->generateCode_phase2(proc, rs, insn, base, noCost);

	    rs->freeRegister(src);
	    dest = rs->allocateRegister(insn, base, noCost);
            if (useCount>0) {
              kept_register=dest;
              rs->keep_register(dest);
	    }

            if (right && (right->type == operandNode) &&
                (right->oType == Constant) &&
                doNotOverflow((int)right->oValue) &&
                (type == opCodeNode))
	    {
	      emitImm(op, src, (reg)right->oValue, dest, insn, base, noCost);
              right->useCount--;
              if (right->useCount==0 && right->kept_register>=0) {
                rs->unkeep_register(right->kept_register);
                right->kept_register=-1;
	      }
	    }
	    else {
	      if (right)
		right_dest = right->generateCode_phase2(proc, rs, insn, base, noCost);
              rs->freeRegister(right_dest);
	      (void) emit(op, src, right_dest, dest, insn, base, noCost);
	    }
            removeAst(left);
            removeAst(right);
	}
    } else if (type == operandNode) {
	dest = rs->allocateRegister(insn, base, noCost);
        if (useCount>0) {
          kept_register=dest;
          rs->keep_register(dest);
        }
	if (oType == Constant) {
	    (void) emit(loadConstOp, (reg) oValue, dest, dest, insn, base, noCost);
	} else if (oType == ConstantPtr) {
	    (void) emit(loadConstOp, (reg) (*(unsigned int *) oValue),
		dest, dest, insn, base, noCost);
	} else if (oType == DataPtr) {
	    addr = (unsigned) oValue;
            assert(addr != 0); // check for NULL
	    (void) emit(loadConstOp, (reg) addr, dest, dest, insn, base, noCost);
	} else if (oType == DataIndir) {
	    src = loperand->generateCode_phase2(proc, rs, insn, base, noCost);
	    (void) emit(loadIndirOp, src, 0, dest, insn, base, noCost); 
            rs->freeRegister(src);
	} else if (oType == DataReg) {
            rs->unkeep_register(dest);
            rs->freeRegister(dest);
            dest = (reg)oValue;
            if (useCount>0) {
              kept_register=dest;
              rs->keep_register(dest);
            }
	} else if (oType == DataId) {
#if defined(MT_THREAD)
	    unsigned position;
            Thread *thr = proc->threads[0];
            position = thr->CTvector->getCTmapId((unsigned) oValue);
            assert(position < thr->CTvector->size());
            assert(thr->CTvector->getCTusagePos(position)==1);
            (void) emit(loadConstOp, (reg) position, dest, dest, insn, base, noCost);
#else
	    (void) emit(loadConstOp, (reg) oValue, dest, dest, insn, base, noCost);
#endif
	} else if (oType == DataValue) {
	    addr = (unsigned) oValue;

	    assert(addr != 0); // check for NULL
	    (void) emit(loadOp, (reg) addr, dest, dest, insn, base, noCost);
	} else if (oType == ReturnVal) {
            rs->unkeep_register(dest);
	    rs->freeRegister(dest);
	    src = rs->allocateRegister(insn, base, noCost);
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
	    if (astFlag)
		dest = emit(getSysRetValOp, 0, 0, src, insn, base, noCost);
	    else 
#endif
	        dest = emit(getRetValOp, 0, 0, src, insn, base, noCost);
            if (useCount>0) {
              kept_register=dest;
              rs->keep_register(dest);
	    }
	    if (src != dest) {
		rs->freeRegister(src);
	    }
	} else if (oType == Param) {
            rs->unkeep_register(dest);
	    rs->freeRegister(dest);
	    src = rs->allocateRegister(insn, base, noCost);
	    // return the actual reg it is in.
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
	    if (astFlag)
		dest = emit(getSysParamOp, (reg)oValue, 0, src, insn, base, noCost);
	    else 
#endif
		dest = emit(getParamOp, (reg)oValue, 0, src, insn, base, noCost);
            if (useCount>0) {
              kept_register=dest;
              rs->keep_register(dest);
	    }
	    if (src != dest) {
		rs->freeRegister(src);
	    }
	} else if (oType == DataAddr) {
	  addr = (unsigned) oValue;
	  emit(loadOp, (reg) addr, dest, dest, insn, base, noCost);
	}
    } else if (type == callNode) {
	dest = emitFuncCall(callOp, rs, insn, base, operands, callee, proc, noCost);
        if (useCount>0) {
          kept_register=dest;
          rs->keep_register(dest);
        }
    } else if (type == sequenceNode) {
	(void) loperand->generateCode_phase2(proc, rs, insn, base, noCost);
 	return(roperand->generateCode_phase2(proc, rs, insn, base, noCost));
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
	case loadIndirOp: return("load&");
	case storeIndirOp: return("=&");
	default: return("ERROR");
    }
}

int AstNode::cost() const {
    int total = 0;
    int getInsnCost(opCode t);

    if (type == opCodeNode) {
        if (op == ifOp) {
	    if (loperand) total += loperand->cost();
	    total += getInsnCost(op);
	    if (roperand) total += roperand->cost();
	} else if (op == storeOp) {
	    if (roperand) total += roperand->cost();
	    total += getInsnCost(op);
	} else if (op == storeIndirOp) {
            if (loperand) total += loperand->cost();
	    if (roperand) total += roperand->cost();
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
	} else if (oType == DataId) {
	    total = getInsnCost(loadConstOp);
	} else if (oType == DataIndir) {
	    total = getInsnCost(loadIndirOp);
            total += loperand->cost();
	} else if (oType == DataReg) {
	    total = getInsnCost(loadIndirOp);
	} else if (oType == Param) {
	    total = getInsnCost(getParamOp);
	}
    } else if (type == callNode) {
	total = getPrimitiveCost(callee);
	for (unsigned u = 0; u < operands.size(); u++)
	  if (operands[u]) total += operands[u]->cost();
    } else if (type == sequenceNode) {
	if (loperand) total += loperand->cost();
	if (roperand) total += roperand->cost();
    }
    return(total);
}

void AstNode::print() const {
    if (type == operandNode) {
	if (oType == Constant) {
	    sprintf(errorLine, " %d", (int) oValue);
	    logLine(errorLine);
	} else if (oType == DataPtr) {
	    sprintf(errorLine, " %d", (int) oValue);
	    logLine(errorLine);
	} else if (oType == DataValue) {
	    sprintf(errorLine, "@%d", (int) oValue);
	    logLine(errorLine);
	} else if (oType == DataIndir) {
	    logLine("@[");
            loperand->print();
	    logLine("]");
	} else if (oType == DataReg) {
	    sprintf(errorLine," reg%d ",(int)oValue);
            logLine(errorLine);
            loperand->print();
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
	  operands[u]->print();
	logLine(")");
    } else if (type == sequenceNode) {
	if (loperand) loperand->print();
	logLine(",");
	if (roperand) roperand->print();
    }
}

AstNode *createIf(AstNode *expression, AstNode *action) 
{
  AstNode *t;
  t = new AstNode(ifOp, expression, action);
  return(t);
}

#if !defined(MT_THREAD)

AstNode *createTimer(const string &func, void *dataPtr, 
                     vector<AstNode *> &ast_args)
{
  AstNode *t0=NULL,*t1=NULL;

  t0 = new AstNode(AstNode::DataPtr, (void *) dataPtr);
  ast_args += assignAst(t0);
  removeAst(t0);
  t1 = new AstNode(func, ast_args);
  for (unsigned i=0;i<ast_args.size();i++) removeAst(ast_args[i]);  
  return(t1);
}

AstNode *createCounter(const string &func, void *dataPtr, 
                       AstNode *ast) 
{
   AstNode *t0=NULL, *t1=NULL, *t2=NULL;

   t0 = new AstNode(AstNode::DataValue, (void *)dataPtr);
   if (func=="addCounter") {
     t1 = new AstNode(plusOp,t0,ast);
     t2 = new AstNode(storeOp,t0,t1);
   } else if (func=="subCounter") {
     t1 = new AstNode(minusOp,t0,ast);
     t2 = new AstNode(storeOp,t0,t1);
   } else if (func=="setCounter") {
     t2 = new AstNode(storeOp,t0,ast);
   } else abort();
   removeAst(t0);
   removeAst(t1);
   return(t2);
}

#else

AstNode *computeAddress(void *dataPtr)
{
  AstNode *t0=NULL,*t1=NULL,*t2=NULL,*t3=NULL;
  AstNode *t4=NULL,*t5=NULL,*t6=NULL;
  int value;

  /* We don't need to check wether dataPtr is NULL, because it represents */
  /* an id rather than a pointer - naim 2/18/97                      */

  /* DYNINSTthreadTable[thr_self()] */
  /* make sure we read updated base address */
  t0 = new AstNode(AstNode::DataReg, (void *)REG_MT);
  t1 = new AstNode(AstNode::DataIndir, t0); 

  /* Counter_Timer_Table[counter/timerId] */
  value = sizeof(unsigned);
  t2 = new AstNode(AstNode::DataId, (void *)dataPtr);
  t3 = new AstNode(AstNode::Constant, (void *)value);
  t4 = new AstNode(timesOp, t2, t3);
  t5 = new AstNode(plusOp, t1, t4);
  t6 = new AstNode(AstNode::DataIndir, t5);

  /* address of Counter_Timer->value */
  /* intCounter has value at position 0, so offset is 0 */
  /* tTimer is different */

  removeAst(t0);
  removeAst(t1);
  removeAst(t2);
  removeAst(t3);
  removeAst(t4);
  removeAst(t5);
  return(t6);
}

AstNode *createTimer(const string &func, void *dataPtr, 
                     vector<AstNode *> &ast_args)
{
  AstNode *t0=NULL,*t1=NULL;

  t0 = computeAddress(dataPtr);
  ast_args += assignAst(t0);
  removeAst(t0);
  t1 = new AstNode(func, ast_args);
  for (unsigned i=0;i<ast_args.size();i++) removeAst(ast_args[i]);  

  return(t1);
}

AstNode *createCounter(const string &func, void *dataPtr, 
                       AstNode *ast) 
{ 
  AstNode *t0=NULL,*t1=NULL,*t2=NULL,*t3=NULL;

  t0 = computeAddress(dataPtr);

  if (func == "addCounter") {
    t1 = new AstNode(AstNode::DataIndir, t0);
    t2 = new AstNode(plusOp, t1, ast);
    t3 = new AstNode(storeIndirOp, t0, t2);
  } else if (func == "subCounter") {
    t1 = new AstNode(AstNode::DataIndir, t0);
    t2 = new AstNode(minusOp, t1, ast);
    t3 = new AstNode(storeIndirOp, t0, t2);
  } else if (func == "setCounter") {
    t3 = new AstNode(storeIndirOp, t0, ast);
  }

  removeAst(t0);
  removeAst(t1);
  removeAst(t2);

  return(t3);
}

#endif
