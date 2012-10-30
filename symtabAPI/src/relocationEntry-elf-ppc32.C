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

/* Functions of the relocationEntry class specific to PPC ELF */
#include "Symtab.h"
#include "Serialization.h"
#include "annotations.h"
#include <elf.h>

const char* relocationEntry::relType2Str(unsigned long r, unsigned /*addressWidth*/) {
    switch(r) {
        CASE_RETURN_STR(R_PPC_NONE);
        CASE_RETURN_STR(R_PPC_ADDR32);
        CASE_RETURN_STR(R_PPC_ADDR24);
        CASE_RETURN_STR(R_PPC_ADDR16);
        CASE_RETURN_STR(R_PPC_ADDR16_LO);
        CASE_RETURN_STR(R_PPC_ADDR16_HI);
        CASE_RETURN_STR(R_PPC_ADDR16_HA);
        CASE_RETURN_STR(R_PPC_ADDR14);
        CASE_RETURN_STR(R_PPC_ADDR14_BRTAKEN);
        CASE_RETURN_STR(R_PPC_ADDR14_BRNTAKEN);
        CASE_RETURN_STR(R_PPC_REL24);
        CASE_RETURN_STR(R_PPC_REL14);
        CASE_RETURN_STR(R_PPC_REL14_BRTAKEN);
        CASE_RETURN_STR(R_PPC_REL14_BRNTAKEN);
        CASE_RETURN_STR(R_PPC_GOT16);
        CASE_RETURN_STR(R_PPC_GOT16_LO);
        CASE_RETURN_STR(R_PPC_GOT16_HI);
        CASE_RETURN_STR(R_PPC_GOT16_HA);
        CASE_RETURN_STR(R_PPC_PLTREL24);
        CASE_RETURN_STR(R_PPC_COPY);
        CASE_RETURN_STR(R_PPC_GLOB_DAT);
        CASE_RETURN_STR(R_PPC_JMP_SLOT);
        CASE_RETURN_STR(R_PPC_RELATIVE);
        CASE_RETURN_STR(R_PPC_LOCAL24PC);
        CASE_RETURN_STR(R_PPC_UADDR32);
        CASE_RETURN_STR(R_PPC_UADDR16);
        CASE_RETURN_STR(R_PPC_REL32);
        CASE_RETURN_STR(R_PPC_PLT32);
        CASE_RETURN_STR(R_PPC_PLTREL32);
        CASE_RETURN_STR(R_PPC_PLT16_LO);
        CASE_RETURN_STR(R_PPC_PLT16_HI);
        CASE_RETURN_STR(R_PPC_PLT16_HA);
        CASE_RETURN_STR(R_PPC_SDAREL16);
        CASE_RETURN_STR(R_PPC_SECTOFF);
        CASE_RETURN_STR(R_PPC_SECTOFF_LO);
        CASE_RETURN_STR(R_PPC_SECTOFF_HI);
        CASE_RETURN_STR(R_PPC_SECTOFF_HA);
        CASE_RETURN_STR(R_PPC_TLS);
        CASE_RETURN_STR(R_PPC_DTPMOD32);
        CASE_RETURN_STR(R_PPC_TPREL16);
        CASE_RETURN_STR(R_PPC_TPREL16_LO);
        CASE_RETURN_STR(R_PPC_TPREL16_HI);
        CASE_RETURN_STR(R_PPC_TPREL16_HA);
        CASE_RETURN_STR(R_PPC_TPREL32);
        CASE_RETURN_STR(R_PPC_DTPREL16);
        CASE_RETURN_STR(R_PPC_DTPREL16_LO);
        CASE_RETURN_STR(R_PPC_DTPREL16_HI);
        CASE_RETURN_STR(R_PPC_DTPREL16_HA);
        CASE_RETURN_STR(R_PPC_DTPREL32);
        CASE_RETURN_STR(R_PPC_GOT_TLSGD16);
        CASE_RETURN_STR(R_PPC_GOT_TLSGD16_LO);
        CASE_RETURN_STR(R_PPC_GOT_TLSGD16_HI);
        CASE_RETURN_STR(R_PPC_GOT_TLSGD16_HA);
        CASE_RETURN_STR(R_PPC_GOT_TLSLD16);
        CASE_RETURN_STR(R_PPC_GOT_TLSLD16_LO);
        CASE_RETURN_STR(R_PPC_GOT_TLSLD16_HI);
        CASE_RETURN_STR(R_PPC_GOT_TLSLD16_HA);
        CASE_RETURN_STR(R_PPC_GOT_TPREL16);
        CASE_RETURN_STR(R_PPC_GOT_TPREL16_LO);
        CASE_RETURN_STR(R_PPC_GOT_TPREL16_HI);
        CASE_RETURN_STR(R_PPC_GOT_TPREL16_HA);
        CASE_RETURN_STR(R_PPC_GOT_DTPREL16);
        CASE_RETURN_STR(R_PPC_GOT_DTPREL16_LO);
        CASE_RETURN_STR(R_PPC_GOT_DTPREL16_HI);
        CASE_RETURN_STR(R_PPC_GOT_DTPREL16_HA);
#if defined(R_PPC_NUM)
        CASE_RETURN_STR(R_PPC_NUM);
#endif
        CASE_RETURN_STR(R_PPC_EMB_NADDR32);
        CASE_RETURN_STR(R_PPC_EMB_NADDR16);
        CASE_RETURN_STR(R_PPC_EMB_NADDR16_LO);
        CASE_RETURN_STR(R_PPC_EMB_NADDR16_HI);
        CASE_RETURN_STR(R_PPC_EMB_NADDR16_HA);
        CASE_RETURN_STR(R_PPC_EMB_SDAI16);
        CASE_RETURN_STR(R_PPC_EMB_SDA2I16);
        CASE_RETURN_STR(R_PPC_EMB_SDA2REL);
        CASE_RETURN_STR(R_PPC_EMB_SDA21);
        CASE_RETURN_STR(R_PPC_EMB_MRKREF);
        CASE_RETURN_STR(R_PPC_EMB_RELSEC16);
        CASE_RETURN_STR(R_PPC_EMB_RELST_LO);
        CASE_RETURN_STR(R_PPC_EMB_RELST_HI);
        CASE_RETURN_STR(R_PPC_EMB_RELST_HA);
        CASE_RETURN_STR(R_PPC_EMB_BIT_FLD);
        CASE_RETURN_STR(R_PPC_EMB_RELSDA);
        CASE_RETURN_STR(R_PPC_DIAB_SDA21_LO);
        CASE_RETURN_STR(R_PPC_DIAB_SDA21_HI);
        CASE_RETURN_STR(R_PPC_DIAB_SDA21_HA);
        CASE_RETURN_STR(R_PPC_DIAB_RELSDA_LO);
        CASE_RETURN_STR(R_PPC_DIAB_RELSDA_HI);
        CASE_RETURN_STR(R_PPC_DIAB_RELSDA_HA);
#ifdef R_PPC_REL16
        // Older versions of elf.h may not have these defined.
        CASE_RETURN_STR(R_PPC_REL16);
        CASE_RETURN_STR(R_PPC_REL16_LO);
        CASE_RETURN_STR(R_PPC_REL16_HI);
        CASE_RETURN_STR(R_PPC_REL16_HA);
#endif
        CASE_RETURN_STR(R_PPC_TOC16);
        default:
            return "?";
    }
}

SYMTAB_EXPORT unsigned long relocationEntry::getGlobalRelType(unsigned /*addressWidth*/, Symbol *) {
    return R_PPC_GLOB_DAT;
}
