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

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>

#include "Archive.h"
#include "Object.h"
#include "Region.h"
#include "Symbol.h"
#include "Symtab.h"
#include "debug.h"
#include "emitElfStatic.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

/* NOTE:
 * As most of these functions are defined per architecture, the description of
 * each of these functions is in the emitElfStatic header. Comments describing
 * the function interface are explicitly left out.
 */

bool emitElfStatic::archSpecificRelocation(Symtab *, Symtab *, char *, relocationEntry &, Offset,
                                           Offset, Offset, LinkMap &, string &) {
  assert(false && "Not implemented for AMDGPU");
  return false;
}

bool emitElfStatic::checkSpecialCaseSymbols(Symtab *, Symbol *) {
  assert(false && "Not implemented for AMDGPU");
  return false;
}

Offset emitElfStatic::layoutTLSImage(Offset, Region *, Region *, LinkMap &) {
  assert(false && "Not implemented for AMDGPU");
  return 0;
}

Offset emitElfStatic::adjustTLSOffset(Offset, Offset) { return 0; }

char emitElfStatic::getPaddingValue(Region::RegionType) {
    assert(false && "Not implemented for AMDGPU);
    return 0;
}

void emitElfStatic::cleanupTLSRegionOffsets(map<Region *, LinkMap::AllocPair> &, Region *,
                                            Region *) {
    assert(false && "Not implemented for AMDGPU);
}

bool emitElfStatic::isGOTRelocation(unsigned long) {
    assert(false && "Not implemented for AMDGPU);
    return false;
}

#ifdef OLD_VERSION
Offset emitElfStatic::getGOTSize(LinkMap &) {
    assert(false && "Not implemented for AMDGPU);
    return 0;
}
#endif

Offset emitElfStatic::getGOTSize(Symtab *, LinkMap &, Offset &) {
    assert(false && "Not implemented for AMDGPU);
    return 0;
}

Offset emitElfStatic::getGOTAlign(LinkMap &) {
    assert(false && "Not implemented for AMDGPU);
    return 0;
}

#ifdef OLD_VERSION
void emitElfStatic::buildGOT(LinkMap &) {
    assert(false && "Not implemented for AMDGPU);
}
#endif

void emitElfStatic::buildGOT(Symtab *, LinkMap &) {
    assert(false && "Not implemented for AMDGPU);
}

bool emitElfStatic::isConstructorRegion(Region *) {
    assert(false && "Not implemented for AMDGPU);
    return false;
}

Offset emitElfStatic::layoutNewCtorRegion(LinkMap &) {
    assert(false && "Not implemented for AMDGPU);
    return 0;
}

bool emitElfStatic::createNewCtorRegion(LinkMap &) {
    assert(false && "Not implemented for AMDGPU);
    return false;
}

bool emitElfStatic::isDestructorRegion(Region *) {
    assert(false && "Not implemented for AMDGPU);
    return false;
}

bool emitElfStatic::isGOTRegion(Region *) {
    assert(false && "Not implemented for AMDGPU);
    return false;
}

Offset emitElfStatic::layoutNewDtorRegion(LinkMap &) {
    assert(false && "Not implemented for AMDGPU);
    return 0;
}

bool emitElfStatic::createNewDtorRegion(LinkMap &) {
    assert(false && "Not implemented for AMDGPU);
    return false;
}

void emitElfStatic::getExcludedSymbolNames(set<string> &) {
    assert(false && "Not implemented for AMDGPU);
}

Offset emitElfStatic::allocStubRegions(LinkMap &, Offset){
  assert(false && "Not implemented for AMDGPU);
	return 0;
}

bool emitElfStatic::updateTOC(Symtab *, LinkMap &, Offset) {
  assert(false && "Not implemented for AMDGPU);
	return false;
}

bool emitElfUtils::updateRelocation(Symtab * /* obj */, relocationEntry & /* rel */,
                                    int /* library_adjust */) {
    assert(false && "Not implemented for AMDGPU);
    return false;
}
