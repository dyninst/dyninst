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

// $Id: ast.C,v 1.81 2001/02/26 21:34:39 bernat Exp $

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
#include "dyninstAPI/h/BPatch_type.h"
#include "dyninstAPI/src/BPatch_collections.h"
#else
#include "dyninstAPI/src/pdThread.h"
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
#elif defined(mips_sgi_irix6_4)
#include "dyninstAPI/src/inst-mips.h"
#else
#endif

extern registerSpace *regSpace;
extern bool doNotOverflow(int value);

registerSpace::registerSpace(const unsigned int deadCount, Register *dead, 
                             const unsigned int liveCount, Register *live)
{
#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0)
  initTramps();
#endif

    unsigned i;
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

Register registerSpace::allocateRegister(char *insn, Address &base, bool noCost) 
{
    unsigned i;
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
 	    emitV(saveRegOp, registers[i].number, 0, 0, insn, base, noCost);
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
    return(Null_Register);
}

bool registerSpace::is_keep_register(Register k)
{
  for (unsigned i=0;i<keep_list.size();i++) {
    if (keep_list[i]==k) return(true);
  }
  return(false);
}

void registerSpace::keep_register(Register k)
{
  if (!is_keep_register(k)) {
    keep_list += k;
#if defined(ASTDEBUG)
    sprintf(errorLine,"==> keeping register %d, size is %d <==\n",k,keep_list.size());
    logLine(errorLine);
#endif
  }
}

void registerSpace::unkeep_register(Register k) {
  unsigned ksize = keep_list.size();
  for (unsigned i=0;i<ksize;i++) {
    if (keep_list[i]==k) {
      keep_list[i]=keep_list[ksize-1];
      ksize--;
      keep_list.resize(ksize);
#if !defined(BPATCH_LIBRARY_F)
      freeRegister(k);
#endif
#if defined(ASTDEBUG)
      sprintf(errorLine,"==> un-keeping register %d, size is %d <==\n",k,keep_list.size());
      logLine(errorLine);
#endif
      break;
    }
  }
}

void registerSpace::freeRegister(Register reg) {

    if (is_keep_register(reg)) return;
    for (u_int i=0; i < numRegisters; i++) {
	if (registers[i].number == reg) {
	    registers[i].inUse = false;
	    return;
	}
    }
}

bool registerSpace::isFreeRegister(Register reg) {

    for (u_int i=0; i < numRegisters; i++) {
	if ((registers[i].number == reg) &&
	    (registers[i].inUse == true)) {
	    return false;
	}
    }
    return true;
}

void registerSpace::resetSpace() {
    for (u_int i=0; i < numRegisters; i++) {
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
      calleefunc = src.calleefunc;
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

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
   astFlag = src.astFlag;
#endif
   
   size = src.size;
#if defined(BPATCH_LIBRARY)
   bptype = src.bptype;
   doTypeCheck = src.doTypeCheck;
#endif

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
   kept_register = Null_Register;
   // "operands" is left as an empty vector
   size = 4;
#if defined(BPATCH_LIBRARY)
   bptype = NULL;
   doTypeCheck = true;
#endif
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
    kept_register = Null_Register;
    type = callNode;
    callee = func;
    calleefunc = NULL;
    loperand = roperand = eoperand = NULL;
    if (l) operands += assignAst(l);
    if (r) operands += assignAst(r);
    size = 4;
#if defined(BPATCH_LIBRARY)
    bptype = NULL;
    doTypeCheck = true;
#endif
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
    kept_register = Null_Register;
    type = callNode;
    loperand = roperand = eoperand = NULL;
    callee = func;
    calleefunc = NULL;
    if (l) operands += assignAst(l);
    size = 4;
#if defined(BPATCH_LIBRARY)
    bptype = NULL;
    doTypeCheck = true;
#endif
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
   kept_register = Null_Register;
   for (unsigned i=0;i<ast_args.size();i++) 
     if (ast_args[i]) operands += assignAst(ast_args[i]);
   loperand = roperand = eoperand = NULL;
   type = callNode;
   callee = func;
   calleefunc = NULL;
   size = 4;
#if defined(BPATCH_LIBRARY)
   bptype = NULL;
   doTypeCheck = true;
#endif
}


AstNode::AstNode(function_base *func, vector<AstNode *> &ast_args) {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
    astFlag = false;
#endif
   referenceCount = 1;
   useCount = 0;
   kept_register = Null_Register;
   for (unsigned i=0;i<ast_args.size();i++) 
     if (ast_args[i]) operands += assignAst(ast_args[i]);
   loperand = roperand = eoperand = NULL;
   type = callNode;
   callee = func->prettyName();
   calleefunc = func;
   size = 4;
#if defined(BPATCH_LIBRARY)
   bptype = NULL;
   doTypeCheck = true;
#endif
}


// This is used to create a node for FunctionJump (function call with
// no linkage)
AstNode::AstNode(function_base *func) {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
    astFlag = false;
#endif
   referenceCount = 1;
   useCount = 0;
   kept_register = Null_Register;
   loperand = roperand = eoperand = NULL;
   type = opCodeNode;
   op = funcJumpOp;
   callee = func->prettyName();
   calleefunc = func;
   size = 4;
#if defined(BPATCH_LIBRARY)
   bptype = NULL;
   doTypeCheck = true;
#endif
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
    kept_register = Null_Register;
    type = operandNode;
    oType = ot;
    if (ot == ConstantString)
	oValue = (void *)P_strdup((char *)arg);
    else
    	oValue = (void *) arg;
    loperand = roperand = eoperand = NULL;
    size = 4;
#if defined(BPATCH_LIBRARY)
    bptype = NULL;
    doTypeCheck = true;
#endif
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
    kept_register = Null_Register;
    type = operandNode;
    oType = ot;
    oValue = NULL;
    roperand = NULL;
    eoperand = NULL;
    loperand = assignAst(l);
    size = 4;
#if defined(BPATCH_LIBRARY)
    bptype = NULL;
    doTypeCheck = true;
#endif
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
   kept_register = Null_Register;
   type = sequenceNode;
   loperand = assignAst(l);
   roperand = assignAst(r);
   eoperand = NULL;
   size = 4;
#if defined(BPATCH_LIBRARY)
   bptype = NULL;
   doTypeCheck = true;
#endif
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
   kept_register = Null_Register;
   type = opCodeNode;
   op = ot;
   loperand = roperand = eoperand = NULL;
   size = 4;
#if defined(BPATCH_LIBRARY)
   bptype = NULL;
   doTypeCheck = true;
#endif
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
   kept_register = Null_Register;
   type = opCodeNode;
   op = ot;
   loperand = assignAst(l);
   roperand = NULL;
   eoperand = NULL;
   size = 4;
#if defined(BPATCH_LIBRARY)
   bptype = NULL;
   doTypeCheck = true;
#endif
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
   kept_register = Null_Register;
   type = opCodeNode;
   op = ot;
   loperand = assignAst(l);
   roperand = assignAst(r);
   eoperand = assignAst(e);
   size = 4;
#if defined(BPATCH_LIBRARY)
   bptype = NULL;
   doTypeCheck = true;
#endif
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
   kept_register = Null_Register;

   type = src->type;   
   if (type == opCodeNode)
      op = src->op;             // defined only for operand nodes

   if (type == callNode) {
      callee = src->callee;     // defined only for call nodes
      calleefunc = src->calleefunc;
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
   size = 4;
#if defined(BPATCH_LIBRARY)
   bptype = src->bptype;
   doTypeCheck = src->doTypeCheck;
#endif
}

#if defined(ASTDEBUG)
#define AST_PRINT
#endif

#define AST_PRINT

#if defined(AST_PRINT)
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
      free((char*)oValue);
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
// This procedure should be used every time we assign an AstNode pointer,
// because it increments the reference counter.
//
AstNode *assignAst(AstNode *src) {
#if defined(ASTDEBUG)
  sprintf(errorLine,"assignAst(0x%08X): ", src);
  logLine(errorLine);
#endif
  if (src) {
    src->referenceCount++;
#if defined(ASTDEBUG)
    sprintf(errorLine,"referenceCount -> %d\n", src->referenceCount);
    logLine(errorLine);
  } else {
    logLine("NULL\n");
#endif
  }
  return(src);
}

//
// Decrements the reference count for "ast". If it is "0", it calls the 
// AstNode destructor.
//
void removeAst(AstNode *&ast) {
#if defined(ASTDEBUG)
  sprintf(errorLine,"removeAst(0x%08X): ", ast);
  logLine(errorLine);
#endif
  if (ast) {
#if defined(ASTDEBUG)
    sprintf(errorLine,"referenceCount=%d ", ast->referenceCount);
    logLine(errorLine);
#endif
    assert(ast->referenceCount>0);
    ast->referenceCount--;
    if (ast->referenceCount==0) {
#if defined(ASTDEBUG)
      logLine("deleting...");
#endif
      delete ast;
      ast=NULL;
#if defined(ASTDEBUG)
      logLine("deleted\n");
    } else {
      sprintf(errorLine,"-> %d\n", ast->referenceCount);
      logLine(errorLine);
#endif
    }
  } else {
#if defined(ASTDEBUG)
    logLine("non-existant!");
#endif
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

Address AstNode::generateTramp(process *proc, char *i, 
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
    Address ret=0;
    if (type != opCodeNode || op != noOp) {
        Register tmp;
	preamble->generateCode(proc, regSpace, i, count, noCost, true);
        removeAst(preamble);
	tmp = (Register)generateCode(proc, regSpace, i, count, noCost, true);
        regSpace->freeRegister(tmp);
        ret = trailer->generateCode(proc, regSpace, i, count, noCost, true);
    } else {
        removeAst(preamble);
        emitV(op, 1, 0, 0, i, count, noCost);   // op==noOp
    }
    regSpace->resetSpace();
    return(ret);
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

void AstNode::setUseCount(void) {
  if (useCount == 0) {
    kept_register=Null_Register;
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
  kept_register=Null_Register;
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
Address AstNode::generateCode(process *proc,
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

  // note: this could return the value "(Address)(-1)" -- csserra
  Address tmp = generateCode_phase2(proc, rs, insn, base, noCost);

#if defined(ASTDEBUG)
  if (root) {
    bool err = false ;
    checkUseCount(rs, err) ;
  }
#endif
  return(tmp);
}

Address AstNode::generateCode_phase2(process *proc,
				     registerSpace *rs,
				     char *insn, 
				     Address &base, bool noCost) {
    Address addr;
    Address fromAddr;
    Address startInsn;
    Register src1, src2;
    Register src = Null_Register;
    Register dest = Null_Register;
    Register right_dest = Null_Register;

    useCount--;
    if (kept_register!=Null_Register) { 
#if defined(ASTDEBUG)
      sprintf(errorLine,"==> Returning register %d <==\n",kept_register);
      logLine(errorLine);
#endif
      if (useCount==0) { 
          rs->unkeep_register(kept_register);
          Register tmp=kept_register;
          kept_register=Null_Register;
#ifdef BPATCH_LIBRARY_F
	  assert(!rs->isFreeRegister(tmp));
#endif
          return (Address)tmp;
      }
      return (Address)kept_register;
    }

    if (type == opCodeNode) {
        if (op == branchOp) {
	    assert(loperand->oType == Constant);
	    unsigned offset = (RegValue)loperand->oValue;
	    loperand->useCount--;
	    loperand->kept_register = Null_Register ;
            (void)emitA(branchOp, 0, 0, (RegValue)offset, insn, base, noCost);
        } else if (op == ifOp) {
            // This ast cannot be shared because it doesn't return a register
	    src = (Register)loperand->generateCode_phase2(proc, rs, insn, base, noCost);
	    startInsn = base;
	    fromAddr = emitA(op, src, 0, 0, insn, base, noCost);
            rs->freeRegister(src);
            Register tmp = (Register)roperand->generateCode_phase2(proc, rs, insn, base, noCost);
            rs->freeRegister(tmp);

	    // Is there and else clause?  If yes, generate branch over it
	    Address else_fromAddr = 0;
	    Address else_startInsn = base;
	    if (eoperand) 
	      else_fromAddr = emitA(branchOp, 0, 0, 0,
				   insn, base, noCost);

	    // call emit again now with correct offset.
	    (void) emitA(op, src, 0, (Register) (base-fromAddr), 
			insn, startInsn, noCost);
	   
	    if (eoperand) {
		// If there's an else clause, we need to generate code for it.
		tmp = (Register)eoperand->generateCode_phase2(proc, rs, insn,
						    base, noCost);
		rs->freeRegister(tmp);

		// We also need to fix up the branch at the end of the "true"
		// clause to jump around the "else" clause.
		emitA(branchOp, 0, 0, (Register) (base-else_fromAddr),
		     insn, else_startInsn, noCost);
	    }
	} else if (op == whileOp) {
	    Address top = base ;
	    src = (Register)loperand->generateCode_phase2(proc, rs, insn, base, noCost);
	    startInsn = base;
	    fromAddr = emitA(ifOp, src, 0, 0, insn, base, noCost);
            rs->freeRegister(src);
	    if (roperand) {
                Register tmp=(Register)roperand->generateCode_phase2(proc, rs, insn, base, noCost);
                rs->freeRegister(tmp);
	    }
	    //jump back
	    (void) emitA(branchOp, 0, 0, (Register)(top-base),
			insn, base, noCost) ;

	    // call emit again now with correct offset.
	    (void) emitA(ifOp, src, 0, (Register) (base-fromAddr), 
			insn, startInsn, noCost);
            // sprintf(errorLine,"branch forward %d\n", base - fromAddr);
	} else if (op == doOp) {
	    assert(0) ;
	} else if (op == getAddrOp) {
	    if (loperand->oType == DataAddr) {
		addr = (Address) loperand->oValue;
		assert(addr != 0); // check for NULL
		dest = rs->allocateRegister(insn, base, noCost);
		emitVload(loadConstOp, addr, dest, dest, insn, base, noCost);
	    } else if (loperand->oType == FrameAddr) {
		// load the address fp + addr into dest
		dest = rs->allocateRegister(insn, base, noCost);
		Register temp = rs->allocateRegister(insn, base, noCost);
		addr = (Address) loperand->oValue;
		emitVload(loadFrameAddr, addr, temp, dest, insn, 
		     base, noCost);
		rs->freeRegister(temp);
	    } else if (loperand->oType == DataIndir) {	
		// taking address of pointer de-ref returns the original
		//    expression, so we simple generate the left child's 
		//    code to get the address 
		dest = (Register)loperand->loperand->generateCode_phase2(proc, 
		     rs, insn, base, noCost);
	    } else {
		// error condition
		assert(0);
	    }
	} else if (op == storeOp) {
            // This ast cannot be shared because it doesn't return a register
            // Check loperand because we are not generating code for it on
            // this node.
            loperand->useCount--;

	    //logLine("store: ");
	    //print();
	    //logLine("\n\n");

            if (loperand->useCount==0 && loperand->kept_register!=Null_Register) {
                rs->unkeep_register(loperand->kept_register);
#if defined(BPATCH_LIBRARY_F)
		rs->freeRegister(loperand->kept_register);
#endif
                loperand->kept_register=Null_Register;
	    }
	    src1 = (Register)roperand->generateCode_phase2(proc, rs, insn, base, noCost);
	    src2 = rs->allocateRegister(insn, base, noCost);
	    if (loperand->oType == DataAddr ) {
		addr = (Address) loperand->oValue;
		assert(addr != 0); // check for NULL
		emitVstore(storeOp, src1, src2, addr, insn, base, noCost, size);
	    } else if (loperand->oType == FrameAddr) {
		addr = (Address) loperand->oValue;
		assert(addr != 0); // check for NULL
		emitVstore(storeFrameRelativeOp, src1, src2, addr, insn, 
		    base, noCost, size);
	    } else if (loperand->oType == DataIndir) {
		// store to a an expression (e.g. an array or field use)
		// *(+ base offset) = src1
		dest = (Register)loperand->loperand->generateCode_phase2(proc, 
		     rs, insn, base, noCost);
		// dest now contains address to store into
		emitV(storeIndirOp, src1, 0, dest, insn, base, noCost, size);
	    } else {
		// invalid oType passed to store
		cerr << "invalid oType passed to store: " << (int)loperand->oType << endl;
		abort();
	    }
	    rs->freeRegister(src1);
	    rs->freeRegister(src2);
	} else if (op == storeIndirOp) {
	    src1 = (Register)roperand->generateCode_phase2(proc, rs, insn, base, noCost);
            dest = (Register)loperand->generateCode_phase2(proc, rs, insn, base, noCost);
            emitV(op, src1, 0, dest, insn, base, noCost);          
	    rs->freeRegister(src1);
            rs->freeRegister(dest);
	} else if (op == trampTrailer) {
            // This ast cannot be shared because it doesn't return a register
	    return emitA(op, 0, 0, 0, insn, base, noCost);
	} else if (op == trampPreamble) {
            // This ast cannot be shared because it doesn't return a register
#if defined (i386_unknown_solaris2_5) || (sparc_sun_solaris2_4)
	    // loperand is a constant AST node with the cost, in cycles.
	    //int cost = noCost ? 0 : (int) loperand->oValue;
            Address costAddr = 0; // for now... (won't change if noCost is set)
            loperand->useCount--;

#ifndef SHM_SAMPLING
	    bool err;
	    costAddr = proc->findInternalAddress("DYNINSTobsCostLow", true, err);
	    if (err) {
                string msg = string("Internal error: "
                        "unable to find addr of DYNINSTobsCostLow\n");
		showErrorCallback(79, msg.string_of());
		P_abort();
	    }
#else
	    if (!noCost)
	       costAddr = (Address)proc->getObsCostLowAddrInApplicSpace();
#endif
	    return emitA(op, 0, 0, 0, insn, base, noCost);
#endif
	} else if (op == funcJumpOp) {
	     emitFuncJump(funcJumpOp, insn, base, calleefunc, proc);
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
                        if (!isPowerOf2((Address)right->oValue,result) &&
                             isPowerOf2((Address)left->oValue,result))
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
	      src = (Register)left->generateCode_phase2(proc, rs, insn, base, noCost);

	    rs->freeRegister(src);
	    dest = rs->allocateRegister(insn, base, noCost);
            if (useCount>0) {
              kept_register=dest;
              rs->keep_register(dest);
	    }

            if (right && (right->type == operandNode) &&
                (right->oType == Constant) &&
                doNotOverflow((RegValue)right->oValue) &&
                (type == opCodeNode))
	    {
#ifdef alpha_dec_osf4_0
	      if (op == divOp)
		{
		  assert(false);
		  /* XXX
		   * The following doesn't work right, because we don't save
		   * and restore the scratch registers before and after the
		   * call!
		   */
		  /*
		  bool err;
		  Address divlAddr = proc->findInternalAddress("divide", true, err);  
		  assert(divlAddr);
		  software_divide(src,(RegValue)right->oValue,dest,insn,base,noCost,divlAddr,true);
		  */
		}
	      else
#endif
	      emitImm(op, src, (RegValue)right->oValue, dest, insn, base, noCost);
	     
              right->useCount--;
              if (right->useCount==0 && right->kept_register!=Null_Register) {
                rs->unkeep_register(right->kept_register);
#if defined(BPATCH_LIBRARY_F)
		rs->freeRegister(right->kept_register);
#endif
                right->kept_register=Null_Register;
	      }
	    }
	    else {
	      if (right)
		right_dest = (Register)right->generateCode_phase2(proc, rs, insn, base, noCost);
              rs->freeRegister(right_dest);
#ifdef alpha_dec_osf4_0
	      if (op == divOp)
		{
		  assert(false);
		  /* XXX
		   * The following doesn't work right, because we don't save
		   * and restore the scratch registers before and after the
		   * call!
		   */
		  /*
		  bool err;
		  Address divlAddr = proc->findInternalAddress("divide", true, err);  
		  assert(divlAddr);
		  software_divide(src,right_dest,dest,insn,base,noCost,divlAddr);
		  */
		}
	      else
#endif
	      emitV(op, src, right_dest, dest, insn, base, noCost);
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
	    emitVload(loadConstOp, (Address)oValue, dest, dest, 
                        insn, base, noCost);
	} else if (oType == ConstantPtr) {
	    emitVload(loadConstOp, (*(Address *) oValue), dest, dest, 
                        insn, base, noCost);
/*	} else if (oType == DataPtr) {
	    addr = (Address) oValue;
            assert(addr != 0); // check for NULL
	    emitVload(loadConstOp, addr, dest, dest, insn, base, noCost); */
	} else if (oType == DataIndir) {
	    src = (Register)loperand->generateCode_phase2(proc, rs, insn, base, noCost);
#ifdef BPATCH_LIBRARY
	    BPatch_type *Type = const_cast<BPatch_type *> (getType());
	    assert(Type);
	    int tSize = Type->getSize();
#else
	    int tSize = sizeof(int);
#endif
	    emitV(loadIndirOp, src, 0, dest, insn, base, noCost, tSize); 
            rs->freeRegister(src);
	} 
	else if (oType == DataReg) {
            rs->unkeep_register(dest);
            rs->freeRegister(dest);
            dest = (Address)oValue;
	} 
	else if(oType == PreviousStackFrameDataReg)
	  emitLoadPreviousStackFrameRegister((Address) oValue, dest,insn,base,
					     size, noCost);
	else if (oType == DataId) {
	    emitVload(loadConstOp, (Address)oValue, dest, dest, 
                        insn, base, noCost);
/*	} else if (oType == DataValue) {
	    addr = (Address) oValue;
	    assert(addr != 0); // check for NULL
	    emitVload(loadOp, addr, dest, dest, insn, base, noCost); */
	} else if (oType == ReturnVal) {
            rs->unkeep_register(dest);
	    rs->freeRegister(dest);
	    src = rs->allocateRegister(insn, base, noCost);
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
	    if (loperand) {
                instruction instr;
                instr.raw = (unsigned)(loperand->oValue);
		dest = emitOptReturn(instr, src, insn, base, noCost);
	    }
	    else if (astFlag)
		dest = emitR(getSysRetValOp, 0, 0, src, insn, base, noCost);
	    else 
#endif
	        dest = emitR(getRetValOp, 0, 0, src, insn, base, noCost);
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
		dest = emitR(getSysParamOp, (Register)oValue, 0, src, insn, base, noCost);
	    else 
#endif
		dest = emitR(getParamOp, (Address)oValue, 0, src, insn, base, noCost);
	    if (src != dest) {
		rs->freeRegister(src);
	    }
	} else if (oType == DataAddr) {
	  addr = (Address) oValue;
	  emitVload(loadOp, addr, dest, dest, insn, base, noCost, size);
	} else if (oType == FrameAddr) {
	  addr = (Address) oValue;
	  Register temp = rs->allocateRegister(insn, base, noCost);
	  emitVload(loadFrameRelativeOp, addr, temp, dest, insn, base, noCost);
	  rs->freeRegister(temp);
	} else if (oType == ConstantString) {
	  // XXX This is for the string type.  If/when we fix the string type
	  // to make it less of a hack, we'll need to change this.
	  int len = strlen((char *)oValue) + 1;
	  addr = (Address) inferiorMalloc(proc, len, textHeap); //dataheap
	  if (!proc->writeDataSpace((char *)addr, len, (char *)oValue))
	    perror("ast.C(1351): writing string value");
	  emitVload(loadConstOp, addr, dest, dest, insn, base, noCost);
	} 
    } else if (type == callNode) {
	 dest = emitFuncCall(callOp, rs, insn, base, operands, callee, proc, noCost,
			     calleefunc);
        if (useCount>0) {
          kept_register=dest;
          rs->keep_register(dest);
        }
    } else if (type == sequenceNode) {
#if defined(BPATCH_LIBRARY_F)
	(void) loperand->generateCode_phase2(proc, rs, insn, base, noCost);
#else
	Register tmp = loperand->generateCode_phase2(proc, rs, insn, base, noCost);
	rs->freeRegister(tmp);
#endif
 	return roperand->generateCode_phase2(proc, rs, insn, base, noCost);
    }

    // assert (dest != Null_Register); // oh dear, it seems this happens!

    return (Address)dest;
}


#if defined(AST_PRINT)
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
	case loadFrameRelativeOp: return("load $fp");
        case loadFrameAddr: return("$fp");
	case storeFrameRelativeOp: return("store $fp");
	case getAddrOp: return("&");
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
/*	} else if (oType == DataPtr) {
	    total = getInsnCost(loadConstOp);
	} else if (oType == DataValue) {
	total = getInsnCost(loadOp); */
	} else if (oType == DataId) {
	    total = getInsnCost(loadConstOp);
	} else if (oType == DataIndir) {
	    total = getInsnCost(loadIndirOp);
            total += loperand->cost();
	} else if (oType == DataAddr) {
	    total = getInsnCost(loadOp);
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

#if defined(AST_PRINT)
void AstNode::print() const {
  if (this) {
#if defined(ASTDEBUG)
    sprintf(errorLine,"{%d}", referenceCount) ;
    logLine(errorLine) ;
#endif
    if (type == operandNode) {
      if (oType == Constant) {
	sprintf(errorLine, " %d", (int)(Address) oValue);
	logLine(errorLine);
      } else if (oType == ConstantString) {
        sprintf(errorLine, " %s", (char *)oValue);
	logLine(errorLine) ;
/*
      } else if (oType == DataPtr) {
	sprintf(errorLine, " %d", (int)(Address) oValue);
	logLine(errorLine);
      } else if (oType == DataValue) {
	sprintf(errorLine, " @%d", (int)(Address) oValue);
	logLine(errorLine); 
*/
      } else if (oType == DataIndir) {
	logLine(" @[");
        loperand->print();
	logLine("]");
      } else if (oType == DataReg) {
	sprintf(errorLine," reg%d ",(int)(Address)oValue);
        logLine(errorLine);
        loperand->print();
      } else if (oType == Param) {
	sprintf(errorLine, " param[%d]", (int)(Address) oValue);
	logLine(errorLine);
      } else if (oType == ReturnVal) {
	sprintf(errorLine, "retVal");
	logLine(errorLine);
      } else if (oType == DataAddr)  {
	sprintf(errorLine, " [0x%lx]", (long) oValue);
	logLine(errorLine);
      } else if (oType == FrameAddr)  {
	sprintf(errorLine, " [$fp + %d]", (int)(Address) oValue);
	logLine(errorLine);
      } else {
	logLine(" <Unknown Operand>");
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

#if defined(MT_THREAD)

AstNode *computeAddress(void *level, void *index, int type)
{
  int tSize;

  /* DYNINSTthreadTable[0][thr_self()] */
  AstNode* t0 = new AstNode(AstNode::DataReg, (void *)REG_MT);
  AstNode* t7 ;

  /* Now we compute the offset for the corresponding level. We assume */
  /* that the DYNINSTthreadTable is stored by rows - naim 4/18/97 */
  if ((int)level != 0) {
    tSize = sizeof(unsigned);

    AstNode* t5 = new AstNode(AstNode::Constant, 
		    (void*) (MAX_NUMBER_OF_THREADS*((unsigned) level)*tSize)) ;

    /* Given the level and tid, we compute the position in the thread */
    /* table. */
    AstNode* t6 = new AstNode(plusOp, t0, t5);

    removeAst(t0);
    removeAst(t5);
    /* We then read the address, which is really the base address of the */
    /* vector of counters and timers in the shared memory segment. */
    t7 = new AstNode(AstNode::DataIndir, t6); 
    removeAst(t6);
  } else {
    /* if level is 0, we don't need to compute the offset */
    t7 = new AstNode(AstNode::DataIndir, t0);
    removeAst(t0);
  }
  return(t7);
}


AstNode *checkAddress(AstNode *addr, opCode op)
{  
  AstNode *null_value, *expression;
  null_value = new AstNode(AstNode::Constant,(void *)0);
  expression = new AstNode(op, addr, null_value);
  removeAst(null_value) ;
  return(expression);
}


AstNode *addIndexToAddress(AstNode *addr, void *index, int type)
{
  AstNode *t10, *t11;
  int tSize;

  /* Finally, using the index as an offset, we compute the address of the */
  /* counter/timer. */
  if (type==0) {
    /* intCounter */
    tSize = sizeof(intCounter);
  } else {
    /* Timer */
    tSize = sizeof(tTimer);
  }

  t10 = new AstNode(AstNode::Constant, (void*) (((unsigned)index)*tSize)) ;
  t11 = new AstNode(plusOp, addr, t10); /* address of counter/timer */
  removeAst(t10) ;

  return(t11);
}


AstNode *computeTheAddress(void *level, void *index, int type) {
  AstNode* base = computeAddress(level, index, type) ;
  AstNode* addr = addIndexToAddress(base, index, type) ;
  removeAst(base) ;
  return addr ;
}

AstNode *createTimer(const string &func, void *level, void *index,
                     vector<AstNode *> &ast_args)
{
  // t29: 
  // DYNINST_not_deleted
  vector<AstNode *> arg;
  AstNode* t29 = new AstNode("DYNINST_not_deleted", arg);

  //t18:
  // while (computeAddress() == 0) ;
  //
  vector<AstNode *> dummy ;
  AstNode* t30 = new AstNode("DYNINSTloop", dummy) ;
  AstNode* end = new AstNode(AstNode::Constant, (void*) 36) ;
  AstNode* t32 = new AstNode(branchOp, end) ;
  removeAst(end) ;
  AstNode* t31 = new AstNode(ifOp, t30, t32) ;
  removeAst(t30);
  removeAst(t32); 
  AstNode* t0 = computeAddress(level,index,1); /* 1 means tTimer */
  AstNode* t2 = checkAddress(t0, eqOp);
  removeAst(t0); 
  AstNode* t18 = new AstNode(whileOp, t2, t31) ;
  removeAst(t31) ;
  removeAst(t2) ; 

  //t1:
  // Timer
  AstNode* t3 = addIndexToAddress(t0, index, 1); /* 1 means tTimer */
  ast_args += (t3);
  AstNode* t1 = new AstNode(func, ast_args);
  for(unsigned i=0; i<ast_args.size(); i++)
    removeAst(ast_args[i]) ;

  //t5:
  // {
  //   while(T/C==0) ;
  //   Timer
  // }
  AstNode* t5 = new AstNode(t18, t1);
  removeAst(t18) ;
  removeAst(t1) ;

  //if(t29) t5 ;
  AstNode* ret = new AstNode(ifOp, t29, t5) ;
  removeAst(t29);
  removeAst(t5) ;

  return(ret);
}

AstNode *createCounter(const string &func, void *level, void *index, 
                       AstNode *ast) 
{ 
  AstNode *t0=NULL,*t1=NULL,*t2=NULL,*t3=NULL;
  AstNode *t4=NULL,*t5=NULL,*t6=NULL;

  t0 = computeAddress(level,index,0); /* 0 means intCounter */
  t4 = checkAddress(t0, neOp);
  t5 = addIndexToAddress(t0, index, 0); /* 0 means intCounter */
  removeAst(t0) ;

  if (func == "addCounter") {
    t1 = new AstNode(AstNode::DataIndir, t5);
    t2 = new AstNode(plusOp, t1, ast);
    t3 = new AstNode(storeIndirOp, t5, t2);
    removeAst(t2) ;
  } else if (func == "subCounter") {
    t1 = new AstNode(AstNode::DataIndir, t5);
    t2 = new AstNode(minusOp, t1, ast);
    t3 = new AstNode(storeIndirOp, t5, t2);
    removeAst(t2);
  } else if (func == "setCounter") {
    t3 = new AstNode(storeIndirOp, t5, ast);
  }
  removeAst(t5) ;

  t6 = new AstNode(ifOp, t4, t3) ;
  removeAst(t4);
  removeAst(t3);

  return(t6);
}

#else

// Single threaded case

AstNode *createTimer(const string &func, void *dataPtr, 
                     vector<AstNode *> &ast_args)
{
  AstNode *t0=NULL,*t1=NULL;

  t0 = new AstNode(AstNode::Constant, (void *) dataPtr);  // This was AstNode::DataPtr
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

   t0 = new AstNode(AstNode::DataAddr, (void *)dataPtr);  // This was AstNode::DataValue
   if (func=="addCounter") {
     t1 = new AstNode(plusOp,t0,ast);
     t2 = new AstNode(storeOp,t0,t1);
     removeAst(t1);
   } else if (func=="subCounter") {
     t1 = new AstNode(minusOp,t0,ast);
     t2 = new AstNode(storeOp,t0,t1);
     removeAst(t1);
   } else if (func=="setCounter") {
     t2 = new AstNode(storeOp,t0,ast);
   } else abort();
   removeAst(t0);
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

    if ((loperand || roperand) && getType()) {
	// something has already set the type for us.
	// this is likely an expression for array access
	ret = const_cast<BPatch_type *>(getType());
	return ret;
    }

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
	    } else if (op == funcJumpOp) {
		ret = BPatch::bpatch->type_Untyped;
	    } else if (op == getAddrOp) {
		// Should set type to the infered type not just void * 
		//  - jkh 7/99
		ret = BPatch::bpatch->stdTypes->findType("void *");
		assert(ret != NULL);
		break;
	    } else {
		// XXX The following line must change to decide based on the
		// types and operation involved what the return type of the
		// expression will be.
		ret = lType;
		if (lType != NULL && rType != NULL) {
		    if (!lType->isCompatible(rType)) {
			errorFlag = true;
		    }
		}
	    }
	    break;
	case operandNode:
	    if (oType == DataIndir) {
		assert(roperand == NULL);
		// XXX Should really be pointer to lType -- jkh 7/23/99
		ret = BPatch::bpatch->type_Untyped;
	    } else {
		assert(loperand == NULL && roperand == NULL);
		if ((oType == Param) || (oType == ReturnVal)) {
		    // XXX Params and ReturnVals untyped for now
		    ret = BPatch::bpatch->type_Untyped; 
		} else {
		    ret = const_cast<BPatch_type *>(getType());
		}
		assert(ret != NULL);
	    }
	    break;
	case callNode:
            unsigned i;
	    for (i = 0; i < operands.size(); i++) {
		BPatch_type *operandType = operands[i]->checkType();
		/* XXX Check operands for compatibility */
		if (operandType == BPatch::bpatch->type_Error) {
		    errorFlag = true;
		}
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
    } else if (errorFlag) {
	ret = BPatch::bpatch->type_Untyped;
    }

#if defined(ASTDEBUG)
    // it would be useful to have some indication of what the type applied to
    // (currently it appears to be copious amounts of contextless junk)
    if (ret) {
	logLine(" type is ");
	if (ret->getName()) 
	     logLine(ret->getName());
	else
	     logLine(" <NULL Name String>");
	logLine("\n");
    }
#endif

    // remember what type we are
    setType(ret);

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

// The following two functions are broken because some callNodes have
// a function_base* member (`calleefunc') that is not replaced by
// these functions.  For such Callnodes, the corresponding
// function_base* for func2 must be provided.  Neither of these
// functions is called these days.

// Looks for function func1 in ast and replaces it with function func2
void AstNode::replaceFuncInAst(function_base *func1, function_base *func2)
{
  if (type == callNode) {
       if (calleefunc) {
	    if (calleefunc == func1) {
		 callee = func2->prettyName();
		 calleefunc = func2;
	    }
       } else if (callee == func1->prettyName()) {
	    // Not all call nodes are initialized with function_bases.
	    // Preserve that, continue to deal in names only.
	    if (callee == func1->prettyName())
		 callee = func2->prettyName();
       }
  }
  if (loperand) loperand->replaceFuncInAst(func1,func2);
  if (roperand) roperand->replaceFuncInAst(func1,func2);
  for (unsigned i=0; i<operands.size(); i++)
    operands[i]->replaceFuncInAst(func1, func2) ;
} 

void AstNode::replaceFuncInAst(function_base *func1, function_base *func2,
			       vector<AstNode *> &more_args, int index)
{
  unsigned i ;
  if (type == callNode) {
      bool replargs = false;
      if (calleefunc) {
	   if (calleefunc == func1 || calleefunc == func2) {
		replargs = true;
		calleefunc = func2;
		callee = func2->prettyName();
	   }
      } else if (callee == func1->prettyName() || callee == func2->prettyName()) {
	   replargs = true;
	   callee = func2->prettyName();
      }
      if (replargs) {
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


// This is not the most efficient way to traverse a DAG
bool AstNode::accessesParam(void)
{
  bool ret = false;

  ret = (type == operandNode && oType == Param);

  if (!ret && loperand) 
       ret = loperand->accessesParam();
  if (!ret && roperand) 
      ret = roperand->accessesParam();
  if (!ret && eoperand) 
      ret = eoperand->accessesParam();
  for (unsigned i=0;i<operands.size(); i++) {
      if (!ret)
        ret = operands[i]->accessesParam() ;
      else
        break;
  }

  return ret;
}
