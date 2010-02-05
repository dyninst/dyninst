// LGPL...

#include "relocation.h"
#include "baseTramp.h" // baseTramp, baseTrampInstance
#include "Transformer.h" // transformer class
#include "pcrel.h" // pcrelblargs
#include "debug.h"

#include "addressSpace.h" // findFuncByInternalFunc

#include <sstream>

using namespace Dyninst;
using namespace InstructionAPI;
using namespace Relocation;
using namespace std;

// Let's do the block class first. 

// A Block is a container for a set of instructions and
// instrumentation that we generate out as a single unit. To make life
// easier, it's currently matching the (implied) single-control-flow
// path assumption. That means that edge instrumentation must go into
// a separate Block. Darn.
// 
// A Block consists of three logical types of elements: instructions,
// instrumentation, and flow controllers.
//
// Instruction: a reference to an original code instruction that is
// being moved as part of this block.

// Instrumentation: a reference to a (currently) BaseTrampInstance
// that contains an entire, single-entry, single-exit sequence of
// instrumentation.

// Flow controllers: an abstraction of control flow instructions
// (branches/calls) that end basic blocks. One effect of moving code
// around is that control flow gets b0rked pretty badly, so we toss
// the original instruction and regenerate it later.


Block::Ptr Block::create(bblInstance *bbl) {
  if (!bbl) return Block::Ptr();

  relocation_cerr << "Creating new Block" << endl;

  Ptr newBlock = Ptr(new Block(bbl));  

  // Get the list of instructions in the block
  std::vector<std::pair<Instruction::Ptr, Address> > insns;
  bbl->getInsnInstances(insns);

  for (unsigned i = 0; i < insns.size(); ++i) {
    relocation_cerr << "  Adding instruction " 
		    << std::hex << insns[i].second << std::dec
		    << " " << insns[i].first->format() << endl;
    Element::Ptr ptr = RelocInsn::create(insns[i].first,
					 insns[i].second);
    if (!ptr) {
      // And this will clean up all of the created elements. Nice. 
      return Ptr();
    }
    
    newBlock->elements_.push_back(ptr);
  }

  return newBlock;
}

Block::Ptr Block::create(baseTramp *base) {
  // Instrumentation-only block... must be for 
  // an edge!

  if (!base) return Ptr();

  // Create an Inst element
  Inst::Ptr inst = Inst::create();
  if (!inst) return Ptr();
  inst->addBaseTramp(base);

  Ptr newBlock = Ptr(new Block(base));

  newBlock->elements_.push_back(inst);
  return newBlock;
}
  

// Returns false only on catastrophic failure.

bool Block::analyze(Analyzer &a) {
  bool succ = true;
  for (ElementList::iterator iter = elements_.begin();
       iter != elements_.end(); ++iter) {
    // Check to see if we're an instruction
    // Instrumentation cannot be sensitive
    // Control flow elements can be sensitive (CF sensitivity),
    // but this is subsumed in our CF redo of the block
    if (!((*iter)->apply(a)))
      succ = false;
  }
  return succ;
}

bool Block::transform(Transformer &t) {
  // Unlike (sensitivity) analysis, transformation may alter the 
  // instructions within the block or even the block entirely. For
  // example, we may replace a group of instructions with another group,
  // move instrumentation to aggregate, or the like. Thus this isn't 
  // strictly visitor pattern; instead, we hand the Transformer
  // a list of our blocks.
  relocation_cerr << "Block " << this << " << calling transformer" << endl;
  //  return t.transformBlock(*this);
  return true;
}

// Returns false on catastrophic failure
// Sets changed to true if something about this block
// changed in this generation; e.g. if we should re-run the
// fixpoint algorithm.
//
// Our fixpoint is to minimize the size of the generated code w.r.t.
// branches. So we "change" if the address of the first instruction changes
// (since it might be a target) or if the size of the block changes...
//
// So basically if the incoming start address is different or if the
// block size changed.

bool Block::generate(codeGen &gen, bool &changed) {
  changed = false;

  relocation_cerr << "Block generation for " << this << ", cur addr "
		  << std::hex << curAddr_ << std::dec 
		  << " and size " << size_ << endl;

  if (gen.currAddr() != curAddr_) {
    relocation_cerr << "... current addr " << std::hex << curAddr_ 
		    << " different from new addr " << gen.currAddr()
		    << std::dec << " marking as changed" << endl;
    changed = true;
    curAddr_ = gen.currAddr();
  }

  // Simple form: iterate over every Element, in order, and generate it.
  for (ElementList::iterator iter = elements_.begin(); iter != elements_.end(); ++iter) {
    relocation_cerr << "... Generating element " << (*iter)->format()
		    << ", starting addr " << std::hex << gen.currAddr()
		    << std::dec << endl;
    
    if (!(*iter)->generate(gen)) {
      return false;
      // This leaves the block in an inconsistent state and should only be used
      // for fatal failures.
    }
  }

  unsigned newSize = gen.currAddr() - curAddr_;
  relocation_cerr << "... new size " << newSize << " and previous " << size_ << endl;

  if (newSize < size_) {
    // This is problematic; we may be in a situation where the branch size 
    // is _just_ on the edge of what we can do in a small branch. So one pass
    // concludes we need a large branch, the next a small - and we never 
    // converge. 
    // I'm using Matt's logic here and forcing us to the larger size
    // by noop-padding.
    gen.fill(size_ - newSize, codeGen::cgNOP);    
  }
  else if (newSize > size_) {
    relocation_cerr << "... current size " << size_
		    << " different from new size " << newSize
		    << " marking as changed" << endl;

    changed = true;
    size_ = newSize;
  }

  return true;
}

std::string Block::format() const {
  stringstream ret;
  ret << "Block(" 
      << std::hex << origAddr() << std::dec
      << ") {" << endl;
  for (ElementList::const_iterator iter = elements_.begin();
       iter != elements_.end(); ++iter) {
    ret << "  " << (*iter)->format() << endl;
  }
  ret << "}" << endl;
  return ret.str();
}

/////////////////////////

bool RelocInsn::generate(codeGen &gen) {
  // Ach. Right. I should do stuff here. 
  // Since we're separating transformation and generation,
  // this must be an instruction that is not sensitive
  // to whatever is going on. Therefore it can be copied
  // across without modification.

  gen.copy(insn_->ptr(), insn_->size());
  return true;
}

RelocInsn::Ptr RelocInsn::create(Instruction::Ptr insn,
				 Address addr) {
  return Ptr(new RelocInsn(insn, addr));
}

string RelocInsn::format() const {
  stringstream ret;
  ret << "Insn(" << insn_->format() << ")";
  return ret.str();
}


///////////////////////

// Pick values that don't correspond to actual targets. I'm skipping
// 0 because it's used all over the place as a null.
const Address CFElement::Fallthrough(1);
const Address CFElement::Taken(2);

bool CFElement::generate(codeGen &gen) {
  // We need to create jumps to wherever our successors are
  // We can assume the addresses returned by our Targets
  // are valid, since we'll fixpoint until those stabilize. 
  //
  // There are the following cases:
  //
  // No explicit control flow/unconditional direct branch:
  //   1) One target
  //   2) Generate a branch unless it's unnecessary
  // Conditional branch:
  //   1) Two targets
  //   2) Use stored instruction to generate correct condition
  //   3) Generate a fallthrough "branch" if necessary
  // Call:
  //   1) Two targets (call and natural successor)
  //   2) As above, except make sure call bit is flipped on
  // Indirect branch:
  //   1) Just go for it... we have no control, really
  
  // First check: if we're not an indirect branch
  // and we have no known successors return immediately.
  if (!isIndirect_ && destMap_.empty()) return true;


  // TODO: address translation on an indirect branch...

  typedef enum {
    Illegal,
    Single,
    Taken_FT,
    Indirect } Options;
  
  Options opt = Illegal;

  if (isIndirect_) {
    opt = Indirect;
    relocation_cerr << "  generating CFElement as indirect branch" << endl;
  }
  else if (isConditional_ || isCall_) {
    opt = Taken_FT;
    relocation_cerr << "  generating CFElement as call or conditional branch" << endl;
  }
  else {
    opt = Single;
    relocation_cerr << "  generating CFElement as direct branch" << endl;
  }

  switch (opt) {
  case Single: {

    assert(!isIndirect_);
    assert(!isConditional_);
    assert(!isCall_);

    bool fallthrough = (destMap_.begin()->first == Fallthrough);
    TargetInt *target = destMap_.begin()->second;
    assert(target);

    if (!generateBranch(gen,
			gen.currAddr(),
			target,
			insn_,
			fallthrough)) {
      return false;
    }
    break;
  }
  case Taken_FT: {
    // This can be either a call (with an implicit fallthrough as shown by
    // the FUNLINK) or a conditional branch.
    if (isCall_) {
      // Well, that kinda explains things
      assert(!isConditional_);
      relocation_cerr << "  ... generating call" << endl;
      if (!generateCall(gen,
			gen.currAddr(),
			destMap_[Taken],
			insn_))
	return false;
    }
    else {
      assert(!isCall_);
      relocation_cerr << "  ... generating conditional branch" << endl;
      if (!generateConditionalBranch(gen,
				     gen.currAddr(),
				     destMap_[Taken],
				     insn_))
	return false;
    }

    // Aaand the fallthrough
    // We can have calls that don't return and thus don't have funlink edges
    if (destMap_.find(Fallthrough) != destMap_.end()) {
      relocation_cerr << "  ... generating possible fallthrough branch" << endl;
      if (!generateBranch(gen, 
			  gen.currAddr(), 
			  destMap_[Fallthrough],
			  insn_,
			  true)) {
	return false;
      }
    }
    break;
  }
  case Indirect: {
    bool requireTranslation = false;
    for (DestinationMap::iterator iter = destMap_.begin();
	 iter != destMap_.end(); ++iter) {
      if (iter->first != iter->second->addr()) {
	requireTranslation = true;
	break;
      }
    }
    Register reg; /* = originalRegister... */
    if (requireTranslation) {
      if (!generateAddressTranslator(gen, reg))
	return false;
    }
    if (isCall_) {
      if (!generateIndirectCall(gen, reg, insn_))
	return false;
      // We may be putting another block in between this
      // one and its fallthrough due to edge instrumentation
      // So if there's the possibility for a return put in
      // a fallthrough branch

      if (destMap_.find(Fallthrough) != destMap_.end()) {
	if (!generateBranch(gen,
			    gen.currAddr(),
			    destMap_[Fallthrough],
			    Instruction::Ptr(),
			    true))
	  return false;
      }
    }
    else {
      if (!generateIndirect(gen, reg, insn_))
	return false;
    }
    break;
  }
  default:
    assert(0);
  }
  return true;
}

CFElement::Ptr CFElement::create() {
  return Ptr(new CFElement());
}

CFElement::~CFElement() {
  // Delete all Targets in our map
  for (DestinationMap::iterator i = destMap_.begin(); 
       i != destMap_.end(); ++i) {
    delete i->second;
  }
}

void CFElement::addDestination(Address index, TargetInt *dest) {
  // Annoying required copy... 
  if (destMap_.find(index) != destMap_.end()) {
    // Multiple destinations for the same original
    // address? Complain.
    cerr << "WARNING: re-used original destination in CFElement: " << std::hex << index << std::dec << endl;
  }
  destMap_[index] = dest;
}

void CFElement::updateInsn(Instruction::Ptr insn) {

  relocation_cerr << "Updating CFElement off insn " << insn->format() << endl;

  insn_ = insn;

  isConditional_ = isCall_ = isIndirect_ = false;

  // And set type flags based on what the instruction was
  // If it allows fallthrough it must be conditional...
  if (insn->allowsFallThrough()) {
    relocation_cerr << "... allows fallthrough, setting isConditional" << endl;
    isConditional_ = true;
  }
  // Calls show up as fallthrough-capable, which is true
  // (kind of) for parsing but _really_ not what we want
  // to identify conditional branches...
  if (insn->getCategory() == c_CallInsn) {
    relocation_cerr << "... is call, setting isCall and unsetting isConditional" << endl;
    isCall_ = true;
    isConditional_ = false;
  }

  // And here's the annoying bit - we can't directly determine
  // whether something is indirect. Bill suggests getting the 
  // control flow target, binding *something* as the PC, and
  // evaluating it. I think that sucks. 

  // TODO FIXME
  RegisterAST ip32(r_EIP);
  RegisterAST ip64(r_RIP);

  Expression::Ptr exp = insn->getControlFlowTarget();
  
  // Bind the IP, why not...
  exp->bind(&ip32, Result(u32, addr_));
  exp->bind(&ip64, Result(u64, addr_));

  Result res = exp->eval();
  if (!res.defined) {
    relocation_cerr << "... cannot statically resolve, setting isIndirect" << endl;
    isIndirect_ = true;
  }
  // EMXIF ODOT

}

void CFElement::updateAddr(Address addr) {
  addr_ = addr;
}

bool CFElement::generateBranch(codeGen &gen,
			       Address from,
			       TargetInt *to,
			       Instruction::Ptr insn,
			       bool fallthrough) {
  assert(to);
  if (to->addr() == from) {
    relocation_cerr << "    ... skipping unnecessary branch with 0 offset" << endl;
    return true;
  }

  if (to->addr() == 0) {
    relocation_cerr << "    ... branch to invalid destination!" << endl;
    return true;
  }

  if (fallthrough) {
    // This is a fallthrough 'branch' or the equivalent.
    // Since we generate blocks in order we can guarantee
    // that a fallthrough will have a higher address than
    // the current one; so if to < from then just skip.
    if (to->addr() < from)
      return true;
  }

  // We can put in an unconditional branch as an ender for 
  // a block that doesn't have a real branch. So if we don't have
  // an instruction generate a "generic" branch

  // We can see a problem where we want to branch to (effectively) 
  // the next instruction. So if we ever see that (a branch of offset
  // == size) back up the codeGen and shrink us down.

  Address preBranch = gen.currAddr();
  codeBufIndex_t preBranchIndex = gen.getIndex();

  relocation_cerr << "    ... branch from " << std::hex << from
		  << "->" << to->addr() << std::dec << endl;
  if (insn) {
    instruction ugly_insn(insn->ptr());
    pcRelJump pcr(to->addr(), ugly_insn);
    pcr.gen = &gen;
    pcr.apply(from);
  }
  else {
    instruction::generateBranch(gen, from, to->addr());
  }

  Address postBranch = gen.currAddr();

  relocation_cerr << "        ... post branch check: "
		  << std::hex << from << "->" 
		  << to->addr() << " and mutator side "
		  << preBranch << "->" << postBranch 
		  << std::dec << endl;

  if ((to->addr() - from)  == (postBranch - preBranch)) {
    relocation_cerr << "        ... null branch, removing" << endl;
    gen.setIndex(preBranchIndex);
  }
    

  return true;
}

bool CFElement::generateCall(codeGen &gen,
			     Address from,
			     TargetInt *to,
			     Instruction::Ptr insn) {
  if (!to) {
    // This can mean an inter-module branch...
    relocation_cerr << "    ... skipping call with no target!" << endl;
    return true;
  }

  if (!to->valid()) {
    relocation_cerr << "    ... call to invalid destination!" << endl;
#if 0
    if (insn)
      gen.moveIndex(insn->size());
#endif
    return true;
  }

  relocation_cerr << "    ... call from " << std::hex << from
		  << "->" << to->addr() << std::dec << endl;

  instruction ugly_insn(insn->ptr());
  pcRelCall pcr(to->addr(), ugly_insn);
  pcr.gen = &gen;
  pcr.apply(from);

  return true;
}

bool CFElement::generateConditionalBranch(codeGen &gen,
					  Address from, 
					  TargetInt *to,
					  Instruction::Ptr insn) {
  if (!to->valid()) {
    relocation_cerr << "    ... call to invalid destination!" << endl;
#if 0
    if (insn)
      gen.moveIndex(insn->size());
#endif
    return true;
  }

  // I'm just going to use Matt's patch-based system
  // here...
  // 1) It's based off the instruction class, so make me an instruction

  relocation_cerr << "    ... jcc from " << std::hex << from
		  << "->" << to->addr() << std::dec << endl;
  
  instruction ugly_insn(insn->ptr());
  pcRelJCC pcr(to->addr(), ugly_insn);
  pcr.gen = &gen;
  pcr.apply(from);

  // That should do it...

  return true;
}

bool CFElement::generateIndirect(codeGen &gen,
				 Register reg,
				 Instruction::Ptr insn) {
  // TODO: don't ignore reg...
  // Indirect branches don't use the PC and so are
  // easy - we just copy 'em.
  gen.copy(insn->ptr(), insn->size());
  return true;
}

bool CFElement::generateIndirectCall(codeGen &gen,
				     Register reg,
				     Instruction::Ptr insn) {
  gen.copy(insn->ptr(), insn->size());
  return true;
}

bool CFElement::generateAddressTranslator(codeGen &gen,
					  Register &reg) {
  // Do nothing...
  return true;
}

std::string CFElement::format() const {
  stringstream ret;
  ret << "CFElement(" << std::hex;
  if (isIndirect_) ret << "<ind>";
  if (isConditional_) ret << "<cond>";
  if (isCall_) ret << "<call>";
		     
  for (DestinationMap::const_iterator iter = destMap_.begin();
       iter != destMap_.end();
       ++iter) {
    ret << iter->first << "->" << iter->second << ",";
  }
  ret << std::dec << ")";
  return ret.str();
}

Inst::Ptr Inst::create() {
  return Ptr(new Inst());
}

void Inst::addBaseTramp(baseTramp *b) {
  baseTramps_.push_back(b);
}

bool Inst::empty() const {
  return baseTramps_.empty();
}

bool Inst::generate(codeGen &gen) {
  // Fun for the whole family!
  // Okay. This (initially) is going to hork
  // up all of our address/structure tracking because
  // I just can't be bothered to care. Instead, 
  // we'll get some code, and that's good enough for me.

  // For each baseTramp...
  // Fake a baseTrampInstance (FIXME, TODO, etc. etc.)
  //   bti->generateCodeInlined
  // ... done
  // TODO: baseTramp combining for those rare occasions that
  // someone is crazy enough to do post-instruction + pre-successor
  // instrumentation.

  for (std::list<baseTramp *>::iterator b_iter = baseTramps_.begin();
       b_iter != baseTramps_.end(); ++b_iter) {
    baseTrampInstance fake_bti(*b_iter, NULL);
    fake_bti.updateMTInstances();

    if (!fake_bti.generateCode(gen, gen.currAddr(), NULL)) {
      return false;
    }
  }
  return true;
}

string Inst::format() const {
  stringstream ret;
  ret << "Inst(" << baseTramps_.size() << ")";
  return ret.str();
}

PCRelativeData::Ptr PCRelativeData::create(Instruction::Ptr insn,
					   Address addr,
					   Address target) {
  return Ptr(new PCRelativeData(insn, addr, target));
}

bool PCRelativeData::generate(codeGen &gen) {
  // We want to take the original instruction and emulate
  // it at whatever our new address is. 

  // Fortunately, we can reuse old code to handle the
  // translation

  // Find the original target of the instruction 

  relocation_cerr << "  Generating a PC-relative data access (" << insn_->format()
		  << "," << std::hex << addr_ 
		  <<"," << target_ << std::dec << ")" << endl;

  instruction ugly_insn(insn_->ptr());
  pcRelData pcr(target_, ugly_insn);
  pcr.gen = &gen;
  pcr.apply(gen.currAddr());
  return true;
}

string PCRelativeData::format() const {
  stringstream ret;
  ret << "PCRel(" << insn_->format() << ")";
  return ret.str();
}


////////////////////////
GetPC::Ptr GetPC::create(Instruction::Ptr insn,
			 Address addr,
			 bool isStack,
			 unsigned reg) {
  return Ptr(new GetPC(insn, addr, isStack, reg));
}

bool GetPC::generate(codeGen &gen) {
  // Two options: top of stack (push origAddr) 
  // or into a register (/w/ a mov)


  if (isStack_) {
    // Should do something more platform-independent
    // Should care...

    return PCtoStack(gen);
  }
  else
    return PCtoReg(gen);
}

bool GetPC::PCtoStack(codeGen &gen) {
  GET_PTR(newInsn, gen);

  if(gen.addrSpace()->proc()) {
    *newInsn = 0x68; // Push; we're replacing "call 0" with "push original IP"
    newInsn++;	  
    Address EIP = addr_ + insn_->size();
    unsigned int *temp = (unsigned int *) newInsn;
    *temp = EIP;
    // No 9-byte jumps...
    assert(sizeof(unsigned int) == 4); // should be a compile-time assert
    newInsn += sizeof(unsigned int);
  }
  else {
    *newInsn = 0xE8;
    newInsn++;
    unsigned int *temp = (uint32_t *) newInsn;
    *temp = 0;
    newInsn += sizeof(uint32_t);
    Address offset = addr_ - gen.currAddr();
    *newInsn = 0x81;
    newInsn++;
    *newInsn = 0x04;
    newInsn++;
    *newInsn = 0x24;
    newInsn++;
    temp =  (uint32_t *) newInsn;
    *temp = offset;
    newInsn += sizeof(uint32_t);	  
  }	

  SET_PTR(newInsn, gen);
  return true;
}

bool GetPC::PCtoReg(codeGen &gen) {
  GET_PTR(newInsn, gen);

  if(gen.addrSpace()->proc()) {
    // Okay, put the PC into the 'reg'
    Address EIP = addr_ + insn_->size();
    *newInsn = static_cast<unsigned char>(0xb8 + reg_); 
    // MOV family, destination of the register encoded by
    // 'reg', source is an Iv immediate
    newInsn++;
    unsigned int *temp = (unsigned int *)newInsn;
    *temp = EIP;
    //assert(sizeof(unsigned int *)==4);
    //newInsn += sizeof(unsigned int *);
    newInsn += 4;  // fix for AMD64
  }
  else {
    *newInsn = 0xE8;
    newInsn++;
    unsigned int *temp = (uint32_t *) newInsn;
    *temp = 0;
    newInsn += sizeof(unsigned int);
    Address offset = addr_ - gen.currAddr();
    *newInsn = 0x81;
    newInsn++;
    *newInsn = 0x04;
    newInsn++;
    *newInsn = 0x24;
    newInsn++;
    temp =  (uint32_t*) newInsn;
    *temp = offset;
    newInsn += sizeof(uint32_t);	  
    *newInsn = static_cast<unsigned char>(0x58 + reg_); // POP family
    newInsn++;
  }
  SET_PTR(newInsn, gen);
  return true;
}

string GetPC::format() const {
  stringstream ret;
  ret << "GetPC(" 
      << std::hex << addr_ << std::dec;
  if (isStack_) 
    ret << ",<stack>)";
  else
    ret << ",<reg " << reg_ <<">)";
  return ret.str();
}

//////////////////
//
// Temporary definitions

bool Analyzer::apply(Element &) { return true; }
bool Analyzer::apply(RelocInsn &) { return true; }
bool Analyzer::apply(CFElement &) { return true; }
bool Analyzer::apply(Inst &) { return true; }

