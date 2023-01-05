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
// ppc-specific methods for generating control flow

#include "CFWidget.h"
#include "Widget.h"
#include "../CFG/RelocTarget.h"

#include "instructionAPI/h/Instruction.h"

#include "dyninstAPI/src/debug.h"

#include "../CodeTracker.h"
#include "../CodeBuffer.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/registerSpace.h"

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

using namespace NS_power;

bool CFWidget::generateIndirect(CodeBuffer &buffer,
                              Register,
                              const RelocBlock *trace,
                              Instruction insn) {
  // Copying an indirect jump; unlike x86 we don't do
  // call -> indirect conversion yet. 
  // ... though that would be really freaking easy. 

  NS_power::instruction ugly_insn(insn.ptr());
  IFORM_LK_SET(ugly_insn, 0);
  codeGen gen(4);
  insnCodeGen::generate(gen, ugly_insn);

  // TODO don't ignore the register parameter
   buffer.addPIC(gen, tracker(trace));

   return true;
}



bool CFWidget::generateIndirectCall(CodeBuffer &buffer,
                                    Register /*reg*/,
                                  Instruction insn,
                                  const RelocBlock *trace,
                                    Address /*origAddr*/) 
{
  NS_power::instruction ugly_insn(insn.ptr());
  IFORM_LK_SET(ugly_insn, 1);
  codeGen gen(4);
  insnCodeGen::generate(gen, ugly_insn);

  // TODO don't ignore the register parameter
   buffer.addPIC(gen, tracker(trace));

   return true;
}

bool CFPatch::apply(codeGen &gen, CodeBuffer *buf) {

  if (needsTOCUpdate()) {
     relocation_cerr << "\t\t\t isSpecialCase..." << endl;
     gen.setFunction(const_cast<func_instance *>(func));
     if (!handleTOCUpdate(gen)) {
       relocation_cerr << "TOC special case handling in PPC64 failed" << endl;
       return false;
     }
     return true;
   }

   // Question: are we doing an inter-module static control transfer?
   // If so, things get... complicated
   if (isPLT(gen)) {
     relocation_cerr << "\t\t\t isPLT..." << endl;
      if (!applyPLT(gen, buf)) {
	relocation_cerr << "PLT special case handling in PPC64" << endl;
         return false;
      }
      return true;
   }

   // Otherwise this is a classic, and therefore easy.
   int targetLabel = target->label(buf);
   Address targetAddr = buf->predictedAddr(targetLabel);

   relocation_cerr << "\t\t CFPatch::apply, type " << type << ", origAddr " << hex << origAddr_ 
                   << ", and label " << dec << targetLabel << endl;

   if (orig_insn.isValid()) {
      relocation_cerr << "\t\t\t Currently at " << hex << gen.currAddr() << " and targeting predicted " << targetAddr << dec << endl;
      switch(type) {
         case CFPatch::Jump: {
            relocation_cerr << "\t\t\t Generating CFPatch::Jump from " 
                            << hex << gen.currAddr() << " to " << targetAddr << dec << endl;
            if (!insnCodeGen::modifyJump(targetAddr, *ugly_insn, gen)) {
	      relocation_cerr << "modifyJump failed, ret false" << endl;
               return false;
            }
            return true;
         }
         case CFPatch::JCC: {
            relocation_cerr << "\t\t\t Generating CFPatch::JCC from " 
                            << hex << gen.currAddr() << " to " << targetAddr << dec << endl;            
            if (!insnCodeGen::modifyJcc(targetAddr, *ugly_insn, gen)) {
	      relocation_cerr << "modifyJcc failed, ret false" << endl;
               return false;
            }
            return true;            
         }
         case CFPatch::Call: {
            // Special handling for function call replacement:
            //
            // Here we are certain that we are dealing with
            // an intra-module call. For PIE code, the global entry of 
            // the callee will use R12 to set up R2. Since we do not
            // set R12 to be the global entry, we should use the local entry 
            if (target->type() == TargetInt::BlockTarget) {
                Target<block_instance *> *t = static_cast<Target<block_instance *> *>(target);
                block_instance *tb = t->t();
                func_instance *callee = tb->entryOfFunc();
                if (callee->ifunc()->containsPowerPreamble() && callee->addr() == targetAddr) targetAddr += 8;
            }

            if (!insnCodeGen::modifyCall(targetAddr, *ugly_insn, gen)) {
	      relocation_cerr << "modifyCall failed, ret false" << endl;
               return false;
            }
            return true;
         }
         case CFPatch::Data: {
            if (!insnCodeGen::modifyData(targetAddr, *ugly_insn, gen)) {
	      relocation_cerr << "modifyData failed, ret false" << endl;
               return false;
            }
            return true;
         }
      }
   }
   else {
      switch(type) {
         case CFPatch::Jump:
            insnCodeGen::generateBranch(gen, gen.currAddr(), targetAddr);
            break;
         case CFPatch::Call:
            insnCodeGen::generateCall(gen, gen.currAddr(), targetAddr);
            break;
         default:
            assert(0);
      }
   }
   
   return true;
}

bool CFPatch::isPLT(codeGen &gen) {
   if (!gen.addrSpace()->edit()) return false;

   // We need to PLT if we're in two different 
   // AddressSpaces - the current codegen and
   // the target.

   // First check the target type.
   if (target->type() != TargetInt::BlockTarget) {
      // Either a RelocBlock (which _must_ be local)
      // or an Address (which has to be local to be
      // meaningful); neither reqs PLT
      return false;
   }

   Target<block_instance *> *t = static_cast<Target<block_instance *> *>(target);
   block_instance *tb = t->t();
   if (tb->proc() != gen.addrSpace())
      return true;
   else
      return false;
}

bool CFPatch::applyPLT(codeGen &gen, CodeBuffer *) {
   // We should try and keep any prefixes that were on the instruction. 
   // However... yeah, right. I'm not that good with x86. So instead
   // I'm copying the code from emitCallInstruction...
      
   if (target->type() != TargetInt::BlockTarget) {
      return false;
   }
   // And can only handle calls right now. That's a TODO...
   if (type != Call &&
       type != Jump) {
      return false;
   }

   relocation_cerr << "\t\t\t ApplyPLT..." << endl;

   Target<block_instance *> *t = static_cast<Target<block_instance *> *>(target);
   block_instance *tb = t->t();

   // Set caller in codegen structure
   gen.setFunction(const_cast<func_instance *>(func));

   // We can (for now) only jump to functions
   func_instance *callee = tb->entryOfFunc();
   if (!callee) {
      return false;
   }

   // We need a RegisterSpace for this. Amusingly,
   // we want to use the RegisterSpace corresponding to the
   // entry of the callee, as it doesn't matter what's live
   // here. 
   instPoint *calleeEntry = instPoint::funcEntry(callee);
   gen.setRegisterSpace(registerSpace::actualRegSpace(calleeEntry));

   if (type == Call) 
      gen.codeEmitter()->emitPLTCall(callee, gen);
   else if (type == Jump)
      gen.codeEmitter()->emitPLTJump(callee, gen);
   else
      assert(0);

   return true;
}

bool CFPatch::needsTOCUpdate() {
  // 64-bit check
  if (func->proc()->getAddressWidth() != 8) return false;

  // See if this is inter-module, according to the target
  // and gen
  // Assuming an address target is not inter-module...
  if (target->type() != TargetInt::BlockTarget) return false;

  Target<block_instance *> *t = static_cast<Target<block_instance *> *>(target);
  // If we're in the same object, then we don't need to update TOC
  if (t->t()->obj() == func->obj()) return false;

  return true;
}

bool CFPatch::handleTOCUpdate(codeGen &gen) {
  // Annoying, pain in the butt case...

  assert(target->type() == TargetInt::BlockTarget);
  Target<block_instance *> *t = static_cast<Target<block_instance *> *>(target);


  if (type == Jump)
    return gen.codeEmitter()->emitTOCJump(t->t(), gen);
  else if (type == Call)
    return gen.codeEmitter()->emitTOCCall(t->t(), gen);
  else {
    assert(0);
    return false;
  }
}
