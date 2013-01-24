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
#include "Movement-adhoc.h"
#include "../dyninstAPI/src/debug.h"
#include "../Widgets/Widget.h"
#include "../Widgets/RelDataWidget.h"
#include "../Widgets/CFWidget.h"
#include "../Widgets/PCWidget.h"
#include "../CFG/RelocBlock.h"
#include "dyninstAPI/src/addressSpace.h"
#include "instructionAPI/h/InstructionDecoder.h"
#include "../CFG/RelocGraph.h"
#include "instructionAPI/h/Visitor.h"


using namespace std;
using namespace Dyninst; 
using namespace Relocation;
using namespace InstructionAPI;


bool adhocMovementTransformer::process(RelocBlock *cur, RelocGraph *cfg) {
  // Identify PC-relative data accesses
  // and "get PC" operations and replace them
  // with dedicated Widgets

   RelocBlock::WidgetList &elements = cur->elements();

  relocation_cerr << "PCRelTrans: processing block " 
		  << cur << " with "
		  << elements.size() << " elements." << endl;

  for (RelocBlock::WidgetList::iterator iter = elements.begin();
       iter != elements.end(); ++iter) {
    // Can I do in-place replacement? Apparently I can...
    // first insert new (before iter) and then remove iter

    // Cache this so we don't re-decode...
    InsnPtr insn = (*iter)->insn();

    if (!insn) continue;

    Address target = 0;
    Absloc aloc;

    if (isPCDerefCF(*iter, insn, target)) {
       CFWidget::Ptr cf = boost::dynamic_pointer_cast<CFWidget>(*iter);
       assert(cf);
       cf->setOrigTarget(target);
    }
    if (isPCRelData(*iter, insn, target)) {
      relocation_cerr << "  ... isPCRelData at " 
		      << std::hex << (*iter)->addr() << std::dec << endl;
      // Two options: a memory reference or a indirect call. The indirect call we 
      // just want to set target in the CFWidget, as it has the hardware to handle
      // control flow. Generic memory references get their own atoms. How nice. 
      Widget::Ptr replacement = RelDataWidget::create(insn,
                                                     (*iter)->addr(),
                                                     target);
      (*iter).swap(replacement);
      
    }
    else if (isGetPC(*iter, insn, aloc, target)) {

      Widget::Ptr replacement = PCWidget::create(insn,
					       (*iter)->addr(),
					       aloc,
					       target);
      // This is kind of complex. We don't want to just pull the getPC
      // because it also might end the basic block. If that happens we
      // need to pull the fallthough element out of the CFWidget so
      // that we don't hork control flow. What a pain.
      if ((*iter) != elements.back()) {
	// Easy case; no worries.
	(*iter).swap(replacement);
      }
      else {
         // Remove the taken edge from the trace, as we're removing
         // the call and replacing it with a GetPC operation. 
         Predicates::Interprocedural pred;
         bool removed = cfg->removeEdge(pred, cur->outs());
         assert(removed);
            
         // Before we forget: swap in the GetPC for the current element
         (*iter).swap(replacement);            
         break;
      }
    }
  }
  return true;
}

bool adhocMovementTransformer::isPCDerefCF(Widget::Ptr ptr,
                                           InsnPtr insn,
                                           Address &target) {
   Expression::Ptr cf = insn->getControlFlowTarget();
   if (!cf) return false;
   
   Architecture fixme = insn->getArch();
   if (fixme == Arch_ppc32) fixme = Arch_ppc64;
   
   Expression::Ptr thePC(new RegisterAST(MachRegister::getPC(insn->getArch())));
   Expression::Ptr thePCFixme(new RegisterAST(MachRegister::getPC(fixme)));

   // Okay, see if we're memory
   set<Expression::Ptr> mems;
   insn->getMemoryReadOperands(mems);
   
   for (set<Expression::Ptr>::const_iterator iter = mems.begin();
        iter != mems.end(); ++iter) {
      Expression::Ptr exp = *iter;
      if (exp->bind(thePC.get(), Result(u64, ptr->addr() + insn->size())) ||
          exp->bind(thePCFixme.get(), Result(u64, ptr->addr() + insn->size()))) {
	// Bind succeeded, eval to get target address
	Result res = exp->eval();
	if (!res.defined) {
	  cerr << "ERROR: failed bind/eval at " << std::hex << ptr->addr() << endl;if (insn->getControlFlowTarget()) return false;
	}
	assert(res.defined);
	target = res.convert<Address>();
	break;
      }
   }
   if (target) return true;
   return false;
}



// We define this as "uses PC and is not control flow"
bool adhocMovementTransformer::isPCRelData(Widget::Ptr ptr,
                                           InsnPtr insn,
                                           Address &target) {
  target = 0;
  if (insn->getControlFlowTarget()) return false;

  Architecture fixme = insn->getArch();
  if (fixme == Arch_ppc32) fixme = Arch_ppc64;
  
  Expression::Ptr thePC(new RegisterAST(MachRegister::getPC(insn->getArch())));
  Expression::Ptr thePCFixme(new RegisterAST(MachRegister::getPC(fixme)));

  if (!insn->isRead(thePC) &&
      !insn->isRead(thePCFixme))
    return false;

  // Okay, see if we're memory
  set<Expression::Ptr> mems;
  insn->getMemoryReadOperands(mems);
  insn->getMemoryWriteOperands(mems);
  for (set<Expression::Ptr>::const_iterator iter = mems.begin();
       iter != mems.end(); ++iter) {
    Expression::Ptr exp = *iter;
    if (exp->bind(thePC.get(), Result(u64, ptr->addr() + insn->size())) ||
	exp->bind(thePCFixme.get(), Result(u64, ptr->addr() + insn->size()))) {
      // Bind succeeded, eval to get target address
      Result res = exp->eval();
      if (!res.defined) {
	cerr << "ERROR: failed bind/eval at " << std::hex << ptr->addr() << endl;
        continue;
      }
      assert(res.defined);
      target = res.convert<Address>();
      return true;
    }
  }

  // Didn't use the PC to read memory; thus we have to grind through
  // all the operands. We didn't do this directly because the 
  // memory-topping deref stops eval...
  vector<Operand> operands;
  insn->getOperands(operands);
  for (vector<Operand>::iterator iter = operands.begin();
       iter != operands.end(); ++iter) {
    // If we can bind the PC, then we're in the operand
    // we want.
    Expression::Ptr exp = iter->getValue();
    if (exp->bind(thePC.get(), Result(u64, ptr->addr() + insn->size())) ||
	exp->bind(thePCFixme.get(), Result(u64, ptr->addr() + insn->size()))) {
      // Bind succeeded, eval to get target address
      Result res = exp->eval();
      assert(res.defined);
      target = res.convert<Address>();
      return true;
    }
  }
  if (target == 0) {
     cerr << "Error: failed to bind PC in " << insn->format() << endl;
  }
  assert(target != 0);
  return true;    
}

class thunkVisitor : public InstructionAPI::Visitor
{
public:
  thunkVisitor() : isThunk(true), foundSP(false), foundDeref(false) {}
  virtual ~thunkVisitor() {}

  bool isThunk;
  bool foundSP;
  bool foundDeref;

  virtual void visit(BinaryFunction*)
  {
    relocation_cerr << "\t binfunc, ret false" << endl;
    isThunk = false;
  }

  virtual void visit(Immediate*)
  {
    relocation_cerr << "\t imm, ret false" << endl;
    isThunk = false;
  }
  virtual void visit(RegisterAST* r)
  {
    if (foundSP) {
      isThunk = false;
    }
    else if (!r->getID().isStackPointer()) {
      isThunk = false;
    }
    else {
      foundSP = true;
    }
  }
  virtual void visit(Dereference* )
  {
    if (foundDeref) {
      isThunk = false;
    }
    if (!foundSP) {
      isThunk = false;
    }
    foundDeref = true;
  }  
};


bool adhocMovementTransformer::isGetPC(Widget::Ptr ptr,
                                       InsnPtr insn,
				       Absloc &aloc,
				       Address &thunk) {
  // TODO:
  // Check for call + size;
  // Check for call to thunk.
  // TODO: need a return register parameter.

   if (insn->getCategory() != InstructionAPI::c_CallInsn) return false;

  // Okay: checking for call + size
  Expression::Ptr CFT = insn->getControlFlowTarget();
  if (!CFT) {
    relocation_cerr << "      ... no CFT, ret false from isGetPC" << endl;
    return false;
  }
   
  Architecture fixme = insn->getArch();
  if (fixme == Arch_ppc32) fixme = Arch_ppc64;

  Expression::Ptr thePC(new RegisterAST(MachRegister::getPC(insn->getArch())));
  Expression::Ptr thePCFixme(new RegisterAST(MachRegister::getPC(fixme)));

  switch(insn->getArch()) {
     case Arch_x86:
     case Arch_ppc32:
        CFT->bind(thePC.get(), Result(u32, ptr->addr()));
        CFT->bind(thePCFixme.get(), Result(u32, ptr->addr()));
        break;
     case Arch_x86_64:
     case Arch_ppc64:
        CFT->bind(thePC.get(), Result(u64, ptr->addr()));
        CFT->bind(thePCFixme.get(), Result(u64, ptr->addr()));
        break;
     default:
        assert(0);
        break;
  }

  Result res = CFT->eval();

  if (!res.defined) {
    relocation_cerr << "      ... CFT not evallable, ret false from isGetPC" << endl;
    return false;
  }

  Address target = res.convert<Address>();

  if (target == (ptr->addr() + insn->size())) {
    aloc = Absloc(0, 0, NULL);
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
    if (!buf) {
       cerr << "Error: illegal pointer to buffer!" << endl;
       cerr << "Target of " << hex << target << " from addr " << ptr->addr() << " in insn " << insn->format() << dec << endl;
       assert(0);
    }

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

      thunkVisitor visitor;
      relocation_cerr << "Checking operand " << firstInsn->getOperand(1).format(firstInsn->getArch()) << endl;
      firstInsn->getOperand(1).getValue()->apply(&visitor);
      if (!visitor.isThunk) return false;

#if 0
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

#endif
      
      std::set<RegisterAST::Ptr> writes;
      firstInsn->getWriteSet(writes);
      assert(writes.size() == 1);
      aloc = Absloc((*(writes.begin()))->getID());
      thunk = target;
      return true;
    }
  }
  else {     
     relocation_cerr << "\t Call to " << hex << target << " is not valid address, concluding not thunk" << dec << endl;
  }
  return false;
}
