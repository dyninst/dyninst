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
// x86-specific methods for generating control flow

#include "CFWidget.h"
#include "Widget.h"
#include "Target.h"

#include "instructionAPI/h/Instruction.h"

#include "../dyninstAPI/src/debug.h"

#include "../CodeTracker.h"
#include "../CodeBuffer.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/registerSpace.h"

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

CFPatch::CFPatch(Type a,
                 InstructionAPI::Instruction::Ptr b,
                 TargetInt *c,
                 Address d) :
   type(a), orig_insn(b), target(c), origAddr_(d) {
   if (b)
      ugly_insn = new NS_x86::instruction(b->ptr());
   else
      ugly_insn = NULL;
   // New branches don't get an original instruction...
}


bool CFPatch::apply(codeGen &gen, CodeBuffer *buf) {
   // Question 1: are we doing an inter-module static control transfer?
   // If so, things get... complicated
   if (isPLT(gen)) {
      if (!applyPLT(gen, buf)) {
         cerr << "Failed to apply patch (PLT req'd)" << endl;
         return false;
      }
      return true;
   }

   // Otherwise this is a classic, and therefore easy.
   int targetLabel = target->label(buf);

   relocation_cerr << "\t\t CFPatch::apply(" 
		   << hex << this << dec << "), type " << type << ", origAddr " << hex << origAddr_ 
                   << ", and label " << dec << targetLabel << endl;
   if (orig_insn) {
      relocation_cerr << "\t\t\t Currently at " << hex << gen.currAddr() << " and targeting predicted " << buf->predictedAddr(targetLabel) << dec << endl;
      switch(type) {
         case CFPatch::Jump: {
            relocation_cerr << "\t\t\t Generating CFPatch::Jump from " 
                            << hex << gen.currAddr() << " to " << buf->predictedAddr(targetLabel) << dec << endl;
            if (!insnCodeGen::modifyJump(buf->predictedAddr(targetLabel), *ugly_insn, gen)) {
               cerr << "Failed to modify jump" << endl;
               return false;
            }
            return true;
         }
         case CFPatch::JCC: {
            relocation_cerr << "\t\t\t Generating CFPatch::JCC from " 
                            << hex << gen.currAddr() << " to " << buf->predictedAddr(targetLabel) << dec << endl;            
            if (!insnCodeGen::modifyJcc(buf->predictedAddr(targetLabel), *ugly_insn, gen)) {
               cerr << "Failed to modify conditional jump" << endl;
               return false;
            }
            return true;            
         }
         case CFPatch::Call: {
            if (!insnCodeGen::modifyCall(buf->predictedAddr(targetLabel), *ugly_insn, gen)) {
               cerr << "Failed to modify call" << endl;
               return false;
            }
            return true;
         }
         case CFPatch::Data: {
            if (!insnCodeGen::modifyData(buf->predictedAddr(targetLabel), *ugly_insn, gen)) {
               cerr << "Failed to modify data" << endl;
               return false;
            }
            return true;
         }
      }
   }
   else {
      switch(type) {
         case CFPatch::Jump:
            insnCodeGen::generateBranch(gen, gen.currAddr(), buf->predictedAddr(targetLabel));
            break;
         case CFPatch::Call:
            insnCodeGen::generateCall(gen, gen.currAddr(), buf->predictedAddr(targetLabel));
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
      cerr << "Target type is " << target->type() << ", not block target" << endl;
      return false;
   }
   // And can only handle calls right now. That's a TODO...
   if (type != Call &&
       type != Jump) {
      cerr << "Attempt to make PLT of type " << type << " and can only handle calls or jumps" << endl;
      return false;
   }

   Target<block_instance *> *t = static_cast<Target<block_instance *> *>(target);
   block_instance *tb = t->t();

   // We can (for now) only jump to functions
   func_instance *callee = tb->entryOfFunc();
   if (!callee) {
      cerr << "No callee, ret false" << endl;
      return false;
   }

   // We need a registerSpace for this. For now, assume we're at
   // a call boundary (as that's _really_ the only place we can
   // be for now) and set it to the optimistic register space.
   gen.setRegisterSpace(registerSpace::optimisticRegSpace(gen.addrSpace()));

   if (type == Call) 
      gen.codeEmitter()->emitPLTCall(callee, gen);
   else if (type == Jump)
      gen.codeEmitter()->emitPLTJump(callee, gen);
   else
      assert(0);

   return true;
}

