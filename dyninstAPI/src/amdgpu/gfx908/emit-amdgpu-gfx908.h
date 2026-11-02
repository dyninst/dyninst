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

#ifndef _EMIT_AMDGPU_GFX908_H
#define _EMIT_AMDGPU_GFX908_H

#include "emit-amdgpu.h"

class codeGen;

// Thin per-gen concrete emitter: supplies only this gen's arch + its ENC-typed packers. Everything else
// is inherited from EmitterAmdgpu. A sibling gen (e.g. EmitterAmdgpuGfx942) drops in the same way.
class EmitterAmdgpuGfx908 : public EmitterAmdgpu {
protected:
  Dyninst::Architecture gpuArch() const override { return Dyninst::Arch_amdgpu_gfx908; }
  void rawSop1(unsigned opcode, Dyninst::Register dest, Dyninst::Register src0, bool hasLiteral,
               uint32_t literal, codeGen &gen) override;
  void rawSop2(unsigned opcode, Dyninst::Register dest, Dyninst::Register src1, Dyninst::Register src0,
               codeGen &gen) override;
  void rawSop2WithSrc1Literal(unsigned opcode, Dyninst::Register dest, Dyninst::Register src0,
                              uint32_t literal, codeGen &gen) override;
  void rawSopK(unsigned opcode, Dyninst::Register dest, int16_t simm16, codeGen &gen) override;
  void rawSopC(unsigned opcode, Dyninst::Register src0, Dyninst::Register src1, codeGen &gen) override;
  void rawSmem(unsigned opcode, uint64_t sdata, uint64_t sbase, uint64_t offset, codeGen &gen) override;
};

#endif // _EMIT_AMDGPU_GFX908_H
