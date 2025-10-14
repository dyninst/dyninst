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

// $Id: arch-power.h,v 1.45 2008/03/25 19:24:23 bernat Exp $

#ifndef _ARCH_RISCV64_H
#define _ARCH_RISCV64_H

#include <elf.h>

#if !defined(R_RISCV_NONE)
#define R_RISCV_NONE 0
#endif
#if !defined(R_RISCV_32)
#define R_RISCV_32 1
#endif
#if !defined(R_RISCV_64)
#define R_RISCV_64 2
#endif
#if !defined(R_RISCV_RELATIVE)
#define R_RISCV_RELATIVE 3
#endif
#if !defined(R_RISCV_COPY)
#define R_RISCV_COPY 4
#endif
#if !defined(R_RISCV_JUMP_SLOT)
#define R_RISCV_JUMP_SLOT 5
#endif
#if !defined(R_RISCV_TLS_DTPMOD32)
#define R_RISCV_TLS_DTPMOD32 6
#endif
#if !defined(R_RISCV_TLS_DTPMOD64)
#define R_RISCV_TLS_DTPMOD64 7
#endif
#if !defined(R_RISCV_TLS_DTPREL32)
#define R_RISCV_TLS_DTPREL32 8
#endif
#if !defined(R_RISCV_TLS_DTPREL64)
#define R_RISCV_TLS_DTPREL64 9
#endif
#if !defined(R_RISCV_TLS_TPREL32)
#define R_RISCV_TLS_TPREL32 10
#endif
#if !defined(R_RISCV_TLS_TPREL64)
#define R_RISCV_TLS_TPREL64 11
#endif
#if !defined(R_RISCV_BRANCH)
#define R_RISCV_BRANCH 16
#endif
#if !defined(R_RISCV_JAL)
#define R_RISCV_JAL 17
#endif
#if !defined(R_RISCV_CALL)
#define R_RISCV_CALL 18
#endif
#if !defined(R_RISCV_CALL_PLT)
#define R_RISCV_CALL_PLT 19
#endif
#if !defined(R_RISCV_GOT_HI20)
#define R_RISCV_GOT_HI20 20
#endif
#if !defined(R_RISCV_TLS_GOT_HI20)
#define R_RISCV_TLS_GOT_HI20 21
#endif
#if !defined(R_RISCV_TLS_GD_HI20)
#define R_RISCV_TLS_GD_HI20 22
#endif
#if !defined(R_RISCV_PCREL_HI20)
#define R_RISCV_PCREL_HI20 23
#endif
#if !defined(R_RISCV_PCREL_LO12_I)
#define R_RISCV_PCREL_LO12_I 24
#endif
#if !defined(R_RISCV_PCREL_LO12_S)
#define R_RISCV_PCREL_LO12_S 25
#endif
#if !defined(R_RISCV_HI20)
#define R_RISCV_HI20 26
#endif
#if !defined(R_RISCV_LO12_I)
#define R_RISCV_LO12_I 27
#endif
#if !defined(R_RISCV_LO12_S)
#define R_RISCV_LO12_S 28
#endif
#if !defined(R_RISCV_TPREL_HI20)
#define R_RISCV_TPREL_HI20 29
#endif
#if !defined(R_RISCV_TPREL_LO12_I)
#define R_RISCV_TPREL_LO12_I 30
#endif
#if !defined(R_RISCV_TPREL_LO12_S)
#define R_RISCV_TPREL_LO12_S 31
#endif
#if !defined(R_RISCV_TPREL_ADD)
#define R_RISCV_TPREL_ADD 32
#endif
#if !defined(R_RISCV_ADD8)
#define R_RISCV_ADD8 33
#endif
#if !defined(R_RISCV_ADD16)
#define R_RISCV_ADD16 34
#endif
#if !defined(R_RISCV_ADD32)
#define R_RISCV_ADD32 35
#endif
#if !defined(R_RISCV_ADD64)
#define R_RISCV_ADD64 36
#endif
#if !defined(R_RISCV_SUB8)
#define R_RISCV_SUB8 37
#endif
#if !defined(R_RISCV_SUB16)
#define R_RISCV_SUB16 38
#endif
#if !defined(R_RISCV_SUB32)
#define R_RISCV_SUB32 39
#endif
#if !defined(R_RISCV_SUB64)
#define R_RISCV_SUB64 40
#endif
#if !defined(R_RISCV_GNU_VTINHERIT)
#define R_RISCV_GNU_VTINHERIT 41
#endif
#if !defined(R_RISCV_GNU_VTENTRY)
#define R_RISCV_GNU_VTENTRY 42
#endif
#if !defined(R_RISCV_ALIGN)
#define R_RISCV_ALIGN 43
#endif
#if !defined(R_RISCV_RVC_BRANCH)
#define R_RISCV_RVC_BRANCH 44
#endif
#if !defined(R_RISCV_RVC_JUMP)
#define R_RISCV_RVC_JUMP 45
#endif
#if !defined(R_RISCV_RVC_LUI)
#define R_RISCV_RVC_LUI 46
#endif
#if !defined(R_RISCV_GPREL_I)
#define R_RISCV_GPREL_I 47
#endif
#if !defined(R_RISCV_GPREL_S)
#define R_RISCV_GPREL_S 48
#endif
#if !defined(R_RISCV_TPREL_I)
#define R_RISCV_TPREL_I 49
#endif
#if !defined(R_RISCV_TPREL_S)
#define R_RISCV_TPREL_S 50
#endif
#if !defined(R_RISCV_RELAX)
#define R_RISCV_RELAX 51
#endif
#if !defined(R_RISCV_SUB6)
#define R_RISCV_SUB6 52
#endif
#if !defined(R_RISCV_SET6)
#define R_RISCV_SET6 53
#endif
#if !defined(R_RISCV_SET8)
#define R_RISCV_SET8 54
#endif
#if !defined(R_RISCV_SET16)
#define R_RISCV_SET16 55
#endif
#if !defined(R_RISCV_SET32)
#define R_RISCV_SET32 56
#endif
#if !defined(R_RISCV_32_PCREL)
#define R_RISCV_32_PCREL 57
#endif
#if !defined(R_RISCV_IRELATIVE)
#define R_RISCV_IRELATIVE 58
#endif
#if !defined(R_RISCV_PLT32)
#define R_RISCV_PLT32 59
#endif
#if !defined(R_RISCV_SET_ULEB128)
#define R_RISCV_SET_ULEB128 60
#endif
#if !defined(R_RISCV_SUB_ULEB128)
#define R_RISCV_SUB_ULEB128 61
#endif
#if !defined(R_RISCV_NUM)
#define R_RISCV_NUM 62
#endif

#endif
