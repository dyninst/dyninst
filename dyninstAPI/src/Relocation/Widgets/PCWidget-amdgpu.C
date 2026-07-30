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

#include "../CFG/RelocBlock.h"
#include "../CodeBuffer.h"
#include "PCWidget.h"
#include "instructionAPI/h/Instruction.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/amdgpu-gfx908-details.h"

#include <stdint.h>

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

bool PCWidget::PCtoReturnAddr(const codeGen & /* templ */, const RelocBlock * /* t */,
                              CodeBuffer & /* buffer */) {
  // AMDGPU has no architectural return-address-on-stack idiom (the link register
  // pair s[30:31] is written by s_swappc/s_call, handled on the CFWidget path).
  assert(!"PCWidget::PCtoReturnAddr not expected on AMDGPU");
  return true;
}

// AMDGPU getpc-call / PC-relative relocation.
//
// The original s_getpc_b64 s[a:a+1] produces (its own address + 4). The following
// (compiler-emitted) s_add_u32/s_addc_u32 fold in a BAKED literal offset
//     D_old = target - (G_old + 4)
// so that s[a:a+1] ends up holding `target`, which a later s_swappc_b64 calls.
//
// When mid-block instrumentation RELOCATES this block, the getpc lands at a new
// address G'. A verbatim copy would then compute target + (G' - G_old) — wrong by the
// getpc's displacement. We fix it at the PRODUCER, not the consumer: reproduce the
// ORIGINAL produced PC by emitting getpc + a PC-relative correction
//     s_add_u32/s_addc_u32 s[a:a+1] += (G_old - G')
// and leave the original add/addc/swappc bytes untouched. After the correction
// s[a:a+1] == G_old + 4, so the original add/addc recompute the original `target`.
//
// This is PC-relative (getpc reads the runtime PC), so it is invariant under the
// loader's uniform library_adjust VMA shift — G_old, G' and target all shift
// together. Correct for a KEPT callee (the common device-lib / ockl case); a callee
// that is itself relocated is not yet handled (would need the callee's post-layout
// address instead of its original one). One getpc feeding multiple swappc (target
// reuse) and a re-loaded register pair are handled for free: we key on the getpc
// SITE, and the consumers are never touched.
//
// The correction constant depends on G' (= gen.currAddr()), known only after layout,
// so the real work happens in IPPatch::apply below.
bool PCWidget::PCtoReg(const codeGen & /* templ */, const RelocBlock *t,
                       CodeBuffer &buffer) {
  IPPatch *newPatch = new IPPatch(IPPatch::Reg, addr_, (Register)-1, thunkAddr_,
                                  insn_, t->block(), t->func());
  buffer.addPatch(newPatch, tracker(t));
  return true;
}

bool IPPatch::apply(codeGen &gen, CodeBuffer *) {
  // Destination SGPR pair base (sdst) from the original s_getpc_b64 encoding.
  // SOP1 word layout: [31:23] fixed, [22:16] sdst, [15:8] op, [7:0] ssrc0.
  const unsigned char *raw = reinterpret_cast<const unsigned char *>(insn.ptr());
  uint32_t word = (uint32_t)raw[0] | ((uint32_t)raw[1] << 8) |
                  ((uint32_t)raw[2] << 16) | ((uint32_t)raw[3] << 24);
  uint32_t sdst = (word >> 16) & 0x7Fu;

  Address Gnew = gen.currAddr();                  // where the getpc lands now (G')
  int64_t corr = (int64_t)addr - (int64_t)Gnew;   // G_old - G'  (small signed)
  uint32_t lo = (uint32_t)((uint64_t)corr & 0xffffffffull);
  uint32_t hi = (uint32_t)(((uint64_t)corr >> 32) & 0xffffffffull);

  relocation_cerr << "\t\t IPPatch::apply (AMDGPU getpc) sdst=s[" << sdst << ":" << (sdst + 1)
                  << "], G_old=" << std::hex << addr << ", G'=" << Gnew
                  << ", corr=" << corr << std::dec << endl;

  using namespace AmdgpuGfx908;
  // reg = G' + 4 (runtime: G' + library_adjust + 4)
  emitSop1Raw(S_GETPC_B64, sdst, /*ssrc0=*/0, gen);
  // reg += (G_old - G'), 64-bit via low/high literals -> reg = G_old + 4
  emitSop2RawWithLiteral(S_ADD_U32,  sdst,     sdst,     lo, gen);
  emitSop2RawWithLiteral(S_ADDC_U32, sdst + 1, sdst + 1, hi, gen);
  return true;
}
