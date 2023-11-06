/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: ast.C,v 1.209 2008/09/15 18:37:49 jaw Exp $

#include "dyninstAPI/src/image.h"
#include "function.h"
#include "inst.h"
#include "instP.h"
#include "instPoint.h"
#include "ast.h"
#include "util.h"
#include "debug.h"

extern int dyn_debug_ast;

#include "Instruction.h"
using namespace Dyninst::InstructionAPI;

#include "dyninstAPI/h/BPatch.h"
#include "BPatch_collections.h"
#include "dyninstAPI/h/BPatch_type.h"
#include "BPatch_libInfo.h" // For instPoint->BPatch_point mapping
#include "BPatch_function.h"
#include "dyninstAPI/h/BPatch_point.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "dyninstAPI/h/BPatch_type.h"
#include "dyninstAPI/src/RegisterConversion.h"

#include "addressSpace.h"
#include "binaryEdit.h"

#if defined(arch_power)
#include "inst-power.h"
#elif defined(arch_x86) || defined (arch_x86_64)
#include "inst-x86.h"
#include "emit-x86.h"
#elif defined(arch_aarch64)
#include "inst-aarch64.h"
#else
#error "Unknown architecture in ast.h"
#endif

#include "emitter.h"

#include "registerSpace.h"
#include "mapped_module.h"

#include "legacy-instruction.h"
#include "mapped_object.h"
#include "Buffer.h"

using namespace Dyninst;
using PatchAPI::Point;

extern bool doNotOverflow(int64_t value);

static bool IsSignedOperation(BPatch_type *l, BPatch_type *r) {
    if (l == NULL || r == NULL) return true;
    if (strstr(l->getName(), "unsigned") == NULL) return true;
    if (strstr(r->getName(), "unsigned") == NULL) return true;
    return false;
}

AstNodePtr AstNode::originalAddrNode_ = AstNodePtr();
AstNodePtr AstNode::actualAddrNode_ = AstNodePtr();
AstNodePtr AstNode::dynamicTargetNode_ = AstNodePtr();

AstNode::AstNode() {
//   dyn_debug_ast = 0;
   referenceCount = 0;
   useCount = 0;
   // "operands" is left as an empty vector
   size = 4;
   bptype = NULL;
   doTypeCheck = true;
   lineNum = 0;
   columnNum = 0;
   snippetName = NULL;
   lineInfoSet = false;
   columnInfoSet = false;
   snippetNameSet = false;
}

//The following methods are for error reporting in dynC_API

// Returns the line number at which the ast was declared
int AstNode::getLineNum(){
   return lineNum;
}

// Sets the line number at which the ast was declared
void AstNode::setLineNum(int ln){
   lineInfoSet = true;
   lineNum = ln;
}

// Returns the column number at which the ast was declared
int AstNode::getColumnNum(){
   return columnNum;
}

// Sets the column number at which the ast was declared
void AstNode::setColumnNum(int cn){
   columnInfoSet = true;
   columnNum = cn;
}

char * AstNode::getSnippetName(){
   return snippetName;
}

void AstNode::setSnippetName(char *n){
   if(n != NULL){
//      printf(":::%s\n", n);
      snippetName = n;
      snippetNameSet = true;
   }
}

bool AstNode::hasLineInfo(){
   return lineInfoSet;
}


bool AstNode::hasColumnInfo(){
   return columnInfoSet;
}

bool AstNode::hasNameInfo(){
   return snippetNameSet;
}

//////////////////////////////////////////////////////

AstNodePtr AstNode::nullNode() {
    return AstNodePtr(new AstNullNode());
}

AstNodePtr AstNode::stackInsertNode(int size, MSpecialType type) {
    return AstNodePtr(new AstStackInsertNode(size, type));
}

AstNodePtr AstNode::stackRemoveNode(int size, MSpecialType type) {
    return AstNodePtr(new AstStackRemoveNode(size, type));
}

AstNodePtr AstNode::stackRemoveNode(int size, MSpecialType type, func_instance* func, bool canaryAfterPrologue, long canaryHeight) {
    return AstNodePtr(new AstStackRemoveNode(size, type, func, canaryAfterPrologue, canaryHeight));
}

AstNodePtr AstNode::stackGenericNode() {
    return AstNodePtr(new AstStackGenericNode());
}

AstNodePtr AstNode::labelNode(std::string &label) {
    return AstNodePtr (new AstLabelNode(label));
}

AstNodePtr AstNode::operandNode(operandType ot, void *arg) {
    return AstNodePtr(new AstOperandNode(ot, arg));
}

// TODO: this is an indirect load; should be an operator.
AstNodePtr AstNode::operandNode(operandType ot, AstNodePtr ast) {
    return AstNodePtr(new AstOperandNode(ot, ast));
}

AstNodePtr AstNode::operandNode(operandType ot, const image_variable* iv) {
    return AstNodePtr(new AstOperandNode(ot, iv));
}

AstNodePtr AstNode::sequenceNode(std::vector<AstNodePtr > &sequence) {
//    assert(sequence.size());
    return AstNodePtr(new AstSequenceNode(sequence));
}

AstNodePtr AstNode::variableNode(vector<AstNodePtr> &ast_wrappers,
                                 vector<pair<Dyninst::Offset, Dyninst::Offset> >*ranges) {
    return AstNodePtr(new AstVariableNode(ast_wrappers, ranges));
}

AstNodePtr AstNode::operatorNode(opCode ot, AstNodePtr l, AstNodePtr r, AstNodePtr e) {
    return AstNodePtr(new AstOperatorNode(ot, l, r, e));
}

AstNodePtr AstNode::funcCallNode(const std::string &func, std::vector<AstNodePtr > &args,
      AddressSpace *addrSpace)
{
   if (addrSpace)
   {
      func_instance *ifunc = addrSpace->findOnlyOneFunction(func.c_str());

      if (ifunc == NULL)
      {
         fprintf(stderr, "%s[%d]: Can't find function %s\n", FILE__, __LINE__, func.c_str());
         return AstNodePtr();
      }

      return AstNodePtr(new AstCallNode(ifunc, args));
   }
   else
      return AstNodePtr(new AstCallNode(func, args));
}

AstNodePtr AstNode::funcCallNode(func_instance *func, std::vector<AstNodePtr > &args) {
    if (func == NULL) return AstNodePtr();
    return AstNodePtr(new AstCallNode(func, args));
}

AstNodePtr AstNode::funcCallNode(func_instance *func) {
    if (func == NULL) return AstNodePtr();
    return AstNodePtr(new AstCallNode(func));
}

AstNodePtr AstNode::funcCallNode(Address addr, std::vector<AstNodePtr > &args) {
    return AstNodePtr(new AstCallNode(addr, args));
}

AstNodePtr AstNode::memoryNode(memoryType ma, int which, int size) {
    return AstNodePtr(new AstMemoryNode(ma, which, size));
}

AstNodePtr AstNode::miniTrampNode(AstNodePtr tramp) {
    if (tramp == NULL) return AstNodePtr();
    return AstNodePtr(new AstMiniTrampNode(tramp));
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

AstNodePtr AstNode::dynamicTargetNode() {
    if (dynamicTargetNode_ == NULL) {
        dynamicTargetNode_ = AstNodePtr(new AstDynamicTargetNode());
    }
    return dynamicTargetNode_;
}

AstNodePtr AstNode::snippetNode(Dyninst::PatchAPI::SnippetPtr snip) {
   return AstNodePtr(new AstSnippetNode(snip));
}

AstNodePtr AstNode::scrambleRegistersNode(){
    return AstNodePtr(new AstScrambleRegistersNode());
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
    if(!loperand) return;
    if(roperand) {
      if (op == plusOp) {
          if (loperand->getoType() == operandType::Constant) {
              // Swap left and right...
              AstNodePtr temp = loperand;
              loperand = roperand;
              roperand = temp;
          }
      }
      if (op == timesOp) {
          if (roperand->getoType() == operandType::undefOperandType) {
              // ...
         }
          else if (roperand->getoType() != operandType::Constant) {
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
    }
    if (l != AstNodePtr())
    {
       //Unfortunate hack.  storeOp with a loperand of DataIndir
       // don't actually reference their loperand--they reference
       // the child of the loperand.  Handle that here to keep
       // reference counts sane.
       if (op == storeOp && loperand->getoType() == operandType::DataIndir)
          l->operand()->referenceCount++;
       else
          l->referenceCount++;
    }
    if (r != AstNodePtr())
       r->referenceCount++;
    if (e != AstNodePtr())
       e->referenceCount++;
}

    // Direct operand
AstOperandNode::AstOperandNode(operandType ot, void *arg) :
    AstNode(),
    oType(ot),
    oVar(NULL),
    operand_()
{

    if (ot == operandType::ConstantString)
        oValue = (void *)P_strdup((char *)arg);
    else
        oValue = (void *) arg;
}

// And an indirect (say, a load)
AstOperandNode::AstOperandNode(operandType ot, AstNodePtr l) :
    AstNode(),
    oType(ot),
    oValue(NULL),
    oVar(NULL),
    operand_(l)
{
   l->referenceCount++;
}

AstOperandNode::AstOperandNode(operandType ot, const image_variable* iv) :
  AstNode(),
  oType(ot),
  oValue(NULL),
  oVar(iv),
  operand_()
{
  assert(oVar);
}


AstCallNode::AstCallNode(func_instance *func,
                         std::vector<AstNodePtr > &args) :
    AstNode(),
    func_addr_(0),
    func_(func),
    callReplace_(false),
    constFunc_(false)
{
    for (unsigned i = 0; i < args.size(); i++) {
        args[i]->referenceCount++;
        args_.push_back(args[i]);
    }
}

AstCallNode::AstCallNode(func_instance *func) :
    AstNode(),
    func_addr_(0),
    func_(func),
    callReplace_(true),
    constFunc_(false)
{
}

AstCallNode::AstCallNode(const std::string &func,
                         std::vector<AstNodePtr > &args) :
    AstNode(),
    func_name_(func),
    func_addr_(0),
    func_(NULL),
    callReplace_(false),
    constFunc_(false)
{
    for (unsigned i = 0; i < args.size(); i++) {
        args[i]->referenceCount++;
        args_.push_back(args[i]);
    }
}

AstCallNode::AstCallNode(Address addr,
                         std::vector<AstNodePtr > &args) :
    AstNode(),
    func_addr_(addr),
    func_(NULL),
    callReplace_(false),
    constFunc_(false)
{
    for (unsigned i = 0; i < args.size(); i++) {
        args[i]->referenceCount++;
        args_.push_back(args[i]);
    }
}

AstSequenceNode::AstSequenceNode(std::vector<AstNodePtr > &sequence) :
    AstNode()
{
    for (unsigned i = 0; i < sequence.size(); i++) {
        sequence[i]->referenceCount++;
        sequence_.push_back(sequence[i]);
    }
}

AstVariableNode::AstVariableNode(vector<AstNodePtr>&ast_wrappers, vector<pair<Dyninst::Offset, Dyninst::Offset> > *ranges) :
    ast_wrappers_(ast_wrappers), ranges_(ranges), index(0)
{
   vector<AstNodePtr>::iterator i;
   assert(!ast_wrappers_.empty());

   for (i = ast_wrappers.begin(); i != ast_wrappers.end(); i++) {
      (*i)->referenceCount++;
   }
}

AstMemoryNode::AstMemoryNode(memoryType mem,
                             unsigned which,
                             int size_) :
    AstNode(),
    mem_(mem),
    which_(which) {

    assert(BPatch::bpatch != NULL);
    assert(BPatch::bpatch->stdTypes != NULL);


    switch(mem_) {
    case EffectiveAddr:
        switch (size_) {
            case 1:
                bptype = BPatch::bpatch->stdTypes->findType("char");
                break;
            case 2:
                bptype = BPatch::bpatch->stdTypes->findType("short");
                break;
            case 4:
                bptype = BPatch::bpatch->stdTypes->findType("int");
                break;
            default:
                bptype = BPatch::bpatch->stdTypes->findType("long");
        }
        break;
    case BytesAccessed:
        bptype = BPatch::bpatch->stdTypes->findType("int");
        break;
    default:
        assert(!"Naah...");
    }
    size = bptype->getSize();
    doTypeCheck = BPatch::bpatch->isTypeChecked();
}


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
    std::vector<AstNodePtr > args;
    // By not including a process we'll specialize at code generation.
    indexNode_ = AstNode::funcCallNode("DYNINSTthreadIndex", args);
    assert(indexNode_);
    indexNode_->setConstFunc(true);

    return indexNode_;
}

AstNode::~AstNode() {
    //printf("at ~AstNode()  count=%d\n", referenceCount);
}

Address AstMiniTrampNode::generateTramp(codeGen &gen,
                                        int &trampCost,
                                        bool noCost) {
    static AstNodePtr costAst;
    static AstNodePtr preamble;

    if (costAst == AstNodePtr())
        costAst = AstNode::operandNode(AstNode::operandType::Constant, (void *)0);

    if (preamble == AstNodePtr())
        preamble = AstNode::operatorNode(trampPreamble, costAst);

    // private constructor; assumes NULL for right child

    // we only want to use the cost of the minimum statements that will
    // be executed.  Statements, such as the body of an if statement,
    // will have their costs added to the observed cost global variable
    // only if they are indeed called.  The code to do this in the minitramp
    // right after the body of the if.
    trampCost = preamble->maxCost() + minCost();

    costAst->setOValue((void *) (long) trampCost);

    if (!preamble->generateCode(gen, noCost)) {
        fprintf(stderr, "[%s:%d] WARNING: failure to generate miniTramp preamble\n", __FILE__, __LINE__);
    }

    if (!ast_->generateCode(gen, noCost)) {
        fprintf(stderr, "[%s:%d] WARNING: failure to generate miniTramp body\n", __FILE__, __LINE__);
    }

    return 0;
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
	std::vector<AstNodePtr> children;
	getChildren(children);
	for (unsigned i=0; i<children.size(); i++) {
	    children[i]->setUseCount();
    }
}

void AstNode::cleanUseCount(void)
{
    useCount = 0;

    std::vector<AstNodePtr> children;
    getChildren(children);
    for (unsigned i=0; i<children.size(); i++) {
		children[i]->cleanUseCount();
    }
}

// Allocate a register and make it available for sharing if our
// node is shared
Dyninst::Register AstNode::allocateAndKeep(codeGen &gen, bool noCost)
{
    ast_printf("Allocating register for node %p, useCount %d\n", (void*)this, useCount);
    // Allocate a register
    Dyninst::Register dest = gen.rs()->allocateRegister(gen, noCost);

    ast_printf("Allocator returned %u\n", dest);
    assert(dest != Dyninst::Null_Register);

    if (useCount > 1) {
        ast_printf("Adding kept register %u for node %p: useCount %d\n", dest, (void*)this, useCount);
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
                           Dyninst::Register &retReg) {
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
    setVariableAST(gen);
    ast_printf("====== Code Generation Start ===== \n");
    ast_cerr << format("");
    ast_printf("\n\n");

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

    if (top_level) {
        entered = false;
        stats_codegen.stopTimer(CODEGEN_AST_TIMER);
    }
    return ret;
}

bool AstNode::generateCode(codeGen &gen,
                           bool noCost) {
    Address unused = ADDR_NULL;
    Dyninst::Register unusedReg = Dyninst::Null_Register;
    bool ret = generateCode(gen, noCost, unused, unusedReg);
    gen.rs()->freeRegister(unusedReg);

    return ret;
}

bool AstNode::previousComputationValid(Dyninst::Register &reg,
                                       codeGen &gen) {
	Dyninst::Register keptReg = gen.tracker()->hasKeptRegister(this);
	if (keptReg != Dyninst::Null_Register) {
		reg = keptReg;
		ast_printf("Returning previously used register %u for node %p\n", reg, (void*)this);
		return true;
	}
   return false;
}

// We're going to use this fragment over and over and over...
#define RETURN_KEPT_REG(r) do { if (previousComputationValid(r, gen)) { decUseCount(gen); gen.rs()->incRefCount(r); return true;} } while (0)
#define ERROR_RETURN do { fprintf(stderr, "[%s:%d] ERROR: failure to generate operand\n", __FILE__, __LINE__); return false; } while (0)
#define REGISTER_CHECK(r) do { if ((r) == Dyninst::Null_Register) { fprintf(stderr, "[%s: %d] ERROR: returned register invalid\n", __FILE__, __LINE__); return false; } } while (0)


bool AstNode::initRegisters(codeGen &g) {
    bool ret = true;
    std::vector<AstNodePtr> kids;
    getChildren(kids);
    for (unsigned i = 0; i < kids.size(); i++) {
        if (!kids[i]->initRegisters(g))
            ret = false;
    }
    return ret;
}


bool AstNode::generateCode_phase2(codeGen &, bool,
                                  Address &,
                                  Dyninst::Register &) {
    fprintf(stderr, "ERROR: call to AstNode generateCode_phase2; should be handled by subclass\n");
    fprintf(stderr, "Undefined phase2 for:\n");
    if (dynamic_cast<AstNullNode *>(this)) fprintf(stderr, "nullNode\n");
    if (dynamic_cast<AstStackInsertNode *>(this)) fprintf(stderr, "stackInsertNode\n");
    if (dynamic_cast<AstStackRemoveNode *>(this)) fprintf(stderr, "stackRemoveNode\n");
    if (dynamic_cast<AstStackGenericNode *>(this)) fprintf(stderr, "stackMoveNode\n");
    if (dynamic_cast<AstOperatorNode *>(this)) fprintf(stderr, "operatorNode\n");
    if (dynamic_cast<AstOperandNode *>(this)) fprintf(stderr, "operandNode\n");
    if (dynamic_cast<AstCallNode *>(this)) fprintf(stderr, "callNode\n");
    if (dynamic_cast<AstSequenceNode *>(this)) fprintf(stderr, "seqNode\n");
    if (dynamic_cast<AstVariableNode *>(this)) fprintf(stderr, "varNode\n");
    if (dynamic_cast<AstMiniTrampNode *>(this)) fprintf(stderr, "miniTrampNode\n");
    if (dynamic_cast<AstMemoryNode *>(this)) fprintf(stderr, "memoryNode\n");
    assert(0);
	return 0;
}


bool AstNullNode::generateCode_phase2(codeGen &gen, bool,
                                      Address &retAddr,
                                      Dyninst::Register &retReg) {
    retAddr = ADDR_NULL;
    retReg = Dyninst::Null_Register;

    decUseCount(gen);

    return true;
}

bool AstNode::allocateCanaryRegister(codeGen& gen,
        bool noCost,
        Dyninst::Register& reg,
        bool& needSaveAndRestore)
{
    // Let's see if we can find a dead register to use!
    instPoint* point = gen.point();

    // Try to get a scratch register from the register space
    registerSpace* regSpace = registerSpace::actualRegSpace(point);
    bool realReg = true;
    Dyninst::Register tmpReg = regSpace->getScratchRegister(gen, noCost, realReg);
    if (tmpReg != Dyninst::Null_Register) {
        reg = tmpReg;
        needSaveAndRestore = false;
        if (gen.getArch() == Arch_x86) {
            gen.rs()->noteVirtualInReal(reg, RealRegister(reg));
        }
        return true;
    }

    // Couldn't find a dead register to use :-(
    registerSpace* deadRegSpace = registerSpace::optimisticRegSpace(gen.addrSpace());
    reg = deadRegSpace->getScratchRegister(gen, noCost, realReg);
    if (reg == Dyninst::Null_Register) {
        fprintf(stderr, "WARNING: using default allocateAndKeep in allocateCanaryRegister\n");
        reg = allocateAndKeep(gen, noCost);
    }
    needSaveAndRestore = true;
    fprintf(stderr, "allocateCanaryRegister will require save&restore at 0x%lx\n", gen.point()->addr());

    return true;
}


#if defined(cap_stack_mods)
bool AstStackInsertNode::generateCode_phase2(codeGen &gen, bool noCost,
        Address &,
        Dyninst::Register &)
{
    // Turn off default basetramp instrumentation saves & restores
    gen.setInsertNaked(true);
    gen.setModifiedStackFrame(true);

    bool ignored;
    Dyninst::Register reg_sp = convertRegID(MachRegister::getStackPointer(gen.getArch()), ignored);

    Emitterx86* emitter = dynamic_cast<Emitterx86*>(gen.codeEmitter());
    assert(emitter);

    if (type == GENERIC_AST) {
        /* We're going to use a MOV to insert the new value, and a LEA to update the SP
         * This is instead of using a push, which requires a free register */

        /* Move stack pointer to accomodate new value */
        if (gen.getArch() == Arch_x86) {
            emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -size, reg_sp, gen);
        } else if (gen.getArch() == Arch_x86_64) {
            emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -size, reg_sp, gen);
        }

    } else if (type == CANARY_AST){
//        gen.setCanary(true);

        // Find a register to use
        Dyninst::Register canaryReg = Dyninst::Null_Register;
        bool needSaveAndRestore = true;

        // 64-bit requires stack alignment
        // We'll do this BEFORE we push the canary for two reasons:
        // 1. Easier to pop the canary in the check at the end of the function
        // 2. Ensures that the canary will get overwritten (rather than empty alignment space) in case of an overflow
        if (gen.getArch() == Arch_x86_64) {
            allocateCanaryRegister(gen, noCost, canaryReg, needSaveAndRestore);

            int canarySize = 8;
            int off = AMD64_STACK_ALIGNMENT - canarySize; // canary
            emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -off, reg_sp, gen);
        } else {
            canaryReg = REGNUM_EAX;
            needSaveAndRestore = true;
        }

        // Save canaryReg value if necessary
        if (needSaveAndRestore) {
            if (gen.getArch() == Arch_x86) {
                int disp = 4;
                emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -disp, reg_sp, gen);
                gen.codeEmitter()->emitPush(gen, canaryReg);
                emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, 2*disp, reg_sp, gen);
            } else if (gen.getArch() == Arch_x86_64) {
                int disp = 8;
                emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -disp, reg_sp, gen);
                gen.codeEmitter()->emitPush(gen, canaryReg);
                emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, 2*disp, reg_sp, gen);
            }
        }

        // Set up canary value
        // CURRENT USES GLIBC-PROVIDED VALUE;
        // from gcc/config/i386/gnu-user64.h:
        //  #ifdef TARGET_LIBC_PROVIDES_SSP
        //  /* i386 glibc provides __stack_chk_guard in %gs:0x14,
        //      x32 glibc provides it in %fs:0x18.
        //      x86_64 glibc provides it in %fs:0x28.  */
        //  #define TARGET_THREAD_SSP_OFFSET (TARGET_64BIT ? (TARGET_X32 ? 0x18 : 0x28) : 0x14))) */

        // goal:
        //      x86:    mov %fs:0x14, canaryReg
        //      x86_64: mov %gs:0x28, canaryReg
        if (gen.getArch() == Arch_x86) {
            emitter->emitLoadRelativeSegReg(canaryReg, 0x14, REGNUM_GS, 4, gen);
        } else if (gen.getArch() == Arch_x86_64) {
            emitter->emitLoadRelativeSegReg(canaryReg, 0x28, REGNUM_FS, 8, gen);
        }

        // Push the canary value
        gen.codeEmitter()->emitPush(gen, canaryReg);

        // Clear canary register to prevent info leaking
        emitter->emitXorRegReg(canaryReg, canaryReg, gen);

        // Restore canaryReg value if necessary
        if (needSaveAndRestore) {
            if (gen.getArch() == Arch_x86) {
                int disp = 4;
                emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -disp, reg_sp, gen);
                gen.codeEmitter()->emitPop(gen, canaryReg);
            } else if (gen.getArch() == Arch_x86_64) {
                int disp = 8;
                emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -disp, reg_sp, gen);
                emitter->emitPop(gen, canaryReg);
            }
        }

        // C&P from nullNode
        decUseCount(gen);
    }

    return true;
}

bool AstStackRemoveNode::generateCode_phase2(codeGen &gen, bool noCost,
        Address &,
        Dyninst::Register &)
{
    // Turn off default basetramp instrumentation saves & restores
    gen.setInsertNaked(true);
    gen.setModifiedStackFrame(true);

    bool ignored;
    Dyninst::Register reg_sp = convertRegID(MachRegister::getStackPointer(gen.getArch()), ignored);

    Emitterx86* emitter = dynamic_cast<Emitterx86*>(gen.codeEmitter());
    assert(emitter);

    if (type == GENERIC_AST) {
        /* Adjust stack pointer by size */
        int disp = size;
        if (gen.getArch() == Arch_x86) {
            emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, disp, reg_sp, gen);
        } else if (gen.getArch() == Arch_x86_64) {
            emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, disp, reg_sp, gen);
        }
    } else if (type == CANARY_AST) {
//        gen.setCanary(true);

        // Find a register to use
        Dyninst::Register canaryReg = Dyninst::Null_Register;
        bool needSaveAndRestore = true;
        if (gen.getArch() == Arch_x86_64) {
            allocateCanaryRegister(gen, noCost, canaryReg, needSaveAndRestore);
        }
        else {
            canaryReg = REGNUM_EDX;
            gen.rs()->noteVirtualInReal(canaryReg, RealRegister(canaryReg));
            needSaveAndRestore = true;
        }
        // Save canaryReg value if necessary
        if (needSaveAndRestore) {
            if (gen.getArch() == Arch_x86) {
                int disp = 4;
                gen.codeEmitter()->emitPush(gen, canaryReg);
                emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, disp, reg_sp, gen);
            } else if (gen.getArch() == Arch_x86_64) {
                int disp = 8;
                emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, disp, reg_sp, gen);
            }
        }

        // Retrieve canary value and verify its integrity
        if (!canaryAfterPrologue_) {
            gen.codeEmitter()->emitPop(gen, canaryReg);
        } else {
            Address canaryOffset = -1*(Address)(canaryHeight_);
            if (gen.getArch() == Arch_x86) {
                Dyninst::Register destReg = reg_sp;
                RealRegister canaryReg_r = gen.rs()->loadVirtualForWrite(canaryReg, gen);
                emitMovRMToReg(canaryReg_r, RealRegister(destReg), canaryOffset, gen);
            } else if (gen.getArch() == Arch_x86_64) {
                Dyninst::Register destReg = reg_sp;
                gen.codeEmitter()->emitLoadRelative(canaryReg, canaryOffset, destReg, 0, gen);
            }
        }

        // CURRENTLY USES GLIBC-PROVIDED VALUE;
        //  /* i386 glibc provides __stack_chk_guard in %gs:0x14,
        //      x32 glibc provides it in %fs:0x18.
        //      x86_64 glibc provides it in %fs:0x28.  */
        // goal:
        //      x86: xor %fs:0x14, canaryReg
        //      x86_64: xor %gs:0x28, canaryReg
        if (gen.getArch() == Arch_x86) {
            emitter->emitXorRegSegReg(canaryReg, REGNUM_GS, 0x14, gen);
        } else if (gen.getArch() == Arch_x86_64) {
            emitter->emitXorRegSegReg(canaryReg, REGNUM_FS, 0x28, gen);
        }

        // Restore canaryReg if necessary
        if (needSaveAndRestore) {
            if (gen.getArch() == Arch_x86) {
                int disp = 4;
                emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -disp, reg_sp, gen);
                gen.codeEmitter()->emitPop(gen, canaryReg);
            } else if (gen.getArch() == Arch_x86_64) {
                int disp = 8;
                emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -disp, reg_sp, gen);
                gen.codeEmitter()->emitPop(gen, canaryReg);
            }
       }

        // Fix up the stack in the canaryAfterPrologue case
        if (canaryAfterPrologue_) {
            if (gen.getArch() == Arch_x86) {
                int disp = 4;
                emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -1*canaryHeight_ + disp, reg_sp, gen);
            } else if (gen.getArch() == Arch_x86_64) {
                int disp = 8;
                emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -1*canaryHeight_ + disp, reg_sp, gen);
            }
        }

        // Re-align the stack
        if (gen.getArch() == Arch_x86_64) {
            // 64-bit requires stack alignment (this will include canary cleanup)
            int canarySize = 8;
            int off = AMD64_STACK_ALIGNMENT - canarySize;
            emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, off, reg_sp, gen);
        }

        // If the canary value is valid, jmp to next expected instruction
        int condition = 0x4;
        int offset = 1; // This is just the placeholder
        bool willRegen = true; // This gets the longer form of the jcc, so we can patch it up
        codeBufIndex_t jccIndex = gen.getIndex();
        emitJcc(condition, offset, gen, willRegen);

        // Otherwise, call specified failure function
        // NOTE: Skipping saving live registers that will be clobbered by the call because this will be non-returning
        std::vector<AstNodePtr> operands;
        func_instance* func = func_; // c&p'd from AstCallNode
        codeBufIndex_t preCallIndex = gen.getIndex();
        emitter->emitCallInstruction(gen, func, canaryReg);
        codeBufIndex_t postCallIndex = gen.getIndex();

        // Fix-up the jcc
        offset = postCallIndex - preCallIndex;
        gen.setIndex(jccIndex);
        emitJcc(condition, offset, gen, willRegen);
        gen.setIndex(postCallIndex);

        decUseCount(gen);
    }

    return true;
}

bool AstStackGenericNode::generateCode_phase2(codeGen& gen, bool, Address&, Dyninst::Register&)
{
    gen.setInsertNaked(true);
    gen.setModifiedStackFrame(true);

    // No code generation necessary

    return true;
}
#else
bool AstStackInsertNode::generateCode_phase2(codeGen&, bool,
        Address &,
        Dyninst::Register &) {
    return false;
}

bool AstStackRemoveNode::generateCode_phase2(codeGen&, bool,
        Address &,
        Dyninst::Register &)
{
    return false;
}

bool AstStackGenericNode::generateCode_phase2(codeGen&, bool, Address&, Dyninst::Register&)
{
    return false;
}
#endif

bool AstLabelNode::generateCode_phase2(codeGen &gen, bool,
                                       Address &retAddr,
                                       Dyninst::Register &retReg) {
	assert(generatedAddr_ == 0);
    // Pick up the address we were added at
    generatedAddr_ = gen.currAddr();

    retAddr = ADDR_NULL;
    retReg = Dyninst::Null_Register;

	decUseCount(gen);

    return true;
}

bool AstOperatorNode::initRegisters(codeGen &g) {
    bool ret = true;
    std::vector<AstNodePtr> kids;
    getChildren(kids);
    for (unsigned i = 0; i < kids.size(); i++) {
        if (!kids[i]->initRegisters(g))
            ret = false;
    }

#if !defined(arch_x86)
    // Override: if we're trying to save to an original
    // register, make sure it's saved on the stack.
    if(loperand) {
      if (op == storeOp) {
        if (loperand->getoType() == operandType::origRegister) {
          Address origReg = (Address) loperand->getOValue();
          // Mark that register as live so we are sure to save it.
          registerSlot *r = (*(g.rs()))[origReg];
          r->liveState = registerSlot::live;
        }
      }
    }
#endif
    return ret;
}

#if defined(arch_x86) || defined(arch_x86_64)
bool AstOperatorNode::generateOptimizedAssignment(codeGen &gen, int size_, bool noCost)
{
   if(!(loperand && roperand)) { return false; }

   //Recognize the common case of 'a = a op constant' and try to
   // generate optimized code for this case.
   Address laddr;

   if (loperand->getoType() == operandType::DataAddr)
   {
      laddr = (Address) loperand->getOValue();
   }
   else
   {
      if(loperand->getoType() == operandType::variableValue)
      {
         boost::shared_ptr<AstOperandNode> lnode =
            boost::dynamic_pointer_cast<AstOperandNode>(loperand);

         int_variable* var = lnode->lookUpVar(gen.addrSpace());
         if (!var || gen.addrSpace()->needsPIC(var))
            return false;
         laddr = var->getAddress();
      }
      else
      {
         //Deal with global writes for now.
         return false;
      }

   }

   if (roperand->getoType() == operandType::Constant) {
      //Looks like 'global = constant'
#if defined(arch_x86_64)
     if (laddr >> 32 || ((Address) roperand->getOValue()) >> 32 || size_ == 8) {
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
   if (!arithl || !arithr)
      return false;

   AstNode *const_oper = NULL;
   if (arithl->getoType() == operandType::DataAddr && arithr->getoType() == operandType::Constant &&
       laddr == (Address) arithl->getOValue())
   {
      const_oper = arithr;
   }
   else if (arithl->getoType() == operandType::variableValue && arithr->getoType() == operandType::Constant)
   {
      Address addr = 0;
      int_variable* var = arithl->lookUpVar(gen.addrSpace());
      if (!var || gen.addrSpace()->needsPIC(var))
         return false;
      addr = var->getAddress();
      if (addr == laddr) {
         const_oper = arithr;
      }
   }
   else if (arithr->getoType() == operandType::DataAddr && arithl->getoType() == operandType::Constant &&
            laddr == (Address) arithr->getOValue() && roper->op == plusOp)
   {
      const_oper = arithl;
   }
   else if (arithl->getoType() == operandType::variableValue && arithr->getoType() == operandType::Constant)
   {
      Address addr = 0;
      int_variable* var = arithl->lookUpVar(gen.addrSpace());
      if(!var || gen.addrSpace()->needsPIC(var))
         return false;
      addr = var->getAddress();
      if (addr == laddr) {
         const_oper = arithl;
      }
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
}
#else
bool AstOperatorNode::generateOptimizedAssignment(codeGen &, int, bool)
{
   return false;
}
#endif

bool AstOperatorNode::generateCode_phase2(codeGen &gen, bool noCost,
                                          Address &retAddr,
                                          Dyninst::Register &retReg) {
   if(!loperand) { return false; }

   retAddr = ADDR_NULL; // We won't be setting this...
   // retReg may have a value or be the (register) equivalent of NULL.
   // In either case, we don't touch it...

	RETURN_KEPT_REG(retReg);


   Address addr = ADDR_NULL;

   Dyninst::Register src1 = Dyninst::Null_Register;
   Dyninst::Register src2 = Dyninst::Null_Register;

   Dyninst::Register right_dest = Dyninst::Null_Register;
   Dyninst::Register tmp = Dyninst::Null_Register;

   switch(op) {
      case branchOp: {
         assert(loperand->getoType() == operandType::Constant);
         unsigned offset = (Dyninst::Register) (long) loperand->getOValue();
         // We are not calling loperand->generateCode_phase2,
         // so we decrement its useCount by hand.
         // Would be nice to allow register branches...
         loperand->decUseCount(gen);
         (void)emitA(branchOp, 0, 0, (Dyninst::Register)offset, gen, rc_no_control, noCost);
         retReg = Dyninst::Null_Register; // No return register
         break;
      }
      case ifOp: {
         if(!roperand) { return false; }
         // This ast cannot be shared because it doesn't return a register
         if (!loperand->generateCode_phase2(gen, noCost, addr, src1)) ERROR_RETURN;
         REGISTER_CHECK(src1);
         codeBufIndex_t ifIndex= gen.getIndex();

         size_t preif_patches_size = gen.allPatches().size();
         codeBufIndex_t thenSkipStart = emitA(op, src1, 0, 0, gen, rc_before_jump, noCost);

         size_t postif_patches_size = gen.allPatches().size();

	 // We can reuse src1 for the body of the conditional; however, keep the value here
	 // so that we can use it for the branch fix below.
         Dyninst::Register src1_copy = src1;
         if (loperand->decRefCount())
            gen.rs()->freeRegister(src1);

         // The flow of control forks. We need to add the forked node to
         // the path
         gen.tracker()->increaseConditionalLevel();
         if (!roperand->generateCode_phase2(gen, noCost, addr, src2)) ERROR_RETURN;
         if (roperand->decRefCount())
            gen.rs()->freeRegister(src2);
         gen.tracker()->decreaseAndClean(gen);
         gen.rs()->unifyTopRegStates(gen); //Join the registerState for the if

         // Is there an else clause?  If yes, generate branch over it
         codeBufIndex_t elseSkipStart = 0;
         codeBufIndex_t elseSkipIndex = gen.getIndex();
         size_t preelse_patches_size = 0, postelse_patches_size = 0;
         if (eoperand) {
            gen.rs()->pushNewRegState(); //Create registerState for else
            preelse_patches_size = gen.allPatches().size();
            elseSkipStart = emitA(branchOp, 0, 0, 0,
                                  gen, rc_no_control, noCost);
            postelse_patches_size = gen.allPatches().size();
         }

         // Now that we've generated the "then" section, rewrite the if
         // conditional branch.
         codeBufIndex_t elseStartIndex = gen.getIndex();

         if (preif_patches_size != postif_patches_size) {
            assert(postif_patches_size > preif_patches_size);
            ifTargetPatch if_targ(elseStartIndex + gen.startAddr());
            for (unsigned i=preif_patches_size; i < postif_patches_size; i++) {
               gen.allPatches()[i].setTarget(&if_targ);
            }
            for (unsigned i=preif_patches_size; i < postif_patches_size; i++) {
               gen.allPatches()[i].applyPatch();
            }
         }
         else {
            gen.setIndex(ifIndex);
            // call emit again now with correct offset.
            // This backtracks over current code.
            // If/when we vectorize, we can do this in a two-pass arrangement
            (void) emitA(op, src1_copy, 0,
                         (Dyninst::Register) codeGen::getDisplacement(thenSkipStart, elseStartIndex),
                         gen, rc_no_control, noCost);
            // Now we can free the register
            // Dyninst::Register has already been freed; we're just re-using it.
            //gen.rs()->freeRegister(src1);

            gen.setIndex(elseStartIndex);
         }

         if (eoperand) {
            // If there's an else clause, we need to generate code for it.
            gen.tracker()->increaseConditionalLevel();
            if (!eoperand->generateCode_phase2(gen,
                                               noCost,
                                               addr,
                                               src2)) ERROR_RETURN;
            if (eoperand->decRefCount())
               gen.rs()->freeRegister(src2);
            gen.tracker()->decreaseAndClean(gen);
            gen.rs()->unifyTopRegStates(gen); //Join the registerState for the else

            // We also need to fix up the branch at the end of the "true"
            // clause to jump around the "else" clause.
            codeBufIndex_t endIndex = gen.getIndex();
            if (preelse_patches_size != postelse_patches_size) {
               assert(postif_patches_size > preif_patches_size);
               ifTargetPatch else_targ(endIndex + gen.startAddr());
               for (unsigned i=preelse_patches_size; i < postelse_patches_size; i++) {
                  gen.allPatches()[i].setTarget(&else_targ);
               }
               for (unsigned i=preelse_patches_size; i < postelse_patches_size; i++) {
                  gen.allPatches()[i].applyPatch();
               }
            }
            else {
               gen.setIndex(elseSkipIndex);
               emitA(branchOp, 0, 0,
                     (Dyninst::Register) codeGen::getDisplacement(elseSkipStart, endIndex),
                     gen, rc_no_control, noCost);
               gen.setIndex(endIndex);
            }
         }
         retReg = Dyninst::Null_Register;
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

         BPatch_addressSpace *bproc = (BPatch_addressSpace *) gen.addrSpace()->up_ptr();
         BPatch_point *bpoint = bproc->findOrCreateBPPoint(NULL, gen.point(), BPatch_point::convertInstPointType_t(gen.point()->type()));

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
            if (loperand->decRefCount())
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
            if (loperand->decRefCount())
               gen.rs()->freeRegister(src1);
         }

         break;
      }
      case whileOp: {
        if(!roperand) { return false; }
        codeBufIndex_t top = gen.getIndex(); 

        // BEGIN from ifOp       
        if (!loperand->generateCode_phase2(gen, noCost, addr, src1)) ERROR_RETURN;
         REGISTER_CHECK(src1);
         codeBufIndex_t startIndex= gen.getIndex();

         size_t preif_patches_size = gen.allPatches().size();
         codeBufIndex_t thenSkipStart = emitA(ifOp, src1, 0, 0, gen, rc_before_jump, noCost);

         size_t postif_patches_size = gen.allPatches().size();

	 // We can reuse src1 for the body of the conditional; however, keep the value here
	 // so that we can use it for the branch fix below.
         Dyninst::Register src1_copy = src1;
         if (loperand->decRefCount())
            gen.rs()->freeRegister(src1);

         // The flow of control forks. We need to add the forked node to
         // the path
         gen.tracker()->increaseConditionalLevel();
         if (!roperand->generateCode_phase2(gen, noCost, addr, src2)) ERROR_RETURN;
         if (roperand->decRefCount())
            gen.rs()->freeRegister(src2);
         gen.tracker()->decreaseAndClean(gen);
         gen.rs()->unifyTopRegStates(gen); //Join the registerState for the if
         
         // END from ifOp

         (void) emitA(branchOp, 0, 0, codeGen::getDisplacement(gen.getIndex(), top),
                      gen, rc_no_control, noCost);

         //BEGIN from ifOp

         // Now that we've generated the "then" section, rewrite the if
         // conditional branch.
         codeBufIndex_t elseStartIndex = gen.getIndex();

         if (preif_patches_size != postif_patches_size) {
            assert(postif_patches_size > preif_patches_size);
            ifTargetPatch if_targ(elseStartIndex + gen.startAddr());
            for (unsigned i=preif_patches_size; i < postif_patches_size; i++) {
               gen.allPatches()[i].setTarget(&if_targ);
            }
            for (unsigned i=preif_patches_size; i < postif_patches_size; i++) {
               gen.allPatches()[i].applyPatch();
            }
         }
         else {
            gen.setIndex(startIndex);
            // call emit again now with correct offset.
            // This backtracks over current code.
            // If/when we vectorize, we can do this in a two-pass arrangement
            (void) emitA(ifOp, src1_copy, 0,
                         (Dyninst::Register) codeGen::getDisplacement(thenSkipStart, elseStartIndex),
                         gen, rc_no_control, noCost);
            // Now we can free the register
            // Dyninst::Register has already been freed; we're just re-using it.
            //gen.rs()->freeRegister(src1);

            gen.setIndex(elseStartIndex);
         }
         // END from ifOp
         retReg = Dyninst::Null_Register;
         break;
      }
      case doOp: {
         fprintf(stderr, "[%s:%d] WARNING: do AST node unimplemented!\n", __FILE__, __LINE__);
         return false;
      }
      case getAddrOp: {
         switch(loperand->getoType()) {
            case operandType::variableAddr:
               if (retReg == Dyninst::Null_Register) {
                  retReg = allocateAndKeep(gen, noCost);
               }
               assert (loperand->getOVar());
               loperand->emitVariableLoad(loadConstOp, retReg, retReg, gen, noCost, gen.rs(), size,
                                          gen.point(), gen.addrSpace());
               break;
            case operandType::variableValue:
               if (retReg == Dyninst::Null_Register) {
                  retReg = allocateAndKeep(gen, noCost);
               }
               assert (loperand->getOVar());
               loperand->emitVariableLoad(loadOp, retReg, retReg, gen, noCost, gen.rs(), size,
                                          gen.point(), gen.addrSpace());
               break;
            case operandType::DataAddr:
               {
                  addr = reinterpret_cast<Address>(loperand->getOValue());
                  if (retReg == Dyninst::Null_Register) {
                     retReg = allocateAndKeep(gen, noCost);
                  }
                  assert(!loperand->getOVar());
                  emitVload(loadConstOp, addr, retReg, retReg, gen,
                            noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               }
               break;
            case operandType::FrameAddr: {
               // load the address fp + addr into dest
               if (retReg == Dyninst::Null_Register)
                  retReg = allocateAndKeep(gen, noCost);
               Dyninst::Register temp = gen.rs()->getScratchRegister(gen, noCost);
               addr = (Address) loperand->getOValue();
               emitVload(loadFrameAddr, addr, temp, retReg, gen,
                         noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               break;
            }
            case operandType::RegOffset: {
               assert(loperand->operand());

               // load the address reg + addr into dest
               if (retReg == Dyninst::Null_Register) {
                  retReg = allocateAndKeep(gen, noCost);
               }
               addr = (Address) loperand->operand()->getOValue();

               emitVload(loadRegRelativeAddr, addr, (long)loperand->getOValue(), retReg, gen,
                         noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               break;
            }
            case operandType::DataIndir:
               // taking address of pointer de-ref returns the original
               //    expression, so we simple generate the left child's
               //    code to get the address
               if (!loperand->operand()->generateCode_phase2(gen,
                                                             noCost,
                                                             addr,
                                                             retReg)) ERROR_RETURN;
               // Broken refCounts?
               break;
            case operandType::origRegister:
               // Added 2NOV11 Bernat - some variables live in original registers,
               // and so we need to be able to dereference their contents.
               if (!loperand->generateCode_phase2(gen, noCost, addr, retReg)) ERROR_RETURN;
               break;
            default:
               cerr << "Uh oh, unknown loperand type in getAddrOp: " << static_cast<uint64_t>(loperand->getoType()) << endl;
               cerr << "\t Generating ast " << hex << this << dec << endl;
               assert(0);
         }
         break;
      }
      case storeOp: {
        if(!roperand) { return false; }
	bool result = generateOptimizedAssignment(gen, size, noCost);
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
            case operandType::variableValue:
               loperand->emitVariableStore(storeOp, src1, src2, gen,
                                           noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               loperand->decUseCount(gen);
               break;
            case operandType::DataAddr:
               addr = (Address) loperand->getOValue();
               assert(loperand->getOVar() == NULL);
               emitVstore(storeOp, src1, src2, addr, gen,
                          noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               // We are not calling generateCode for the left branch,
               // so need to decrement the refcount by hand
               loperand->decUseCount(gen);
               break;
            case operandType::FrameAddr:
               addr = (Address) loperand->getOValue();
               emitVstore(storeFrameRelativeOp, src1, src2, addr, gen,
                          noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               loperand->decUseCount(gen);
               break;
            case operandType::RegOffset: {
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
            case operandType::DataIndir: {
               // store to a an expression (e.g. an array or field use)
               // *(+ base offset) = src1
               if (!loperand->operand()->generateCode_phase2(gen,
                                                             noCost,
                                                             addr,
                                                             tmp)) ERROR_RETURN;
               REGISTER_CHECK(tmp);

               // tmp now contains address to store into
               emitV(storeIndirOp, src1, 0, tmp, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               if (loperand->operand()->decRefCount())
                  gen.rs()->freeRegister(tmp);
               loperand->decUseCount(gen);
               break;
            }
            case operandType::origRegister:
               gen.rs()->writeProgramRegister(gen, (Dyninst::Register)(long)loperand->getOValue(),
                                              src1, getSize());
               //emitStorePreviousStackFrameRegister((Address) loperand->getOValue(),
               //src1, gen, getSize(), noCost);
               loperand->decUseCount(gen);
               break;
            case operandType::Param:
            case operandType::ParamAtCall:
            case operandType::ParamAtEntry: {
               boost::shared_ptr<AstOperandNode> lnode =
                  boost::dynamic_pointer_cast<AstOperandNode>(loperand);
               emitR(getParamOp, (Address)lnode->oValue,
                     src1, src2, gen, noCost, gen.point(),
                     gen.addrSpace()->multithread_capable());
               loperand->decUseCount(gen);
               break;
            }
            case operandType::ReturnVal:
               emitR(getRetValOp, Dyninst::Null_Register,
                     src1, src2, gen, noCost, gen.point(),
                     gen.addrSpace()->multithread_capable());
               loperand->decUseCount(gen);
               break;
            case operandType::ReturnAddr:
                emitR(getRetAddrOp, Dyninst::Null_Register,
                      src1, src2, gen, noCost, gen.point(),
                      gen.addrSpace()->multithread_capable());
                break;
            default: {
               // Could be an error, could be an attempt to load based on an arithmetic expression
               // Generate the left hand side, store the right to that address
               if (!loperand->generateCode_phase2(gen, noCost, addr, tmp)) ERROR_RETURN;
               REGISTER_CHECK(tmp);

               emitV(storeIndirOp, src1, 0, tmp, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());
               if (loperand->decRefCount())
                  gen.rs()->freeRegister(tmp);
               break;
            }
         }
         if (roperand->decRefCount())
            gen.rs()->freeRegister(src1);
         gen.rs()->freeRegister(src2);
         retReg = Dyninst::Null_Register;
         break;
      }
      case storeIndirOp: {
        if(!roperand) { return false; }
         if (!roperand->generateCode_phase2(gen, noCost, addr, src1)) ERROR_RETURN;
         if (!loperand->generateCode_phase2(gen, noCost, addr, src2)) ERROR_RETURN;
         REGISTER_CHECK(src1);
         REGISTER_CHECK(src2);
         emitV(op, src1, 0, src2, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace());
         if (roperand->decRefCount())
            gen.rs()->freeRegister(src1);
         if (loperand->decRefCount())
            gen.rs()->freeRegister(src2);
         retReg = Dyninst::Null_Register;
         break;
      }
      case trampPreamble: {
         // This ast cannot be shared because it doesn't return a register
         retReg = Dyninst::Null_Register;
         break;
      }
      case plusOp:
      case minusOp:
      case xorOp:
      case timesOp:
      case divOp:
      case orOp:
      case andOp:
      case eqOp:
      case neOp:
      case lessOp:
      case leOp:
      case greaterOp:
      case geOp:
      default:
      {
         if(!roperand) { return false; }
         bool signedOp = IsSignedOperation(loperand->getType(), roperand->getType());
         src1 = Dyninst::Null_Register;
         right_dest = Dyninst::Null_Register;
            if (!loperand->generateCode_phase2(gen,
                                               noCost, addr, src1)) ERROR_RETURN;
            REGISTER_CHECK(src1);

         if ((roperand->getoType() == operandType::Constant) &&
             doNotOverflow((int64_t)roperand->getOValue())) {
            if (retReg == Dyninst::Null_Register) {
               retReg = allocateAndKeep(gen, noCost);
               ast_printf("Operator node, const RHS, allocated register %u\n", retReg);
            }
            else
               ast_printf("Operator node, const RHS, keeping register %u\n", retReg);

            emitImm(op, src1, (RegValue) roperand->getOValue(), retReg, gen, noCost, gen.rs(), signedOp);

            if (src1 != Dyninst::Null_Register && loperand->decRefCount())
               gen.rs()->freeRegister(src1);

            // We do not .generateCode for roperand, so need to update its
            // refcounts manually
            roperand->decUseCount(gen);
         }
         else {
               if (!roperand->generateCode_phase2(gen, noCost, addr, right_dest)) ERROR_RETURN;
               REGISTER_CHECK(right_dest);
            if (retReg == Dyninst::Null_Register) {
               retReg = allocateAndKeep(gen, noCost);
            }
            emitV(op, src1, right_dest, retReg, gen, noCost, gen.rs(), size, gen.point(), gen.addrSpace(), signedOp);
            if (src1 != Dyninst::Null_Register && loperand->decRefCount()) {
               // Don't free inputs until afterwards; we have _no_ idea
               gen.rs()->freeRegister(src1);
            }
            // what the underlying code might do with a temporary register.
            if (right_dest != Dyninst::Null_Register && roperand->decRefCount())
               gen.rs()->freeRegister(right_dest);
         }
      }
   }
	decUseCount(gen);
   return true;
}

bool AstOperandNode::generateCode_phase2(codeGen &gen, bool noCost,
                                         Address &,
                                         Dyninst::Register &retReg) {
	RETURN_KEPT_REG(retReg);


    Address addr = ADDR_NULL;
    Dyninst::Register src = Dyninst::Null_Register;

   // Allocate a register to return
   if (oType != operandType::DataReg) {
       if (retReg == Dyninst::Null_Register) {
           retReg = allocateAndKeep(gen, noCost);
       }
   }
   Dyninst::Register temp;
   int tSize;
   int len;
   BPatch_type *Type;
   switch (oType) {
   case operandType::Constant:
     assert(oVar == NULL);
     emitVload(loadConstOp, (Address)oValue, retReg, retReg, gen,
		 noCost, gen.rs(), size, gen.point(), gen.addrSpace());
     break;
   case operandType::DataIndir:
      if (!operand_->generateCode_phase2(gen, noCost, addr, src)) ERROR_RETURN;
      REGISTER_CHECK(src);
      Type = const_cast<BPatch_type *> (getType());
      // Internally generated calls will not have type information set
      if(Type)
         tSize = Type->getSize();
      else
         tSize = sizeof(long);
      emitV(loadIndirOp, src, 0, retReg, gen, noCost, gen.rs(), tSize, gen.point(), gen.addrSpace());
      if (operand_->decRefCount())
         gen.rs()->freeRegister(src);
      break;
   case operandType::DataReg:
       retReg = (Dyninst::Register) (long) oValue;
       break;
   case operandType::origRegister:
      gen.rs()->readProgramRegister(gen, (Dyninst::Register)(long)oValue, retReg, size);
       //emitLoadPreviousStackFrameRegister((Address) oValue, retReg, gen,
       //size, noCost);
       break;
   case operandType::variableAddr:
     assert(oVar);
     emitVariableLoad(loadConstOp, retReg, retReg, gen,
		 noCost, gen.rs(), size, gen.point(), gen.addrSpace());
     break;
   case operandType::variableValue:
     assert(oVar);
     emitVariableLoad(loadOp, retReg, retReg, gen,
        noCost, gen.rs(), size, gen.point(), gen.addrSpace());
     break;
   case operandType::ReturnVal:
       src = emitR(getRetValOp, 0, Dyninst::Null_Register, retReg, gen, noCost, gen.point(),
                   gen.addrSpace()->multithread_capable());
       REGISTER_CHECK(src);
       if (src != retReg) {
           // Move src to retReg. Can't simply return src, since it was not
           // allocated properly
           emitImm(orOp, src, 0, retReg, gen, noCost, gen.rs());
       }
       break;
   case operandType::ReturnAddr:
       src = emitR(getRetAddrOp, 0, Dyninst::Null_Register, retReg, gen, noCost, gen.point(),
                   gen.addrSpace()->multithread_capable());
       REGISTER_CHECK(src);
       if (src != retReg) {
           // Move src to retReg. Can't simply return src, since it was not
           // allocated properly
           emitImm(orOp, src, 0, retReg, gen, noCost, gen.rs());
       }
       break;
   case operandType::Param:
   case operandType::ParamAtCall:
   case operandType::ParamAtEntry: {
       opCode paramOp = undefOp;
       switch(oType) {
           case operandType::Param:
               paramOp = getParamOp;
               break;
           case operandType::ParamAtCall:
               paramOp = getParamAtCallOp;
               break;
           case operandType::ParamAtEntry:
               paramOp = getParamAtEntryOp;
               break;
           default:
               assert(0);
               break;
       }
       src = emitR(paramOp, (Address)oValue, Dyninst::Null_Register,
                   retReg, gen, noCost, gen.point(),
                   gen.addrSpace()->multithread_capable());
       REGISTER_CHECK(src);
       if (src != retReg) {
           // Move src to retReg. Can't simply return src, since it was not
           // allocated properly
           emitImm(orOp, src, 0, retReg, gen, noCost, gen.rs());
       }
       }
       break;
   case operandType::DataAddr:
       assert(oVar == NULL);
       addr = reinterpret_cast<Address>(oValue);
       emitVload(loadOp, addr, retReg, retReg, gen, noCost, NULL, size, gen.point(), gen.addrSpace());
       break;
   case operandType::FrameAddr:
       addr = (Address) oValue;
       temp = gen.rs()->allocateRegister(gen, noCost);
       emitVload(loadFrameRelativeOp, addr, temp, retReg, gen, noCost, gen.rs(),
               size, gen.point(), gen.addrSpace());
       gen.rs()->freeRegister(temp);
       break;
   case operandType::RegOffset:
       // Prepare offset from value in any general register (not just fp).
       // This AstNode holds the register number, and loperand holds offset.
       assert(operand_);
       addr = (Address) operand_->getOValue();
       emitVload(loadRegRelativeOp, addr, (long)oValue, retReg, gen, noCost,
               gen.rs(), size, gen.point(), gen.addrSpace());
       break;
   case operandType::ConstantString:
       // XXX This is for the std::string type.  If/when we fix the std::string type
       // to make it less of a hack, we'll need to change this.
       len = strlen((char *)oValue) + 1;

       addr = (Address) gen.addrSpace()->inferiorMalloc(len, dataHeap); //dataheap

       if (!gen.addrSpace()->writeDataSpace((char *)addr, len, (char *)oValue)) {
           ast_printf("Failed to write string constant into mutatee\n");
           return false;
       }

       if(!gen.addrSpace()->needsPIC())
       {
          emitVload(loadConstOp, addr, retReg, retReg, gen, noCost, gen.rs(),
                  size, gen.point(), gen.addrSpace());
       }
       else
       {
          gen.codeEmitter()->emitLoadShared(loadConstOp, retReg, NULL, true, size, gen, addr);
       }
       break;
   default:
       fprintf(stderr, "[%s:%d] ERROR: Unknown operand type %d in AstOperandNode generation\n",
               __FILE__, __LINE__, static_cast<int>(oType));
       return false;
       break;
   }
	decUseCount(gen);
   return true;
}

bool AstMemoryNode::generateCode_phase2(codeGen &gen, bool noCost,
                                        Address &,
                                        Dyninst::Register &retReg) {

	RETURN_KEPT_REG(retReg);

    const BPatch_memoryAccess* ma;
    const BPatch_addrSpec_NP *start;
    const BPatch_countSpec_NP *count;
    if (retReg == Dyninst::Null_Register)
        retReg = allocateAndKeep(gen, noCost);
    switch(mem_) {
    case EffectiveAddr: {

        // VG(11/05/01): get effective address
        // VG(07/31/02): take care which one
        // 1. get the point being instrumented & memory access info
        assert(gen.point());

        BPatch_addressSpace *bproc = (BPatch_addressSpace *)gen.addrSpace()->up_ptr();
        BPatch_point *bpoint = bproc->findOrCreateBPPoint(NULL, gen.point(), BPatch_point::convertInstPointType_t(gen.point()->type()));
        if (bpoint == NULL) {
            fprintf(stderr, "ERROR: Unable to find BPatch point for internal point %p/0x%lx\n",
                    (void*)gen.point(), gen.point()->insnAddr());
        }
        assert(bpoint);
        ma = bpoint->getMemoryAccess();
        if(!ma) {
            bpfatal( "Memory access information not available at this point.\n");
            bpfatal( "Make sure you create the point in a way that generates it.\n");
            bpfatal( "E.g.: findPoint(const std::set<BPatch_opCode>& ops).\n");
            assert(0);
        }
        if(which_ >= ma->getNumberOfAccesses()) {
            bpfatal( "Attempt to instrument non-existent memory access number.\n");
            bpfatal( "Consider using filterPoints()...\n");
            assert(0);
        }
        start = ma->getStartAddr(which_);
        emitASload(start, retReg, 0, gen, noCost);
        break;
    }
    case BytesAccessed: {
        // 1. get the point being instrumented & memory access info
        assert(gen.point());

        BPatch_addressSpace *bproc = (BPatch_addressSpace *)gen.addrSpace()->up_ptr();
        BPatch_point *bpoint = bproc->findOrCreateBPPoint(NULL, gen.point(), BPatch_point::convertInstPointType_t(gen.point()->type()));
        ma = bpoint->getMemoryAccess();
        if(!ma) {
            bpfatal( "Memory access information not available at this point.\n");
            bpfatal("Make sure you create the point in a way that generates it.\n");
            bpfatal( "E.g.: findPoint(const std::set<BPatch_opCode>& ops).\n");
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
    std::vector<AstNodePtr > kids;
    getChildren(kids);
    for (unsigned i = 0; i < kids.size(); i++) {
        if (!kids[i]->initRegisters(gen))
            ret = false;
    }

    if (callReplace_) return true;
    
    // We also need a function object.
    func_instance *callee = func_;
    if (!callee) {
        // Painful lookup time
        callee = gen.addrSpace()->findOnlyOneFunction(func_name_.c_str());
    }
    assert(callee);

    // Marks registers as used based on the callee's behavior
    // This means we'll save them if necessary (also, lets us use
    // them in our generated code because we've saved, instead
    // of saving others).
    assert(gen.codeEmitter());
    gen.codeEmitter()->clobberAllFuncCall(gen.rs(), callee);

    return ret;

}

bool AstCallNode::generateCode_phase2(codeGen &gen, bool noCost,
                                      Address &,
                                      Dyninst::Register &retReg) {
	// We call this anyway... not that we'll ever be kept.
	// Well... if we can somehow know a function is entirely
	// dependent on arguments (a flag?) we can keep it around.
	RETURN_KEPT_REG(retReg);

    // VG(11/06/01): This platform independent fn calls a platfrom
    // dependent fn which calls it back for each operand... Have to
    // fix those as well to pass location...

    func_instance *use_func = func_;

    if (!use_func && !func_addr_) {
        // We purposefully don't cache the func_instance object; the AST nodes
        // are process independent, and functions kinda are.
        use_func = gen.addrSpace()->findOnlyOneFunction(func_name_.c_str());
        if (!use_func) {
            fprintf(stderr, "ERROR: failed to find function %s, unable to create call\n",
                    func_name_.c_str());
        }
        assert(use_func); // Otherwise we've got trouble...
    }

    Dyninst::Register tmp = 0;

    if (use_func && !callReplace_) {
        tmp = emitFuncCall(callOp, gen, args_,
                           noCost, use_func);
    }
    else if (use_func && callReplace_) {
	tmp = emitFuncCall(funcJumpOp, gen, args_,
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
    if (tmp == Dyninst::Null_Register) {
        // Happens in function replacement... didn't allocate
        // a return register.
    }
    else if (retReg == Dyninst::Null_Register) {
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
                                          Dyninst::Register &retReg) {
    RETURN_KEPT_REG(retReg);
    Dyninst::Register tmp = Dyninst::Null_Register;
    Address unused = ADDR_NULL;

    if (sequence_.size() == 0) {
      // Howzat???
      return true;
    }

    for (unsigned i = 0; i < sequence_.size() - 1; i++) {
      if (!sequence_[i]->generateCode_phase2(gen,
                                               noCost,
                                               unused,
                                               tmp)) ERROR_RETURN;
        if (sequence_[i]->decRefCount())
           gen.rs()->freeRegister(tmp);
        tmp = Dyninst::Null_Register;
    }

    // We keep the last one
    if (!sequence_.back()->generateCode_phase2(gen, noCost, unused, retReg)) ERROR_RETURN;

	decUseCount(gen);

    return true;
}

bool AstVariableNode::generateCode_phase2(codeGen &gen, bool noCost,
                                          Address &addr,
                                          Dyninst::Register &retReg) {
    return ast_wrappers_[index]->generateCode_phase2(gen, noCost, addr, retReg);
}

bool AstOriginalAddrNode::generateCode_phase2(codeGen &gen,
                                              bool noCost,
                                              Address &,
                                              Dyninst::Register &retReg) {
    RETURN_KEPT_REG(retReg);
    if (retReg == Dyninst::Null_Register) {
        retReg = allocateAndKeep(gen, noCost);
    }
    if (retReg == Dyninst::Null_Register) return false;

    emitVload(loadConstOp,
              (Address) gen.point()->addr_compat(),
              retReg, retReg, gen, noCost);
    return true;
}

bool AstActualAddrNode::generateCode_phase2(codeGen &gen,
                                            bool noCost,
                                            Address &,
                                            Dyninst::Register &retReg) {
    if (retReg == Dyninst::Null_Register) {
        retReg = allocateAndKeep(gen, noCost);
    }
    if (retReg == Dyninst::Null_Register) return false;

    emitVload(loadConstOp,
              (Address) gen.currAddr(),
              retReg, retReg,
              gen, noCost);

    return true;
}

bool AstDynamicTargetNode::generateCode_phase2(codeGen &gen,
                                            bool noCost,
                                            Address & retAddr,
                                            Dyninst::Register &retReg)
{
    if (gen.point()->type() != instPoint::PreCall &&
       gen.point()->type() != instPoint::FuncExit &&
       gen.point()->type() != instPoint::PreInsn)
       return false;

   InstructionAPI::Instruction insn = gen.point()->block()->getInsn(gen.point()->block()->last());
   if (insn.getCategory() == c_ReturnInsn) {
      // if this is a return instruction our AST reads the top stack value
      if (retReg == Dyninst::Null_Register) {
         retReg = allocateAndKeep(gen, noCost);
      }
      if (retReg == Dyninst::Null_Register) return false;

#if defined (arch_x86)
        emitVload(loadRegRelativeOp,
                  (Address)0,
                  REGNUM_ESP,
                  retReg,
                  gen, noCost);
#elif defined (arch_x86_64)
        emitVload(loadRegRelativeOp,
                  (Address)0,
                  REGNUM_RSP,
                  retReg,
                  gen, noCost);
#elif defined (arch_power) // KEVINTODO: untested
        emitVload(loadRegRelativeOp,
                  (Address) sizeof(Address),
                  REG_SP,
                  retReg,
                  gen, noCost);
#elif defined (arch_aarch64)
			//#warning "This function is not implemented yet!"
			assert(0);
#else
        assert(0);
#endif
      return true;
   }
   else {// this is a dynamic ctrl flow instruction, have
      // getDynamicCallSiteArgs generate the necessary AST
      std::vector<AstNodePtr> args;
      if (!gen.addrSpace()->getDynamicCallSiteArgs(insn, gen.point()->block()->last(), args)) {
         return false;
      }
      if (!args[0]->generateCode_phase2(gen, noCost, retAddr, retReg)) {
         return false;
      }
      return true;
   }
}

bool AstScrambleRegistersNode::generateCode_phase2(codeGen &gen,
 						  bool ,
						  Address&,
						  Dyninst::Register& )
{
   (void)gen; // unused
#if defined(arch_x86_64)
   for (int i = 0; i < gen.rs()->numGPRs(); i++) {
      registerSlot *reg = gen.rs()->GPRs()[i];
      if (reg->encoding() != REGNUM_RBP && reg->encoding() != REGNUM_RSP)
          gen.codeEmitter()->emitLoadConst(reg->encoding() , -1, gen);
   }
#endif
   return true;
}

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
    if (oType == operandType::Constant) {
        total = getInsnCost(loadConstOp);
    } else if (oType == operandType::DataIndir) {
        total = getInsnCost(loadIndirOp);
        total += operand()->costHelper(costStyle);
    } else if (oType == operandType::DataAddr) {
        total = getInsnCost(loadOp);
    } else if (oType == operandType::DataReg) {
        total = getInsnCost(loadIndirOp);
    } else if (oType == operandType::Param || oType == operandType::ParamAtCall || oType == operandType::ParamAtEntry) {
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

int AstVariableNode::costHelper(enum CostStyleType /*costStyle*/) const{
    int total = 0;
    return total;
}

BPatch_type *AstNode::checkType(BPatch_function*) {
    return BPatch::bpatch->type_Untyped;
}

BPatch_type *AstOperatorNode::checkType(BPatch_function* func) {
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

    if (loperand) lType = loperand->checkType(func);

    if (roperand) rType = roperand->checkType(func);

    if (eoperand) eType = eoperand->checkType(func);
    (void)eType; // unused...

    if (lType == BPatch::bpatch->type_Error ||
        rType == BPatch::bpatch->type_Error)
       errorFlag = true;

    switch (op) {
    case ifOp:
    case whileOp:
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
                fprintf(stderr, "WARNING: LHS type %s not compatible with RHS type %s\n",
                        lType->getName(), rType->getName());
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

    // remember what type we are
    setType(ret);

    return ret;
}

BPatch_type *AstOperandNode::checkType(BPatch_function* func)
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

    if (operand_) type = operand_->checkType(func);

    if (type == BPatch::bpatch->type_Error)
       errorFlag = true;

    if (oType == operandType::DataIndir) {
        // XXX Should really be pointer to lType -- jkh 7/23/99
        ret = BPatch::bpatch->type_Untyped;
    }
    else if ((oType == operandType::Param) || (oType == operandType::ParamAtCall) ||
             (oType == operandType::ParamAtEntry) || (oType == operandType::ReturnVal)
             || (oType == operandType::ReturnAddr)) {
      if(func)
      {
	switch(oType)
	{
	case operandType::ReturnVal:
	  {
	    ret = func->getReturnType();
	    if(!ret || (ret->isCompatible(BPatch::bpatch->builtInTypes->findBuiltInType("void")))) {
		  if(ret) {
		      errorFlag = true;
		  }
	      ret = BPatch::bpatch->type_Untyped;
	    }
	    break;
	  }
	default:
	  ret = BPatch::bpatch->type_Untyped;
	}

      }
      else
      {
	// If we don't have a function context, then ignore types
        ret = BPatch::bpatch->type_Untyped;
      }
    }
    else if (oType == operandType::origRegister) {
        ret = BPatch::bpatch->type_Untyped;
    }
    else {
        ret = const_cast<BPatch_type *>(getType());
    }
    assert(ret != NULL);

    if (errorFlag && doTypeCheck) {
       ret = BPatch::bpatch->type_Error;
    } else if (errorFlag) {
       ret = BPatch::bpatch->type_Untyped;
    }

    // remember what type we are
    setType(ret);

    return ret;

}


BPatch_type *AstCallNode::checkType(BPatch_function* func) {
    BPatch_type *ret = NULL;
    bool errorFlag = false;

    assert(BPatch::bpatch != NULL);	/* We'll use this later. */

    unsigned i;
    for (i = 0; i < args_.size(); i++) {
        BPatch_type *opType = args_[i]->checkType(func);
        /* XXX Check operands for compatibility */
        if (opType == BPatch::bpatch->type_Error) {
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

    // remember what type we are
    setType(ret);

    return ret;
}

BPatch_type *AstSequenceNode::checkType(BPatch_function* func) {
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
        sType = sequence_[i]->checkType(func);
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

    // remember what type we are
    setType(ret);

    return ret;
}

bool AstNode::accessesParam() {
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

bool AstVariableNode::accessesParam() {
    return ast_wrappers_[index]->accessesParam();
}

// Our children may have incorrect useCounts (most likely they
// assume that we will not bother them again, which is wrong)
void AstNode::fixChildrenCounts()
{
    std::vector<AstNodePtr> children;
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
    case xorOp:
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
    case operandType::DataReg:
    case operandType::DataIndir:
    case operandType::RegOffset:
    case operandType::origRegister:
    case operandType::DataAddr:
    case operandType::variableValue:
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
                fprintf(stderr, "AST %p: labelled const func but argument %u cannot be kept!\n",
                        (const void*)this, i);
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

bool AstVariableNode::canBeKept() const {
    return ast_wrappers_[index]->canBeKept();
}

bool AstMiniTrampNode::canBeKept() const {
	// Well... depends on the actual AST, doesn't it.
	assert(ast_);

	return ast_->canBeKept();
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

void AstNode::getChildren(std::vector<AstNodePtr > &) {
#if 0
    fprintf(stderr, "Undefined call to getChildren for type: ");
    if (dynamic_cast<AstNullNode *>(this)) fprintf(stderr, "nullNode\n");
    else if (dynamic_cast<AstOperatorNode *>(this)) fprintf(stderr, "operatorNode\n");
    else if (dynamic_cast<AstOperandNode *>(this)) fprintf(stderr, "operandNode\n");
    else if (dynamic_cast<AstCallNode *>(this)) fprintf(stderr, "callNode\n");
    else if (dynamic_cast<AstSequenceNode *>(this)) fprintf(stderr, "seqNode\n");
    else if (dynamic_cast<AstInsnNode *>(this)) fprintf(stderr, "insnNode\n");
    else if (dynamic_cast<AstMiniTrampNode *>(this)) fprintf(stderr, "miniTrampNode\n");
    else if (dynamic_cast<AstMemoryNode *>(this)) fprintf(stderr, "memoryNode\n");
    else fprintf(stderr, "unknownNode\n");
#endif
}

void AstNode::setChildren(std::vector<AstNodePtr > &) {
#if 0
    fprintf(stderr, "Undefined call to setChildren for type: ");
    if (dynamic_cast<AstNullNode *>(this)) fprintf(stderr, "nullNode\n");
    else if (dynamic_cast<AstOperatorNode *>(this)) fprintf(stderr, "operatorNode\n");
    else if (dynamic_cast<AstOperandNode *>(this)) fprintf(stderr, "operandNode\n");
    else if (dynamic_cast<AstCallNode *>(this)) fprintf(stderr, "callNode\n");
    else if (dynamic_cast<AstSequenceNode *>(this)) fprintf(stderr, "seqNode\n");
    else if (dynamic_cast<AstInsnNode *>(this)) fprintf(stderr, "insnNode\n");
    else if (dynamic_cast<AstMiniTrampNode *>(this)) fprintf(stderr, "miniTrampNode\n");
    else if (dynamic_cast<AstMemoryNode *>(this)) fprintf(stderr, "memoryNode\n");
    else fprintf(stderr, "unknownNode\n");
#endif
}

void AstOperatorNode::getChildren(std::vector<AstNodePtr > &children) {
    if (loperand) children.push_back(loperand);
    if (roperand) children.push_back(roperand);
    if (eoperand) children.push_back(eoperand);
}

void AstOperatorNode::setChildren(std::vector<AstNodePtr > &children){
   int count = (loperand ? 1 : 0) + (roperand ? 1 : 0) + (eoperand ? 1 : 0);
   if ((int)children.size() == count){
      //memory management?
      if (loperand) loperand = children[0];
      if (roperand) roperand = children[1];
      if (eoperand) eoperand = children[2];
   }else{
      fprintf(stderr, "OPERATOR setChildren given bad arguments. Wanted:%d , given:%d\n", count, (int)children.size());
   }
}

AstNodePtr AstOperatorNode::deepCopy(){
   AstNodePtr copy = operatorNode(op, (loperand ? loperand->deepCopy() : loperand),
                                  (roperand ? roperand->deepCopy() : roperand),
                                  (eoperand ? eoperand->deepCopy() : eoperand));
   copy->setType(bptype);
   copy->setTypeChecking(doTypeCheck);

   copy->setLineNum(getLineNum());
   copy->setColumnNum(getColumnNum());
   copy->setSnippetName(snippetName);

/* TODO: Impliment this copy.
   copy->columnInfoSet = columnInfoSet
   copy->lineInfoSet = lineInfoSet;
*/
   return copy;
}

void AstOperandNode::getChildren(std::vector<AstNodePtr > &children) {
    if (operand_) children.push_back(operand_);
}

void AstOperandNode::setChildren(std::vector<AstNodePtr > &children){
   if (children.size() == 1){
      //memory management?
      operand_ = children[0];
   }else{
      fprintf(stderr, "OPERAND setChildren given bad arguments. Wanted:%d , given:%d\n", 1,  (int)children.size());
   }
}

AstNodePtr AstOperandNode::deepCopy(){
   AstOperandNode * copy = new AstOperandNode();
   copy->oType = oType;
   copy->oValue = oValue; //this might need to be copied deeper
   copy->oVar = oVar;
   if(operand_) copy->operand_ = operand_->deepCopy();

   copy->setType(bptype);
   copy->setTypeChecking(doTypeCheck);

   copy->setLineNum(getLineNum());
   copy->lineInfoSet = lineInfoSet;
   copy->setColumnNum(getColumnNum());
   copy->columnInfoSet = columnInfoSet;
   copy->setSnippetName(getSnippetName());
   return AstNodePtr(copy);
}

void AstCallNode::getChildren(std::vector<AstNodePtr > &children) {
    for (unsigned i = 0; i < args_.size(); i++)
        children.push_back(args_[i]);
}

void AstCallNode::setChildren(std::vector<AstNodePtr > &children){
   if (children.size() == args_.size()){
      //memory management?
      for (unsigned i = 0; i < args_.size(); i++){
         AstNodePtr * newNode = new AstNodePtr(children[i]);
         args_.push_back(*newNode);
         args_.erase(args_.begin() + i + 1);
      }
   }else{
      fprintf(stderr, "CALL setChildren given bad arguments. Wanted:%d , given:%d\n",  (int)args_.size(),  (int)children.size());
   }
}

AstNodePtr AstCallNode::deepCopy(){
   std::vector<AstNodePtr> empty_args;

   AstCallNode * copy;

   if(func_name_.empty()){
      copy = new AstCallNode();
   }else{
      copy = new AstCallNode(func_name_, empty_args);
   }
//   copy->func_name_ = func_name_;
   copy->func_addr_ = func_addr_;
   copy->func_ = func_;

   for(unsigned int i = 0; i < args_.size(); ++i){
      copy->args_.push_back(args_[i]->deepCopy());
   }

   copy->callReplace_ = callReplace_;
   copy->constFunc_ = constFunc_;

   copy->setType(bptype);
   copy->setTypeChecking(doTypeCheck);

   copy->setLineNum(getLineNum());
   copy->lineInfoSet = lineInfoSet;
   copy->setColumnNum(getColumnNum());
   copy->columnInfoSet = columnInfoSet;
   copy->setSnippetName(getSnippetName());
   copy->snippetNameSet = snippetNameSet;

   return AstNodePtr(copy);
}

void AstSequenceNode::getChildren(std::vector<AstNodePtr > &children) {
    for (unsigned i = 0; i < sequence_.size(); i++)
        children.push_back(sequence_[i]);
}

void AstSequenceNode::setChildren(std::vector<AstNodePtr > &children){
   if (children.size() == sequence_.size()){
      //memory management?
      for (unsigned i = 0; i < sequence_.size(); i++){
         AstNodePtr * newNode = new AstNodePtr(children[i]);
         sequence_.push_back(*newNode);
         sequence_.erase(sequence_.begin() + i + 1);
      }
   }else{
      fprintf(stderr, "SEQ setChildren given bad arguments. Wanted:%d , given:%d\n", (int)sequence_.size(),  (int)children.size());
   }
}

AstNodePtr AstSequenceNode::deepCopy(){
   AstSequenceNode * copy = new AstSequenceNode();
   for(unsigned int i = 0; i < sequence_.size(); ++i){
      copy->sequence_.push_back(sequence_[i]->deepCopy());
   }

   copy->setType(bptype);
   copy->setTypeChecking(doTypeCheck);

   copy->setLineNum(getLineNum());
   copy->lineInfoSet = lineInfoSet;
   copy->setColumnNum(getColumnNum());
   copy->columnInfoSet = columnInfoSet;
   copy->setSnippetName(getSnippetName());
   copy->snippetNameSet = snippetNameSet;

   return AstNodePtr(copy);
}

void AstVariableNode::getChildren(std::vector<AstNodePtr > &children) {
    ast_wrappers_[index]->getChildren(children);
}

void AstVariableNode::setChildren(std::vector<AstNodePtr > &children){
   ast_wrappers_[index]->setChildren(children);
}

AstNodePtr AstVariableNode::deepCopy(){
   AstVariableNode * copy = new AstVariableNode();
   copy->index = index;
   copy->ranges_ = ranges_; //i'm not sure about this one. (it's a vector)
   for(unsigned int i = 0; i < ast_wrappers_.size(); ++i){
      copy->ast_wrappers_.push_back(ast_wrappers_[i]->deepCopy());
   }

   copy->setType(bptype);
   copy->setTypeChecking(doTypeCheck);

   copy->setLineNum(getLineNum());
   copy->lineInfoSet = lineInfoSet;
   copy->setColumnNum(getColumnNum());
   copy->columnInfoSet = columnInfoSet;
   copy->setSnippetName(getSnippetName());
   copy->snippetNameSet = snippetNameSet;

   return AstNodePtr(copy);
}

void AstMiniTrampNode::getChildren(std::vector<AstNodePtr > &children) {
    children.push_back(ast_);
}

void AstMiniTrampNode::setChildren(std::vector<AstNodePtr > &children){
   if (children.size() == 1){
      //memory management?
      ast_ = children[0];
   }else{
 fprintf(stderr, "MINITRAMP setChildren given bad arguments. Wanted:%d , given:%d\n", 1,  (int)children.size());
   }
}

AstNodePtr AstMiniTrampNode::deepCopy(){
   AstMiniTrampNode * copy = new AstMiniTrampNode();
   copy->inline_ = inline_;
   copy->ast_ = ast_->deepCopy();

   copy->setType(bptype);
   copy->setTypeChecking(doTypeCheck);

   copy->setLineNum(getLineNum());
   copy->lineInfoSet = lineInfoSet;
   copy->setColumnNum(getColumnNum());
   copy->columnInfoSet = columnInfoSet;
   copy->setSnippetName(getSnippetName());
   copy->snippetNameSet = snippetNameSet;

   return AstNodePtr(copy);
}


void AstOperatorNode::setVariableAST(codeGen &g) {
    if(loperand) loperand->setVariableAST(g);
    if(roperand) roperand->setVariableAST(g);
    if(eoperand) eoperand->setVariableAST(g);
}

void AstOperandNode::setVariableAST(codeGen &g){
    if(operand_) operand_->setVariableAST(g);
}

void AstCallNode::setVariableAST(codeGen &g){
    for (unsigned i = 0; i < args_.size(); i++)
        args_[i]->setVariableAST(g);
}

void AstSequenceNode::setVariableAST(codeGen &g) {
    for (unsigned i = 0; i < sequence_.size(); i++)
        sequence_[i]->setVariableAST(g);
}

void AstVariableNode::setVariableAST(codeGen &gen){
    //fprintf(stderr, "Generating code for variable in function %s with start address 0x%lx at address 0x%lx\n",gen.func()->prettyName().c_str(), gen.func()->getAddress(),gen.point()->addr());
    if(!ranges_)
        return;
    if(!gen.point())    //oneTimeCode. Set the AST at the beginning of the function??
    {
        index = 0;
        return;
    }
    Address addr = gen.point()->addr_compat();     //Dyninst::Offset of inst point from function base address
    bool found = false;
    for(unsigned i=0; i< ranges_->size();i++){
       if((*ranges_)[i].first<=addr && addr<=(*ranges_)[i].second) {
          index = i;
          found = true;
       }
    }
    if (!found) {
       cerr << "Error: unable to find AST representing variable at " << hex << addr << dec << endl;
       cerr << "Pointer " << hex << this << dec << endl;
       cerr << "Options are: " << endl;
       for(unsigned i=0; i< ranges_->size();i++){
          cerr << "\t" << hex << (*ranges_)[i].first << "-" << (*ranges_)[i].second << dec << endl;
       }
    }
    assert(found);
}

void AstMiniTrampNode::setVariableAST(codeGen &g){
    if(ast_) ast_->setVariableAST(g);
}

bool AstCallNode::containsFuncCall() const {
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

bool AstVariableNode::containsFuncCall() const
{
    return ast_wrappers_[index]->containsFuncCall();
}

bool AstNullNode::containsFuncCall() const
{
   return false;
}

bool AstStackInsertNode::containsFuncCall() const
{
    return false;
}

bool AstStackRemoveNode::containsFuncCall() const
{
    return false;
}

bool AstStackGenericNode::containsFuncCall() const
{
    return false;
}

bool AstLabelNode::containsFuncCall() const
{
   return false;
}

bool AstMemoryNode::containsFuncCall() const
{
   return false;
}

bool AstOriginalAddrNode::containsFuncCall() const
{
   return false;
}

bool AstActualAddrNode::containsFuncCall() const
{
   return false;
}

bool AstDynamicTargetNode::containsFuncCall() const
{
   return false;
}
bool AstScrambleRegistersNode::containsFuncCall() const
{
   return false;
}

bool AstCallNode::usesAppRegister() const {
   for (unsigned i=0; i<args_.size(); i++) {
      if (args_[i] && args_[i]->usesAppRegister()) return true;
   }
   return false;
}

bool AstOperatorNode::usesAppRegister() const {
	if (loperand && loperand->usesAppRegister()) return true;
	if (roperand && roperand->usesAppRegister()) return true;
	if (eoperand && eoperand->usesAppRegister()) return true;
	return false;
}

bool AstOperandNode::usesAppRegister() const {
   if (oType == AstNode::operandType::FrameAddr ||
       oType == AstNode::operandType::RegOffset ||
       oType == AstNode::operandType::origRegister ||
       oType == AstNode::operandType::Param ||
       oType == AstNode::operandType::ParamAtEntry ||
       oType == AstNode::operandType::ParamAtCall ||
       oType == AstNode::operandType::ReturnVal)
   {
      return true;
   }

   if (operand_ && operand_->usesAppRegister()) return true;
   return false;
}

bool AstMiniTrampNode::usesAppRegister() const {
	if (ast_ && ast_->usesAppRegister()) return true;
	return false;
}

bool AstSequenceNode::usesAppRegister() const {
	for (unsigned i = 0; i < sequence_.size(); i++) {
		if (sequence_[i]->usesAppRegister()) return true;
	}
	return false;
}

bool AstVariableNode::usesAppRegister() const
{
    return ast_wrappers_[index]->usesAppRegister();
}

bool AstNullNode::usesAppRegister() const
{
   return false;
}

bool AstStackInsertNode::usesAppRegister() const
{
    return false;
}

bool AstStackRemoveNode::usesAppRegister() const
{
    return false;
}

bool AstStackGenericNode::usesAppRegister() const
{
    return false;
}

bool AstLabelNode::usesAppRegister() const
{
   return false;
}

bool AstDynamicTargetNode::usesAppRegister() const
{
   return false;
}

bool AstActualAddrNode::usesAppRegister() const
{
   return false;
}

bool AstOriginalAddrNode::usesAppRegister() const
{
   return false;
}

bool AstMemoryNode::usesAppRegister() const
{
   return true;
}

bool AstScrambleRegistersNode::usesAppRegister() const
{
   return true;
}

void regTracker_t::addKeptRegister(codeGen &gen, AstNode *n, Dyninst::Register reg) {
	assert(n);
	if (tracker.find(n) != tracker.end()) {
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
   auto iter = tracker.find(n);
   if (iter == tracker.end()) return;

   gen.rs()->unKeepRegister(iter->second.keptRegister);
   tracker.erase(iter);
}

Dyninst::Register regTracker_t::hasKeptRegister(AstNode *n) {
   auto iter = tracker.find(n);
   if (iter == tracker.end())
      return Dyninst::Null_Register;
   else return iter->second.keptRegister;
}

// Find if the given register is "owned" by an AST node,
// and if so nuke it.

bool regTracker_t::stealKeptRegister(Dyninst::Register r) {
	ast_printf("STEALING kept register %u for someone else\n", r);
        for (auto iter = tracker.begin(); iter != tracker.end(); ++iter) {
           if (iter->second.keptRegister == r) {
              tracker.erase(iter);
              return true;
           }
        }
	fprintf(stderr, "Odd - couldn't find kept register %u\n", r);
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

void regTracker_t::decreaseAndClean(codeGen &) {

    assert(condLevel > 0);

    ast_printf("Exiting from conditional branch, level currently %d\n", condLevel);

    std::vector<AstNode *> delete_list;

    for (auto iter = tracker.begin(); iter != tracker.end();) {
       if (iter->second.keptLevel == condLevel) {
          iter = tracker.erase(iter);
       }
       else {
          ++iter;
       }
    }

    condLevel--;
}

unsigned regTracker_t::astHash(AstNode* const &ast) {
	return addrHash4((Address) ast);
}

void regTracker_t::debugPrint() {
    if (!dyn_debug_ast) return;

    fprintf(stderr, "==== Begin debug dump of register tracker ====\n");

    fprintf(stderr, "Condition level: %d\n", condLevel);

    for (auto iter = tracker.begin(); iter != tracker.end(); ++iter) {
       fprintf(stderr, "AstNode %p: register %u, condition level %d\n",
               (void*)iter->first, iter->second.keptRegister, iter->second.keptLevel);
    }
    fprintf(stderr, "==== End debug dump of register tracker ====\n");
}

unsigned AstNode::getTreeSize() {
	std::vector<AstNodePtr > children;
	getChildren(children);

	unsigned size_ = 1; // Us
	for (unsigned i = 0; i < children.size(); i++)
		size_ += children[i]->getTreeSize();
	return size_;

}

int_variable* AstOperandNode::lookUpVar(AddressSpace* as)
{
  mapped_module *mod = as->findModule(oVar->pdmod()->fileName());
  if(mod && mod->obj())// && (oVar->pdmod() == mod->pmod()))
  {
      int_variable* tmp = mod->obj()->findVariable(const_cast<image_variable*>(oVar));
      return tmp;
  }
  return NULL;
}

void AstOperandNode::emitVariableLoad(opCode op, Dyninst::Register src2, Dyninst::Register dest, codeGen& gen,
				      bool noCost, registerSpace* rs,
				      int size_, const instPoint* point, AddressSpace* as)
{
  int_variable* var = lookUpVar(as);
  if(var && !as->needsPIC(var))
  {
    emitVload(op, var->getAddress(), src2, dest, gen, noCost, rs, size_, point, as);
  }
  else
  {
    gen.codeEmitter()->emitLoadShared(op, dest, oVar, (var!=NULL),size_, gen, 0);
  }
}

void AstOperandNode::emitVariableStore(opCode op, Dyninst::Register src1, Dyninst::Register src2, codeGen& gen,
				      bool noCost, registerSpace* rs,
				      int size_, const instPoint* point, AddressSpace* as)
{
  int_variable* var = lookUpVar(as);
  if (var && !as->needsPIC(var))
  {
    emitVstore(op, src1, src2, var->getAddress(), gen, noCost, rs, size_, point, as);
  }
  else
  {
    gen.codeEmitter()->emitStoreShared(src1, oVar, (var!=NULL), size_, gen);
  }
}

bool AstNode::decRefCount()
{
   referenceCount--;
   //return referenceCount <= 0;
   return true;
}

bool AstNode::generate(Point *point, Buffer &buffer) {
   // For now, be really inefficient. Yay!
   codeGen gen(1024);
   instPoint *ip = IPCONV(point);

   gen.setPoint(ip);
   gen.setRegisterSpace(registerSpace::actualRegSpace(ip));
   gen.setAddrSpace(ip->proc());
   if (!generateCode(gen, false)) return false;

   unsigned char *start_ptr = (unsigned char *)gen.start_ptr();
   unsigned char *cur_ptr = (unsigned char *)gen.cur_ptr();
   buffer.copy(start_ptr, cur_ptr);

   return true;
}

bool AstSnippetNode::generateCode_phase2(codeGen &gen,
                                         bool,
                                         Address &,
                                         Dyninst::Register &) {
   Buffer buf(gen.currAddr(), 1024);
   if (!snip_->generate(gen.point(), buf)) return false;
   gen.copy(buf.start_ptr(), buf.size());
   return true;
}

std::string AstNode::format(std::string indent) {
   std::stringstream ret;
   ret << indent << "Default/" << hex << this << dec << "()" << endl;
   return ret.str();
}

std::string AstNullNode::format(std::string indent) {
   std::stringstream ret;
   ret << indent << "Null/" << hex << this << dec << "()" << endl;
   return ret.str();
}

std::string AstStackInsertNode::format(std::string indent) {
    std::stringstream ret;
    ret << indent << "StackInsert/" << hex << this;
    ret << "(size " << size << ")";
    if (type == CANARY_AST) ret << " (is canary)";
    ret << endl;
    return ret.str();
}

std::string AstStackRemoveNode::format(std::string indent) {
    std::stringstream ret;
    ret << indent << "StackRemove/" << hex << this;
    ret << "(size " << size << ")";
    if (type == CANARY_AST) ret << "(is canary)";
    ret << endl;
    return ret.str();
}

std::string AstStackGenericNode::format(std::string indent) {
    std::stringstream ret;
    ret << indent << "StackGeneric/" << hex << this;
    ret << endl;
    return ret.str();
}

std::string AstOperatorNode::format(std::string indent) {
   std::stringstream ret;
   ret << indent << "Op/" << hex << this << dec << "(" << convert(op) << ")" << endl;
   if (loperand) ret << indent << loperand->format(indent + "  ");
   if (roperand) ret << indent << roperand->format(indent + "  ");
   if (eoperand) ret << indent << eoperand->format(indent + "  ");

   return ret.str();
}

std::string AstOperandNode::format(std::string indent) {
   std::stringstream ret;
   ret << indent << "Oper/" << hex << this << dec << "(" << convert(oType) << "/" << oValue << ")" << endl;
   if (operand_) ret << indent << operand_->format(indent + "  ");

   return ret.str();
}


std::string AstCallNode::format(std::string indent) {
   std::stringstream ret;
   ret << indent << "Call/" << hex << this << dec;
   if (func_) ret << "(" << func_->name() << ")";
   else ret << "(" << func_name_ << ")";
   ret << endl;
   indent += "  ";
   for (unsigned i = 0; i < args_.size(); ++i) {
      ret << indent << args_[i]->format(indent + "  ");
   }

   return ret.str();
}

std::string AstSequenceNode::format(std::string indent) {
   std::stringstream ret;
   ret << indent << "Seq/" << hex << this << dec << "()" << endl;
   for (unsigned i = 0; i < sequence_.size(); ++i) {
      ret << indent << sequence_[i]->format(indent + "  ");
   }
   return ret.str();
}


std::string AstVariableNode::format(std::string indent) {
   std::stringstream ret;
   ret << indent << "Var/" << hex << this << dec << "(" << ast_wrappers_.size() << ")" << endl;
   for (unsigned i = 0; i < ast_wrappers_.size(); ++i) {
      ret << indent << ast_wrappers_[i]->format(indent + "  ");
   }

   return ret.str();
}

std::string AstMemoryNode::format(std::string indent) {
   std::stringstream ret;
   ret << indent << "Mem/" << hex << this << dec << "("
       << ((mem_ == EffectiveAddr) ? "EffAddr" : "BytesAcc")
       << ")" << endl;

   return ret.str();
}

std::string AstNode::convert(operandType type) {
   switch(type) {
      case operandType::Constant: return "Constant";
      case operandType::ConstantString: return "ConstantString";
      case operandType::DataReg: return "DataReg";
      case operandType::DataIndir: return "DataIndir";
      case operandType::Param: return "Param";
      case operandType::ParamAtCall: return "ParamAtCall";
      case operandType::ParamAtEntry: return "ParamAtEntry";
      case operandType::ReturnVal: return "ReturnVal";
      case operandType::ReturnAddr: return "ReturnAddr";
      case operandType::DataAddr: return "DataAddr";
      case operandType::FrameAddr: return "FrameAddr";
      case operandType::RegOffset: return "RegOffset";
      case operandType::origRegister: return "OrigRegister";
      case operandType::variableAddr: return "variableAddr";
      case operandType::variableValue: return "variableValue";
      default: return "UnknownOperand";
   }
}

std::string AstNode::convert(opCode op) {
   switch(op) {
      case invalidOp: return "invalid";
      case plusOp: return "plus";
      case minusOp: return "minus";
      case xorOp: return "xor";
      case timesOp: return "times";
      case divOp: return "div";
      case lessOp: return "less";
      case leOp: return "le";
      case greaterOp: return "greater";
      case geOp: return "ge";
      case eqOp: return "equal";
      case neOp: return "ne";
      case loadOp: return "loadOp";
      case loadConstOp: return "loadConstOp";
      case loadFrameRelativeOp: return "loadFrameRelativeOp";
      case loadFrameAddr: return "loadFrameAddr";
      case loadRegRelativeOp: return "loadRegRelativeOp";
      case loadRegRelativeAddr: return "loadRegRelativeAddr";
      case storeOp: return "storeOp";
      case storeFrameRelativeOp: return "storeFrameRelativeOp";
      case ifOp: return "if";
      case whileOp: return "while";
      case doOp: return "do";
      case callOp: return "call";
      case noOp: return "no";
      case orOp: return "or";
      case andOp: return "and";
      case getRetValOp: return "getRetValOp";
      case getRetAddrOp: return "getRetAddrOp";
      case getSysRetValOp: return "getSysRetValOp";
      case getParamOp: return "getParamOp";
      case getParamAtCallOp: return "getParamAtCallOp";
      case getParamAtEntryOp: return "getParamAtEntryOp";
      case getSysParamOp: return "getSysParamOp";
      case getAddrOp: return "getAddrOp";
      case loadIndirOp: return "loadIndirOp";
      case storeIndirOp: return "storeIndirOp";
      case saveRegOp: return "saveRegOp";
      case loadRegOp: return "loadRegOp";
      case saveStateOp: return "saveStateOp";
      case loadStateOp: return "loadStateOp";
      case funcJumpOp: return "funcJump";
      case funcCallOp: return "funcCall";
      case branchOp: return "branch";
      case ifMCOp: return "ifMC";
      case breakOp: return "break";
      default: return "UnknownOp";
   }
}

bool AstOperandNode::initRegisters(codeGen &g) {
    bool ret = true;
    std::vector<AstNodePtr> kids;
    getChildren(kids);
    for (unsigned i = 0; i < kids.size(); i++) {
        if (!kids[i]->initRegisters(g))
            ret = false;
    }

    // If we're an origRegister, override its state as live.
    if (oType == operandType::origRegister) {
       Address origReg = (Address) oValue;
       // Mark that register as live so we are sure to save it.
       registerSlot *r = (*(g.rs()))[origReg];
       r->liveState = registerSlot::live;
    }

    return ret;
}
