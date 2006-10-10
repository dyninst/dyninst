/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: ast.C,v 1.173 2006/10/10 22:04:23 bernat Exp $

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/showerror.h"

#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/src/BPatch_collections.h"
#include "dyninstAPI/h/BPatch_type.h"
#include "dyninstAPI/src/BPatch_libInfo.h" // For instPoint->BPatch_point mapping

#include "dyninstAPI/h/BPatch_point.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

#if defined(sparc_sun_sunos4_1_3) \
 || defined(sparc_sun_solaris2_4)
#include "dyninstAPI/src/inst-sparc.h"

#elif defined(hppa1_1_hp_hpux)
#include "dyninstAPI/src/inst-hppa.h"

#elif defined(rs6000_ibm_aix3_2) \
   || defined(rs6000_ibm_aix4_1)
#include "dyninstAPI/src/inst-power.h"

#elif defined(i386_unknown_solaris2_5) \
   || defined(i386_unknown_nt4_0) \
   || defined(i386_unknown_linux2_0) \
   || defined(x86_64_unknown_linux2_4)
#include "dyninstAPI/src/inst-x86.h"

#elif defined(ia64_unknown_linux2_4) /* Why is this done here, instead of, e.g., inst.h? */
#include "dyninstAPI/src/inst-ia64.h"
#endif


#include "registerSpace.h"
extern registerSpace *regSpace;

extern bool doNotOverflow(int value);

AstNode::AstNode() {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
   // used in mdl.C
   referenceCount = 1;
   useCount = 0;
   kept_register = Null_Register;
   // "operands" is left as an empty vector
   size = 4;
   bptype = NULL;
   doTypeCheck = true;
}

AstNode *AstNode::nullNode() {
    return new AstNullNode();
}

AstNode *AstNode::operandNode(operandType ot, void *arg) {
    AstNode *ret = new AstOperandNode(ot, arg);
    return ret;
}

// TODO: this is an indirect load; should be an operator.
AstNode *AstNode::operandNode(operandType ot, AstNode *ast) {
    AstNode *ret = new AstOperandNode(ot, ast);
    return ret;
}

AstNode *AstNode::sequenceNode(pdvector<AstNode *> &sequence) {
    AstNode *ret = new AstSequenceNode(sequence);
    return ret;
}

AstNode *AstNode::operatorNode(opCode ot, AstNode *l, AstNode *r, AstNode *e) {
    AstNode *ret = new AstOperatorNode(ot, l, r, e);
    return ret;
}

AstNode *AstNode::funcCallNode(const pdstring &func, pdvector<AstNode *> &args, process *proc) {
    if (proc) {
        int_function *ifunc = proc->findOnlyOneFunction(func);
        if (ifunc == NULL) return NULL;
        return new AstCallNode(ifunc, args);
    }
    else
        return new AstCallNode(func, args);
}

AstNode *AstNode::funcCallNode(int_function *func, pdvector<AstNode *> &args) {
    if (func == NULL) return NULL;
    return new AstCallNode(func, args);
}

AstNode *AstNode::funcReplacementNode(int_function *func) {
    if (func == NULL) return NULL;
    return new AstReplacementNode(func);
}

AstNode *AstNode::memoryNode(memoryType ma, int which) {
    return new AstMemoryNode(ma, which);
}

AstNode *AstNode::miniTrampNode(AstNode *tramp) {
    if (tramp == NULL) return NULL;
    return new AstMiniTrampNode(tramp);
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

#if 0
// TODO break into pieces
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
   if (type == opCodeNode_t)
      op = src.op; // defined only for operand nodes

   if (type == callNode) {
      callee = src.callee; // defined only for call nodes
      calleefunc = src.calleefunc;
      calleefuncAddr = src.calleefuncAddr;
      for (unsigned i=0;i<src.operands.size();i++) 
        operands.push_back(assignAst(src.operands[i]));
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

   size = src.size;
   bptype = src.bptype;
   doTypeCheck = src.doTypeCheck;

   return *this;
}
#endif

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

unsigned totalAstSize = 0;
unsigned totalAstCount = 0;

AstOperatorNode::AstOperatorNode(opCode opC, AstNode *l, AstNode *r, AstNode *e) :
    AstNode(),
    op(opC),
    loperand(NULL),
    roperand(NULL),
    eoperand(NULL)
{
    totalAstSize += sizeof(AstOperatorNode);
    totalAstCount++;
    if ((totalAstCount % 100) == 0)
        fprintf(stderr, "Cumulative AST count: %d, size %d\n",
                totalAstCount, totalAstSize);


    if (l) loperand = assignAst(l);
    if (r) roperand = assignAst(r);
    if (e) eoperand = assignAst(e);

    // Optimization pass...
    
    if (op == plusOp) {
        if (loperand->getoType() == Constant) {
            // Swap left and right...
            AstNode *temp = loperand;
            loperand = roperand;
            roperand = temp;
        }
    }
    if (op == timesOp) {
        if (roperand->getoType() == undefOperandType) {
            // ...
        }
        else if (roperand->getoType() != Constant) {
            AstNode *temp = roperand;
            roperand = loperand;
            loperand = temp;
        }
        else {
            int result;
            if (!isPowerOf2((Address)roperand->getOValue(),result) &&
                isPowerOf2((Address)loperand->getOValue(),result)) {
                AstNode *temp = roperand;
                roperand = loperand;
                loperand = temp;
            }
        }
    }
};

    // Direct operand
AstOperandNode::AstOperandNode(operandType ot, void *arg) :
    AstNode(),
    oType(ot),
    operand_(NULL) {
    totalAstSize += sizeof(AstOperandNode);
    totalAstCount++;
    if ((totalAstCount % 100) == 0)
        fprintf(stderr, "Cumulative AST count: %d, size %d\n",
                totalAstCount, totalAstSize);
    
    if (ot == ConstantString)
        oValue = (void *)P_strdup((char *)arg);
    else
        oValue = (void *) arg;
}

// And an indirect (say, a load)
AstOperandNode::AstOperandNode(operandType ot, AstNode *l) :
    AstNode(),
    oType(ot),
    oValue(NULL),
    operand_(l)
{
    totalAstSize += sizeof(AstOperandNode);
    totalAstCount++;
    if ((totalAstCount % 100) == 0)
        fprintf(stderr, "Cumulative AST count: %d, size %d\n",
                totalAstCount, totalAstSize);
}


AstCallNode::AstCallNode(int_function *func,
                         pdvector<AstNode *> &args) :
    AstNode(),
    func_(func)
{
    totalAstSize += sizeof(AstCallNode);
    totalAstCount++;
    if ((totalAstCount % 100) == 0)
        fprintf(stderr, "Cumulative AST count: %d, size %d\n",
                totalAstCount, totalAstSize);
    for (unsigned i = 0; i < args.size(); i++) {
        args_.push_back(assignAst(args[i]));
    }
}

AstCallNode::AstCallNode(const pdstring &func,
                         pdvector<AstNode *> &args) :
    AstNode(),
    func_name_(func),
    func_(NULL)
{
    totalAstSize += sizeof(AstCallNode);
    totalAstCount++;
    if ((totalAstCount % 100) == 0)
        fprintf(stderr, "Cumulative AST count: %d, size %d\n",
                totalAstCount, totalAstSize);
    for (unsigned i = 0; i < args.size(); i++) {
        args_.push_back(assignAst(args[i]));
    }
}
 
AstSequenceNode::AstSequenceNode(pdvector<AstNode *> &sequence) :
    AstNode()
{
    totalAstSize += sizeof(AstSequenceNode);
    totalAstCount++;
    if ((totalAstCount % 100) == 0)
        fprintf(stderr, "Cumulative AST count: %d, size %d\n",
                totalAstCount, totalAstSize);
    for (unsigned i = 0; i < sequence.size(); i++) {
        sequence_.push_back(assignAst(sequence[i]));
    }
}

AstInstructionNode::AstInstructionNode(instruction *insn) :
    AstNode(),
    insn_(insn) {
    totalAstSize += sizeof(AstInstructionNode);
    totalAstCount++;
    if ((totalAstCount % 100) == 0)
        fprintf(stderr, "Cumulative AST count: %d, size %d\n",
                totalAstCount, totalAstSize);
}

AstMemoryNode::AstMemoryNode(memoryType mem,
                             unsigned which) :
    AstNode(),
    mem_(mem),
    which_(which) {
    totalAstSize += sizeof(AstMemoryNode);
    totalAstCount++;
    if ((totalAstCount % 100) == 0)
        fprintf(stderr, "Cumulative AST count: %d, size %d\n",
                totalAstCount, totalAstSize);
    
    assert(BPatch::bpatch != NULL);
    assert(BPatch::bpatch->stdTypes != NULL);

    
    switch(mem_) {
    case EffectiveAddr:
        bptype = BPatch::bpatch->stdTypes->findType("void *");
        break;
    case BytesAccessed:
        bptype = BPatch::bpatch->stdTypes->findType("int");
        break;
    default:
        assert(!"Naah...");
    }
    size = bptype->getSize();
    doTypeCheck = BPatch::bpatch->isTypeChecked();
};

#if defined(ASTDEBUG)
#define AST_PRINT
#endif

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
  assert(src);

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


Address AstMiniTrampNode::generateTramp(process *proc, 
                                        const instPoint *location,
                                        codeGen &gen,
                                        int &trampCost, 
                                        bool noCost,
                                        bool merged) {
    static AstNode *trailer=NULL;
    static AstNode *costAst = NULL;
    static AstNode *preamble = NULL;

    if (trailer == NULL)
        trailer = AstNode::operatorNode(trampTrailer);

    if (costAst == NULL)
        costAst = AstNode::operandNode(AstNode::Constant, (void *)0);

    if (preamble == NULL)
        preamble = AstNode::operatorNode(trampPreamble, costAst);

    // private constructor; assumes NULL for right child
    
    // we only want to use the cost of the minimum statements that will
    // be executed.  Statements, such as the body of an if statement,
    // will have their costs added to the observed cost global variable
    // only if they are indeed called.  The code to do this in the minitramp
    // right after the body of the if.
    trampCost = preamble->maxCost() + minCost() + trailer->maxCost();

    costAst->setOValue((void *) (long) trampCost);
    
    // needed to initialize regSpace below
    // shouldn't we do a "delete regSpace" first to avoid
    // leaking memory?

    initTramps(proc->multithread_capable()); 

    regSpace->resetSpace();
    
#if defined( ia64_unknown_linux2_4 )
    extern Register deadRegisterList[];
    defineBaseTrampRegisterSpaceFor( location, regSpace, deadRegisterList);
#endif


    Address ret=0;

    Register tmp;
    preamble->generateCode(proc, regSpace, gen, noCost, true);
    removeAst(preamble);
    tmp = (Register) ast_->generateCode(proc, regSpace, gen, noCost, true, location);
    regSpace->freeRegister(tmp);
    if (!merged)
        ret = trailer->generateCode(proc, regSpace, gen, noCost, true);
    
    regSpace->resetSpace();
    
    if  (merged) {
        /* We save the information at the inst-point, since
           the next mini-tramp will reset the clobber information */
        regSpace->saveClobberInfo(location);
        regSpace->resetClobbers();
    }
    
    
    return(ret);
}

void AstNode::setUseCount(registerSpace *rs) 
{
    if (useCount == 0 || !canBeKept()) {
	kept_register = Null_Register;

	// Set useCounts for our children
	pdvector<AstNode*> children;
	getChildren(children);
	for (unsigned i=0; i<children.size(); i++) {
	    children[i]->setUseCount(rs);
	}
    }
    // Occasionally, we call setUseCount to fix up useCounts in the middle
    // of code generation. If the node has already been computed, we need to
    // bump up the ref count of the register that keeps our value
    if (hasKeptRegister()) {
	rs->incRefCount(kept_register);
    }    
    useCount++;

}

void AstNode::cleanUseCount(void)
{
    useCount = 0;
    kept_register = Null_Register;

    pdvector<AstNode*> children;
    getChildren(children);
    for (unsigned i=0; i<children.size(); i++) {
	children[i]->cleanUseCount();
    }
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

// Allocate a register and make it available for sharing if our
// node is shared
Register AstNode::allocateAndKeep(registerSpace *rs, 
				  const pdvector<AstNode*> &ifForks,
				  codeGen &gen, bool noCost)
{
    // Allocate a register
    Register dest = rs->allocateRegister(gen, noCost);

    //rs->fixRefCount(dest, useCount+1); // FIXME

    // Make this register available for sharing
    //if (useCount > 0) {
    //keepRegister(dest, ifForks);
    //}

    
    return dest;
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
Address AstNode::generateCode(process *proc,
			      registerSpace *rs,
			      codeGen &gen, 
                              bool noCost, 
                              bool root,
			      const instPoint *location) {
    // Note: MIPSPro compiler complains about redefinition of default argument
    if (root) {
        cleanUseCount();
        setUseCount(rs);
#if defined(ASTDEBUG)
        print() ;
#endif
    }

  // rs->checkLeaks(Null_Register);

  // Create an empty vector to keep track of if statements
  pdvector<AstNode*> ifForks;

  // note: this could return the value "(Address)(-1)" -- csserra
  Address tmp = generateCode_phase2(proc, rs, gen, noCost, 
				    ifForks, location);
  // rs->checkLeaks(tmp);

#if defined(ASTDEBUG)
  if (root) {
    bool err = false ;
    checkUseCount(rs, err) ;
  }
#endif
  return(tmp);
}

bool AstNode::previousComputationValid(Register &reg,
                                       const pdvector<AstNode *> &ifForks,
                                       registerSpace *rs) {
    return false; // FIXME

    if (hasKeptRegister()) {
        // Before using kept_register we need to make sure it was computed
        // on the same path as we are right now
        if (subpath(kept_path, ifForks)) { 
#if defined(ASTDEBUG)
            sprintf(errorLine,"==> Returning register %d <==\n",kept_register);
            logLine(errorLine);
#endif
            if (useCount == 0) { 
                Register tmp=kept_register;
                unkeepRegister();
                assert(!rs->isFreeRegister(tmp));
                reg = tmp;
                return true;
            }
            reg = kept_register;
            return true;
        }
        else {
            // Can't keep the register anymore
            forceUnkeepAndFree(rs);
            fixChildrenCounts(rs);
        }
    }
    return false;
}

Address AstNode::generateCode_phase2(process *,
                                     registerSpace *,
                                     codeGen &, bool,
                                     const pdvector<AstNode*> &,
                                     const instPoint *) {
    fprintf(stderr, "ERROR: call to AstNode generateCode_phase2; should be handled by subclass\n");
    fprintf(stderr, "Undefined phase2 for:\n");
    if (dynamic_cast<AstNullNode *>(this)) fprintf(stderr, "nullNode\n");
    if (dynamic_cast<AstOperatorNode *>(this)) fprintf(stderr, "operatorNode\n");
    if (dynamic_cast<AstOperandNode *>(this)) fprintf(stderr, "operandNode\n");
    if (dynamic_cast<AstCallNode *>(this)) fprintf(stderr, "callNode\n");
    if (dynamic_cast<AstReplacementNode *>(this)) fprintf(stderr, "replacementNode\n");
    if (dynamic_cast<AstSequenceNode *>(this)) fprintf(stderr, "seqNode\n");
    if (dynamic_cast<AstInstructionNode *>(this)) fprintf(stderr, "insnNode\n");
    if (dynamic_cast<AstMiniTrampNode *>(this)) fprintf(stderr, "miniTrampNode\n");
    if (dynamic_cast<AstMemoryNode *>(this)) fprintf(stderr, "memoryNode\n");
    assert(0);
	return 0;
}


Address AstNullNode::generateCode_phase2(process *,
                                         registerSpace *,
                                         codeGen &, bool,
                                         const pdvector<AstNode*> &,
                                         const instPoint *) {
    return 0;
}


Address AstInstructionNode::generateCode_phase2(process *,
                                                registerSpace *,
                                                codeGen &, bool,
                                                const pdvector<AstNode*> &,
                                                const instPoint *) {
    assert(0 && "Unimplemented");
    return 0;
}

Address AstReplacementNode::generateCode_phase2(process *proc,
                                                registerSpace *,
                                                codeGen &gen, bool noCost,
                                                const pdvector<AstNode*> &,
                                                const instPoint *location) {
    if (replacement) {
        emitFuncJump(funcJumpOp, gen, replacement, proc,
                     location, noCost);
    }
    return 0;
}


Address AstOperatorNode::generateCode_phase2(process *proc,
                                             registerSpace *rs,
                                             codeGen &gen, bool noCost,
                                             const pdvector<AstNode*> &ifForks,
                                             const instPoint *location) {
    Register reg;
    if (previousComputationValid(reg, ifForks, rs))
        return reg;

   Address addr;
   Register src1, src2;
   Register src = Null_Register;
   Register dest = Null_Register;
   Register right_dest = Null_Register;
    
    switch(op) {
    case branchOp: {
        assert(loperand->getoType() == Constant);
        unsigned offset = (RegValue)loperand->getOValue();
        // We are not calling loperand->generateCode_phase2,
        // so we decrement its useCount by hand.
        loperand->decUseCount(rs);
        (void)emitA(branchOp, 0, 0, (RegValue)offset, gen, noCost);
        break;
    }
    case ifOp: {
        // This ast cannot be shared because it doesn't return a register
        src = (Register)loperand->generateCode_phase2(proc, rs, gen, noCost, ifForks, location);
        codeBufIndex_t ifIndex= gen.getIndex();
        codeBufIndex_t thenSkipStart = emitA(op, src, 0, 0, gen, noCost);
        rs->freeRegister(src);
        
        // The flow of control forks. We need to add the forked node to 
        // the path
        pdvector<AstNode*> thenFork = ifForks;
        thenFork.push_back(roperand);
        Register tmp = (Register)roperand->generateCode_phase2(proc, rs, gen, noCost, thenFork, location);
        rs->freeRegister(tmp);
        
        // Is there an else clause?  If yes, generate branch over it
        codeBufIndex_t elseSkipStart = 0;
        codeBufIndex_t elseSkipIndex = gen.getIndex();
        if (eoperand) 
            elseSkipStart = emitA(branchOp, 0, 0, 0,
                                  gen, noCost);
        // Now that we've generated the "then" section, rewrite the if
        // conditional branch.
        codeBufIndex_t elseStartIndex = gen.getIndex();
        
        gen.setIndex(ifIndex);
        // call emit again now with correct offset.
        // This backtracks over current code.
        // If/when we vectorize, we can do this in a two-pass arrangement
        (void) emitA(op, src, 0, 
                     (Register) codeGen::getDisplacement(thenSkipStart, elseStartIndex),
                     gen, noCost);
        gen.setIndex(elseStartIndex);
        
        if (eoperand) {
            // If there's an else clause, we need to generate code for it.
            pdvector<AstNode*> elseFork = ifForks;
            elseFork.push_back(eoperand);// Add the forked node to the path
            tmp = (Register)eoperand->generateCode_phase2(proc, rs, gen,
                                                          noCost, 
                                                          elseFork,
                                                          location);
            rs->freeRegister(tmp);
            
            // We also need to fix up the branch at the end of the "true"
            // clause to jump around the "else" clause.
            codeBufIndex_t endIndex = gen.getIndex();
            gen.setIndex(elseSkipIndex);
            emitA(branchOp, 0, 0, 
                  (Register) codeGen::getDisplacement(elseSkipStart, endIndex),
                  gen, noCost);
            gen.setIndex(endIndex);
        }
    }
        break;
    case ifMCOp: {
        assert(location);
        
        // TODO: Right now we get the condition from the memory access info,
        // because scanning for memory accesses is the only way to detect these
        // conditional instructions. The right way(TM) would be to attach that
        // info directly to the point...
        // Okay. The info we need is stored in the BPatch_point. We have the instPoint. 
        // Yay.
        
        // Since someone who shall not be named removed the BPatch
        // link from the process class, we perform a PID-based
        // lookup.
        BPatch_process *bproc = BPatch::bpatch->getProcessByPid(proc->getPid());
        BPatch_point *bpoint = bproc->instp_map->get(location);
        
        const BPatch_memoryAccess* ma = bpoint->getMemoryAccess();
        assert(ma);
        int cond = ma->conditionCode_NP();
        if(cond > -1) {
            codeBufIndex_t startIndex = gen.getIndex();
            emitJmpMC(cond, 0 /* target, changed later */, gen);
            codeBufIndex_t fromIndex = gen.getIndex();
            // Add the snippet to the ifForks, as AM has indicated...
            pdvector<AstNode*> thenFork = ifForks;
            thenFork.push_back(loperand);
            // generate code with the right path
            Register tmp = (Register)loperand->generateCode_phase2(proc, rs, gen,
                                                                   noCost, thenFork, location);
            rs->freeRegister(tmp);
            codeBufIndex_t endIndex = gen.getIndex();
            // call emit again now with correct offset.
            gen.setIndex(startIndex);
            emitJmpMC(cond, codeGen::getDisplacement(fromIndex, endIndex), gen);
            gen.setIndex(endIndex);
        }
        else {
            Register tmp = (Register)loperand->generateCode_phase2(proc, rs, gen,
                                                                   noCost, ifForks, location);
            rs->freeRegister(tmp);
        }
    }
        break;
    case whileOp: {
        codeBufIndex_t top = gen.getIndex();
        src = (Register)loperand->generateCode_phase2(proc, rs, gen, noCost, ifForks, location);
        codeBufIndex_t startIndex = gen.getIndex();
        codeBufIndex_t fromIndex = emitA(ifOp, src, 0, 0, gen, noCost);
        rs->freeRegister(src);
        if (roperand) {
            pdvector<AstNode*> whileFork = ifForks;
            whileFork.push_back(eoperand);//Add the forked node to the path
            Register tmp = 
                (Register)roperand->generateCode_phase2(proc, rs, 
                                                        gen, noCost,
                                                        whileFork, 
                                                        location);
            rs->freeRegister(tmp);
        }
        //jump back
        (void) emitA(branchOp, 0, 0, codeGen::getDisplacement(top, gen.getIndex()),
                     gen, noCost);
        
        // Rewind and replace the skip jump
        codeBufIndex_t endIndex = gen.getIndex();
        gen.setIndex(startIndex);
        (void) emitA(ifOp, src, 0, (Register) codeGen::getDisplacement(fromIndex, endIndex), 
                     gen, noCost);
        gen.setIndex(endIndex);
        // sprintf(errorLine,"branch forward %d\n", base - fromAddr);
    }
        break;
    case doOp: {
        assert(0) ;
    }
        break;
    case getAddrOp: {
        switch(loperand->getoType()) {
        case DataAddr:
            addr = (Address) loperand->getOValue();
            assert(addr != 0); // check for NULL
            dest = allocateAndKeep(rs, ifForks, gen, noCost);
            emitVload(loadConstOp, addr, dest, dest, gen, noCost);
            break;
        case FrameAddr: {
            // load the address fp + addr into dest
            dest = allocateAndKeep(rs, ifForks, gen, noCost);
            Register temp = rs->allocateRegister(gen, noCost);
            addr = (Address) loperand->getOValue();
	    
            emitVload(loadFrameAddr, addr, temp, dest, gen,
                      noCost, rs, size, location, proc);
            rs->freeRegister(temp);
            break;
        }
        case RegOffset: {
            assert(loperand);
            assert(loperand->operand());
            
            // load the address reg + addr into dest
            dest = allocateAndKeep(rs, ifForks, gen, noCost);
            addr = (Address) loperand->operand()->getOValue();
	    
            emitVload(loadRegRelativeAddr, addr, (long)loperand->getOValue(), dest, gen,
                      noCost, rs, size, location, proc);
            break;
        }
        case DataIndir:
            // taking address of pointer de-ref returns the original
            //    expression, so we simple generate the left child's 
            //    code to get the address 
            dest = (Register)loperand->operand()->generateCode_phase2(proc, 
                                                                      rs, gen, 
                                                                      noCost, 
                                                                      ifForks, location);
            // Broken refCounts?
            break;
        default:
            assert(0);
        }
        break;
    }
    case storeOp: {
        // This ast cannot be shared because it doesn't return a register
        src1 = (Register)roperand->generateCode_phase2(proc, rs, gen, 
                                                       noCost, ifForks, location);
        // loperand's value will be invalidated. Discard the cached reg
        if (loperand->hasKeptRegister()) {
            loperand->forceUnkeepAndFree(rs);
        }
        // We will access loperand's children directly. They do not expect
        // it, so we need to bump up their useCounts
        loperand->fixChildrenCounts(rs);
        
        src2 = rs->allocateRegister(gen, noCost);
        switch (loperand->getoType()) {
        case DataAddr:
            addr = (Address) loperand->getOValue();
            assert(addr != 0); // check for NULL
            emitVstore(storeOp, src1, src2, addr, gen, noCost, rs, size);
            // We are not calling generateCode for the left branch,
            // so need to decrement the refcount by hand
            loperand->decUseCount(rs);
            break;
        case FrameAddr:
            addr = (Address) loperand->getOValue();
            emitVstore(storeFrameRelativeOp, src1, src2, addr, gen, 
                       noCost, rs, size, location, proc);
            loperand->decUseCount(rs);
            break;
        case RegOffset: {
            assert(loperand->operand());
            addr = (Address) loperand->operand()->getOValue();
            
            // This is cheating, but I need to pass 4 data values into emitVstore, and
            // it only allows for 3.  Prepare the dest address in scratch register src2.
            
            emitVload(loadRegRelativeAddr, addr, (long)loperand->getOValue(), src2,
                      gen, noCost, rs, size, location, proc);
            
            // Same as DataIndir at this point.
            emitV(storeIndirOp, src1, 0, src2, gen, noCost, rs, size, location, proc);
            loperand->decUseCount(rs);
            break;
        }
        case DataIndir: {
            // store to a an expression (e.g. an array or field use)
            // *(+ base offset) = src1
            Register tmp = 
                (Register)loperand->operand()->generateCode_phase2(proc, rs, 
                                                                   gen,
                                                                   noCost,
                                                                   ifForks,
                                                                   location);
            // tmp now contains address to store into
            emitV(storeIndirOp, src1, 0, tmp, gen, noCost, rs, size, location, proc);
            rs->freeRegister(tmp);
            loperand->decUseCount(rs);
            break;
        }
        default: {
            // Could be an error, could be an attempt to load based on an arithmetic expression
            // Generate the left hand side, store the right to that address
            Register tmp = (Register)loperand->generateCode_phase2(proc, rs, gen, noCost, ifForks, location);
            emitV(storeIndirOp, src1, 0, tmp, gen, noCost, rs, size, location, proc);
            rs->freeRegister(tmp);
            break;
        }
        }
        rs->freeRegister(src1);
        rs->freeRegister(src2);
        break;
    }
    case storeIndirOp: {
        src1 = (Register)roperand->generateCode_phase2(proc, rs, gen, noCost, ifForks, location);
        src2 = (Register)loperand->generateCode_phase2(proc, rs, gen, noCost, ifForks, location);
        emitV(op, src1, 0, src2, gen, noCost);          
        rs->freeRegister(src1);
        rs->freeRegister(src2);
        break;
    }
    case trampTrailer: {
        // This ast cannot be shared because it doesn't return a register
        codeBufIndex_t ret_index = emitA(op, 0, 0, 0, gen, noCost);
        // Convert to a bytecount
        // From 0: assume we're starting at the beginning of the code generator. Not a tremendous
        // assumption but valid in all cases.
        return gen.getDisplacement(0, ret_index);
        break;
    }
    case trampPreamble: {
        // This ast cannot be shared because it doesn't return a register
#if defined(i386_unknown_solaris2_5) \
 || defined(sparc_sun_solaris2_4)
        // loperand is a constant AST node with the cost, in cycles.
        //int cost = noCost ? 0 : (int) loperand->getOValue();
        Address costAddr = 0; // for now... (won't change if noCost is set)
        loperand->decUseCount(rs);
        costAddr = proc->getObservedCostAddr();
        return emitA(op, 0, 0, 0, gen, noCost);
#endif
        break;
    }
    default: {
        src = Null_Register;
        right_dest = Null_Register;
        if (loperand) {
            src = (Register)loperand->generateCode_phase2(proc, rs, gen,
                                                          noCost, ifForks,
                                                          location);
        }
        
        if (roperand &&
            (roperand->getoType() == Constant) &&
            doNotOverflow((RegValue)roperand->getOValue()))
            {
                rs->freeRegister(src); // may be able to reuse it for dest
                dest = allocateAndKeep(rs, ifForks, gen, noCost);
                
                emitImm(op, src, (RegValue)roperand->getOValue(), dest, gen, noCost, rs);
                // We do not .generateCode for roperand, so need to update its
                // refcounts manually
                roperand->decUseCount(rs);
            }
        else {
            if (roperand)
                right_dest = (Register)roperand->generateCode_phase2(proc, rs, gen, noCost, ifForks, location);
	    rs->freeRegister(src); // may be able to reuse it for dest
	    rs->freeRegister(right_dest); // may be able to reuse it for dest
            dest = allocateAndKeep(rs, ifForks, gen, noCost);
            emitV(op, src, right_dest, dest, gen, noCost, rs, size, location, proc);
        }
    }
    }
    return (Address) dest;
}

Address AstOperandNode::generateCode_phase2(process *proc,
                                            registerSpace *rs,
                                            codeGen &gen, bool noCost,
                                            const pdvector<AstNode*> &ifForks,
                                            const instPoint *location) {
    Register reg;
    if (previousComputationValid(reg, ifForks, rs))
        return reg;

    


   Address addr;
   Register src = Null_Register;
   Register dest = Null_Register;

   useCount--;
#if defined(ASTDEBUG)
   sprintf(errorLine,"### location: %p ###\n", location);
   logLine(errorLine);
#endif
   // Allocate a register to return
   if (oType != DataReg) {
       dest = allocateAndKeep(rs, ifForks, gen, noCost);
   }
   Register temp;
   int tSize;
   int len;
   BPatch_type *Type;
   switch (oType) {
   case Constant:
       emitVload(loadConstOp, (Address)oValue, dest, dest, 
		 gen, noCost);
       break;
   case DataIndir:
       src = (Register)operand_->generateCode_phase2(proc, rs, gen, noCost, ifForks, location);
       Type = const_cast<BPatch_type *> (getType());
       // Internally generated calls will not have type information set
       if(Type)
           tSize = Type->getSize();
       else
           tSize = sizeof(long);
       emitV(loadIndirOp, src, 0, dest, gen, noCost, rs, tSize, location, proc); 
       rs->freeRegister(src);
       break;
   case DataReg:
       dest = (Address)oValue;
       
       break;
   case PreviousStackFrameDataReg:
       emitLoadPreviousStackFrameRegister((Address) oValue, dest, gen,
                                          size, noCost);
       break;
   case ReturnVal:
       src = emitR(getRetValOp, 0, 0, dest, gen, noCost, location,
                   proc->multithread_capable());
       if (src != dest) {
           // Move src to dest. Can't simply return src, since it was not
           // allocated properly
           emitImm(orOp, src, 0, dest, gen, noCost, rs);
       }
       break;
   case Param:
       src = emitR(getParamOp, (Address)oValue, Null_Register,
                   dest, gen, noCost, location,
                   proc->multithread_capable());
       if (src != dest) {
           // Move src to dest. Can't simply return src, since it was not
           // allocated properly
           emitImm(orOp, src, 0, dest, gen, noCost, rs);
       }
       break;
   case DataAddr:
       addr = (Address) oValue;
       emitVload(loadOp, addr, dest, dest, gen, noCost, NULL, size);
       break;
   case FrameAddr:
       addr = (Address) oValue;
       temp = rs->allocateRegister(gen, noCost);
       
       emitVload(loadFrameRelativeOp, addr, temp, dest, gen, noCost, rs, size, location, proc);
       rs->freeRegister(temp);
       break;
   case RegOffset:
       // Prepare offset from value in any general register (not just fp).
       // This AstNode holds the register number, and loperand holds offset.
       assert(operand_);
       addr = (Address) operand_->getOValue();
       emitVload(loadRegRelativeOp, addr, (long)oValue, dest, gen, noCost, rs, size, location, proc);
       break;
   case ConstantString:
       // XXX This is for the pdstring type.  If/when we fix the pdstring type
       // to make it less of a hack, we'll need to change this.
       len = strlen((char *)oValue) + 1;
       
#if defined(rs6000_ibm_aix4_1) //ccw 30 jul 2002
       if(proc->requestTextMiniTramp){
           bool mallocFlag;
           addr = (Address) proc->inferiorMalloc(len, (inferiorHeapType) (textHeap | uncopiedHeap), 0x10000000, &mallocFlag); //textheap
       }else{	
           addr = (Address) proc->inferiorMalloc(len, dataHeap); //dataheap
       }
#else
       addr = (Address) proc->inferiorMalloc(len, dataHeap); //dataheap
#endif
       
       if (!proc->writeDataSpace((char *)addr, len, (char *)oValue))
           perror("ast.C(1351): writing string value");
       
       emitVload(loadConstOp, addr, dest, dest, gen, noCost);
       break;
   default:
       cerr << "Unknown oType " << oType << endl;
       assert(0 && "Unknown oType in operandNode");
       break;
   }
    return (Address) dest;
}

Address AstMemoryNode::generateCode_phase2(process *proc,
                                           registerSpace *rs,
                                           codeGen &gen, bool noCost,
                                           const pdvector<AstNode*> &ifForks,
                                           const instPoint *location) {
    Register reg;
    if (previousComputationValid(reg, ifForks, rs))
        return reg;

    const BPatch_memoryAccess* ma;
    const BPatch_addrSpec_NP *start;
    const BPatch_countSpec_NP *count;
    Register dest = allocateAndKeep(rs, ifForks, gen, noCost);    
    switch(mem_) {
    case EffectiveAddr: {
        
        // VG(11/05/01): get effective address
        // VG(07/31/02): take care which one
        // 1. get the point being instrumented & memory access info
        assert(location);
        
        BPatch_process *bproc = BPatch::bpatch->getProcessByPid(proc->getPid());
        BPatch_point *bpoint = bproc->instp_map->get(location);
        ma = bpoint->getMemoryAccess();
        if(!ma) {
            bpfatal( "Memory access information not available at this point.\n");
            bpfatal( "Make sure you create the point in a way that generates it.\n");
            bpfatal( "E.g.: find*Point(const BPatch_Set<BPatch_opCode>& ops).\n");
            assert(0);
        }
        if(which_ >= ma->getNumberOfAccesses()) {
            bpfatal( "Attempt to instrument non-existent memory access number.\n");
            bpfatal( "Consider using filterPoints()...\n");
            assert(0);
        }
        start = ma->getStartAddr(which_);
        emitASload(start, dest, gen, noCost);
        break;
    }
    case BytesAccessed: {
        // 1. get the point being instrumented & memory access info
        assert(location);
        
        BPatch_process *bproc = BPatch::bpatch->getProcessByPid(proc->getPid());
        BPatch_point *bpoint = bproc->instp_map->get(location);
        ma = bpoint->getMemoryAccess();
        if(!ma) {
            bpfatal( "Memory access information not available at this point.\n");
            bpfatal("Make sure you create the point in a way that generates it.\n");
            bpfatal( "E.g.: find*Point(const BPatch_Set<BPatch_opCode>& ops).\n");
            assert(0);
        }
        if(which_ >= ma->getNumberOfAccesses()) {
            bpfatal( "Attempt to instrument non-existent memory access number.\n");
            bpfatal( "Consider using filterPoints()...\n");
            assert(0);
        }
        count = ma->getByteCount(which_);
        emitCSload(count, dest, gen, noCost);
        break;
    }
    default:
        assert(0);
    }
    return (Address) dest;
}

Address AstCallNode::generateCode_phase2(process *proc,
                                             registerSpace *rs,
                                             codeGen &gen, bool noCost,
                                             const pdvector<AstNode*> &ifForks,
                                             const instPoint *location) {
    Register reg;
    if (previousComputationValid(reg, ifForks, rs))
        return reg;

    Register dest;

    // VG(11/06/01): This platform independent fn calls a platfrom
    // dependent fn which calls it back for each operand... Have to
    // fix those as well to pass location...

    int_function *use_func = func_;

    if (!use_func) {
        // We purposefully don't cache the int_function object; the AST nodes
        // are process independent, and functions kinda are.
        use_func = proc->findOnlyOneFunction(func_name_);
    }
    if (!use_func) {
        char msg[256];
        sprintf(msg, "%s[%d]:  internal error:  unable to find %s",
                __FILE__, __LINE__, func_name_.c_str());
        showErrorCallback(100, msg);
        assert(0);  // can probably be more graceful
    }
    
    Register tmp = emitFuncCall(callOp, rs, gen, args_,  
                                proc, noCost, use_func, ifForks, 
                                location);
    if (useCount > 0) {
        rs->fixRefCount(tmp, useCount+1);
        keepRegister(tmp, ifForks);
    }
    dest = tmp;

    return (Address) dest;
}

Address AstSequenceNode::generateCode_phase2(process *proc,
                                             registerSpace *rs,
                                             codeGen &gen, bool noCost,
                                             const pdvector<AstNode*> &ifForks,
                                             const instPoint *location) {
    Register reg;
    if (previousComputationValid(reg, ifForks, rs))
        return reg;
    
    Register dest = 0;
    
    for (unsigned i = 0; i < sequence_.size(); i++) {
        dest = sequence_[i]->generateCode_phase2(proc, rs, gen, 
                                                 noCost, ifForks,location);
        if (i < (sequence_.size()-1)) {
            rs->freeRegister(dest);
        }
    }

   // assert (dest != Null_Register); // oh dear, it seems this happens!

    return (Address)dest;
}


#if defined(AST_PRINT)
pdstring getOpString(opCode op)
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
	case ifMCOp: return("ifMC");
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
	case loadRegRelativeOp: return("load $reg");
        case loadFrameAddr: return("$fp");
	case storeFrameRelativeOp: return("store $fp");
	case getAddrOp: return("&");
	default: return("ERROR");
    }
}
#endif

#undef MIN
#define MIN(x,y) ((x)>(y) ? (y) : (x))
#undef MAX
#define MAX(x,y) ((x)>(y) ? (x) : (y))
#undef AVG
#define AVG(x,y) (((x)+(y))/2)

int AstOperatorNode::costHelper(enum CostStyleType costStyle) const {
    int total = 0;
    int getInsnCost(opCode t);
    
    if (op == ifOp) {
        // loperand is the conditional expression
        if (loperand) total += loperand->costHelper(costStyle);
        total += getInsnCost(op);
        int rcost = 0, ecost = 0;
        if (roperand) {
            rcost = roperand->costHelper(costStyle);
            if (eoperand)
                rcost += getInsnCost(branchOp);
        }
        if (eoperand)
            ecost = eoperand->costHelper(costStyle);
        if(ecost == 0) { // ie. there's only the if body
            if(costStyle      == Min)  total += 0;
            //guess half time body not executed
            else if(costStyle == Avg)  total += rcost / 2;
            else if(costStyle == Max)  total += rcost;
        } else {  // ie. there's an else block also, for the statements
            if(costStyle      == Min)  total += MIN(rcost, ecost);
            else if(costStyle == Avg)  total += AVG(rcost, ecost);
            else if(costStyle == Max)  total += MAX(rcost, ecost);
        }
    } else if (op == storeOp) {
        if (roperand) total += roperand->costHelper(costStyle);
        total += getInsnCost(op);
    } else if (op == storeIndirOp) {
        if (loperand) total += loperand->costHelper(costStyle);
        if (roperand) total += roperand->costHelper(costStyle);
        total += getInsnCost(op);
    } else if (op == trampTrailer) {
        total = getInsnCost(op);
    } else if (op == trampPreamble) {
        total = getInsnCost(op);
    } else {
        if (loperand) 
            total += loperand->costHelper(costStyle);
        if (roperand) 
            total += roperand->costHelper(costStyle);
        total += getInsnCost(op);
    }
    return total;
}

int AstOperandNode::costHelper(enum CostStyleType costStyle) const {
    int total = 0;
    if (oType == Constant) {
        total = getInsnCost(loadConstOp);
    } else if (oType == DataIndir) {
        total = getInsnCost(loadIndirOp);
        total += operand()->costHelper(costStyle);
    } else if (oType == DataAddr) {
        total = getInsnCost(loadOp);
    } else if (oType == DataReg) {
        total = getInsnCost(loadIndirOp);
    } else if (oType == Param) {
        total = getInsnCost(getParamOp);
    }
    return total;
}

int AstCallNode::costHelper(enum CostStyleType costStyle) const {
    int total = 0;
    if (func_) total += getPrimitiveCost(func_->prettyName());
    else total += getPrimitiveCost(func_name_);

    for (unsigned u = 0; u < args_.size(); u++)
        if (args_[u]) total += args_[u]->costHelper(costStyle);
    return total;
}
    

int AstSequenceNode::costHelper(enum CostStyleType costStyle) const {
    int total = 0;
    for (unsigned i = 0; i < sequence_.size(); i++) {
        total += sequence_[i]->costHelper(costStyle);
    }

    return total;
}

#if defined(AST_PRINT)
void AstNode::print() const {
  if (this) {
#if defined(ASTDEBUG)
    bpfatal("{%d}", referenceCount) ;
#endif
    if (type == operandNode) {
      if (oType == Constant) {
	fprintf(stderr,"%d", (int)(Address) oValue);
      } else if (oType == ConstantString) {
        fprintf(stderr," %s", (char *)oValue);
      } else if (oType == DataIndir) {
	fprintf(stderr," @[");
        loperand->print();
	fprintf(stderr,"]");
      } else if (oType == DataReg) {
	fprintf(stderr," reg%d ",(int)(Address)oValue);
        loperand->print();
      } else if (oType == Param) {
	fprintf(stderr," param[%d]", (int)(Address) oValue);
      } else if (oType == ReturnVal) {
	fprintf(stderr,"retVal");
      } else if (oType == DataAddr)  {
	fprintf(stderr," [0x%lx]", (long) oValue);
      } else if (oType == FrameAddr)  {
	fprintf(stderr," [$fp + %d]", (int)(Address) oValue);
      } else if (oType == RegOffset)  {
	fprintf(stderr," [$%d + %d]", (int)(Address) loperand->getOValue(), (int)(Address) oValue);
      } else if (oType == EffectiveAddr)  {
	fprintf(stderr," <<effective address>>");
      } else if (oType == BytesAccessed)  {
	fprintf(stderr," <<bytes accessed>>");
      } else {
	fprintf(stderr," <Unknown Operand>");
      }
    } else if (type == opCodeNode_t) {
      cerr << "(" << getOpString(op);
      if (loperand) loperand->print();
      if (roperand) roperand->print();
      if (eoperand) eoperand->print();
      fprintf(stderr,")\n");
    } else if (type == callNode) {
      cerr << "(" << callee;
      for (unsigned u = 0; u < operands.size(); u++)
	operands[u]->print();
      fprintf(stderr,")\n");
    } else if (type == sequenceNode_t) {
      if (loperand) loperand->print();
      fprintf(stderr,",");
      if (roperand) roperand->print();
      fprintf(stderr,"\n");
    }
  }
}
#endif

BPatch_type *AstNode::checkType() {
    return BPatch::bpatch->type_Untyped;
}

BPatch_type *AstOperatorNode::checkType() {
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

    if (loperand) lType = loperand->checkType();

    if (roperand) rType = roperand->checkType();

    if (eoperand) eType = eoperand->checkType();

    if (lType == BPatch::bpatch->type_Error ||
	rType == BPatch::bpatch->type_Error)
	errorFlag = true;
    
    switch (op) {
    case ifOp:
        // XXX No checking for now.  Should check that loperand
        // is boolean.
        ret = BPatch::bpatch->type_Untyped;
        break;
    case noOp:
        ret = BPatch::bpatch->type_Untyped;
        break;
    case funcJumpOp:
        ret = BPatch::bpatch->type_Untyped;
        break;
    case getAddrOp:
        // Should set type to the infered type not just void * 
        //  - jkh 7/99
        ret = BPatch::bpatch->stdTypes->findType("void *");
        assert(ret != NULL);
        break;
    default:
        // XXX The following line must change to decide based on the
        // types and operation involved what the return type of the
        // expression will be.
        ret = lType;
        if (lType != NULL && rType != NULL) {
            if (!lType->isCompatible(rType)) {
                errorFlag = true;
            }
        }
        break;
    }
    assert (ret != NULL);

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

BPatch_type *AstOperandNode::checkType()
{
    BPatch_type *ret = NULL;
    BPatch_type *type = NULL;
    bool errorFlag = false;
    
    assert(BPatch::bpatch != NULL);	/* We'll use this later. */

    if (operand_ && getType()) {
	// something has already set the type for us.
	// this is likely an expression for array access
	ret = const_cast<BPatch_type *>(getType());
	return ret;
    }
    
    if (operand_) type = operand_->checkType();

    if (type == BPatch::bpatch->type_Error)
	errorFlag = true;
    
    if (oType == DataIndir) {
        // XXX Should really be pointer to lType -- jkh 7/23/99
        ret = BPatch::bpatch->type_Untyped;
    } else {
        if ((oType == Param) || (oType == ReturnVal)) {
            // XXX Params and ReturnVals untyped for now
            ret = BPatch::bpatch->type_Untyped; 
        } else {
            ret = const_cast<BPatch_type *>(getType());
        }
        assert(ret != NULL);
    }

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


BPatch_type *AstCallNode::checkType() {
    BPatch_type *ret = NULL;
    bool errorFlag = false;
    
    assert(BPatch::bpatch != NULL);	/* We'll use this later. */
    
    unsigned i;
    for (i = 0; i < args_.size(); i++) {
        BPatch_type *operandType = args_[i]->checkType();
        /* XXX Check operands for compatibility */
        if (operandType == BPatch::bpatch->type_Error) {
            errorFlag = true;
        }
    }
    /* XXX Should set to return type of function. */
    ret = BPatch::bpatch->type_Untyped;

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

BPatch_type *AstSequenceNode::checkType() {
    BPatch_type *ret = NULL;
    BPatch_type *sType = NULL;
    bool errorFlag = false;
    
    assert(BPatch::bpatch != NULL);	/* We'll use this later. */

    if (getType()) {
	// something has already set the type for us.
	// this is likely an expression for array access
	ret = const_cast<BPatch_type *>(getType());
	return ret;
    }

    for (unsigned i = 0; i < sequence_.size(); i++) {
        sType = sequence_[i]->checkType();
        if (sType == BPatch::bpatch->type_Error)
            errorFlag = true;
    }

    ret = sType;

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

bool AstNode::accessesParam() {
#if 0
    fprintf(stderr, "Undefined call to getChildren for type: ");
    if (dynamic_cast<AstNullNode *>(this)) fprintf(stderr, "nullNode\n");
    else if (dynamic_cast<AstOperatorNode *>(this)) fprintf(stderr, "operatorNode\n");
    else if (dynamic_cast<AstOperandNode *>(this)) fprintf(stderr, "operandNode\n");
    else if (dynamic_cast<AstCallNode *>(this)) fprintf(stderr, "callNode\n");
    else if (dynamic_cast<AstReplacementNode *>(this)) fprintf(stderr, "replacementNode\n");
    else if (dynamic_cast<AstSequenceNode *>(this)) fprintf(stderr, "seqNode\n");
    else if (dynamic_cast<AstInstructionNode *>(this)) fprintf(stderr, "insnNode\n");
    else if (dynamic_cast<AstMiniTrampNode *>(this)) fprintf(stderr, "miniTrampNode\n");
    else if (dynamic_cast<AstMemoryNode *>(this)) fprintf(stderr, "memoryNode\n");
    else fprintf(stderr, "unknownNode\n");
#endif
    return false;
}


// This is not the most efficient way to traverse a DAG
bool AstOperatorNode::accessesParam()
{
    bool ret = false;
    if (loperand)
        ret |= loperand->accessesParam();
    if (roperand)
        ret |= roperand->accessesParam();
    if (eoperand)
        ret |= eoperand->accessesParam();
    return ret;
}


bool AstCallNode::accessesParam() {
    for (unsigned i = 0; i < args_.size(); i++) {
        if (args_[i]->accessesParam())
            return true;
    }
    return false;
}

bool AstSequenceNode::accessesParam() {
    for (unsigned i = 0; i < sequence_.size(); i++) {
        if (sequence_[i]->accessesParam())
            return true;
    }
    return false;
}

// Record the register to share as well as the path that lead
// to its computation
void AstNode::keepRegister(Register r, pdvector<AstNode*> path)
{
    kept_register = r;
    kept_path = path;
}

void AstNode::unkeepRegister()
{
    assert(kept_register != Null_Register);
    assert(useCount == 0); // Use force-unkeep otherwise
    kept_register = Null_Register;
}

// Do not keep the register and force-free it
void AstNode::forceUnkeepAndFree(registerSpace *rs)
{
    assert(kept_register != Null_Register);
    rs->forceFreeRegister(kept_register);
    kept_register = Null_Register;
}

// Our children may have incorrect useCounts (most likely they 
// assume that we will not bother them again, which is wrong)
void AstNode::fixChildrenCounts(registerSpace *rs)
{
    pdvector<AstNode*> children;
    getChildren(children);
    for (unsigned i=0; i<children.size(); i++) {
	children[i]->setUseCount(rs);
    }
}

// Check to see if the value had been computed earlier
bool AstNode::hasKeptRegister() const
{
    return (kept_register != Null_Register);
}

// Check if the node can be kept at all. Some nodes (e.g., storeOp)
// can not be cached. In fact, there are fewer nodes that can be cached.
bool AstOperatorNode::canBeKept() const {
    return false; // FIXME
    switch (op) {
    case branchOp:
    case ifOp:
    case whileOp:
    case doOp:
    case storeOp:
    case storeIndirOp:
    case trampTrailer:
    case trampPreamble:
    case funcJumpOp:
        return false;
    default:
        return true;
    }
}

bool AstOperandNode::canBeKept() const {
    return false; // FIXME
    if (oType == DataReg)
        return false;
    return true;
}

bool AstCallNode::canBeKept() const {
    return false; // FIXME
    return true;
}

bool AstSequenceNode::canBeKept() const {
    return false;
}

bool AstReplacementNode::canBeKept() const { 
    return false;
}

// Occasionally, we do not call .generateCode_phase2 for the referenced node, 
// but generate code by hand. This routine decrements its use count properly
void AstNode::decUseCount(registerSpace *rs)
{
    useCount--;
    // assert(useCount >= 0);

    if (hasKeptRegister()) { // kept_register still thinks it will be used
	rs->freeRegister(kept_register);
	if (useCount == 0) {
	    unkeepRegister();
	}
    }
}

// Check to see if path1 is a subpath of path2
bool AstNode::subpath(const pdvector<AstNode*> &path1, 
		      const pdvector<AstNode*> &path2) const
{
    if (path1.size() > path2.size()) {
	return false;
    }
    for (unsigned i=0; i<path1.size(); i++) {
	if (path1[i] != path2[i]) {
	    return false;
	}
    }
    return true;
}

// Return all children of this node ([lre]operand, ..., operands[])

void AstNode::getChildren(pdvector<AstNode *> &) {
#if 0
    fprintf(stderr, "Undefined call to getChildren for type: ");
    if (dynamic_cast<AstNullNode *>(this)) fprintf(stderr, "nullNode\n");
    else if (dynamic_cast<AstOperatorNode *>(this)) fprintf(stderr, "operatorNode\n");
    else if (dynamic_cast<AstOperandNode *>(this)) fprintf(stderr, "operandNode\n");
    else if (dynamic_cast<AstCallNode *>(this)) fprintf(stderr, "callNode\n");
    else if (dynamic_cast<AstReplacementNode *>(this)) fprintf(stderr, "replacementNode\n");
    else if (dynamic_cast<AstSequenceNode *>(this)) fprintf(stderr, "seqNode\n");
    else if (dynamic_cast<AstInstructionNode *>(this)) fprintf(stderr, "insnNode\n");
    else if (dynamic_cast<AstMiniTrampNode *>(this)) fprintf(stderr, "miniTrampNode\n");
    else if (dynamic_cast<AstMemoryNode *>(this)) fprintf(stderr, "memoryNode\n");
    else fprintf(stderr, "unknownNode\n");
#endif
}


void AstOperatorNode::getChildren(pdvector<AstNode *> &children) {
    if (loperand) children.push_back(loperand);
    if (roperand) children.push_back(roperand);
    if (eoperand) children.push_back(eoperand);
}

void AstOperandNode::getChildren(pdvector<AstNode *> &children) {
    if (operand_) children.push_back(operand_);
}

void AstCallNode::getChildren(pdvector<AstNode *> &children) {
    for (unsigned i = 0; i < args_.size(); i++)
        children.push_back(args_[i]);
}

void AstSequenceNode::getChildren(pdvector<AstNode *> &children) {
    for (unsigned i = 0; i < sequence_.size(); i++)
        children.push_back(sequence_[i]);
}

void AstMiniTrampNode::getChildren(pdvector<AstNode *> &children) {
    children.push_back(ast_);
}

