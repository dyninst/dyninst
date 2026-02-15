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

#include <cassert>
#include <codegen.h>
#include <stdint.h>

#ifndef INST_RISCV64_H
#define INST_RISCV64_H

#define DEAD_REG 0
#define LIVE_REG 1
#define LIVE_UNCLOBBERED_REG 2
#define LIVE_CLOBBERED_REG 3

#define GPRSIZE_32 4
#define GPRSIZE_64 8
#define FPRSIZE_32 8
#define FPRSIZE_64 16

#define REG_GUARD_ADDR 5
#define REG_GUARD_VALUE 6
#define REG_GUARD_OFFSET 6

#define REG_COST_ADDR 5
#define REG_COST_VALUE 6

#define REG_FP 8
#define REG_SP 2

#define REG_SCRATCH 10

#define STACKSKIP 288

#define ALIGN_QUADWORD(x) (((x) + 0xf) & ~0xf) // x is positive or unsigned

#define GPRSAVE_64 (32 * GPRSIZE_64)
#define FPRSAVE_64 (32 * FPRSIZE_64)
#define SPRSAVE_64 (0)

#define TRAMP_FRAME_SIZE_64 ALIGN_QUADWORD(GPRSAVE_64 + FPRSAVE_64 + SPRSAVE_64)

#define TRAMP_GPR_OFFSET_64 (0)
#define TRAMP_FPR_OFFSET_64 (TRAMP_GPR_OFFSET_64 + GPRSAVE_64)
#define TRAMP_SPR_OFFSET_64 (TRAMP_FPR_OFFSET_64 + FPRSAVE_64)

inline int tramp_offset(int xlen_bytes, int offset64) {
  assert(xlen_bytes == 8 && "Only RV64 is supported");
  return offset64;
}

inline int TRAMP_GPR_OFFSET(int xlen_bytes) {
  return tramp_offset(xlen_bytes, TRAMP_GPR_OFFSET_64);
}

inline int TRAMP_FPR_OFFSET(int xlen_bytes) {
  return tramp_offset(xlen_bytes, TRAMP_FPR_OFFSET_64);
}

inline int TRAMP_SPR_OFFSET(int xlen_bytes) {
  return tramp_offset(xlen_bytes, TRAMP_SPR_OFFSET_64);
}

void pushStack(codeGen &gen, int size);
void popStack(codeGen &gen, int size);

#endif
