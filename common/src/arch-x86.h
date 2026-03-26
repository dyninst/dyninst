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

// $Id: arch-x86.h,v 1.67 2008/10/28 18:42:39 bernat Exp $
// x86 instruction declarations

#ifndef _ARCH_X86_H
#define _ARCH_X86_H

#include "dyntypes.h"
#include <assert.h>
#include <stdio.h>
#include <set>
#include <map>
#include <vector>
#include "entryIDs.h"
#include "registers/MachRegister.h"
#include "common/src/ia32_locations.h"
#include "dyn_register.h"
#include "encoding-x86.h"
#include "ia32_memacc.h"
#include "ia32_prefixes.h"
#include "ia32_entry.h"
#include "ia32_instruction.h"
#include "instruction-x86.h"

namespace NS_x86 {

typedef unsigned char codeBuf_t;
typedef unsigned codeBufIndex_t;


// VG(02/07/2002): Information that the decoder can return is
//   #defined below. The decoder always returns the size of the 
//   instruction because that has to be determined anyway.
//   Please don't add things that should be external to the
//   decoder, e.g.: how may bytes a relocated instruction needs
//   IMHO that stuff should go into inst-x86...

#define IA32_DECODE_PREFIXES	(1<<0)
#define IA32_DECODE_MNEMONICS	(1<<1)
#define IA32_DECODE_OPERANDS	(1<<2)
#define IA32_DECODE_JMPS	    (1<<3)
#define IA32_DECODE_MEMACCESS	(1<<4)
#define IA32_DECODE_CONDITION 	(1<<5)

#define IA32_FULL_DECODER (IA32_DECODE_PREFIXES \
        | IA32_DECODE_MNEMONICS \
        | IA32_DECODE_OPERANDS \
        | IA32_DECODE_JMPS \
        | IA32_DECODE_MEMACCESS \
        | IA32_DECODE_CONDITION)
#define IA32_SIZE_DECODER 0

/* TODO: documentation*/
DYNINST_EXPORT bool ia32_decode_prefixes(const unsigned char *addr, ia32_instruction &insn, bool mode_64);

/**
 * Decode just the opcode of the given instruction. This implies that
 * ia32_decode_prefixes has already been called on the given instruction
 * and addr has been moved past the prefix bytes. Returns zero on success,
 * non zero otherwise.
 */
DYNINST_EXPORT int
ia32_decode_opcode(unsigned int capa, const unsigned char *addr, ia32_instruction &instruct, ia32_entry **gotit_ret,
                   bool mode_64);

/**
 * Do a complete decoding of the instruction at the given address. This
 * function calls ia32_decode_prefixes, ia32_decode_opcode and
 * ia32_decode_operands. Returns zero on success, non zero otherwise.
 * When there is a decoding failure, the state of the given instruction
 * is not defined. capabilities is a mask of the above flags (IA32_DECODE_*).
 * The mask determines what part of the instruction should be decoded.
 */
DYNINST_EXPORT ia32_instruction &
ia32_decode(unsigned int capabilities, const unsigned char *addr, ia32_instruction &, bool mode_64);

inline bool is_disp8(long disp) {
   return (disp >= -128 && disp < 127);
}

inline bool is_disp32(long disp) {
  return (disp <= INT32_MAX && disp >= INT32_MIN);
}
inline bool is_disp32(Dyninst::Address a1, Dyninst::Address a2) {
  return is_disp32(a2 - (a1 + JUMP_REL32_SZ));
}
inline bool is_addr32(Dyninst::Address addr) {
    return (addr < UINT32_MAX);
}

} // namespace arch_x86

#endif

