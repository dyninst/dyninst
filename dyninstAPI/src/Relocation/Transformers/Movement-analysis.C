/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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



#include "Transformer.h"
#include "Movement-analysis.h"
#include "dyninstAPI/src/debug.h"
#include "../Atoms/Atom.h"
#include "dyninstAPI/src/function.h"
#include "../Atoms/CFAtom.h"
#include "../Atoms/GetPC.h"
#include "dataflowAPI/h/stackanalysis.h"
#include "dyninstAPI/src/addressSpace.h"
#include "symtabAPI/h/Symtab.h" 
#include "dyninstAPI/src/mapped_object.h"
#include "instructionAPI/h/InstructionDecoder.h"

#include "dataflowAPI/h/slicing.h"

using namespace std;
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;
using namespace SymtabAPI;

using namespace DataflowAPI;

bool PCSensitiveTransformer::postprocess(TraceList &) {
   sensitivity_cerr << dec << "Sensitive count: " << Sens_ << ", failed " << overApprox_ << ", ext " << extSens_ << ", int " << intSens_ << ", thunk " << thunk_ << endl;
  return true;
}

int DEBUG_hi = -1;
int DEBUG_lo = -1;

bool PCSensitiveTransformer::analysisRequired(TraceList::iterator &b_iter) {
   if ( (*b_iter)->func()->obj()->parse_img()->codeObject()->defensiveMode())
      return true;
   return false;
}

bool PCSensitiveTransformer::processTrace(TraceList::iterator &b_iter) {
#if 0
   if (!analysisRequired(b_iter)) {
      return adhoc.processTrace(b_iter);
   }
#endif

   const bblInstance *bbl = (*b_iter)->bbl();
  
  // Can be true if we see an instrumentation block...
  if (!bbl) return true;
  
  Trace::AtomList &elements = (*b_iter)->elements();
  for (Trace::AtomList::iterator iter = elements.begin();
       iter != elements.end(); ++iter) {

    // Get the instruction contained by this element; might be from
    // an original instruction (RelocInsn) or the CF wrapper (CFAtom)
    Instruction::Ptr insn = (*iter)->insn();
    if (!insn) continue;
    Address addr = (*iter)->addr();


    // We want to identify all PC-sensitive instructions and 
    // determine whether they are externally sensitive; that is, 
    // whether they will misbehave (heh) given the new structure
    // of the binary. 
    //
    // An instruction is PC sensitive if it is moved and uses the 
    // PC. We further subdivide PCsens instructions into two categories:
    // trivially PC sensitive instructions that use the PC to define the
    // PC and non-trivial PC sensitive instructions (all others). 
    // 
    // Since our CF localization will entirely handle the trivial category
    // we focus here on the non-trivial category. Thus we're looking for 
    // two things:
    // 1) Is this instruction PC sensitive - does it use the PC and define
    // a non-PC location.
    // 2) Is it externally sensitive... will this instruction cause the program
    // to produce a different result.

    if (isSyscall(insn, addr)) {
      continue;
    }
    
    AssignList sensitiveAssignments;
    // This function also returns the sensitive assignments
    if (!isPCSensitive(insn,
		       addr,
		       (*b_iter)->bbl()->func(),
		       sensitiveAssignments)) {
      //cerr << "Instruction " << insn->format() << " not PC sensitive, skipping" << endl;
      continue;
    }

    Sens_++;

    sensitivity_cerr << "Instruction is sensitive @ " << hex << addr << dec << endl;

    // Optimization: before we do some heavyweight analysis, see if we can shortcut
    bool intSens = false;
    bool extSens = false;
    bool approx = false;
    Absloc dest;

    if (insnIsThunkCall(insn, addr, dest)) {
      relocation_cerr << "Thunk @ " << hex << addr << dec << endl;
      handleThunkCall(b_iter, iter, dest);
      intSens_++;
      extSens_++;
      thunk_++;
      continue;
    }

    if (exceptionSensitive(addr+insn->size(), bbl)) {
      extSens = true;
      sensitivity_cerr << "Sensitive by exception @ " << hex << addr << dec << endl;
    }

    for (AssignList::iterator a_iter = sensitiveAssignments.begin();
	 a_iter != sensitiveAssignments.end(); ++a_iter) {

       sensitivity_cerr << "Forward slice from " << (*a_iter)->format() << " in func " << bbl->func()->prettyName() << endl;

      Graph::Ptr slice = forwardSlice(*a_iter,
				      bbl->block()->llb(),
				      bbl->func()->ifunc());
      if (!slice) {
         // Safe assumption, as always
         sensitivity_cerr << "\t slice failed!" << endl;
        approx = true;
      }
      else {
         if (slice->size() > 10) {
// HACK around a problem with slice sizes
            approx = true;
         }
         else if (!determineSensitivity(slice, intSens, extSens)) {
	  // Analysis failed for some reason... go conservative
            sensitivity_cerr << "\t sensitivity analysis failed!" << endl;
            approx = true;
         }
         else {
            sensitivity_cerr << "\t sens analysis returned " << (intSens ? "intSens" : "") << " / " 
                             << (extSens ? "extSens" : "") << endl;
         }
      }

      if (approx || (intSens && extSens)) {
	break; 
      }
    }

    if (approx) {
       overApprox_++;
       intSens = true;
       extSens = true;
    }
    else {
       if (extSens) {
          extSens_++;
       }
       if (intSens) {
          intSens_++;
       }
    }
  

    if (extSens) {
      //cerr << "ExtSens @ " << std::hex << addr << std::dec << endl;
      // Okay, someone wants the original version. That means, for now, we're emulating.
      if (intSens) {
	// Fun for the whole family! We have one instruction that wants the changed
	// version (likely a load or equivalent) and one instruction that wants the
	// original value (that would be a return). Let's see if we can match a 
	// thunk call...
	Absloc destination;
	if (insnIsThunkCall(insn, addr, destination)) {
	  // A first example of a group transformation. The "internal" piece comes from
	  // calling a 2-instruction function that copies the return address elsewhere
	  // and returns. So we can remove the internal sensitivity by inlining the 
	  // call.
	  handleThunkCall(b_iter,
			  iter, 
			  destination);
	  continue;
	}
	else {
	  // Not a thunk call, and both externally and internally sensitive. Ugh. 
	  // Well, because of the external sensitivity we're going to emulate the
	  // original instruction, which means the internally sensitive target will
	  // be transferring back to the original instruction address. Go ahead and
	  // record this...
	  recordIntSensitive(addr+insn->size());
	}
      }
      // And now to the real work. Replace this instruction with an emulation sequence
      emulateInsn(b_iter,
		  iter, 
		  insn, 
		  addr);
    }
  }
  return true;
}

bool PCSensitiveTransformer::isPCSensitive(Instruction::Ptr insn,
					   Address addr,
					   int_function *func,
					   AssignList &sensitiveAssignments) {
  Absloc pc = Absloc::makePC(func->ifunc()->isrc()->getArch());

  // Crack open the instruction and see who uses PC...
  std::vector<Assignment::Ptr> assignments;
  aConverter.convert(insn,
		     func->addrToOffset(addr),
		     func->ifunc(),
		     assignments);
  for (std::vector<Assignment::Ptr>::iterator a_iter = assignments.begin();
       a_iter != assignments.end(); ++a_iter) {
    // Assignments that define PC are not sensitive
    if ((*a_iter)->out().contains(pc))
      continue;
    
    // Do you use PC?
    const std::vector<AbsRegion> &ins = (*a_iter)->inputs();
    for (std::vector<AbsRegion>::const_iterator i = ins.begin();
	 i != ins.end(); ++i) {
      if (i->contains(pc)) {
	relocation_cerr << insn->format() << " @" << hex << addr << dec << " is PCsens" << endl;
	sensitiveAssignments.push_back(*a_iter);
      }
    }
  }
  return !sensitiveAssignments.empty();
}

class M_A_Predicates : public Slicer::Predicates {
public:
  bool haveWidened;

  M_A_Predicates() : haveWidened(false) {};
  virtual ~M_A_Predicates() {};

  virtual bool endAtPoint (Assignment::Ptr p) {
    if (p->out().absloc().isPC()) return true;
    return false;
  }

  virtual bool widenAtPoint (Assignment::Ptr p) {
    if (p->out().absloc() == Absloc()) {
      haveWidened = true;
      return true;
    }
    return false;
  };

  virtual bool widenAtAssignment(const AbsRegion &search, const AbsRegion &found) {
    if (search != found) {
      haveWidened = true;
      return true;
    }
    return false;
  }
  
  virtual bool followCall (ParseAPI::Function *func, 
			   std::stack<std::pair<ParseAPI::Function *, int> > &cs,
			   AbsRegion a) {
    // If we're looking for a stack slot and entering
    // a grand-callee, call it quits. 
    // TODO: it'd be nice to have some way of saying 
    // "an absloc in the grandparent's stack frame", 
    // rather than just "on the stack". Since what happens if
    // this is a stack-passed parameter? Ugh...

    if (haveWidened) {
      return false;
    }
    
    image_func *f = static_cast<image_func *>(func);
    if (f && f->isPLTFunction()) {
      // Don't bother following
      return false;
    }
    
    // Let's try to figure out some sort of reasonable way
    // to not worry about going into a grandparent stack frame...
    if (cs.size() > 1) {
      if (a.absloc().type() == Absloc::Stack) {
         return false;
      }
    }
    return true;
  }
};

Graph::Ptr PCSensitiveTransformer::forwardSlice(Assignment::Ptr ptr,
						image_basicBlock *block,
						image_func *func) {
  M_A_Predicates pred;
  Slicer slicer(ptr, block, func);

  Graph::Ptr g = slicer.forwardSlice(pred);
  return g;
}
		     

// Examine a slice to determine whether any of its terminal nodes
// will cause the program to produce a different value. As a secondary,
// divide terminal nodes into the set that will produce a different value
// (pos) and those that will not (neg).

bool PCSensitiveTransformer::determineSensitivity(Graph::Ptr slice,
						  bool &internal,
						  bool &external) {

  // Step 1: get a symbolic expansion of each node in the slice
  DataflowAPI::Result_t results;
  DataflowAPI::SymEval::expand(slice, results);

  // Step 2: iterate over each exit node in the slice
  NodeIterator exitBegin, exitEnd;
  slice->exitNodes(exitBegin, exitEnd);

  for (; exitBegin != exitEnd; ++exitBegin) {
    SliceNode::Ptr aNode = dyn_detail::boost::dynamic_pointer_cast<SliceNode>(*exitBegin);
    assert(aNode);

    // By definition, a widen point is potentially behavior changing.
    if (Slicer::isWidenNode(*exitBegin)) {
      return false;
    }
    
    AST::Ptr ast = results[aNode->assign()];
    if (ast == AST::Ptr()) {
      //cerr << "\t\t Symbolic expansion failed" << endl;
      return false;
    }

    // Now for the real work - determine the difference in values produced by this
    // tree for the changed IP. 
    // TODO: we need either a symbolic "moved by" number or we need to know
    // where we're moving it. I think I prefer symbolic. For now: 42!
    ExtPCSensVisitor v(aNode->assign()->out());

    if (v.isExtSens(ast)) {
      //cerr << "\t\t is externally sensitive" << endl;
      //cerr << "\t\t\t @ " << hex << aNode->addr() << dec << endl;
      external = true;
    }
    else {
      //cerr << "\t\t is internally sensitive" <<endl;
      internal = true;
    }
  }
  return true;
}

// An example of a group transformation. If this is a call to a thunk
// function then record both that (as in return true) and where the return
// address gets put...
bool PCSensitiveTransformer::insnIsThunkCall(InstructionAPI::Instruction::Ptr insn,
					     Address addr,
					     Absloc &destination) {
  // Should be able to handle this much more efficiently by following the CFG

  Expression::Ptr CFT = insn->getControlFlowTarget();
  if (!CFT) {
    return false;
  }
   
  // Bind current PC
  static Expression::Ptr thePC(new RegisterAST(MachRegister::getPC(Arch_x86)));
  static Expression::Ptr thePC64(new RegisterAST(MachRegister::getPC(Arch_x86_64)));

  // Bind the IP, why not...
  CFT->bind(thePC.get(), Result(u32, addr));
  CFT->bind(thePC64.get(), Result(u64, addr));

  Result res = CFT->eval();

  if (!res.defined) {
    relocation_cerr << "      ... CFT not evallable, ret false from isGetPC" << endl;
    return false;
  }

  Address target = res.convert<Address>();

  // Check for a call to a thunk function
  if (target == (addr + insn->size())) {
    destination = Absloc(0, 0, NULL);
    return true;
  }
  
  // This is yoinked from arch-x86.C...
  if (addrSpace->isValidAddress(target)) {

    // Get us an instrucIter    
    const unsigned char* buf = reinterpret_cast<const unsigned char*>(addrSpace->getPtrToInstruction(target));

    InstructionDecoder decoder(buf,
			       2*InstructionDecoder::maxInstructionLength,
			       addrSpace->getArch());

    Instruction::Ptr firstInsn = decoder.decode();
    Instruction::Ptr secondInsn = decoder.decode();
    relocation_cerr << "      ... decoded target insns "
		    << firstInsn->format() << ", " 
		    << secondInsn->format() << endl;

    if(firstInsn && firstInsn->getOperation().getID() == e_mov
       && firstInsn->readsMemory() && !firstInsn->writesMemory()
       && secondInsn && secondInsn->getCategory() == c_ReturnInsn) {

      // Check to be sure we're reading memory
      std::set<RegisterAST::Ptr> reads;
      firstInsn->getReadSet(reads);
      bool found = false;
      for (std::set<RegisterAST::Ptr>::iterator iter = reads.begin();
	   iter != reads.end(); ++iter) {
	if ((*iter)->getID().isStackPointer()) {
	  found = true;
	  break;
	}
      }

      if (!found) return false;
      
      std::set<RegisterAST::Ptr> writes;
      firstInsn->getWriteSet(writes);
      assert(writes.size() == 1);
      destination = Absloc((*(writes.begin()))->getID());
      return true;
    }
  }
  return false;
}

void PCSensitiveTransformer::handleThunkCall(TraceList::iterator &b_iter, 
					     Trace::AtomList::iterator &iter,
					     Absloc &destination) {

  Atom::Ptr replacement = GetPC::create((*iter)->insn(),
					   (*iter)->addr(),
					   destination);

  // This is kind of complex. We don't want to just pull the getPC
  // because it also might end the basic block. If that happens we
  // need to pull the fallthough element out of the CFAtom so
  // that we don't hork control flow. What a pain.
  if ((*iter) != (*b_iter)->elements().back()) {
    // Easy case; no worries.
    (*iter).swap(replacement);
  }
  else {
    CFAtom::Ptr cf = dyn_detail::boost::dynamic_pointer_cast<CFAtom>(*iter);
    // We don't want to be doing this pre-CF-creation...
    assert(cf); 
    
    // There are two types of thunks we deal with;
    // one is a call to a thunk function (which we want to skip), 
    // and the second is a call forward within the same function.
    // The function call variant we want to replace with a (faked)
    // fallthrough to wherever the call returned to. The jump
    // equivalent doesn't _have_ a fallthrough, so we want to use
    // the taken edge instead.

    // Ignore a taken edge, but create a new CFAtom with the
    // required fallthrough edge
    CFAtom::DestinationMap::iterator dest = cf->destMap_.find(CFAtom::Fallthrough);
    if (dest == cf->destMap_.end()) {
      dest = cf->destMap_.find(CFAtom::Taken);
    }
    if (dest != cf->destMap_.end()) {
      CFAtom::Ptr newCF = CFAtom::create((*b_iter)->bbl());
      newCF->updateAddr(cf->addr());
      // Explicitly do _NOT_ reuse old information - this is just a branch
      
      newCF->destMap_[CFAtom::Fallthrough] = dest->second;
      
      // And since we delete destMap_ elements, NUKE IT from the original!
      cf->destMap_.erase(dest);
      
      // Before we forget: swap in the GetPC for the current element
      (*iter).swap(replacement);
      
      (*b_iter)->elements().push_back(newCF);
      // Go ahead and skip it...
      iter++;
    }
  }
}

void PCSensitiveTransformer::recordIntSensitive(Address addr) {
  // All we have from this is a raw address. Suck...
  // Look up the bblInstances that map to this address. 
  std::vector<int_function *> funcs;
  addrSpace->findFuncsByAddr(addr, funcs);

  for (unsigned i = 0; i < funcs.size(); ++i) {
    int_basicBlock *block = funcs[i]->findBlockByAddr(addr);
    priMap[block->origInstance()] = Required;
  }
}

void PCSensitiveTransformer::emulateInsn(TraceList::iterator &b_iter,
					 Trace::AtomList::iterator &iter,
					 InstructionAPI::Instruction::Ptr insn,
					 Address addr) {
  //cerr << "Emulating @" << std::hex << addr << std::dec  << endl;
  // We emulate calls by replacing them with push/jump combinations. The jump will be handled
  // by a CFAtom, so we just need a "push" (and then to create everything else).  

  assert(insn->getOperation().getID() == e_call);

  // Construct a new Atom that will emulate the original instruction here. 
  static Absloc stack_loc(0, 0, NULL);
  Atom::Ptr replacement = GetPC::create(insn, addr, stack_loc);

  // Okay, now wire this in as appropriate.
  if ((*iter) != (*b_iter)->elements().back()) {
    //cerr << "... middle of block" << endl;
    // Easy case; no worries.
    // This is the case for call+5s...
    (*iter).swap(replacement);
  }
  else {
    //cerr << "... end of block" << endl;
    CFAtom::Ptr cf = dyn_detail::boost::dynamic_pointer_cast<CFAtom>(*iter);
    // We don't want to be doing this pre-CF-creation...
    assert(cf); 
    
    // This is a call, so there are two options: direct (known) or indirect.
    // Direct, we just put in a push/jump <dest> combination.

    // Indirect, we put in a push/jump <reg> combination.

    CFAtom::Ptr newCF = CFAtom::create((*b_iter)->bbl());
    newCF->updateInfo(cf);

    CFAtom::DestinationMap::iterator dest = cf->destMap_.find(CFAtom::Taken);
    if (dest != cf->destMap_.end()) {
      // Explicitly do _NOT_ reuse old information - this is just a branch
      
      newCF->destMap_[CFAtom::Taken] = dest->second;
      
      // And since we delete destMap_ elements, NUKE IT from the original!
      cf->destMap_.erase(dest);
    }
    else {
      // Indirect!
      // So we want to use the current CFelement, but strip the "call" part of it
      // to turn it into an indirect branch.

      newCF->updateInsn(insn);
      newCF->updateAddr(addr);
      if (!newCF->isIndirect_) { 
	// WTF???
	cerr << "Error: unknown insn " << insn->format() 
	     << std::hex << "@" << addr << dec << endl;

	for (CFAtom::DestinationMap::iterator foo = cf->destMap_.begin();
	     foo != cf->destMap_.end(); ++foo) {
	  //cerr << hex << foo->first << " -> " << foo->second->addr() << dec << endl;
	}

      }

      //assert(newCF->isIndirect_);
      newCF->isCall_ = false;
    }

    //cerr << "Swapping in replacement" << endl;
    // Before we forget: swap in the GetPC for the current element
    (*iter).swap(replacement);
    
    //cerr << "And adding new CF" << endl;
    (*b_iter)->elements().push_back(newCF);
    // Go ahead and skip it...
    iter++;
  }
}

// TODO: fix t
bool PCSensitiveTransformer::exceptionSensitive(Address a, const bblInstance *bbl) {
  // If we're within the try section of an exception, return true.
  // Otherwise return false.
  
  // First, convert a to an offset and dig out the Symtab of the
  // block we're looking at.
  int_function *func = bbl->func();
  Offset o = func->addrToOffset(a);
  Symtab *symtab = func->obj()->parse_img()->getObject();

  ExceptionBlock eBlock;
  // Amusingly, existence is sufficient for us.
  return symtab->findException(eBlock, o);
}

ExtPCSensVisitor::ExtPCSensVisitor(const AbsRegion &a) :
  isExtSens_(false) {
  if (a.absloc().isPC()) {
    assignPC_ = true;
  }
  else {
    assignPC_ = false;
  }
}

AST::Ptr ExtPCSensVisitor::visit(AST *a) {
  // Should never be able to get this
  isExtSens_ = true;
  return a->ptr();
}

AST::Ptr ExtPCSensVisitor::visit(BottomAST *b) {
  isExtSens_ = true;
  return b->ptr();
}

AST::Ptr ExtPCSensVisitor::visit(ConstantAST *c) {
  diffs_.push(DiffVar(c->val().val, 0));
  return c->ptr();
}

AST::Ptr ExtPCSensVisitor::visit(VariableAST *v) {
  const AbsRegion &reg = v->val().reg;
  const Absloc &aloc = reg.absloc();
  if (aloc.isPC()) {
    // Right on!
    diffs_.push(DiffVar(0, 1));
  }
  else {
    diffs_.push(DiffVar(v->val(), 0));
  }
  return v->ptr();
}

AST::Ptr ExtPCSensVisitor::visit(StackAST *s) {
  // If we see one of these we're getting a weird "pc + esp", 
  // so we can consider it a constant.
  if (s->val().isBottom()) {
    isExtSens_ = true;
  }
  else {
    diffs_.push(DiffVar(s->val().height(), 0));
  }
  return s->ptr();
}

AST::Ptr ExtPCSensVisitor::visit(RoseAST *r) {
  // Abort (ish) if we're already sensitive
  if (isExtSens_) return r->ptr();
  
  // Simplify children to the stack. 
  // Discard the pointers because we really don't care.
  // Go backwards so that the stack order matches the child order.
  // (that is, child 1 on top)
  for (unsigned i = r->numChildren(); i > 0; --i) {
    r->child(i-1)->accept(this);
  }
  // Again, if we've concluded we're externally sensitive
  // then return immediately.
  if (isExtSens_) return r->ptr();

  // Okay, let's see what's goin' on...
  switch(r->val().op) {
  case ROSEOperation::derefOp: {
    // A dereference is a decision point: either we're externally
    // sensitive (if the calculated difference depends on the PC at all)
    // or we reset the difference to 0.
    if (diffs_.top().b != 0) {
      isExtSens_ = true;
    }
    // Ignore the other entries... might be conditional loads, etc.
    for (unsigned i = 0; i < r->numChildren(); i++) {
      diffs_.pop();
    }
    // A non-modified dereference resets our "what's the difference" to 0. 
    diffs_.push(DiffVar(0, 0));
    
    break;
  }
  case ROSEOperation::addOp: {
    DiffVar sum(0,0);
    for (unsigned i = 0; i < r->numChildren(); ++i) {
      sum += diffs_.top(); diffs_.pop();
    }
    diffs_.push(sum);
    break;
  }
  case ROSEOperation::invertOp: {
    diffs_.top() *= -1;
    break;
  }
  case ROSEOperation::extendMSBOp:
  case ROSEOperation::extractOp: {
    DiffVar tmp = diffs_.top();
    for (unsigned i = 0; i < r->numChildren(); ++i) {
      diffs_.pop();
    }
    diffs_.push(tmp);
    break;
  }
  case ROSEOperation::equalToZeroOp:
    if (diffs_.top().b != 0) {
      isExtSens_ = true;
    }
    for (unsigned i = 0; i < r->numChildren(); ++i) {
      diffs_.pop();
    }
    diffs_.push(DiffVar(0, 0));
    break;
  case ROSEOperation::ifOp: {
    DiffVar c = diffs_.top(); diffs_.pop();
    DiffVar t = diffs_.top(); diffs_.pop();
    DiffVar e = diffs_.top(); diffs_.pop();

    if (c.b != 0) {
      isExtSens_ = true;
    }
    if (assignPC_) {
      if ((t.b != 1) ||
	  (e.b != 1)) { 
	isExtSens_ = true;
      }
    }
    else { 
      if ((t.b != 0) ||
	  (e.b != 0)) {
	isExtSens_ = true;
      }
    }
    // Pick one and propagate it up
    // Should split the analysis here... but this situation never actually
    // appears, so it seems silly to code for it.
    diffs_.push(t);

    break;
  }
  default:
    for (unsigned i = 0; i < r->numChildren(); i++) {
      if (diffs_.top().b != 0) {
	isExtSens_ = true;
      }
      diffs_.pop();
    }
    diffs_.push(DiffVar(0, 0));
    break;
  }
  return r->ptr();
}

bool ExtPCSensVisitor::isExtSens(AST::Ptr a) {
  a->accept(this);

  // Simplify...
  if (isExtSens_) return true;

  assert(diffs_.size() == 1);

  // By my model, we are externally sensitive if:
  //   def defines pc: diff != delta
  //   def defines _: diff != 0
  // Since I did the visitor over a set of linear variables of the
  // form a + b*delta, we can ignore a (as those will cancel) 
  // and return if b != 1.

  if (assignPC_) {
    return (diffs_.top().b != 1);
  }
  else {
    return (diffs_.top().b != 0);
  }
}

bool PCSensitiveTransformer::isSyscall(InstructionAPI::Instruction::Ptr insn, Address addr) {
  // call *%gs:0x10
  // Build a GS
  static Expression::Ptr x86_gs(new RegisterAST(x86::gs));

  if (insn->isRead(x86_gs)) {
    relocation_cerr << "Skipping syscall " << insn->format() << hex << "@ " << addr << dec << endl;
    return true;
  }
  return false;
}
