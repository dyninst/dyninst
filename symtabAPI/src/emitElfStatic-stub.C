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

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

// This needs to be a #define so the assert actually shows the message
#define EMIT_STATIC_ASSERT "This function is currently unimplemented on this architecture."

bool emitElfStatic::archSpecificRelocation(Symtab *, Symtab *, char *, relocationEntry &,
        Offset, Offset, Offset, LinkMap &, string &) {
    assert(!EMIT_STATIC_ASSERT);
    return false;
}

bool emitElfStatic::checkSpecialCaseSymbols(Symtab *, Symbol *) {
    assert(!EMIT_STATIC_ASSERT);
    return false;
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

Offset emitElfStatic::getGOTSize(LinkMap &) {
    assert(!EMIT_STATIC_ASSERT);
    return 0;
}

Offset emitElfStatic::getGOTAlign(LinkMap &) {
    assert(!EMIT_STATIC_ASSERT);
    return 0;
}

void emitElfStatic::buildGOT(LinkMap &) {
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
