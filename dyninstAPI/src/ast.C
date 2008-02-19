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

// $Id: ast.C,v 1.201 2008/02/19 13:37:19 rchen Exp $

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/debug.h"

#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/src/BPatch_collections.h"
#include "dyninstAPI/h/BPatch_type.h"
#include "dyninstAPI/src/BPatch_libInfo.h" // For instPoint->BPatch_point mapping

#include "dyninstAPI/h/BPatch_point.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "dyninstAPI/h/BPatch_type.h"

#if defined(arch_sparc)
#include "dyninstAPI/src/inst-sparc.h"
#elif defined(arch_power)
#include "dyninstAPI/src/inst-power.h"
#elif defined(arch_x86) || defined (arch_x86_64)
#include "dyninstAPI/src/inst-x86.h"
#elif defined(arch_ia64)
#include "dyninstAPI/src/inst-ia64.h"
#else
#error "Unknown architecture in ast.h"
#endif

#include "emitter.h"

#include "registerSpace.h"

using namespace Dyninst;

extern bool doNotOverflow(int value);

AstNodePtr AstNode::originalAddrNode_ = AstNodePtr();
AstNodePtr AstNode::actualAddrNode_ = AstNodePtr();


AstNode::AstNode() {
#if defined(ASTDEBUG)
   ASTcounter();
#endif
   // used in mdl.C
   referenceCount = 1;
   useCount = 0;
   // "operands" is left as an empty vector
   size = 4;
   bptype = NULL;
   doTypeCheck = true;
}

AstNodePtr AstNode::nullNode() {
    return AstNodePtr(new AstNullNode());
}

AstNodePtr AstNode::labelNode(pdstring &label) {
    return AstNodePtr (new AstLabelNode(label));
}

AstNodePtr AstNode::operandNode(operandType ot, void *arg) {
    return AstNodePtr(new AstOperandNode(ot, arg));
}

// TODO: this is an indirect load; should be an operator.
AstNodePtr AstNode::operandNode(operandType ot, AstNodePtr ast) {
    return AstNodePtr(new AstOperandNode(ot, ast));
}

AstNodePtr AstNode::sequenceNode(pdvector<AstNodePtr > &sequence) {
    return AstNodePtr(new AstSequenceNode(sequence));
}

AstNodePtr AstNode::operatorNode(opCode ot, AstNodePtr l, AstNodePtr r, AstNodePtr e) {
    return AstNodePtr(new AstOperatorNode(ot, l, r, e));
}

AstNodePtr AstNode::funcCallNode(const pdstring &func, pdvector<AstNodePtr > &args,
                                 AddressSpace *addrSpace) {
    if (addrSpace) {
        int_function *ifunc = addrSpace->findOnlyOneFunction(func.c_str());
        if (ifunc == NULL) {
            fprintf(stderr, "Bitch whine moan\n");
            return AstNodePtr();
        }
        return AstNodePtr(new AstCallNode(ifunc, args));
    }
    else
        return AstNodePtr(new AstCallNode(func, args));
}

AstNodePtr AstNode::funcCallNode(int_function *func, pdvector<AstNodePtr > &args) {
    if (func == NULL) return AstNodePtr();
    return AstNodePtr(new AstCallNode(func, args));
}

AstNodePtr AstNode::funcCallNode(Address addr, pdvector<AstNodePtr > &args) {
    return AstNodePtr(new AstCallNode(addr, args));
}

AstNodePtr AstNode::funcReplacementNode(int_function *func) {
    if (func == NULL) return AstNodePtr();
    return AstNodePtr(new AstReplacementNode(func));
}

AstNodePtr AstNode::memoryNode(memoryType ma, int which) {
    return AstNodePtr(new AstMemoryNode(ma, which));
}

AstNodePtr AstNode::miniTrampNode(AstNodePtr tramp) {
    if (tramp == NULL) return AstNodePtr();
    return AstNodePtr(new AstMiniTrampNode(tramp));
}

AstNodePtr AstNode::insnNode(BPatch_instruction *insn) {
    // Figure out what kind of instruction we've got...
    if (dynamic_cast<BPatch_memoryAccess *>(insn)) {
        return AstNodePtr(new AstInsnMemoryNode(insn->insn(), (Address) insn->getAddress()));
    } 
    
    return AstNodePtr(new AstInsnNode(insn->insn(), (Address) insn->getAddress()));
}

AstNodePtr AstNode::originalAddrNode() {
    if (originalAddrNode_ == NULL) {
        originalAddrNode_ = AstNodePtr(new AstOriginalAddrNode());
    }
    return originalAddrNode_;
}

AstNodePtr AstNode::actualAddrNode() {
    if (actualAddrNode_ == NULL) {
        actualAddrNode_ = AstNodePtr(new AstActualAddrNode());
    }
    return actualAddrNode_;
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

BPatch_type *AstNode::getType() { return bptype; }

void AstNode::setType(BPatch_type *t) { 
    bptype = t;
    if (t != NULL) { 
        size = t->getSize(); 
    }
}

AstOperatorNode::AstOperatorNode(opCode opC, AstNodePtr l, AstNodePtr r, AstNodePtr e) :
    AstNode(),
    op(opC),
    loperand(l),
    roperand(r),
    eoperand(e)
{
    // Optimization pass...
    
    if (op == plusOp) {
        if (loperand->getoType() == Constant) {
            // Swap left and right...
            AstNodePtr temp = loperand;
            loperand = roperand;
            roperand = temp;
        }
    }
    if (op == timesOp) {
        if (roperand->getoType() == undefOperandType) {
            // ...
        }
        else if (roperand->getoType() != Constant) {
            AstNodePtr temp = roperand;
            roperand = loperand;
            loperand = temp;
        }
        else {
            int result;
            if (!isPowerOf2((Address)roperand->getOValue(),result) &&
                isPowerOf2((Address)loperand->getOValue(),result)) {
                AstNodePtr temp = roperand;
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
    operand_() {
    
    if (ot == ConstantString)
        oValue = (void *)P_strdup((char *)arg);
    else
        oValue = (void *) arg;
}

// And an indirect (say, a load)
AstOperandNode::AstOperandNode(operandType ot, AstNodePtr l) :
    AstNode(),
    oType(ot),
    oValue(NULL),
    operand_(l)
{
}


AstCallNode::AstCallNode(int_function *func,
                         pdvector<AstNodePtr > &args) :
    AstNode(),
    func_addr_(0),
    func_(func),
    constFunc_(false)
{
    for (unsigned i = 0; i < args.size(); i++) {
        args_.push_back(args[i]);
    }
}

AstCallNode::AstCallNode(const pdstring &func,
                         pdvector<AstNodePtr > &args) :
    AstNode(),
    func_name_(func),
    func_addr_(0),
    func_(NULL),
    constFunc_(false)
{
    for (unsigned i = 0; i < args.size(); i++) {
        args_.push_back(args[i]);
    }
}

AstCallNode::AstCallNode(Address addr,
                         pdvector<AstNodePtr > &args) :
    AstNode(),
    func_addr_(addr),
    func_(NULL),
    constFunc_(false)
{
    for (unsigned i = 0; i < args.size(); i++) {
        args_.push_back(args[i]);
    }
}
 
AstSequenceNode::AstSequenceNode(pdvector<AstNodePtr > &sequence) :
    AstNode()
{
    for (unsigned i = 0; i < sequence.size(); i++) {
        sequence_.push_back(sequence[i]);
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

AstNodePtr AstNode::threadIndexNode() {
    // We use one of these across all platforms, since it
    // devolves into a process-specific function node. 
    // However, this lets us delay that until code generation
    // when we have the process pointer.
    static AstNodePtr indexNode_;

    // Since we only ever have one, keep a static copy around. If
    // we get multiples, we'll screw up our pointer-based common subexpression
    // elimination.

    if (indexNode_ != AstNodePtr()) return indexNode_;
    pdvector<AstNodePtr > args;
    // By not including a process we'll specialize at code generation.
    indexNode_ = AstNode::funcCallNode("DYNINSTthreadIndex", args);
    assert(indexNode_);
    indexNode_->setConstFunc(true);

    return indexNode_;
}


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
}

Address AstMiniTrampNode::generateTramp(codeGen &gen,
                                        int &trampCost, 
                                        bool noCost,
                                        bool merged) {
    static AstNodePtr trailer;
    static AstNodePtr costAst;
    static AstNodePtr preamble;

    if (trailer == AstNodePtr())
        trailer = AstNode::operatorNode(trampTrailer);

    if (costAst == AstNodePtr())
        costAst = AstNode::operandNode(AstNode::Constant, (void *)0);

    if (preamble == AstNodePtr())
        preamble = AstNode::operatorNode(trampPreamble, costAst);

    // private constructor; assumes NULL for right child
    
    // we only want to use the cost of the minimum statements that will
    // be executed.  Statements, such as the body of an if statement,
    // will have their costs added to the observed cost global variable
    // only if they are indeed called.  The code to do this in the minitramp
    // right after the body of the if.
    trampCost = preamble->maxCost() + minCost() + trailer->maxCost();

    costAst->setOValue((void *) (long) trampCost);
    

//#if defined( ia64_unknown_linux2_4 )
//    extern Register deadRegisterList[];
//    defineBaseTrampRegisterSpaceFor( gen.point(), gen.rs(), deadRegisterList);
//#endif

    if (!preamble->generateCode(gen, noCost)) {
        fprintf(stderr, "[%s:%d] WARNING: failure to generate miniTramp preamble\n", __FILE__, __LINE__);
    }
    
    if (!ast_->generateCode(gen, noCost)) {
        fprintf(stderr, "[%s:%d] WARNING: failure to generate miniTramp body\n", __FILE__, __LINE__);
    }

    Register tmp = REG_NULL;
    Address trampJumpOffset = 0;
    if (!merged) {
        if (!trailer->generateCode(gen, noCost, trampJumpOffset, tmp)) {
            fprintf(stderr, "[%s:%d] WARNING: failure to generate miniTramp trailer\n", __FILE__, __LINE__);
        }
        gen.rs()->freeRegister(tmp);
    }

    
    return trampJumpOffset;
}

// This name is a bit of a misnomer. It's not the strict use count; it's the
// use count modified by whether a node can be kept or not. We can treat
// un-keepable nodes (AKA those that don't strictly depend on their AST inputs)
// as multiple different nodes that happen to have the same children; keepable
// nodes are the "same". If that makes any sense. 
//
// In any case, we use the following algorithm to set use counts:
//
//DFS through the AST graph. 
//If an AST can be kept:
//  Increase its use count;
//  Return.
//If an AST cannot be kept:
//  Recurse to each child;
//  Return
//
// The result is all nodes having counts of 0, 1, or >1. These mean:
// 0: node cannot be kept, or is only reached via a keepable node.
// 1: Node can be kept, but doesn't matter as it's only used once.
// >1: keep result in a register.

void AstNode::setUseCount() 
{
    // This code fails on IA-64. Until I can ask Todd about it, 
    // it's just getting commented out...
    //#if defined(arch_ia64)
    //return;
    //#endif
	if (useCount) {
		// If the useCount is 1, then it means this node can
		// be shared, and there is a copy. In that case, we assume
		// that when this particular incarnation is generated, the
		// result will already be calculated and sitting in a register.
		// Since that's the case, just up the useCount so we know when
		// we can free said register.
		useCount++;
		return;	
	}
	if (canBeKept()) {
		useCount++;
		// We purposefully fall through... if our use count
		// is 1, we'll have to calculate this node instead of
		// keeping it around. In that case, see if any of the 
		// children are shared (because we can reuse them when
		// calculating this guy)
	}
	// We can't be kept, but maybe our children can.
	pdvector<AstNodePtr> children;
	getChildren(children);
	for (unsigned i=0; i<children.size(); i++) {
	    children[i]->setUseCount();
    }
}

void AstNode::cleanUseCount(void)
{
    useCount = 0;

    pdvector<AstNodePtr> children;
    getChildren(children);
    for (unsigned i=0; i<children.size(); i++) {
		children[i]->cleanUseCount();
    }
}

// Allocate a register and make it available for sharing if our
// node is shared
Register AstNode::allocateAndKeep(codeGen &gen, bool noCost)
{
    ast_printf("Allocating register for node %p, useCount %d\n", this, useCount);
    // Allocate a register
    Register dest = gen.rs()->allocateRegister(gen, noCost);
    
    if (useCount > 1) {
        ast_printf("Adding kept register %d for node %p: useCount %d\n", dest, this, useCount);
        // If use count is 0 or 1, we don't want to keep
        // it around. If it's > 1, then we can keep the node
        // (by construction) and want to since there's another
        // use later.
        gen.tracker()->addKeptRegister(gen, this, dest);
    }
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
                           Address &retAddr,
                           Register &retReg) {
    static bool entered = false;


    bool ret = true;

    bool top_level;
    if (entered) {
        top_level = false;
    }
    else {
        entered = true;
        top_level = true;
        stats_codegen.startTimer(CODEGEN_AST_TIMER);
        stats_codegen.incrementCounter(CODEGEN_AST_COUNTER);
    }

    entered = true;

    cleanUseCount();
    setUseCount();
    ast_printf("====== Code Generation Start ===== \n");
    debugPrint();
    ast_printf("\n\n\n\n");

    // We can enter this guy recursively... inst-ia64 goes through
    // emitV and calls generateCode on the frame pointer AST. Now, it
    // really shouldn't, but them's the breaks. So we only want
    // to build a regTracker if there isn't one already...
    if (top_level) {
        gen.setRegTracker(new regTracker_t);
    }
    
    // note: this could return the value "(Address)(-1)" -- csserra
    if (!generateCode_phase2(gen, noCost, retAddr, retReg)) {
        fprintf(stderr, "WARNING: failed in generateCode internals!\n");
        ret = false;
    }
    
    if (top_level) {
        delete gen.tracker();
        gen.setRegTracker(NULL);
    }
    
    ast_printf("====== Code Generation End ===== \n");
    debugPrint();
    ast_printf("\n\n\n\n");

    if (top_level) {
        entered = false;
        stats_codegen.stopTimer(CODEGEN_AST_TIMER);
    }

    return ret;
}

bool AstNode::generateCode(codeGen &gen, 
                           bool noCost) {
    Address unused = ADDR_NULL;
    Register unusedReg = REG_NULL;
    bool ret = generateCode(gen, noCost, unused, unusedReg);
    gen.rs()->freeRegister(unusedReg);

    return ret;
}

bool AstNode::previousComputationValid(Register &reg,
                                       codeGen &gen) {
	Register keptReg = gen.tracker()->hasKeptRegister(this);
	if (keptReg != REG_NULL) {
		reg = keptReg;
		ast_printf("Returning previously used register %d for node %p\n", reg, this);
		return true;
	}
    return false;
}

// We're going to use this fragment over and over and over...
#define RETURN_KEPT_REG(r) { if (previousComputationValid(r, gen)) { decUseCount(gen); gen.rs()->incRefCount(r); return true;} }
#define ERROR_RETURN { fprintf(stderr, "[%s:%d] ERROR: failure to generate operand\n", __FILE__, __LINE__); return false; }
#define REGISTER_CHECK(r) if (r == REG_NULL) { fprintf(stderr, "[%s: %d] ERROR: returned register invalid\n", __FILE__, __LINE__); return false; }


bool AstNode::initRegisters(codeGen &g) {
    bool ret = true;
    pdvector<AstNodePtr> kids;
    getChildren(kids);
    for (unsigned i = 0; i < kids.size(); i++) {
        if (!kids[i]->initRegisters(g))
            ret = false;
    }
    return ret;
}

bool AstNode::generateCode_phase2(codeGen &, bool,
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


bool AstNullNode::generateCode_phase2(codeGen &gen, bool,
                                      Address &retAddr,
                                      Register &retReg) {
    retAddr = ADDR_NULL;
    retReg = REG_NULL;

	decUseCount(gen);

    return true;
}

bool AstLabelNode::generateCode_phase2(codeGen &gen, bool,
                                          Address &retAddr,
                                          Register &retReg) {
	assert(generatedAddr_ == 0);
    // Pick up the address we were added at
    generatedAddr_ = gen.currAddr();

    retAddr = ADDR_NULL;
    retReg = REG_NULL;

	decUseCount(gen);

    return true;
}

bool AstReplacementNode::generateCode_phase2(codeGen &gen, bool noCost,
                                                Address &retAddr,
                                                Register &retReg) {
    retAddr = ADDR_NULL;
    retReg = REG_NULL;

	assert(replacement);

    emitFuncJump(funcJumpOp, gen, replacement, gen.addrSpace(),
                 gen.point(), noCost);

	decUseCount(gen);
	return true;
}

bool AstOperatorNode::generateOptimizedAssignment(codeGen &gen, bool noCost) 
{
#if defined(arch_x86) || defined(arch_x86_64)
   //Recognize the common case of 'a = a op constant' and try to 
   // generate optimized code for this case.

   if (loperand->getoType() != DataAddr) {
      //Deal with global writes for now.
      return false;
   }
   Address laddr = (Address) loperand->getOValue();

   if (roperand->getoType() == Constant) {
      //Looks like 'global = constant'
#if defined(arch_x86_64)
      if (laddr >> 32 || ((Address) roperand->getOValue()) >> 32) {
         // Make sure value and address are 32-bit values.
         return false;
      }
#endif
      int imm = (int) (long) roperand->getOValue();
      emitStoreConst(laddr, (int) imm, gen, noCost);
      loperand->decUseCount(gen);
      roperand->decUseCount(gen);
      return true;
   }

   AstOperatorNode *roper = dynamic_cast<AstOperatorNode *>(roperand.get());
   if (!roper)
      return false;
   
   if (roper->op != plusOp && roper->op != minusOp)
      return false;
   
   AstOperandNode *arithl = dynamic_cast<AstOperandNode *>(roper->loperand.get());
   AstOperandNode *arithr = dynamic_cast<AstOperandNode *>(roper->roperand.get());
   if (!arithl && !arithr)
      return false;
   
   AstOperandNode *data_oper = NULL, *const_oper = NULL;
   if (arithl->getoType() == DataAddr && arithr->getoType() == Constant &&
       laddr == (Address) arithl->getOValue())
   {
      data_oper = arithl;
      const_oper = arithr;
   }
   else if (arithr->getoType() == DataAddr && arithl->getoType() == Constant &&
            laddr == (Address) arithr->getOValue() && roper->op == plusOp)
   {
      data_oper = arithr;
      const_oper = arithl;
   }
   else
   {
      return false;
   }

   long int imm = (long int) const_oper->getOValue();
   if (roper->op == plusOp) {
      emitAddSignedImm(laddr, imm, gen, noCost);
   }
   else {
      emitSubSignedImm(laddr, imm, gen, noCost);
   }
   
   loperand->decUseCount(gen);
   roper->roperand->decUseCount(gen);
   roper->loperand->decUseCount(gen);
   roper->decUseCount(gen);

   return true;
#endif
   return false;   
}

bool AstOperatorNode::generateCode_phase2(codeGen &gen, bool noCost,
                                          Address &retAddr,
                                          Register &retReg) {

    retAddr = ADDR_NULL; // We won't be setting this...
    // retReg may have a value or be the (register) equivalent of NULL.
    // In either case, we don't touch it...

	RETURN_KEPT_REG(retReg);

    
    Address addr = ADDR_NULL;
 
    Register src1 = Null_Register;
    Register src2 = Null_Register;

    Register right_dest = Null_Register;
    Register tmp = Null_Register;
    
    switch(op) {
    case branchOp: {
        assert(loperand->getoType() == Constant);
        unsigned offset = (Register) (long) loperand->getOValue();
        // We are not calling loperand->generateCode_phase2,
        // so we decrement its useCount by hand.
        // Would be nice to allow register branches...
        loperand->decUseCount(gen);
        (void)emitA(branchOp, 0, 0, (Register)offset, gen, noCost);
        retReg = REG_NULL; // No return register
        break;
    }
    case ifOp: {
        // This ast cannot be shared because it doesn't return a register
        if (!loperand->generateCode_phase2(gen, noCost, addr, src1)) ERROR_RETURN;
        
        REGISTER_CHECK(src1);
        codeBufIndex_t ifIndex= gen.getIndex();
        codeBufIndex_t thenSkipStart = emitA(op, src1, 0, 0, gen, noCost);
        // DO NOT FREE THE REGISTER HERE!!! we use it later to regenerate the
        // jump once code has been generated. This is annoying, since we
        // don't really need the register... we just want it allocated.
        
        // I'm ignoring the above; we'll keep the _value_ of src1, but free it
        // for internal code.
        Register src1_copy = src1;
        gen.rs()->freeRegister(src1);
        
        // The flow of control forks. We need to add the forked node to 
        // the path
        gen.tracker()->increaseConditionalLevel();
        if (!roperand->generateCode_phase2(gen, noCost, addr, src2)) ERROR_RETURN;
        gen.rs()->freeRegister(src2);
        gen.tracker()->decreaseAndClean(gen);
        
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
        (void) emitA(op, src1_copy, 0, 
                     (Register) codeGen::getDisplacement(thenSkipStart, elseStartIndex),
                     gen, noCost);
        // Now we can free the register
        // Register has already been freed; we're just re-using it.
        //gen.rs()->freeRegister(src1);
        
        gen.setIndex(elseStartIndex);
        
        if (eoperand) {
            // If there's an else clause, we need to generate code for it.
			gen.tracker()->increaseConditionalLevel();
            if (!eoperand->generateCode_phase2(gen,
                                               noCost, 
                                               addr,
                                               src2)) ERROR_RETURN;
            gen.rs()->freeRegister(src2);
			gen.tracker()->decreaseAndClean(gen);
            
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
        BPatch_process *bproc = (BPatch_process *) gen.addrSpace()->up_ptr();
        BPatch_point *bpoint = bproc->instp_map->get(gen.point());
        
        const BPatch_memoryAccess* ma = bpoint->getMemoryAccess();
        assert(ma);
        int cond = ma->conditionCode_NP();
        if(cond > -1) {
            codeBufIndex_t startIndex = gen.getIndex();
            emitJmpMC(cond, 0 /* target, changed later */, gen);
            codeBufIndex_t fromIndex = gen.getIndex();
            // Add the snippet to the tracker, as AM has indicated...
 			gen.tracker()->increaseConditionalLevel();
            // generate code with the right path
            if (!loperand->generateCode_phase2(gen,
                                               noCost,
                                               addr,
                                               src1)) ERROR_RETURN;
            gen.rs()->freeRegister(src1);
			gen.tracker()->decreaseAndClean(gen);
            codeBufIndex_t endIndex = gen.getIndex();
            // call emit again now with correct offset.
            gen.setIndex(startIndex);
            emitJmpMC(cond, codeGen::getDisplacement(fromIndex, endIndex), gen);
            gen.setIndex(endIndex);
        }
        else {
            if (!loperand->generateCode_phase2(gen,
                                               noCost,
                                               addr, 
                                               src1)) ERROR_RETURN;
            gen.rs()->freeRegister(src1);
        }

        break;
    }
    case whileOp: {
        codeBufIndex_t top = gen.getIndex();
        if (!loperand->generateCode_phase2(gen, noCost, addr, src1)) ERROR_RETURN;
        REGISTER_CHECK(src1);
        codeBufIndex_t startIndex = gen.getIndex();
        codeBufIndex_t fromIndex = emitA(ifOp, src1, 0, 0, gen, noCost);
        gen.rs()->freeRegister(src1);
        if (roperand) {
			gen.tracker()->increaseConditionalLevel();
            if (!roperand->generateCode_phase2(gen, noCost,
                                               addr,
                                               src1)) ERROR_RETURN;
            gen.rs()->freeRegister(src1);
			gen.tracker()->decreaseAndClean(gen);
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
                retReg = allocateAndKeep(gen, noCost);
            }
            emitVload(loadConstOp, addr, retReg, retReg, gen,
		      noCost, gen.rs(), size, gen.point(), gen.addrSpace());
            break;
        case FrameAddr: {
            // load the address fp + addr into dest
            if (retReg == REG_NULL)
                retReg = allocateAndKeep(gen, noCost);
            Register temp = gen.rs()->getScratchRegister(gen, noCost);
            addr = (Address) loperand->getOValue();
	    
            emitVload(loadFrameAddr, addr, temp, retReg, gen,
                      noCost, gen.rs(), size, gen.point(), gen.addrSpace());
            break;
        }
        case RegOffset: {
            assert(loperand);
            assert(loperand->operand());
            
            // load the address reg + addr into dest
            if (retReg == REG_NULL) {
                retReg = allocateAndKeep(gen, noCost);
            }
            addr = (Address) loperand->operand()->getOValue();
	    
            emitVload(loadRegRelativeAddr, addr, (long)loperand->getOValue(), retReg, gen,
                      noCost, gen.rs(), size, gen.point(), gen.addrSpace());
            break;
        }
        case DataIndir:
            // taking address of pointer de-ref returns the original
            //    expression, so we simple generate the left child's 
            //    code to get the address 
            if (!loperand->operand()->generateCode_phase2(gen,
                                                          noCost, 
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
        bool result = generateOptimizedAssignment(gen, noCost);
        if (result)
           break;
       
        // This ast cannot be shared because it doesn't return a register
        if (!roperand->generateCode_phase2(gen,
                                          noCost,
                                          addr,
                                          src1))  { 
			fprintf(stderr, "ERROR: failure generating roperand\n"); 
			ERROR_RETURN; 
		}
        REGISTER_CHECK(src1);
        // We will access loperand's children directly. They do not expect
        // it, so we need to bump up their useCounts
        loperand->fixChildrenCounts();
        
        src2 = gen.rs()->allocateRegister(gen, noCost);
        switch (loperand->getoType()) {
        case DataAddr:
            addr = (Address) loperand->getOValue();
            assert(addr != 0); // check for NULL
            emitVstore(storeOp, src1, src2, addr, gen,
		       noCost, gen.rs(), size, gen.point(), gen.addrSpace());
            // We are not calling generateCode for the left branch,
            // so need to decrement the refcount by hand
            loperand->decUseCount(gen);
            break;
        case FrameAddr:
            addr = (Address) loperand->getOValue();
            emitVstore(storeFrameRelativeOp, src1, src2, addr, gen, 
                       noCost, gen.rs(), size, gen.point(), gen.addrSpace());
            loperand->decUseCount(gen);
            break;
        case RegOffset: {
            assert(loperand->operand());
            addr = (Address) loperand->operand()->getOValue();
            
            // This is cheating, but I need to pass 4 data values into emitVstore, and
            // it only allows for 3.  Prepare the dest address in scratch register src2.
            
            emitVload(loadRegRelativeAddr, addr, (long)loperand->getOValue(), src2,
                      gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());
            
            // Same as DataIndir at this point.
            emitV(storeIndirOp, src1, 0, src2, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());
            loperand->decUseCount(gen);
            break;
        }
        case DataIndir: {
            // store to a an expression (e.g. an array or field use)
            // *(+ base offset) = src1
            if (!loperand->operand()->generateCode_phase2(gen,
                                                          noCost,
                                                          addr,
                                                          tmp)) ERROR_RETURN;
            REGISTER_CHECK(tmp);

            // tmp now contains address to store into
            emitV(storeIndirOp, src1, 0, tmp, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());
            gen.rs()->freeRegister(tmp);
            loperand->decUseCount(gen);
            break;
        }
        default: {
            // Could be an error, could be an attempt to load based on an arithmetic expression
            // Generate the left hand side, store the right to that address
            if (!loperand->generateCode_phase2(gen, noCost, addr, tmp)) ERROR_RETURN;
            REGISTER_CHECK(tmp);

            emitV(storeIndirOp, src1, 0, tmp, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());
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
        
        if (!roperand->generateCode_phase2(gen, noCost, addr, src1)) ERROR_RETURN;
        if (!loperand->generateCode_phase2(gen, noCost, addr, src2)) ERROR_RETURN;
        REGISTER_CHECK(src1);
        REGISTER_CHECK(src2);
        emitV(op, src1, 0, src2, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());
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
        loperand->decUseCount(gen);
        costAddr = gen.addrSpace()->getObservedCostAddr();
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
                                               noCost, addr, src1)) ERROR_RETURN;
            REGISTER_CHECK(src1);
        }
        
        if (roperand &&
            (roperand->getoType() == Constant) &&
            doNotOverflow((Register) (long) roperand->getOValue())) {
            gen.rs()->freeRegister(src1); // may be able to reuse it for dest
            if (retReg == REG_NULL) {
                retReg = allocateAndKeep(gen, noCost);
				ast_printf("Operator node, const RHS, allocated register %d\n", retReg);
            }
			else
				ast_printf("Operator node, const RHS, keeping register %d\n", retReg);
				
            emitImm(op, src1, (Register) (long) roperand->getOValue(), retReg, gen, noCost, gen.rs());
            // We do not .generateCode for roperand, so need to update its
            // refcounts manually
            roperand->decUseCount(gen);
        }
        else {
            if (roperand) {
                if (!roperand->generateCode_phase2(gen, noCost, addr, right_dest)) ERROR_RETURN;
                REGISTER_CHECK(right_dest);
            }
	    gen.rs()->freeRegister(src1); // may be able to reuse it for dest
	    gen.rs()->freeRegister(right_dest); // may be able to reuse it for dest
            if (retReg == REG_NULL) {
                retReg = allocateAndKeep(gen, noCost);
            }
            emitV(op, src1, right_dest, retReg, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());
        }
    }
    }
	decUseCount(gen);
    return true;
}

bool AstOperandNode::generateCode_phase2(codeGen &gen, bool noCost,
                                         Address &,
                                         Register &retReg) {
	RETURN_KEPT_REG(retReg);
    

    Address addr = ADDR_NULL;
    Register src = Null_Register;

#if defined(ASTDEBUG)
   sprintf(errorLine,"### location: %p ###\n", gen.point());
   logLine(errorLine);
#endif
   // Allocate a register to return
   if (oType != DataReg) {
       if (retReg == REG_NULL) {
           retReg = allocateAndKeep(gen, noCost);
       }
   }
   Register temp;
   int tSize;
   int len;
   BPatch_type *Type;
   switch (oType) {
   case Constant:
       emitVload(loadConstOp, (Address)oValue, retReg, retReg, gen,
		 noCost, gen.rs(), size, gen.point(), gen.addrSpace());
       break;
   case DataIndir:
       if (!operand_->generateCode_phase2(gen, noCost, addr, src)) ERROR_RETURN;
       REGISTER_CHECK(src);
       Type = const_cast<BPatch_type *> (getType());
       // Internally generated calls will not have type information set
       if(Type)
           tSize = Type->getSize();
       else
           tSize = sizeof(long);
       emitV(loadIndirOp, src, 0, retReg, gen, noCost, gen.rs(), tSize, gen.point(), gen.addrSpace()); 
       gen.rs()->freeRegister(src);
       break;
   case DataReg:
       retReg = (Register) (long) oValue;
       break;
   case PreviousStackFrameDataReg:
       emitLoadPreviousStackFrameRegister((Address) oValue, retReg, gen,
                                          size, noCost);
       break;
   case ReturnVal:
       src = emitR(getRetValOp, 0, 0, retReg, gen, noCost, gen.point(),
                   gen.addrSpace()->multithread_capable());
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
                   gen.addrSpace()->multithread_capable());
       REGISTER_CHECK(src);
       if (src != retReg) {
           // Move src to retReg. Can't simply return src, since it was not
           // allocated properly
           emitImm(orOp, src, 0, retReg, gen, noCost, gen.rs());
       }
       break;
   case DataAddr:
       addr = (Address) oValue;
       emitVload(loadOp, addr, retReg, retReg, gen, noCost, NULL, size, gen.point(), gen.addrSpace());
       break;
   case FrameAddr:
       addr = (Address) oValue;
       temp = gen.rs()->allocateRegister(gen, noCost);
       
       emitVload(loadFrameRelativeOp, addr, temp, retReg, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());
       gen.rs()->freeRegister(temp);
       break;
   case RegOffset:
       // Prepare offset from value in any general register (not just fp).
       // This AstNode holds the register number, and loperand holds offset.
       assert(operand_);
       addr = (Address) operand_->getOValue();
       emitVload(loadRegRelativeOp, addr, (long)oValue, retReg, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());
       break;
   case ConstantString:
       // XXX This is for the pdstring type.  If/when we fix the pdstring type
       // to make it less of a hack, we'll need to change this.
       len = strlen((char *)oValue) + 1;
       
#if defined(rs6000_ibm_aix4_1) //ccw 30 jul 2002
       addr = (Address) gen.addrSpace()->inferiorMalloc(len, dataHeap); //dataheap
#else
       addr = (Address) gen.addrSpace()->inferiorMalloc(len, dataHeap); //dataheap
#endif
       
       if (!gen.addrSpace()->writeDataSpace((char *)addr, len, (char *)oValue))
           perror("ast.C(1351): writing string value");
       
       emitVload(loadConstOp, addr, retReg, retReg, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());
       break;
	case RegValue: {
		gen.rs()->readRegister(gen, (Register) (long) oValue, retReg);
		break;
	}
   default:
       fprintf(stderr, "[%s:%d] ERROR: Unknown operand type %d in AstOperandNode generation\n",
               __FILE__, __LINE__, oType);
       return false;
       break;
   }
	decUseCount(gen);
   return true;
}

bool AstMemoryNode::generateCode_phase2(codeGen &gen, bool noCost,
                                        Address &,
                                        Register &retReg) {
	RETURN_KEPT_REG(retReg);
	
    const BPatch_memoryAccess* ma;
    const BPatch_addrSpec_NP *start;
    const BPatch_countSpec_NP *count;
    if (retReg == REG_NULL)
        retReg = allocateAndKeep(gen, noCost);    
    switch(mem_) {
    case EffectiveAddr: {
        
        // VG(11/05/01): get effective address
        // VG(07/31/02): take care which one
        // 1. get the point being instrumented & memory access info
        assert(gen.point());
        
        BPatch_process *bproc = (BPatch_process *)gen.addrSpace()->up_ptr();

        BPatch_point *bpoint = bproc->instp_map->get(gen.point());
        if (bpoint == NULL) {
            fprintf(stderr, "ERROR: Unable to find BPatch point for internal point %p/0x%lx\n",
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
        
        BPatch_process *bproc = (BPatch_process *)gen.addrSpace()->up_ptr();
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
	decUseCount(gen);
    return true;
}

bool AstCallNode::initRegisters(codeGen &gen) {
    // For now, we only care if we should save everything. "Everything", of course, 
    // is platform dependent. This is the new location of the clobberAllFuncCalls
    // that had previously been in emitCall.

    bool ret = true;

    // First, check kids
    pdvector<AstNodePtr > kids;
    getChildren(kids);
    for (unsigned i = 0; i < kids.size(); i++) {
        if (!kids[i]->initRegisters(gen))
            ret = false;
    }

    // Platform-specific...
#if defined(arch_x86) || defined(arch_x86_64)
    // Our "everything" is "floating point registers".
    // We also need a function object.
    int_function *callee = func_;
    if (!callee) {
        // Painful lookup time
        callee = gen.addrSpace()->findOnlyOneFunction(func_name_.c_str());
    }
    assert(callee);

    // Marks registers as used based on the callee's behavior
    // This means we'll save them if necessary (also, lets us use
    // them in our generated code because we've saved, instead
    // of saving others).
    gen.codeEmitter()->clobberAllFuncCall(gen.rs(), callee);


#endif
#if defined(arch_power)
    // This code really doesn't work right now...
    int_function *callee = func_;
    if (!callee) {
        // Painful lookup time
        callee = gen.addrSpace()->findOnlyOneFunction(func_name_.c_str());
        assert(callee);
    }
    bool clobbered = gen.codeEmitter()->clobberAllFuncCall(gen.rs(), callee);
    // We clobber in clobberAllFuncCall...

    // Monotonically increasing...
#endif
    return ret;
    
}

bool AstCallNode::generateCode_phase2(codeGen &gen, bool noCost,
                                      Address &, 
                                      Register &retReg) {
	// We call this anyway... not that we'll ever be kept.
	// Well... if we can somehow know a function is entirely
	// dependent on arguments (a flag?) we can keep it around.
	RETURN_KEPT_REG(retReg);

    // VG(11/06/01): This platform independent fn calls a platfrom
    // dependent fn which calls it back for each operand... Have to
    // fix those as well to pass location...

    int_function *use_func = func_;

    if (!use_func && !func_addr_) {
        // We purposefully don't cache the int_function object; the AST nodes
        // are process independent, and functions kinda are.
        use_func = gen.addrSpace()->findOnlyOneFunction(func_name_.c_str());
        if (!use_func) {
            fprintf(stderr, "ERROR: failed to find function %s, unable to create call\n",
                    func_name_.c_str());
        }
        assert(use_func); // Otherwise we've got trouble...
    }

    Register tmp = 0;

    if (use_func) {
        tmp = emitFuncCall(callOp, gen, args_,  
                           noCost, use_func);
    }
    else if (func_addr_) {
        tmp = emitFuncCall(callOp, gen, args_,  
                           noCost, func_addr_);
    }
    else {
        char msg[256];
        sprintf(msg, "%s[%d]:  internal error:  unable to find %s",
                __FILE__, __LINE__, func_name_.c_str());
        showErrorCallback(100, msg);
        assert(0);  // can probably be more graceful
    }
    
	// TODO: put register allocation here and have emitCall just
	// move the return result.
    
    if (retReg == REG_NULL) {
        //emitFuncCall allocated tmp; we can use it, but let's see
        // if we should keep it around.
        retReg = tmp;
        // from allocateAndKeep:
        if (useCount > 1) {
            // If use count is 0 or 1, we don't want to keep
            // it around. If it's > 1, then we can keep the node
            // (by construction) and want to since there's another
            // use later.
            gen.tracker()->addKeptRegister(gen, this, retReg);
        }
    }		
    else if (retReg != tmp) {
        emitImm(orOp, tmp, 0, retReg, gen, noCost, gen.rs());
        gen.rs()->freeRegister(tmp);
    }
    decUseCount(gen);
    return true;
}

bool AstSequenceNode::generateCode_phase2(codeGen &gen, bool noCost,
                                          Address &,
                                          Register &retReg) {
    RETURN_KEPT_REG(retReg);
    Register tmp = REG_NULL;
    Address unused = ADDR_NULL;
    
    for (unsigned i = 0; i < sequence_.size() - 1; i++) {
        if (!sequence_[i]->generateCode_phase2(gen,
                                               noCost, 
                                               unused,
                                               tmp)) ERROR_RETURN;
        gen.rs()->freeRegister(tmp);
        tmp = REG_NULL;
    }

    // We keep the last one
    if (!sequence_.back()->generateCode_phase2(gen, noCost, unused, retReg)) ERROR_RETURN;

	decUseCount(gen);
    
    return true;
}

bool AstInsnNode::generateCode_phase2(codeGen &gen, bool,
                                      Address &, Register &) {
    assert(insn_);
    
    insn_->generate(gen, gen.addrSpace(), origAddr_, gen.currAddr());
    decUseCount(gen);
    
    return true;
}

bool AstInsnBranchNode::generateCode_phase2(codeGen &gen, bool noCost,
                                            Address &, Register & ) {
    assert(insn_);
    
    // Generate side 2 and get the result...
    Address targetAddr = ADDR_NULL;
    Register targetReg = REG_NULL;
    if (target_) {
        // TODO: address vs. register...
        if (!target_->generateCode_phase2(gen, noCost, targetAddr, targetReg)) ERROR_RETURN;
    }
    // We'd now generate a fixed or register branch. But we don't. So there.
    assert(0 && "Unimplemented");
    insn_->generate(gen, gen.addrSpace(), origAddr_, gen.currAddr(), 0, 0);
	decUseCount(gen);

    return true;
}

bool AstInsnMemoryNode::generateCode_phase2(codeGen &gen, bool noCost,
                                            Address &, Register &) {
    Register loadReg = REG_NULL;
    Register storeReg = REG_NULL;
    Address loadAddr = ADDR_NULL;
    Address storeAddr = ADDR_NULL;
    assert(insn_);

    // Step 1: save machine-specific state (AKA flags) and mark registers used in
    // the instruction itself as off-limits.

    gen.rs()->saveVolatileRegisters(gen);
    pdvector<int> usedRegisters;
    if (insn_->getUsedRegs(usedRegisters)) {
        for (unsigned i = 0; i < usedRegisters.size(); i++) {
            gen.rs()->markReadOnly(usedRegisters[i]);
        }
    }
    else {
        // We don't know who to avoid... return false?
        fprintf(stderr, "WARNING: unknown \"off limits\" register set, returning false from memory modification\n");
        return false;
    }

    // Step 2: generate code (this may spill registers)

    if (load_)
        if (!load_->generateCode_phase2(gen, noCost, loadAddr, loadReg)) ERROR_RETURN;
    
    if (store_)
        if (!store_->generateCode_phase2(gen, noCost, storeAddr, storeReg)) ERROR_RETURN;

    // Step 3: restore flags (before the original instruction)

    gen.rs()->restoreVolatileRegisters(gen);

    // Step 4: generate the memory instruction
    if (!insn_->generateMem(gen, origAddr_, gen.currAddr(), loadReg, storeReg)) {
        fprintf(stderr, "ERROR: generateMem call failed\n");
        return false;
    }

    // Step 5: restore any registers that were st0mped. 


    gen.rs()->restoreAllRegisters(gen, true);

    decUseCount(gen);
    return true;
}


bool AstOriginalAddrNode::generateCode_phase2(codeGen &gen,
                                              bool noCost,
                                              Address &,
                                              Register &retReg) {
    if (retReg == REG_NULL) {
        retReg = allocateAndKeep(gen, noCost);
    }
    if (retReg == REG_NULL) return false;

    emitVload(loadConstOp, 
              (Address) gen.point()->addr(),
              retReg, retReg, gen, noCost);

    return true;
}

bool AstActualAddrNode::generateCode_phase2(codeGen &gen,
                                            bool noCost,
                                            Address &,
                                            Register &retReg) {
    if (retReg == REG_NULL) {
        retReg = allocateAndKeep(gen, noCost);
    }
    if (retReg == REG_NULL) return false;

    emitVload(loadConstOp, 
              (Address) gen.currAddr(),
              retReg, retReg, 
              gen, noCost);

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
    if (func_) total += getPrimitiveCost(func_->prettyName().c_str());
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

// Our children may have incorrect useCounts (most likely they 
// assume that we will not bother them again, which is wrong)
void AstNode::fixChildrenCounts()
{
    pdvector<AstNodePtr> children;
    getChildren(children);
    for (unsigned i=0; i<children.size(); i++) {
		children[i]->setUseCount();
    }
}


// Check if the node can be kept at all. Some nodes (e.g., storeOp)
// can not be cached. In fact, there are fewer nodes that can be cached.
bool AstOperatorNode::canBeKept() const {
    switch (op) {
    case plusOp:
    case minusOp:
    case timesOp:
    case divOp:
    case neOp:
    case noOp:
    case orOp:
    case andOp:
		break;
    default:
        return false;
    }

    // The switch statement is a little odd, but hey. 
    if (loperand && !loperand->canBeKept()) return false;
    if (roperand && !roperand->canBeKept()) return false;
    if (eoperand && !eoperand->canBeKept()) return false;
    
    return true;
}

bool AstOperandNode::canBeKept() const {
    
    switch (oType) {
    case DataReg:
    case DataIndir:
    case RegOffset:
    case RegValue:
    case DataAddr:
        return false;
    default:
		break;
    }
    if (operand_ && !operand_->canBeKept()) return false;
    return true;
}

bool AstCallNode::canBeKept() const {
    if (constFunc_) {
        for (unsigned i = 0; i < args_.size(); i++) {
            if (!args_[i]->canBeKept()) {
                fprintf(stderr, "AST %p: labelled const func but argument %d cannot be kept!\n",
                        this, i);
                return false;
            }
        }
        return true;
    }
    return false;
    
}

bool AstSequenceNode::canBeKept() const {
	// Theoretically we could keep the entire thing, but... not sure
	// that's a terrific idea. For now, don't keep a sequence node around.
    return false;
}

bool AstMiniTrampNode::canBeKept() const {
	// Well... depends on the actual AST, doesn't it.
	assert(ast_);
	
	return ast_->canBeKept();
}

bool AstReplacementNode::canBeKept() const { 
    return false;
}

bool AstMemoryNode::canBeKept() const {
	// Despite our memory loads, we can be kept;
	// we're loading off process state, which is defined
	// to be invariant during the instrumentation phase.
	return true;
}

// Occasionally, we do not call .generateCode_phase2 for the referenced node, 
// but generate code by hand. This routine decrements its use count properly
void AstNode::decUseCount(codeGen &gen)
{
    if (useCount == 0) return;
    
    useCount--;
    
    if (useCount == 0) {
        gen.tracker()->removeKeptRegister(gen, this);
    }
}

// Return all children of this node ([lre]operand, ..., operands[])

void AstNode::getChildren(pdvector<AstNodePtr > &) {
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


void AstOperatorNode::getChildren(pdvector<AstNodePtr > &children) {
    if (loperand) children.push_back(loperand);
    if (roperand) children.push_back(roperand);
    if (eoperand) children.push_back(eoperand);
}

void AstOperandNode::getChildren(pdvector<AstNodePtr > &children) {
    if (operand_) children.push_back(operand_);
}

void AstCallNode::getChildren(pdvector<AstNodePtr > &children) {
    for (unsigned i = 0; i < args_.size(); i++)
        children.push_back(args_[i]);
}

void AstSequenceNode::getChildren(pdvector<AstNodePtr > &children) {
    for (unsigned i = 0; i < sequence_.size(); i++)
        children.push_back(sequence_[i]);
}

void AstMiniTrampNode::getChildren(pdvector<AstNodePtr > &children) {
    children.push_back(ast_);
}

bool AstNode::containsFuncCall() const { 
   return false; 
}

bool AstCallNode::containsFuncCall() const {
   return true;
}

bool AstReplacementNode::containsFuncCall() const {
   return true;
}

bool AstOperatorNode::containsFuncCall() const {
	if (loperand && loperand->containsFuncCall()) return true;
	if (roperand && roperand->containsFuncCall()) return true;
	if (eoperand && eoperand->containsFuncCall()) return true;
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

void regTracker_t::addKeptRegister(codeGen &gen, AstNode *n, Register reg) {
	assert(n);
	if (tracker.find(n)) {
		assert(tracker[n].keptRegister == reg);
		return;
	}
	commonExpressionTracker t;
	t.keptRegister = reg;
	t.keptLevel = condLevel;
	tracker[n] = t;
	gen.rs()->markKeptRegister(reg);
}

void regTracker_t::removeKeptRegister(codeGen &gen, AstNode *n) {
	if (!tracker.find(n)) {
		return;
	}
	gen.rs()->unKeepRegister(tracker[n].keptRegister);
	tracker.undef(n);
}

Register regTracker_t::hasKeptRegister(AstNode *n) {
	if (tracker.find(n))
		return tracker[n].keptRegister;
	return REG_NULL;	
}

// Find if the given register is "owned" by an AST node,
// and if so nuke it.

bool regTracker_t::stealKeptRegister(Register r) {
	AstNode *a;
	commonExpressionTracker c;
	ast_printf("STEALING kept register %d for someone else\n", r);
	dictionary_hash_iter<AstNode *, commonExpressionTracker> reg_iter(tracker);
        while (reg_iter.next(a, c)) {
            if (c.keptRegister == r) {
                tracker.undef(a);
                return true;
            }
	}
	fprintf(stderr, "Odd - couldn't find kept register %d\n", r);
	return true;
}

void regTracker_t::reset() {
	condLevel = 0;
	tracker.clear();
}

void regTracker_t::increaseConditionalLevel() {
	condLevel++;
	ast_printf("Entering conditional branch, level now %d\n", condLevel);
}

void regTracker_t::decreaseAndClean(codeGen &gen) {
    AstNode *a;
    commonExpressionTracker c;
    assert(condLevel > 0);
    
    ast_printf("Exiting from conditional branch, level currently %d\n", condLevel);
    
    dictionary_hash_iter<AstNode *, commonExpressionTracker> reg_iter(tracker);
    while (reg_iter.next(a, c)) {
        if (c.keptLevel == condLevel) {
            tracker.undef(a);
            gen.rs()->unKeepRegister(c.keptRegister);
            ast_printf("Removing kept register %d, level %d, for AST %p\n", 
                       c.keptRegister, c.keptLevel, a);
        }
    }
    
    condLevel--;
}

unsigned regTracker_t::astHash(AstNode* const &ast) {
	return addrHash4((Address) ast);
}

void AstNode::debugPrint(unsigned level) {
    if (!dyn_debug_ast) return;
    
    for (unsigned i = 0; i < level; i++) fprintf(stderr, "%s", " ");
    
    pdstring type;
    if (dynamic_cast<AstNullNode *>(this)) type = "nullNode";
    else if (dynamic_cast<AstOperatorNode *>(this)) type = "operatorNode";
    else if (dynamic_cast<AstOperandNode *>(this)) type = "operandNode";
    else if (dynamic_cast<AstCallNode *>(this)) type = "callNode";
    else if (dynamic_cast<AstReplacementNode *>(this)) type = "replacementNode";
    else if (dynamic_cast<AstSequenceNode *>(this)) type = "sequenceNode";
    else if (dynamic_cast<AstInsnNode *>(this)) type = "insnNode";
    else if (dynamic_cast<AstMiniTrampNode *>(this)) type = "miniTrampNode";
    else if (dynamic_cast<AstMemoryNode *>(this)) type = "memoryNode";
    
    
    ast_printf("Node %s: ptr %p, useCount is %d, canBeKept %d\n", type.c_str(), this, useCount, canBeKept());
    
    pdvector<AstNodePtr> children;
    getChildren(children);
    for (unsigned i=0; i<children.size(); i++) {
        children[i]->debugPrint(level+1);
    }
}

void regTracker_t::debugPrint() {
    if (!dyn_debug_ast) return;
    
    fprintf(stderr, "==== Begin debug dump of register tracker ====\n");
    
    fprintf(stderr, "Condition level: %d\n", condLevel);
    
    AstNode *a;
    commonExpressionTracker c;
    
    dictionary_hash_iter<AstNode *, commonExpressionTracker> reg_iter(tracker);
    while (reg_iter.next(a, c)) {
        fprintf(stderr, "AstNode %p: register %d, condition level %d\n",
                a, c.keptRegister, c.keptLevel);
    }	
    fprintf(stderr, "==== End debug dump of register tracker ====\n");
}

unsigned AstNode::getTreeSize() {
	pdvector<AstNodePtr > children;
	getChildren(children);

	unsigned size = 1; // Us
	for (unsigned i = 0; i < children.size(); i++)
		size += children[i]->getTreeSize();
	return size;
	
}
