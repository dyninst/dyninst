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
#include "Movement-adhoc.h"
#include "../patchapi_debug.h"
#include "../Atoms/Atom.h"
#include "../Atoms/RelDataAtom.h"
#include "../Atoms/CFAtom.h"
#include "../Atoms/PCAtom.h"
#include "../Atoms/Trace.h"
#include "dyninstAPI/src/addressSpace.h"
#include "instructionAPI/h/InstructionDecoder.h"

using namespace std;
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;


bool adhocMovementTransformer::processTrace(TraceList::iterator &b_iter, const TraceMap &) {
  // Identify PC-relative data accesses
  // and "get PC" operations and replace them
  // with dedicated Atoms

   Trace::AtomList &elements = (*b_iter)->elements();

  relocation_cerr << "PCRelTrans: processing block " 
		  << (*b_iter).get() << " with "
		  << elements.size() << " elements." << endl;

  for (Trace::AtomList::iterator iter = elements.begin();
       iter != elements.end(); ++iter) {
    // Can I do in-place replacement? Apparently I can...
    // first insert new (before iter) and then remove iter

    relocation_cerr << "    "
		    << std::hex << (*iter)->addr() << std::dec << endl;

    if (!((*iter)->insn())) continue;

    Address target = 0;
    Absloc aloc;

    if (isPCDerefCF(*iter, target)) {
       CFAtom::Ptr cf = dyn_detail::boost::dynamic_pointer_cast<CFAtom>(*iter);
       assert(cf);
       cf->setOrigTarget(target);
    }
    if (isPCRelData(*iter, target)) {
      relocation_cerr << "  ... isPCRelData at " 
		      << std::hex << (*iter)->addr() << std::dec << endl;
      // Two options: a memory reference or a indirect call. The indirect call we 
      // just want to set target in the CFAtom, as it has the hardware to handle
      // control flow. Generic memory references get their own atoms. How nice. 
      Atom::Ptr replacement = RelDataAtom::create((*iter)->insn(),
                                                     (*iter)->addr(),
                                                     target);
      (*iter).swap(replacement);
      
    }
    else if (isGetPC(*iter, aloc, target)) {

      Atom::Ptr replacement = PCAtom::create((*iter)->insn(),
					       (*iter)->addr(),
					       aloc,
					       target);
      // This is kind of complex. We don't want to just pull the getPC
      // because it also might end the basic block. If that happens we
      // need to pull the fallthough element out of the CFAtom so
      // that we don't hork control flow. What a pain.
      if ((*iter) != elements.back()) {
	// Easy case; no worries.
	(*iter).swap(replacement);
      }
      else {
         // Remove the taken edge from the trace, as we're removing
         // the call and replacing it with a GetPC operation. 
         bool removed = (*b_iter)->removeTargets(ParseAPI::CALL);
         assert(removed);
            
         // Before we forget: swap in the GetPC for the current element
         (*iter).swap(replacement);            
         break;
      }
    }
  }
  return true;
}

bool adhocMovementTransformer::isPCDerefCF(Atom::Ptr ptr,
                                           Address &target) {
   Expression::Ptr cf = ptr->insn()->getControlFlowTarget();
   if (!cf) return false;

   static Expression::Ptr x86PC(new RegisterAST(MachRegister::getPC(Arch_x86)));
   static Expression::Ptr x86PC64(new RegisterAST(MachRegister::getPC(Arch_x86_64)));
   static Expression::Ptr ppcPC(new RegisterAST(MachRegister::getPC(Arch_ppc32)));
   static Expression::Ptr ppcPC64(new RegisterAST(MachRegister::getPC(Arch_ppc64)));

   // Okay, see if we're memory
   set<Expression::Ptr> mems;
   ptr->insn()->getMemoryReadOperands(mems);
   
   for (set<Expression::Ptr>::const_iterator iter = mems.begin();
        iter != mems.end(); ++iter) {
      Expression::Ptr exp = *iter;
      if (exp->bind(x86PC.get(), Result(u32, ptr->addr() + ptr->insn()->size())) ||
          exp->bind(x86PC64.get(), Result(u64, ptr->addr() + ptr->insn()->size())) ||
          exp->bind(ppcPC.get(), Result(u64, ptr->addr() + ptr->insn()->size())) ||
          exp->bind(ppcPC64.get(), Result(u64, ptr->addr() + ptr->insn()->size()))) {
	// Bind succeeded, eval to get target address
	Result res = exp->eval();
	if (!res.defined) {
	  cerr << "ERROR: failed bind/eval at " << std::hex << ptr->addr() << endl;if (ptr->insn()->getControlFlowTarget()) return false;
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
bool adhocMovementTransformer::isPCRelData(Atom::Ptr ptr,
                                           Address &target) {
  target = 0;
  if (ptr->insn()->getControlFlowTarget()) return false;

  // TODO FIXME
  static Expression::Ptr x86PC(new RegisterAST(MachRegister::getPC(Arch_x86)));
  static Expression::Ptr x86PC64(new RegisterAST(MachRegister::getPC(Arch_x86_64)));
  static Expression::Ptr ppcPC(new RegisterAST(MachRegister::getPC(Arch_ppc32)));
  static Expression::Ptr ppcPC64(new RegisterAST(MachRegister::getPC(Arch_ppc64)));
  
  if (!ptr->insn()->isRead(x86PC) &&
      !ptr->insn()->isRead(x86PC64) &&
      !ptr->insn()->isRead(ppcPC) &&
      !ptr->insn()->isRead(ppcPC64))
    return false;

  // Okay, see if we're memory
  set<Expression::Ptr> mems;
  ptr->insn()->getMemoryReadOperands(mems);
  ptr->insn()->getMemoryWriteOperands(mems);
  for (set<Expression::Ptr>::const_iterator iter = mems.begin();
       iter != mems.end(); ++iter) {
    Expression::Ptr exp = *iter;
    cerr << "Memory-using PC reference: expression " << exp->format() << endl;
    if (exp->bind(x86PC.get(), Result(u32, ptr->addr() + ptr->insn()->size())) ||
	exp->bind(x86PC64.get(), Result(u64, ptr->addr() + ptr->insn()->size())) ||
	exp->bind(ppcPC.get(), Result(u32, ptr->addr() + ptr->insn()->size())) ||
	exp->bind(ppcPC64.get(), Result(u64, ptr->addr() + ptr->insn()->size()))) {
      // Bind succeeded, eval to get target address
      Result res = exp->eval();
      if (!res.defined) {
	cerr << "ERROR: failed bind/eval at " << std::hex << ptr->addr() << endl;
        continue;
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
    if (exp->bind(x86PC.get(), Result(u32, ptr->addr() + ptr->insn()->size())) ||
	exp->bind(x86PC64.get(), Result(u64, ptr->addr() + ptr->insn()->size())) ||
	exp->bind(ppcPC.get(), Result(u32, ptr->addr() + ptr->insn()->size())) ||
	exp->bind(ppcPC64.get(), Result(u64, ptr->addr() + ptr->insn()->size()))) {
      // Bind succeeded, eval to get target address
      Result res = exp->eval();
      assert(res.defined);
      target = res.convert<Address>();
      break;
    }
  }
  if (target == 0) {
     cerr << "Error: failed to bind PC in " << ptr->insn()->format() << endl;
  }
  assert(target != 0);
  return true;    
}

bool adhocMovementTransformer::isGetPC(Atom::Ptr ptr,
				       Absloc &aloc,
				       Address &thunk) {
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
  static Expression::Ptr x86PC(new RegisterAST(MachRegister::getPC(Arch_x86)));
  static Expression::Ptr x86PC64(new RegisterAST(MachRegister::getPC(Arch_x86_64)));
  static Expression::Ptr ppcPC(new RegisterAST(MachRegister::getPC(Arch_ppc32)));
  static Expression::Ptr ppcPC64(new RegisterAST(MachRegister::getPC(Arch_ppc64)));

  // Bind the IP, why not...
  CFT->bind(x86PC.get(), Result(u32, ptr->addr()));
  CFT->bind(x86PC64.get(), Result(u64, ptr->addr()));
  CFT->bind(ppcPC.get(), Result(u32, ptr->addr()));
  CFT->bind(ppcPC64.get(), Result(u64, ptr->addr()));

  Result res = CFT->eval();

  if (!res.defined) {
    relocation_cerr << "      ... CFT not evallable, ret false from isGetPC" << endl;
    return false;
  }

  Address target = res.convert<Address>();

  if (target == (ptr->addr() + ptr->insn()->size())) {
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
       cerr << "Target of " << hex << target << " from addr " << ptr->addr() << " in insn " << ptr->insn()->format() << dec << endl;
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
      aloc = Absloc((*(writes.begin()))->getID());
      thunk = target;
      return true;
    }
  }
  else {
    relocation_cerr << "      ... not call thunk, ret false" << endl;
  }
  return false;
}
