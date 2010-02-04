// LGPL...

#include "relocation.h" // Elements
#include "function.h" // bblInstance
#include "Transformer.h" // Transformer classes
#include "debug.h"

#include "Expression.h" // PC-relative cracking
#include "addressSpace.h" // CFG wibbling

#include "baseTramp.h" // Basetramps!

#include "InstructionDecoder.h" // getPC analysis

using namespace std;
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

bool CFElementTransformer::processBlock(BlockList::iterator &iter) {
  const bblInstance *bbl = (*iter)->bbl();

  // Can be true if we see an instrumentation block...
  if (!bbl) return true;

  CFElement::Ptr ender = CFElement::create();
  // Okay, now we need to construct a CFElement matching this block's successors.
  // The CFElement contains a certain amount of modelling *when* an edge is taken,
  // so we need to reconstruct that. We can do that via edge types. Here
  // [Taken] and [Fallthrough] are constants defined by CFElement
  
  // ET_CALL(callee) : [Taken] -> Target(callee)
  // ET_COND_TAKEN(targ) : [Taken]  -> Target(targ)
  // ET_COND_NOT_TAKEN(ft) : [Fallthrough] -> Target(ft)
  // ET_INDIR(b) : b.addr -> Target(b)
  // ET_DIRECT(targ) : [Fallthrough] -> Target(targ)
  // ET_CATCH(...) : (ignored)
  // ET_FUNLINK(succ) : [Fallthrough] -> Target(succ)
  // ET_NOEDGE : wtf?

  relocation_cerr << "Creating block ender..." << endl;

  SuccVec successors;
  getInterproceduralSuccessors(bbl, successors);

  for (unsigned i = 0; i < successors.size(); ++i) {
    TargetInt *targ = successors[i].first;
    EdgeTypeEnum type = successors[i].second;
    switch(type) {
    case ET_INDIR: {
      relocation_cerr << "Adding indirect destination: "
		      << std::hex << targ->addr() << std::dec << endl;
      ender->addDestination(targ->addr(), targ);
      break;
    }
    case ET_CALL:
    case ET_DIRECT:
    case ET_COND_TAKEN:
      relocation_cerr << "Adding taken destination: "
		      << std::hex << targ->addr() << std::dec << endl;
      ender->addDestination(CFElement::Taken, targ);
      break;
    case ET_COND_NOT_TAKEN:
    case ET_FALLTHROUGH:
    case ET_FUNLINK:
      relocation_cerr << "Adding fallthrough destination: "
		      << std::hex << targ->addr() << std::dec << endl;
      ender->addDestination(CFElement::Fallthrough, targ);
      break;
    case ET_NOEDGE:
    case ET_CATCH:
    default:
      relocation_cerr << "Ignoring destination type " << type << endl;
      // Ignore...
      break;
    }
  }

  // Now check the last Element. If it's an explicit CF instruction
  // pull it and replace it with the CFElement; otherwise append.

  RelocInsn::Ptr reloc = dyn_detail::boost::dynamic_pointer_cast<RelocInsn>((*iter)->elements().back());
  assert(reloc);

  Instruction::Ptr insn = reloc->insn();
  Address addr = reloc->addr();

  if ((insn->getCategory() != c_CompareInsn) &&
      (insn->getCategory() != c_NoCategory)) {
    // Remove it so that it will be replaced by end
    (*iter)->elements().pop_back();
    
    // We want to shove this into the CFElement so it'll know how to regenerate the
    // branch/call/whatever
    ender->updateInsn(insn);
    ender->updateAddr(addr);
  } 

  (*iter)->elements().push_back(ender);

  return true;
}


void CFElementTransformer::getInterproceduralSuccessors(const bblInstance *bbl,
							SuccVec &succ) {
  int_basicBlock *block = bbl->block();
  int_function *func = block->func();

  // This function is only annoying because our parsing layer is so incredibly
  // unintuitive and backwards. Hence... yeah. Annoyance. 

  // The int_ layer (that bblInstances work on) has two restrictions:
  // Single-function; we don't see interprocedural edges
  // No efficient way to get a list of all edges with edge types;
  //   instead we see target blocks and can query the type of each
  //   edge to each block (nice O(n^2)...)

  // Instead we do it ourselves. I've got the code here for one simple
  // reason: handling calls between modules in the static case. Calls
  // go to PLT entries and we don't parse those, so normally we'd see
  // an image_basicBlock with no image_function, int_function,
  // int_basicBlock, or bblInstance... useless. Instead I'm using the
  // Target concept to create a destination out of whole cloth.

  // This requires an... interesting... dodge through to the internals
  pdvector<image_edge *> ib_outs;
  block->llb()->getTargets(ib_outs);
  for (unsigned i = 0; i < ib_outs.size(); i++) {
    Succ out;
    out.first = NULL;
    out.second = ib_outs[i]->getType();
    
    // We have an image_basicBlock... now we need to map up
    // to both an int_basicBlock and an bblInstance.
    image_basicBlock *ib = ib_outs[i]->getTarget();
    if (ib->containedIn(func->ifunc())) {
      int_basicBlock *targ = func->findBlockByImage(ib);
      assert(targ);
      out.first = new Target<const bblInstance *>(targ->origInstance());
    }
    else {
      // Oy...
      assert(out.second == ET_CALL);
      // Block must be an entry point since we reach it with
      // a call...
      image_func *iCallee = ib->getEntryFunc();
      if (iCallee) {
	int_function *callee = bbl->proc()->findFuncByInternalFunc(iCallee);
	assert(callee);
	// Make sure it's parsed
	callee->blocks();
	// Same as above
	int_basicBlock *targ = callee->findBlockByImage(ib);
	out.first = new Target<const bblInstance *>(targ->origInstance());
      }
      else {
	// Okay. This is obviously (really) a call to a PLT
	// entry. Now, calls to PLT entries are tricky, since
	// we currently don't parse them. OTOH, that means that
	// I don't have anything to find a bblInstance with.
	// Instead we use a special-form Target

	// First, assert that our offset is 0. If it isn't we're in
	// real trouble since we can't upcast.
	assert(bbl->firstInsnAddr() == block->llb()->firstInsnOffset());
	out.first = new Target<Address>(ib->firstInsnOffset());
      }
    }
    assert(out.first);
    succ.push_back(out);
  }
}


bool LocalCFTransformer::processBlock(BlockList::iterator &iter) {
  // We don't care about elements that aren't CFElements
  // We may remove CFElements and replace them with a new 
  // CFElement

  const Block::ElementList &elements = (*iter)->elements();

  relocation_cerr << "localCFTransformer going to work on block " 
		  << std::hex << (*iter)->origAddr() << std::dec << endl;

  // We don't need to iterate over all the elements; by definition
  // the only one we care about is the last one. 
  // 
  // FIXME if this assumption no longer holds; that is, if we start
  // using traces instead of blocks. 

  CFElement::Ptr cf = dyn_detail::boost::dynamic_pointer_cast<CFElement>(elements.back() );
  
  if (!cf) return true;

  // If this CFElement contains a Target that is a bblInstance
  // in our map, replace it with a Target to that block.
  for (CFElement::DestinationMap::iterator d_iter = cf->destMap_.begin();
       d_iter != cf->destMap_.end(); ++d_iter) {

    cerr << "BTW, block ender is " << d_iter->second << endl;

    if (d_iter->second->valid() == false) {
      // Whatnow?
      continue;
    }
    
    // Can't use d_iter->first because that can contain the distinguished
    // values Taken and Fallthrough...
    Address addr = d_iter->second->addr();
    relocation_cerr << "   considering destination " 
		    << std::hex << addr << std::dec << endl;
    BlockMap::const_iterator found = bMap_.find(addr);
    if (found != bMap_.end()) {
      relocation_cerr << "      found matching block " << found->second.get() << endl;

      // First, record the bblInstance we removed an edge from. We will check
      // later to see if we removed all the incoming edges from this instance...
      recordIncomingEdges(d_iter->second);

      // And be sure not to leak
      if (d_iter->second)
	delete d_iter->second;

      Target<Block::Ptr> *t = new Target<Block::Ptr>(found->second);
      d_iter->second = t;

      relocation_cerr << "        Incrementing removed edge count for " 
		      << std::hex << found->first << std::dec << endl;
      replacedCount_[found->first]++;
    }
  }
  return true;
}

// Count up how many incoming edges we still have
// to see if we need to branch in to this block
bool LocalCFTransformer::postprocess(BlockList &) {
  relocation_cerr << "Postprocessing LocalCFTransformer" << endl;
  for (std::map<Address, int>::iterator iter = replacedCount_.begin();
       iter != replacedCount_.end(); ++iter) {
    Address addr = iter->first;

    int removedEdges = iter->second;

    // see if this block has any incoming edges that we didn't remove
    int incomingEdges = incomingCount_[addr];

    relocation_cerr << "   Considering block at " 
		    << std::hex << addr << std::dec
		    << ": incoming " << incomingEdges 
		    << " and removed " << removedEdges
		    << endl;

    
    if (removedEdges == incomingEdges) {
      pMap_[addr] = Suggested;
    }
  }
  return true;
}

int LocalCFTransformer::getInEdgeCount(const bblInstance *bbl) {
  relocation_cerr << "   ... getting number of in edges for block at "
		  << std::hex << bbl->firstInsnAddr() << std::dec << endl;

  // Need to duck to internals to get interprocedural counts
  int_basicBlock *block = bbl->block();
  pdvector<image_edge *> ib_ins;
  block->llb()->getSources(ib_ins);
  int ins = ib_ins.size();
  relocation_cerr << "   ... ret " << ins << endl;
  return ins;
}

void LocalCFTransformer::recordIncomingEdges(const TargetInt *in) {
  if (!in) return;
  if (incomingCount_.find(in->addr()) != incomingCount_.end()) return;

  const Target<const bblInstance *> *targ = dynamic_cast<const Target<const bblInstance *> *>(in);
  if (!targ) return;
 
  incomingCount_[targ->addr()] = getInEdgeCount(targ->t());
}

bool PCRelTransformer::processBlock(BlockList::iterator &b_iter) {
  // Identify PC-relative data accesses
  // and "get PC" operations and replace them
  // with dedicated Elements

  Block::ElementList &elements = (*b_iter)->elements();

  relocation_cerr << "PCRelTrans: processing block " 
		  << (*b_iter).get() << " with "
		  << elements.size() << " elements." << endl;

  for (Block::ElementList::iterator iter = elements.begin();
       iter != elements.end(); ++iter) {
    // Can I do in-place replacement? Apparently I can...
    // first insert new (before iter) and then remove iter

    relocation_cerr << "    "
		    << std::hex << (*iter)->addr() << std::dec << endl;

    if (!((*iter)->insn())) continue;

    Address target = 0;
    bool stackTop = false;
    unsigned reg = 0;
    if (isPCRelData(*iter, target)) {
      relocation_cerr << "  ... isPCRelData at " 
		      << std::hex << (*iter)->addr() << std::dec << endl;
      Element::Ptr replacement = PCRelativeData::create((*iter)->insn(),
							(*iter)->addr(),
							target);
      (*iter).swap(replacement);
    }
    else if (isGetPC(*iter, stackTop, reg)) {
      relocation_cerr << "  ... isGetPC at " 
		      << std::hex << (*iter)->addr() << std::dec << endl;
      Element::Ptr replacement = GetPC::create((*iter)->insn(),
					       (*iter)->addr(),
					       stackTop,
					       reg);
      // This is kind of complex. We don't want to just pull the getPC
      // because it also might end the basic block. If that happens we
      // need to pull the fallthough element out of the CFElement so
      // that we don't hork control flow. What a pain.
      if ((*iter) != elements.back()) {
	// Easy case; no worries.
	(*iter).swap(replacement);
      }
      else {
	CFElement::Ptr cf = dyn_detail::boost::dynamic_pointer_cast<CFElement>(*iter);
	// We don't want to be doing this pre-CF-creation...
	assert(cf); 
	
	// Ignore a taken edge, but create a new CFElement with the
	// required fallthrough edge
	CFElement::DestinationMap::iterator dest = cf->destMap_.find(CFElement::Fallthrough);
	if (dest != cf->destMap_.end()) {
	  CFElement::Ptr newCF = CFElement::create();
	  // Explicitly do _NOT_ reuse old information - this is just a branch
	  cerr << " Reusing old Target " << dest->second << " in new CF block ender " << endl;
	  cerr << "Checking validity: " << dest->second->valid() << endl;

	  newCF->destMap_[CFElement::Fallthrough] = dest->second;

	  // And since we delete destMap_ elements, NUKE IT from the original!
	  cf->destMap_.erase(dest);

	  // Before we forget: swap in the GetPC for the current element
	  (*iter).swap(replacement);

	  elements.push_back(newCF);
	  // Go ahead and skip it...
	  iter++;
	}
      }
    }
  }
  return true;
}

// We define this as "uses PC and is not control flow"
bool PCRelTransformer::isPCRelData(Element::Ptr ptr,
				   Address &target) {
  target = 0;

  if (ptr->insn()->getControlFlowTarget()) return false;

  static RegisterAST::Ptr ip32(new RegisterAST(r_EIP));
  static RegisterAST::Ptr ip64(new RegisterAST(r_RIP));
  
  if (!ptr->insn()->isRead(ip32) &&
      !ptr->insn()->isRead(ip64)) 
    return false;

  // Okay, see if we're memory
  set<Expression::Ptr> mems;
  ptr->insn()->getMemoryReadOperands(mems);
  ptr->insn()->getMemoryWriteOperands(mems);

  for (set<Expression::Ptr>::const_iterator iter = mems.begin();
       iter != mems.end(); ++iter) {
    Expression::Ptr exp = *iter;
    if (exp->bind(ip32.get(), Result(u32, ptr->addr() + ptr->insn()->size())) ||
	exp->bind(ip64.get(), Result(u64, ptr->addr() + ptr->insn()->size()))) {
      // Bind succeeded, eval to get target address
      Result res = exp->eval();
      if (!res.defined) {
	cerr << "ERROR: failed bind/eval at " << std::hex << ptr->addr() << endl;
      }
      assert(res.defined);
      target = res.convert<Address>();
      break;
    }
  }
  if (target) return true;

  // Didn't use the PC to read memory; thus we have to grind through
  // all the operands. We didn't do this directly because the 
  // memory-topping deref stops eval...
  vector<Operand> operands;
  ptr->insn()->getOperands(operands);
  
  for (vector<Operand>::iterator iter = operands.begin();
       iter != operands.end(); ++iter) {
    // If we can bind the PC, then we're in the operand
    // we want.

    Expression::Ptr exp = iter->getValue();
    if (exp->bind(ip32.get(), Result(u32, ptr->addr() + ptr->insn()->size())) ||
	exp->bind(ip64.get(), Result(u64, ptr->addr() + ptr->insn()->size()))) {
      // Bind succeeded, eval to get target address
      Result res = exp->eval();
      assert(res.defined);
      target = res.convert<Address>();
      break;
    }
  }
  assert(target != 0);
  return true;    
}

bool PCRelTransformer::isGetPC(Element::Ptr ptr,
			       bool &stackTop,
			       unsigned &reg) {
  stackTop = false;
  reg = 0;

  // TODO:
  // Check for call + size;
  // Check for call to thunk.
  // TODO: need a return register parameter.

  // Okay: checking for call + size
  Expression::Ptr CFT = ptr->insn()->getControlFlowTarget();
  if (!CFT) {
    relocation_cerr << "      ... no CFT, ret false from isGetPC" << endl;
    return false;
  }
   
  // Bind current PC
  RegisterAST ip32(r_EIP);
  RegisterAST ip64(r_RIP);

  // Bind the IP, why not...
  CFT->bind(&ip32, Result(u32, ptr->addr()));
  CFT->bind(&ip64, Result(u64, ptr->addr()));

  Result res = CFT->eval();

  if (!res.defined) {
    relocation_cerr << "      ... CFT not evallable, ret false from isGetPC" << endl;
    return false;
  }

  Address target = res.convert<Address>();

  if (target == (ptr->addr() + ptr->insn()->size())) {
    stackTop = true;
    relocation_cerr << "      ... call next insn, ret true" << endl;
    return true;
  }

  // Check for a call to a thunk function
  // TODO: replace entirely with sensitivity analysis. But for now? 
  // Yeah.
  
  // This is yoinked from arch-x86.C...
  if (addrSpace->isValidAddress(target)) {
    // Get us an instrucIter
    const unsigned char* buf = reinterpret_cast<const unsigned char*>(addrSpace->getPtrToInstruction(target));
    InstructionDecoder d(buf, 32);
    d.setMode(addrSpace->getAddressWidth() == 8);
    Instruction::Ptr firstInsn = d.decode();
    Instruction::Ptr secondInsn = d.decode();

    relocation_cerr << "      ... decoded target insns "
		    << firstInsn->format() << ", " 
		    << secondInsn->format() << endl;

    if(firstInsn && firstInsn->getOperation().getID() == e_mov
       && firstInsn->readsMemory() && !firstInsn->writesMemory()
       && secondInsn && secondInsn->getCategory() == c_ReturnInsn) {
      // We need to fake this by figuring out the register
      // target (assuming we're moving stack->reg),
      // and constructing an immediate with the value of the
      // original address of the call (+size)
      // This was copied from function relocation code... I 
      // don't understand most of it -- bernat
      const unsigned char *ptr = (const unsigned char*)(firstInsn->ptr());
      unsigned char modrm = *(ptr + 1);
      reg = static_cast<unsigned char>((modrm >> 3) & 0x3);

      if ((modrm == 0x0c) || (modrm == 0x1c)) {
	// Check source register (%esp == 0x24)
	if ((*(ptr + 2) == 0x24)) 
	  return true;
      }
    }
  }
  else {
    relocation_cerr << "      ... not call thunk, ret false" << endl;
  }
  return false;
}


bool InstTransformer::processBlock(BlockList::iterator &iter) {
  relocation_cerr << "InstTransformer, processing block " 
		  << std::hex << (*iter)->origAddr() << std::dec << endl;

  if ((*iter)->bbl() == NULL)
    return true;

  // Basic concept: iterate over all of our instructions and look
  // up the instPoint for that instruction. If it exists, prepend
  // or append an instrumentation element at the appropriate
  // point in the list.
  
  // TODO: edge instrumentation... that will require modifying
  // the block list as well as elements within a block. 

  baseTramp *pre = NULL;
  baseTramp *post = NULL;
  instPoint *point = NULL;

  ElementList &elements = (*iter)->elements();

  for (ElementList::iterator e_iter = elements.begin();
       e_iter != elements.end(); ++e_iter) {
    assert(pre == NULL);

    // We're inserting an Inst element before us (if there is a baseTramp
    // with something interesting, that is). 

    // Assertion: we have no Inst elements already
    Address addr = (*e_iter)->addr();
    relocation_cerr << "  Checking for point at " << std::hex << addr << std::dec << endl;

    point = (*iter)->bbl()->func()->findInstPByAddr(addr);
    if (!point) continue;

    relocation_cerr << "   Found instrumentation at addr " 
		    << std::hex << addr << std::dec << endl;
    
    pre = point->preBaseTramp();

    Inst::Ptr inst = Inst::create();
    if (post && !post->empty())
      inst->addBaseTramp(post);

    if (pre && !pre->empty())
      inst->addBaseTramp(pre);

    if (!inst->empty())
      elements.insert(e_iter, inst);
    // Otherwise it silently disappears...

    post = point->postBaseTramp();
    pre = NULL;
  }

  // Edge instrumentation time
  // Only the final point can have edge instrumentation;
  // this includes the postBaseTramp (for fallthrough)
  // or a targetBaseTramp (for taken edges)

  if (point) {
    relocation_cerr << "   Trailing <point>, checking edge instrumentation" << endl;
    baseTramp *target = point->targetBaseTramp();
    // post is still assigned from above
    if (!target &&
	!post) {
      relocation_cerr << "   ... neither target nor post, no edge" << endl;
      return true;
    }

    // Get the stuff we need: a CFElement for the last instruction
    CFElement::Ptr cf = dyn_detail::boost::dynamic_pointer_cast<CFElement>(elements.back());
    assert(cf);

    if (post) {
      relocation_cerr << "   ... fallthrough inst, adding" << endl;
      if (!addEdgeInstrumentation(post,
				  cf,
				  CFElement::Fallthrough,
				  *iter))
	return false;
    }
    if (target) {
      relocation_cerr << "   ... target inst, adding" << endl;
      if (!addEdgeInstrumentation(target,
				  cf,
				  CFElement::Taken,
				  *iter))
	return false;
    }
  }
  return true;
}

bool InstTransformer::postprocess(BlockList &bl) {
  // Yuck iteration... anyone have a better idea?

  relocation_cerr << "InstTransformer: postProcess" << endl;
  
  if (edgeBlocks_.empty()) {
    relocation_cerr << "  ... nothing to do, returning" << endl;
    return true;
  }

  for (BlockList::iterator iter = bl.begin();
       iter != bl.end(); ++iter) {
    relocation_cerr << "   Testing block " << iter->get() << endl;
    // Try pre-insertion
    EdgeBlocks::iterator pre = edgeBlocks_.find(std::make_pair(*iter, Before));
    if (pre != edgeBlocks_.end()) {
      relocation_cerr << "     Inserting " << pre->second.size() << " pre blocks" << endl;
      bl.insert(iter, pre->second.begin(), pre->second.end());
    }
    // And post-insertion?
    EdgeBlocks::iterator post = edgeBlocks_.find(std::make_pair(*iter, After));
    if (post != edgeBlocks_.end()) {
      relocation_cerr << "    Inserting " << post->second.size() << " post blocks" << endl;
      // Game the main iterator here...
      ++iter; // To get successor
      bl.insert(iter, post->second.begin(), post->second.end());
      // We're now one too far; back up so the for loop will
      // move us forward.
      --iter;
    }
  }
  return true;
}


bool InstTransformer::addEdgeInstrumentation(baseTramp *tramp,
					     CFElement::Ptr cf,
					     Address dest,
					     Block::Ptr cur) {
  // We handle edge instrumentation by creating a new Block and
  // wiring it in in the appropriate place. The actual adding is
  // done later, since we can't modify the list from here. 
  // What we can do is leave a marker of _where_ it should be
  // inserted. 

  Block::Ptr inst = Block::create(tramp);
  Target<Block::Ptr> *t = new Target<Block::Ptr>(inst);

  // 1) Find the appropriate successor S of the current block B
  // 2) Set the fallthrough successor of the instrumentation block I to S
  // 3) Set the appropriate successor of S to I
  // 4) Add the inst block to the "add this to the list" list.

  // 1)
  CFElement::DestinationMap::iterator d_iter = cf->destMap_.find(dest);
  // What if someone requested edge instrumentation for an edge that
  // doesn't exist? Oopsie.
  if (d_iter == cf->destMap_.end()) {
    delete t;
    return true;
  }

  // 2)
  // Keep this info for later...
  TargetInt *target = d_iter->second;

  CFElement::Ptr postCF = CFElement::create();
  postCF->addDestination(CFElement::Fallthrough, target);
  inst->elements().push_back(postCF);

  // 3)
  d_iter->second = t;
    
  
  // 4) 
  // It's more efficient branch-wise to put a block in
  // before its target rather than after this; 
  // however, if we aren't moving the target then we 
  // fall back.
  Block::Ptr insertPoint;
  Target<Block::Ptr> *targ = dynamic_cast<Target<Block::Ptr> *>(target);
  if (targ) {
    edgeBlocks_[std::make_pair(targ->t(), Before)].push_back(inst);
  }
  else {
    edgeBlocks_[std::make_pair(cur, After)].push_back(inst);
  }

  return true;
}
