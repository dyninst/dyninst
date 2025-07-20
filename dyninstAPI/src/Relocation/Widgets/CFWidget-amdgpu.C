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
#include "dyninstAPI/src/emit-amdgpu.h"
#include "dyninstAPI/src/registerSpace.h"


using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

using namespace NS_amdgpu;

bool CFWidget::generateIndirect(CodeBuffer & /* buffer */,
                                Register,
                                const RelocBlock * /* trace */,
                                Instruction /* insn */) {
        return true;
}


bool CFWidget::generateIndirectCall(CodeBuffer &/* buffer */,
                                    Register /*reg*/,
                                    Instruction /* insn */,
                                    const RelocBlock * /* trace */,
                                    Address /*origAddr*/) {
    return true;
}

bool CFPatch::apply(codeGen &gen, CodeBuffer *buf) {
   int targetLabel = target->label(buf);

   relocation_cerr << "\t\t CFPatch::apply, type " << type << ", origAddr " << hex << origAddr_
                   << ", and label " << dec << targetLabel << endl;
   if (orig_insn.isValid()) {
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
            assert(false && "CFPatch::apply for CFPatch::Call isn't implemented yet");
        }
         case CFPatch::Data: {
            assert(false && "CFPatch::apply for CFPatch::Data isn't implemented yet");
         }
      }
   }
   else {
      switch(type) {
         case CFPatch::Jump:
            insnCodeGen::generateBranch(gen, gen.currAddr(), buf->predictedAddr(targetLabel));
            break;
         default:
            assert(0);
      }
   }
   return true;
   // std::cerr << "in CFPatch::apply\n";
   // switch(type) {
   //   case CFPatch::Jump:
   //    std::cerr << "jump patch\n";
   //    break;
   //   case CFPatch::JCC:
   //    std::cerr << "jcc patch\n";
   //    break;
   //   case CFPatch::Call:
   //    std::cerr << "call patch\n";
   //    break;
   //   case CFPatch::Data:
   //    std::cerr << "data patch\n";
   //    break;
   // }
   //  return true;
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

/*
bool CFPatch::needsTOCUpdate() {
	assert(0);
  return true;
}

bool CFPatch::handleTOCUpdate(codeGen &gen) {
	assert(0);
	return false;
}
*/

