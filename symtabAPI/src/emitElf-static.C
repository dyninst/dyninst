/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <set>
#include <map>

#include "emitElf.h"
#include "Symtab.h"
#include "Symbol.h"
#include "Archive.h"
#include "Object.h"
#include "Region.h"
#include "debug.h"
#include "Function.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

static const string CODE_NAME = ".dyninstCode";
static const string DATA_NAME = ".dyninstData";
static const string BSS_NAME = ".dyninstBss";
static const string GOT_NAME = ".dyninstGot";
static const string TLS_DATA_NAME = ".tdata";
static const string DEFAULT_COM_NAME = ".common";

#if defined(arch_x86)
static const Offset GOT_RESERVED_SLOTS = 3;
#else
static const Offset GOT_RESERVED_SLOTS = 0;
#endif

/*
 * Most of these functions take a reference to a 
 * StaticLinkError and a string for error reporting 
 * purposes. These should prove useful in identifying
 * the cause of an error
 */

/**
 * Statically links object files into the specified Symtab object, 
 * as specified by state in the Symtab object
 *
 * @param target - the Symtab object being rewritten
 *
 * @return a pointer to the newly allocated space, to be written
 *         out during the rewrite process, NULL, if there is an error
 */
char *emitElf::linkStatic(Symtab *target, 
        StaticLinkError &err, string &errMsg) 
{
    // Basic algorithm is:
    // * locate defined versions of symbols for undefined symbols in 
    //   either instrumentation or dependent code, produces a vector
    //   of Symtab objects that contain the defined versions
    // * Allocate storage for all new code and data to be linked into 
    //   the target, produces a LinkMap containing all allocation 
    //   information
    // * Compute relocations and set their targets to these values. 
    // * The passed Symtab object will reflect all these changes

    // Errors are set by the functions
    
    // Holds all information necessary to work with block of data created by createLinkMap
    LinkMap lmap;
    
    // Determine starting location of new Regions
    Offset globalOffset = 0;
    vector<Region *> newRegs;
    if( target->getAllNewRegions(newRegs) ) {
        // This is true, only if other regions have already been added
        vector<Region *>::iterator newReg_it;
        for(newReg_it = newRegs.begin(); newReg_it != newRegs.end(); ++newReg_it) {
            Offset newRegEndAddr = (*newReg_it)->getRegionSize() + (*newReg_it)->getRegionAddr();
            if( newRegEndAddr > globalOffset ) {
                globalOffset = newRegEndAddr;
            }
        }
    }

    if( globalOffset == 0 ) {
        err = Link_Location_Error;
        errMsg = "failed to find location for new code and data.";
        return lmap.allocatedData;
    }

    // Holds all necessary dependencies, as determined by resolveSymbols
    vector<Symtab *> dependentObjects;
    if( !resolveSymbols(target, dependentObjects, err, errMsg) ) {
        return lmap.allocatedData;
    }

    if( !createLinkMap(target, dependentObjects, globalOffset, lmap, err, errMsg) ) {
        return lmap.allocatedData;
    }

    addNewRegions(target, globalOffset, lmap);

    // Print out the link map for debugging
    rewrite_printf("Global Offset = 0x%lx\n", globalOffset);
    if( sym_debug_rewrite ) {
        lmap.printAll(cout, globalOffset);
        lmap.printBySymtab(cout, dependentObjects, globalOffset);
    }


    if( !applyRelocations(target, dependentObjects, globalOffset, lmap, err, errMsg) ) {
        if( lmap.allocatedData ) delete lmap.allocatedData;
        return lmap.allocatedData;
    }

    rewrite_printf("\n*** Finished static linking\n\n");

    err = No_Static_Link_Error;
    errMsg = "";
    return lmap.allocatedData;
}

/**
 * Resolves undefined symbols in the specified Symtab object, usually due
 * to the addition of new Symbols to the Symtab object
 */
bool emitElf::resolveSymbols(Symtab *target, 
        vector<Symtab *> &dependentObjects,
        StaticLinkError &err, string &errMsg) 
{
    // Collection of objects that currently need their symbols resolved
    set<Symtab *> workSet;

    // Collection of objects that have already had their symbols resolved
    // this is necessary to avoid errors related to circular dependencies
    set<Symtab *> linkedSet;

    set<string> excludeSymNames;
    getExcludedSymbolNames(excludeSymNames);

    // Establish list of libraries to search for symbols
    vector<Archive *> libraries;
    target->getLinkingResources(libraries);

    // Add all object files explicitly referenced by new symbols, these
    // essentially fuel the linking process
    vector<Symtab *> explicitRefs;
    target->getExplicitSymtabRefs(explicitRefs);

    vector<Symtab *>::iterator expObj_it;
    for(expObj_it = explicitRefs.begin(); expObj_it != explicitRefs.end();
            ++expObj_it) 
    {
        dependentObjects.push_back(*expObj_it);
        workSet.insert(*expObj_it);
        linkedSet.insert(*expObj_it);
    }

    set<Symtab *>::iterator curObjFilePtr = workSet.begin();
    while( curObjFilePtr != workSet.end() ) {
        // Take an object file off the working set
        Symtab *curObjFile = *curObjFilePtr;
        workSet.erase(curObjFile);

        rewrite_printf("\n*** Resolving symbols for object: %s\n\n",
                curObjFile->memberName().c_str());

        // Build the map of Symbols to relocations
        // Also, construct collection of Symbols to be placed in GOT
        map<Symbol *, vector<relocationEntry *> > symToRels;
        vector<Region *> allRegions;
        curObjFile->getAllRegions(allRegions);

        vector<Region *>::iterator region_it;
        for(region_it = allRegions.begin(); region_it != allRegions.end();
                ++region_it)
        {
            vector<relocationEntry> &region_rels = (*region_it)->getRelocations();
            vector<relocationEntry>::iterator rel_it;
            for( rel_it = region_rels.begin(); rel_it != region_rels.end(); ++rel_it) {
                symToRels[rel_it->getDynSym()].push_back(&(*rel_it));
            }
        }

        // Iterate over all undefined symbols, attempting to resolve them
        vector<Symbol *> undefSyms;
        curObjFile->getAllUndefinedSymbols(undefSyms);

        vector<Symbol *>::iterator undefSym_it;
        for(undefSym_it = undefSyms.begin(); undefSym_it != undefSyms.end();
                ++undefSym_it) 
        {
            Symbol *curUndefSym = *undefSym_it;

            // Skip symbols that are don't cares
            if( excludeSymNames.count(curUndefSym->getPrettyName()) ) continue;

            // First, search the target for the symbol
            Symbol *extSymbol = NULL;

            // Attempt to search the target 
            if( !target->isStripped()) {
                 vector<Symbol *> foundSyms;
                 if( target->findSymbol(foundSyms, curUndefSym->getMangledName(),
                    curUndefSym->getType()) )
                 {
                    if( foundSyms.size() > 1 ) {
                        err = Symbol_Resolution_Failure;
                        errMsg = "ambiguous symbol definition: " + 
                            curUndefSym->getMangledName();
                        return false;
                    }

                    extSymbol = foundSyms[0];

                    rewrite_printf("Found external symbol %s in target with address = 0x%lx\n",
                            extSymbol->getPrettyName().c_str(), extSymbol->getOffset());
                 }
            }

            if( extSymbol == NULL ) {
                /* search loaded libraries and 
                 * add any new object files as dependent objects */
                vector<Archive *>::iterator lib_it;
                Symtab *containingSymtab = NULL;
                for(lib_it = libraries.begin(); lib_it != libraries.end(); ++lib_it) {
                    Symtab *tmpSymtab;
                    if( (*lib_it)->getMemberByGlobalSymbol(tmpSymtab, 
                                const_cast<string&>(curUndefSym->getMangledName())) ) 
                    {
                        /** 
                         * A common approach is to use the symbol that is defined first
                         * in the list of libraries (in the order specified to the linker)
                         *
                         * I am choosing to mirror that behavior here.
                         */
                        containingSymtab = tmpSymtab;
                        break;
                    }else{
                        /* Regarding duplicate symbols in Archives:
                         *
                         * gcc/ld appear to ignore the case where an Archive 
                         * contains the same symbol in different members and
                         * chooses the symbol that occurs first in the Archive's
                         * symbol table. This could produce unexpected results
                         * so it is better to alert the user of this error
                         */
                        if( Archive::getLastError() == Duplicate_Symbol ) {
                            err = Symbol_Resolution_Failure;
                            errMsg = Archive::printError(Duplicate_Symbol);
                            return false;
                        }
                    }
                }

                if( containingSymtab != NULL ) {
                    vector<Symbol *> foundSyms;
                    if( containingSymtab->findSymbol(foundSyms, curUndefSym->getMangledName(),
                        curUndefSym->getType()) )
                    {
                        if( foundSyms.size() > 1 ) {
                            err = Symbol_Resolution_Failure;
                            errMsg = "ambiguous symbol definition: " + 
                                curUndefSym->getMangledName();
                            return false;
                        }

                        extSymbol = foundSyms[0];
                        if( !linkedSet.count(containingSymtab) ) {
                            dependentObjects.push_back(containingSymtab);

                            // Resolve all symbols for this new Symtab, if it hasn't already
                            // been done

                            workSet.insert(containingSymtab);
                            linkedSet.insert(containingSymtab);
                        }

                        rewrite_printf("Found external symbol %s in module %s(%s)\n",
                                extSymbol->getPrettyName().c_str(),
                                containingSymtab->getParentArchive()->name().c_str(),
                                containingSymtab->memberName().c_str());
                    }else{
                        err = Symbol_Resolution_Failure;
                        errMsg = "inconsistency found between archive's symbol table and member's symbol table";
                        return false;   
                    }
                }
            }

            if( extSymbol == NULL ) {
                // If it is a weak symbol, it isn't an error that the symbol wasn't resolved
                if( curUndefSym->getLinkage() == Symbol::SL_WEAK ) continue;

                err = Symbol_Resolution_Failure;
                errMsg = "failed to locate symbol '" + curUndefSym->getMangledName()
                    + "' for module '" + curObjFile->memberName() + "'";
                return false;
            }

            // Store the found symbol with the related relocations
            map<Symbol *, vector<relocationEntry *> >::iterator relMap_it;
            relMap_it = symToRels.find(curUndefSym);
            if( relMap_it != symToRels.end() ) {
                vector<relocationEntry *>::iterator rel_it;
                for(rel_it = relMap_it->second.begin(); rel_it != relMap_it->second.end();
                        ++rel_it)
                {
                    (*rel_it)->addDynSym(extSymbol);
                }
            }   /* else
                 *
                 * Some libraries define a reference to a symbol and then never
                 * use it in order to ensure that the module that defines the
                 * symbol is linked 
                 */
        }

        curObjFilePtr = workSet.begin();
    }
    
    return true;
}

/**
 * Given a collection of Symtab objects, combines the code, data, bss and
 * other miscellaneous Regions into groups and places them in a new block
 * of data.
 * 
 * Also, allocates COMMON symbols in the collection of Symtab objects
 * as bss
 *
 * Also, creates a new TLS initialization image, combining the target image
 * and the image that exists in the collection of Symtab objects
 *
 * Also, creates a GOT used for indirect memory accesses that would used be
 * removed by link-time code relaxations.
 */
bool emitElf::createLinkMap(Symtab *target,
        vector<Symtab *> & dependentObjects, 
        Offset & globalOffset, 
        LinkMap &lmap,
        StaticLinkError &err, string &errMsg) 
{
    rewrite_printf("\n*** Allocating storage for new objects\n\n");

    // Create a temporary region for COMMON storage
    Region *commonStorage;

    // Used to create a new COMMON block
    multimap<Offset, Symbol *> commonAlignments;
    Offset commonRegionAlign = 0;

    // Collect all Regions that should be allocated in the new data block
    vector<Symtab *>::iterator obj_it;
    for(obj_it = dependentObjects.begin(); obj_it != dependentObjects.end(); ++obj_it) {
        vector<Region *> regs;
        if( !(*obj_it)->getAllRegions(regs) ) {
            err = Storage_Allocation_Failure;
            errMsg = "failed to locate regions in dependent object";
            return false;
        }

        vector<Region *>::iterator reg_it;
        for(reg_it = regs.begin(); reg_it != regs.end(); ++reg_it) {
            if( (*reg_it)->isLoadable() && (*reg_it)->getRegionSize() > 0) {
                if( (*reg_it)->isTLS() ) {
                    switch((*reg_it)->getRegionType()) {
                        case Region::RT_DATA:
                        case Region::RT_BSS:
                            lmap.tlsRegions.push_back(*reg_it);
                            // Determine alignment of combined region
                            if( (*reg_it)->getMemAlignment() > lmap.tlsRegionAlign ) {
                                lmap.tlsRegionAlign = (*reg_it)->getMemAlignment();
                            }
                            break;
                        default:
                            // ignore any other regions
                            break;
                    }
                }else{
                    switch((*reg_it)->getRegionType()) {
                        case Region::RT_TEXT:
                            lmap.codeRegions.push_back(*reg_it);
                            // Determine alignment of combined region
                            if( (*reg_it)->getMemAlignment() > lmap.codeRegionAlign ) {
                                lmap.codeRegionAlign = (*reg_it)->getMemAlignment();
                            }
                            break;
                        case Region::RT_DATA:
                            lmap.dataRegions.push_back(*reg_it);
                            // Determine alignment of combined region
                            if( (*reg_it)->getMemAlignment() > lmap.dataRegionAlign ) {
                                lmap.dataRegionAlign = (*reg_it)->getMemAlignment();
                            }
                            break;
                        case Region::RT_BSS:
                            lmap.bssRegions.push_back(*reg_it);
                            // Determine alignment of combined region
                            if( (*reg_it)->getMemAlignment() > lmap.bssRegionAlign ) {
                                lmap.bssRegionAlign = (*reg_it)->getMemAlignment();
                            }
                            break;
                        case Region::RT_TEXTDATA:
                            lmap.codeRegions.push_back(*reg_it);
                            // Determine alignment of combined region
                            if( (*reg_it)->getMemAlignment() > lmap.codeRegionAlign ) {
                                lmap.codeRegionAlign = (*reg_it)->getMemAlignment();
                            }
                            break;
                        default:
                            // skip other regions
                            continue;
                    }
                }
            }

            // Find symbols that need to be put in the GOT
            vector<relocationEntry> &region_rels = (*reg_it)->getRelocations();
            vector<relocationEntry>::iterator rel_it;
            for( rel_it = region_rels.begin(); rel_it != region_rels.end(); ++rel_it) {
                if( isGOTRelocation(rel_it->getRelType()) ) {
                    // initialize mapping
                    lmap.gotSymbols.insert(make_pair(rel_it->getDynSym(), 0));
                }
            }

        }

        vector<Symbol *> definedSyms;
        if( (*obj_it)->getAllDefinedSymbols(definedSyms) ) {
            vector<Symbol *>::iterator sym_it;
            for(sym_it = definedSyms.begin(); sym_it != definedSyms.end(); ++sym_it) {
                // Note: the assumption is made that a Symbol cannot 
                // be both TLS and COMMON
                if( (*sym_it)->isCommonStorage() ) {
                    // For a symbol in common storage, the offset/value is the alignment
                    commonAlignments.insert(make_pair((*sym_it)->getOffset(), *sym_it));

                    // The alignment of a COMMON Region is the maximum alignment of its
                    // symbols
                    if( (*sym_it)->getOffset() > commonRegionAlign ) {
                        commonRegionAlign = (*sym_it)->getOffset();
                    }
                }else if( (*sym_it)->getType() == Symbol::ST_THREAD_LOCAL ) {
                    lmap.tlsSymbols.push_back(*sym_it);
                }
            }
        }
    }

    /* START common block processing */

    /* Combine all common symbols into a single block
     *
     * The COMMON region is the first Region in the new linked data because it
     * is more efficient to determine the size of the COMMON Region and compute
     * the offsets of COMMON symbols in the target at the same time. The Region
     * is first because the offset of the first Region is known now, not after
     * all the sizes of different combined Regions have been calculated.
     */
    if( commonAlignments.size() > 0 ) {
        // The alignment of the COMMON Region should be the alignment for the BSS
        // region
        if( commonRegionAlign > lmap.bssRegionAlign ) {
            lmap.bssRegionAlign = commonRegionAlign;
        }

        // The following approach to laying out the COMMON symbols is greedy and
        // suboptimal (in terms of space in the target), but it's quick...
        Offset commonOffset = globalOffset;

        // Make sure the COMMON Region's alignment is satisfied
        commonOffset += computePadding(globalOffset, lmap.bssRegionAlign);
        Offset commonStartOffset = commonOffset;

        // From now on, the globalOffset starts at the beginning of the COMMON Region
        globalOffset = commonStartOffset;

        multimap<Offset, Symbol *>::iterator align_it;
        for(align_it = commonAlignments.begin(); 
            align_it != commonAlignments.end(); ++align_it)
        {
            commonOffset += computePadding(commonOffset, align_it->first);

            // Update symbol with place in new linked data
            align_it->second->setOffset(commonOffset);
            commonOffset += align_it->second->getSize();
        }

        // Update the size of common storage
        if( commonAlignments.size() > 0 ) {
            // A COMMON region is not really complete
            Region::createRegion(commonStorage, 0, Region::RP_RW,
                    Region::RT_BSS, commonOffset - commonStartOffset, 0, 
                    commonOffset - commonStartOffset,
                    DEFAULT_COM_NAME, NULL, true, false, commonRegionAlign);
            lmap.bssRegions.push_front(commonStorage);
        }
    }
    /* END common block processing */

    // Determine how all the Regions in the dependent objects will be
    // allocated
    Offset currentOffset = 0;

    // Allocate bss regions (new BSS Region is already aligned at the 
    // current offset)
    lmap.bssRegionOffset = currentOffset;
    currentOffset = layoutRegions(lmap.bssRegions, lmap.regionAllocs,
            lmap.bssRegionOffset, globalOffset);
    if( currentOffset == ~0UL ) {
        err = Storage_Allocation_Failure;
        errMsg = "assumption failed while creating link map";
        return false;
    }
    lmap.bssSize = currentOffset - lmap.bssRegionOffset;

    // Allocate code regions 
    lmap.codeRegionOffset = currentOffset;
    lmap.codeRegionOffset += computePadding(globalOffset + lmap.codeRegionOffset,
            lmap.codeRegionAlign);
    currentOffset = layoutRegions(lmap.codeRegions, lmap.regionAllocs,
            lmap.codeRegionOffset, globalOffset);
    if( currentOffset == ~0UL ) {
        err = Storage_Allocation_Failure;
        errMsg = "assumption failed while creating link map";
        return false;
    }
    lmap.codeSize = currentOffset - lmap.codeRegionOffset;

    // Allocate data regions 
    lmap.dataRegionOffset = currentOffset;
    lmap.dataRegionOffset += computePadding(globalOffset + lmap.dataRegionOffset,
            lmap.dataRegionAlign);
    currentOffset = layoutRegions(lmap.dataRegions, lmap.regionAllocs, 
            lmap.dataRegionOffset, globalOffset);
    if( currentOffset == ~0UL ) {
        err = Storage_Allocation_Failure;
        errMsg = "assumption failed while creating link map";
        return false;
    }
    lmap.dataSize = currentOffset - lmap.dataRegionOffset;

    // Allocate space for a GOT Region, if necessary
    lmap.gotSize = getGOTSize(lmap);
    if( lmap.gotSize > 0 ) {
        lmap.gotRegionAlign = getGOTAlign(lmap);
        currentOffset += computePadding(globalOffset + currentOffset,
                lmap.gotRegionAlign);
        lmap.gotRegionOffset = currentOffset;
        currentOffset += lmap.gotSize;
    }

    /* 
     * Build new TLS initialization image for target, if necessary
     *
     * Find current TLS Regions in target, also check for multiple
     * TLS Regions of the same type => how to construct the TLS image
     * would be undefined for multiple TLS Regions of the same type
     */
    Region *dataTLS = NULL, *bssTLS = NULL;
    lmap.tlsSize = 0;
    
    vector<Region *> regions;
    if( !target->getAllRegions(regions) ) {
        err = Storage_Allocation_Failure;
        errMsg = "failed to retrieve regions from target";
        return false;
    }
    
    vector<Region *>::iterator reg_it;
    for(reg_it = regions.begin(); reg_it != regions.end(); ++reg_it) {
        if( (*reg_it)->isTLS() ) {
            if( (*reg_it)->getRegionType() == Region::RT_DATA ) {
                if( dataTLS != NULL ) {
                    err = Storage_Allocation_Failure;
                    errMsg = "found more than one TLS data region";
                    return false;
                }
                dataTLS = *reg_it;
                lmap.tlsSize += dataTLS->getRegionSize();
                if( dataTLS->getMemAlignment() > lmap.tlsRegionAlign ) {
                    lmap.tlsRegionAlign = dataTLS->getMemAlignment();
                }
            }else if( (*reg_it)->getRegionType() == Region::RT_BSS ) {
                if( bssTLS != NULL ) {
                    err = Storage_Allocation_Failure;
                    errMsg = "found more than one TLS bss region";
                    return false;
                }
                bssTLS = *reg_it;
                lmap.tlsSize += bssTLS->getRegionSize();
                if( bssTLS->getMemAlignment() > lmap.tlsRegionAlign ) {
                    lmap.tlsRegionAlign = bssTLS->getMemAlignment();
                }
            }
        }
    }

    if( lmap.tlsRegions.size() > 0 ) {
        // Allocate the new TLS region, if necessary
        lmap.tlsRegionOffset = currentOffset;

        lmap.tlsRegionOffset += computePadding(lmap.tlsRegionOffset + globalOffset,
                lmap.tlsRegionAlign);
        currentOffset = layoutTLSImage(globalOffset, dataTLS, bssTLS, lmap);

        if( currentOffset == lmap.tlsRegionOffset ) {
            err = Storage_Allocation_Failure;
            errMsg = "failed to create TLS initialization image";
            return false;
        }

        // The size of this Region is counted twice
        if( dataTLS != NULL ) {
            lmap.tlsSize -= dataTLS->getRegionSize();
        }

        lmap.tlsSize += currentOffset - lmap.tlsRegionOffset;

        // Adjust offsets for all existing TLS symbols, as their offset
        // in the TLS block could have changed
        vector<Symbol *> definedSyms;
        if( target->getAllDefinedSymbols(definedSyms) ) {
            vector<Symbol *>::iterator sym_it;
            for(sym_it = definedSyms.begin(); sym_it != definedSyms.end(); ++sym_it) {
                if( (*sym_it)->getType() == Symbol::ST_THREAD_LOCAL ) {

                    map<Region *, LinkMap::AllocPair>::iterator result;
                    result = lmap.regionAllocs.find((*sym_it)->getRegion());

                    if( result != lmap.regionAllocs.end() ) {
                        Offset regionOffset = result->second.second;
                        Offset symbolOffset = (*sym_it)->getOffset();

                        symbolOffset += regionOffset - lmap.tlsRegionOffset;
                        symbolOffset = adjustTLSOffset(symbolOffset, lmap.tlsSize);

                        (*sym_it)->setOffset(symbolOffset);
                    }
                }
            }
        }

        cleanupTLSRegionOffsets(lmap.regionAllocs, dataTLS, bssTLS);
        if( bssTLS != NULL ) lmap.tlsSize -= bssTLS->getRegionSize();

        hasRewrittenTLS = true;
    }else{
        // Adjust offsets for all existing TLS symbols, as the offsets are
        // architecture dependent
        vector<Symbol *> definedSyms;
        if( target->getAllDefinedSymbols(definedSyms) ) {
            vector<Symbol *>::iterator sym_it;
            for(sym_it = definedSyms.begin(); sym_it != definedSyms.end(); ++sym_it) {
                if( (*sym_it)->getType() == Symbol::ST_THREAD_LOCAL ) {
                    Offset symbolOffset = (*sym_it)->getOffset();
                    symbolOffset = adjustTLSOffset(symbolOffset, lmap.tlsSize);
                    (*sym_it)->setOffset(symbolOffset);
                }
            }
        }

        // The size of the original TLS image is no longer needed
        lmap.tlsSize = 0;
    }

    // Update all relevant symbols with their offsets in the new target
    for(obj_it = dependentObjects.begin(); obj_it != dependentObjects.end(); ++obj_it) {
        vector<Symbol *> definedSyms;
        if( (*obj_it)->getAllDefinedSymbols(definedSyms) ) {
            vector<Symbol *>::iterator sym_it;
            for(sym_it = definedSyms.begin(); sym_it != definedSyms.end(); ++sym_it) {
                if(    !(*sym_it)->isCommonStorage() 
                    && (*sym_it)->getType() != Symbol::ST_THREAD_LOCAL) 
                {
                    map<Region *, LinkMap::AllocPair>::iterator result;
                    result = lmap.regionAllocs.find((*sym_it)->getRegion());
                    if( result != lmap.regionAllocs.end() ) {
                        Offset regionOffset = result->second.second;
                        Offset symbolOffset = (*sym_it)->getOffset();

                        symbolOffset += globalOffset + regionOffset;

                        (*sym_it)->setOffset(symbolOffset);
                    }
                }
            }
        }
    }

    /* ASSUMPTION
     * At this point, the layout of the new regions is fixed, and
     * addresses of all symbols are known
     */

    // Allocate storage space
    lmap.allocatedData = new char[currentOffset];
    lmap.allocatedSize = currentOffset;

    // Copy the Regions from the dependent objects into the new storage space
    copyRegions(lmap);

    return true;
}

/**
 * Lays out the specified regions into the specified storage space
 */
Offset emitElf::layoutRegions(deque<Region *> &regions, 
        map<Region *, LinkMap::AllocPair> &regionAllocs,
        Offset currentOffset, Offset globalOffset) 
{
    Offset retOffset = currentOffset;

    deque<Region *>::iterator copyReg_it;
    for(copyReg_it = regions.begin(); copyReg_it != regions.end(); ++copyReg_it) {
        // Skip empty Regions
        if( (*copyReg_it)->getRegionSize() == 0 ) continue;

        // Make sure the Region is aligned correctly in the new aggregate Region
        Offset padding = computePadding(globalOffset + retOffset, (*copyReg_it)->getMemAlignment());
        retOffset += padding;

        // Set up mapping for a new Region in the specified storage space
        pair<map<Region *, LinkMap::AllocPair>::iterator, bool> result;
        result = regionAllocs.insert(make_pair(*copyReg_it, make_pair(padding, retOffset)));

        // If the map already contains this Region, this is a logic error
        if( !result.second ) retOffset = ~0UL;
                       
        retOffset += (*copyReg_it)->getRegionSize();
    }

    return retOffset;
}

// Add new Regions to the target 
void emitElf::addNewRegions(Symtab *target, Offset globalOffset, LinkMap &lmap) {
    char *newTargetData = lmap.allocatedData;
    
    if( lmap.bssSize > 0 ) {
        target->addRegion(globalOffset + lmap.bssRegionOffset, 
                reinterpret_cast<void *>(&newTargetData[lmap.bssRegionOffset]),
                static_cast<unsigned int>(lmap.bssSize),
                BSS_NAME, Region::RT_DATA, true, lmap.bssRegionAlign);
    }

    if( lmap.codeSize > 0 ) {
        target->addRegion(globalOffset + lmap.codeRegionOffset, 
                reinterpret_cast<void *>(&newTargetData[lmap.codeRegionOffset]),
                static_cast<unsigned int>(lmap.codeSize),
                CODE_NAME, Region::RT_TEXT, true, lmap.codeRegionAlign);
    }

    if( lmap.dataSize > 0 ) {
        target->addRegion(globalOffset + lmap.dataRegionOffset, 
                reinterpret_cast<void *>(&newTargetData[lmap.dataRegionOffset]),
                static_cast<unsigned int>(lmap.dataSize),
                DATA_NAME, Region::RT_DATA, true, lmap.dataRegionAlign);
    }

    if( lmap.gotSize > 0 ) {
        buildGOT(lmap);

        target->addRegion(globalOffset + lmap.gotRegionOffset,
                reinterpret_cast<void *>(&newTargetData[lmap.gotRegionOffset]),
                static_cast<unsigned int>(lmap.gotSize),
                GOT_NAME, Region::RT_DATA, true, lmap.gotRegionAlign);
    }

    // Better to check for existence of TLS Symbols instead of size
    if( lmap.tlsSize > 0 ) {
        target->addRegion(globalOffset + lmap.tlsRegionOffset,
                reinterpret_cast<void *>(&newTargetData[lmap.tlsRegionOffset]),
                static_cast<unsigned int>(lmap.tlsSize),
                TLS_DATA_NAME, Region::RT_DATA, true, lmap.tlsRegionAlign, true);
    }

}

void emitElf::copyRegions(LinkMap &lmap) {
    char *targetData = lmap.allocatedData;

    map<Region *, LinkMap::AllocPair>::iterator regOff_it;
    for(regOff_it = lmap.regionAllocs.begin(); regOff_it != lmap.regionAllocs.end(); ++regOff_it) {
        Region *depRegion = regOff_it->first;
        Offset regionOffset = regOff_it->second.second;
        Offset padding = regOff_it->second.first;

        // Copy in the Region data
        char *rawRegionData = reinterpret_cast<char *>(depRegion->getPtrToRawData());

        // Currently, BSS is expanded
        if( depRegion->isBSS() ) {
            bzero(&targetData[regionOffset], depRegion->getRegionSize());
        }else{
            memcpy(&targetData[regionOffset], rawRegionData, depRegion->getRegionSize());
        }

        // Set the padded space to a meaningful value
        memset(&targetData[regionOffset - padding], getPaddingValue(depRegion->getRegionType()), padding);
    }
}

inline
Offset emitElf::computePadding(Offset candidateOffset, Offset alignment) {
    Offset padding = 0;
    if( alignment != 0 && (candidateOffset % alignment) != 0 ) {
        padding = alignment - (candidateOffset % alignment);
    }
    return padding;
}

/**
 * Given a collection of newly allocated regions in the specified storage space, 
 * computes relocations and places the values at the location specified by the
 * relocation entry (stored with the Regions)
 */
bool emitElf::applyRelocations(Symtab *target, vector<Symtab *> &dependentObjects,
        Offset globalOffset, LinkMap &lmap,
        StaticLinkError &err, string &errMsg) 
{
    // Iterate over all relocations in all .o's
    vector<Symtab *>::iterator depObj_it;
    for(depObj_it = dependentObjects.begin(); depObj_it != dependentObjects.end(); ++depObj_it) {
        vector<Region *> allRegions;
        (*depObj_it)->getAllRegions(allRegions);

        rewrite_printf("\n*** Computing relocations for object: %s\n\n",
                (*depObj_it)->memberName().c_str());

        // Relocations are stored with the Region to which they will be applied
        // As an ELF example, .rel.text relocations are stored with the Region .text
        vector<Region *>::iterator region_it;
        for(region_it = allRegions.begin(); region_it != allRegions.end(); ++region_it) {
            vector<relocationEntry>::iterator rel_it;
            for(rel_it = (*region_it)->getRelocations().begin();
                rel_it != (*region_it)->getRelocations().end();
                ++rel_it)
            {
                // Only compute relocations for the new Regions
                map<Region *, LinkMap::AllocPair>::iterator result;
                result = lmap.regionAllocs.find(*region_it);
                if( result != lmap.regionAllocs.end() ) {
                    Offset regionOffset = result->second.second;

                    // Compute destination of relocation
                    Offset dest = regionOffset + rel_it->rel_addr();

                    Offset relOffset = globalOffset + dest;

                    char *targetData = lmap.allocatedData;
                    if( !archSpecificRelocation(targetData, *rel_it, dest, 
                                relOffset, globalOffset, lmap, errMsg) ) 
                    {
                        err = Relocation_Computation_Failure;
                        errMsg = "Failed to compute relocation: " + errMsg;
                        return false;
                    }
                }
            }
        }
    }

    rewrite_printf("\n*** Computing relocations added to target.\n\n");

    // Compute relocations added to static target 
    vector<Region *> allRegions;
    target->getAllRegions(allRegions);

    vector<Region *>::iterator reg_it;
    for(reg_it = allRegions.begin(); reg_it != allRegions.end(); ++reg_it) {
        char *regionData = reinterpret_cast<char *>((*reg_it)->getPtrToRawData());
        
        vector<relocationEntry>::iterator rel_it;
        for(rel_it = (*reg_it)->getRelocations().begin();
            rel_it != (*reg_it)->getRelocations().end();
            ++rel_it)
        {
            if( !archSpecificRelocation(regionData, *rel_it,
                        rel_it->rel_addr() - (*reg_it)->getRegionAddr(),
                        rel_it->rel_addr(), globalOffset, lmap, errMsg) )
            {
                err = Relocation_Computation_Failure;
                errMsg = "Failed to compute relocation: " + errMsg;
                return false;
            }
        }
    }

    return true;
}

string emitElf::printStaticLinkError(StaticLinkError err) {
    switch(err) {
        CASE_RETURN_STR(No_Static_Link_Error);
        CASE_RETURN_STR(Symbol_Resolution_Failure);
        CASE_RETURN_STR(Relocation_Computation_Failure);
        CASE_RETURN_STR(Storage_Allocation_Failure);
        default:
            return "unknown error";
    }
}

/** The following functions are all somewhat architecture-specific */

/* TLS Info
 *
 * TLS handling is pseudo-architecture dependent. The implementation of the TLS
 * functions depend on the implementation of TLS on a specific architecture.
 *
 * The following material comes from the "ELF Handling For TLS" white paper.
 * According to this paper, their are two variants w.r.t. creating a TLS
 * initialization image.
 *
 * ======================
 *
 * The first is:
 *
 *            beginning of image
 *            |
 *            V                              high address
 * +----+-----+----------+---------+---------+
 * |    | TCB | image 1  | image 2 | image 3 |
 * +----+---- +----------+---------+---------+
 *
 * where TCB = thread control block, and each image is the
 * TLS initialization image for a module (in this context an executable or 
 * shared library).
 *
 * ========================
 *
 * The second is:
 * 
 * beginning of image
 * | 
 * V                                        high address
 * +---------+----------+---------+---------+
 * | image 3 | image 2  | image 1 |  TCB    |
 * +---------+----------+---------+---------+
 *
 * At least on x86 (TODO for other architectures), an image is:
 *
 * +--------+--------+
 * | DATA   |  BSS   |
 * +--------+--------+
 *
 * ==========================
 * 
 * These are the two variants one would see when working with ELF files. So, an
 * architecture either uses variant 1 or 2.
 */

Offset emitElf::tlsLayoutVariant1(Offset globalOffset, Region *dataTLS, Region *bssTLS,
        LinkMap &lmap)
{
    // The original init. image needs to remain in the image 1 slot because
    // the TLS data references are relative to that position
    unsigned long tlsBssSize = 0;
    if( dataTLS != NULL ) lmap.tlsRegions.push_back(dataTLS);
    if( bssTLS != NULL ) tlsBssSize = bssTLS->getRegionSize();

    // Create the image, note new BSS regions are expanded
    Offset endOffset = layoutRegions(lmap.tlsRegions,
            lmap.regionAllocs, lmap.tlsRegionOffset, globalOffset);
    if( endOffset == ~0UL ) return lmap.tlsRegionOffset;
    Offset adjustedEnd = endOffset - lmap.tlsRegionOffset;

    // This is necessary so the offsets of existing TLS symbols can be updated
    // in a uniform, architecture independent way
    if( bssTLS != NULL ) {
        if( dataTLS != NULL ) {
            lmap.regionAllocs.insert(make_pair(bssTLS, lmap.regionAllocs[dataTLS]));
        }else{
            lmap.regionAllocs.insert(make_pair(bssTLS, make_pair(1, endOffset)));
        }
    }

    // Update the symbols with their offsets relative to the TCB address
    vector<Symbol *>::iterator sym_it;
    for(sym_it = lmap.tlsSymbols.begin(); sym_it != lmap.tlsSymbols.end(); ++sym_it) {
        map<Region *, LinkMap::AllocPair>::iterator result;
        result = lmap.regionAllocs.find((*sym_it)->getRegion());

        // It is a programming error if the region for the symbol
        // was not passed to this function
        if( result == lmap.regionAllocs.end() ) {
            endOffset = lmap.tlsRegionOffset;
            break;
        }

        Offset regionOffset = result->second.second;
        Offset symbolOffset = (*sym_it)->getOffset();
        symbolOffset += (regionOffset - lmap.tlsRegionOffset) - (adjustedEnd + tlsBssSize);
        (*sym_it)->setOffset(symbolOffset);
    }

    return endOffset;
}

Offset emitElf::tlsLayoutVariant2(Offset, Region *, Region *, LinkMap &)
{
    assert(!"Layout of TLS initialization image, Variant 2, currently not implemented.");
    return 0;
}

// Note: Variant 2 does not require any modifications, so a separate
// function is not necessary
Offset emitElf::tlsAdjustVariant1(Offset curOffset, Offset tlsSize) {
    // For Variant 1, offsets relative to the TCB need to be negative
    Offset retOffset = curOffset;
    retOffset -= tlsSize;
    return retOffset;
}

void emitElf::tlsCleanupVariant1(map<Region *, LinkMap::AllocPair> &regionAllocs,
        Region *, Region *bssTLS) 
{
    // Existing BSS TLS region is not copied to the new target data
    if( bssTLS != NULL ) regionAllocs.erase(bssTLS);
}

void emitElf::tlsCleanupVariant2(map<Region *, LinkMap::AllocPair> &,
        Region *, Region *) 
{
    assert(!"Cleanup of TLS Regions from allocation map currently unimplemented");
}

/* START LinkMap functions */

ostream & operator<<(ostream &os, LinkMap &lm) {
     lm.printAll(os, 0);
     return os;
}

void LinkMap::printAll(ostream &os, Offset globalOffset) {
    os << "Size of allocated space = 0x" << hex << allocatedSize << dec << endl;
    if( bssRegions.size() > 0 ) {
        os << "New BSS Region: Offset: 0x" << hex << (globalOffset + bssRegionOffset) << dec
           << " Size: 0x" << hex << bssSize << dec 
           << " Alignment: 0x" << hex << bssRegionAlign << dec
           << endl;
        printRegions(os, bssRegions, globalOffset);
        os << endl;
    }

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
        os << "\tRegion: " << region->getRegionName() 
           << " Padding: 0x" << hex << pair.first << dec
           << " Offset: 0x" << hex << (pair.second + globalOffset) << dec
           << " - 0x" << hex << (pair.second + globalOffset + region->getRegionSize() - 1) << dec
           << " Size: 0x" << hex << region->getRegionSize() << dec
           << " Alignment: 0x" << hex << region->getMemAlignment() << dec
           << endl;
    }
}
