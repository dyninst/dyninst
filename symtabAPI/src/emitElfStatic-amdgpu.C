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
 * Holds architecture specific stub functions needed by static executable
 * rewriter. This file should be used for architectures where this feature is
 * not implemented.
 */

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <set>
#include <map>

#include "emitElfStatic.h"
#include "Symtab.h"
#include "Symbol.h"
#include "Archive.h"
#include "Object.h"
#include "Region.h"
#include "debug.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

// This needs to be a #define so the assert actually shows the message
#define EMIT_STATIC_ASSERT "This function is currently unimplemented on this architecture."

static const unsigned AARCH64_WIDTH = 8;

/* NOTE:
 * As most of these functions are defined per architecture, the description of
 * each of these functions is in the emitElfStatic header. Comments describing
 * the function interface are explicitly left out.
 */

bool emitElfStatic::archSpecificRelocation(Symtab *, Symtab *, char *, relocationEntry &,
        Offset, Offset, Offset, LinkMap &, string &) {
    assert(!EMIT_STATIC_ASSERT);
    return false;
}

//steve: TODO not sure
bool emitElfStatic::checkSpecialCaseSymbols(Symtab *, Symbol *) {
    //assert(!EMIT_STATIC_ASSERT);
    //return false;
    return true;
}

Offset emitElfStatic::layoutTLSImage(Offset, Region *, Region *, LinkMap &) {
    assert(!EMIT_STATIC_ASSERT);
    return 0;
}

Offset emitElfStatic::adjustTLSOffset(Offset, Offset) {
    assert(!EMIT_STATIC_ASSERT);
    return 0;
}

char emitElfStatic::getPaddingValue(Region::RegionType) {
    assert(!EMIT_STATIC_ASSERT);
    return 0;
}

void emitElfStatic::cleanupTLSRegionOffsets(map<Region *, LinkMap::AllocPair> &,
        Region *, Region *)
{
    assert(!EMIT_STATIC_ASSERT);
}

bool emitElfStatic::isGOTRelocation(unsigned long) {
    assert(!EMIT_STATIC_ASSERT);
    return false;
}

#ifdef OLD_VERSION
Offset emitElfStatic::getGOTSize(LinkMap &) {
    assert(!EMIT_STATIC_ASSERT);
    return 0;
}
#endif

//steve: TODO
Offset emitElfStatic::getGOTSize(Symtab *, LinkMap &, Offset &) {
    assert(!EMIT_STATIC_ASSERT);
    return 0;
}

Offset emitElfStatic::getGOTAlign(LinkMap &) {
    assert(!EMIT_STATIC_ASSERT);
    return 0;
}

#ifdef OLD_VERSION
void emitElfStatic::buildGOT(LinkMap &) {
    assert(!EMIT_STATIC_ASSERT);
}
#endif

//steve: TODO
void emitElfStatic::buildGOT(Symtab *, LinkMap &) {
    assert(!EMIT_STATIC_ASSERT);
}

bool emitElfStatic::isConstructorRegion(Region *) {
    assert(!EMIT_STATIC_ASSERT);
    return false;
}

Offset emitElfStatic::layoutNewCtorRegion(LinkMap &) {
    assert(!EMIT_STATIC_ASSERT);
    return 0;
}

bool emitElfStatic::createNewCtorRegion(LinkMap &) {
    assert(!EMIT_STATIC_ASSERT);
    return false;
}

bool emitElfStatic::isDestructorRegion(Region *) {
    assert(!EMIT_STATIC_ASSERT);
    return false;
}

bool emitElfStatic::isGOTRegion(Region *) {
    assert(!EMIT_STATIC_ASSERT);
    return false;
}

Offset emitElfStatic::layoutNewDtorRegion(LinkMap &) {
    assert(!EMIT_STATIC_ASSERT);
    return 0;
}

bool emitElfStatic::createNewDtorRegion(LinkMap &) {
    assert(!EMIT_STATIC_ASSERT);
    return false;
}

void emitElfStatic::getExcludedSymbolNames(set<string> &) {
    assert(!EMIT_STATIC_ASSERT);
}

//************
//steve: added
Offset emitElfStatic::allocStubRegions(LinkMap &, Offset){
  assert(!EMIT_STATIC_ASSERT);
	return 0;
}

bool emitElfStatic::updateTOC(Symtab *, LinkMap &, Offset){
  assert(!EMIT_STATIC_ASSERT);
	return false;
}
//***********


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

    // Used to update a Region
    /*switch( rel.getCategory( obj->getAddressWidth() ))
    {
        case relocationEntry::relative:
            rel.setAddend(rel.addend() + library_adjust);
            break;

        case relocationEntry::jump_slot:
            if( !adjustValInRegion(targetRegion,
                        rel.rel_addr() - targetRegion->getDiskOffset(),
                        addressWidth, library_adjust) )
            {
                rewrite_printf("Failed to update relocation\n");
                return false;
            }
            break;

        case relocationEntry::absolute:
            break;

        default:
            fprintf(stderr, "Undefined relocation category.\n");
            assert(0);
    }*/

    unsigned addressWidth = obj->getAddressWidth();
    if( addressWidth == 8 ) {
        switch(rel.getRelType()) {
            case R_AARCH64_RELATIVE:
            case R_AARCH64_IRELATIVE:
                rel.setAddend(rel.addend() + library_adjust);
                break;
            case R_AARCH64_JUMP_SLOT:
                if( !adjustValInRegion(targetRegion,
                           rel.rel_addr() - targetRegion->getDiskOffset(),
                           addressWidth, library_adjust) )
                {
                    rewrite_printf("Failed to update relocation\n");
                    return false;
                }
                break;
            case R_AARCH64_GLOB_DAT:
                break;
            default:
                //fprintf(stderr, "Unimplemented relType for architecture: %d\n", rel.getRelType());
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
