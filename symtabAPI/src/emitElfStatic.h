/*
 * Copyright (c) 1996-2010 Barton P. Miller
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

#if !defined(_emit_Elf_Static_h_)
#define _emit_Elf_Static_h

#include "Symtab.h"
#include "Archive.h"
#include "Region.h"
#include "Symbol.h"
#include "LinkMap.h"
#include "Object.h"

#include <deque>
#include <map>
#include <vector>
#include <set>
using namespace std;

namespace Dyninst{
namespace SymtabAPI{

/*
 * XXX
 *
 * This class is unnecessary. However, at the time of writing, emitElf was
 * split into two different classes (one for 32-bit and 64-bit). Instead of
 * duplicating code, this class was created to share code between the
 * two emitElf classes.
 *
 * Once the emitElf classes are merged, this class can be merged with the new
 * emitElf class.
 */

extern const std::string SYMTAB_CTOR_LIST_REL;
extern const std::string SYMTAB_DTOR_LIST_REL;

class emitElfStatic {
    public:

    emitElfStatic(unsigned addressWidth, bool isStripped);

    enum StaticLinkError {
        No_Static_Link_Error,
        Link_Location_Error,
        Symbol_Resolution_Failure,
        Relocation_Computation_Failure,
        Storage_Allocation_Failure
    };

    static std::string printStaticLinkError(StaticLinkError);

    // Entry point for static linking
    char *linkStatic(Symtab *target, 
                     StaticLinkError &err, 
                     string &errMsg);

    bool resolveSymbols(Symtab *target, 
                        vector<Symtab *> &relocatableObjects, 
                        StaticLinkError &err, 
                        string &errMsg);

    bool createLinkMap(Symtab *target,
                       vector<Symtab *> &relocatableObjects,
                       Offset& globalOffset,
                       LinkMap &lmap,
                       StaticLinkError &err, 
                       string &errMsg);

    Offset layoutRegions(deque<Region *> &regions, 
                           map<Region *, LinkMap::AllocPair> &regionAllocs,
                           Offset currentOffset, 
                           Offset globalOffset);

    bool addNewRegions(Symtab *target,
                       Offset globalOffset,
                       LinkMap &lmap);
    
    void copyRegions(LinkMap &lmap); 

    bool applyRelocations(Symtab *target,
                          vector<Symtab *> &relocatableObjects,
                          Offset globalOffset,
                          LinkMap &lmap,
                          StaticLinkError &err, 
                          string &errMsg);

    bool hasRewrittenTLS() const;

    private:

    Offset computePadding(Offset candidateOffset, Offset alignment);
    char getPaddingValue(Region::RegionType rtype);

    bool archSpecificRelocation(char *targetData, 
                                relocationEntry &rel, 
                                Offset dest, 
                                Offset relOffset,
                                Offset globalOffset,
                                LinkMap &lmap,
                                string &errMsg);

    // Functions for dealing with special sections (GOT, TLS, CTORS, DTORS, etc)
    Offset layoutTLSImage(Offset globalOffset, Region *dataTLS, Region *bssTLS, LinkMap &lmap);
    Offset tlsLayoutVariant1(Offset globalOffset, Region *dataTLS, Region *bssTLS, LinkMap &lmap);
    Offset tlsLayoutVariant2(Offset globalOffset, Region *dataTLS, Region *bssTLS, LinkMap &lmap);
    Offset adjustTLSOffset(Offset curOffset, Offset tlsSize);
    Offset tlsAdjustVariant2(Offset curOffset, Offset tlsSize);
    void cleanupTLSRegionOffsets(map<Region *, LinkMap::AllocPair> &regionAllocs,
            Region *dataTLS, Region *bssTLS);
    void tlsCleanupVariant1(map<Region *, LinkMap::AllocPair> &regionAllocs,
            Region *dataTLS, Region *bssTLS);
    void tlsCleanupVariant2(map<Region *, LinkMap::AllocPair> &regionAllocs,
            Region *dataTLS, Region *bssTLS);

    bool isGOTRelocation(unsigned long relType);
    void buildGOT(LinkMap &lmap);
    Offset getGOTSize(LinkMap &lmap);
    Offset getGOTAlign(LinkMap &lmap);

    bool isConstructorRegion(Region *reg);
    Offset layoutNewCtorRegion(LinkMap &lmap);
    bool createNewCtorRegion(LinkMap &lmap);

    bool isDestructorRegion(Region *reg);
    Offset layoutNewDtorRegion(LinkMap &lmap);
    bool createNewDtorRegion(LinkMap &lmap);

    void getExcludedSymbolNames(std::set<std::string> &symNames);
    bool checkSpecialCaseSymbols(Symtab *target, Symbol *checkSym);

    unsigned addressWidth_;
    bool isStripped_;
    bool hasRewrittenTLS_;
};

} // Dyninst
} // SymtabAPI

#endif
