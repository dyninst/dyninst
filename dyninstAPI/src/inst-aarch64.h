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

#include <stdint.h>
#include <cassert>

#ifndef INST_AARCH64_H
#define INST_AARCH64_H

#define DEAD_REG              0
#define LIVE_REG              1
#define LIVE_UNCLOBBERED_REG  2
#define LIVE_CLOBBERED_REG    3

#define GPRSIZE_32            4
#define GPRSIZE_64            8
#define FPRSIZE_64           16

#define REG_FP               29
#define REG_LR               30
#define REG_SP               31
#define REG_TOC               2   /* TOC anchor                            */
// REG_GUARD_OFFSET and REG_GUARD_VALUE could overlap.
#define REG_GUARD_ADDR        5   /* Arbitrary                             */
#define REG_GUARD_VALUE       6
#define REG_GUARD_OFFSET      6

#define REG_COST_ADDR         5
#define REG_COST_VALUE        6

#define REG_SCRATCH          10

// #sasha This seemed to be copy and paste. Not sure if it all stands
// for ARM.
//
// The stack grows down from high addresses toward low addresses.
// There is a maximum number of bytes on the stack below the current
// value of the stack frame pointer that a function can use without
// first establishing a new stack frame.  When our instrumentation
// needs to use the stack, we make sure not to write into this
// potentially used area.  64-bit PowerPC ELF ABI Supplement,
// Version 1.9, 2004-10-23, used by Linux, stated 288 bytes for this
// area.
#define STACKSKIP          288

#define ALIGN_QUADWORD(x)  ( ((x) + 0xf) & ~0xf )  //x is positive or unsigned

//TODO Fix for ARM
#define GPRSAVE_64  (31*GPRSIZE_64)
#define FPRSAVE_64  (32*FPRSIZE_64)
#define SPRSAVE_64  (1*8+3*4)
// #sasha Are these necessary?
#define FUNCSAVE_64 (32*8)
#define FUNCARGS_64 (16*8)
#define LINKAREA_64 (6*8)

// #sasha Why is PowerPC stuff here?
#if defined(os_linux)
#define PARAM_OFFSET(mutatee_address_width)                         \
        (                                                           \
            ((mutatee_address_width) == sizeof(uint64_t))           \
            ? (   /* 64-bit ELF PowerPC Linux                   */  \
                  sizeof(uint64_t) +  /* TOC save               */  \
                  sizeof(uint64_t) +  /* link editor doubleword */  \
                  sizeof(uint64_t) +  /* compiler doubleword    */  \
                  sizeof(uint64_t) +  /* LR save                */  \
                  sizeof(uint64_t) +  /* CR save                */  \
                  sizeof(uint64_t)    /* Stack frame back chain */  \
              )                                                     \
            : (   /* 32-bit ELF PowerPC Linux                   */  \
                  sizeof(uint32_t) +  /* LR save                */  \
                  sizeof(uint32_t)    /* Stack frame back chain */  \
              )                                                     \
        )
#else
#error "Unknown operating system in inst-aarch64.h"
#endif


/*
#define TRAMP_FRAME_SIZE_64 ALIGN_QUADWORD(STACKSKIP + GPRSAVE_64 + FPRSAVE_64 \
                                           + SPRSAVE_64 \
                                           + FUNCSAVE_64 + FUNCARGS_64 + LINKAREA_64)
                                           */
#define TRAMP_FRAME_SIZE_64 ALIGN_QUADWORD(GPRSAVE_64 + FPRSAVE_64 + SPRSAVE_64)

//#define PDYN_RESERVED_64 (LINKAREA_64 + FUNCARGS_64 + FUNCSAVE_64)

//#define TRAMP_SPR_OFFSET_64 (PDYN_RESERVED_64)
#define TRAMP_SPR_OFFSET_64 (0)
#define STK_LR       (              0)
#define STK_NZCV     (STK_SP_EL0  + 8)
#define STK_FPCR     (STK_NZCV    + 4)
#define STK_FPSR     (STK_FPCR    + 4)

#define TRAMP_FPR_OFFSET_64 (TRAMP_SPR_OFFSET_64 + SPRSAVE_64)
#define TRAMP_GPR_OFFSET_64 (TRAMP_FPR_OFFSET_64 + FPRSAVE_64)
#define FUNC_CALL_SAVE_64   (LINKAREA_64 + FUNCARGS_64)

inline int TRAMP_GPR_OFFSET(int x) {
  if(x == 8) {
    return TRAMP_GPR_OFFSET_64;
  }
  assert(!"32-bit offsets not support on aarch64");
  return 0;
}

inline int TRAMP_FPR_OFFSET(int x) {
  if(x == 8) {
    return TRAMP_FPR_OFFSET_64;
  }
  assert(!"32-bit offsets not support on aarch64");
  return 0;
}

inline int TRAMP_SPR_OFFSET(int x) {
  if(x == 8) {
    return TRAMP_SPR_OFFSET_64;
  }
  assert(!"32-bit offsets not support on aarch64");
  return 0;
}

class codeGen;

void pushStack(codeGen &gen);
void popStack(codeGen &gen);

#endif
