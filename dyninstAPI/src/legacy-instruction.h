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

#ifndef LEGACY_BPATCH_INSTRUCTION_H
#define LEGACY_BPATCH_INSTRUCTION_H

/*
 * Legacy support for BPatch_instruction and BPatch_memoryAccess,
 * both of which hold a pointer to an opaque type containing the
 * platform-specific `instruction' type.
 */

#include "arch-forward-decl.h"

#if defined(DYNINST_HOST_ARCH_POWER)
using namespace NS_power;

#elif defined(i386_unknown_nt4_0) \
   || defined(DYNINST_HOST_ARCH_X86)           \
   || defined(DYNINST_HOST_ARCH_X86_64)
using namespace NS_x86;

#elif defined(DYNINST_HOST_ARCH_AARCH64)
using namespace NS_aarch64;

#else
#error "unknown architecture"

#endif

class internal_instruction {
 public:
    explicit internal_instruction(instruction * insn)
        : _insn(insn)
    { }

    instruction * insn() const { return _insn; }
 private:
    instruction * _insn; 
};


#endif 
