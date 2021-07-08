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

static const Offset GOT_RESERVED_SLOTS = 3;
static const unsigned X86_WIDTH = 4;
static const unsigned X86_64_WIDTH = 8;

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
 * Specific to x86
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
        LinkMap &lmap, string &, Offset &symbolOffset)
{
  /*
    if( rel.name() ==  SYMTAB_CTOR_LIST_REL ) {
        // This needs to be: (the location of the .ctors table)
        if( lmap.newCtorRegions.size() > 0 ) {
            symbolOffset = lmap.ctorRegionOffset + globalOffset;
        }else if( lmap.originalCtorRegion != NULL ) {
            symbolOffset = lmap.originalCtorRegion->getDiskOffset();
        }else{
            errMsg = "Failed to locate original .ctors Region -- cannot apply relocation";
            return false;
        }
    }else if( rel.name() == SYMTAB_DTOR_LIST_REL ) {
        // This needs to be: (the location of the .dtors table)
        if( lmap.newDtorRegions.size() > 0 ) {
            symbolOffset = lmap.dtorRegionOffset + globalOffset;
        }else if( lmap.originalDtorRegion != NULL ) {
            symbolOffset = lmap.originalDtorRegion->getDiskOffset();
        }else{
            errMsg = "Failed to locate original .dtors Region -- cannot apply relocation";
            return false;
        }
	} else*/
    if (rel.name() == SYMTAB_IREL_START) {
      // Start of our moved relocation section
      symbolOffset = globalOffset + lmap.relRegionOffset;
    }
    else if (rel.name() == SYMTAB_IREL_END) {
      // End of our moved relocation section
      symbolOffset = globalOffset + lmap.relRegionOffset + lmap.relSize;
    }

    return true;
}

bool emitElfStatic::archSpecificRelocation(Symtab *, Symtab *, char *targetData, relocationEntry &rel,
       Offset dest, Offset relOffset, Offset globalOffset, LinkMap &lmap,
       string &errMsg) 
{
  Offset symbolOffset = 0;
  if (rel.getDynSym()->getType() != Symbol::ST_INDIRECT) {
    // Easy case, just use the symbol as given
    symbolOffset = rel.getDynSym()->getOffset();
  }
  else {
    return true;
  }
    
    if( X86_WIDTH == addressWidth_ ) {
        /*
         * Referring to the SYSV 386 supplement:
         *
         * All relocations on x86 are one word32 == Elf32_Word
         *
         * S = symbolOffset
         * A = addend
         * P = relOffset
         */
       
        Elf32_Word addend = 0;
        if( rel.regionType() == Region::RT_REL ) {
            memcpy(&addend, &targetData[dest], sizeof(Elf32_Word));
        }else if( rel.regionType() == Region::RT_RELA ) {
            addend = rel.addend();
        }

        if( !computeCtorDtorAddress(rel, globalOffset, lmap, errMsg, symbolOffset) ) {
            return false;
        }

        rewrite_printf("relocation for '%s': TYPE = %s(%lu) S = %lx A = %x P = %lx\n",
                rel.name().c_str(), 
                relocationEntry::relType2Str(rel.getRelType(), addressWidth_),
                rel.getRelType(), symbolOffset, addend, relOffset);

        Offset relocation = 0;
        map<Symbol *, Offset>::iterator result;
        stringstream tmp;

        switch(rel.getRelType()) {
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
#if defined(R_386_IRELATIVE)
	case R_386_IRELATIVE:
              // Consistency error; we should never try to process one of these
              // ourselves.
	  assert(0);
	  break;
#endif
            default:
                tmp << "Relocation type " << rel.getRelType() 
                    << " currently unimplemented";
                errMsg = tmp.str();
                return false;
        }

        rewrite_printf("relocation = 0x%lx @ 0x%lx\n", relocation, relOffset);

        memcpy(&targetData[dest], &relocation, sizeof(Elf32_Word));
    }else if( X86_64_WIDTH == addressWidth_ ) {
        /*
         * Referring to the SYSV supplement:
         *
         * There are a few data types used by x86_64 relocations.
         *
         * word8 = uint8_t
         * word16 = uint16_t
         * word32 = Elf64_Word
         * word64 = Elf64_Xword
         *
         * S = symbolOffset
         * A = addend
         * P = relOffset
         * Z = symbolSize
         *
         * x86_64 only uses relocations that contain the addend.
         */
        unsigned symbolSize = rel.getDynSym()->getSize();

        Elf64_Xword addend = 0;
        if( Region::RT_RELA == rel.regionType() ) {
            addend = rel.addend();
        }

        if( !computeCtorDtorAddress(rel, globalOffset, lmap, errMsg, symbolOffset) ) {
            return false;
        }

        rewrite_printf("relocation for '%s': TYPE = %s(%lu) S = %lx A = %lx P = %lx Z = %x\n",
                rel.name().c_str(), 
                relocationEntry::relType2Str(rel.getRelType(), addressWidth_),
                rel.getRelType(),
                symbolOffset, addend, relOffset, symbolSize);

        Offset relocation = 0;
        unsigned fieldSize = 0; // This is set depending on the type of relocation
        map<Symbol *, Offset>::iterator result;
        stringstream tmp;

        switch(rel.getRelType()) {
            case R_X86_64_64:
                fieldSize = sizeof(Elf64_Xword);
                relocation = symbolOffset + addend;
                break;
            case R_X86_64_PLT32:
            case R_X86_64_PC32:
            case R_X86_64_REX_GOTPCRELX:
                fieldSize = sizeof(Elf64_Word);
                relocation = symbolOffset + addend - relOffset;
                break;
            case R_X86_64_GOT32: // The address is computed when the GOT is built
                fieldSize = sizeof(Elf64_Word);
                result = lmap.gotSymbols.find(rel.getDynSym());
                if( result == lmap.gotSymbols.end() ) {
                    errMsg = "Expected GOT symbol does not exist in GOT symbol mapping";
                    return false;
                }

                relocation = result->second + addend;
                break;
            case R_X86_64_TPOFF32: // The necessary value is set during storage allocation
                fieldSize = sizeof(Elf64_Word);
                relocation = symbolOffset;
                break;
            case R_X86_64_GLOB_DAT:
            case R_X86_64_JUMP_SLOT:
                fieldSize = sizeof(Elf64_Xword);
                relocation = symbolOffset;
                break;
            case R_X86_64_TLSLD: // Load the symbol offset from the GOT
            case R_X86_64_TLSGD: 
            case R_X86_64_GOTTPOFF:
            case R_X86_64_GOTPCREL:
                fieldSize = sizeof(Elf64_Word);

                result = lmap.gotSymbols.find(rel.getDynSym());
                if( result == lmap.gotSymbols.end() ) {
                    errMsg = "Expected GOT symbol does not exist in GOT symbol mapping";
                    return false;
                }

                relocation = result->second + lmap.gotRegionOffset + 
                    globalOffset + addend - relOffset;
                break;
            case R_X86_64_DTPOFF32:
                fieldSize = sizeof(Elf64_Word);
                relocation = addend;
                break;
            case R_X86_64_32:
            case R_X86_64_32S:
                fieldSize = sizeof(Elf64_Word);
                relocation = symbolOffset + addend;
                break;
            case R_X86_64_16:
                fieldSize = sizeof(uint16_t);
                relocation = symbolOffset + addend;
                break;
            case R_X86_64_PC16:
                fieldSize = sizeof(uint16_t);
                relocation = symbolOffset + addend - relOffset;
                break;
            case R_X86_64_8:
                fieldSize = sizeof(uint8_t);
                relocation = symbolOffset + addend;
                break;
            case R_X86_64_PC8:
                fieldSize = sizeof(uint8_t);
                relocation = symbolOffset + addend - relOffset;
                break;
            case R_X86_64_RELATIVE:
            case R_X86_64_COPY:
                tmp << "ERROR: encountered relocation type(" << rel.getRelType() << 
                       ") that is meant for use during dynamic linking";
                errMsg = tmp.str();
                return false;
#if defined(R_X86_64_IRELATIVE)
           case R_X86_64_IRELATIVE:
              // Consistency error; we should never try to process one of these
              // ourselves.
              assert(0);
              return false;
#endif
            case R_X86_64_DTPMOD64:
            case R_X86_64_DTPOFF64:
            case R_X86_64_TPOFF64:
            default:
                tmp << "Relocation type " << rel.getRelType() 
                    << " currently unimplemented";
                errMsg = tmp.str();
                return false;
        }

        rewrite_printf("relocation = 0x%lx @ 0x%lx\n", relocation, relOffset);

        memcpy(&targetData[dest], &relocation, fieldSize);
    }else{
        assert(!UNKNOWN_ADDRESS_WIDTH_ASSERT);
    }

    return true;
}

static const string LIBC_ATEXIT_INTERNAL("__cxa_exit");
static const string LIBC_ATEXIT("atexit");

bool emitElfStatic::checkSpecialCaseSymbols(Symtab *member, Symbol *checkSym) {
    /* 
     * Special Case 1
     *
     * When linking code into stripped binaries, some linked functions will not
     * work correctly because they rely on some state being initialized or
     * updated in the original binary.
     *
     * For example, the libc function atexit internally keeps a list of
     * functions to call after the main function returns. When main returns,
     * this list is processed by libc finalization code. 
     * 
     * When the original binary is stripped, there is a disconnect between the
     * atexit function in the linked code and the original atexit function.
     * Consequently, any functionality that relies on atexit will not work
     * (i.e. destructors for global C++ objects).
     *
     * The initial release of the binary rewriter for static binaries doesn't
     * provide a solution to this problem. Therefore, a warning needs to be
     * generated to alert the user to this outstanding problem.
     */
    if( isStripped_ ) {
        if( LIBC_ATEXIT_INTERNAL == checkSym->getPrettyName() ||
            LIBC_ATEXIT == checkSym->getPrettyName() )
        {
            fprintf(stderr, "WARNING: code in %s(%s) references the libc function %s.\n",
                    member->getParentArchive()->name().c_str(),
                    member->memberName().c_str(), checkSym->getPrettyName().c_str());
            fprintf(stderr, "         Also, the binary to be rewritten is stripped.\n");
            fprintf(stderr, "         Currently, the combination of these two "
                            "is unsupported and unexpected behavior may occur.\n");
        }
    }

    return true;
}

/* The TLS implementation on x86 is Variant 2 */

Offset emitElfStatic::layoutTLSImage(Offset globalOffset, Region *dataTLS, Region *bssTLS, LinkMap &lmap) {
    return tlsLayoutVariant2(globalOffset, dataTLS, bssTLS, lmap);
}

Offset emitElfStatic::adjustTLSOffset(Offset curOffset, Offset tlsSize) {
    return tlsAdjustVariant2(curOffset, tlsSize);
}

char emitElfStatic::getPaddingValue(Region::RegionType rtype) {
    const char X86_NOP = (char)0x90;

    char retChar = 0;
    if( rtype == Region::RT_TEXT || rtype == Region::RT_TEXTDATA ) {
        retChar = X86_NOP;
    }

    return retChar;
}

void emitElfStatic::cleanupTLSRegionOffsets(map<Region *, LinkMap::AllocPair> &regionAllocs,
        Region *dataTLS, Region *bssTLS) 
{
    tlsCleanupVariant2(regionAllocs, dataTLS, bssTLS);
}

void emitElfStatic::getExcludedSymbolNames(set<string> &symNames) {
    /*
     * It appears that some .o's have a reference to _GLOBAL_OFFSET_TABLE_
     * This symbol is an indication to the linker that a GOT should be 
     * created, it isn't a symbol that should be resolved. 
     *
     * Consequently, a GOT shouldn't always be created when linking 
     * the .o's into the target. A GOT is built when certain relocations
     * exist that require a GOT.
     */
    symNames.insert("_GLOBAL_OFFSET_TABLE_");
}

bool emitElfStatic::isGOTRelocation(unsigned long relType) {
    if( X86_WIDTH == addressWidth_ ) {
        switch(relType) {
            case R_386_GOT32:
            case R_386_TLS_IE:
            case R_386_TLS_GOTIE:
            case R_386_TLS_LDM:
            case R_386_TLS_GD:
                return true;
                break;
            default:
                return false;
                break;
        }
    }else if( X86_64_WIDTH == addressWidth_ ) {
        switch(relType) {
            case R_X86_64_TLSLD:
            case R_X86_64_TLSGD:
            case R_X86_64_GOTTPOFF:
            case R_X86_64_GOT32:
            case R_X86_64_GOTPCREL:
                return true;
                break;
            default:
                return false;
                break;
        }
    }else{
        assert(!UNKNOWN_ADDRESS_WIDTH_ASSERT);
    }

    return false;
}

Offset emitElfStatic::getGOTSize(Symtab *, LinkMap &lmap, Offset &layoutStart) {
    Offset size = 0;
    layoutStart = 0;

    unsigned slotSize = 0;
    if( X86_WIDTH == addressWidth_ ) {
        slotSize = sizeof(Elf32_Addr);
    }else if( X86_64_WIDTH == addressWidth_ ) {
        slotSize = sizeof(Elf64_Addr);
    }else{
        assert(!UNKNOWN_ADDRESS_WIDTH_ASSERT);
    }

    // According to the ELF abi, entries 0, 1, 2 are reserved in a GOT on x86
    if( lmap.gotSymbols.size() > 0 ) {
      size = (lmap.gotSymbols.size()+GOT_RESERVED_SLOTS)*slotSize;
    }

    return size;
}

Offset emitElfStatic::getGOTAlign(LinkMap &) {
    if( X86_WIDTH == addressWidth_ ) {
        return sizeof(Elf32_Word);
    }else if( X86_64_WIDTH == addressWidth_ ) {
        return sizeof(Elf64_Xword);
    }else{
        assert(!UNKNOWN_ADDRESS_WIDTH_ASSERT);
    }

    return 0;
}

void emitElfStatic::buildGOT(Symtab *, LinkMap &lmap) {
    char *targetData = lmap.allocatedData;

    unsigned slotSize = 0;
    if( X86_WIDTH == addressWidth_ ) {
        slotSize = sizeof(Elf32_Addr);
    }else if( X86_64_WIDTH == addressWidth_ ) {
        slotSize = sizeof(Elf64_Addr);
    }else{
        assert(!UNKNOWN_ADDRESS_WIDTH_ASSERT);
    }

    // For each GOT symbol, allocate an entry and copy the value of the
    // symbol into the table, additionally store the offset in the GOT
    // back into the map
    Offset curOffset = GOT_RESERVED_SLOTS*slotSize;
    memset(&targetData[lmap.gotRegionOffset], 0, GOT_RESERVED_SLOTS*slotSize);

    map<Symbol *, Offset>::iterator sym_it;
    for(sym_it = lmap.gotSymbols.begin(); sym_it != lmap.gotSymbols.end(); ++sym_it) {
        Offset value = sym_it->first->getOffset();
        memcpy(&targetData[lmap.gotRegionOffset + curOffset], &value, slotSize);

        sym_it->second = curOffset;

        curOffset += slotSize;
    }
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
static const string DTOR_NAME2(".fini_array");
static const string CTOR_NAME2(".init_array");

bool emitElfStatic::isConstructorRegion(Region *reg) {
    return ( CTOR_NAME.compare(reg->getRegionName()) == 0 ||
	     CTOR_NAME2.compare(reg->getRegionName()) == 0);
}

bool emitElfStatic::isGOTRegion(Region *) {
        return false;
}

Offset emitElfStatic::layoutNewCtorRegion(LinkMap &lmap) {
    /* 
     * .ctors sections are processed in reverse order on Linux x86. New .ctors
     * sections need to be placed before the original .ctors section
     */
    Offset retOffset = lmap.ctorRegionOffset; 

    pair<map<Region *, LinkMap::AllocPair>::iterator, bool> result;

    for(auto reg_it = lmap.newCtorRegions.begin(); 
	reg_it != lmap.newCtorRegions.end(); ++reg_it) {
      
        result = lmap.regionAllocs.insert(make_pair(*reg_it, make_pair(0, retOffset)));

        // If the map already contains this Region, this is a logic error
        if( !result.second ) {
            return ~0UL;
        }

        retOffset += (*reg_it)->getDiskSize();
    }
    return retOffset;
}

bool emitElfStatic::createNewCtorRegion(LinkMap &) {
    return true;
}

bool emitElfStatic::isDestructorRegion(Region *reg) {
    return ( DTOR_NAME.compare(reg->getRegionName()) == 0 ||
	     DTOR_NAME2.compare(reg->getRegionName()) == 0);
}

Offset emitElfStatic::layoutNewDtorRegion(LinkMap &lmap) {
    /*
     * .dtors sections are processed in forward order on Linux x86. So new
     * .dtors sections need to be placed after the original .dtors section
     */
    Offset retOffset = lmap.dtorRegionOffset;

    pair<map<Region *, LinkMap::AllocPair>::iterator, bool> result;


    for(auto reg_it = lmap.newDtorRegions.begin(); reg_it != lmap.newDtorRegions.end(); ++reg_it) {
        result = lmap.regionAllocs.insert(make_pair(*reg_it, make_pair(0, retOffset)));

        // If the map already contains this Region, this is a logic error
        if( !result.second ) {
            return ~0UL;
        }

        retOffset += (*reg_it)->getDiskSize();
    }
    return retOffset;
}

bool emitElfStatic::createNewDtorRegion(LinkMap &) {

    return true;
}

bool emitElfStatic::updateTOC(Symtab *, LinkMap &, Offset) {
   return true;
}

Offset emitElfStatic::allocStubRegions(LinkMap &lmap, Offset) {
   // Size 0
   return lmap.stubRegionOffset;
}

inline
static bool adjustValInRegion(Region *reg, Offset offInReg, Offset addressWidth, int adjust) {
    Offset newValue;
    unsigned char *oldValues;

    oldValues = reinterpret_cast<unsigned char *>(reg->getPtrToRawData());
    memcpy(&newValue, &oldValues[offInReg], addressWidth);
    newValue += adjust;
    return reg->patchData(offInReg, &newValue, addressWidth);
}

bool emitElfUtils::updateRelocation(Symtab *obj, relocationEntry &rel, int library_adjust) {
    // Currently, only verified on x86 and x86_64 -- this may work on other architectures
    Region *targetRegion = obj->findEnclosingRegion(rel.rel_addr());
    if( NULL == targetRegion ) {
        rewrite_printf("Failed to find enclosing Region for relocation");
        return false;
    }

    unsigned addressWidth = obj->getAddressWidth();
    if( addressWidth == 8 ) {
        switch(rel.getRelType()) {
            case R_X86_64_IRELATIVE:
            case R_X86_64_RELATIVE:
                rel.setAddend(rel.addend() + library_adjust);
                break;
            case R_X86_64_JUMP_SLOT:
                if( !adjustValInRegion(targetRegion,
                           rel.rel_addr() - targetRegion->getDiskOffset(),
                           addressWidth, library_adjust) )
                {
                    rewrite_printf("Failed to update relocation\n");
                    return false;
                }
                break;
            default:
                //fprintf(stderr, "Unimplemented relType for architecture: %d\n", rel.getRelType());
                //assert(0);
                break;
        }
    }else{
        switch(rel.getRelType()) {
            case R_386_IRELATIVE:
            case R_386_RELATIVE:
                // On x86, addends are stored in their target location
                if( !adjustValInRegion(targetRegion,
                           rel.rel_addr() - targetRegion->getDiskOffset(),
                           addressWidth, library_adjust) )
                {
                    rewrite_printf("Failed to update relocation\n");
                    return false;
                }
                break;
            case R_386_JMP_SLOT:
                if( !adjustValInRegion(targetRegion,
                           rel.rel_addr() - targetRegion->getDiskOffset(),
                           addressWidth, library_adjust) )
                {
                    rewrite_printf("Failed to update relocation\n");
                    return false;
                }
                break;
            default:
                //fprintf(stderr, "Unimplemented relType for architecture\n");
                //assert(0);
                break;
        }
    }

    // XXX The GOT also holds a pointer to the DYNAMIC segment -- this is currently not
    // updated. However, this appears to be unneeded for regular shared libraries.

    // From the SYS V ABI x86 supplement
    // "The table's entry zero is reserved to hold the address of the dynamic structure,
    // referenced with the symbol _DYNAMIC. This allows a program, such as the
    // dynamic linker, to find its own dynamic structure without having yet processed
    // its relocation entries. This is especially important for the dynamic linker, because
    // it must initialize itself without relying on other programs to relocate its memory
    // image."

    // In order to implement this, would have determine the final address of a new .dynamic
    // section before outputting the patched GOT data -- this will require some refactoring.

    //rewrite_printf("WARNING: updateRelocation is not implemented on this architecture\n");
    //(void) obj; (void) rel; (void) library_adjust; //silence warnings

    return true;
}
