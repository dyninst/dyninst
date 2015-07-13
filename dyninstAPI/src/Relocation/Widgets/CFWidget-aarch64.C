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

#include "../dyninstAPI/src/debug.h"

#include "../CodeTracker.h"
#include "../CodeBuffer.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/emit-aarch64.h"
#include "dyninstAPI/src/registerSpace.h"


using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

using namespace NS_aarch64;

bool CFWidget::generateIndirect(CodeBuffer &buffer,
                              Register,
                              const RelocBlock *trace,
                              Instruction::Ptr insn) {
	assert(0);
   return true;
}



bool CFWidget::generateIndirectCall(CodeBuffer &buffer,
                                  Register reg,
                                  Instruction::Ptr insn,
                                  const RelocBlock *trace,
				  Address origAddr)
{
	assert(0);
   return true;
}

bool CFPatch::apply(codeGen &gen, CodeBuffer *buf) {
	assert(0);

   return true;
}

bool CFPatch::isPLT(codeGen &gen) {
	assert(0);
      return false;
}

bool CFPatch::applyPLT(codeGen &gen, CodeBuffer *) {
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

bool CFWidget::generateAddressTranslator(CodeBuffer &buffer,
                                       const codeGen &templ,
                                       Register &reg,
                                       const RelocBlock *trace)
{
	assert(0);
#if !defined(cap_mem_emulation)
   return true;
#else
   assert(0);
   return false;
#endif

}

