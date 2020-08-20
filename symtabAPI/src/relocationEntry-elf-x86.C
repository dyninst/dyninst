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

/* Functions of the relocationEntry class specific to x86 ELF */
#include "Symtab.h"
#include "annotations.h"
#include <elf.h>

#if defined(os_freebsd)
#define R_X86_64_JUMP_SLOT R_X86_64_JMP_SLOT
#endif

static const unsigned X86_64_WIDTH = 8;

const char* relocationEntry::relType2Str(unsigned long r, unsigned addressWidth) {
    if( X86_64_WIDTH == addressWidth ) {
        switch(r) {
            CASE_RETURN_STR(R_X86_64_NONE);
            CASE_RETURN_STR(R_X86_64_64);
            CASE_RETURN_STR(R_X86_64_PC32);
            CASE_RETURN_STR(R_X86_64_GOT32);
            CASE_RETURN_STR(R_X86_64_PLT32);
            CASE_RETURN_STR(R_X86_64_COPY);
            CASE_RETURN_STR(R_X86_64_GLOB_DAT);
            CASE_RETURN_STR(R_X86_64_RELATIVE);
#if defined(R_X86_64_IRELATIVE)
            CASE_RETURN_STR(R_X86_64_IRELATIVE);
#endif
            CASE_RETURN_STR(R_X86_64_GOTPCREL);
            CASE_RETURN_STR(R_X86_64_32);
            CASE_RETURN_STR(R_X86_64_32S);
            CASE_RETURN_STR(R_X86_64_16);
            CASE_RETURN_STR(R_X86_64_PC16);
            CASE_RETURN_STR(R_X86_64_8);
            CASE_RETURN_STR(R_X86_64_PC8);
            CASE_RETURN_STR(R_X86_64_DTPMOD64);
            CASE_RETURN_STR(R_X86_64_DTPOFF64);
            CASE_RETURN_STR(R_X86_64_TPOFF64);
            CASE_RETURN_STR(R_X86_64_TLSGD);
            CASE_RETURN_STR(R_X86_64_TLSLD);
            CASE_RETURN_STR(R_X86_64_DTPOFF32);
            CASE_RETURN_STR(R_X86_64_GOTTPOFF);
            CASE_RETURN_STR(R_X86_64_TPOFF32);
            CASE_RETURN_STR(R_X86_64_JUMP_SLOT);
            default:
                return "?";
        }
    }else{
        switch(r) {
            CASE_RETURN_STR(R_386_NONE);
            CASE_RETURN_STR(R_386_32);
            CASE_RETURN_STR(R_386_PC32);
            CASE_RETURN_STR(R_386_GOT32);
            CASE_RETURN_STR(R_386_PLT32);
            CASE_RETURN_STR(R_386_COPY);
            CASE_RETURN_STR(R_386_GLOB_DAT);
            CASE_RETURN_STR(R_386_JMP_SLOT);
            CASE_RETURN_STR(R_386_RELATIVE);
            CASE_RETURN_STR(R_386_GOTOFF);
            CASE_RETURN_STR(R_386_GOTPC);
            CASE_RETURN_STR(R_386_TLS_TPOFF);
            CASE_RETURN_STR(R_386_TLS_IE);
            CASE_RETURN_STR(R_386_TLS_GOTIE);
            CASE_RETURN_STR(R_386_TLS_LE);
            CASE_RETURN_STR(R_386_TLS_GD);
            CASE_RETURN_STR(R_386_TLS_LDM);
            CASE_RETURN_STR(R_386_TLS_GD_32);
            CASE_RETURN_STR(R_386_TLS_GD_PUSH);
            CASE_RETURN_STR(R_386_TLS_GD_CALL);
            CASE_RETURN_STR(R_386_TLS_GD_POP);
            CASE_RETURN_STR(R_386_TLS_LDM_32);
            CASE_RETURN_STR(R_386_TLS_LDM_PUSH);
            CASE_RETURN_STR(R_386_TLS_LDM_CALL);
            CASE_RETURN_STR(R_386_TLS_LDM_POP);
            CASE_RETURN_STR(R_386_TLS_LDO_32);
            CASE_RETURN_STR(R_386_TLS_IE_32);
            CASE_RETURN_STR(R_386_TLS_LE_32);
            CASE_RETURN_STR(R_386_TLS_DTPMOD32);
            CASE_RETURN_STR(R_386_TLS_DTPOFF32);
            CASE_RETURN_STR(R_386_TLS_TPOFF32);
#if !defined(os_freebsd)
            CASE_RETURN_STR(R_386_16);
            CASE_RETURN_STR(R_386_PC16);
            CASE_RETURN_STR(R_386_8);
            CASE_RETURN_STR(R_386_PC8);
            CASE_RETURN_STR(R_386_32PLT);
#endif
            default:
                return "?";
        }
    }
}

SYMTAB_EXPORT unsigned long relocationEntry::getGlobalRelType(unsigned addressWidth, Symbol *) {
    if( X86_64_WIDTH == addressWidth ) {
        return R_X86_64_GLOB_DAT;
    }else{
        return R_386_GLOB_DAT;
    }
}


relocationEntry::category
relocationEntry::getCategory( unsigned addressWidth )
{
    if( addressWidth == 8 ) {
       switch( getRelType() )
       {
           case R_X86_64_RELATIVE:
           case R_X86_64_IRELATIVE:
               return category::relative; 
           case R_X86_64_JUMP_SLOT:
               return category::jump_slot; 
           default:
               return category::absolute;
       }
    }else{
       switch( getRelType() )
       {
           case R_386_RELATIVE:
           case R_386_IRELATIVE:
               return category::relative; 
           case R_386_JMP_SLOT:
               return category::jump_slot; 
           default:
               return category::absolute;
       }
    }
}



