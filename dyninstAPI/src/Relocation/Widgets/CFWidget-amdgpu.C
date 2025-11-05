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

// Control flow patching for AMDGPU

#include "../CFG/RelocTarget.h"
#include "../CodeBuffer.h"
#include "CFWidget.h"
#include "dyninstAPI/src/debug.h"

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

using namespace NS_amdgpu;

bool CFWidget::generateIndirect(CodeBuffer & /* buffer */, Register, const RelocBlock * /* trace */,
                                Instruction /* insn */) {
  // Called by CFWidget::generate
  return true;
}

bool CFWidget::generateIndirectCall(CodeBuffer & /* buffer */, Register /*reg*/,
                                    Instruction /* insn */, const RelocBlock * /* trace */,
                                    Address /*origAddr*/) {
  // Called by CFWidget::generate
  return true;
}

bool CFPatch::apply(codeGen &gen, CodeBuffer *buf) {
  int targetLabel = target->label(buf);

  relocation_cerr << "\t\t CFPatch::apply, type " << type << ", origAddr " << hex << origAddr_
                  << ", and label " << dec << targetLabel << endl;
  if (orig_insn.isValid()) {
    relocation_cerr << "\t\t\t Currently at " << hex << gen.currAddr()
                    << " and targeting predicted " << buf->predictedAddr(targetLabel) << dec
                    << endl;
    switch (type) {
    case CFPatch::Jump: {
      relocation_cerr << "\t\t\t Generating CFPatch::Jump from " << hex << gen.currAddr() << " to "
                      << buf->predictedAddr(targetLabel) << dec << endl;
      if (!insnCodeGen::modifyJump(buf->predictedAddr(targetLabel), *ugly_insn, gen)) {
        cerr << "Failed to modify jump" << endl;
        return false;
      }
      return true;
    }
    case CFPatch::JCC: {
      relocation_cerr << "\t\t\t Generating CFPatch::JCC from " << hex << gen.currAddr() << " to "
                      << buf->predictedAddr(targetLabel) << dec << endl;
      if (!insnCodeGen::modifyJcc(buf->predictedAddr(targetLabel), *ugly_insn, gen)) {
        cerr << "Failed to modify conditional jump" << endl;
        return false;
      }
      return true;
    }
    case CFPatch::Call: {
      assert(!"CFPatch::apply for CFPatch::Call isn't implemented yet");
    }
    case CFPatch::Data: {
      assert(!"CFPatch::apply for CFPatch::Data isn't implemented yet");
    }
    }
  } else {
    switch (type) {
    case CFPatch::Jump:
      insnCodeGen::generateBranch(gen, gen.currAddr(), buf->predictedAddr(targetLabel));
      break;
    default:
      assert(0);
    }
  }
  return true;
}

bool CFPatch::isPLT(codeGen & /* gen */) {
  assert(!"Not implemented for AMDGPU");
  return false;
}

bool CFPatch::applyPLT(codeGen & /* gen */, CodeBuffer *) {
  assert(!"Not implemented for AMDGPU");
  return false;
}
