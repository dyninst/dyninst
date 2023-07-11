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

#if !defined(_Link_Map_h_)
#define _Link_Map_h_

#include "Symtab.h"
#include "Region.h"
#include "Symbol.h"
#include "Function.h"

#include <string>
#include <utility>
#include <deque>
#include <map>
#include <vector>
using namespace std;

namespace Dyninst{
namespace SymtabAPI{

template <typename T>
struct CtorComp
{
  bool operator()(T lhs, T rhs) const
  {
    static std::string first_ctor_name("RTstatic_ctors_dtors_begin.c.o");
    static std::string last_ctor_name("RTstatic_ctors_dtors_end.c.o");
    if(lhs->symtab()->name() == first_ctor_name) return true;
    if(rhs->symtab()->name() == first_ctor_name) return false;
    if(lhs->symtab()->name() == last_ctor_name) return false;
    if(rhs->symtab()->name() == last_ctor_name) return true;
    return true;
  }
  
};
 
 
 

/*
 * A data structure that holds all the information necessary to perform a
 * static link once all the relocatable files have been copied into a new data
 * block
 */
class LinkMap {
    public:
        LinkMap();
        ~LinkMap();

        // A pair representing an allocation for a Region
        // The first Offset is the amount of padding before the Region
        // The second Offset is the offset of the location in the allocatedData
        typedef pair<Offset, Offset> AllocPair;

        // prints the LinkMap (really for debugging purposes)
        // uses globalOffset as the location of allocatedData in the target
        void print(Offset globalOffset);
        void printAll(ostream &os, Offset globalOffset);
        void printBySymtab(ostream &os, vector<Symtab *> &symtabs, Offset globalOffset);
        void printRegions(ostream &os, deque<Region *> &regions, Offset globalOffset);
        void printRegion(ostream &os, Region *region, Offset globalOffset);
        void printRegionFromInfo(ostream &os, Region *region, Offset regionOffset, Offset padding);
        
        friend ostream & operator<<(ostream &os, LinkMap &lm);

        // Data Members
        // all other members describe this block of data
        char *allocatedData; 
        Offset allocatedSize;

        // map of Regions placed in allocatedData
        map<Region *, AllocPair> regionAllocs;

        // Keep track of the dynamically allocated COMMON symbol Region
        Region *commonStorage;

        // new Region info
        // Offset -> offset in allocatedData
        // Size -> size of Region
        // Regions -> existing Regions which make up this Region

        // new bss Region info
        Offset bssRegionOffset;
        Offset bssSize;
        Offset bssRegionAlign;
        deque<Region *> bssRegions;

        // new data Region info
        Offset dataRegionOffset;
        Offset dataSize;
        Offset dataRegionAlign;
        deque<Region *> dataRegions;
        
	// Stub code region
	Offset stubRegionOffset;
	Offset stubSize;
	std::map<Symbol *, Offset> stubMap;

        // new code Region info
        Offset codeRegionOffset;
        Offset codeSize;
        Offset codeRegionAlign;
        deque<Region *> codeRegions;
        
        // new TLS Region info
        Offset tlsRegionOffset;
        Offset tlsSize;
        Offset tlsRegionAlign;
        deque<Region *> tlsRegions;
        vector<Symbol *> tlsSymbols;

        // new GOT Region info
        Offset gotRegionOffset;
        Offset gotSize;
        Offset gotRegionAlign;
	     vector<pair<Symbol *, Offset> >gotSymbolTable;
        map <Symbol *, Offset> gotSymbols;
        deque<Region *> gotRegions;

        Symtab *ctorDtorHandler;

        // new constructor Region info
        Offset ctorRegionOffset;
        Offset ctorSize;
        Offset ctorRegionAlign;
        Region *originalCtorRegion;
        set<Region *, CtorComp<Region*> > newCtorRegions;

        // new destructor Region info
        Offset dtorRegionOffset;
        Offset dtorSize;
        Offset dtorRegionAlign;
        Region *originalDtorRegion;
        set<Region *, CtorComp<Region*> > newDtorRegions;

        // Keep track of changes made to symbols and relocations
        vector< pair<Symbol *, Offset> > origSymbols;
        vector< pair<relocationEntry *, Symbol *> > origRels;

	// GNU extension: indirect functions and IRELATIV relocations
	// We basically create mini-PLT entries for load-time 
	// decisions of which function to call
	Offset pltRegionOffset;
	Offset pltSize;
	Offset pltRegionAlign;
	// First Offset: offset of the PLT stub. 
	// Second Offset: offset of the GOT entry referenced by the stub. 
	std::map<Symbol *, std::pair<Offset, Offset> > pltEntries;
	std::map<Symbol *, Offset> pltEntriesInGOT;

	// GNU extension: create a new rel section
	Offset relRegionOffset;
	Offset relSize;
	Offset relRegionAlign;
	// With a GOT-equivalent
	Offset relGotRegionOffset;
	Offset relGotSize;
	Offset relGotRegionAlign;
};

} // SymtabAPI
} // Dyninst

#endif
