/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

/* Functions of the relocationEntry class specific to SPARC ELF */
#define ELF_TARGET_SPARC 1
#include "Symtab.h"
#include "Serialization.h"
#include "annotations.h"
#include <elf.h>

const char* relocationEntry::relType2Str(unsigned long r, unsigned /*addressWidth*/) {
    switch(r) {
        CASE_RETURN_STR(R_SPARC_NONE);
        CASE_RETURN_STR(R_SPARC_8);
        CASE_RETURN_STR(R_SPARC_16);
        CASE_RETURN_STR(R_SPARC_32);
        CASE_RETURN_STR(R_SPARC_DISP8);
        CASE_RETURN_STR(R_SPARC_DISP16);
        CASE_RETURN_STR(R_SPARC_DISP32);
        CASE_RETURN_STR(R_SPARC_WDISP30);
        CASE_RETURN_STR(R_SPARC_WDISP22);
        CASE_RETURN_STR(R_SPARC_HI22);
        CASE_RETURN_STR(R_SPARC_22);
        CASE_RETURN_STR(R_SPARC_13);
        CASE_RETURN_STR(R_SPARC_LO10);
        CASE_RETURN_STR(R_SPARC_GOT10);
        CASE_RETURN_STR(R_SPARC_GOT13);
        CASE_RETURN_STR(R_SPARC_GOT22);
        CASE_RETURN_STR(R_SPARC_PC10);
        CASE_RETURN_STR(R_SPARC_PC22);
        CASE_RETURN_STR(R_SPARC_WPLT30);
        CASE_RETURN_STR(R_SPARC_COPY);
        CASE_RETURN_STR(R_SPARC_GLOB_DAT);
        CASE_RETURN_STR(R_SPARC_JMP_SLOT);
        CASE_RETURN_STR(R_SPARC_RELATIVE);
        CASE_RETURN_STR(R_SPARC_UA32);
        CASE_RETURN_STR(R_SPARC_PLT32);
        CASE_RETURN_STR(R_SPARC_HIPLT22);
        CASE_RETURN_STR(R_SPARC_LOPLT10);
        CASE_RETURN_STR(R_SPARC_PCPLT32);
        CASE_RETURN_STR(R_SPARC_PCPLT22);
        CASE_RETURN_STR(R_SPARC_PCPLT10);
        CASE_RETURN_STR(R_SPARC_10);
        CASE_RETURN_STR(R_SPARC_11);
        CASE_RETURN_STR(R_SPARC_64);
        CASE_RETURN_STR(R_SPARC_OLO10);
        CASE_RETURN_STR(R_SPARC_HH22);
        CASE_RETURN_STR(R_SPARC_HM10);
        CASE_RETURN_STR(R_SPARC_LM22);
        CASE_RETURN_STR(R_SPARC_PC_HH22);
        CASE_RETURN_STR(R_SPARC_PC_HM10);
        CASE_RETURN_STR(R_SPARC_PC_LM22);
        CASE_RETURN_STR(R_SPARC_WDISP16);
        CASE_RETURN_STR(R_SPARC_WDISP19);
        CASE_RETURN_STR(R_SPARC_7);
        CASE_RETURN_STR(R_SPARC_5);
        CASE_RETURN_STR(R_SPARC_6);
        CASE_RETURN_STR(R_SPARC_DISP64);
        CASE_RETURN_STR(R_SPARC_PLT64);
        CASE_RETURN_STR(R_SPARC_HIX22);
        CASE_RETURN_STR(R_SPARC_LOX10);
        CASE_RETURN_STR(R_SPARC_H44);
        CASE_RETURN_STR(R_SPARC_M44);
        CASE_RETURN_STR(R_SPARC_L44);
        CASE_RETURN_STR(R_SPARC_REGISTER);
        CASE_RETURN_STR(R_SPARC_UA64);
        CASE_RETURN_STR(R_SPARC_UA16);
        CASE_RETURN_STR(R_SPARC_TLS_GD_HI22);
        CASE_RETURN_STR(R_SPARC_TLS_GD_LO10);
        CASE_RETURN_STR(R_SPARC_TLS_GD_ADD);
        CASE_RETURN_STR(R_SPARC_TLS_GD_CALL);
        CASE_RETURN_STR(R_SPARC_TLS_LDM_HI22);
        CASE_RETURN_STR(R_SPARC_TLS_LDM_LO10);
        CASE_RETURN_STR(R_SPARC_TLS_LDM_ADD);
        CASE_RETURN_STR(R_SPARC_TLS_LDM_CALL);
        CASE_RETURN_STR(R_SPARC_TLS_LDO_HIX22);
        CASE_RETURN_STR(R_SPARC_TLS_LDO_LOX10);
        CASE_RETURN_STR(R_SPARC_TLS_LDO_ADD);
        CASE_RETURN_STR(R_SPARC_TLS_IE_HI22);
        CASE_RETURN_STR(R_SPARC_TLS_IE_LO10);
        CASE_RETURN_STR(R_SPARC_TLS_IE_LD);
        CASE_RETURN_STR(R_SPARC_TLS_IE_LDX);
        CASE_RETURN_STR(R_SPARC_TLS_IE_ADD);
        CASE_RETURN_STR(R_SPARC_TLS_LE_HIX22);
        CASE_RETURN_STR(R_SPARC_TLS_LE_LOX10);
        CASE_RETURN_STR(R_SPARC_TLS_DTPMOD32);
        CASE_RETURN_STR(R_SPARC_TLS_DTPMOD64);
        CASE_RETURN_STR(R_SPARC_TLS_DTPOFF32);
        CASE_RETURN_STR(R_SPARC_TLS_DTPOFF64);
        CASE_RETURN_STR(R_SPARC_TLS_TPOFF32);
        CASE_RETURN_STR(R_SPARC_TLS_TPOFF64);
        default:
            return "?";
    }
}

SYMTAB_EXPORT unsigned long relocationEntry::getGlobalRelType(unsigned /*addressWidth*/) {
    return R_SPARC_GLOB_DAT;
}
