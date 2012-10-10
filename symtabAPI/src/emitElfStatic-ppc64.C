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

static const Offset GOT_RESERVED_SLOTS = 0;

unsigned int setBits(unsigned int target, unsigned int pos, unsigned int len, unsigned int value) {
    rewrite_printf("setBits target 0x%lx value 0x%lx pos %d len %d \n", target, value, pos, len);
// There are three parts of the target - 0:pos, pos:len, len:32 
// We want to create a mask with 0:pos and len:32 set to 1
    unsigned int mask, mask1, mask2;
    mask1 = ~(~0 << pos);
    mask1 = (mask1 << (32-pos));
    mask2 = ~(~0 << (32-(pos+len)));
    mask = mask1 | mask2;
    target = target & mask;
    rewrite_printf(" mask1 0x%lx mask2 0x%lx mask 0x%lx target 0x%lx \n", mask1, mask2, mask, target);

    if(len != 32)
    	mask = ~mask;

    value = value & mask;
    rewrite_printf(" mask 0x%lx value 0x%lx \n", mask, value);

    target = target | value;
    rewrite_printf( "setBits target 0x%lx value 0x%lx pos %d len %d \n", target, value, pos, len);

    return target;
}

unsigned long setBits64(unsigned long target, unsigned int pos, unsigned int len, unsigned long value) {
    unsigned  long mask;
    mask = ~(~0 << len);
    mask = (mask << pos);
    target = target & mask;
        if(len != 32)
    mask = ~mask;
    value = value & mask;

    target = target | value;
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
	    rewrite_printf("new CTOR computeCtorDtorAddress symbolOffset 0x%lx \n", symbolOffset);
        }else if( lmap.originalCtorRegion != NULL ) {
            symbolOffset = lmap.originalCtorRegion->getRegionAddr();
	    rewrite_printf("original CTOR computeCtorDtorAddress symbolOffset 0x%lx \n", symbolOffset);
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


bool emitElfStatic::archSpecificRelocation(Symtab* targetSymtab, Symtab* srcSymtab, char *targetData, relocationEntry &rel,
       Offset dest, Offset relOffset, Offset globalOffset, LinkMap &lmap,
       string &errMsg) 
{

    if( PPC64_WIDTH == addressWidth_ ) {

	Symbol *dynsym = rel.getDynSym();
	Offset TOCoffset = targetSymtab->getTOCoffset();

	rewrite_printf(" archSpecificRelocation %s dynsym %s address 0x%lx TOC 0x%lx dest %d \n", 
		 rel.name().c_str(), dynsym->getName().c_str(), relOffset, TOCoffset, dest );

	int relocation_length = sizeof(Elf64_Word)*8; // in bits
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

        Elf64_Word addend;
        if( rel.regionType() == Region::RT_REL ) {
            memcpy(&addend, &targetData[dest], sizeof(Elf64_Word));
        }else if( rel.regionType() == Region::RT_RELA ) {
            addend = rel.addend();
        }
        
	if(!computeCtorDtorAddress(rel, globalOffset, lmap, errMsg, symbolOffset)) {
		return false;
	}
	
//	printf("Original symbol offset 0x%lx \n", symbolOffset);
	// If symbol is .toc, we must return the got offset
    	if(rel.getRelType() == R_PPC64_TOC16_DS || rel.getRelType() == R_PPC64_TOC16) {
		vector<Region *> allRegions;
        	srcSymtab->getAllRegions(allRegions);
		vector<Region *>::iterator region_it;
        	for(region_it = allRegions.begin(); region_it != allRegions.end(); ++region_it) {
//			printf("srcSymtab %s regions %s \n", srcSymtab->name().c_str(), (*region_it)->getRegionName().c_str());
			if((*region_it)->getRegionName().compare(".toc") == 0) {
				map<Region *, LinkMap::AllocPair>::iterator result = lmap.regionAllocs.find(*region_it);
		            	if( result != lmap.regionAllocs.end() ) {
		                	Offset regionOffset = result->second.second;
					symbolOffset = globalOffset + regionOffset;
//					printf(" regionOffset 0x%lx symbolOffset 0x%lx \n", regionOffset, symbolOffset);
			    	}
			}
		}

    	}

        rewrite_printf("relocation for '%s': TYPE = %s(%lu) S = %lx A = %lx P = %lx Total 0x%lx \n",
                rel.name().c_str(), 
                relocationEntry::relType2Str(rel.getRelType(), addressWidth_),
                rel.getRelType(), symbolOffset, addend, relOffset, symbolOffset+addend);


        Offset relocation = 0;
        map<Symbol *, Offset>::iterator result;
        stringstream tmp;

        switch(rel.getRelType()) {
/* PowerPC64 relocations defined by the ABIs */
case R_PPC64_ADDR64:
case R_PPC64_GLOB_DAT:
	relocation_length = 64;
	relocation = symbolOffset + addend;
	break;
case R_PPC64_GOT_TPREL16_DS:
case R_PPC64_REL24:
	relocation_length = 24;
      	relocation_pos = 6;
        //relocation = (symbolOffset + addend - relOffset)>> 2;
        relocation = symbolOffset + addend - relOffset;
	rewrite_printf(" R_PPC64_REL24 S = 0x%lx A = %d relOffset = 0x%lx relocation without shift 0x%lx %ld \n", 
		symbolOffset, addend, relOffset, symbolOffset + addend - relOffset, symbolOffset + addend - relOffset);
      	break;
case R_PPC64_REL32:
	relocation_length = 24;
      	relocation_pos = 6;
        relocation = symbolOffset + addend - relOffset;
      	break;
case R_PPC64_REL64:
        relocation = symbolOffset + addend - relOffset;
      	break;
case R_PPC64_TLS:
	break;
case R_PPC64_TOC16:
	relocation_length = 16;
      	relocation_pos = 0;
        relocation = symbolOffset + addend - TOCoffset;
      	break;
case R_PPC64_TOC16_DS:
	relocation_length = 16;
      	relocation_pos = 16;
        relocation = (symbolOffset + addend - TOCoffset) >> 2 ;
        relocation = relocation << 2;
	break;
}
        rewrite_printf("before: relocation = 0x%lx @ 0x%lx target data %lx %lx %lx %lx %lx %lx \n", 
	relocation, relOffset,targetData[dest-2],  targetData[dest-1], targetData[dest], targetData[dest+1],  targetData[dest+2],  targetData[dest+3]);

	if (rel.getRelType() == R_PPC64_ADDR64 || rel.getRelType() == R_PPC64_GLOB_DAT) {
		char *td = (targetData + dest - (dest%8));
	        unsigned long target = *((unsigned long *) td);
	        target = setBits64(target, relocation_pos, relocation_length, relocation);
        	memcpy(td, &target, 2*sizeof(Elf64_Word));
	} else {
		char *td = (targetData + dest - (dest%4));
	        unsigned int target = *((unsigned int *) td);
	        target = setBits(target, relocation_pos, relocation_length, relocation);
        	memcpy(td, &target, sizeof(Elf64_Word));
	}

        rewrite_printf("after: relocation = 0x%lx @ 0x%lx target data %lx %lx %lx %lx %lx %lx \n", relocation, relOffset,targetData[dest-2],  targetData[dest-1], targetData[dest], targetData[dest+1],  targetData[dest+2],  targetData[dest+3]);

/*
    if (branch_pred >= 0) {
	unsigned int *td = (unsigned int *) targetData;
	unsigned int target;
	target = td[dest/4];
	target = setBits(target, 10, 1, branch_pred);
        memcpy(&td[dest/4], &target, sizeof(Elf64_Word));
    } 
*/

#if 0
switch(0){
case R_PPC64_NONE           :/* R_PPC_NONE */
case R_PPC64_ADDR32         :/* R_PPC_ADDR32  32bit absolute address */
case R_PPC64_ADDR24         :/* R_PPC_ADDR24  26bit address, word aligned */
case R_PPC64_ADDR16         :/* R_PPC_ADDR16  16bit absolute address */
case R_PPC64_ADDR16_LO      :/* R_PPC_ADDR16_LO  lower 16bits of address */
case R_PPC64_ADDR16_HI      :/* R_PPC_ADDR16_HI  high 16bits of address. */
case R_PPC64_ADDR16_HA      :/* R_PPC_ADDR16_HA  adjusted high 16bits.  */
case R_PPC64_ADDR14         :/* R_PPC_ADDR14  16bit address, word aligned */
case R_PPC64_ADDR14_BRTAKEN :/* R_PPC_ADDR14_BRTAKEN */
case R_PPC64_ADDR14_BRNTAKEN:/* R_PPC_ADDR14_BRNTAKEN */
case R_PPC64_REL24          :/* R_PPC_REL24 PC-rel. 26 bit, word aligned */
case R_PPC64_REL14          :/* R_PPC_REL14  PC relative 16 bit */
case R_PPC64_REL14_BRTAKEN  :/* R_PPC_REL14_BRTAKEN */
case R_PPC64_REL14_BRNTAKEN :/* R_PPC_REL14_BRNTAKEN */
case R_PPC64_GOT16          :/* R_PPC_GOT16 */
case R_PPC64_GOT16_LO       :/* R_PPC_GOT16_LO */
case R_PPC64_GOT16_HI       :/* R_PPC_GOT16_HI */
case R_PPC64_GOT16_HA       :/* R_PPC_GOT16_HA*/
case R_PPC64_COPY           :/* R_PPC_COPY*/
case R_PPC64_GLOB_DAT       :/* R_PPC_GLOB_DAT*/
case R_PPC64_JMP_SLOT       :/* R_PPC_JMP_SLOT*/
case R_PPC64_RELATIVE       :/* R_PPC_RELATIVE*/
case R_PPC64_UADDR32        :/* R_PPC_UADDR32*/
case R_PPC64_UADDR16        :/* R_PPC_UADDR16*/
case R_PPC64_REL32          :/* R_PPC_REL32*/
case R_PPC64_PLT32          :/* R_PPC_PLT32*/
case R_PPC64_PLTREL32       :/* R_PPC_PLTREL32*/
case R_PPC64_PLT16_LO       :/* R_PPC_PLT16_LO*/
case R_PPC64_PLT16_HI       :/* R_PPC_PLT16_HI*/
case R_PPC64_PLT16_HA       :/* R_PPC_PLT16_HA*/
case R_PPC64_SECTOFF        :/* R_PPC_SECTOFF*/
case R_PPC64_SECTOFF_LO     :/* R_PPC_SECTOFF_LO*/
case R_PPC64_SECTOFF_HI     :/* R_PPC_SECTOFF_HI*/
case R_PPC64_SECTOFF_HA     :/* R_PPC_SECTOFF_HA*/
case R_PPC64_ADDR30         : /* 37  word30 (S + A - P) >> 2 */
case R_PPC64_ADDR64         : /* 38 doubleword64 S + A */
case R_PPC64_ADDR16_HIGHER  : /* 39 half16 #higher(S + A) */
case R_PPC64_ADDR16_HIGHERA :/* 40 half16 #highera(S + A) */
case R_PPC64_ADDR16_HIGHEST :/*41 half16 #highest(S + A) */
case R_PPC64_ADDR16_HIGHESTA:/*42  half16 #highesta(S + A) */
case R_PPC64_UADDR64        :/*43  doubleword64 S + A */
case R_PPC64_REL64          :/*44  doubleword64 S + A - P */
case R_PPC64_PLT64          :/*45  doubleword64 L + A */
case R_PPC64_PLTREL64       :/*46  doubleword64 L + A - P */
case R_PPC64_TOC16          :/*47  half16* S + A - .TOC */
case R_PPC64_TOC16_LO       :/*48  half16 #lo(S + A - .TOC.) */
case R_PPC64_TOC16_HI       :/*49  half16 #hi(S + A - .TOC.) */
case R_PPC64_TOC16_HA       :/*50  half16 #ha(S + A - .TOC.) */
case R_PPC64_TOC            :/*51  doubleword64 .TOC */
case R_PPC64_PLTGOT16       :/*52  half16* M + A */
case R_PPC64_PLTGOT16_LO    :/*53  half16 #lo(M + A) */
case R_PPC64_PLTGOT16_HI    :/*54  half16 #hi(M + A) */
case R_PPC64_PLTGOT16_HA    :/*55  half16 #ha(M + A) */

case R_PPC64_ADDR16_DS      :/*56  half16ds* (S + A) >> 2 */
case R_PPC64_ADDR16_LO_DS   :/*57  half16ds  #lo(S + A) >> 2 */
case R_PPC64_GOT16_DS       :/*58  half16ds* (G + A) >> 2 */
case R_PPC64_GOT16_LO_DS    :/*59  half16ds  #lo(G + A) >> 2 */
case R_PPC64_PLT16_LO_DS    :/*60  half16ds  #lo(L + A) >> 2 */
case R_PPC64_SECTOFF_DS     :/*61  half16ds* (R + A) >> 2 */
case R_PPC64_SECTOFF_LO_DS  :/*62  half16ds  #lo(R + A) >> 2 */
case R_PPC64_TOC16_DS       :/*63  half16ds* (S + A - .TOC.) >> 2 */
case R_PPC64_TOC16_LO_DS    :/*64  half16ds  #lo(S + A - .TOC.) >> 2 */
case R_PPC64_PLTGOT16_DS    :/*65  half16ds* (M + A) >> 2 */
case R_PPC64_PLTGOT16_LO_DS :/*66  half16ds  #lo(M + A) >> 2 */

/* PowerPC64 relocations defined for the TLS access ABI.  */
case R_PPC64_TLS            :/*67  none      (sym+add)@tls */
case R_PPC64_DTPMOD64       :/*68  doubleword64 (sym+add)@dtpmod */
case R_PPC64_TPREL16        :/*69  half16*   (sym+add)@tprel */
case R_PPC64_TPREL16_LO     :/*70  half16    (sym+add)@tprel@l */
case R_PPC64_TPREL16_HI     :/*71  half16    (sym+add)@tprel@h */
case R_PPC64_TPREL16_HA     :/*72  half16    (sym+add)@tprel@ha */
case R_PPC64_TPREL64        :/*73  doubleword64 (sym+add)@tprel */
case R_PPC64_DTPREL16       :/*74  half16*   (sym+add)@dtprel */
case R_PPC64_DTPREL16_LO    :/*75  half16    (sym+add)@dtprel@l */
case R_PPC64_DTPREL16_HI    :/*76  half16    (sym+add)@dtprel@h */
case R_PPC64_DTPREL16_HA    :/*77  half16    (sym+add)@dtprel@ha */
case R_PPC64_DTPREL64       :/*78  doubleword64 (sym+add)@dtprel */
case R_PPC64_GOT_TLSGD16    :/*79  half16*   (sym+add)@got@tlsgd */
case R_PPC64_GOT_TLSGD16_LO :/*80  half16    (sym+add)@got@tlsgd@l */
case R_PPC64_GOT_TLSGD16_HI :/*81  half16    (sym+add)@got@tlsgd@h */
case R_PPC64_GOT_TLSGD16_HA :/*82  half16    (sym+add)@got@tlsgd@ha */
case R_PPC64_GOT_TLSLD16    :/*83  half16*   (sym+add)@got@tlsld */
case R_PPC64_GOT_TLSLD16_LO :/*84  half16    (sym+add)@got@tlsld@l */
case R_PPC64_GOT_TLSLD16_HI :/*85  half16    (sym+add)@got@tlsld@h */
case R_PPC64_GOT_TLSLD16_HA :/*86  half16    (sym+add)@got@tlsld@ha */
case R_PPC64_GOT_TPREL16_DS :/*87  half16ds* (sym+add)@got@tprel */
case R_PPC64_GOT_TPREL16_LO_DS:/*88  half16ds (sym+add)@got@tprel@l */
case R_PPC64_GOT_TPREL16_HI :/*89  half16    (sym+add)@got@tprel@h */
case R_PPC64_GOT_TPREL16_HA :/*90  half16    (sym+add)@got@tprel@ha */
case R_PPC64_GOT_DTPREL16_DS:/*91  half16ds* (sym+add)@got@dtprel */
case R_PPC64_GOT_DTPREL16_LO_DS:/*92  half16ds (sym+add)@got@dtprel@l */
case R_PPC64_GOT_DTPREL16_HI:/*93  half16    (sym+add)@got@dtprel@h */
case R_PPC64_GOT_DTPREL16_HA:/*94  half16    (sym+add)@got@dtprel@ha */
case R_PPC64_TPREL16_DS     :/*95  half16ds* (sym+add)@tprel */
case R_PPC64_TPREL16_LO_DS  :/*96  half16ds  (sym+add)@tprel@l */
case R_PPC64_TPREL16_HIGHER :/*97  half16    (sym+add)@tprel@higher */
case R_PPC64_TPREL16_HIGHERA:/*98  half16    (sym+add)@tprel@highera */
case R_PPC64_TPREL16_HIGHEST:/*99  half16    (sym+add)@tprel@highest */
case R_PPC64_TPREL16_HIGHESTA:/*100  half16  (sym+add)@tprel@highesta */
case R_PPC64_DTPREL16_DS    :/*101  half16ds* (sym+add)@dtprel */
case R_PPC64_DTPREL16_LO_DS :/*102  half16ds (sym+add)@dtprel@l */
case R_PPC64_DTPREL16_HIGHER:/*103  half16   (sym+add)@dtprel@higher */
case R_PPC64_DTPREL16_HIGHERA:/*104  half16  (sym+add)@dtprel@highera */
case R_PPC64_DTPREL16_HIGHEST:/*105  half16  (sym+add)@dtprel@highest */
case R_PPC64_DTPREL16_HIGHESTA:/*106  half16 (sym+add)@dtprel@highesta */

/* Keep this the last entry.  */
case R_PPC64_NUM            :/*107 */

		default:
			break;
	}
#endif
    } else if (PPC32_WIDTH == addressWidth_ ){
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
   // Can't noop-pad because this returns a char, not an unsigned
   return 0x0;
}

void emitElfStatic::cleanupTLSRegionOffsets(map<Region *, LinkMap::AllocPair> &regionAllocs,
                                            Region *dataTLS, Region *bssTLS) 
{
    tlsCleanupVariant2(regionAllocs, dataTLS, bssTLS);
}

Offset emitElfStatic::getGOTSize(Symtab *target, LinkMap &lmap) {
    Offset size = 0;

    unsigned slotSize = 0;
    if( PPC32_WIDTH == addressWidth_ ) {
        slotSize = sizeof(Elf32_Addr);
    }else if( PPC64_WIDTH == addressWidth_ ) {
        slotSize = sizeof(Elf64_Addr)*2;
    }else{
        assert(!UNKNOWN_ADDRESS_WIDTH_ASSERT);
    }

    // According to the ELF abi, entries 0, 1, 2 are reserved in a GOT on x86
    if( lmap.gotSymbolTable.size() > 0 ) {
       rewrite_printf("Determining new GOT size: %d symtab entries, %d reserved slots, %d slot size\n",
                      lmap.gotSymbolTable.size(), GOT_RESERVED_SLOTS, slotSize);
        size = (lmap.gotSymbolTable.size()+GOT_RESERVED_SLOTS)*slotSize;
    }

    // On PPC64 we move the original GOT as well, as we can't guarantee that we can reach
    // the new GOT from the old. 
    if (PPC64_WIDTH == addressWidth_) {
       rewrite_printf("New GOT size is 0x%lx, adding original GOT size 0x%lx\n", size, target->getObject()->gotSize());
       size += target->getObject()->gotSize();
    }

    return size;
}

Offset emitElfStatic::getGOTAlign(LinkMap &) {

    if( PPC32_WIDTH == addressWidth_ ) {
	return 0;
    }
    if( PPC32_WIDTH == addressWidth_ ) {
        return sizeof(Elf32_Word);
    }else if( PPC64_WIDTH == addressWidth_ ) {
        return sizeof(Elf64_Xword);
    }else{
        assert(!UNKNOWN_ADDRESS_WIDTH_ASSERT);
    }

    return 0;
}

void emitElfStatic::buildGOT(Symtab *target, LinkMap &lmap) {

    if( PPC32_WIDTH == addressWidth_ ) {
	return;
    }
    char *targetData = lmap.allocatedData;

    unsigned slotSize = 0;
    if( PPC32_WIDTH == addressWidth_ ) {
        slotSize = sizeof(Elf32_Addr);
    }else if( PPC64_WIDTH == addressWidth_ ) {
        slotSize = sizeof(Elf64_Addr);
    }else{
        assert(!UNKNOWN_ADDRESS_WIDTH_ASSERT);
    }

    Offset curOffset = 0;

    // Copy over the original GOT
    // Inefficient lookup, but it's easy to debug :)
    Region *origGOT = NULL; 
    if (target->findRegion(origGOT, ".got")) {
       memcpy(&targetData[lmap.gotRegionOffset + curOffset], origGOT->getPtrToRawData(), origGot->getDiskSize());
       curOffset += origGot->getDiskSize();
       rewrite_printf("Copying 0x%lx bytes of original GOT to new\n", origGOT->getDiskSize());
    }
    

    // For each GOT symbol, allocate an entry and copy the value of the
    // symbol into the table, additionally store the offset in the GOT
    // back into the map
    memset(&targetData[lmap.gotRegionOffset], 0, GOT_RESERVED_SLOTS*slotSize);
    curOffset += GOT_RESERVED_SLOTS*slotSize;

    vector<pair<Symbol *, Offset> >::iterator sym_it;
    for(sym_it = lmap.gotSymbolTable.begin(); sym_it != lmap.gotSymbolTable.end(); ++sym_it) {
        Offset value = sym_it->first->getOffset()+sym_it->second;
        memcpy(&targetData[lmap.gotRegionOffset + curOffset], &value, slotSize);

        sym_it->second = curOffset;
	rewrite_printf(" Building GOT at 0x%lx is 0x%lx name %s Offset 0x%lx addend 0x%lx addr 0x%lx \n", 
	curOffset, value,  sym_it->first->getPrettyName().c_str(),  sym_it->first->getOffset(), sym_it->second, lmap.gotRegionOffset + curOffset);
        curOffset += slotSize;
    }
    assert(curOffset == lmap.gotSize);
}

/*
 * .ctors and .dtors section handling
 * 
 * .ctors/.dtors sections are not defined by the ELF standard, LSB defines them.
 * This is why this implementation is specific to Linux and x86.
 *
 * Layout of .ctors and .dtors sections on Linux x86
 *
 * Executable .ctors/.dtors format (size in bytes = n)
 *
 *  byte 0..3    byte 4..7     byte 8..11        byte n-4..n-1
 * 0xffffffff <func. ptr 1> <func. ptr 2> ...  0x00000000
 *
 * Relocatable file .ctors/.dtors format (size in bytes = n)
 *
 *   byte 0..3         byte n-4..n-1
 * <func. ptr 1> ... <last func. ptr>
 *
 * The layout is the same on Linux x86_64 except each entry is 8 bytes
 * instead of 4. So the header and trailler are the same, but extended to
 * 8 bytes.
 */
static const string DTOR_NAME(".dtors");
static const string CTOR_NAME(".ctors");
static const string TOC_NAME(".toc");

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

    vector<Region *>::iterator reg_it;
    for(reg_it = lmap.newDtorRegions.begin(); reg_it != lmap.newDtorRegions.end(); ++reg_it) {
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

bool emitElfStatic::isGOTRegion(Region *reg) {

    if( PPC32_WIDTH == addressWidth_ ) {
	return false;
    }
	return ( TOC_NAME.compare(reg->getRegionName()) == 0 );
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
    } else if( PPC64_WIDTH == addressWidth_) {
        switch(relType) {
            case R_PPC64_GOT16:
            case R_PPC64_GOT16_LO:
            case R_PPC64_GOT16_HI:
            case R_PPC64_GOT16_HA:
            case R_PPC64_TLS:
	    case R_PPC64_TOC16_DS:
            case R_PPC64_TOC16:
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

void emitElfStatic::getExcludedSymbolNames(set<string> &) {
}

