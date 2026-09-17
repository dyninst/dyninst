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

#ifndef INST_LOONGARCH64_H
#define INST_LOONGARCH64_H

#define DEAD_REG              0
#define LIVE_REG              1
#define LIVE_UNCLOBBERED_REG  2
#define LIVE_CLOBBERED_REG    3

#define GPRSIZE_32            4
#define GPRSIZE_64            8
#define FPRSIZE_64           16

// LoongArch64 register conventions:
// $r0  = $zero (always 0)
// $r1  = $ra   (return address)
// $r2  = $tp   (thread pointer)
// $r3  = $sp   (stack pointer)
// $r4-$r11 = $a0-$a7 (argument registers)
// $r12-$r20 = $t0-$t8 (temporaries)
// $r21 = reserved
// $r22 = $fp/$s0 (frame pointer)
// $r23-$r31 = $s1-$s9 (saved registers)

#define REG_FP               22   // $s0/$fp
#define REG_LR               1    // $ra
#define REG_SP               3    // $sp
#define REG_TOC               2   // $tp (thread pointer)

#define REG_GUARD_ADDR        12  // $t0
#define REG_GUARD_VALUE       13  // $t1
#define REG_GUARD_OFFSET      13  // $t1

#define REG_COST_ADDR         12  // $t0
#define REG_COST_VALUE        13  // $t1

#define REG_SCRATCH           14  // $t2

#define STACKSKIP          288

#define ALIGN_QUADWORD(x)  ( ((x) + 0xf) & ~0xf )

#define GPRSAVE_64  (32*GPRSIZE_64)
#define FPRSAVE_64  (32*FPRSIZE_64)
#define SPRSAVE_64  (1*8+3*4)
#define FUNCSAVE_64 (32*8)
#define FUNCARGS_64 (16*8)
#define LINKAREA_64 (6*8)

#if defined(os_linux)
#define PARAM_OFFSET(mutatee_address_width)                         \
        (                                                           \
            ((mutatee_address_width) == sizeof(uint64_t))           \
            ? (   /* 64-bit ELF LoongArch64 Linux                */ \
                  sizeof(uint64_t) +  /* return address save     */ \
                  sizeof(uint64_t) +  /* frame pointer save      */ \
                  sizeof(uint64_t)    /* Stack frame back chain  */ \
              )                                                     \
            : (   /* 32-bit ELF LoongArch64 Linux                */ \
                  sizeof(uint32_t) +  /* return address save     */ \
                  sizeof(uint32_t)    /* Stack frame back chain  */ \
              )                                                     \
        )
#else
#error "Unknown operating system in inst-loongarch64.h"
#endif

#define TRAMP_FRAME_SIZE_64 ALIGN_QUADWORD(GPRSAVE_64 + FPRSAVE_64 + SPRSAVE_64)

#define TRAMP_SPR_OFFSET_64 (0)
#define STK_LR       (              0)
#define STK_FPCR     (STK_LR      + 8)
#define STK_FPSR     (STK_FPCR    + 4)

#define TRAMP_FPR_OFFSET_64 (TRAMP_SPR_OFFSET_64 + SPRSAVE_64)
#define TRAMP_GPR_OFFSET_64 (TRAMP_FPR_OFFSET_64 + FPRSAVE_64)
#define FUNC_CALL_SAVE_64   (LINKAREA_64 + FUNCARGS_64)

inline int TRAMP_GPR_OFFSET(int x) {
  if(x == 8) {
    return TRAMP_GPR_OFFSET_64;
  }
  assert(!"32-bit offsets not support on loongarch64");
  return 0;
}

inline int TRAMP_FPR_OFFSET(int x) {
  if(x == 8) {
    return TRAMP_FPR_OFFSET_64;
  }
  assert(!"32-bit offsets not support on loongarch64");
  return 0;
}

inline int TRAMP_SPR_OFFSET(int x) {
  if(x == 8) {
    return TRAMP_SPR_OFFSET_64;
  }
  assert(!"32-bit offsets not support on loongarch64");
  return 0;
}

class codeGen;

void pushStack(codeGen &gen);
void popStack(codeGen &gen);

#endif