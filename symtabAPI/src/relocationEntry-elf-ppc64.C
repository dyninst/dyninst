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

/* Functions of the relocationEntry class specific to PPC64 ELF */
#include <elf.h>
#include "Symtab.h"
#include "annotations.h"

const char* relocationEntry::relType2Str(unsigned long r, unsigned /*addressWidth*/) {
    switch(r) {
        CASE_RETURN_STR(R_PPC64_NONE);
        CASE_RETURN_STR(R_PPC64_ADDR32);
        CASE_RETURN_STR(R_PPC64_ADDR24);
        CASE_RETURN_STR(R_PPC64_ADDR16);
        CASE_RETURN_STR(R_PPC64_ADDR16_LO);
        CASE_RETURN_STR(R_PPC64_ADDR16_HI);
        CASE_RETURN_STR(R_PPC64_ADDR16_HA);
        CASE_RETURN_STR(R_PPC64_ADDR14);
        CASE_RETURN_STR(R_PPC64_ADDR14_BRTAKEN);
        CASE_RETURN_STR(R_PPC64_ADDR14_BRNTAKEN);
        CASE_RETURN_STR(R_PPC64_REL24);
        CASE_RETURN_STR(R_PPC64_REL14);
        CASE_RETURN_STR(R_PPC64_REL14_BRTAKEN);
        CASE_RETURN_STR(R_PPC64_REL14_BRNTAKEN);
        CASE_RETURN_STR(R_PPC64_GOT16);
        CASE_RETURN_STR(R_PPC64_GOT16_LO);
        CASE_RETURN_STR(R_PPC64_GOT16_HI);
        CASE_RETURN_STR(R_PPC64_GOT16_HA);
        CASE_RETURN_STR(R_PPC64_COPY);
        CASE_RETURN_STR(R_PPC64_GLOB_DAT);
        CASE_RETURN_STR(R_PPC64_JMP_SLOT);
        CASE_RETURN_STR(R_PPC64_RELATIVE);
        CASE_RETURN_STR(R_PPC64_UADDR32);
        CASE_RETURN_STR(R_PPC64_UADDR16);
        CASE_RETURN_STR(R_PPC64_REL32);
        CASE_RETURN_STR(R_PPC64_PLT32);
        CASE_RETURN_STR(R_PPC64_PLTREL32);
        CASE_RETURN_STR(R_PPC64_PLT16_LO);
        CASE_RETURN_STR(R_PPC64_PLT16_HI);
        CASE_RETURN_STR(R_PPC64_PLT16_HA);
        CASE_RETURN_STR(R_PPC64_SECTOFF);
        CASE_RETURN_STR(R_PPC64_SECTOFF_LO);
        CASE_RETURN_STR(R_PPC64_SECTOFF_HI);
        CASE_RETURN_STR(R_PPC64_SECTOFF_HA);
        CASE_RETURN_STR(R_PPC64_ADDR30);
        CASE_RETURN_STR(R_PPC64_ADDR64);
        CASE_RETURN_STR(R_PPC64_ADDR16_HIGHER);
        CASE_RETURN_STR(R_PPC64_ADDR16_HIGHERA);
        CASE_RETURN_STR(R_PPC64_ADDR16_HIGHEST);
        CASE_RETURN_STR(R_PPC64_ADDR16_HIGHESTA);
        CASE_RETURN_STR(R_PPC64_UADDR64);
        CASE_RETURN_STR(R_PPC64_REL64);
        CASE_RETURN_STR(R_PPC64_PLT64);
        CASE_RETURN_STR(R_PPC64_PLTREL64);
        CASE_RETURN_STR(R_PPC64_TOC16);
        CASE_RETURN_STR(R_PPC64_TOC16_LO);
        CASE_RETURN_STR(R_PPC64_TOC16_HI);
        CASE_RETURN_STR(R_PPC64_TOC16_HA);
        CASE_RETURN_STR(R_PPC64_TOC);
        CASE_RETURN_STR(R_PPC64_PLTGOT16);
        CASE_RETURN_STR(R_PPC64_PLTGOT16_LO);
        CASE_RETURN_STR(R_PPC64_PLTGOT16_HI);
        CASE_RETURN_STR(R_PPC64_PLTGOT16_HA);
        CASE_RETURN_STR(R_PPC64_ADDR16_DS);
        CASE_RETURN_STR(R_PPC64_ADDR16_LO_DS);
        CASE_RETURN_STR(R_PPC64_GOT16_DS);
        CASE_RETURN_STR(R_PPC64_GOT16_LO_DS);
        CASE_RETURN_STR(R_PPC64_PLT16_LO_DS);
        CASE_RETURN_STR(R_PPC64_SECTOFF_DS);
        CASE_RETURN_STR(R_PPC64_SECTOFF_LO_DS);
        CASE_RETURN_STR(R_PPC64_TOC16_DS);
        CASE_RETURN_STR(R_PPC64_TOC16_LO_DS);
        CASE_RETURN_STR(R_PPC64_PLTGOT16_DS);
        CASE_RETURN_STR(R_PPC64_PLTGOT16_LO_DS);
        CASE_RETURN_STR(R_PPC64_TLS);
        CASE_RETURN_STR(R_PPC64_DTPMOD64);
        CASE_RETURN_STR(R_PPC64_TPREL16);
        CASE_RETURN_STR(R_PPC64_TPREL16_LO);
        CASE_RETURN_STR(R_PPC64_TPREL16_HI);
        CASE_RETURN_STR(R_PPC64_TPREL16_HA);
        CASE_RETURN_STR(R_PPC64_TPREL64);
        CASE_RETURN_STR(R_PPC64_DTPREL16);
        CASE_RETURN_STR(R_PPC64_DTPREL16_LO);
        CASE_RETURN_STR(R_PPC64_DTPREL16_HI);
        CASE_RETURN_STR(R_PPC64_DTPREL16_HA);
        CASE_RETURN_STR(R_PPC64_DTPREL64);
        CASE_RETURN_STR(R_PPC64_GOT_TLSGD16);
        CASE_RETURN_STR(R_PPC64_GOT_TLSGD16_LO);
        CASE_RETURN_STR(R_PPC64_GOT_TLSGD16_HI);
        CASE_RETURN_STR(R_PPC64_GOT_TLSGD16_HA);
        CASE_RETURN_STR(R_PPC64_GOT_TLSLD16);
        CASE_RETURN_STR(R_PPC64_GOT_TLSLD16_LO);
        CASE_RETURN_STR(R_PPC64_GOT_TLSLD16_HI);
        CASE_RETURN_STR(R_PPC64_GOT_TLSLD16_HA);
        CASE_RETURN_STR(R_PPC64_GOT_TPREL16_DS);
        CASE_RETURN_STR(R_PPC64_GOT_TPREL16_LO_DS);
        CASE_RETURN_STR(R_PPC64_GOT_TPREL16_HI);
        CASE_RETURN_STR(R_PPC64_GOT_TPREL16_HA);
        CASE_RETURN_STR(R_PPC64_GOT_DTPREL16_DS);
        CASE_RETURN_STR(R_PPC64_GOT_DTPREL16_LO_DS);
        CASE_RETURN_STR(R_PPC64_GOT_DTPREL16_HI);
        CASE_RETURN_STR(R_PPC64_GOT_DTPREL16_HA);
        CASE_RETURN_STR(R_PPC64_TPREL16_DS);
        CASE_RETURN_STR(R_PPC64_TPREL16_LO_DS);
        CASE_RETURN_STR(R_PPC64_TPREL16_HIGHER);
        CASE_RETURN_STR(R_PPC64_TPREL16_HIGHERA);
        CASE_RETURN_STR(R_PPC64_TPREL16_HIGHEST);
        CASE_RETURN_STR(R_PPC64_TPREL16_HIGHESTA);
        CASE_RETURN_STR(R_PPC64_DTPREL16_DS);
        CASE_RETURN_STR(R_PPC64_DTPREL16_LO_DS);
        CASE_RETURN_STR(R_PPC64_DTPREL16_HIGHER);
        CASE_RETURN_STR(R_PPC64_DTPREL16_HIGHERA);
        CASE_RETURN_STR(R_PPC64_DTPREL16_HIGHEST);
        CASE_RETURN_STR(R_PPC64_DTPREL16_HIGHESTA);
#if defined(R_PPC64_NUM)
        CASE_RETURN_STR(R_PPC64_NUM);
#endif
        default:
            return "?";
    }
}

SYMTAB_EXPORT unsigned long relocationEntry::getGlobalRelType(unsigned addressWidth, Symbol *sym) {
  if (addressWidth == 4)
    return R_PPC_GLOB_DAT;

  if (!sym) return R_PPC64_GLOB_DAT;
  if (sym->getType() == Symbol::ST_FUNCTION)
    return R_PPC64_JMP_SLOT;
  else
    return R_PPC64_GLOB_DAT;
}
