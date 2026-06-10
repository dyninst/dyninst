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

#include "codegen/RegControl.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "dynproc/dynProcess.h"
#include "registerSpace/registerSpace.h"
#include "arch-amdgpu.h"
#include "emit-amdgpu.h"
#include "dyn_register.h"
#include "Architecture.h"
#include "addressSpace.h"
#include "binaryEdit.h"
#include "function.h"
#include "Symbol.h"

#include <cstdlib>
#include <cstring>

using codeGenASTPtr = Dyninst::DyninstAPI::codeGenASTPtr;

/***********************************************************************************************/
/***********************************************************************************************/

// TODO: ALL THESE MUST GO AWAY ENTIRELY AS CODEGEN MATURES

void emitASload(const BPatch_addrSpec_NP * /* as */, Register /* dest */, int /* stackShift */,
                codeGen & /* gen */) {
  assert(!"Not imeplemented for AMDGPU");
}

void emitStorePreviousStackFrameRegister(Address, Register, codeGen &, int, bool) {}

Address EmitterAmdgpuGfx908::getInterModuleVarAddr(const image_variable * /* var */, codeGen & /* gen */) {
  assert(!"Not implemented for AMDGPU");
  return 0;
}

Address EmitterAmdgpuGfx908::getInterModuleFuncAddr(func_instance *func, codeGen &gen) {
  // Reserve a zero-initialized slot in the rewritten binary and register a
  // dependent relocation against the callee's symbol. The loader will fill
  // the slot with the resolved address at load time; emitCall reads it
  // back via SMEM and jumps through it with S_SWAPPC_B64.
  //
  // Mirrors Emitterx86::getInterModuleFuncAddr (codegen/emitters/x86/Emitterx86.C).

  AddressSpace *addrSpace = gen.addrSpace();
  BinaryEdit *binEdit = addrSpace ? addrSpace->edit() : nullptr;

  // Live-process indirect call is not supported on AMDGPU yet; only the
  // static-rewriter path goes through here.
  assert(binEdit && "AMDGPU getInterModuleFuncAddr: only supported under static rewriting");
  assert(func && "AMDGPU getInterModuleFuncAddr: callee is null");

  SymtabAPI::Symbol *referring = func->getRelocSymbol();

  // If we've already minted a slot for this symbol, reuse it.
  Address relocation_address = binEdit->getDependentRelocationAddr(referring);
  if(relocation_address) {
    return relocation_address;
  }

  const unsigned int jump_slot_size = getArchAddressWidth(gen.getArch());
  relocation_address = binEdit->inferiorMalloc(jump_slot_size);

  std::vector<unsigned char> zero(jump_slot_size, 0);
  binEdit->writeDataSpace(reinterpret_cast<void*>(relocation_address),
                          jump_slot_size, zero.data());

  binEdit->addDependentRelocation(relocation_address, referring);

  return relocation_address;
}

void EmitterAmdgpuGfx908::emitImm(opCode /* op */, Register /* src1 */, RegValue /* src2imm */, Register /* dest */,
             codeGen & /* gen */, bool /* s */) {
  assert(!"Not implemented for AMDGPU");
}

codeBufIndex_t EmitterAmdgpuGfx908::emitA(opCode /* op */, Register /* src1 */, long /* dest */,
                     codeGen & /* gen */, Dyninst::DyninstAPI::RegControl /* rc */) {
  assert(!"Not implemented for AMDGPU");
  return 0;
}

Register EmitterAmdgpuGfx908::emitR(opCode /* op */, Register /* src1 */, Register /* src2 */, Register /* dest */,
               codeGen & /* gen */, const instPoint *) {
  assert(!"Not implemented for AMDGPU");
  return 0;
}

void EmitterAmdgpuGfx908::emitV(opCode /* op */, Register /* src1 */, Register /* src2 */, Register /* dest */,
           codeGen & /* gen */, int /* size */,
           AddressSpace * /* proc */, bool /* s */) {
  assert(!"Not imeplemented for AMDGPU");
}

void EmitterAmdgpuGfx908::emitVload(opCode /* op */, Address /* src1 */, Register /* src2 */, Register /* dest */,
               codeGen & /* gen */, int /* size */,
               AddressSpace *) {
  assert(!"Not imeplemented for AMDGPU");
}

void EmitterAmdgpuGfx908::emitVstore(opCode /* op */, Register /* src1 */, Register /*src2*/, Address /* dest */,
                codeGen & /* gen */, int /* size */,
                AddressSpace *) {
  assert(!"Not imeplemented for AMDGPU");
}
