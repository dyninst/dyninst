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



#include "Transformer.h"
#include "Movement-analysis.h"
#include "Modification.h"
#include "dyninstAPI/src/debug.h"
#include "../Widgets/Widget.h"
#include "dyninstAPI/src/function.h"
#include "../Widgets/CFWidget.h"
#include "../Widgets/PCWidget.h"
#include "dataflowAPI/h/stackanalysis.h"
#include "dyninstAPI/src/addressSpace.h"
#include "symtabAPI/h/Symtab.h" 
#include "dyninstAPI/src/mapped_object.h"
#include "instructionAPI/h/InstructionDecoder.h"
#include "dyninstAPI/src/instPoint.h"
#include "registers/x86_regs.h"
#include "dataflowAPI/h/slicing.h"
#include "instructionAPI/h/syscalls.h"
#include "../CFG/RelocBlock.h"
#include "../CFG/RelocGraph.h"

using namespace std;
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;
using namespace SymtabAPI;

using namespace DataflowAPI;

PCSensitiveTransformer::AnalysisCache PCSensitiveTransformer::analysisCache_;

int DEBUG_hi = -1;
int DEBUG_lo = -1;

bool PCSensitiveTransformer::analysisRequired(RelocBlock *blk) {
   if ( blk->func()->obj()->parse_img()->codeObject()->defensiveMode())
      return true;
   return false;
}

bool PCSensitiveTransformer::process(RelocBlock *reloc, RelocGraph *g) {

    /* Should we run the adhoc analysis before we return? */
    bool adhoc_required = !analysisRequired(reloc);
    bool adhoc_result = false;

    /* If we need to run the adhoc analysis then run it first */
    if(adhoc_required)
        adhoc_result = adhoc.process(reloc, g);

    const block_instance *block = reloc->block();
    const func_instance *func = reloc->func();

    /* We need a block in order to do sensitivity analysis */
    if(!block)
    {
        if(adhoc_required)
        {
            // sensitivity_cerr << "Warning: No block, running adhoc: " << adhoc_result << endl;
            return adhoc_result;
        }
        else return true;
    }

    RelocBlock::WidgetList &elements = reloc->elements();
    for (RelocBlock::WidgetList::iterator iter = elements.begin();
            iter != elements.end(); ++iter) {

        // Get the instruction contained by this element; might be from
        // an original instruction (RelocInsn) or the CF wrapper (CFWidget)
        Instruction insn = (*iter)->insn();
        if (!insn.isValid()) continue;
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

        if (Dyninst::InstructionAPI::isSystemCall(insn)) {
            continue;
        }


        // sensitivity_cerr << "Instruction is sensitive @ " << hex << addr << dec << endl;

        // Optimization: before we do some heavyweight analysis, see if we can shortcut
        bool intSens = false;
        bool extSens = false;
        bool approx = false;
        Absloc dest;

        AssignList sensitiveAssignments;
        if (insnIsThunkCall(insn, addr, dest)) {
            relocation_cerr << "\tThunk @ " << hex << addr << dec << endl;
            handleThunkCall(reloc, g, iter, dest);
            intSens_++;
            extSens_++;
            thunk_++;
            continue;
        } else if (insn.getCategory() == c_CallInsn && exceptionSensitive(addr+insn.size(), block)) {
            extSens = true;
            sensitivity_cerr << "\tException sensitive @ " << hex << addr << dec << endl;
        }
        // This function also returns the sensitive assignments
        else if (isPCSensitive(insn,
                           addr,
                           func,
                           block,
                           sensitiveAssignments)) {
            Sens_++;

            if (!queryCache(block, addr, intSens, extSens)) {
                for (AssignList::iterator a_iter = sensitiveAssignments.begin();
                     a_iter != sensitiveAssignments.end(); ++a_iter) {

                    //cerr << "Forward slice from " << (*a_iter)->format() << hex << " @ " << addr << " (parse of " << (*a_iter)->addr() << dec << ") in func " << block->func()->prettyName() << endl;

                    Graph::Ptr slice = forwardSlice(*a_iter,
                                                    block->llb(),
                                                    func->ifunc());

                    if (!slice) {
                        // Safe assumption, as always
                        // sensitivity_cerr << "\t slice failed!" << endl;
                        approx = true;
                    }
                    else {
                        if (slice->size() > 10) {
                            // HACK around a problem with slice sizes
                            approx = true;
                        }
                        else if (!determineSensitivity(slice, intSens, extSens)) {
                            // Analysis failed for some reason... go conservative
                            // cerr << "\t Warning: sensitivity analysis failed!" << endl;
                            approx = true;
                        }
                        else {
                            //sensitivity_cerr << "\t sens analysis returned " << (intSens ? "intSens" : "") << " / "
                            //<< (extSens ? "extSens" : "") << endl;
                        }
                    }

                    if (approx || (intSens && extSens)) {
                        break;
                    }
                }
            }

            if (approx && !adhoc_required) {
                overApprox_++;
                intSens = true;
                extSens = true;
            } else {
                if (extSens) {
                    extSens_++;
                }
                if (intSens) {
                    intSens_++;
                }
            }
        }



        if (extSens) {
            sensitivity_cerr << "\tExtSens @ " << std::hex << addr << std::dec << endl;

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
                    handleThunkCall(reloc,
                            g,
                            iter, 
                            destination);
                    continue;
                } 
            }

            // cerr << "\tEmulating instruction..." << endl; 
            // And now to the real work. Replace this instruction with an emulation sequence

            emulateInsn(reloc,
                    g,
                    iter, 
                    insn, 
                    addr);
        }
        
        if(!adhoc_required)
        {
            // sensitivity_cerr << "\tRunning cache analysis..." << endl; 
            cacheAnalysis(block, addr, intSens, extSens);
        }
    }

    /* do we still have to run the adhoc analysis? */
    if(adhoc_required)
    {
        // bool adhoc_result = adhoc.process(reloc, g);
        // sensitivity_cerr << "Completing analysis with adhoc process: " << adhoc_result << endl;
        return adhoc_result;
    }

    /* return success */
    return true;
}

bool PCSensitiveTransformer::isPCSensitive(Instruction insn,
                                           Address addr,
                                           const func_instance *func,
                                           const block_instance *block,
                                           AssignList &sensitiveAssignments) {
  if (!(insn.getOperation().getID() == e_call)) return false;
    if(func->obj()->hybridMode() == BPatch_normalMode) return false;
  // FIXME for loopnz instruction
  Absloc pc = Absloc::makePC(func->ifunc()->isrc()->getArch());

  // Crack open the instruction and see who uses PC...
  std::vector<Assignment::Ptr> assignments;
  aConverter.convert(insn,
		     func->addrToOffset(addr),
		     func->ifunc(),
			 block->llb(),
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
	//relocation_cerr << insn->format() << " @" << hex << addr << dec << " is PCsens" << endl;
	sensitiveAssignments.push_back(*a_iter);
      }
    }
  }
  return !sensitiveAssignments.empty();
}

class M_A_Predicates : public Slicer::Predicates {
public:
  bool haveWidened;

  M_A_Predicates() : haveWidened(false) {}
  virtual ~M_A_Predicates() {}

  virtual bool endAtPoint (Assignment::Ptr p) {
    if (p->out().absloc().isPC()) return true;
	return false;
  }

  virtual bool widenAtPoint (Assignment::Ptr p) {
    if (p->out().type() != Absloc::Unknown) {
      haveWidened = true;
      return true;
    }
	if (p->out().containsOfType(Absloc::Register) &&
		!p->out().absloc().isPC()) {
			haveWidened = true;
			return true;
	}
	return false;
  }

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
    
    parse_func *f = static_cast<parse_func *>(func);
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
						parse_block *block,
						parse_func *func) {
  M_A_Predicates pred;
  Slicer slicer(ptr, block, func, false, false);

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
     SliceNode::Ptr aNode = boost::static_pointer_cast<SliceNode>(*exitBegin);

    // By definition, a widen point is potentially behavior changing.
    if (Slicer::isWidenNode(*exitBegin)) {
      // cerr << "\t\t isWidenNode!" << endl;
      return false;
    }
    
    AST::Ptr ast = results[aNode->assign()];
    if (ast == AST::Ptr()) {
      // cerr << "\t\t Symbolic expansion failed" << endl;
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
bool PCSensitiveTransformer::insnIsThunkCall(Instruction insn,
                                             Address addr,
                                             Absloc &destination) {
  // Should be able to handle this much more efficiently by following the CFG
  if (insn.getCategory() != c_CallInsn) {
    return false;
  }
  Expression::Ptr CFT = insn.getControlFlowTarget();
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
    //relocation_cerr << "      ... CFT not evallable, ret false from isGetPC" << endl;
    return false;
  }

  Address target = res.convert<Address>();

  // Check for a call to a thunk function
  if (target == (addr + insn.size())) {
    destination = Absloc(0, 0, NULL);
    return true;
  }
  
  // This is yoinked from arch-x86.C...
  if (addrSpace->isValidAddress(target)) {

    const unsigned char* buf = reinterpret_cast<const unsigned char*>(addrSpace->getPtrToInstruction(target));

    InstructionDecoder decoder(buf,
			       2*InstructionDecoder::maxInstructionLength,
			       addrSpace->getArch());

    Instruction firstInsn = decoder.decode();
    Instruction secondInsn = decoder.decode();
    //relocation_cerr << "      ... decoded target insns "
		    //<< firstInsn->format() << ", " 
		    //<< secondInsn->format() << endl;

    if(firstInsn.isValid() && firstInsn.getOperation().getID() == e_mov
       && firstInsn.readsMemory() && !firstInsn.writesMemory()
       && secondInsn.isValid() && secondInsn.getCategory() == c_ReturnInsn) {

      // Check to be sure we're reading memory
      std::set<RegisterAST::Ptr> reads;
      firstInsn.getReadSet(reads);
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
      firstInsn.getWriteSet(writes);
      assert(writes.size() == 1);
      destination = Absloc((*(writes.begin()))->getID());
      return true;
    }
  }
  return false;
}

void PCSensitiveTransformer::handleThunkCall(RelocBlock *reloc,
                                             RelocGraph *cfg,
					     RelocBlock::WidgetList::iterator &iter,
					     Absloc &destination) {

   Widget::Ptr replacement = PCWidget::create((*iter)->insn(),
					   (*iter)->addr(),
					   destination);

  // This is kind of complex. We don't want to just pull the getPC
  // because it also might end the basic block. If that happens we
  // need to pull the fallthough element out of the CFWidget so
  // that we don't hork control flow. What a pain.
  if ((*iter) != reloc->elements().back()) {
    // Easy case; no worries.
    (*iter).swap(replacement);
  }
  else {
    // There are two types of thunks we deal with;
    // one is a call to a thunk function (which we want to skip), 
    // and the second is a call forward within the same function.
    // The function call variant we want to replace with a (faked)
    // fallthrough to wherever the call returned to. The jump
    // equivalent doesn't _have_ a fallthrough, so we want to use
    // the taken edge instead.

     Predicates::Interprocedural pred;
     bool removed = cfg->removeEdge(pred, reloc->outs());
     assert(removed);
     (*iter).swap(replacement);
  }
}

void PCSensitiveTransformer::emulateInsn(RelocBlock *reloc,
                                         RelocGraph *cfg,
                                         RelocBlock::WidgetList::iterator &iter,
                                         Instruction insn,
                                         Address addr) {
  //cerr << "Emulating @" << std::hex << addr << std::dec  << endl;
  // We emulate calls by replacing them with push/jump combinations. The jump will be handled
  // by a CFWidget, so we just need a "push" (and then to create everything else).

  if(insn.getOperation().getID() != e_call) {
        // emulating a non-call is a no-op
      return;
  }

  // Construct a new Widget that will emulate the original instruction here. 
  static Absloc stack_loc(0, 0, NULL);
  Widget::Ptr replacement = PCWidget::create(insn, addr, stack_loc);

  // Okay, now wire this in as appropriate.
  if ((*iter) != reloc->elements().back()) {
    //cerr << "... middle of block" << endl;
    // Easy case; no worries.
    // This is the case for call+5s...
    (*iter).swap(replacement);
  }
  else {
    //cerr << "... end of block" << endl;
     CFWidget::Ptr cf = boost::dynamic_pointer_cast<CFWidget>(*iter);
     // We don't want to be doing this pre-CF-creation...
     assert(cf); 
    
    // Add the <push> part of this whole thing...
    (*iter).swap(replacement);
    
    // And put the CF back in
    reloc->elements().push_back(cf);
    // But it's not a call anymore, it's a jump
    cf->clearIsCall();

    // And skip it
    ++iter;

#if 0
    // Remove all non-call edges
    // Replace a call edge with a taken edge
    Predicates::NonCall pred;
    cfg->removeEdge(pred, reloc->outs());
#endif

    Predicates::Call pred2;
    cfg->changeType(pred2, reloc->outs(), ParseAPI::DIRECT);
  }
}

/**
 * Check to see if the given address is exception sensitive. If the address is
 * in a function that contains a catch block, true is returned. Otherwise false
 * is returned.
 * @return Whether or not the given address is exception senstive.
 */
bool PCSensitiveTransformer::exceptionSensitive(Address a, const block_instance *bbl) {
    return false;
    sensitivity_cerr << "Checking address 0x" << std::hex << a << std::dec 
        << " for exception sensitivity" << endl;

    Symtab *symtab = bbl->obj()->parse_img()->getObject();
    Offset off = a - (bbl->start() - bbl->llb()->start());

    sensitivity_cerr << "Address: 0x" << hex << a << " Offset: 0x" << hex << a << dec << endl;

    sensitivity_cerr << "Checking offset 0x" << hex << off << dec << endl;

    /* Get all of the exceptions in this symbol table */
    std::vector<ExceptionBlock*> exceptions;
    if(!symtab->getAllExceptions(exceptions))
    {
        sensitivity_cerr << "\tWarning: There aren't any exceptions in this symtab!" << endl;
        return false;
    }

    /* Get the function that owns this address */
    Function* function = NULL;
    if(!symtab->getContainingFunction(off, function))
        return false;

    /* If there is no function for this instruction, we can't do analysis */
    if(!function)
    {
        sensitivity_cerr << "\tERROR: Cannot do sensitivity analysis " << 
           "on instruction not in function." << endl;
        return false;
    }

    /* Get the ranges for the function*/
    const FuncRangeCollection& ranges = function->getRanges();

    /* See if any try block intersect this function */
    for(auto e_iter = exceptions.begin();e_iter != exceptions.end();e_iter++)
    {
        ExceptionBlock* exception = *e_iter;

        Offset ts = exception->tryStart();
        Offset te = exception->tryEnd();

        sensitivity_cerr << "\tts: 0x" << hex << ts << "  te: 0x" << te << dec << endl;

        /* Check to see if any of the ranges overlap with the try region */
        for(auto r_iter = ranges.begin();r_iter != ranges.end();r_iter++)
        {
            const FuncRange& r = *r_iter;

            sensitivity_cerr << "\t\tstart: 0x" << hex << r.low() 
                << " end: 0x" << r.high() << dec << endl;

            if((r.low() <= ts && r.high() >= ts)
                    || (r.low() < te && r.high() >= te))
            {
                sensitivity_cerr << "\t\t\tWithin range <<<<<<<<<<<<<<" << endl;
                return true;
            } else {
                sensitivity_cerr << "\t\t\tNot Within range." << endl;
            }
        }


    }

    return false;
}

void PCSensitiveTransformer::cacheAnalysis(const block_instance *bbl, Address addr, bool intSens, bool extSens) {
   analysisCache_[bbl][addr] = std::make_pair(intSens, extSens);
}

bool PCSensitiveTransformer::queryCache(const block_instance *bbl, Address addr, bool &intSens, bool &extSens) {
	//intSens = true;
	//extSens = true;
	//return true;
	AnalysisCache::const_iterator iter = analysisCache_.find(bbl);
   if (iter == analysisCache_.end()) return false;
   CacheEntry::const_iterator iter2 = iter->second.find(addr);
   if (iter2 == iter->second.end()) return false;
   intSens = iter2->second.first;
   extSens = iter2->second.second;
   return true;
}

void PCSensitiveTransformer::invalidateCache(const block_instance *b) {
   // Clear everything corresponding to an addr in the block;
   // overapproximation for shared functions and shared blocks,
   // but hey. 
	analysisCache_.erase(b);
}

void PCSensitiveTransformer::invalidateCache(func_instance *f) {
   // We want to invalidate any cache results for f directly,
   // as well as any for blocks that call f. 

   const PatchFunction::Blockset &blocks = f->blocks();
   for (PatchFunction::Blockset::const_iterator iter = blocks.begin();
        iter != blocks.end(); ++iter) {
      invalidateCache(SCAST_BI(*iter));
   }
   
   // Get callers of this function
   PatchAPI::PatchBlock::edgelist edges = f->entry()->sources();
   for (PatchAPI::PatchBlock::edgelist::iterator iter = edges.begin(); iter != edges.end(); ++iter) {
      invalidateCache(SCAST_BI((*iter)->src()));
   }
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
  diffs_.push(DiffVar((int)c->val().val, 0));
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
