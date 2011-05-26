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

/* 
 * holds architecture specific functions for x86 and x86_64 architecture needed for the
 * static executable rewriter
 */

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <set>
#include <map>
#include <sstream>

#include "emitElfStatic.h"
#include "Symtab.h"
#include "Symbol.h"
#include "Archive.h"
#include "Object.h"
#include "Region.h"
#include "debug.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

static const unsigned PPC32_WIDTH = 4;
static const unsigned PPC64_WIDTH = 8;

static const Elf64_Word X86_HEADER = 0xffffffff;
static const Elf64_Word X86_TRAILER = 0x00000000;
static const Elf64_Xword X86_64_HEADER = 0xffffffffffffffffULL;
static const Elf64_Xword X86_64_TRAILER = 0x0000000000000000ULL;

unsigned int setBits(unsigned int target, unsigned int pos, unsigned int len, unsigned int value) {
	rewrite_printf("setBits target 0x%lx value 0x%lx pos %d len %d \n", target, value, pos, len);
    unsigned int mask;
    mask = ~(~0 << len);
    value = value & mask;

    mask = ~(mask << pos);
    value = value << pos;

    target = target & mask;
    target = target | value;
	rewrite_printf( "setBits target 0x%lx value 0x%lx pos %d len %d \n", target, value, pos, len);
	return target;
}

#if defined(os_freebsd)
#define R_X86_64_JUMP_SLOT R_X86_64_JMP_SLOT
#endif

// Used in an assert so needs to be a macro
#define UNKNOWN_ADDRESS_WIDTH_ASSERT "An unknown address width was encountered, can't continue"

/* NOTE:
 * As most of these functions are defined per architecture, the description of
 * each of these functions is in the emitElfStatic header. Comments describing
 * the function interface are explicitly left out.
 */

/** 
 *
 * Given a relocation, determines if the relocation corresponds to a .ctors or .dtors
 * table that requires special consideration. Modifies the passed symbol offset to
 * point to the right table, if applicable.
 *
 * rel          The relocation entry to examine
 * globalOffset The offset of the linked code (used for symbol offset calculation)
 * lmap         Holds information about .ctors/.dtors tables
 * errMsg       Set on error
 * symbolOffset Modified by this routine to contain the offset of the table
 *
 * Returns true, if there are no errors including the case where the relocation 
 * entry doesn't reference the .ctors/.dtors tables.
 */

static bool computeCtorDtorAddress(relocationEntry &rel, Offset globalOffset,
        LinkMap &lmap, string &errMsg, Offset &symbolOffset)
{
    if( rel.name() ==  SYMTAB_CTOR_LIST_REL ) {
        // This needs to be: (the location of the .ctors table)
        if( lmap.newCtorRegions.size() > 0 ) {
            symbolOffset = lmap.ctorRegionOffset + globalOffset;
        }else if( lmap.originalCtorRegion != NULL ) {
            symbolOffset = lmap.originalCtorRegion->getRegionAddr();
        }else{
            errMsg = "Failed to locate original .ctors Region -- cannot apply relocation";
            rewrite_printf("Failed to locate original .ctors Region -- cannot apply relocation\n");
            return false;
        }
    }else if( rel.name() == SYMTAB_DTOR_LIST_REL ) {
        // This needs to be: (the location of the .dtors table)
        if( lmap.newDtorRegions.size() > 0 ) {
            symbolOffset = lmap.dtorRegionOffset + globalOffset;
        }else if( lmap.originalDtorRegion != NULL ) {
            symbolOffset = lmap.originalDtorRegion->getRegionAddr();
        }else{
            errMsg = "Failed to locate original .dtors Region -- cannot apply relocation";
            rewrite_printf("Failed to locate original .dtors Region -- cannot apply relocation\n");
            return false;
        }
    }

    return true;
}


bool emitElfStatic::archSpecificRelocation(char *targetData, relocationEntry &rel,
       Offset dest, Offset relOffset, Offset globalOffset, LinkMap &lmap,
       string &errMsg) 
{
	rewrite_printf(" archSpecificRelocation %s  \n",  rel.name().c_str());
    if( PPC32_WIDTH == addressWidth_ ) {
	int relocation_size = sizeof(Elf32_Word);
	int relocation_length = sizeof(Elf32_Word)*8; // in bits
	int relocation_pos = 0; // in bits
        /*
         * Referring to the SYSV 386 supplement:
         *
         * All relocations on x86 are one word32 == Elf32_Word
         *
         * S = symbolOffset
         * A = addend
         * P = relOffset
         */
       
        Offset symbolOffset = rel.getDynSym()->getOffset();

        Elf32_Word addend;
        if( rel.regionType() == Region::RT_REL ) {
            memcpy(&addend, &targetData[dest], sizeof(Elf32_Word));
        }else if( rel.regionType() == Region::RT_RELA ) {
            addend = rel.addend();
        }
        
	if(!computeCtorDtorAddress(rel, globalOffset, lmap, errMsg, symbolOffset)) {
		return false;
	}

        rewrite_printf("relocation for '%s': TYPE = %s(%lu) S = %lx A = %lx P = %lx\n",
                rel.name().c_str(), 
                relocationEntry::relType2Str(rel.getRelType(), addressWidth_),
                rel.getRelType(), symbolOffset, addend, relOffset);

        Offset relocation = 0;
        map<Symbol *, Offset>::iterator result;
        stringstream tmp;

        switch(rel.getRelType()) {
	    case R_PPC_ADDR16_HA:
		//relocation_size = 2;
		relocation_length = 16;
		relocation_pos = 0;
                relocation = symbolOffset + addend;
	        relocation = (((relocation >> 16) + ((relocation & 0x8000)? 1:0)) & 0xffff);
		break;
	    case R_PPC_ADDR16_LO:
		//relocation_size = 2;
		relocation_length = 16;
		relocation_pos = 0;
		relocation = symbolOffset + addend;
		relocation = (relocation & 0xffff);
		break;
	    case R_PPC_ADDR32:
                relocation = symbolOffset + addend;
		break;
            case R_PPC_GLOB_DAT:
                rewrite_printf("relocation R_PPC_GLOB_DAT for '%s': TYPE = %s(%lu) S = %lx A = %lx P = %lx\n",
                rel.name().c_str(), 
                relocationEntry::relType2Str(rel.getRelType(), addressWidth_),
                rel.getRelType(), symbolOffset, addend, relOffset);
 
                relocation = symbolOffset + addend;
		break;
            case R_PPC_GOT_TPREL16:
		//relocation_size = 2;
		relocation_length = 16;
		relocation_pos = 16;
                rewrite_printf("relocation R_PPC_GOT_TPREL16 for '%s': TYPE = %s(%lu) S = %lx A = %lx P = %lx\n",
                rel.name().c_str(), 
                relocationEntry::relType2Str(rel.getRelType(), addressWidth_),
                rel.getRelType(), symbolOffset, addend, relOffset);
                relocation = symbolOffset + addend;
		break;
	    case R_PPC_LOCAL24PC:
		relocation_length = 24;
		relocation_pos = 2;
                rewrite_printf("relocation R_PPC_LOCAL24PC for '%s': TYPE = %s(%lu) S = %lx A = %lx P = %lx\n",
                rel.name().c_str(), 
                relocationEntry::relType2Str(rel.getRelType(), addressWidth_),
                rel.getRelType(), symbolOffset, addend, relOffset);
                relocation = (symbolOffset + addend - relOffset) >> 2;
		break;
	
            case R_PPC_REL24:
		relocation_length = 24;
		relocation_pos = 2;
                relocation = (symbolOffset + addend - relOffset) >> 2;
		break;
            case R_PPC_REL32:
                relocation = symbolOffset + addend - relOffset;
		break;
            case R_PPC_TLS:
                rewrite_printf("relocation R_PPC_TLS for '%s': TYPE = %s(%lu) S = %lx A = %lx P = %lx\n",
                rel.name().c_str(), 
                relocationEntry::relType2Str(rel.getRelType(), addressWidth_),
                rel.getRelType(), symbolOffset, addend, relOffset);
 
                relocation = symbolOffset + addend;
		break;
/*
            case R_386_32:
                relocation = symbolOffset + addend;
                break;
            case R_386_PLT32: // this works because the address of the symbol is known at link time
            case R_386_PC32:
                relocation = symbolOffset + addend - relOffset;
                break;
            case R_386_TLS_LE: // The necessary value is set during storage allocation
            case R_386_GLOB_DAT:
            case R_386_JMP_SLOT:
                relocation = symbolOffset;
                break;
            case R_386_TLS_GD: // The address is computed when the GOT is built
            case R_386_GOT32: 
            case R_386_TLS_GOTIE:
                result = lmap.gotSymbols.find(rel.getDynSym());
                if( result == lmap.gotSymbols.end() ) {
                    errMsg = "Expected GOT symbol does not exist in GOT symbol mapping";
                    return false;
                }

                relocation = result->second;
                break;
            case R_386_GOTOFF:
                relocation = symbolOffset + addend - (lmap.gotRegionOffset + globalOffset);
                break;
            case R_386_GOTPC:
                relocation = globalOffset + lmap.gotRegionOffset + addend - relOffset;
                break;
            case R_386_TLS_IE:
                result = lmap.gotSymbols.find(rel.getDynSym());
                if( result == lmap.gotSymbols.end() ) {
                    errMsg = "Expected GOT symbol does not exist in GOT symbol mapping";
                    return false;
                }

                relocation = result->second + lmap.gotRegionOffset + globalOffset;
                break;
            case R_386_COPY:
            case R_386_RELATIVE:
                tmp << "ERROR: encountered relocation type(" << rel.getRelType() << 
                       ") that is meant for use during dynamic linking";
                errMsg = tmp.str();
                return false;
*/
            default:
                tmp << "Relocation type " << rel.getRelType() 
                    << " currently unimplemented";
                errMsg = tmp.str();
                return false;
        }

        rewrite_printf(" relocation = 0x%lx @ 0x%lx %d target data 0x%lx %lx %lx %lx \n", relocation, relOffset, relocation_size, targetData[dest], targetData[dest+1],  targetData[dest+2],  targetData[dest+3]);
	if (rel.getRelType() == R_PPC_REL24) {
	unsigned int *td = (unsigned int *) targetData;
	unsigned int target;
	target = td[dest/4];
	target = setBits(target, relocation_pos, relocation_length, relocation);
        memcpy(&targetData[dest], &target, relocation_size);
	} else {
	unsigned int *td = (unsigned int *) targetData;
	unsigned int target;
	target = td[dest/4];
	target = setBits(target, relocation_pos, relocation_length, relocation);
        memcpy(&td[dest/4], &target, relocation_size);
//        memcpy(&targetData[dest], r+2, relocation_size);
        rewrite_printf(" relocation = 0x%lx @ 0x%lx %d target data 0x%lx %lx %lx %lx \n", relocation, relOffset, relocation_size, targetData[dest], targetData[dest+1],  targetData[dest+2],  targetData[dest+3]);
	}
    } else{
        assert(!UNKNOWN_ADDRESS_WIDTH_ASSERT);
    }

    return true;
}

bool emitElfStatic::checkSpecialCaseSymbols(Symtab *, Symbol *) {
    return true;
}
char emitElfStatic::getPaddingValue(Region::RegionType rtype) {
    const char PPC32_NOP = 0x60000000;

    char retChar = 0;
    if( rtype == Region::RT_TEXT || rtype == Region::RT_TEXTDATA ) {
        retChar = PPC32_NOP;
    }

    return retChar;
}
/*
Offset emitElfStatic::layoutTLSImage(Offset, Region *, Region *, LinkMap &) {
    return 0;
}

Offset emitElfStatic::adjustTLSOffset(Offset, Offset) {
    return 0;
}

void emitElfStatic::cleanupTLSRegionOffsets(map<Region *, LinkMap::AllocPair> &,
        Region *, Region *)
{
}
*/


static const string CTOR_NAME(".ctors");
static const string DTOR_NAME(".dtors"); 
Offset emitElfStatic::layoutNewCtorRegion(LinkMap &lmap) {
    /* 
     * .ctors sections are processed in reverse order on Linux x86. New .ctors
     * sections need to be placed before the original .ctors section
     */

    Offset retOffset = lmap.ctorRegionOffset;
    retOffset += addressWidth_;

    pair<map<Region *, LinkMap::AllocPair>::iterator, bool> result;

    vector<Region *>::iterator reg_it;
    for(reg_it = lmap.newCtorRegions.begin(); reg_it != lmap.newCtorRegions.end(); ++reg_it) {
        result = lmap.regionAllocs.insert(make_pair(*reg_it, make_pair(0, retOffset)));

        // If the map already contains this Region, this is a logic error
        if( !result.second ) {
            return ~0UL;
        }

        retOffset += (*reg_it)->getRegionSize();
    }

    if( lmap.originalCtorRegion != NULL ) {
        // Account for original .ctors section (minus the header and trailer)
        retOffset += lmap.originalCtorRegion->getRegionSize() - addressWidth_ - addressWidth_;
    }
    retOffset += addressWidth_;

    return retOffset;

	return 0;
}

bool emitElfStatic::createNewCtorRegion(LinkMap &lmap) {
    char *targetData = lmap.allocatedData;

    if( PPC32_WIDTH != addressWidth_ && PPC64_WIDTH != addressWidth_ ) {
        assert(!UNKNOWN_ADDRESS_WIDTH_ASSERT);
    }

    unsigned trailerSize, headerSize;

    /* Give the new Region a header and trailer */
    Offset headerOffset = lmap.ctorRegionOffset;
    Offset trailerOffset;
    if( PPC32_WIDTH == addressWidth_ ) {
        memcpy(&targetData[headerOffset], &X86_HEADER, sizeof(X86_HEADER));
        trailerOffset = lmap.ctorRegionOffset + lmap.ctorSize - sizeof(X86_TRAILER);
        memcpy(&targetData[trailerOffset], &X86_TRAILER, sizeof(X86_TRAILER));
        headerSize = sizeof(X86_HEADER);
        trailerSize = sizeof(X86_TRAILER);
    }else{
        memcpy(&targetData[headerOffset], &X86_64_HEADER, sizeof(X86_64_HEADER));
        trailerOffset = lmap.ctorRegionOffset + lmap.ctorSize - sizeof(X86_64_TRAILER);
        memcpy(&targetData[trailerOffset], &X86_64_TRAILER, sizeof(X86_64_TRAILER));
        headerSize = sizeof(X86_64_HEADER);
        trailerSize = sizeof(X86_64_TRAILER);
    }

    if( lmap.originalCtorRegion != NULL ) {
        /* Determine where the original .ctors section should be placed */
        Offset originalOffset = lmap.ctorRegionOffset + lmap.ctorSize -
            trailerSize - (lmap.originalCtorRegion->getRegionSize() - headerSize - trailerSize);

        /* Copy the original .ctors section w/o the header and trailer */
        char *rawRegionData = reinterpret_cast<char *>(lmap.originalCtorRegion->getPtrToRawData());
        memcpy(&targetData[originalOffset], &rawRegionData[headerSize],
                lmap.originalCtorRegion->getRegionSize() - headerSize - trailerSize);
    }

    return true;
}


Offset emitElfStatic::layoutNewDtorRegion(LinkMap &lmap) {
    /*
     * .dtors sections are processed in forward order on Linux x86. So new
     * .dtors sections need to be placed after the original .dtors section
     */

    Offset retOffset = lmap.dtorRegionOffset;
    retOffset += addressWidth_;

    pair<map<Region *, LinkMap::AllocPair>::iterator, bool> result;
    if( lmap.originalDtorRegion != NULL ) {
        // Account for the original .dtors section (minus the header and trailer)
        retOffset += lmap.originalDtorRegion->getRegionSize() - addressWidth_ - addressWidth_;
    }

    vector<Region *>::iterator reg_it;
    for(reg_it = lmap.newDtorRegions.begin(); reg_it != lmap.newDtorRegions.end(); ++reg_it) {
        result = lmap.regionAllocs.insert(make_pair(*reg_it, make_pair(0, retOffset)));

        // If the map already contains this Region, this is a logic error
        if( !result.second ) {
            return ~0UL;
        }

        retOffset += (*reg_it)->getRegionSize();
    }

    retOffset += addressWidth_;
    return retOffset;

	return 0;
}

bool emitElfStatic::createNewDtorRegion(LinkMap &lmap) {
    char *targetData = lmap.allocatedData;

    if( PPC32_WIDTH != addressWidth_ && PPC64_WIDTH != addressWidth_ ) {
        assert(!UNKNOWN_ADDRESS_WIDTH_ASSERT);
    }

    unsigned headerSize, trailerSize;

    /* Give the new Region a header and trailer */
    Offset headerOffset = lmap.dtorRegionOffset;
    Offset trailerOffset;
    if( PPC32_WIDTH == addressWidth_ ) {
        memcpy(&targetData[headerOffset], &X86_HEADER, sizeof(X86_HEADER));
        trailerOffset = lmap.dtorRegionOffset + lmap.dtorSize - sizeof(X86_TRAILER);
        memcpy(&targetData[trailerOffset], &X86_TRAILER, sizeof(X86_TRAILER));
        headerSize = sizeof(X86_HEADER);
        trailerSize = sizeof(X86_TRAILER);
    }else{
        memcpy(&targetData[headerOffset], &X86_64_HEADER, sizeof(X86_64_HEADER));
        trailerOffset = lmap.dtorRegionOffset + lmap.dtorSize - sizeof(X86_64_TRAILER);
        memcpy(&targetData[trailerOffset], &X86_64_TRAILER, sizeof(X86_64_TRAILER));
        headerSize = sizeof(X86_64_HEADER);
        trailerSize = sizeof(X86_64_TRAILER);
    }

    if( lmap.originalDtorRegion != NULL ) {
        /* Determine where the original .dtors section should be placed */
        Offset originalOffset = lmap.dtorRegionOffset + headerSize;

        /* Copy the original .dtors section w/o header and trailer */
        char *rawRegionData = reinterpret_cast<char *>(lmap.originalDtorRegion->getPtrToRawData());
        memcpy(&targetData[originalOffset], &rawRegionData[headerSize],
                lmap.originalDtorRegion->getRegionSize() - headerSize - trailerSize);
    }

    return true;

}

bool emitElfStatic::isConstructorRegion(Region *reg) {
	return ( CTOR_NAME.compare(reg->getRegionName()) == 0 );

}

bool emitElfStatic::isDestructorRegion(Region *reg) {
	return ( DTOR_NAME.compare(reg->getRegionName()) == 0 );
}

bool emitElfStatic::isGOTRelocation(unsigned long relType) {
    if( PPC32_WIDTH == addressWidth_ ) {
        switch(relType) {
            case R_PPC_GOT16:
            case R_PPC_GOT16_LO:
            case R_PPC_GOT16_HI:
            case R_PPC_GOT16_HA:
            case R_PPC_GOT_TPREL16:
            case R_PPC_TLS:
                return true;
                break;
            default:
                return false;
                break;
        }
    } else{
        assert(!UNKNOWN_ADDRESS_WIDTH_ASSERT);
    }

    return false;
}

Offset emitElfStatic::getGOTSize(LinkMap &) {
    return 0;
}

Offset emitElfStatic::getGOTAlign(LinkMap &) {
    return 0;
}

void emitElfStatic::buildGOT(LinkMap &) {
}

void emitElfStatic::getExcludedSymbolNames(set<string> &) {
}

