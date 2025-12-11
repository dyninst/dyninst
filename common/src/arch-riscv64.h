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

#ifndef _ARCH_RISCV64_H
#define _ARCH_RISCV64_H

#include <stdint.h>

// In RISC-V, instruction length can be a multiple of 2 bytes

// Standard RISC-V instructions (RVI, RVA, RVM, RVF, RVD, ...) are 4 bytes
using rvInsn_t = uint32_t;
constexpr int RISCV_INSN_SIZE = 4;
static_assert(sizeof(rvInsn_t) == RISCV_INSN_SIZE, "rvInst_t size mismatch");

// Compressed instructions (RVC) are 2 bytes
using rvcInsn_t = uint16_t;
constexpr int RISCVC_INSN_SIZE = 2;
static_assert(sizeof(rvcInsn_t) == RISCVC_INSN_SIZE, "rvcInsn_t size mismatch");

// The minimum instruction size is 2 bytes
using rvMinInsn_t = rvcInsn_t;
constexpr int RISCV_MIN_INSN_SIZE = RISCVC_INSN_SIZE;
static_assert(sizeof(rvMinInsn_t) == RISCV_MIN_INSN_SIZE, "rvMinInst_t size mismatch");

// The maximum instruction size is 4 bytes
using rvMaxInsn_t = rvInsn_t;
constexpr int RISCV_MAX_INSN_SIZE = RISCV_INSN_SIZE;
static_assert(sizeof(rvMaxInsn_t) == RISCV_MAX_INSN_SIZE, "rvMaxInsn_t size mismatch");

// The bits in the register is 64 bits
constexpr int RISCV_REG_SIZE = 8;
// The immediates in instructions varies.
// The maximum is 21 bits, which can be stored in a 32 bit integer
constexpr int RISCV_IMM_SIZE = 4;

// Raw register encoding used in RISC-V instruction encoding
// These values identify registers when decoding instructions
// They are also used to emit register encoding during codegen
constexpr int32_t GPR_X0     = 0;
constexpr int32_t GPR_X1     = 1;
constexpr int32_t GPR_X2     = 2;
constexpr int32_t GPR_X3     = 3;
constexpr int32_t GPR_X4     = 4;
constexpr int32_t GPR_X5     = 5;
constexpr int32_t GPR_X6     = 6;
constexpr int32_t GPR_X7     = 7;
constexpr int32_t GPR_X8     = 8;
constexpr int32_t GPR_X9     = 9;
constexpr int32_t GPR_X10    = 10;
constexpr int32_t GPR_X11    = 11;
constexpr int32_t GPR_X12    = 12;
constexpr int32_t GPR_X13    = 13;
constexpr int32_t GPR_X14    = 14;
constexpr int32_t GPR_X15    = 15;
constexpr int32_t GPR_X16    = 16;
constexpr int32_t GPR_X17    = 17;
constexpr int32_t GPR_X18    = 18;
constexpr int32_t GPR_X19    = 19;
constexpr int32_t GPR_X20    = 20;
constexpr int32_t GPR_X21    = 21;
constexpr int32_t GPR_X22    = 22;
constexpr int32_t GPR_X23    = 23;
constexpr int32_t GPR_X24    = 24;
constexpr int32_t GPR_X25    = 25;
constexpr int32_t GPR_X26    = 26;
constexpr int32_t GPR_X27    = 27;
constexpr int32_t GPR_X28    = 28;
constexpr int32_t GPR_X29    = 29;
constexpr int32_t GPR_X30    = 30;
constexpr int32_t GPR_X31    = 31;

constexpr int32_t GPR_ZERO   = 0;
constexpr int32_t GPR_RA     = 1;
constexpr int32_t GPR_SP     = 2;
constexpr int32_t GPR_GP     = 3;
constexpr int32_t GPR_TP     = 4;
constexpr int32_t GPR_T0     = 5;
constexpr int32_t GPR_T1     = 6;
constexpr int32_t GPR_T2     = 7;
constexpr int32_t GPR_S0     = 8;
constexpr int32_t GPR_FP     = 8;
constexpr int32_t GPR_S1     = 9;
constexpr int32_t GPR_A0     = 10;
constexpr int32_t GPR_A1     = 11;
constexpr int32_t GPR_A2     = 12;
constexpr int32_t GPR_A3     = 13;
constexpr int32_t GPR_A4     = 14;
constexpr int32_t GPR_A5     = 15;
constexpr int32_t GPR_A6     = 16;
constexpr int32_t GPR_A7     = 17;
constexpr int32_t GPR_S2     = 18;
constexpr int32_t GPR_S3     = 19;
constexpr int32_t GPR_S4     = 20;
constexpr int32_t GPR_S5     = 21;
constexpr int32_t GPR_S6     = 22;
constexpr int32_t GPR_S7     = 23;
constexpr int32_t GPR_S8     = 24;
constexpr int32_t GPR_S9     = 25;
constexpr int32_t GPR_S10    = 26;
constexpr int32_t GPR_S11    = 27;
constexpr int32_t GPR_T3     = 28;
constexpr int32_t GPR_T4     = 29;
constexpr int32_t GPR_T5     = 30;
constexpr int32_t GPR_T6     = 31;

// Register encoding
constexpr int32_t REG_RD_ENC_OFFSET = 7;
constexpr int32_t REG_RS1_ENC_OFFSET = 15;
constexpr int32_t REG_RS2_ENC_OFFSET = 20;
constexpr int32_t REG_ENC_MASK = 0x1f;

#endif
