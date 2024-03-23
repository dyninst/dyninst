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

#ifndef _emit_Elf_Static_h_
#define _emit_Elf_Static_h_

#include "Symtab.h"
#include "Archive.h"
#include "Region.h"
#include "Symbol.h"
#include "LinkMap.h"
#include "Object.h"

#include <string>
#include <utility>
#include <deque>
#include <map>
#include <vector>
#include <set>
using namespace std;

#include "boost/tuple/tuple.hpp"

namespace Dyninst{
namespace SymtabAPI{

extern const std::string SYMTAB_CTOR_LIST_REL;
extern const std::string SYMTAB_DTOR_LIST_REL;
extern const std::string SYMTAB_IREL_START;
extern const std::string SYMTAB_IREL_END;


class emitElfUtils {
 public:
    static Address orderLoadableSections(
        Symtab *obj, vector<Region*> & sections);
    static bool sort_reg(const Region*a, const Region*b);
    static bool updateHeapVariables(Symtab *obj, unsigned long loadSecsSize);
    static bool updateRelocation(Symtab *obj, relocationEntry &rel, int library_adjust);
};

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

    char *linkStatic(Symtab *target, 
                     StaticLinkError &err, 
                     string &errMsg);

    bool resolveSymbols(Symtab *target, 
                        vector<Symtab *> &relocatableObjects, 
                        LinkMap &lmap,
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

    Offset allocStubRegions(LinkMap &lmap, Offset globalOffset);

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
    bool buildPLT(Symtab *target,
		  Offset globalOffset,
		  LinkMap &lmap,
		  StaticLinkError &err,
		  string &errMsg);

    bool buildRela(Symtab *target,
		  Offset globalOffset,
		  LinkMap &lmap,
		  StaticLinkError &err,
		  string &errMsg);		   

    bool hasRewrittenTLS() const;

    private:

    Offset computePadding(Offset candidateOffset, Offset alignment);

    char getPaddingValue(Region::RegionType rtype);

    bool archSpecificRelocation(Symtab *targetSymtab,
    				Symtab *srcSymtab,
				char *targetData, 
                                relocationEntry &rel, 
                                Offset dest, 
                                Offset relOffset,
                                Offset globalOffset,
                                LinkMap &lmap,
                                string &errMsg);

    bool handleInterModuleSpecialCase(Symtab *target,
				      Symtab *src,
				      LinkMap &lmap,
				      char *data,
				      relocationEntry rel,
				      Offset newTOC,
				      Offset oldTOC,
				      Offset dest,
				      Offset relOffset,
				      Offset globalOffset);
    Offset findOrCreateStub(Symbol *sym, LinkMap &lmap, Offset newTOC, Offset oldTOC, char *data, Offset global);
    void createStub(unsigned *stub, Offset stubOffset, Offset newTOC, Offset oldTOC, Offset dest);

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

    void buildGOT(Symtab *target, LinkMap &lmap);

    Offset getGOTSize(Symtab *target, LinkMap &lmap, Offset &layoutStart);

    Offset getGOTAlign(LinkMap &lmap);

    bool isConstructorRegion(Region *reg);

    Offset layoutNewCtorRegion(LinkMap &lmap);

    bool createNewCtorRegion(LinkMap &lmap);

    bool isDestructorRegion(Region *reg);
    bool isGOTRegion(Region *reg);

    Offset layoutNewDtorRegion(LinkMap &lmap);

    bool createNewDtorRegion(LinkMap &lmap);

    void getExcludedSymbolNames(std::set<std::string> &symNames);

    bool checkSpecialCaseSymbols(Symtab *member, Symbol *checkSym);

    bool calculateTOCs(Symtab *target, deque<Region *> &regions, Offset GOTbase, Offset newGOToffset, Offset globalOffset);

    Offset allocatePLTEntries(std::map<Symbol *, std::pair<Offset, Offset> > &entries,
			      Offset pltOffset, Offset &size);

    Offset allocateRelocationSection(std::map<Symbol *, std::pair<Offset, Offset> > &entries,
				     Offset relocOffset, Offset &size,
				     Symtab *target);

    Offset allocateRelGOTSection(const std::map<Symbol *, std::pair<Offset, Offset> > &entries,
				 Offset relocOffset, Offset &size);

    bool addIndirectSymbol(Symbol *sym, LinkMap &lmap);
    
    bool updateTOC(Symtab *file, LinkMap &lmap, Offset globalOffset);

    unsigned addressWidth_;
    bool isStripped_;
    bool hasRewrittenTLS_;

    typedef boost::tuple<Offset, Offset, Offset> TOCstub;
    std::map<Symbol *, TOCstub> stubMap;
    Offset getStubOffset(TOCstub &t) { return boost::get<0>(t); }
    Offset getNewTOC(TOCstub &t) { return boost::get<1>(t); }
    Offset getOldTOC(TOCstub &t) { return boost::get<2>(t); }

};

} // Dyninst
} // SymtabAPI

#endif
