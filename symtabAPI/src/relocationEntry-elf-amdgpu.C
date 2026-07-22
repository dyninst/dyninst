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

/* Functions of the relocationEntry class specific to AMDGPU ELF. */

#include <elf.h>
#include "Symtab.h"

using namespace Dyninst;
using namespace SymtabAPI;

// AMDGPU dynamic-relocation type values from the AMDGPU ABI (LLVM).
// Stock <elf.h> from glibc does not yet define R_AMDGPU_*, so we provide
// the values directly. Guarded so we don't collide once it does.
#ifndef R_AMDGPU_NONE
#  define R_AMDGPU_NONE           0
#  define R_AMDGPU_ABS32_LO       1
#  define R_AMDGPU_ABS32_HI       2
#  define R_AMDGPU_ABS64          3
#  define R_AMDGPU_REL32          4
#  define R_AMDGPU_REL64          5
#  define R_AMDGPU_ABS32          6
#  define R_AMDGPU_GOTPCREL       7
#  define R_AMDGPU_GOTPCREL32_LO  8
#  define R_AMDGPU_GOTPCREL32_HI  9
#  define R_AMDGPU_REL32_LO      10
#  define R_AMDGPU_REL32_HI      11
#  define R_AMDGPU_RELATIVE64    13
#endif

const char *relocationEntry::relType2Str(unsigned long r, unsigned /*addressWidth*/) {
    switch (r) {
        CASE_RETURN_STR(R_AMDGPU_NONE);
        CASE_RETURN_STR(R_AMDGPU_ABS32_LO);
        CASE_RETURN_STR(R_AMDGPU_ABS32_HI);
        CASE_RETURN_STR(R_AMDGPU_ABS64);
        CASE_RETURN_STR(R_AMDGPU_REL32);
        CASE_RETURN_STR(R_AMDGPU_REL64);
        CASE_RETURN_STR(R_AMDGPU_ABS32);
        CASE_RETURN_STR(R_AMDGPU_GOTPCREL);
        CASE_RETURN_STR(R_AMDGPU_GOTPCREL32_LO);
        CASE_RETURN_STR(R_AMDGPU_GOTPCREL32_HI);
        CASE_RETURN_STR(R_AMDGPU_REL32_LO);
        CASE_RETURN_STR(R_AMDGPU_REL32_HI);
        CASE_RETURN_STR(R_AMDGPU_RELATIVE64);
        default:
            return "Unknown AMDGPU relocation type";
    }
}

DYNINST_EXPORT unsigned long relocationEntry::getGlobalRelType(unsigned /*addressWidth*/,
                                                               Symbol * /*sym*/) {
    // AMDGPU is 64-bit. Use an absolute 64-bit relocation so the loader
    // writes the resolved address of the target symbol into the slot.
    return R_AMDGPU_ABS64;
}

relocationEntry::category
relocationEntry::getCategory(unsigned /*addressWidth*/) {
    switch (getRelType()) {
        case R_AMDGPU_RELATIVE64:
            return category::relative;
        default:
            return category::absolute;
    }
}
