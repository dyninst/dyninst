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

/* Functions of the relocationEntry class specific to LoongArch64 ELF */
#include <elf.h>
#include "Symtab.h"
#include "annotations.h"

using namespace Dyninst;
using namespace SymtabAPI;

#ifndef R_LARCH_NONE
#define R_LARCH_NONE           0
#define R_LARCH_32             1
#define R_LARCH_64             2
#define R_LARCH_RELATIVE       3
#define R_LARCH_COPY           4
#define R_LARCH_JUMP_SLOT      5
#define R_LARCH_TLS_DTPMOD32   6
#define R_LARCH_TLS_DTPMOD64   7
#define R_LARCH_TLS_DTPREL32   8
#define R_LARCH_TLS_DTPREL64   9
#define R_LARCH_TLS_TPREL32   10
#define R_LARCH_TLS_TPREL64   11
#define R_LARCH_IRELATIVE     12
#endif

const char *relocationEntry::relType2Str(unsigned long r, unsigned /*addressWidth*/) {
    switch(r) {
        CASE_RETURN_STR(R_LARCH_NONE);
        CASE_RETURN_STR(R_LARCH_32);
        CASE_RETURN_STR(R_LARCH_64);
        CASE_RETURN_STR(R_LARCH_RELATIVE);
        CASE_RETURN_STR(R_LARCH_COPY);
        CASE_RETURN_STR(R_LARCH_JUMP_SLOT);
        CASE_RETURN_STR(R_LARCH_TLS_DTPMOD32);
        CASE_RETURN_STR(R_LARCH_TLS_DTPMOD64);
        CASE_RETURN_STR(R_LARCH_TLS_DTPREL32);
        CASE_RETURN_STR(R_LARCH_TLS_DTPREL64);
        CASE_RETURN_STR(R_LARCH_TLS_TPREL32);
        CASE_RETURN_STR(R_LARCH_TLS_TPREL64);
        CASE_RETURN_STR(R_LARCH_IRELATIVE);
        default:
            return "Unknown LoongArch64 relocation type";
    }
    return "?";
}

DYNINST_EXPORT unsigned long relocationEntry::getGlobalRelType(unsigned addressWidth, Symbol *sym) {
    if(addressWidth == 4)
        return R_LARCH_32;

    if(!sym)
        return R_LARCH_64;
    if(sym->getType() == Symbol::ST_FUNCTION)
        return R_LARCH_JUMP_SLOT;
    else
        return R_LARCH_64;

    return relocationEntry::dynrel;
}

relocationEntry::category
relocationEntry::getCategory( unsigned addressWidth )
{
    if( addressWidth == 8 ) {
       switch( getRelType() )
       {
           case R_LARCH_RELATIVE:
           case R_LARCH_IRELATIVE:
               return category::relative;
           case R_LARCH_JUMP_SLOT:
               return category::jump_slot;
           default:
               return category::absolute;
       }
    }else{
       switch( getRelType() )
       {
           case R_LARCH_RELATIVE:
           case R_LARCH_IRELATIVE:
               return category::relative;
           case R_LARCH_JUMP_SLOT:
               return category::jump_slot;
           default:
               return category::absolute;
       }
    }
}