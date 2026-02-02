#include "addressSpace.h"
#include "ast_helpers.h"
#include "AstNode.h"
#include "BPatch.h"
#include "BPatch_type.h"
#include "Buffer.h"
#include "debug.h"
#include "Point.h"
#include "registerSpace.h"

AstNodePtr AstNode::originalAddrNode_ = AstNodePtr();
AstNodePtr AstNode::actualAddrNode_ = AstNodePtr();
AstNodePtr AstNode::dynamicTargetNode_ = AstNodePtr();

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

AstNodePtr AstNode::stackRemoveNode(int size, MSpecialType type, func_instance *func,
                                    bool canaryAfterPrologue, long canaryHeight) {
  return AstNodePtr(new AstStackRemoveNode(size, type, func, canaryAfterPrologue, canaryHeight));
}

AstNodePtr AstNode::stackGenericNode() {
  return AstNodePtr(new AstStackGenericNode());
}

AstNodePtr AstNode::operandNode(operandType ot, void *arg) {
  return AstNodePtr(new AstOperandNode(ot, arg));
}

// TODO: this is an indirect load; should be an operator.
AstNodePtr AstNode::operandNode(operandType ot, AstNodePtr ast) {
  return AstNodePtr(new AstOperandNode(ot, ast));
}

AstNodePtr AstNode::operandNode(operandType ot, const image_variable *iv) {
  return AstNodePtr(new AstOperandNode(ot, iv));
}

AstNodePtr AstNode::sequenceNode(std::vector<AstNodePtr> &sequence) {
  return AstNodePtr(new AstSequenceNode(sequence));
}

AstNodePtr AstNode::variableNode(std::vector<AstNodePtr> &ast_wrappers,
                                 std::vector<std::pair<Dyninst::Offset, Dyninst::Offset>> *ranges) {
  return AstNodePtr(new AstVariableNode(ast_wrappers, ranges));
}

AstNodePtr AstNode::operatorNode(opCode ot, AstNodePtr l, AstNodePtr r, AstNodePtr e) {
  return AstNodePtr(new AstOperatorNode(ot, l, r, e));
}

AstNodePtr AstNode::funcCallNode(const std::string &func, std::vector<AstNodePtr> &args,
                                 AddressSpace *addrSpace) {
  if(addrSpace) {
    func_instance *ifunc = addrSpace->findOnlyOneFunction(func.c_str());

    if(ifunc == NULL) {
      fprintf(stderr, "%s[%d]: Can't find function %s\n", FILE__, __LINE__, func.c_str());
      return AstNodePtr();
    }

    return AstNodePtr(new AstCallNode(ifunc, args));
  } else {
    return AstNodePtr(new AstCallNode(func, args));
  }
}

AstNodePtr AstNode::funcCallNode(func_instance *func, std::vector<AstNodePtr> &args) {
  if(func == NULL) {
    return AstNodePtr();
  }
  return AstNodePtr(new AstCallNode(func, args));
}

AstNodePtr AstNode::funcCallNode(func_instance *func) {
  if(func == NULL) {
    return AstNodePtr();
  }
  return AstNodePtr(new AstCallNode(func));
}

AstNodePtr AstNode::funcCallNode(Address addr, std::vector<AstNodePtr> &args) {
  return AstNodePtr(new AstCallNode(addr, args));
}

AstNodePtr AstNode::memoryNode(memoryType ma, int which, int size) {
  return AstNodePtr(new AstMemoryNode(ma, which, size));
}

AstNodePtr AstNode::originalAddrNode() {
  if(originalAddrNode_ == NULL) {
    originalAddrNode_ = AstNodePtr(new AstOriginalAddrNode());
  }
  return originalAddrNode_;
}

AstNodePtr AstNode::actualAddrNode() {
  if(actualAddrNode_ == NULL) {
    actualAddrNode_ = AstNodePtr(new AstActualAddrNode());
  }
  return actualAddrNode_;
}

AstNodePtr AstNode::dynamicTargetNode() {
  if(dynamicTargetNode_ == NULL) {
    dynamicTargetNode_ = AstNodePtr(new AstDynamicTargetNode());
  }
  return dynamicTargetNode_;
}

AstNodePtr AstNode::snippetNode(Dyninst::PatchAPI::SnippetPtr snip) {
  return AstNodePtr(new AstSnippetNode(snip));
}

AstNodePtr AstNode::scrambleRegistersNode() {
  return AstNodePtr(new AstScrambleRegistersNode());
}

AstNodePtr AstNode::atomicOperationStmtNode(opCode astOpcode, AstNodePtr variable,
                                            AstNodePtr constant) {
  return AstNodePtr(new AstAtomicOperationStmtNode(astOpcode, variable, constant));
}

void AstNode::setType(BPatch_type *t) {
  bptype = t;
  if(t != NULL) {
    size = t->getSize();
  }
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

  if(indexNode_ != AstNodePtr()) {
    return indexNode_;
  }
  std::vector<AstNodePtr> args;
  // By not including a process we'll specialize at code generation.
  indexNode_ = AstNode::funcCallNode("DYNINSTthreadIndex", args);
  assert(indexNode_);
  indexNode_->setConstFunc(true);

  return indexNode_;
}

// This name is a bit of a misnomer. It's not the strict use count; it's the
// use count modified by whether a node can be kept or not. We can treat
// un-keepable nodes (AKA those that don't strictly depend on their AST inputs)
// as multiple different nodes that happen to have the same children; keepable
// nodes are the "same". If that makes any sense.
//
// In any case, we use the following algorithm to set use counts:
//
// DFS through the AST graph.
// If an AST can be kept:
//  Increase its use count;
//  Return.
// If an AST cannot be kept:
//  Recurse to each child;
//  Return
//
// The result is all nodes having counts of 0, 1, or >1. These mean:
// 0: node cannot be kept, or is only reached via a keepable node.
// 1: Node can be kept, but doesn't matter as it's only used once.
// >1: keep result in a register.

void AstNode::setUseCount() {
  if(useCount) {
    // If the useCount is 1, then it means this node can
    // be shared, and there is a copy. In that case, we assume
    // that when this particular incarnation is generated, the
    // result will already be calculated and sitting in a register.
    // Since that's the case, just up the useCount so we know when
    // we can free said register.
    useCount++;
    return;
  }
  if(canBeKept()) {
    useCount++;
    // We purposefully fall through... if our use count
    // is 1, we'll have to calculate this node instead of
    // keeping it around. In that case, see if any of the
    // children are shared (because we can reuse them when
    // calculating this guy)
  }
  // We can't be kept, but maybe our children can.
  for(unsigned i = 0; i < children.size(); i++) {
    children[i]->setUseCount();
  }
}

void AstNode::cleanUseCount() {
  useCount = 0;

  for(unsigned i = 0; i < children.size(); i++) {
    children[i]->cleanUseCount();
  }
}

// Allocate a register and make it available for sharing if our
// node is shared
Dyninst::Register AstNode::allocateAndKeep(codeGen &gen, bool noCost) {
  ast_printf("Allocating register for node %p, useCount %d\n", (void *)this, useCount);
  // Allocate a register
  Dyninst::Register dest = gen.rs()->allocateRegister(gen, noCost);

  ast_printf("Allocator returned %u\n", dest.getId());
  assert(dest != Dyninst::Null_Register);

  if(useCount > 1) {
    ast_printf("Adding kept register %u for node %p: useCount %d\n", dest.getId(), (void *)this, useCount);
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
bool AstNode::generateCode(codeGen &gen, bool noCost, Address &retAddr, Dyninst::Register &retReg) {
  static bool entered = false;

  bool ret = true;

  bool top_level;
  if(entered) {
    top_level = false;
  } else {
    entered = true;
    top_level = true;
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
  if(top_level) {
    gen.setRegTracker(new regTracker_t);
  }

  // note: this could return the value "(Address)(-1)" -- csserra
  if(!generateCode_phase2(gen, noCost, retAddr, retReg)) {
    fprintf(stderr, "WARNING: failed in generateCode internals!\n");
    ret = false;
  }

  if(top_level) {
    delete gen.tracker();
    gen.setRegTracker(NULL);
  }

  if(top_level) {
    entered = false;
  }
  return ret;
}

bool AstNode::generateCode(codeGen &gen, bool noCost) {
  Address unused = ADDR_NULL;
  Dyninst::Register unusedReg = Dyninst::Null_Register;
  bool ret = generateCode(gen, noCost, unused, unusedReg);
  gen.rs()->freeRegister(unusedReg);

  return ret;
}

bool AstNode::previousComputationValid(Dyninst::Register &reg, codeGen &gen) {
  Dyninst::Register keptReg = gen.tracker()->hasKeptRegister(this);
  if(keptReg != Dyninst::Null_Register) {
    reg = keptReg;
    ast_printf("Returning previously used register %u for node %p\n", reg.getId(), (void *)this);
    return true;
  }
  return false;
}

bool AstNode::initRegisters(codeGen &g) {
  bool ret = true;
  for(unsigned i = 0; i < children.size(); i++) {
    if(!children[i]->initRegisters(g)) {
      ret = false;
    }
  }
  return ret;
}

bool AstNode::allocateCanaryRegister(codeGen &gen, bool noCost, Dyninst::Register &reg,
                                     bool &needSaveAndRestore) {
  // Let's see if we can find a dead register to use!
  instPoint *point = gen.point();

  // Try to get a scratch register from the register space
  registerSpace *regSpace = registerSpace::actualRegSpace(point);
  bool realReg = true;
  Dyninst::Register tmpReg = regSpace->getScratchRegister(gen, noCost, realReg);
  if(tmpReg != Dyninst::Null_Register) {
    reg = tmpReg;
    needSaveAndRestore = false;
    if(gen.getArch() == Arch_x86) {
      gen.rs()->noteVirtualInReal(reg, RealRegister(reg));
    }
    return true;
  }

  // Couldn't find a dead register to use :-(
  registerSpace *deadRegSpace = registerSpace::optimisticRegSpace(gen.addrSpace());
  reg = deadRegSpace->getScratchRegister(gen, noCost, realReg);
  if(reg == Dyninst::Null_Register) {
    fprintf(stderr, "WARNING: using default allocateAndKeep in allocateCanaryRegister\n");
    reg = allocateAndKeep(gen, noCost);
  }
  needSaveAndRestore = true;
  fprintf(stderr, "allocateCanaryRegister will require save&restore at 0x%lx\n",
          gen.point()->addr());

  return true;
}

BPatch_type *AstNode::checkType(BPatch_function *) {
  return BPatch::bpatch->type_Untyped;
}

// Our children may have incorrect useCounts (most likely they
// assume that we will not bother them again, which is wrong)
void AstNode::fixChildrenCounts() {
  for(unsigned i = 0; i < children.size(); i++) {
    children[i]->setUseCount();
  }
}

// Occasionally, we do not call .generateCode_phase2 for the referenced node,
// but generate code by hand. This routine decrements its use count properly
void AstNode::decUseCount(codeGen &gen) {
  if(useCount == 0) {
    return;
  }

  useCount--;

  if(useCount == 0) {
    gen.tracker()->removeKeptRegister(gen, this);
  }
}

bool AstNode::generate(Point *point, Buffer &buffer) {
  // For now, be really inefficient. Yay!
  codeGen gen(1024);
  instPoint *ip = IPCONV(point);

  gen.setPoint(ip);
  gen.setRegisterSpace(registerSpace::actualRegSpace(ip));
  gen.setAddrSpace(ip->proc());
  if(!generateCode(gen, false)) {
    return false;
  }

  unsigned char *start_ptr = (unsigned char *)gen.start_ptr();
  unsigned char *cur_ptr = (unsigned char *)gen.cur_ptr();
  buffer.copy(start_ptr, cur_ptr);

  return true;
}

std::string AstNode::format(std::string indent) {
  std::stringstream ret;
  ret << indent << "Default/" << std::hex << this << "()\n";
  return ret.str();
}
