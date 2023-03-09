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

#include "PCWidget.h"
#include "instructionAPI/h/Instruction.h"
#include "dyninstAPI/src/debug.h"
#include "../CFG/RelocBlock.h"
#include "../CodeBuffer.h"
#include "../CodeTracker.h"
#include "dyninstAPI/src/function.h"

#include "dyninstAPI/src/addressSpace.h" // For determining which type of getPC to emit
#include "dyninstAPI/src/RegisterConversion.h"
#include "dyninstAPI/src/registerSpace.h"

#include "dyninstAPI/src/emitter.h"

#include "unaligned_memory_access.h"
#include <cstdint>
#include <limits>

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

bool PCWidget::PCtoReturnAddr(const codeGen &templ, const RelocBlock *t, CodeBuffer &buffer) {
  if(templ.addrSpace()->proc()) {
    std::vector<unsigned char> newInsn;
    if (templ.getArch() == Arch_x86_64) {
        codeGen gen(16);
        Address RIP = addr_ + insn_.size();
        insnCodeGen::generatePush64(gen, RIP);
        buffer.addPIC(gen, tracker(t));
    } else if (templ.getArch() == Arch_x86) {
        newInsn.push_back(0x68); // push
        Address EIP = addr_ + insn_.size();
        unsigned char *tmp = (unsigned char *) &EIP;
        newInsn.insert(newInsn.end(), tmp, tmp+sizeof(unsigned int));
        buffer.addPIC(newInsn, tracker(t));
    }  else {
        assert(!"Unimplemented architecture!");
    }
  }
  else {
    IPPatch *newPatch = new IPPatch(IPPatch::Push, addr_, insn_, t->block(), t->func());
    buffer.addPatch(newPatch, tracker(t));
  }	
   
  return true;
}

bool PCWidget::PCtoReg(const codeGen &templ, const RelocBlock *t, CodeBuffer &buffer) {
  bool ignored;
  Register reg = convertRegID(a_.reg(), ignored);

  if(templ.addrSpace()->proc()) {
    std::vector<unsigned char> newInsn;
    newInsn.push_back(static_cast<unsigned char>(0xb8 + reg));
    // MOV family, destination of the register encoded by
    // 'reg', source is an Iv immediate
     
    Address EIP = addr_ + insn_.size();
    unsigned char *tmp = (unsigned char *) &EIP;
    newInsn.insert(newInsn.end(), tmp, tmp + sizeof(unsigned int));
    buffer.addPIC(newInsn, tracker(t));          
  }
  else {
    IPPatch *newPatch = new IPPatch(IPPatch::Reg, addr_, reg, thunkAddr_, insn_, t->block(), t->func());
    buffer.addPatch(newPatch, tracker(t));
  }
  return true;
}

#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/inst-x86.h"

bool IPPatch::apply(codeGen &gen, CodeBuffer *) {
  relocation_cerr << "\t\t IPPatch::apply" << endl;
  relocation_cerr << "\t\t\t Generating IPPatch for target address " << std::hex << addr << ", CodeGen current address " << std::hex << gen.currAddr() << " and register number " << reg << endl;

  // We want to generate addr (as modified) into the appropriate location.
  // TODO get rid of the #ifdef here...


  // Emit a call to the next instruction to get current PC
  // This is necessary for PIC code
  GET_PTR(newInsn, gen); 
  append_memory_as_byte(newInsn, 0xE8);
  append_memory_as(newInsn, uint32_t{0});
  SET_PTR(newInsn, gen);
  // Compensating PC on stack to the original location
  int64_t offset = addr - gen.currAddr() + insn.size();
  REGET_PTR(newInsn, gen);
  append_memory_as_byte(newInsn, 0x81);
  append_memory_as_byte(newInsn, 0x04);
  append_memory_as_byte(newInsn, 0x24);
  // offset is 64-bits, assert if the value does not fit in 32-bits
  assert(numeric_limits<int32_t>::lowest() <= offset && offset <= numeric_limits<int32_t>::max() && "offset more than 32 bits");
  append_memory_as(newInsn, static_cast<int32_t>(offset));

  if (type == Reg) {
    assert(reg != (Register) -1);
    // pop...
    append_memory_as_byte(newInsn, 0x58 + reg); // POP family
  }
  SET_PTR(newInsn, gen);
  return true;
}

