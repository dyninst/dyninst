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

/* Functions of the relocationEntry class specific to AArch64 ELF */
#include <elf.h>
#include "Symtab.h"
#include "annotations.h"

// TODO

using namespace Dyninst;
using namespace SymtabAPI;

const char *relocationEntry::relType2Str(unsigned long r, unsigned /*addressWidth*/) {
    switch (r) {
        CASE_RETURN_STR(R_RISCV_NONE);
        CASE_RETURN_STR(R_RISCV_32);
        CASE_RETURN_STR(R_RISCV_64);
        CASE_RETURN_STR(R_RISCV_RELATIVE);
        CASE_RETURN_STR(R_RISCV_COPY);
        CASE_RETURN_STR(R_RISCV_JUMP_SLOT);
        CASE_RETURN_STR(R_RISCV_TLS_DTPMOD32);
        CASE_RETURN_STR(R_RISCV_TLS_DTPMOD64);
        CASE_RETURN_STR(R_RISCV_TLS_DTPREL32);
        CASE_RETURN_STR(R_RISCV_TLS_DTPREL64);
        CASE_RETURN_STR(R_RISCV_TLS_TPREL32);
        CASE_RETURN_STR(R_RISCV_TLS_TPREL64);
        CASE_RETURN_STR(R_RISCV_BRANCH);
        CASE_RETURN_STR(R_RISCV_JAL);
        CASE_RETURN_STR(R_RISCV_CALL);
        CASE_RETURN_STR(R_RISCV_CALL_PLT);
        CASE_RETURN_STR(R_RISCV_GOT_HI20);
        CASE_RETURN_STR(R_RISCV_TLS_GOT_HI20);
        CASE_RETURN_STR(R_RISCV_TLS_GD_HI20);
        CASE_RETURN_STR(R_RISCV_PCREL_HI20);
        CASE_RETURN_STR(R_RISCV_PCREL_LO12_I);
        CASE_RETURN_STR(R_RISCV_PCREL_LO12_S);
        CASE_RETURN_STR(R_RISCV_HI20);
        CASE_RETURN_STR(R_RISCV_LO12_I);
        CASE_RETURN_STR(R_RISCV_LO12_S);
        CASE_RETURN_STR(R_RISCV_TPREL_HI20);
        CASE_RETURN_STR(R_RISCV_TPREL_LO12_I);
        CASE_RETURN_STR(R_RISCV_TPREL_LO12_S);
        CASE_RETURN_STR(R_RISCV_TPREL_ADD);
        CASE_RETURN_STR(R_RISCV_ADD8);
        CASE_RETURN_STR(R_RISCV_ADD16);
        CASE_RETURN_STR(R_RISCV_ADD32);
        CASE_RETURN_STR(R_RISCV_ADD64);
        CASE_RETURN_STR(R_RISCV_SUB8);
        CASE_RETURN_STR(R_RISCV_SUB16);
        CASE_RETURN_STR(R_RISCV_SUB32);
        CASE_RETURN_STR(R_RISCV_SUB64);
        CASE_RETURN_STR(R_RISCV_GNU_VTINHERIT);
        CASE_RETURN_STR(R_RISCV_GNU_VTENTRY);
        CASE_RETURN_STR(R_RISCV_ALIGN);
        CASE_RETURN_STR(R_RISCV_RVC_BRANCH);
        CASE_RETURN_STR(R_RISCV_RVC_JUMP);
        CASE_RETURN_STR(R_RISCV_RVC_LUI);
        CASE_RETURN_STR(R_RISCV_GPREL_I);
        CASE_RETURN_STR(R_RISCV_GPREL_S);
        CASE_RETURN_STR(R_RISCV_TPREL_I);
        CASE_RETURN_STR(R_RISCV_TPREL_S);
        CASE_RETURN_STR(R_RISCV_RELAX);
        CASE_RETURN_STR(R_RISCV_SUB6);
        CASE_RETURN_STR(R_RISCV_SET6);
        CASE_RETURN_STR(R_RISCV_SET8);
        CASE_RETURN_STR(R_RISCV_SET16);
        CASE_RETURN_STR(R_RISCV_SET32);
        CASE_RETURN_STR(R_RISCV_32_PCREL);
        CASE_RETURN_STR(R_RISCV_IRELATIVE);
        CASE_RETURN_STR(R_RISCV_PLT32);
        CASE_RETURN_STR(R_RISCV_SET_ULEB128);
        CASE_RETURN_STR(R_RISCV_SUB_ULEB128);
        default:
            return "Unknown relocation type";
    }
    return "?";
}

DYNINST_EXPORT unsigned long relocationEntry::getGlobalRelType(unsigned /*addressWidth*/, Symbol *sym) {
    if (!sym) {
        return R_RISCV_64;
    }
    if (sym->getType() == Symbol::ST_FUNCTION) {
        return R_RISCV_JUMP_SLOT;
    }
    else {
        return R_RISCV_64;
    }
}


relocationEntry::category
relocationEntry::getCategory( unsigned /*addressWidth*/ )
{
    switch (getRelType()) {
        case R_RISCV_RELATIVE:
        case R_RISCV_IRELATIVE:
            return category::relative;
        case R_RISCV_JUMP_SLOT:
            return category::jump_slot;
        default:
            return category::absolute;
    }
}
