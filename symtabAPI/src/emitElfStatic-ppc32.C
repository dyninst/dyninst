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
            symbolOffset = lmap.originalCtorRegion->getMemOffset();
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
            symbolOffset = lmap.originalDtorRegion->getMemOffset();
        }else{
            errMsg = "Failed to locate original .dtors Region -- cannot apply relocation";
            rewrite_printf("Failed to locate original .dtors Region -- cannot apply relocation\n");
            return false;
        }
    }

    return true;
}


bool emitElfStatic::archSpecificRelocation(Symtab *, Symtab *, char *targetData, relocationEntry &rel,
       Offset dest, Offset relOffset, Offset globalOffset, LinkMap &lmap,
       string &errMsg) 
{
	rewrite_printf(" archSpecificRelocation %s  \n",  rel.name().c_str());
    if( PPC32_WIDTH == addressWidth_ ) {
	int relocation_length = sizeof(Elf32_Word)*8; // in bits
	int relocation_pos = 0; // in bits
	int branch_pred = -1;
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

/* PowerPC relocations defined by the ABIs */
case R_PPC_NONE:/*            0 */
	break;
case R_PPC_ADDR32:/*          1        32bit absolute address */
	relocation = symbolOffset + addend;
	break;
case R_PPC_ADDR24:/*          2        26bit address, 2 bits ignored.  */
      	relocation_length = 26;
      	relocation_pos = 2;
	relocation = (symbolOffset + addend) >> 2;
	break;
case R_PPC_ADDR16:/*          3        16bit absolute address */
      	relocation_length = 16;
      	relocation_pos = 16;
	relocation = symbolOffset + addend;
	break;
case R_PPC_ADDR16_LO:/*       4        lower 16bit of absolute address */
      	relocation_length = 16;
      	relocation_pos = 0;
      	relocation = symbolOffset + addend;
	relocation = (relocation & 0xffff);
	break;
case R_PPC_ADDR16_HI:/*       5        high 16bit of absolute address */
	relocation_length = 16;
     	relocation_pos = 0;
      	relocation = symbolOffset + addend;
	relocation = ((relocation >> 16) & 0xffff);
	break;
case R_PPC_ADDR16_HA:/*       6        adjusted high 16bit */
	relocation_length = 16;
     	relocation_pos = 0;
        relocation = symbolOffset + addend;
        relocation = (((relocation >> 16) + ((relocation & 0x8000)? 1:0)) & 0xffff);
      	break;
case R_PPC_ADDR14:/*          7        16bit address, 2 bits ignored */
	relocation_length = 14;
     	relocation_pos = 16;
	relocation = (symbolOffset + addend) >> 2;
	break;
case R_PPC_ADDR14_BRTAKEN:/*  8 */
	relocation_length = 14;
     	relocation_pos = 16;
	relocation = (symbolOffset + addend) >> 2;
	// bit 10 is set
	branch_pred = 1;
	break;
case R_PPC_ADDR14_BRNTAKEN:/* 9 */
	relocation_length = 14;
     	relocation_pos = 16;
	relocation = (symbolOffset + addend) >> 2;
	// bit 10 is set
	branch_pred = 0;
	break;
case R_PPC_REL24:/*           10       PC relative 26 bit */
	relocation_length = 24;
      	relocation_pos = 2;
        relocation = (symbolOffset + addend - relOffset) >> 2;
      	break;
case R_PPC_REL14:/*           11       PC relative 16 bit */
	relocation_length = 14;
     	relocation_pos = 16;
        relocation = (symbolOffset + addend - relOffset) >> 2;
      	break;
case R_PPC_REL14_BRTAKEN:/*   12*/
	relocation_length = 14;
     	relocation_pos = 16;
        relocation = (symbolOffset + addend - relOffset) >> 2;
	branch_pred = 1;
      	break;
case R_PPC_REL14_BRNTAKEN:/*  13*/
	relocation_length = 14;
     	relocation_pos = 16;
        relocation = (symbolOffset + addend - relOffset) >> 2;
	branch_pred = 0;
      	break;
case R_PPC_GOT16:/*           14*/
        result = lmap.gotSymbols.find(rel.getDynSym());
        if( result == lmap.gotSymbols.end() ) {
            errMsg = "Expected GOT symbol does not exist in GOT symbol mapping";
            return false;
        }
        relocation = result->second;
	relocation = (relocation) >> 16;
      	relocation_length = 16;
      	relocation_pos = 16;
        break;
case R_PPC_GOT16_LO:/*        15*/
        result = lmap.gotSymbols.find(rel.getDynSym());
        if( result == lmap.gotSymbols.end() ) {
            errMsg = "Expected GOT symbol does not exist in GOT symbol mapping";
            return false;
        }
        relocation = result->second;
	relocation = (relocation & 0xffff);
      	relocation_length = 16;
      	relocation_pos = 0;
        break;
case R_PPC_GOT16_HI:/*        16*/
        result = lmap.gotSymbols.find(rel.getDynSym());
        if( result == lmap.gotSymbols.end() ) {
            errMsg = "Expected GOT symbol does not exist in GOT symbol mapping";
            return false;
        }
        relocation = result->second;
	relocation = ((relocation >> 16) & 0xffff);
      	relocation_length = 16;
      	relocation_pos = 0;
        break;
case R_PPC_GOT16_HA:/*        17*/
        result = lmap.gotSymbols.find(rel.getDynSym());
        if( result == lmap.gotSymbols.end() ) {
            errMsg = "Expected GOT symbol does not exist in GOT symbol mapping";
            return false;
        }
        relocation = result->second;
        relocation = (((relocation >> 16) + ((relocation & 0x8000)? 1:0)) & 0xffff);
      	relocation_length = 16;
      	relocation_pos = 0;
        break;
case R_PPC_PLTREL24:/*        18*/
	relocation_length = 24;
      	relocation_pos = 2;
	relocation = (symbolOffset + addend - relOffset) >> 2;
	break;
case R_PPC_COPY:/*            19*/
	break;
case R_PPC_GLOB_DAT:/*        20*/
	relocation = symbolOffset + addend;
	break;
case R_PPC_JMP_SLOT:/*        21*/
	break;
case R_PPC_RELATIVE:/*        22*/
       	tmp << "ERROR: encountered relocation type(" << rel.getRelType() << 
                ") that is meant for use during dynamic linking";
        errMsg = tmp.str();
        return false;
case R_PPC_LOCAL24PC:/*       23*/
	relocation_length = 24;
      	relocation_pos = 2;
	relocation = (symbolOffset + addend - relOffset) >> 2;
	break;
case R_PPC_UADDR32:/*         24*/
	relocation = symbolOffset + addend ;
	break;
case R_PPC_UADDR16:/*         25*/
      	relocation_length = 16;
      	relocation_pos = 16;
	relocation = symbolOffset + addend;
	break;
case R_PPC_REL32:/*           26*/
	relocation = symbolOffset + addend - relOffset;
        break;
case R_PPC_PLT32:/*           27*/
	relocation = symbolOffset + addend;
        break;
case R_PPC_PLTREL32:/*        28*/
	relocation = symbolOffset + addend - relOffset;
        break;
case R_PPC_PLT16_LO:/*        29*/
	relocation = symbolOffset + addend;
	relocation = (relocation & 0xffff);
        break;
case R_PPC_PLT16_HI:/*        30*/
	relocation = symbolOffset + addend;
	relocation = ((relocation >> 16) & 0xffff);
        break;
case R_PPC_PLT16_HA:/*        31*/
	relocation = symbolOffset + addend;
        relocation = (((relocation >> 16) + ((relocation & 0x8000)? 1:0)) & 0xffff);
        break;
case R_PPC_SDAREL16:/*        32*/
case R_PPC_SECTOFF:/*         33*/
case R_PPC_SECTOFF_LO:/*      34*/
case R_PPC_SECTOFF_HI:/*      35*/
case R_PPC_SECTOFF_HA:/*      36*/
        tmp << "Relocation type " << rel.getRelType() 
            << " currently unimplemented";
        errMsg = tmp.str();
      rewrite_printf(" Relocation type %s  currently unimplemented \n", relocationEntry::relType2Str(rel.getRelType(), addressWidth_));
        return false;

/* PowerPC relocations defined for the TLS access ABI.  */
case R_PPC_TLS:/*             67  none      (sym+add)@tls */
case R_PPC_DTPMOD32:/*        68  word32    (sym+add)@dtpmod */
case R_PPC_TPREL16:/*         69  half16*   (sym+add)@tprel */
case R_PPC_TPREL16_LO:/*      70  half16    (sym+add)@tprel@l */
case R_PPC_TPREL16_HI:/*      71  half16    (sym+add)@tprel@h */
case R_PPC_TPREL16_HA:/*      72  half16    (sym+add)@tprel@ha */
case R_PPC_TPREL32:/*         73  word32    (sym+add)@tprel */
case R_PPC_DTPREL16:/*        74  half16*   (sym+add)@dtprel */
case R_PPC_DTPREL16_LO:/*     75  half16    (sym+add)@dtprel@l */
case R_PPC_DTPREL16_HI:/*     76  half16    (sym+add)@dtprel@h */
case R_PPC_DTPREL16_HA:/*     77  half16    (sym+add)@dtprel@ha */
case R_PPC_DTPREL32:/*        78  word32    (sym+add)@dtprel */
case R_PPC_GOT_TLSGD16:/*     79  half16*   (sym+add)@got@tlsgd */
case R_PPC_GOT_TLSGD16_LO:/*  80  half16    (sym+add)@got@tlsgd@l */
case R_PPC_GOT_TLSGD16_HI:/*  81  half16    (sym+add)@got@tlsgd@h */
case R_PPC_GOT_TLSGD16_HA:/*  82  half16    (sym+add)@got@tlsgd@ha */
case R_PPC_GOT_TLSLD16:/*     83  half16*   (sym+add)@got@tlsld */
case R_PPC_GOT_TLSLD16_LO:/*  84  half16    (sym+add)@got@tlsld@l */
case R_PPC_GOT_TLSLD16_HI:/*  85  half16    (sym+add)@got@tlsld@h */
case R_PPC_GOT_TLSLD16_HA:/*  86  half16    (sym+add)@got@tlsld@ha */
case R_PPC_GOT_TPREL16:/*     87  half16*   (sym+add)@got@tprel */
case R_PPC_GOT_TPREL16_LO:/*  88  half16    (sym+add)@got@tprel@l */
case R_PPC_GOT_TPREL16_HI:/*  89  half16    (sym+add)@got@tprel@h */
case R_PPC_GOT_TPREL16_HA:/*  90  half16    (sym+add)@got@tprel@ha */
case R_PPC_GOT_DTPREL16:/*    91  half16*   (sym+add)@got@dtprel */
case R_PPC_GOT_DTPREL16_LO:/* 92  half16*   (sym+add)@got@dtprel@l */
case R_PPC_GOT_DTPREL16_HI:/* 93  half16*   (sym+add)@got@dtprel@h */
case R_PPC_GOT_DTPREL16_HA:/* 94  half16*   (sym+add)@got@dtprel@ha */
      relocation_length = 16;
      relocation_pos = 16;
      relocation = symbolOffset + addend;
      rewrite_printf(" Relocation type %s  currently unimplemented \n", relocationEntry::relType2Str(rel.getRelType(), addressWidth_));
      break;

/* GNU relocs used in PIC code sequences.  */
/* NOTE: The following relocations are not defined in some elf.h
   Hence, using numbers instead of name */
case 249: /*R_PPC_REL16:           249      word32   (sym-.) */
	relocation_length = 16;
     	relocation_pos = 16;
        relocation = symbolOffset + addend - relOffset ;
      	break;
case 250: /*R_PPC_REL16_LO:        250      half16   (sym-.)@l */
	relocation_length = 16;
     	relocation_pos = 16;
        relocation = symbolOffset + addend - relOffset ;
	relocation = (relocation & 0xffff);
      	break;
case 251: /*R_PPC_REL16_HI:        251      half16   (sym-.)@h */
	relocation_length = 16;
     	relocation_pos = 16;
        relocation = symbolOffset + addend - relOffset ;
	relocation = ((relocation >> 16) & 0xffff);
      	break;
case 252: /*R_PPC_REL16_HA:        252      half16   (sym-.)@ha */
	relocation_length = 16;
     	relocation_pos = 16;
        relocation = symbolOffset + addend - relOffset ;
        relocation = (((relocation >> 16) + ((relocation & 0x8000)? 1:0)) & 0xffff);
      	break;
/* This is a phony reloc to handle any old fashioned TOC16 references
   that may still be in object files.  */
case 255: /*R_PPC_TOC16:              255*/
      	break;

default:
     tmp << "Relocation type " << rel.getRelType() 
         << " currently unimplemented";
     rewrite_printf(" Relocation type %s  currently unimplemented \n", relocationEntry::relType2Str(rel.getRelType(), addressWidth_));
     errMsg = tmp.str();
     return false;
        }

        rewrite_printf(" relocation = 0x%lx @ 0x%lx target data 0x%lx %lx %lx %lx \n", relocation, relOffset, targetData[dest], targetData[dest+1],  targetData[dest+2],  targetData[dest+3]);
	if (rel.getRelType() == R_PPC_REL24) {
	unsigned int *td = (unsigned int *) targetData;
	unsigned int target;
	target = td[dest/4];
	target = setBits(target, relocation_pos, relocation_length, relocation);
        memcpy(&targetData[dest], &target, sizeof(Elf32_Word));
	} else {
	unsigned int *td = (unsigned int *) targetData;
	unsigned int target;
	target = td[dest/4];
	target = setBits(target, relocation_pos, relocation_length, relocation);
        memcpy(&td[dest/4], &target, sizeof(Elf32_Word));
//        memcpy(&targetData[dest], r+2, relocation_size);
        rewrite_printf(" relocation = 0x%lx @ 0x%lx target data 0x%lx %lx %lx %lx \n", relocation, relOffset, targetData[dest], targetData[dest+1],  targetData[dest+2],  targetData[dest+3]);
	}
    if (branch_pred >= 0) {
	unsigned int *td = (unsigned int *) targetData;
	unsigned int target;
	target = td[dest/4];
	target = setBits(target, 10, 1, branch_pred);
        memcpy(&td[dest/4], &target, sizeof(Elf32_Word));
    } 

    } else{
        assert(!UNKNOWN_ADDRESS_WIDTH_ASSERT);
    }
    return true;
}

bool emitElfStatic::checkSpecialCaseSymbols(Symtab *, Symbol *) {
    return true;
}

/* The TLS implementation on ppc is Variant 1 */

Offset emitElfStatic::layoutTLSImage(Offset globalOffset, Region *dataTLS, Region *bssTLS, LinkMap &lmap) {
    return tlsLayoutVariant1(globalOffset, dataTLS, bssTLS, lmap);
}

Offset emitElfStatic::adjustTLSOffset(Offset curOffset, Offset tlsSize) {
    return curOffset;
}

char emitElfStatic::getPaddingValue(Region::RegionType rtype) {
   // TODO: if this matters, we can noop-pad by returning an unsigned
   // instead of a char...
   return 0x0;
}

void emitElfStatic::cleanupTLSRegionOffsets(map<Region *, LinkMap::AllocPair> &regionAllocs,
        Region *dataTLS, Region *bssTLS) 
{
    tlsCleanupVariant2(regionAllocs, dataTLS, bssTLS);
}

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

    for(auto reg_it = lmap.newCtorRegions.begin(); reg_it != lmap.newCtorRegions.end(); ++reg_it) {
        result = lmap.regionAllocs.insert(make_pair(*reg_it, make_pair(0, retOffset)));

        // If the map already contains this Region, this is a logic error
        if( !result.second ) {
            return ~0UL;
        }

        retOffset += (*reg_it)->getDiskSize();
    }

    if( lmap.originalCtorRegion != NULL ) {
        // Account for original .ctors section (minus the header and trailer)
        retOffset += lmap.originalCtorRegion->getDiskSize() - addressWidth_ - addressWidth_;
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
            trailerSize - (lmap.originalCtorRegion->getDiskSize() - headerSize - trailerSize);

        /* Copy the original .ctors section w/o the header and trailer */
        char *rawRegionData = reinterpret_cast<char *>(lmap.originalCtorRegion->getPtrToRawData());
        memcpy(&targetData[originalOffset], &rawRegionData[headerSize],
                lmap.originalCtorRegion->getDiskSize() - headerSize - trailerSize);
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
        retOffset += lmap.originalDtorRegion->getDiskSize() - addressWidth_ - addressWidth_;
    }

    for(auto reg_it = lmap.newDtorRegions.begin(); reg_it != lmap.newDtorRegions.end(); ++reg_it) {
        result = lmap.regionAllocs.insert(make_pair(*reg_it, make_pair(0, retOffset)));

        // If the map already contains this Region, this is a logic error
        if( !result.second ) {
            return ~0UL;
        }

        retOffset += (*reg_it)->getDiskSize();
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
                lmap.originalDtorRegion->getDiskSize() - headerSize - trailerSize);
    }

    return true;

}

bool emitElfStatic::isConstructorRegion(Region *reg) {
	return ( CTOR_NAME.compare(reg->getRegionName()) == 0 );

}

bool emitElfStatic::isDestructorRegion(Region *reg) {
	return ( DTOR_NAME.compare(reg->getRegionName()) == 0 );
}

bool emitElfStatic::isGOTRegion(Region *) {
	return false;
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

Offset emitElfStatic::getGOTSize(Symtab *, LinkMap &, Offset &) {
    return 0;
}

Offset emitElfStatic::getGOTAlign(LinkMap &) {
    return 0;
}

void emitElfStatic::buildGOT(Symtab *, LinkMap &) {
}

void emitElfStatic::getExcludedSymbolNames(set<string> &) {
}

Offset emitElfStatic::allocStubRegions(LinkMap &lmap, Offset) {
   // Size 0
   return lmap.stubRegionOffset;
}

bool emitElfStatic::updateTOC(Symtab *, LinkMap &, Offset) {
  return true;
}
