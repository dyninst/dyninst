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
#include "dyninstAPI/src/debug.h"
#include "../Widgets/Widget.h"
#include "../Widgets/RelDataWidget.h"
#include "../Widgets/StackModWidget.h"
#include "../Widgets/CFWidget.h"
#include "../Widgets/PCWidget.h"
#include "../CFG/RelocBlock.h"
#include "dyninstAPI/src/addressSpace.h"
#include "instructionAPI/h/InstructionDecoder.h"
#include "../CFG/RelocGraph.h"
#include "instructionAPI/h/Visitor.h"

#include "StackMod.h"
#include "function.h"
#include "dataflowAPI/h/stackanalysis.h"

using namespace std;
using namespace Dyninst; 
using namespace Relocation;
using namespace InstructionAPI;


bool adhocMovementTransformer::process(RelocBlock *cur, RelocGraph *cfg) {
  // Identify PC-relative data accesses
  // and "get PC" operations and replace them
  // with dedicated Widgets

   RelocBlock::WidgetList &elements = cur->elements();

  relocation_cerr << "PCRelTrans: processing block (ID= "
                  << cur->id() << ") " 
		  << cur << " with "
		  << elements.size() << " elements." << endl;

#if defined(cap_stack_mods)
  // Grab some stack modification data structures if we need them
  OffsetVector* offVec = NULL;
  TMap* tMap = NULL;
  if (cur->func()->hasStackMod()) {
    // Make sure we have an offset vector and transformation mapping.  We
    // shouldn't be able to get to relocation without having generated these.
    if (!cur->func()->hasOffsetVector()) {
      cur->func()->createOffsetVector();
    }
    offVec = cur->func()->getOffsetVector();
    assert(offVec);

    if (!cur->func()->hasValidOffsetVector()) {
      // We should not be able to get here, but enforce that we don't
      return false;
    }

    tMap = cur->func()->getTMap();
    assert(tMap);

    // Analyze definitions to figure out how their displacements need to be
    // modified.
    std::map<Address, StackAccess *> *definitionMap =
      cur->func()->getDefinitionMap();
    for (auto defIter = definitionMap->begin(); defIter != definitionMap->end();
      defIter++) {
      const Address defAddr = defIter->first;
      StackAccess *defAccess = defIter->second;

      // Set up parameters for call to  isStackFrameSensitive
      Offset origDisp;
      signed long delta;
      Accesses tmpAccesses;
      tmpAccesses[defAccess->reg()].insert(defAccess);
      ParseAPI::Block *defBlock = NULL;
      const ParseAPI::Function::blocklist &blocks =
          cur->func()->ifunc()->blocks();
      for (auto blockIter = blocks.begin(); blockIter != blocks.end();
        blockIter++) {
        ParseAPI::Block *block = *blockIter;
        Address tmp;
        if (block->start() <= defAddr && defAddr < block->end() &&
          block->consistent(defAddr, tmp)) {
          defBlock = block;
          break;
        }
      }
      assert(defBlock != NULL);

      // Get origDisp, delta for this definition via isStackFrameSensitive
      stackmods_printf("Checking isStackFrameSensitive for def @ 0x%lx\n",
        defAddr);
      try {
        if (isStackFrameSensitive(origDisp, delta, &tmpAccesses, offVec, tMap,
          defBlock, defAddr)) {
          definitionDeltas[defAddr] = std::make_pair(origDisp, delta);
        }
      } catch (stackmod_exception &e) {
        // isStackFrameSensitive throws an exception if it's unsafe to modify
        // this function.
        return false;
      }
    }
  }
#endif

  for (RelocBlock::WidgetList::iterator iter = elements.begin();
       iter != elements.end(); ++iter) {
    // Can I do in-place replacement? Apparently I can...
    // first insert new (before iter) and then remove iter

    // Cache this so we don't re-decode...
    auto insn = (*iter)->insn();
    if (!insn.isValid()) continue;

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
#if defined(cap_stack_mods)
    else {
      // If we have stack modifications, check for sensitive instructions;
      // we must update the displacement encoded in these instructions
      if (cur->func()->hasStackMod() && cur->func()->getMods()->size()) {
        const Accesses* accesses = cur->func()->getAccesses((*iter)->addr());
        // If this function makes no stack accesses, no need to perform
        // analysis; there won't be any sensitive instructions
        if (!accesses) {
          continue;
        }

        // Perform check and generate StackModWidget if necessary
        Offset origDisp;
        signed long delta;
        Architecture arch = insn.getArch();
        stackmods_printf("Checking isStackFrameSensitive @ 0x%lx = %s\n",
          (*iter)->addr(), insn.format().c_str());
        try {
          if (isStackFrameSensitive(origDisp, delta, accesses, offVec, tMap,
            cur->block()->llb(), (*iter)->addr())) {
            signed long newDisp = origDisp + delta;

            stackmods_printf(" ... is Stack Frame SENSITIVE at 0x%lx\n",
              (*iter)->addr());
            stackmods_printf("\t\t origDisp = %lu, delta = %ld, newDisp = %ld\n",
              origDisp, delta, newDisp);

            relocation_cerr << " ... is Stack Frame Sensitive at "
              << std::hex << (*iter)->addr()
              << std::dec
              << ", origDisp = " << origDisp
              << ", delta = " << delta
              << ", newDisp = " << newDisp
              << endl;

            Widget::Ptr replacement = StackModWidget::create(insn,
              (*iter)->addr(), newDisp, arch);
            (*iter).swap(replacement);
          }
        } catch (stackmod_exception &e) {
          // isStackFrameSensitive throws an exception if it's unsafe to modify
          // this function.
          return false;
        }
      }
    }
#endif
  }
  return true;
}

static Address PCValue(Address addr, Instruction insn) {
    // ARM is for sure using pre-insnstruction PC value
    if (insn.getArch() == Arch_aarch64 || insn.getArch() == Arch_aarch32) {
        return addr;
    } else {
        return addr + insn.size();
    }
}

bool adhocMovementTransformer::isPCDerefCF(Widget::Ptr ptr,
                                           Instruction insn,
                                           Address &target) {
   Expression::Ptr cf = insn.getControlFlowTarget();
   if (!cf) return false;
   
//   Architecture fixme = insn->getArch();
//   if (fixme == Arch_ppc32) fixme = Arch_ppc64;
   
   Expression::Ptr thePC(new RegisterAST(MachRegister::getPC(insn.getArch())));
//   Expression::Ptr thePCFixme(new RegisterAST(MachRegister::getPC(fixme)));

   // Okay, see if we're memory
   set<Expression::Ptr> mems;
   insn.getMemoryReadOperands(mems);
   
   for (set<Expression::Ptr>::const_iterator iter = mems.begin();
        iter != mems.end(); ++iter) {
      Expression::Ptr exp = *iter;
      if (exp->bind(thePC.get(), Result(u64, PCValue(ptr->addr() , insn)))) {
	// Bind succeeded, eval to get target address
	Result res = exp->eval();
	if (!res.defined) {
	  cerr << "ERROR: failed bind/eval at " << std::hex << ptr->addr() << endl;if (insn.getControlFlowTarget()) return false;
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
                                           Instruction insn,
                                           Address &target) {
  target = 0;
  switch(insn.getCategory())
  {
      case c_CallInsn:
      case c_BranchInsn:
      case c_ReturnInsn:
          return false;
      default:
	  break;
  }
  if (insn.getControlFlowTarget()) return false;


  Expression::Ptr thePC(new RegisterAST(MachRegister::getPC(insn.getArch())));

  if (!insn.isRead(thePC))
    return false;

  // Okay, see if we're memory
  set<Expression::Ptr> mems;
  insn.getMemoryReadOperands(mems);
  insn.getMemoryWriteOperands(mems);
  for (set<Expression::Ptr>::const_iterator iter = mems.begin();
       iter != mems.end(); ++iter) {
    Expression::Ptr exp = *iter;
    if (exp->bind(thePC.get(), Result(u64, PCValue(ptr->addr(), insn)))) {
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
  insn.getOperands(operands);
  for (vector<Operand>::iterator iter = operands.begin();
       iter != operands.end(); ++iter) {
    // If we can bind the PC, then we're in the operand
    // we want.
    Expression::Ptr exp = iter->getValue();
    if (exp->bind(thePC.get(), Result(u64, PCValue(ptr->addr(), insn)))) {
      // Bind succeeded, eval to get target address
      Result res = exp->eval();
      assert(res.defined);
      target = res.convert<Address>();
      return true;
    }
  }
  if (target == 0) {
     cerr << "Error: failed to bind PC in " << insn.format() << endl;
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
                                       Instruction insn,
                                       Absloc &aloc,
                                       Address &thunk) {
  // TODO:
  // Check for call + size;
  // Check for call to thunk.
  // TODO: need a return register parameter.

   if (insn.getCategory() != InstructionAPI::c_CallInsn) return false;

  // Okay: checking for call + size
  Expression::Ptr CFT = insn.getControlFlowTarget();
  if (!CFT) {
    relocation_cerr << "      ... no CFT, ret false from isGetPC" << endl;
    return false;
  }
   

  Expression::Ptr thePC(new RegisterAST(MachRegister::getPC(insn.getArch())));

  switch(insn.getArch()) {
     case Arch_x86:
     case Arch_ppc32:
        CFT->bind(thePC.get(), Result(u32, ptr->addr()));
        break;
     case Arch_x86_64:
     case Arch_ppc64:
     case Arch_aarch64:
        CFT->bind(thePC.get(), Result(u64, ptr->addr()));
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

  if (target == (ptr->addr() + insn.size())) {
    aloc = Absloc(0, 0, NULL);
    relocation_cerr << "      ... call next insn, ret true" << endl;
    return true;
  }

  // Check for a call to a thunk function
  // TODO: replace entirely with sensitivity analysis. But for now? 
  // Yeah.
  
  // This is yoinked from arch-x86.C...
  if (addrSpace->isValidAddress(target)) {
    const unsigned char* buf = reinterpret_cast<const unsigned char*>(addrSpace->getPtrToInstruction(target));
    if (!buf) {
       cerr << "Error: illegal pointer to buffer!" << endl;
       cerr << "Target of " << hex << target << " from addr " << ptr->addr() << " in insn " << insn.format() << dec << endl;
       assert(0);
    }

    InstructionDecoder decoder(buf,
			       2*InstructionDecoder::maxInstructionLength,
			       addrSpace->getArch());

    Instruction firstInsn = decoder.decode();
    Instruction secondInsn = decoder.decode();

    relocation_cerr << "      ... decoded target insns "
		    << firstInsn.format() << ", "
		    << secondInsn.format() << endl;

    if(firstInsn.isValid() && firstInsn.getOperation().getID() == e_mov
       && firstInsn.readsMemory() && !firstInsn.writesMemory()
       && secondInsn.isValid() && secondInsn.getCategory() == c_ReturnInsn) {

      thunkVisitor visitor;
      relocation_cerr << "Checking operand "
                      << firstInsn.getOperand(1).format(firstInsn.getArch()) << endl;
      firstInsn.getOperand(1).getValue()->apply(&visitor);
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
      firstInsn.getWriteSet(writes);
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

#if defined(cap_stack_mods)
// Determines if an instruction is stack frame sensitive (i.e. needs to be
// updated with a new displacement).  If so, returns in delta the amount by
// which the displacement needs to be updated.
bool adhocMovementTransformer::isStackFrameSensitive(Offset& origDisp,
    signed long& delta,
    const Accesses* accesses,
    OffsetVector*& offVec,
    TMap*& tMap,
    ParseAPI::Block* block,
    Address addr)
{
    // Short-circuit if this instruction is a definition that we already
    // analyzed.
    if (definitionDeltas.find(addr) != definitionDeltas.end()) {
       origDisp = definitionDeltas[addr].first;
       delta = definitionDeltas[addr].second;
       return true;
    }

    delta = 0;

    assert(accesses->size() <= 1);
    if (accesses->size() == 1) {
        MachRegister curReg = accesses->begin()->first;
        const std::set<StackAccess *> &accessSet = accesses->begin()->second;
        const unsigned accessSetSize = accessSet.size();

        // Change in base register height for each possible location accessed
        StackAnalysis::Height *regDeltas =
            new StackAnalysis::Height[accessSetSize];
        // Change in access height for each possible location accessed
        StackAnalysis::Height *readDeltas =
            new StackAnalysis::Height[accessSetSize];
        // Change in definition height for each possible location accessed
        StackAnalysis::Height *defDeltas =
            new StackAnalysis::Height[accessSetSize];

        unsigned i = 0;
        for (auto iter = accessSet.begin(); iter != accessSet.end(); iter++) {
            StackAccess *access = *iter;

            // The original difference between base register height and access
            // height
            origDisp = access->disp();

            stackmods_printf("\t %s\n", access->format().c_str());

            StackAnalysis::Height regHeightPrime = access->regHeight();
            StackAnalysis::Height readHeightPrime = access->readHeight();

            stackmods_printf("\t\t regHeight = %ld, readHeight = %ld\n",
                access->regHeight().height(), access->readHeight().height());

            // Idea: Check for exact match first, then check for overlaps.  This
            // should fix the case where we add an exact match. However, find is
            // going to search for any match, so we need to look ourselves.

            bool foundReg = false;
            bool foundRead = false;

            // Find any updates to regHeight or readHeight (exact matches)
            for (auto tIter = tMap->begin(); (!foundReg || !foundRead) &&
                tIter != tMap->end(); ++tIter) {
                StackLocation* src = (*tIter).first;
                StackLocation* dest = (*tIter).second;

                stackmods_printf("\t\t\t Checking %s -> %s\n",
                    src->format().c_str(), dest->format().c_str());

                if (src->isStackMemory() && dest->isStackMemory()) {
                    if (src->isRegisterHeight()) {
                        if (src->off() == access->regHeight()) {
                            int tmp;
                            if (src->valid() &&
                                !src->valid()->find(addr, tmp)) {
                                stackmods_printf("\t\t\t\t Matching src height "
                                    "not valid for this PC\n");
                                continue;
                            }

                            if (src->reg() == curReg) {
                                foundReg = true;
                                assert(!offVec->isSkip(curReg,
                                    access->regHeight(), block->start(), addr));
                                regHeightPrime = dest->off();
                                stackmods_printf("\t\t\t\t regHeight' = %ld\n",
                                    regHeightPrime.height());
                            }
                        }
                    } else {
                        if (src->off() == access->readHeight()) {
                            int tmp;
                            if (src->valid() &&
                                !src->valid()->find(addr, tmp)) {
                                stackmods_printf("\t\t\t\t Matching src height "
                                    "not valid for this PC\n");
                                continue;
                            }

                            foundRead = true;
                            readHeightPrime = dest->off();
                            stackmods_printf("\t\t\t\t readHeight' = %ld\n",
                                readHeightPrime.height());
                        }
                    }
                } else if (src->isNull()) {
                    stackmods_printf("\t\t\t\t Ignoring: insertion\n");
                } else {
                    assert(0);
                }
            }

            // Still necessary because accesses contained inside other accesses
            // may not have an exact offset match in O.
            // Look for ranges that include the readHeight.
            for (auto tIter = tMap->begin(); !foundRead && tIter != tMap->end();
                ++tIter) {
                StackLocation* src = (*tIter).first;
                StackLocation* dest = (*tIter).second;

                stackmods_printf("\t\t\t Checking %s -> %s\n",
                    src->format().c_str(), dest->format().c_str());
                if (src->isStackMemory() && dest->isStackMemory()) {
                    if (src->isRegisterHeight()) {
                        // Nothing to do
                    } else {
                        if (src->off() < access->readHeight() &&
                            access->readHeight() < src->off() + src->size()) {
                            int tmp;
                            if (src->valid() &&
                                !src->valid()->find(addr, tmp)) {
                                stackmods_printf("\t\t\t\t Matching src height "
                                    "not valid for this PC\n");
                                continue;
                            }
                            foundRead = true;
                            readHeightPrime = dest->off() +
                                (access->readHeight() - src->off());
                            stackmods_printf("\t\t\t\t readHeight' = %ld\n",
                                readHeightPrime.height());
                        }
                    }
                }
            }

            regDeltas[i] = regHeightPrime - access->regHeight();
            readDeltas[i] = readHeightPrime - access->readHeight();

            // Check if we have a delta for this access's defintion.  Take it
            // into account if we do.
            const StackAnalysis::Definition &def = access->regDef();
            if (definitionDeltas.find(def.addr) == definitionDeltas.end()) {
                defDeltas[i] = 0;
            } else {
                defDeltas[i] = definitionDeltas[def.addr].second;
            }

            //stackmods_printf("[isStackFrameSensitive] readD: %ld, regD: %ld, "
            //   "defD: %ld\n", readDeltas[i].height(), regDeltas[i].height(),
            //   defDeltas[i].height());
            i++;
        }

        // Ensure that this access always needs the same change in displacement
        // regardless of which location is being accessed.  If accesses to
        // different locations require different displacements, we can't fix
        // this instruction for all locations.  In that case, we fail by
        // throwing an exception.
        delta = readDeltas[0].height() - regDeltas[0].height() -
            defDeltas[0].height();
        for (unsigned j = 1; j < accessSetSize; j++) {
            if (delta != readDeltas[j].height() - regDeltas[j].height() -
                defDeltas[j].height()) {
                fprintf(stderr, "Access to multiple locations is unresolvable: "
                    "different displacements required\n");
                STACKMOD_ASSERT(false);
            }
        }

        delete[] regDeltas;
        delete[] readDeltas;
        delete[] defDeltas;
    }

    return delta != 0;
}
#endif
