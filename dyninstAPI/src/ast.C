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

// $Id: ast.C,v 1.57 1998/09/08 21:35:27 buck Exp $

#include "dyninstAPI/src/pdThread.h"

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/showerror.h"
#if defined(BPATCH_LIBRARY)
#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/src/BPatch_type.h"
#endif

#ifndef BPATCH_LIBRARY
#include "rtinst/h/rtinst.h"
#include "paradynd/src/metric.h"
#endif

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
#include "dyninstAPI/src/inst-sparc.h"
#elif defined(hppa1_1_hp_hpux)
#include "dyninstAPI/src/inst-hppa.h"
#elif defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
#include "dyninstAPI/src/inst-power.h"
#elif defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0)
#include "dyninstAPI/src/inst-x86.h"
#elif defined(alpha_dec_osf4_0)
#include "dyninstAPI/src/inst-alpha.h"
#else
#endif

extern registerSpace *regSpace;
extern bool doNotOverflow(int value);

registerSpace::registerSpace(int deadCount, int *dead, int liveCount, int *live)
{
#if defined(i386_unknown_solaris2_5)
  initTramps();
#endif

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
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
        if (registers[i+deadCount].number == REG_MT) {
          registers[i+deadCount].inUse = true;
          registers[i+deadCount].needsSaving = true;
        }
#endif
    }

}

reg registerSpace::allocateRegister(char *insn, Address &base, bool noCost) 
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
          //sprintf(errorLine,"WARNING: register %d is still in use\n",registers[i].number);
          //logLine(errorLine);
        }
	registers[i].inUse = false;
	registers[i].mustRestore = false;
	registers[i].needsSaving = registers[i].startsLive;
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
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
   if (eoperand) {
      if (src.eoperand) {
        if (eoperand!=src.eoperand) {
          removeAst(eoperand);
        }
      } else {
        removeAst(eoperand);
      }
   }
   if (type == operandNode && oType == ConstantString)
       free((char *)oValue);
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
      // XXX This is for the string type.  If/when we fix the string type to
      // make it less of a hack, we'll need to change this.
      if (oType == ConstantString)
	  oValue = P_strdup((char *)src.oValue);
      else
    	  oValue = src.oValue;
   }

   loperand = assignAst(src.loperand);
   roperand = assignAst(src.roperand);
   eoperand = assignAst(src.eoperand);

   firstInsn = src.firstInsn;
   lastInsn = src.lastInsn;

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
   astFlag = src.astFlag;
#endif

   bptype = src.bptype;
   doTypeCheck = src.doTypeCheck;

   return *this;
}

#if defined(ASTDEBUG)
static int ASTcount=0;

void ASTcounter()
{
  ASTcount++;
  sprintf(errorLine,"AstNode CONSTRUCTOR - ASTcount is %d\n",ASTcount);
  //logLine(errorLine);
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
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
    astFlag = false;
#endif
   // used in mdl.C
   type = opCodeNode;
   op = noOp;
   loperand = roperand = eoperand = NULL;
   referenceCount = 1;
   useCount = 0;
   kept_register = -1;
   // "operands" is left as an empty vector
   bptype = NULL;
   doTypeCheck = true;
}

AstNode::AstNode(const string &func, AstNode *l, AstNode *r) {
#if defined(ASTDEBUG)
    ASTcounter();
#endif
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
    astFlag = false;
#endif
    referenceCount = 1;
    useCount = 0;
    kept_register = -1;
    type = callNode;
    callee = func;
    loperand = roperand = eoperand = NULL;
    if (l) operands += assignAst(l);
    if (r) operands += assignAst(r);
    bptype = NULL;
    doTypeCheck = true;
}

AstNode::AstNode(const string &func, AstNode *l) {
#if defined(ASTDEBUG)
    ASTcounter();
#endif
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
    astFlag = false;
#endif
    referenceCount = 1;
    useCount = 0;
    kept_register = -1;
    type = callNode;
    loperand = roperand = eoperand = NULL;
    callee = func;
    if (l) operands += assignAst(l);
    bptype = NULL;
    doTypeCheck = true;
}

AstNode::AstNode(const string &func, vector<AstNode *> &ast_args) {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
    astFlag = false;
#endif
   referenceCount = 1;
   useCount = 0;
   kept_register = -1;
   for (unsigned i=0;i<ast_args.size();i++) 
     if (ast_args[i]) operands += assignAst(ast_args[i]);
   loperand = roperand = eoperand = NULL;
   type = callNode;
   callee = func;
   bptype = NULL;
   doTypeCheck = true;
}

AstNode::AstNode(operandType ot, void *arg) {
#if defined(ASTDEBUG)
    ASTcounterNP();
#endif
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
    astFlag = false;
#endif
    referenceCount = 1;
    useCount = 0;
    kept_register = -1;
    type = operandNode;
    oType = ot;
    if (ot == ConstantString)
	oValue = (void *)P_strdup((char *)arg);
    else
    	oValue = (void *) arg;
    loperand = roperand = eoperand = NULL;
    bptype = NULL;
    doTypeCheck = true;
};

AstNode::AstNode(operandType ot, AstNode *l) {
#if defined(ASTDEBUG)
    ASTcounter();
#endif
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
    astFlag = false;
#endif
    referenceCount = 1;
    useCount = 0;
    kept_register = -1;
    type = operandNode;
    oType = ot;
    oValue = NULL;
    roperand = NULL;
    eoperand = NULL;
    loperand = assignAst(l);
    bptype = NULL;
    doTypeCheck = true;
};

AstNode::AstNode(AstNode *l, AstNode *r) {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
    astFlag = false;
#endif
   referenceCount = 1;
   useCount = 0;
   kept_register = -1;
   type = sequenceNode;
   loperand = assignAst(l);
   roperand = assignAst(r);
   eoperand = NULL;
   bptype = NULL;
   doTypeCheck = true;
};

AstNode::AstNode(opCode ot) {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
    astFlag = false;
#endif
   // a private constructor
   referenceCount = 1;
   useCount = 0;
   kept_register = -1;
   type = opCodeNode;
   op = ot;
   loperand = roperand = eoperand = NULL;
   bptype = NULL;
   doTypeCheck = true;
}

AstNode::AstNode(opCode ot, AstNode *l) {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
    astFlag = false;
#endif
   // a private constructor
   referenceCount = 1;
   useCount = 0;
   kept_register = -1;
   type = opCodeNode;
   op = ot;
   loperand = assignAst(l);
   roperand = NULL;
   eoperand = NULL;
   bptype = NULL;
   doTypeCheck = true;
}

AstNode::AstNode(opCode ot, AstNode *l, AstNode *r, AstNode *e) {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
    astFlag = false;
#endif
   referenceCount = 1;
   useCount = 0;
   kept_register = -1;
   type = opCodeNode;
   op = ot;
   loperand = assignAst(l);
   roperand = assignAst(r);
   eoperand = assignAst(e);
   bptype = NULL;
   doTypeCheck = true;
};

AstNode::AstNode(AstNode *src) {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
    astFlag = false;
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
      // XXX This is for the string type.  If/when we fix the string type to
      // make it less of a hack, we'll need to change this.
      if (oType == ConstantString)
	  oValue = P_strdup((char *)src->oValue);
      else
	  oValue = src->oValue;
   }

   loperand = assignAst(src->loperand);
   roperand = assignAst(src->roperand);
   eoperand = assignAst(src->eoperand);
   firstInsn = src->firstInsn;
   lastInsn = src->lastInsn;
   bptype = src->bptype;
   doTypeCheck = src->doTypeCheck;
}

#if defined(ASTDEBUG)
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
    if (eoperand) {
      logLine("RC eoperand\n");
      eoperand->printRC();
    }
}
#endif

AstNode::~AstNode() {
#if defined(ASTDEBUG)
  ASTcount--;
  sprintf(errorLine,"AstNode DESTRUCTOR - ASTcount is %d\n",ASTcount);
  //logLine(errorLine);
#endif
  if (loperand) {
    removeAst(loperand);
  }
  if (roperand) {
    removeAst(roperand);
  }
  if (eoperand) {
    removeAst(eoperand);
  }
  if (type==callNode) {
    for (unsigned i=0;i<operands.size();i++) {
      removeAst(operands[i]);
    }
  } else if (type == operandNode && oType == ConstantString) {
      free(oValue);
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
			   Address &count, 
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
	preamble->generateCode(proc, regSpace, i, count, noCost, true);
        removeAst(preamble);
	tmp = generateCode(proc, regSpace, i, count, noCost, true);
        regSpace->freeRegister(tmp);
        return_reg = trailer->generateCode(proc, regSpace, i, count, noCost, true);
    } else {
        removeAst(preamble);
        return_reg = (unsigned) emit(op, 1, 0, 0, i, count, noCost);
    }
    regSpace->resetSpace();
    return(return_reg);
}

bool isPowerOf2(int value, int &result)
{
  if (value<=0) return(false);
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

/*
void AstNode::setUseCount(void)
{

  useCount=referenceCount ;
  kept_register=-1;
  if (loperand) loperand->setUseCount();
  if (roperand) roperand->setUseCount();
  if (eoperand) eoperand->setUseCount();
  for (unsigned i=0;i<operands.size(); i++)
    operands[i]->setUseCount() ;
}
*/

void AstNode::setUseCount(void) {
  if (useCount == 0) {
    kept_register=-1;
    if (loperand) loperand->setUseCount();
    if (roperand) roperand->setUseCount();
    if (eoperand) eoperand->setUseCount();
    for (unsigned i=0;i<operands.size(); i++)
      operands[i]->setUseCount() ;
  }
  useCount++ ;
}

void AstNode::cleanUseCount(void)
{
  useCount=0;
  kept_register=-1;
  if (loperand) loperand->cleanUseCount();
  if (roperand) roperand->cleanUseCount();
  if (eoperand) eoperand->cleanUseCount();
  for (unsigned i=0;i<operands.size(); i++)
    operands[i]->cleanUseCount() ;
}

#if defined(ASTDEBUG)
bool AstNode::checkUseCount(registerSpace* rs, bool& err)
{
  if (useCount>0) {
    sprintf(errorLine, "[ERROR] kept register, useCount=%d\n", useCount) ;
    logLine(errorLine) ;
    logLine("++++++\n") ; 
    print() ; 
    logLine("\n------\n") ;
    err = true ;
  }
  if (loperand) loperand->checkUseCount(rs, err);
  if (roperand) roperand->checkUseCount(rs, err);
  if (eoperand) eoperand->checkUseCount(rs, err);
  for (unsigned i=0;i<operands.size(); i++)
    operands[i]->checkUseCount(rs, err) ;
  return err ;
}
#endif

#if defined(ASTDEBUG)
void AstNode::printUseCount(void)
{
}
#endif

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
			  Address &base, bool noCost, bool root) {
  if (root) {
    cleanUseCount();
    setUseCount();
#if defined(ASTDEBUG)
    print() ;
#endif
  }
  unsigned start_insn = base ;
  reg tmp=generateCode_phase2(proc,rs,insn,base,noCost);

#if defined(ASTDEBUG)
  if (root) {
    bool err = false ;
    checkUseCount(rs, err) ;
  }
#endif
  return(tmp);
}

reg AstNode::generateCode_phase2(process *proc,
			         registerSpace *rs,
			         char *insn, 
			         Address &base, bool noCost) {
    Address addr;
    Address fromAddr;
    Address startInsn;
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
        if (op == branchOp) {
	    assert(loperand->oType == Constant);
	    Address offset = (Address)loperand->oValue;
	    loperand->useCount--;
	    loperand->kept_register = -1 ;
            emit(branchOp, (reg) 0, (reg) 0, (int)offset, insn, base, noCost);
        } else if (op == ifOp) {
            // This ast cannot be shared because it doesn't return a register
	    src = loperand->generateCode_phase2(proc, rs, insn, base, noCost);
	    startInsn = base;
	    fromAddr = emit(op, src, (reg) 0, (reg) 0, insn, base, noCost);
            rs->freeRegister(src);
            reg tmp = roperand->generateCode_phase2(proc, rs, insn, base, noCost);
            rs->freeRegister(tmp);

	    // Is there and else clause?  If yes, generate branch over it
	    Address else_fromAddr;
	    Address else_startInsn = base;
	    if (eoperand) 
	      else_fromAddr = emit(branchOp, (reg) 0, (reg) 0, (reg) 0,
				     insn, base, noCost);

	    // call emit again now with correct offset.
	    (void) emit(op, src, (reg) 0, (reg) ((long)base - (long)fromAddr), 
			insn, startInsn, noCost);
            // sprintf(errorLine,branch forward %d\n", base - fromAddr);
	   
	    if (eoperand) {
		// If there's an else clause, we need to generate code for it.
		tmp = eoperand->generateCode_phase2(proc, rs, insn, base,
						    noCost);
		rs->freeRegister(tmp);

		// We also need to fix up the branch at the end of the "true"
		// clause to jump around the "else" clause.
		emit(branchOp, (reg) 0, (reg) 0,
		     ((long)base - (long)else_fromAddr),
		     insn, else_startInsn, noCost);
	    }
	} else if (op == whileOp) {
	    unsigned top = base ;
	    src = loperand->generateCode_phase2(proc, rs, insn, base, noCost);
	    startInsn = base;
	    fromAddr = emit(ifOp, src, (reg) 0, (reg) 0, insn, base, noCost);
            rs->freeRegister(src);
	    if (roperand) {
                reg tmp=roperand->generateCode_phase2(proc, rs, insn, base, noCost);
                rs->freeRegister(tmp);
	    }
	    //jump back
	    (void) emit(branchOp, (reg) 0, (reg) 0, ((int) top - (int) base),
			insn, base, noCost) ;

	    // call emit again now with correct offset.
	    (void) emit(ifOp, src, (reg) 0, (reg) ((int)base - (int)fromAddr), 
			insn, startInsn, noCost);
            // sprintf(errorLine,branch forward %d\n", base - fromAddr);
	} else if (op == doOp) {
	     assert(0) ;
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
	    addr = (Address) loperand->oValue;
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
#if defined (i386_unknown_solaris2_5) || (sparc_sun_solaris2_4)
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
	    return (Address) emit(op, 0, 0, 0, insn, base, noCost);
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
#ifdef alpha_dec_osf4_0
	      if (op == divOp)
		{
		  bool err;
		  Address divlAddr = proc->findInternalAddress("divide", true, err);  
		  assert(divlAddr);
		  software_divide(src,(reg)right->oValue,dest,insn,base,noCost,divlAddr,true);
		}
	      else emitImm(op, src, (reg)right->oValue, dest, insn, base, noCost);
#else
	      emitImm(op, src, (reg)right->oValue, dest, insn, base, noCost);
#endif
	     
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
#ifdef alpha_dec_osf4_0
	      if (op == divOp)
		{
		  bool err;
		  Address divlAddr = proc->findInternalAddress("divide", true, err);  
		  assert(divlAddr);
		  software_divide(src,right_dest,dest,insn,base,noCost,divlAddr);
		}
	      else
		(void) emit(op, src, right_dest, dest, insn, base, noCost);
#else
	      (void) emit(op, src, right_dest, dest, insn, base, noCost);
#endif
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
	    addr = (Address) oValue;
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
	} else if (oType == DataId) {
	    (void) emit(loadConstOp, (reg) oValue, dest, dest, insn, base, noCost);
	} else if (oType == DataValue) {
	    addr = (Address) oValue;

	    assert(addr != 0); // check for NULL
	    (void) emit(loadOp, (reg) addr, dest, dest, insn, base, noCost);
	} else if (oType == ReturnVal) {
            rs->unkeep_register(dest);
	    rs->freeRegister(dest);
	    src = rs->allocateRegister(insn, base, noCost);
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
	    if (loperand) {
		unsigned emitOptReturn(unsigned, reg, char *, unsigned &, bool);
		dest = emitOptReturn((unsigned)loperand->oValue, src, insn, base, noCost);
	    }
	    else if (astFlag)
		dest = emit(getSysRetValOp, 0, 0, src, insn, base, noCost);
	    else 
#endif
	        dest = emit(getRetValOp, 0, 0, src, insn, base, noCost);
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
	    if (src != dest) {
		rs->freeRegister(src);
	    }
	} else if (oType == DataAddr) {
	  addr = (Address) oValue;
	  emit(loadOp, (reg) addr, dest, dest, insn, base, noCost);
	} else if (oType == ConstantString) {
	  // XXX This is for the string type.  If/when we fix the string type
	  // to make it less of a hack, we'll need to change this.
	  int len = strlen((char *)oValue) + 1;
	  addr = (Address) inferiorMalloc(proc, len, dataHeap);
	  proc->writeDataSpace((char *)addr, len, (char *)oValue);
	  emit(loadConstOp, (reg) addr, dest, dest, insn, base, noCost);
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


#if defined(ASTDEBUG)
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
	case whileOp: return("while") ;
	case doOp: return("while") ;
	case trampPreamble: return("preTramp");
	case trampTrailer: return("goto");
	case branchOp: return("goto");
	case noOp: return("nop");
	case andOp: return("and");
	case orOp: return("or");
	case loadIndirOp: return("load&");
	case storeIndirOp: return("=&");
	default: return("ERROR");
    }
}
#endif

int AstNode::cost() const {
    int total = 0;
    int getInsnCost(opCode t);

    if (type == opCodeNode) {
        if (op == ifOp) {
	    if (loperand) total += loperand->cost();
	    total += getInsnCost(op);
	    int rcost = 0, ecost = 0;
	    if (roperand) {
		rcost = roperand->cost();
		if (eoperand)
		    rcost += getInsnCost(branchOp);
	    }
	    if (eoperand)
		ecost = eoperand->cost();
	    if (rcost > ecost)	    
		total += rcost;
	    else
		total += ecost;
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

#if defined(ASTDEBUG)
void AstNode::print() const {
  if (this) {
    sprintf(errorLine,"{%d}", referenceCount) ;
    logLine(errorLine) ;
    if (type == operandNode) {
      if (oType == Constant) {
	sprintf(errorLine, " %d", (int) oValue);
	logLine(errorLine);
      } else if (oType == ConstantString) {
        sprintf(errorLine, "%s", (char *)oValue);
	logLine(errorLine) ;
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
      if (eoperand) eoperand->print();
      logLine(")\n");
    } else if (type == callNode) {
      ostrstream os(errorLine, 1024, ios::out);
      os << "(" << callee << ends;
      logLine(errorLine);
      for (unsigned u = 0; u < operands.size(); u++)
	operands[u]->print();
      logLine(")\n");
    } else if (type == sequenceNode) {
      if (loperand) loperand->print();
      logLine(",");
      if (roperand) roperand->print();
      logLine("\n");
    }
  }
}
#endif

AstNode *createIf(AstNode *expression, AstNode *action) 
{
  AstNode *t;
  t = new AstNode(ifOp, expression, action);
  return(t);
}

#ifndef BPATCH_LIBRARY

// Multithreaded case with shared memory sampling

#if defined(SHM_SAMPLING) && defined(MT_THREAD)

AstNode *computeAddress(void *level, void *index, int type)
{
  AstNode *t0=NULL,*t1=NULL,*t2=NULL,*t3=NULL;
  AstNode *t4=NULL,*t5=NULL,*t6=NULL,*t7=NULL;
  AstNode *t8=NULL,*t9=NULL,*t10=NULL,*t11=NULL;
  int tSize;

  /* DYNINSTthreadTable[0][thr_self()] */
  t0 = new AstNode(AstNode::DataReg, (void *)REG_MT);

  /* Now we compute the offset for the corresponding level. We assume */
  /* that the DYNINSTthreadTable is stored by rows - naim 4/18/97 */
  if ((int)level != 0) {
    tSize = sizeof(unsigned long);
    t1 = new AstNode(AstNode::Constant, (void *)MAX_NUMBER_OF_THREADS);
    t2 = new AstNode(AstNode::Constant, level);
    t3 = new AstNode(timesOp, t1, t2);
    t4 = new AstNode(AstNode::Constant, (void *)tSize);
    t5 = new AstNode(timesOp, t3, t4); /* offset including just level */

    /* Given the level and tid, we compute the position in the thread */
    /* table. */
    t6 = new AstNode(plusOp, t0, t5);

    /* We then read the address, which is really the base address of the */
    /* vector of counters and timers in the shared memory segment. */
    t7 = new AstNode(AstNode::DataIndir, t6); 
  } else {
    /* if level is 0, we don't need to compute the offset */
    t7 = new AstNode(AstNode::DataIndir, t0);
  }

  /* Finally, using the index as an offset, we compute the address of the */
  /* counter/timer. */
  if (type==0) {
    /* intCounter */
    tSize = sizeof(intCounter);
  } else {
    /* Timer */
    tSize = sizeof(tTimer);
  }
  t8 = new AstNode(AstNode::Constant, (void *)index);
  t9 = new AstNode(AstNode::Constant, (void *)tSize);
  t10 = new AstNode(timesOp, t8, t9);
  t11 = new AstNode(plusOp, t7, t10); /* address of counter/timer */

  removeAst(t0);
  removeAst(t1);
  removeAst(t2);
  removeAst(t3);
  removeAst(t4);
  removeAst(t5);
  removeAst(t6);
  removeAst(t7);
  removeAst(t8);
  removeAst(t9);
  removeAst(t10);
  return(t11);
}

AstNode *createTimer(const string &func, void *level, void *index,
                     vector<AstNode *> &ast_args)
{
  AstNode *t0=NULL,*t1=NULL;

  t0 = computeAddress(level,index,1); /* 1 means tTimer */
  ast_args += assignAst(t0);
  removeAst(t0);
  t1 = new AstNode(func, ast_args);
  for (unsigned i=0;i<ast_args.size();i++) removeAst(ast_args[i]);  

  return(t1);
}

AstNode *createCounter(const string &func, void *level, void *index, 
                       AstNode *ast) 
{ 
  AstNode *t0=NULL,*t1=NULL,*t2=NULL,*t3=NULL;

  t0 = computeAddress(level,index,0); /* 0 means intCounter */

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

#else

// Single threaded case

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

#endif
#endif

#ifdef BPATCH_LIBRARY
BPatch_type *AstNode::checkType()
{
    BPatch_type *ret = NULL;
    BPatch_type *lType = NULL, *rType = NULL, *eType = NULL;
    bool errorFlag = false;

    assert(BPatch::bpatch != NULL);	/* We'll use this later. */

    assert( (!loperand && !roperand) || getType() == NULL );

    if (loperand)
	lType = loperand->checkType();

    if (roperand)
	rType = roperand->checkType();

    if (eoperand)
	eType = eoperand->checkType();

    if (lType == BPatch::bpatch->type_Error ||
	rType == BPatch::bpatch->type_Error)
	errorFlag = true;

    
    switch (type) { // Type here is nodeType, not BPatch library type
	case sequenceNode:
	    ret = rType;
	    break;
	case opCodeNode:
	    if (op == ifOp) {
		// XXX No checking for now.  Should check that loperand
		// is boolean.
		ret = BPatch::bpatch->type_Untyped;
	    } else if (op == noOp) {
		ret = BPatch::bpatch->type_Untyped;
	    } else {
		if (lType != NULL && rType != NULL) {
		    if (!lType->isCompatible(*rType)) {
			errorFlag = true;
		    }
		}
		// XXX The following line must change to decide based on the
		// types and operation involved what the return type of the
		// expression will be.
		ret = lType;
	    }
	    break;
	case operandNode:
	    assert(loperand == NULL && roperand == NULL);
	    if ((oType == Param) || (oType == ReturnVal)) {
		// XXX Params and ReturnVals untyped for now
		ret = BPatch::bpatch->type_Untyped; 
	    } else
    		ret = (BPatch_type *)getType(); /* XXX Cast away const */
	    assert(ret != NULL);
	    break;
	case callNode:
	    int i;
	    for (i = 0; i < operands.size(); i++) {
		BPatch_type *operandType = operands[i]->checkType();
		/* XXX Check operands for compatibility */
		if (operandType == BPatch::bpatch->type_Error)
		    errorFlag = true;
	    }
	    /* XXX Should set to return type of function. */
	    ret = BPatch::bpatch->type_Untyped;
	    break;
      default:
	assert(0);
    }

    assert(ret != NULL);

    if (errorFlag && doTypeCheck) {
	ret = BPatch::bpatch->type_Error;
    }

    return ret;
}
#endif

bool AstNode::findFuncInAst(string func) {
  if (type == callNode && callee == func) 
      return true ;

  if (loperand && loperand->findFuncInAst(func))
      return true ;

  if (roperand && roperand->findFuncInAst(func))
      return true ;

  for (unsigned i=0; i<operands.size(); i++)
    if (operands[i]->findFuncInAst(func))
      return true ;

  return false ;
} 

// Looks for function func1 in ast and replaces it with function func2
void AstNode::replaceFuncInAst(string func1, string func2)
{
  if (type == callNode) {
    if (callee == func1) {
      callee = func2;
    }
  }
  if (loperand) loperand->replaceFuncInAst(func1,func2);
  if (roperand) roperand->replaceFuncInAst(func1,func2);
  for (unsigned i=0; i<operands.size(); i++)
    operands[i]->replaceFuncInAst(func1, func2) ;
} 

void AstNode::replaceFuncInAst(string func1, string func2, vector<AstNode *> &more_args, int index)
{
  unsigned i ;
  /*
  cerr << "AstNode::replaceFuncInAst(" 
       << func1.string_of() << "' "
       << func2.string_of() << ", ..."
       << index << ")" << endl ;
  */
  if (type == callNode) {
      if (callee == func1 || callee == func2) {
	  callee = func2 ;
	  unsigned int j = 0 ;
	  for (i=index; i< operands.size() && j <more_args.size(); i++){
	    removeAst(operands[i]) ;
	    operands[i] = assignAst(more_args[j++]) ;
	  }
	  while (j<more_args.size()) {
	    operands += assignAst(more_args[j++]) ;
	  }
      }
  }
  if (loperand) loperand->replaceFuncInAst(func1, func2, more_args, index) ;
  if (roperand) roperand->replaceFuncInAst(func1, func2, more_args, index) ;
  for (i=0; i<operands.size(); i++)
    operands[i]->replaceFuncInAst(func1, func2, more_args, index) ;
}


