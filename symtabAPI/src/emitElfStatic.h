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
 *
 * See the implementation for descriptions of functions.
 */

extern const std::string SYMTAB_CTOR_LIST_REL;
extern const std::string SYMTAB_DTOR_LIST_REL;
extern const std::string SYMTAB_IREL_START;
extern const std::string SYMTAB_IREL_END;


/*
 * The above "not necessary" comment applies to this class as well.
 * These routines should be in a private namespace inside a unified
 * emit class file or something.
 */
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

    // Entry point for static linking
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

    /**
     * Architecture specific
     *
     * Given the Region type of a combined Region, gets the padding value to
     * use in between Regions that make up the combined Region.
     *
     * rtype    The Region type for the combined Region
     *
     * Returns the padding character
     */
    char getPaddingValue(Region::RegionType rtype);

    /**
     * Architecture specific
     *
     * Calculates a relocation and applies it to the specified location in the
     * target.
     *
     * targetData       The target buffer
     * rel              The relocation entry
     * dest             The offset in the target buffer
     * relOffset        The absolute offset of the relocation
     * globalOffset     The absolute offset of the newly linked code
     * lmap             Holds information necessary to compute relocation
     *
     * Returns true, on success; false, otherwise and sets errMsg
     */
    bool archSpecificRelocation(Symtab *targetSymtab,
    				Symtab *srcSymtab,
				char *targetData, 
                                relocationEntry &rel, 
                                Offset dest, 
                                Offset relOffset,
                                Offset globalOffset,
                                LinkMap &lmap,
                                string &errMsg);

    // PPC64 TOC-changing inter-module calls
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

    // Functions for dealing with special sections (GOT, TLS, CTORS, DTORS, etc) //

    /**
     * Architecture specific (similar to layoutRegions)
     *
     * Creates a new TLS initialization image from the existing TLS Regions in the
     * target and any new TLS Regions from the relocatable objects.
     *
     * globalOffset     The absolute offset of the newly linked code
     * dataTLS          The original TLS data Region from the target (can be NULL)
     * bssTLS           The original TLS bss Region from the target (can be NULL)
     * lmap             Holds information necessary to do layout
     *
     * Returns the ending Offset of the Region
     */
    Offset layoutTLSImage(Offset globalOffset, Region *dataTLS, Region *bssTLS, LinkMap &lmap);
    Offset tlsLayoutVariant1(Offset globalOffset, Region *dataTLS, Region *bssTLS, LinkMap &lmap);
    Offset tlsLayoutVariant2(Offset globalOffset, Region *dataTLS, Region *bssTLS, LinkMap &lmap);

    /**
     * Architecture specific
     *
     * Updates the TLS offset of a Symbol, given the size of the new TLS initialization image.
     *
     * curOffset        The current offset of the TLS symbol
     * tlsSize          The size of the new TLS initialization image
     *
     * Returns the adjusted offset
     */
    Offset adjustTLSOffset(Offset curOffset, Offset tlsSize);
    Offset tlsAdjustVariant2(Offset curOffset, Offset tlsSize);

    /**
     * Architecture specific
     *
     * In order to simplify the creation of a new TLS initialization image, some cleanup 
     * work may be necessary after the new TLS initialization image is created.
     *
     * regionAllocs     The map of Regions to their place in the newly linked code
     * dataTLS          The original TLS data section from the target (can be NULL)
     * bssTLS           The original TLS bss section from the target (can be NULL)
     */
    void cleanupTLSRegionOffsets(map<Region *, LinkMap::AllocPair> &regionAllocs,
            Region *dataTLS, Region *bssTLS);
    void tlsCleanupVariant1(map<Region *, LinkMap::AllocPair> &regionAllocs,
            Region *dataTLS, Region *bssTLS);
    void tlsCleanupVariant2(map<Region *, LinkMap::AllocPair> &regionAllocs,
            Region *dataTLS, Region *bssTLS);

    /**
     * Architecture specific
     *
     * Determines if the passed relocation type requires the building of a GOT
     *
     * relType          The relocation type to check
     *
     * Returns true if the relocation type requires a GOT
     */
    bool isGOTRelocation(unsigned long relType);

    /**
     * Architecture specific
     *
     * Constructions a new GOT Region from information in the LinkMap
     */
    void buildGOT(Symtab *target, LinkMap &lmap);

    /**
     * Architecture specific
     *
     * Determines the size of the GOT Region from information in the LinkMap
     */
    Offset getGOTSize(Symtab *target, LinkMap &lmap, Offset &layoutStart);

    /**
     * Architecture specific
     *
     * Determines the GOT Region alignment from information in the LinkMap
     */
    Offset getGOTAlign(LinkMap &lmap);

    /**
     * Architecture specific
     *
     * Determines if the passed Region corresponds to a constructor table Region
     */
    bool isConstructorRegion(Region *reg);

    /**
     * Architecture specific
     *
     * Lays out a new constructor table Region from the existing constructor
     * table in the target and any new constructor Regions in the relocatable files
     *
     * Returns the ending offset of the new Region
     */
    Offset layoutNewCtorRegion(LinkMap &lmap);

    /**
     *
     * Creates a new constructor Table Region using information stored in the LinkMap
     *
     * Returns true on success
     */
    bool createNewCtorRegion(LinkMap &lmap);

    /**
     * Architecture specific
     *
     * Determines if the passed Region corresponds to a destructor table Region
     */
    bool isDestructorRegion(Region *reg);
    bool isGOTRegion(Region *reg);

    /**
     * Architecture specific
     *
     * Lays out a new destructor table Region from the existing destructor
     * table in the target and any new destructor Regions in the relocatable files
     *
     * Returns the ending offset of the new Region
     */
    Offset layoutNewDtorRegion(LinkMap &lmap);

    /**
     * Architecture specific
     *
     * Creates a new destructor Table Region using information stored in the LinkMap
     *
     * Returns true on success
     */
    bool createNewDtorRegion(LinkMap &lmap);

    /**
     * Architecture specific
     *
     * Gets the symbols that should be excluded when resolving symbols
     *
     * symNames         This set is populated by the function
     */
    void getExcludedSymbolNames(std::set<std::string> &symNames);

    /**
     * Architecture specific
     *
     * Checks if the specified symbol satisfies a special case that is
     * currently not handled by emitElfStatic.
     *
     * member           The reloctable object to examine
     * checkSym         The symbol to check
     *
     * Returns false if the symbol satisfies a special case
     */
    bool checkSpecialCaseSymbols(Symtab *member, Symbol *checkSym);

    /**
     * More with the architecture specific
     *
     * Calculate new TOC values if we care (PPC64)
     */
    bool calculateTOCs(Symtab *target, deque<Region *> &regions, Offset GOTbase, Offset newGOToffset, Offset globalOffset);

    /**
     * Somewhat architecture specific
     *
     * Allocate PLT entries for each INDIRECT-typed symbol
     * Each PLT entry has an arch-specific size
     */
    Offset allocatePLTEntries(std::map<Symbol *, std::pair<Offset, Offset> > &entries,
			      Offset pltOffset, Offset &size);

    /**
     * Aaand... architecture specific. 
     * Generate a new relocation section that combines relocs
     * from any indirect symbols with original relocs
     */
    Offset allocateRelocationSection(std::map<Symbol *, std::pair<Offset, Offset> > &entries,
				     Offset relocOffset, Offset &size,
				     Symtab *target);

    Offset allocateRelGOTSection(const std::map<Symbol *, std::pair<Offset, Offset> > &entries,
				 Offset relocOffset, Offset &size);

    bool addIndirectSymbol(Symbol *sym, LinkMap &lmap);
    
    // Update the TOC pointer if necessary (PPC, 64-bit)
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
