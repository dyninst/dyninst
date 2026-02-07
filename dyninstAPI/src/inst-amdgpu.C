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

#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "dyninstAPI/src/dynProcess.h"
#include "dyninstAPI/src/registerSpace.h"
#include "arch-amdgpu.h"
#include "emit-amdgpu.h"
#include "dyn_register.h"

/************************************* Register Space **************************************/

void registerSpace::initialize32() {
  static bool done = false;
  if (done)
    return;

  std::vector<registerSlot *> registers;

  // TODO: This initialization doesn't consider all registers. Right now only the registers we typically
  // use for instrumentation are considered. This might need work later on.

  // SGPRs
  for (unsigned id = NS_amdgpu::MIN_SGPR_ID; id <= NS_amdgpu::MAX_SGPR_ID; id++) {
    char name[32];
    sprintf(name, "s%u", id);
    Register sgpr = Register::makeScalarRegister(OperandRegId(id), BlockSize(1));
    registers.push_back(new registerSlot(sgpr, name,
                                         /*offLimits =*/false, registerSlot::liveAlways,
                                         registerSlot::SGPR));
  }

  // VGPRs
  for (unsigned id = 0; id <= 255; id++) {
    char name[32];
    sprintf(name, "v%u", id);
    Register vgpr = Register::makeVectorRegister(OperandRegId(id), BlockSize(1));
    registers.push_back(new registerSlot(vgpr, name,
                                         /*offLimits =*/false, registerSlot::liveAlways,
                                         registerSlot::VGPR));
  }

  // clang-format off
    // SPRs
    registers.push_back(new registerSlot(NS_amdgpu::RegisterConstants::flat_scratch_lo, "flat_scratch_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(NS_amdgpu::RegisterConstants::flat_scratch_hi, "flat_scratch_hi", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(NS_amdgpu::RegisterConstants::xnack_mask_lo, "xnack_mask_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(NS_amdgpu::RegisterConstants::xnack_mask_hi, "xnack_mask_hi", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(NS_amdgpu::RegisterConstants::vcc_lo, "vcc_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(NS_amdgpu::RegisterConstants::vcc_hi, "vcc_hi", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(NS_amdgpu::RegisterConstants::exec_lo, "exec_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(NS_amdgpu::RegisterConstants::exec_hi, "exec_hi", true, registerSlot::liveAlways, registerSlot::SPR));
  // clang-format on

  registerSpace::createRegisterSpace64(registers);
  done = true;
}

void registerSpace::initialize64() { assert(!"No 64-bit registers for AMDGPU"); }

void registerSpace::initialize() { initialize32(); }

/***********************************************************************************************/
/***********************************************************************************************/

// TODO: ALL THESE MUST GO AWAY ENTIRELY AS CODEGEN MATURES
void emitImm(opCode /* op */, Register /* src1 */, RegValue /* src2imm */, Register /* dest */,
             codeGen & /* gen */, bool /*noCost*/, registerSpace * /* rs */, bool /* s */) {
  assert(!"Not implemented for AMDGPU");
}

Register emitFuncCall(opCode, codeGen &, std::vector<AstNodePtr> &, bool, Address) {
  assert(!"Not implemented for AMDGPU");
  return 0;
}

Register emitFuncCall(opCode /* op */, codeGen & /* gen */,
                      std::vector<AstNodePtr> & /* operands */, bool /* noCost */,
                      func_instance * /* callee */) {
  assert(!"Not implemented for AMDGPU");
  return 0;
}

codeBufIndex_t emitA(opCode /* op */, Register /* src1 */, Register, long /* dest */,
                     codeGen & /* gen */, RegControl /* rc */, bool) {
  assert(!"Not implemented for AMDGPU");
  return 0;
}

Register emitR(opCode /* op */, Register /* src1 */, Register /* src2 */, Register /* dest */,
               codeGen & /* gen */, bool /*noCost*/, const instPoint *, bool /*for_MT*/) {
  assert(!"Not implemented for AMDGPU");
  return 0;
}

void emitJmpMC(int /*condition*/, int /*offset*/, codeGen &) {
  assert(!"Not implemented for AMDGPU");
}

void emitASload(const BPatch_addrSpec_NP * /* as */, Register /* dest */, int /* stackShift */,
                codeGen & /* gen */, bool) {
  assert(!"Not imeplemented for AMDGPU");
}

void emitCSload(const BPatch_addrSpec_NP *, Register, codeGen &, bool) {
  assert(!"Not imeplemented for AMDGPU");
}

void emitVload(opCode /* op */, Address /* src1 */, Register /* src2 */, Register /* dest */,
               codeGen & /* gen */, bool /*noCost*/, registerSpace * /*rs*/, int /* size */,
               const instPoint * /* location */, AddressSpace *) {
  assert(!"Not imeplemented for AMDGPU");
}

void emitVstore(opCode /* op */, Register /* src1 */, Register /*src2*/, Address /* dest */,
                codeGen & /* gen */, bool, registerSpace * /* rs */, int /* size */,
                const instPoint * /* location */, AddressSpace *) {
  assert(!"Not imeplemented for AMDGPU");
}

void emitV(opCode /* op */, Register /* src1 */, Register /* src2 */, Register /* dest */,
           codeGen & /* gen */, bool /*noCost*/, registerSpace * /*rs*/, int /* size */,
           const instPoint * /* location */, AddressSpace * /* proc */, bool /* s */) {
  assert(!"Not imeplemented for AMDGPU");
}

bool doNotOverflow(int64_t /* value */) {
  assert(!"Not implemented for AMDGPU");
  return false;
}

void emitLoadPreviousStackFrameRegister(Address /* register_num */, Register /* dest */,
                                        codeGen & /* gen */, int /* size */, bool) {}

void emitStorePreviousStackFrameRegister(Address, Register, codeGen &, int, bool) {}

bool AddressSpace::getDynamicCallSiteArgs(InstructionAPI::Instruction /* i */, Address /* addr */,
                                          std::vector<AstNodePtr> & /* args */) {
  assert(!"Not implemented for AMDGPU");
  return false;
}

bool writeFunctionPtr(AddressSpace * /* p */, Address /* addr */, func_instance * /* f */) {
  assert(!"Not implemented for AMDGPU");
  return false;
}

Emitter *AddressSpace::getEmitter() {
  static EmitterAmdgpuGfx908 gfx908Emitter;
  return &gfx908Emitter;
}

Address Emitter::getInterModuleVarAddr(const image_variable * /* var */, codeGen & /* gen */) {
  assert(!"Not implemented for AMDGPU");
  return 0;
}

Address Emitter::getInterModuleFuncAddr(func_instance * /* func */, codeGen & /* gen */) {
  assert(!"Not implemented for AMDGPU");
  return 0;
}
