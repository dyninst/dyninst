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

#include "AmdgpuTraceData.h"
#include "ABI.h"
#include "amdgpu-gfx908-details.h"
#include "arch-amdgpu.h"
#include "codegen/codegen.h"
#include "patching/instPoint.h"
#include "registerSpace/registerSpace.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"

#include <iomanip>
#include <sstream>

namespace Dyninst { namespace DyninstAPI {

namespace {
  // SOP1 operand encodings of M0 and of "a 32-bit literal follows".
  constexpr uint32_t m0Operand = 124;
  constexpr uint32_t literalOperand = 255;

  bool isM0Live(codeGen &gen) {
    instPoint *point = gen.point();
    if (!point) {
      return true;
    }
    // instPoint::liveRegisters() is indexed by the 64-bit ABI's register map.
    int const m0Index = ABI::getABI(8)->getIndex(Dyninst::amdgpu_gfx908::m0);
    assert(m0Index >= 0 && "m0 must be in the AMDGPU liveness register map");
    return point->liveRegisters()[m0Index];
  }
}

std::string AmdgpuTraceDataAST::format(std::string indent) {
  std::stringstream ret;
  ret << indent << "AmdgpuTraceData/" << std::hex << this << "(";
  if (source_ == Dyninst::Null_Register) {
    ret << "0x" << value_;
  } else {
    ret << "operand " << std::dec << source_.getId();
  }
  ret << ")\n";
  return ret.str();
}

// s_ttracedata takes its value from M0, so M0 is overwritten. Where the program's M0 is
// live, it is kept in a free SGPR and restored afterwards:
//
//   s_mov_b32 sN, m0            ; only if M0 is live
//   s_mov_b32 m0, <value>
//   s_ttracedata
//   s_mov_b32 m0, sN            ; only if M0 is live
bool AmdgpuTraceDataAST::generateCode_phase2(codeGen &gen, Dyninst::Address &retAddr,
                                             Dyninst::Register &retReg) {
  retAddr = Dyninst::ADDR_NULL;
  retReg = Dyninst::Null_Register;

  Dyninst::Register saved = Dyninst::Null_Register;
  if (isM0Live(gen)) {
    saved = gen.rs()->allocateGprBlock(Dyninst::RegKind::SCALAR, 1, NS_amdgpu::SINGLE_ALIGNMENT);
    assert(saved != Dyninst::Null_Register && "no free SGPR to preserve the live M0");
    AmdgpuGfx908::emitSop1(AmdgpuGfx908::S_MOV_B32, saved.getId(), m0Operand, false, 0, gen);
  }

  if (source_ == Dyninst::Null_Register) {
    AmdgpuGfx908::emitSop1(AmdgpuGfx908::S_MOV_B32, m0Operand, literalOperand, true, value_, gen);
  } else {
    AmdgpuGfx908::emitSop1(AmdgpuGfx908::S_MOV_B32, m0Operand, source_.getId(), false, 0, gen);
  }
  AmdgpuGfx908::emitTTraceData(gen);

  if (saved != Dyninst::Null_Register) {
    AmdgpuGfx908::emitSop1(AmdgpuGfx908::S_MOV_B32, m0Operand, saved.getId(), false, 0, gen);
    gen.rs()->freeRegister(saved);
  }

  decUseCount(gen);
  return true;
}

}}
