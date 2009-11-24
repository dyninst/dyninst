/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* 
 * holds architecture specific functions for x86 architecture needed for the
 * static executable rewriter
 */

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <set>
#include <map>
#include <sstream>

#include "emitElf.h"
#include "Symtab.h"
#include "Symbol.h"
#include "Archive.h"
#include "Object.h"
#include "Region.h"
#include "debug.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

static const Offset GOT_RESERVED_SLOTS = 3;

/**
 * Computes the relocation value and copies it into the target location
 */
bool emitElf::archSpecificRelocation(char *targetData, relocationEntry &rel,
       Offset dest, Offset relOffset, Offset globalOffset, LinkMap &lmap,
       string &errMsg) 
{
    // All relocations on x86 are one word32 == Offset
    // Referring to the SYSV 386 supplement:
    // S = rel.getDynSym()->getOffset()
    // A = addend
    // P = relOffset
   
    Offset addend;
    if( rel.regionType() == Region::RT_REL ) {
        memcpy(&addend, &targetData[dest], sizeof(Offset));
    }else if( rel.regionType() == Region::RT_RELA ) {
        addend = rel.addend();
    }

    rewrite_printf("relocation for %s: S = %lx A = %lx P = %lx\n",
            rel.name().c_str(), rel.getDynSym()->getOffset(), addend, relOffset);

    Offset relocation = 0;
    map<Symbol *, Offset>::iterator result;
    stringstream tmp;
    switch(rel.getRelType()) {
        case R_386_32:
            relocation = rel.getDynSym()->getOffset() + addend;
            break;
        case R_386_PLT32: // this works because the address of the symbol is known at link time
        case R_386_PC32:
            relocation = rel.getDynSym()->getOffset() + addend - relOffset;
            break;
        case R_386_TLS_LE: // The necessary value is set during storage allocation
        case R_386_GLOB_DAT:
        case R_386_JMP_SLOT:
            relocation = rel.getDynSym()->getOffset();
            break;
        case R_386_GOT32:
            result = lmap.gotSymbols.find(rel.getDynSym());
            if( result == lmap.gotSymbols.end() ) {
                errMsg = "Expected GOT symbol does not exist GOT symbol mapping";
                return false;
            }

            relocation = result->second + addend - relOffset;
            break;
        case R_386_GOTOFF:
            relocation = rel.getDynSym()->getOffset() + addend - (lmap.gotRegionOffset + globalOffset);
            break;
        case R_386_GOTPC:
            relocation = globalOffset + lmap.gotRegionOffset + addend - relOffset;
            break;
        case R_386_TLS_IE:
            result = lmap.gotSymbols.find(rel.getDynSym());
            if( result == lmap.gotSymbols.end() ) {
                errMsg = "Expected GOT symbol does not exist GOT symbol mapping";
                return false;
            }

            relocation = result->second + lmap.gotRegionOffset + globalOffset;
            break;
        case R_386_TLS_GOTIE:
            result = lmap.gotSymbols.find(rel.getDynSym());
            if( result == lmap.gotSymbols.end() ) {
                errMsg = "Expected GOT symbol does not exist GOT symbol mapping";
                return false;
            }

            relocation = result->second;
            break;
        case R_386_COPY:
        case R_386_RELATIVE:
            tmp << "ERROR: encountered relocation type(" << rel.getRelType() << 
                   ") that is meant for use during dynamic linking";
            errMsg = tmp.str();
            return false;
        default:
            tmp << "Relocation type " << rel.getRelType() 
                << " currently unimplemented";
            errMsg = tmp.str();
            return false;
    }

    rewrite_printf("relocation = 0x%lx @ 0x%lx\n", relocation, relOffset);

    memcpy(&targetData[dest], &relocation, sizeof(Offset));

    return true;
}

Offset emitElf::layoutTLSImage(Offset globalOffset, Region *dataTLS, Region *bssTLS, LinkMap &lmap) {
    // x86 is Variant 1
    return tlsLayoutVariant1(globalOffset, dataTLS, bssTLS, lmap);
}

Offset emitElf::adjustTLSOffset(Offset curOffset, Offset tlsSize) {
    // x86 is Variant 1
    return tlsAdjustVariant1(curOffset, tlsSize);
}

char emitElf::getPaddingValue(Region::RegionType rtype) {
    const char X86_NOP = 0x90;

    char retChar = 0;
    if( rtype == Region::RT_TEXT || rtype == Region::RT_TEXTDATA ) {
        // This should be the value of a NOP (or equivalent instruction)
        retChar = X86_NOP;
    }

    return retChar;
}

void emitElf::cleanupTLSRegionOffsets(map<Region *, LinkMap::AllocPair> &regionAllocs,
        Region *dataTLS, Region *bssTLS) 
{
    tlsCleanupVariant1(regionAllocs, dataTLS, bssTLS);
}

void emitElf::getExcludedSymbolNames(set<string> &symNames) {
    /*
     * It appears that some .o's have a reference to _GLOBAL_OFFSET_TABLE_
     * This reference is only needed for dynamic linking.
     */
    symNames.insert("_GLOBAL_OFFSET_TABLE_");
}

bool emitElf::isGOTRelocation(unsigned long relType) {
    switch(relType) {
        case R_386_GOT32:
        case R_386_TLS_IE:
        case R_386_TLS_GOTIE:
            return true;
            break;
        default:
            return false;
            break;
    }
    return false;
}

Offset emitElf::getGOTSize(LinkMap &lmap) {
    Offset size = 0;
    // According to the ELF abi, entries 0, 1, 2 are reserved in a GOT on x86
    if( lmap.gotSymbols.size() > 0 ) {
        size = (lmap.gotSymbols.size()+GOT_RESERVED_SLOTS)*sizeof(Elf32_Addr);
    }
    return size;
}

Offset emitElf::getGOTAlign(LinkMap &) {
    return sizeof(Offset);
}

void emitElf::buildGOT(LinkMap &lmap) {
    char *targetData = lmap.allocatedData;

    // For each GOT symbol, allocate an entry and copy the value of the
    // symbol into the table, additionally store the offset in the GOT
    // back into the map
    Offset curOffset = GOT_RESERVED_SLOTS*sizeof(Offset);
    bzero(&targetData[lmap.gotRegionOffset], GOT_RESERVED_SLOTS*sizeof(Offset));

    map<Symbol *, Offset>::iterator sym_it;
    for(sym_it = lmap.gotSymbols.begin(); sym_it != lmap.gotSymbols.end(); ++sym_it) {
        Offset value = sym_it->first->getOffset();
        memcpy(&targetData[lmap.gotRegionOffset + curOffset], &value, sizeof(Offset));

        sym_it->second = curOffset;

        curOffset += sizeof(Offset);
    }
}
