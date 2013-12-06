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

#include "LinkMap.h"
#include <iostream>

using namespace Dyninst;
using namespace SymtabAPI;

LinkMap::LinkMap() :
    allocatedData(NULL), allocatedSize(0), commonStorage(NULL),
    bssRegionOffset(0), bssSize(0), bssRegionAlign(0),
    dataRegionOffset(0), dataSize(0), dataRegionAlign(0),
    stubRegionOffset(0), stubSize(0),
    codeRegionOffset(0), codeSize(0), codeRegionAlign(0),
    tlsRegionOffset(0), tlsSize(0), tlsRegionAlign(0),
    gotRegionOffset(0), gotSize(0), gotRegionAlign(0),
    ctorDtorHandler(NULL), ctorRegionOffset(0), ctorSize(0),
    ctorRegionAlign(0), originalCtorRegion(NULL), dtorRegionOffset(0),
    dtorSize(0), dtorRegionAlign(0), originalDtorRegion(NULL),
    pltRegionOffset(0), pltSize(0), pltRegionAlign(0),
    relRegionOffset(0), relSize(0), relRegionAlign(0),
    relGotRegionOffset(0), relGotSize(0), relGotRegionAlign(0)
{
}

LinkMap::~LinkMap() {
    if( commonStorage ) delete commonStorage;
}

ostream & operator<<(ostream &os, LinkMap &lm) {
     lm.printAll(os, 0);
     return os;
}

void LinkMap::print(Offset globalOffset) {
    printAll(std::cout, globalOffset);
}

void LinkMap::printAll(ostream &os, Offset globalOffset) {
    os << "Size of allocated space = 0x" << hex << allocatedSize << dec << endl;

    if( codeRegions.size() > 0 ) {
        os << "New CODE Region: Offset: 0x" << hex << (globalOffset + codeRegionOffset) << dec
           << " Size: 0x" << hex << codeSize << dec 
           << " Alignment: 0x" << hex << codeRegionAlign << dec
           << endl;
        printRegions(os, codeRegions, globalOffset);
        os << endl;
    }

    if( dataRegions.size() > 0 ) {
        os << "New DATA Region: Offset: 0x" << hex << (globalOffset + dataRegionOffset) << dec
           << " Size: 0x" << hex << dataSize << dec 
           << " Alignment: 0x" << hex << dataRegionAlign << dec
           << endl;
        printRegions(os, dataRegions, globalOffset);
        os << endl;
    }

    if( tlsRegions.size() > 0 ) {
        os << "New TLS Region: Offset: 0x" << hex << (globalOffset + tlsRegionOffset) << dec
           << " Size: 0x" << hex << tlsSize << dec 
           << " Alignment: 0x" << hex << tlsRegionAlign << dec
           << endl;
        printRegions(os, tlsRegions, globalOffset);

        os << endl;

        // Print each symbol ordered by its offset from the TCB
        map<Offset, Symbol *> off2Sym;
        vector<Symbol *>::iterator sym_it;
        for(sym_it = tlsSymbols.begin(); sym_it != tlsSymbols.end(); ++sym_it) {
            off2Sym.insert(make_pair((*sym_it)->getOffset(), *sym_it));
        }

        map<Offset, Symbol *>::iterator off_it;
        for(off_it = off2Sym.begin(); off_it != off2Sym.end(); ++off_it) {
            os << "\tSymbol: " << off_it->second->getMangledName() 
               << " Offset: 0x" << hex << off_it->first << dec 
               << endl;
        }

        os << endl;
    }

    if( gotSize > 0 ) {
        os << "New GOT Region: Offset: 0x" << hex << (globalOffset + gotRegionOffset) << dec
           << " Size: 0x" << hex << gotSize << dec 
           << " Alignment: 0x" << hex << gotRegionAlign << dec
           << endl;

        // Print out GOT entries in order
        map<Offset, Symbol *> gotByEntry;
        map<Symbol *, Offset>::iterator sym_it;
        for(sym_it = gotSymbols.begin(); sym_it != gotSymbols.end(); ++sym_it) {
            gotByEntry.insert(make_pair(sym_it->second, sym_it->first));
        }

        map<Offset, Symbol *>::iterator off_it;
        for(off_it = gotByEntry.begin(); off_it != gotByEntry.end(); ++off_it) {
            os << "\tGOT Offset: 0x" << hex << off_it->first << dec
               << " Symbol: " << off_it->second->getMangledName()
               << " Offset: 0x" << hex << off_it->second->getOffset() << dec << endl;
        }

        os << endl;
    }

    if( newCtorRegions.size() > 0 ) {
        os << "New .ctors region: Offset: 0x" << hex << (globalOffset + ctorRegionOffset) << dec
           << " Size: 0x" << hex << ctorSize << dec
           << " Alignment: 0x" << hex << ctorRegionAlign << dec
           << endl;

        if( originalCtorRegion != NULL ) {
            printRegionFromInfo(os, originalCtorRegion, 0, 0);
        }

        for(auto reg_it = newCtorRegions.begin(); 
	    reg_it != newCtorRegions.end(); ++reg_it) {
            printRegion(os, *reg_it, globalOffset);
        }

        os << endl;
    }

    if( newDtorRegions.size() > 0 ) {
         os << "New .dtors region: Offset: 0x" << hex << (globalOffset + dtorRegionOffset) << dec
           << " Size: 0x" << hex << dtorSize << dec
           << " Alignment: 0x" << hex << dtorRegionAlign << dec
           << endl;

        if( originalDtorRegion != NULL ) {
            printRegionFromInfo(os, originalDtorRegion, 0, 0);
        }

        for(auto reg_it = newDtorRegions.begin(); 
	    reg_it != newDtorRegions.end(); ++reg_it) {
            printRegion(os, *reg_it, globalOffset);
        }

        os << endl;
        
    }

    if( bssRegions.size() > 0 ) {
        os << "New BSS Region: Offset: 0x" << hex << (globalOffset + bssRegionOffset) << dec
           << " Size: 0x" << hex << bssSize << dec 
           << " Alignment: 0x" << hex << bssRegionAlign << dec
           << endl;
        printRegions(os, bssRegions, globalOffset);
        os << endl;
    }

}

void LinkMap::printBySymtab(ostream &os, vector<Symtab *> &symtabs, Offset globalOffset) {
    vector<Symtab *>::iterator symtab_it;
    for(symtab_it = symtabs.begin(); symtab_it != symtabs.end(); ++symtab_it) {
        os << "Object: " << (*symtab_it)->memberName() << endl;

        // Print the location of all the Regions
        vector<Region *> regions;
        if( !(*symtab_it)->getAllRegions(regions) ) continue;

        vector<Region *>::iterator reg_it;
        for(reg_it = regions.begin(); reg_it != regions.end(); ++reg_it) {
            printRegion(os, *reg_it, globalOffset);
        }

        os << endl;

        // Print the location of all the Functions
        vector<Function *> funcs;
        if( !(*symtab_it)->getAllFunctions(funcs) ) continue;

        vector<Function *>::iterator func_it;
        for(func_it = funcs.begin(); func_it != funcs.end(); ++func_it) {
            Symbol *symbol = (*func_it)->getFirstSymbol();
            os << "\tFunction: " << symbol->getPrettyName()
               << " Offset: 0x" << hex << symbol->getOffset() << dec
               << " - 0x" << hex << (symbol->getOffset() + symbol->getSize() - 1) << dec
               << " Size: 0x" << hex << symbol->getSize() << dec
               << endl;
        }

        os << endl;
    }
}

void LinkMap::printRegions(ostream &os, deque<Region *> &regions, Offset globalOffset) {
    deque<Region *>::iterator reg_it;
    for(reg_it = regions.begin(); reg_it != regions.end(); ++reg_it) {
        printRegion(os, *reg_it, globalOffset);
    }
}

void LinkMap::printRegion(ostream &os, Region *region, Offset globalOffset) {
    map<Region *, AllocPair>::iterator result;
    result = regionAllocs.find(region);
    if( result != regionAllocs.end() ) {
        AllocPair pair = result->second;
        printRegionFromInfo(os, region, globalOffset + pair.second,
                pair.first);
    }
}

void LinkMap::printRegionFromInfo(ostream &os, Region *region, Offset regionOffset, Offset padding)
{
    os << "\tRegion " << region->getRegionName() 
           << " Padding: 0x" << hex << padding << dec
           << " Offset: 0x" << hex << regionOffset << dec
           << " - 0x" << hex << (regionOffset + region->getMemSize() - 1) << dec
           << " Size: 0x" << hex << region->getMemSize() << dec
           << " Alignment: 0x" << hex << region->getMemAlignment() << dec
           << endl;
}
