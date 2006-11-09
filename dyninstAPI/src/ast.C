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

// $Id: ast.C,v 1.176 2006/11/09 17:16:05 bernat Exp $

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

AstNode *AstNode::labelNode(pdstring &label) {
    return new AstLabelNode(label);
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

AstNode *AstNode::funcCallNode(Address addr, pdvector<AstNode *> &args) {
    return new AstCallNode(addr, args);
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

AstNode *AstNode::insnNode(BPatch_instruction *insn) {
    // Figure out what kind of instruction we've got...
    if (dynamic_cast<BPatch_memoryAccess *>(insn)) {
        return new AstInsnMemoryNode(insn->insn(), (Address) insn->getAddress());
    } 
    
    return new AstInsnNode(insn->insn(), (Address) insn->getAddress());
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

AstOperatorNode::AstOperatorNode(opCode opC, AstNode *l, AstNode *r, AstNode *e) :
    AstNode(),
    op(opC),
    loperand(NULL),
    roperand(NULL),
    eoperand(NULL)
{
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
}


AstCallNode::AstCallNode(int_function *func,
                         pdvector<AstNode *> &args) :
    AstNode(),
    func_addr_(0),
    func_(func)
{
    for (unsigned i = 0; i < args.size(); i++) {
        args_.push_back(assignAst(args[i]));
    }
}

AstCallNode::AstCallNode(const pdstring &func,
                         pdvector<AstNode *> &args) :
    AstNode(),
    func_name_(func),
    func_addr_(0),
    func_(NULL)
{
    for (unsigned i = 0; i < args.size(); i++) {
        args_.push_back(assignAst(args[i]));
    }
}

AstCallNode::AstCallNode(Address addr,
                         pdvector<AstNode *> &args) :
    AstNode(),
    func_addr_(addr),
    func_(NULL)
{
    for (unsigned i = 0; i < args.size(); i++) {
        args_.push_back(assignAst(args[i]));
    }
}
 
AstSequenceNode::AstSequenceNode(pdvector<AstNode *> &sequence) :
    AstNode()
{
    for (unsigned i = 0; i < sequence.size(); i++) {
        sequence_.push_back(assignAst(sequence[i]));
    }
}

AstInsnNode::AstInsnNode(instruction *insn, Address addr) :
    AstNode(),
    insn_(insn),
    origAddr_(addr) {
}

AstMemoryNode::AstMemoryNode(memoryType mem,
                             unsigned which) :
    AstNode(),
    mem_(mem),
    which_(which) {
    
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


Address AstMiniTrampNode::generateTramp(codeGen &gen,
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

    initTramps(gen.proc()->multithread_capable()); 

#if defined( ia64_unknown_linux2_4 )
    extern Register deadRegisterList[];
    defineBaseTrampRegisterSpaceFor( gen.point(), gen.rs(), deadRegisterList);
#endif

	// We assume all registers are unallocated before, and will be after
	gen.rs()->cleanSpace();

    if (!preamble->generateCode(gen, noCost, true)) {
        fprintf(stderr, "[%s:%d] WARNING: failure to generate miniTramp preamble\n", __FILE__, __LINE__);
    }
    removeAst(preamble);
    
    if (!ast_->generateCode(gen, noCost, true)) {
        fprintf(stderr, "[%s:%d] WARNING: failure to generate miniTramp body\n", __FILE__, __LINE__);
    }

    Register tmp = REG_NULL;
    Address trampJumpOffset = 0;
    if (!merged) {
        if (!trailer->generateCode(gen, noCost, true, trampJumpOffset, tmp)) {
            fprintf(stderr, "[%s:%d] WARNING: failure to generate miniTramp trailer\n", __FILE__, __LINE__);
        }
    }

	gen.rs()->cleanSpace();
    
    if  (merged) {
        /* We save the information at the inst-point, since
           the next mini-tramp will reset the clobber information */
        gen.rs()->saveClobberInfo(gen.point());
        gen.rs()->resetClobbers();
    }
    
    return trampJumpOffset;
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
bool AstNode::generateCode(codeGen &gen, 
                           bool noCost, 
                           bool root,
                           Address &retAddr,
                           Register &retReg) {
    // Note: MIPSPro compiler complains about redefinition of default argument
    if (root) {
        cleanUseCount();
        setUseCount(gen.rs());
#if defined(ASTDEBUG)
        print() ;
#endif
    }
    
    // gen.rs()->checkLeaks(Null_Register);
    
    // Create an empty vector to keep track of if statements
    pdvector<AstNode*> ifForks;

    // note: this could return the value "(Address)(-1)" -- csserra
    if (!generateCode_phase2(gen, noCost, ifForks, retAddr, retReg)) return false;

    // gen.rs()->checkLeaks(tmp);
    
#if defined(ASTDEBUG)
    if (root) {
        bool err = false ;
        checkUseCount(rs, err) ;
  }
#endif

    return true;
}

bool AstNode::generateCode(codeGen &gen, 
                           bool noCost, 
                           bool root) {
    Address unused = ADDR_NULL;
    Register unusedReg = REG_NULL;
    bool ret = generateCode(gen, noCost, root, unused, unusedReg);
    gen.rs()->freeRegister(unusedReg);
    return ret;
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

#define ERROR_RETURN { fprintf(stderr, "[%s:%d] ERROR: failure to generate operand\n", __FILE__, __LINE__); return false; }
#define REGISTER_CHECK(r) if (r == REG_NULL) { fprintf(stderr, "[%s: %d] ERROR: returned register invalid\n", __FILE__, __LINE__); return false; }

bool AstNode::generateCode_phase2(codeGen &, bool,
                                  const pdvector<AstNode*> &,
                                  Address &,
                                  Register &) {
    fprintf(stderr, "ERROR: call to AstNode generateCode_phase2; should be handled by subclass\n");
    fprintf(stderr, "Undefined phase2 for:\n");
    if (dynamic_cast<AstNullNode *>(this)) fprintf(stderr, "nullNode\n");
    if (dynamic_cast<AstOperatorNode *>(this)) fprintf(stderr, "operatorNode\n");
    if (dynamic_cast<AstOperandNode *>(this)) fprintf(stderr, "operandNode\n");
    if (dynamic_cast<AstCallNode *>(this)) fprintf(stderr, "callNode\n");
    if (dynamic_cast<AstReplacementNode *>(this)) fprintf(stderr, "replacementNode\n");
    if (dynamic_cast<AstSequenceNode *>(this)) fprintf(stderr, "seqNode\n");
    if (dynamic_cast<AstInsnNode *>(this)) fprintf(stderr, "insnNode\n");
    if (dynamic_cast<AstMiniTrampNode *>(this)) fprintf(stderr, "miniTrampNode\n");
    if (dynamic_cast<AstMemoryNode *>(this)) fprintf(stderr, "memoryNode\n");
    assert(0);
	return 0;
}


bool AstNullNode::generateCode_phase2(codeGen &, bool,
                                      const pdvector<AstNode*> &, 
                                      Address &retAddr,
                                      Register &retReg) {
    retAddr = ADDR_NULL;
    retReg = REG_NULL;

    return true;
}

bool AstLabelNode::generateCode_phase2(codeGen &g, bool,
                                          const pdvector<AstNode*> &,
                                          Address &retAddr,
                                          Register &retReg) {
    // Pick up the address we were added at
    generatedAddr_ = g.currAddr();

    retAddr = ADDR_NULL;
    retReg = REG_NULL;

    return true;
}

bool AstReplacementNode::generateCode_phase2(codeGen &gen, bool noCost,
                                                const pdvector<AstNode*> &,
                                                Address &retAddr,
                                                Register &retReg) {
    retAddr = ADDR_NULL;
    retReg = REG_NULL;

    if (replacement) {
        emitFuncJump(funcJumpOp, gen, replacement, gen.proc(),
                     gen.point(), noCost);
        return true;
    }
        
    return false;
}


bool AstOperatorNode::generateCode_phase2(codeGen &gen, bool noCost,
                                             const pdvector<AstNode*> &ifForks,
                                             Address &retAddr,
                                             Register &retReg) {

    retAddr = ADDR_NULL; // We won't be setting this...
    // retReg may have a value or be the (register) equivalent of NULL.
    // In either case, we don't touch it...

    if (previousComputationValid(retReg, ifForks, gen.rs()))
        return true;
    
    Address addr = ADDR_NULL;
 
    Register src1 = Null_Register;
    Register src2 = Null_Register;

    Register right_dest = Null_Register;
    Register tmp = Null_Register;
    
    switch(op) {
    case branchOp: {
        assert(loperand->getoType() == Constant);
        unsigned offset = (Register)loperand->getOValue();
        // We are not calling loperand->generateCode_phase2,
        // so we decrement its useCount by hand.
        // Would be nice to allow register branches...
        loperand->decUseCount(gen.rs());
        (void)emitA(branchOp, 0, 0, (Register)offset, gen, noCost);
        retReg = REG_NULL; // No return register
        break;
    }
    case ifOp: {
        // This ast cannot be shared because it doesn't return a register
        if (!loperand->generateCode_phase2(gen, noCost, ifForks, addr, src1)) ERROR_RETURN;
        REGISTER_CHECK(src1);
        codeBufIndex_t ifIndex= gen.getIndex();
        codeBufIndex_t thenSkipStart = emitA(op, src1, 0, 0, gen, noCost);
        // DO NOT FREE THE REGISTER HERE!!! we use it later to regenerate the
        // jump once code has been generated. This is annoying, since we
        // don't really need the register... we just want it allocated.
        //gen.rs()->freeRegister(src1);
        
        // The flow of control forks. We need to add the forked node to 
        // the path
        pdvector<AstNode*> thenFork = ifForks;
        thenFork.push_back(roperand);
        if (!roperand->generateCode_phase2(gen, noCost, thenFork, addr, src2)) ERROR_RETURN;
        gen.rs()->freeRegister(src2);
        
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
        (void) emitA(op, src1, 0, 
                     (Register) codeGen::getDisplacement(thenSkipStart, elseStartIndex),
                     gen, noCost);
        // Now we can free the register
        gen.rs()->freeRegister(src1);

        gen.setIndex(elseStartIndex);
        
        if (eoperand) {
            // If there's an else clause, we need to generate code for it.
            pdvector<AstNode*> elseFork = ifForks;
            elseFork.push_back(eoperand);// Add the forked node to the path
            if (!eoperand->generateCode_phase2(gen,
                                               noCost, 
                                               elseFork,
                                               addr,
                                               src2)) ERROR_RETURN;
            gen.rs()->freeRegister(src2);
            
            // We also need to fix up the branch at the end of the "true"
            // clause to jump around the "else" clause.
            codeBufIndex_t endIndex = gen.getIndex();
            gen.setIndex(elseSkipIndex);
            emitA(branchOp, 0, 0, 
                  (Register) codeGen::getDisplacement(elseSkipStart, endIndex),
                  gen, noCost);
            gen.setIndex(endIndex);
        }
        retReg = REG_NULL; 
        break;
    }
    case ifMCOp: {
        assert(gen.point());
        
        // TODO: Right now we get the condition from the memory access info,
        // because scanning for memory accesses is the only way to detect these
        // conditional instructions. The right way(TM) would be to attach that
        // info directly to the point...
        // Okay. The info we need is stored in the BPatch_point. We have the instPoint. 
        // Yay.
        
        // Since someone who shall not be named removed the BPatch
        // link from the process class, we perform a PID-based
        // lookup.
        BPatch_process *bproc = BPatch::bpatch->getProcessByPid(gen.proc()->getPid());
        BPatch_point *bpoint = bproc->instp_map->get(gen.point());
        
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
            if (!loperand->generateCode_phase2(gen,
                                               noCost, thenFork,
                                               addr,
                                               src1)) ERROR_RETURN;
            gen.rs()->freeRegister(src1);
            codeBufIndex_t endIndex = gen.getIndex();
            // call emit again now with correct offset.
            gen.setIndex(startIndex);
            emitJmpMC(cond, codeGen::getDisplacement(fromIndex, endIndex), gen);
            gen.setIndex(endIndex);
        }
        else {
            if (!loperand->generateCode_phase2(gen,
                                               noCost, ifForks,
                                               addr, 
                                               src1)) ERROR_RETURN;
            gen.rs()->freeRegister(src1);
        }

        break;
    }
    case whileOp: {
        codeBufIndex_t top = gen.getIndex();
        if (!loperand->generateCode_phase2(gen, noCost, ifForks, addr, src1)) ERROR_RETURN;
        REGISTER_CHECK(src1);
        codeBufIndex_t startIndex = gen.getIndex();
        codeBufIndex_t fromIndex = emitA(ifOp, src1, 0, 0, gen, noCost);
        gen.rs()->freeRegister(src1);
        if (roperand) {
            pdvector<AstNode*> whileFork = ifForks;
            whileFork.push_back(eoperand);//Add the forked node to the path
            if (!roperand->generateCode_phase2(gen, noCost,
                                               whileFork,
                                               addr,
                                               src1)) ERROR_RETURN;
            gen.rs()->freeRegister(src1);
        }
        //jump back
        (void) emitA(branchOp, 0, 0, codeGen::getDisplacement(top, gen.getIndex()),
                     gen, noCost);
        
        // Rewind and replace the skip jump
        codeBufIndex_t endIndex = gen.getIndex();
        gen.setIndex(startIndex);
        (void) emitA(ifOp, src1, 0, (Register) codeGen::getDisplacement(fromIndex, endIndex), 
                     gen, noCost);
        gen.setIndex(endIndex);
        // sprintf(errorLine,"branch forward %d\n", base - fromAddr);
        break;
    }
    case doOp: {
        fprintf(stderr, "[%s:%d] WARNING: do AST node unimplemented!\n", __FILE__, __LINE__);
        return false;
    }
    case getAddrOp: {
        switch(loperand->getoType()) {
            case DataAddr:
            addr = (Address) loperand->getOValue();
            assert(addr != 0); // check for NULL
            if (retReg == REG_NULL) {
                retReg = allocateAndKeep(gen.rs(), ifForks, gen, noCost);
            }
            emitVload(loadConstOp, addr, retReg, retReg, gen, noCost);
            break;
        case FrameAddr: {
            // load the address fp + addr into dest
            if (retReg == REG_NULL)
                retReg = allocateAndKeep(gen.rs(), ifForks, gen, noCost);
            Register temp = gen.rs()->allocateRegister(gen, noCost);
            addr = (Address) loperand->getOValue();
	    
            emitVload(loadFrameAddr, addr, temp, retReg, gen,
                      noCost, gen.rs(), size, gen.point(), gen.proc());
            gen.rs()->freeRegister(temp);
            break;
        }
        case RegOffset: {
            assert(loperand);
            assert(loperand->operand());
            
            // load the address reg + addr into dest
            if (retReg == REG_NULL) {
                retReg = allocateAndKeep(gen.rs(), ifForks, gen, noCost);
            }
            addr = (Address) loperand->operand()->getOValue();
	    
            emitVload(loadRegRelativeAddr, addr, (long)loperand->getOValue(), retReg, gen,
                      noCost, gen.rs(), size, gen.point(), gen.proc());
            break;
        }
        case DataIndir:
            // taking address of pointer de-ref returns the original
            //    expression, so we simple generate the left child's 
            //    code to get the address 
            if (!loperand->operand()->generateCode_phase2(gen,
                                                          noCost, 
                                                          ifForks,
                                                          addr,
                                                          retReg)) ERROR_RETURN;
            // Broken refCounts?
            break;
        default:
            assert(0);
        }
        break;
    }
    case storeOp: {
        // This ast cannot be shared because it doesn't return a register
        if (!roperand->generateCode_phase2(gen,
                                          noCost, ifForks,
                                          addr,
                                          src1))  { fprintf(stderr, "ERROR: failure generating roperand\n"); ERROR_RETURN; }
        REGISTER_CHECK(src1);
        // loperand's value will be invalidated. Discard the cached reg
        if (loperand->hasKeptRegister()) {
            loperand->forceUnkeepAndFree(gen.rs());
        }
        // We will access loperand's children directly. They do not expect
        // it, so we need to bump up their useCounts
        loperand->fixChildrenCounts(gen.rs());
        
        src2 = gen.rs()->allocateRegister(gen, noCost);
        switch (loperand->getoType()) {
        case DataAddr:
            addr = (Address) loperand->getOValue();
            assert(addr != 0); // check for NULL
            emitVstore(storeOp, src1, src2, addr, gen, noCost, gen.rs(), size);
            // We are not calling generateCode for the left branch,
            // so need to decrement the refcount by hand
            loperand->decUseCount(gen.rs());
            break;
        case FrameAddr:
            addr = (Address) loperand->getOValue();
            emitVstore(storeFrameRelativeOp, src1, src2, addr, gen, 
                       noCost, gen.rs(), size, gen.point(), gen.proc());
            loperand->decUseCount(gen.rs());
            break;
        case RegOffset: {
            assert(loperand->operand());
            addr = (Address) loperand->operand()->getOValue();
            
            // This is cheating, but I need to pass 4 data values into emitVstore, and
            // it only allows for 3.  Prepare the dest address in scratch register src2.
            
            emitVload(loadRegRelativeAddr, addr, (long)loperand->getOValue(), src2,
                      gen, noCost, gen.rs(), size, gen.point(), gen.proc());
            
            // Same as DataIndir at this point.
            emitV(storeIndirOp, src1, 0, src2, gen, noCost, gen.rs(), size, gen.point(), gen.proc());
            loperand->decUseCount(gen.rs());
            break;
        }
        case DataIndir: {
            // store to a an expression (e.g. an array or field use)
            // *(+ base offset) = src1
            if (!loperand->operand()->generateCode_phase2(gen,
                                                          noCost,
                                                          ifForks,
                                                          addr,
                                                          tmp)) ERROR_RETURN;
            REGISTER_CHECK(tmp);

            // tmp now contains address to store into
            emitV(storeIndirOp, src1, 0, tmp, gen, noCost, gen.rs(), size, gen.point(), gen.proc());
            gen.rs()->freeRegister(tmp);
            loperand->decUseCount(gen.rs());
            break;
        }
        default: {
            // Could be an error, could be an attempt to load based on an arithmetic expression
            // Generate the left hand side, store the right to that address
            if (!loperand->generateCode_phase2(gen, noCost, ifForks, addr, tmp)) ERROR_RETURN;
            REGISTER_CHECK(tmp);

            emitV(storeIndirOp, src1, 0, tmp, gen, noCost, gen.rs(), size, gen.point(), gen.proc());
            gen.rs()->freeRegister(tmp);
            break;
        }
        }
        gen.rs()->freeRegister(src1);
        gen.rs()->freeRegister(src2);
        retReg = REG_NULL;
        break;
    }
    case storeIndirOp: {
        
        if (!roperand->generateCode_phase2(gen, noCost, ifForks, addr, src1)) ERROR_RETURN;
        if (!loperand->generateCode_phase2(gen, noCost, ifForks, addr, src2)) ERROR_RETURN;
        REGISTER_CHECK(src1);
        REGISTER_CHECK(src2);
        emitV(op, src1, 0, src2, gen, noCost);          
        gen.rs()->freeRegister(src1);
        gen.rs()->freeRegister(src2);
        retReg = REG_NULL;
        break;
    }
    case trampTrailer: {
        // This ast cannot be shared because it doesn't return a register
        codeBufIndex_t ret_index = emitA(op, 0, 0, 0, gen, noCost);

        // Convert to a bytecount
        // From 0: assume we're starting at the beginning of the code generator. Not a tremendous
        // assumption but valid in all cases.
        retAddr = gen.getDisplacement(0, ret_index);
        retReg = REG_NULL;
        break;
    }
    case trampPreamble: {
        // This ast cannot be shared because it doesn't return a register
#if defined(i386_unknown_solaris2_5) \
 || defined(sparc_sun_solaris2_4)
        // loperand is a constant AST node with the cost, in cycles.
        //int cost = noCost ? 0 : (int) loperand->getOValue();
        Address costAddr = 0; // for now... (won't change if noCost is set)
        loperand->decUseCount(gen.rs());
        costAddr = gen.proc()->getObservedCostAddr();
        retAddr = emitA(op, 0, 0, 0, gen, noCost);
#endif
        retReg = REG_NULL;
        break;
    }
    default: {
        src1 = Null_Register;
        right_dest = Null_Register;
        if (loperand) {
            if (!loperand->generateCode_phase2(gen,
                                               noCost, ifForks, addr, src1)) ERROR_RETURN;
            REGISTER_CHECK(src1);
        }
        
        if (roperand &&
            (roperand->getoType() == Constant) &&
            doNotOverflow((Register)roperand->getOValue())) {
            gen.rs()->freeRegister(src1); // may be able to reuse it for dest
            if (retReg == REG_NULL) {
                retReg = allocateAndKeep(gen.rs(), ifForks, gen, noCost);
            }

            emitImm(op, src1, (Register)roperand->getOValue(), retReg, gen, noCost, gen.rs());
            // We do not .generateCode for roperand, so need to update its
            // refcounts manually
            roperand->decUseCount(gen.rs());
        }
        else {
            if (roperand) {
                if (!roperand->generateCode_phase2(gen, noCost, ifForks, addr, right_dest)) ERROR_RETURN;
                REGISTER_CHECK(right_dest);
            }
	    gen.rs()->freeRegister(src1); // may be able to reuse it for dest
	    gen.rs()->freeRegister(right_dest); // may be able to reuse it for dest
            if (retReg == REG_NULL) {
                retReg = allocateAndKeep(gen.rs(), ifForks, gen, noCost);
            }
            emitV(op, src1, right_dest, retReg, gen, noCost, gen.rs(), size, gen.point(), gen.proc());
        }
    }
    }
    return true;
}

bool AstOperandNode::generateCode_phase2(codeGen &gen, bool noCost,
                                         const pdvector<AstNode*> &ifForks,
                                         Address &,
                                         Register &retReg) {
    if (previousComputationValid(retReg, ifForks, gen.rs()))
        return true;

    

    Address addr = ADDR_NULL;
    Register src = Null_Register;

   useCount--;
#if defined(ASTDEBUG)
   sprintf(errorLine,"### location: %p ###\n", gen.point());
   logLine(errorLine);
#endif
   // Allocate a register to return
   if (oType != DataReg) {
       if (retReg == REG_NULL) {
           retReg = allocateAndKeep(gen.rs(), ifForks, gen, noCost);
       }
   }
   Register temp;
   int tSize;
   int len;
   BPatch_type *Type;
   switch (oType) {
   case Constant:
       emitVload(loadConstOp, (Address)oValue, retReg, retReg, 
		 gen, noCost);
       break;
   case DataIndir:
       if (!operand_->generateCode_phase2(gen, noCost, ifForks, addr, src)) ERROR_RETURN;
       REGISTER_CHECK(src);
       Type = const_cast<BPatch_type *> (getType());
       // Internally generated calls will not have type information set
       if(Type)
           tSize = Type->getSize();
       else
           tSize = sizeof(long);
       emitV(loadIndirOp, src, 0, retReg, gen, noCost, gen.rs(), tSize, gen.point(), gen.proc()); 
       gen.rs()->freeRegister(src);
       break;
   case DataReg:
       retReg = (Register)oValue;
       break;
   case PreviousStackFrameDataReg:
       emitLoadPreviousStackFrameRegister((Address) oValue, retReg, gen,
                                          size, noCost);
       break;
   case ReturnVal:
       src = emitR(getRetValOp, 0, 0, retReg, gen, noCost, gen.point(),
                   gen.proc()->multithread_capable());
       REGISTER_CHECK(src);
       if (src != retReg) {
           // Move src to retReg. Can't simply return src, since it was not
           // allocated properly
           emitImm(orOp, src, 0, retReg, gen, noCost, gen.rs());
       }
       break;
   case Param:
       src = emitR(getParamOp, (Address)oValue, Null_Register,
                   retReg, gen, noCost, gen.point(),
                   gen.proc()->multithread_capable());
       REGISTER_CHECK(src);
       if (src != retReg) {
           // Move src to retReg. Can't simply return src, since it was not
           // allocated properly
           emitImm(orOp, src, 0, retReg, gen, noCost, gen.rs());
       }
       break;
   case DataAddr:
       addr = (Address) oValue;
       emitVload(loadOp, addr, retReg, retReg, gen, noCost, NULL, size);
       break;
   case FrameAddr:
       addr = (Address) oValue;
       temp = gen.rs()->allocateRegister(gen, noCost);
       
       emitVload(loadFrameRelativeOp, addr, temp, retReg, gen, noCost, gen.rs(), size, gen.point(), gen.proc());
       gen.rs()->freeRegister(temp);
       break;
   case RegOffset:
       // Prepare offset from value in any general register (not just fp).
       // This AstNode holds the register number, and loperand holds offset.
       assert(operand_);
       addr = (Address) operand_->getOValue();
       emitVload(loadRegRelativeOp, addr, (long)oValue, retReg, gen, noCost, gen.rs(), size, gen.point(), gen.proc());
       break;
   case ConstantString:
       // XXX This is for the pdstring type.  If/when we fix the pdstring type
       // to make it less of a hack, we'll need to change this.
       len = strlen((char *)oValue) + 1;
       
#if defined(rs6000_ibm_aix4_1) //ccw 30 jul 2002
       if(gen.proc()->requestTextMiniTramp){
           bool mallocFlag;
           addr = (Address) gen.proc()->inferiorMalloc(len, (inferiorHeapType) (textHeap | uncopiedHeap), 0x10000000, &mallocFlag); //textheap
       }else{	
           addr = (Address) gen.proc()->inferiorMalloc(len, dataHeap); //dataheap
       }
#else
       addr = (Address) gen.proc()->inferiorMalloc(len, dataHeap); //dataheap
#endif
       
       if (!gen.proc()->writeDataSpace((char *)addr, len, (char *)oValue))
           perror("ast.C(1351): writing string value");
       
       emitVload(loadConstOp, addr, retReg, retReg, gen, noCost);
       break;
	case RegValue: {
		gen.rs()->readRegister(gen, (Register) oValue, retReg);
		break;
	}
   default:
       fprintf(stderr, "[%s:%d] ERROR: Unknown operand type %d in AstOperandNode generation\n",
               __FILE__, __LINE__, oType);
       return false;
       break;
   }

   return true;
}

bool AstMemoryNode::generateCode_phase2(codeGen &gen, bool noCost,
                                        const pdvector<AstNode*> &ifForks,
                                        Address &,
                                        Register &retReg) {
    if (previousComputationValid(retReg, ifForks, gen.rs()))
        return true;

    const BPatch_memoryAccess* ma;
    const BPatch_addrSpec_NP *start;
    const BPatch_countSpec_NP *count;
    if (retReg == REG_NULL)
        retReg = allocateAndKeep(gen.rs(), ifForks, gen, noCost);    
    switch(mem_) {
    case EffectiveAddr: {
        
        // VG(11/05/01): get effective address
        // VG(07/31/02): take care which one
        // 1. get the point being instrumented & memory access info
        assert(gen.point());
        
        BPatch_process *bproc = BPatch::bpatch->getProcessByPid(gen.proc()->getPid());
        BPatch_point *bpoint = bproc->instp_map->get(gen.point());
        if (bpoint == NULL) {
            fprintf(stderr, "ERROR: Unable to find BPatch point for internal point %p/0x%llx\n",
                    gen.point(), gen.point()->addr());
        }
        assert(bpoint);
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
        emitASload(start, retReg, gen, noCost);
        break;
    }
    case BytesAccessed: {
        // 1. get the point being instrumented & memory access info
        assert(gen.point());
        
        BPatch_process *bproc = BPatch::bpatch->getProcessByPid(gen.proc()->getPid());
        BPatch_point *bpoint = bproc->instp_map->get(gen.point());
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
        emitCSload(count, retReg, gen, noCost);
        break;
    }
    default:
        assert(0);
    }
    return true;
}

bool AstCallNode::generateCode_phase2(codeGen &gen, bool noCost,
                                      const pdvector<AstNode*> &ifForks,
                                      Address &, 
                                      Register &retReg) {

    // VG(11/06/01): This platform independent fn calls a platfrom
    // dependent fn which calls it back for each operand... Have to
    // fix those as well to pass location...

    int_function *use_func = func_;

    if (!use_func) {
        // We purposefully don't cache the int_function object; the AST nodes
        // are process independent, and functions kinda are.
        use_func = gen.proc()->findOnlyOneFunction(func_name_);
    }

    Register tmp = 0;

    if (use_func) {
        tmp = emitFuncCall(callOp, gen.rs(), gen, args_,  
                           gen.proc(), noCost, use_func, ifForks, 
                           gen.point());
    }
    else if (func_addr_) {
        tmp = emitFuncCall(callOp, gen.rs(), gen, args_,  
                           gen.proc(), noCost, func_addr_, ifForks, 
                           gen.point());
    }
    else {
        char msg[256];
        sprintf(msg, "%s[%d]:  internal error:  unable to find %s",
                __FILE__, __LINE__, func_name_.c_str());
        showErrorCallback(100, msg);
        assert(0);  // can probably be more graceful
    }
    
    if (useCount > 0) {
        gen.rs()->fixRefCount(tmp, useCount+1);
        keepRegister(tmp, ifForks);
    }
    if (retReg == REG_NULL) 
        retReg = tmp;
    else if (retReg != tmp) {
        if (retReg > 40) assert(0);
        emitImm(orOp, tmp, 0, retReg, gen, noCost, gen.rs());
        // Free tmp?
    }

    return true;
}

bool AstSequenceNode::generateCode_phase2(codeGen &gen, bool noCost,
                                          const pdvector<AstNode*> &ifForks,
                                          Address &,
                                          Register &retReg) {
    Register tmp = REG_NULL;
    Address unused = ADDR_NULL;
    
    for (unsigned i = 0; i < sequence_.size() - 1; i++) {
        if (!sequence_[i]->generateCode_phase2(gen,
                                               noCost, 
                                               ifForks, 
                                               unused,
                                               tmp)) ERROR_RETURN;
        gen.rs()->freeRegister(tmp);
    }

    // We keep the last one
    if (!sequence_.back()->generateCode_phase2(gen, noCost, ifForks, unused, retReg)) ERROR_RETURN;
    
    return true;
}

bool AstInsnNode::generateCode_phase2(codeGen &gen, bool,
                                      const pdvector<AstNode*> &,
                                      Address &, Register &) {
    assert(insn_);
    
    insn_->generate(gen, gen.proc(), origAddr_, gen.currAddr());
    
    return true;
}

bool AstInsnBranchNode::generateCode_phase2(codeGen &gen, bool noCost,
                                            const pdvector<AstNode*> &ifForks,
                                            Address &, Register & ) {
    assert(insn_);
    
    // Generate side 2 and get the result...
    Address targetAddr = ADDR_NULL;
    Register targetReg = REG_NULL;
    if (target_) {
        // TODO: address vs. register...
        if (!target_->generateCode_phase2(gen, noCost, ifForks, targetAddr, targetReg)) ERROR_RETURN;
    }
    // We'd now generate a fixed or register branch. But we don't. So there.
    
    insn_->generate(gen, gen.proc(), origAddr_, gen.currAddr(), 0, 0);

    return false;
}

bool AstInsnMemoryNode::generateCode_phase2(codeGen &gen, bool noCost,
                                               const pdvector<AstNode *> &ifForks,
                                            Address &, Register &) {
    Register loadReg = REG_NULL;
    Register storeReg = REG_NULL;
    Address loadAddr = ADDR_NULL;
    Address storeAddr = ADDR_NULL;

    // Step 1: save machine-specific state (AKA flags)

    gen.rs()->saveVolatileRegisters(gen);

    // Step 2: generate code (this may spill registers)

    if (load_)
        if (!load_->generateCode_phase2(gen, noCost, ifForks, loadAddr, loadReg)) ERROR_RETURN;
    
    if (store_)
        if (!store_->generateCode_phase2(gen, noCost, ifForks, storeAddr, storeReg)) ERROR_RETURN;

    // Step 3: restore flags (before the original instruction)

    gen.rs()->restoreVolatileRegisters(gen);

    // Step 4: generate the memory instruction
    assert(insn_);
    if (!insn_->generateMem(gen, origAddr_, gen.currAddr(), loadReg, storeReg)) {
        fprintf(stderr, "ERROR: generateMem call failed\n");
        return false;
    }

    // Step 5: restore any registers that were st0mped. 

    gen.rs()->restoreAllRegisters(gen, true);
    
    return true;
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
    else if (dynamic_cast<AstInsnNode *>(this)) fprintf(stderr, "insnNode\n");
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
    else if (dynamic_cast<AstInsnNode *>(this)) fprintf(stderr, "insnNode\n");
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

bool AstOperatorNode::containsFuncCall() const {
	if (loperand && loperand->containsFuncCall()) return true;
	if (roperand && loperand->containsFuncCall()) return true;
	if (eoperand && loperand->containsFuncCall()) return true;
	return false;
}

bool AstOperandNode::containsFuncCall() const {
	if (operand_ && operand_->containsFuncCall()) return true;
	return false;
}

bool AstMiniTrampNode::containsFuncCall() const {
	if (ast_ && ast_->containsFuncCall()) return true;
	return false;
}

bool AstSequenceNode::containsFuncCall() const {
	for (unsigned i = 0; i < sequence_.size(); i++) {
		if (sequence_[i]->containsFuncCall()) return true;
	}
	return false;
}

bool AstInsnMemoryNode::containsFuncCall() const {
    if (load_ && load_->containsFuncCall()) return true;
    if (store_ && store_->containsFuncCall()) return true;
    return false;
}
