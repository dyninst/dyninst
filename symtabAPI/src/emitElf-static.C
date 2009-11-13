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

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

static const std::string CODE_NAME = ".dyninstCode";
static const std::string DATA_NAME = ".dyninstData";
static const std::string BSS_NAME = ".dyninstBss";
static const std::string GOT_NAME = ".dyninstGot";
static const std::string TLS_DATA_NAME = ".tdata";
static const std::string DEFAULT_BSS_NAME = ".bss";

/*
 * Most of these functions take a reference to a 
 * StaticLinkError and a string for error reporting 
 * purposes. These should prove useful in identifying
 * the cause of an error
 */

//TODO don't use std::map<>.count function

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
        StaticLinkError &err, std::string &errMsg) 
{
    // Basic algorithm is:
    // * locate defined versions of symbols for undefined symbols in 
    //   either instrumentation or dependent code, produces a vector
    //   of Symtab objects that contain the defined versions
    // * Allocate storage for all new code and data to be linked into 
    //   the target, produces a map of old regions to their new offsets
    //   in the target
    // * Compute relocations and set their targets to these values. 
    // * The passed symtab object will reflect all these changes

    // Errors are set by the methods

    // Holds all necessary dependencies, as determined by esolveSymbols
    std::vector<Symtab *> dependentObjects;
    if( !resolveSymbols(target, dependentObjects, err, errMsg) ) {
        return NULL;
    }

    // Determine starting location of new Regions
    Offset globalOffset = 0;
    std::vector<Region *> newRegs;
    if( target->getAllNewRegions(newRegs) ) {
        // This is true, only if other regions have already been added
        std::vector<Region *>::iterator newReg_it;
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
        return NULL;
    }

    // Holds location information for Regions copied into the static executable
    std::map<Region *, Offset> regionOffsets;

    // Holds location information for Symbols placed in the GOT
    std::map<Symbol *, Offset> gotOffsets;
    Offset gotOffset;
    
    char *rawDependencyData = allocateStorage(target, dependentObjects, 
            regionOffsets, globalOffset, gotOffsets, gotOffset, err, errMsg);

    if( rawDependencyData == NULL ) {
        return NULL;
    }

    if( rawDependencyData && !applyRelocations(rawDependencyData, dependentObjects, 
                regionOffsets, gotOffsets, globalOffset, gotOffset, target, err, errMsg) ) 
    {
        delete rawDependencyData;
        return NULL;
    }

    /* Originally, this code reverted all changes made to Symbols' offset field
     * This was required because Symtab caches objects, and future references to
     * this Symtab would see these changes. However, there is going to be 
     * functionality to mark a Symtab as dirty and that would be the preferred
     * solution to this problem

    // Restore original offsets for all defined symbols in all dependent objects
    std::vector<Symtab *>::iterator obj_it;
    for(obj_it = dependentObjects.begin(); obj_it != dependentObjects.end(); ++obj_it) {
        std::vector<Symbol *> definedSyms;
        if( (*obj_it)->getAllDefinedSymbols(definedSyms) ) {
            std::vector<Symbol *>::iterator sym_it;
            for(sym_it = definedSyms.begin(); sym_it != definedSyms.end(); ++sym_it) {
                if(    regionOffsets.count((*sym_it)->getRegion())
                    && !(*sym_it)->isCommonStorage() ) 
                {
                    Offset symbolOffset = (*sym_it)->getOffset();
                    symbolOffset -= globalOffset + regionOffsets[(*sym_it)->getRegion()];
                    (*sym_it)->setOffset(symbolOffset);
                }
            }
        }
    }

    // Restore alignments for common storage symbols
    std::multimap<Offset, Symbol *>::iterator align_it;
    for(align_it = commonAlignments.begin(); 
        align_it != commonAlignments.end(); ++align_it) 
    {
        align_it->second->setOffset(align_it->first);
    }
    */

    fprintf(stderr, "\n*** Finished static linking\n\n");

    err = No_Static_Link_Error;
    errMsg = "";
    return rawDependencyData;
}

/**
 * Resolves undefined symbols in the specified Symtab object, usually due
 * to the addition of new Symbols to the Symtab object
 *
 * @param target - the Symtab object being rewritten
 *
 * @param dependentObjects - on success, will hold all Symtab objects which
 * were used to resolve symbols i.e. they need to be copied into the target
 * 
 * @return true, if sucessful, false, otherwise
 */
bool emitElf::resolveSymbols(Symtab *target, 
        std::vector<Symtab *> &dependentObjects,
        StaticLinkError &err, std::string &errMsg) 
{
    // Collection of objects that currently need their symbols resolved
    std::set<Symtab *> workSet;

    // Collection of objects that have already had their symbols resolved
    // this is necessary to avoid errors related to circular dependencies
    std::set<Symtab *> linkedSet;

    std::set<std::string> excludeSymNames;
    getExcludedSymbolNames(excludeSymNames);

    // Add all object files explicitly referenced by new symbols, these
    // essentially fuel the linking process
    std::vector<Symtab *> explicitRefs;
    target->getExplicitSymtabRefs(explicitRefs);

    std::vector<Symtab *>::iterator expObj_it;
    for(expObj_it = explicitRefs.begin(); expObj_it != explicitRefs.end();
            ++expObj_it) 
    {
        dependentObjects.push_back(*expObj_it);
        workSet.insert(*expObj_it);
        linkedSet.insert(*expObj_it);
    }

    // Establish list of libraries to search for symbols
    std::vector<Archive *> libraries;
    target->getLinkingResources(libraries);

    std::set<Symtab *>::iterator curObjFilePtr = workSet.begin();
    while( curObjFilePtr != workSet.end() ) {
        // Take an object file off the working set
        Symtab *curObjFile = *curObjFilePtr;
        workSet.erase(curObjFile);

        fprintf(stderr, "\n*** Resolving symbols for object: %s\n\n",
                curObjFile->memberName().c_str());

        // Build the map of Symbols to relocations
        // Also, construct collection of Symbols to be placed in GOT
        std::map<Symbol *, std::vector<relocationEntry *> > symToRels;
        std::vector<Region *> allRegions;
        curObjFile->getAllRegions(allRegions);

        std::vector<Region *>::iterator region_it;
        for(region_it = allRegions.begin(); region_it != allRegions.end();
                ++region_it)
        {
            std::vector<relocationEntry> &region_rels = (*region_it)->getRelocations();
            std::vector<relocationEntry>::iterator rel_it;
            for( rel_it = region_rels.begin(); rel_it != region_rels.end(); ++rel_it) {
                symToRels[rel_it->getDynSym()].push_back(&(*rel_it));
            }
        }

        // Iterate over all undefined symbols, attempting to resolve them
        std::vector<Symbol *> undefSyms;
        curObjFile->getAllUndefinedSymbols(undefSyms);

        std::vector<Symbol *>::iterator undefSym_it;
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
                 std::vector<Symbol *> foundSyms;
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

                    fprintf(stderr, "Found external symbol %s in target with address = 0x%lx\n",
                            extSymbol->getPrettyName().c_str(), extSymbol->getOffset());
                 }
            }

            if( extSymbol == NULL ) {
                //search loaded libraries and system libraries and add
                //any new object files as dependent objects
                std::vector<Archive *>::iterator lib_it;
                Symtab *containingSymtab = NULL;
                Archive *containingArchive = NULL;
                for(lib_it = libraries.begin(); lib_it != libraries.end(); ++lib_it) {
                    Symtab *tmpSymtab;
                    if( (*lib_it)->getMemberByGlobalSymbol(tmpSymtab, 
                                const_cast<std::string&>(curUndefSym->getMangledName())) ) 
                    {
                        /** A common approach is to used the symbol that is defined first
                         *  in the list of libraries (in the order specified to the linker)
                         *
                         *  I am choosing to mirror that behavior here.
                        if( containingSymtab != NULL ) {
                            err = Symbol_Resolution_Failure;
                            errMsg = "symbol defined in multiple libraries: " +
                                curUndefSym->getMangledName();
                            return false;
                        }
                        */

                        containingArchive = *lib_it;
                        containingSymtab = tmpSymtab;
                        break;
                    }else{
                        // gcc/ld appear to ignore this error and just choose the
                        // symbol that occurs first in the Archive. This could 
                        // produce unexpected results so it is better to alert
                        // the user of this error
                        if( Archive::getLastError() == Duplicate_Symbol ) {
                            err = Symbol_Resolution_Failure;
                            errMsg = Archive::printError(Duplicate_Symbol);
                            return false;
                        }
                    }
                }

                if( containingSymtab != NULL ) {
                    std::vector<Symbol *> foundSyms;
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

                        fprintf(stderr, "Found external symbol %s in module %s(%s)\n",
                                extSymbol->getPrettyName().c_str(),
                                containingArchive->name().c_str(),
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

            assert(extSymbol != NULL);

            // Store the found symbol with the related relocations
            std::map<Symbol *, std::vector<relocationEntry *> >::iterator relMap_it;
            relMap_it = symToRels.find(curUndefSym);
            if( relMap_it != symToRels.end() ) {
                std::vector<relocationEntry *>::iterator rel_it;
                for(rel_it = relMap_it->second.begin(); rel_it != relMap_it->second.end();
                        ++rel_it)
                {
                    (*rel_it)->addDynSym(extSymbol);
                }
            }else{
                /* This really shouldn't be a warning, some libraries define a reference
                 * to a symbol and then never use it in order to ensure that the module that
                 * defines the symbol is linked 
                 *
                fprintf(stderr, 
                        "WARNING: LINK ERROR: failed to find relocation for undefined symbol '%s'.\n",
                        curUndefSym->getPrettyName().c_str());
                */
            }
        }

        curObjFilePtr = workSet.begin();
    }
    
    return true;
}

/**
 * Given a collection of Symtab objects, creates new Regions for 
 * code, data and bss in the specified, target Symtab object.
 * 
 * Additionally, allocates COMMON symbols in the collect of Symtab objects
 * as bss.
 *
 * @param target - the Symtab object being rewritten
 *
 * @param dependentObjects - the Symtab objects that need to be copied
 * into the target
 *
 * @param regionOffset - populated by this function, this map will provide
 * the offsets of Regions in the dependentObjects in the new storage space
 *
 * @param globalOffset - the Offset in the target where new code will be
 * placed
 */
char *emitElf::allocateStorage(Symtab *target,
        std::vector<Symtab *> & dependentObjects, 
        std::map<Region *, Offset> & regionOffsets, 
        Offset globalOffset, 
        std::map<Symbol *, Offset> &gotOffsets,
        Offset & gotOffset,
        StaticLinkError &err, std::string &errMsg) 
{
    fprintf(stderr, "\n*** Allocating storage for new objects\n\n");

    fprintf(stderr, "globalOffset = 0x%lx\n", globalOffset);

    std::deque<Region *> codeRegions;
    std::deque<Region *> dataRegions;
    std::deque<Region *> bssRegions;
    std::deque<Region *> tlsRegions;

    // Create a temporary region for COMMON storage
    Region *commonStorage;

    // Used to create a new COMMON block
    std::multimap<Offset, Symbol *> commonAlignments;

    // Create a collection of TLS symbols
    std::vector<Symbol *> tlsSyms;

    // Create a collection of GOT symbols
    std::set<Symbol *> gotSymbols;

    std::vector<Symtab *>::iterator obj_it;
    for(obj_it = dependentObjects.begin(); obj_it != dependentObjects.end(); ++obj_it) {
        std::vector<Region *> regs;
        if( !(*obj_it)->getAllRegions(regs) ) {
            err = Storage_Allocation_Failure;
            errMsg = "failed to locate regions in dependent object";
            return NULL;
        }

        std::vector<Region *>::iterator reg_it;
        for(reg_it = regs.begin(); reg_it != regs.end(); ++reg_it) {
            if( (*reg_it)->isLoadable() ) {
                if( (*reg_it)->isTLS() ) {
                    switch((*reg_it)->getRegionType()) {
                        case Region::RT_DATA:
                        case Region::RT_BSS:
                            tlsRegions.push_back(*reg_it);
                            break;
                        default:
                            // ignore any other regions
                            break;
                    }
                }else{
                    switch((*reg_it)->getRegionType()) {
                        case Region::RT_TEXT:
                            codeRegions.push_back(*reg_it);
                            break;
                        case Region::RT_DATA:
                            dataRegions.push_back(*reg_it);
                            break;
                        case Region::RT_BSS:
                            bssRegions.push_back(*reg_it);
                            break;
                        case Region::RT_TEXTDATA:
                            codeRegions.push_back(*reg_it);
                            break;
                        default:
                            // skip other regions
                            continue;
                    }
                }
            }

            // Find symbols that need to be put in the GOT
            std::vector<relocationEntry> &region_rels = (*reg_it)->getRelocations();
            std::vector<relocationEntry>::iterator rel_it;
            for( rel_it = region_rels.begin(); rel_it != region_rels.end(); ++rel_it) {
                if( isGOTRelocation(rel_it->getRelType()) ) {
                    gotSymbols.insert(rel_it->getDynSym());
                }
            }

        }

        std::vector<Symbol *> definedSyms;
        if( (*obj_it)->getAllDefinedSymbols(definedSyms) ) {
            std::vector<Symbol *>::iterator sym_it;
            for(sym_it = definedSyms.begin(); sym_it != definedSyms.end(); ++sym_it) {
                // Note: the assumption is made that a Symbol cannot 
                // be both TLS and COMMON
                if( (*sym_it)->isCommonStorage() ) {
                    // For a symbol in common storage, the offset/value is the alignment
                    commonAlignments.insert(make_pair((*sym_it)->getOffset(), *sym_it));
                }else if( (*sym_it)->getType() == Symbol::ST_THREAD_LOCAL ) {
                    tlsSyms.push_back(*sym_it);
                }
            }
        }
    }

    /* START common block processing */

    // Combine all common symbols into a single block

    // The following approach is greedy and suboptimal
    // (in terms of space in the target), but it's quick...
    Offset commonOffset = globalOffset;
    std::multimap<Offset, Symbol *>::iterator align_it;
    for(align_it = commonAlignments.begin(); 
        align_it != commonAlignments.end(); ++align_it)
    {
        commonOffset += computePadding(commonOffset, align_it->first);

        // Update symbol with place in new linked code
        align_it->second->setOffset(commonOffset);
        commonOffset += align_it->second->getSize();
    }

    // Update the size of common storage
    if( commonAlignments.size() > 0 ) {
        // A COMMON region is not really complete
        Region::createRegion(commonStorage, 0, Region::RP_RW,
                Region::RT_BSS, commonOffset - globalOffset, 0, commonOffset - globalOffset,
                DEFAULT_BSS_NAME, NULL, true, false, computeAlignment(globalOffset));
        bssRegions.push_front(commonStorage);
    }

    /* END common block processing */

    std::map<Region *, Offset> paddingMap;

    // Determine how all the Regions in the dependent objects will be
    // allocated
    Offset currentOffset = 0;

    // Allocate bss regions 
    Offset bssRegionOffset = currentOffset;
    currentOffset = allocateRegions(bssRegions, regionOffsets,
            paddingMap, bssRegionOffset, globalOffset);

    // Allocate code regions 
    Offset codeRegionOffset = currentOffset;
    currentOffset = allocateRegions(codeRegions, regionOffsets, 
            paddingMap, codeRegionOffset, globalOffset);

    // Allocate data regions 
    Offset dataRegionOffset = currentOffset;
    currentOffset = allocateRegions(dataRegions, regionOffsets, 
            paddingMap, dataRegionOffset, globalOffset);

    // Allocate the new TLS region, if necessary
    Offset tlsImageOffset = currentOffset;

    // Find current TLS Regions in target, also check for multiple
    // TLS Regions of the same type => how to construct the TLS image
    // would be undefined for multiple TLS Regions of the same type
    Region *dataTLS = NULL, *bssTLS = NULL;
    Offset tlsSize = 0;
    
    std::vector<Region *> regions;
    if( !target->getAllRegions(regions) ) {
        err = Storage_Allocation_Failure;
        errMsg = "failed to retrieve regions from target";
        return NULL;
    }
    
    std::vector<Region *>::iterator reg_it;
    for(reg_it = regions.begin(); reg_it != regions.end(); ++reg_it) {
        if( (*reg_it)->isTLS() ) {
            if( (*reg_it)->getRegionType() == Region::RT_DATA ) {
                if( dataTLS != NULL ) {
                    err = Storage_Allocation_Failure;
                    errMsg = "found more than one TLS data region";
                    return NULL;
                }
                dataTLS = *reg_it;
                tlsSize += dataTLS->getRegionSize();
            }else if( (*reg_it)->getRegionType() == Region::RT_BSS ) {
                if( bssTLS != NULL ) {
                    err = Storage_Allocation_Failure;
                    errMsg = "found more than one TLS bss region";
                    return NULL;
                }
                bssTLS = *reg_it;
                tlsSize += bssTLS->getRegionSize();
            }
        }
    }

    if( tlsRegions.size() > 0 ) {
        fprintf(stderr, "\n*** Creating new TLS image\n\n");

        currentOffset = allocateTLSImage(tlsImageOffset, globalOffset,
                regionOffsets, paddingMap, tlsRegions, dataTLS, bssTLS, tlsSyms);

        if( currentOffset == tlsImageOffset ) {
            err = Storage_Allocation_Failure;
            errMsg = "failed to create TLS initialization image";
            return NULL;
        }

        tlsSize += currentOffset - tlsImageOffset;

        // This size is included twice (once by allocateTLS and once
        // in the previous code block)
        if( dataTLS != NULL ) {
            tlsSize -= dataTLS->getRegionSize();
        }

        fprintf(stderr, "TLS Size = %lx\n", tlsSize);

        // Adjust offsets for all existing TLS symbols, as their offset
        // in the TLS block could have changed
        std::vector<Symbol *> definedSyms;
        if( target->getAllDefinedSymbols(definedSyms) ) {
            std::vector<Symbol *>::iterator sym_it;
            for(sym_it = definedSyms.begin(); sym_it != definedSyms.end(); ++sym_it) {
                if( (*sym_it)->getType() == Symbol::ST_THREAD_LOCAL &&
                    regionOffsets.count((*sym_it)->getRegion()) )
                {
                    Offset symbolOffset = (*sym_it)->getOffset();
                    symbolOffset += regionOffsets[(*sym_it)->getRegion()] - tlsImageOffset;
                    symbolOffset = adjustTLSOffset(symbolOffset, tlsSize);
                    (*sym_it)->setOffset(symbolOffset);
                }
            }
        }

        cleanupTLSRegionOffsets(regionOffsets, dataTLS, bssTLS);

        hasRewrittenTLS = true;
    }else{
        // Adjust offsets for all existing TLS symbols, as the offsets are
        // architecture dependent
        std::vector<Symbol *> definedSyms;
        if( target->getAllDefinedSymbols(definedSyms) ) {
            std::vector<Symbol *>::iterator sym_it;
            for(sym_it = definedSyms.begin(); sym_it != definedSyms.end(); ++sym_it) {
                if(    regionOffsets.count((*sym_it)->getRegion())
                    && (*sym_it)->getType() == Symbol::ST_THREAD_LOCAL ) 
                {
                    Offset symbolOffset = (*sym_it)->getOffset();
                    symbolOffset = adjustTLSOffset(symbolOffset, tlsSize);
                    (*sym_it)->setOffset(symbolOffset);
                }
            }
        }
    }

    // Update all relevant symbols with their offsets in the new target
    for(obj_it = dependentObjects.begin(); obj_it != dependentObjects.end(); ++obj_it) {
        std::vector<Symbol *> definedSyms;
        if( (*obj_it)->getAllDefinedSymbols(definedSyms) ) {
            std::vector<Symbol *>::iterator sym_it;
            for(sym_it = definedSyms.begin(); sym_it != definedSyms.end(); ++sym_it) {
                if(    regionOffsets.count((*sym_it)->getRegion())
                    && !(*sym_it)->isCommonStorage()
                    && (*sym_it)->getType() != Symbol::ST_THREAD_LOCAL) 
                {
                    Offset symbolOffset = (*sym_it)->getOffset();
                    symbolOffset += globalOffset + regionOffsets[(*sym_it)->getRegion()];
                    (*sym_it)->setOffset(symbolOffset);
                }
            }
        }
    }

    // Allocate space for a GOT Region, if necessary
    gotOffset = currentOffset;
    currentOffset += getGOTSize(gotSymbols);

    // At this point, the layout of the new regions is fixed, and
    // addresses of all symbols are known

    // Allocate storage space
    fprintf(stderr, "Size of allocated space = %lx\n", currentOffset);
    char *newTargetData = new char[currentOffset];

    // Copy the Regions from the dependent objects into the new storage space
    copyRegions(newTargetData, regionOffsets, paddingMap);

    // For debugging purposes, lists the location of every Region, organized by
    // parent Symtab
    for(obj_it = dependentObjects.begin(); obj_it != dependentObjects.end(); ++obj_it) {
        fprintf(stderr, "\nExamining object: %s\n", (*obj_it)->memberName().c_str());
        std::vector<Region *> regs;
        if( !(*obj_it)->getAllRegions(regs) ) {
            break;
        }

        std::vector<Region *>::iterator reg_it;
        for(reg_it = regs.begin(); reg_it != regs.end(); ++reg_it) {
            std::map<Region *, Offset>::iterator result_it;
            result_it = regionOffsets.find((*reg_it));
            if( result_it != regionOffsets.end() ) {
                fprintf(stderr, "Region %s placed at 0x%lx, size = 0x%lx, alignment = 0x%lx\n", 
                        (*reg_it)->getRegionName().c_str(),
                        result_it->second + globalOffset, (*reg_it)->getRegionSize(), (*reg_it)->getMemAlignment());
            }
        }
    }

    // Build a new GOT, if necessary
    if( gotSymbols.size() > 0 ) {
        buildGOT(&newTargetData[gotOffset], gotOffsets, gotSymbols);
    }

    // Add new Regions to the target
    Offset bssRegionAlign = computeAlignment(globalOffset + bssRegionOffset);
    target->addRegion(globalOffset + bssRegionOffset, 
            reinterpret_cast<void *>(&newTargetData[bssRegionOffset]),
            static_cast<unsigned int>(codeRegionOffset - bssRegionOffset),
            BSS_NAME, Region::RT_DATA, true, bssRegionAlign);
    fprintf(stderr, "\nPlaced region %s @ 0x%lx, size = 0x%lx, alignment = 0x%lx\n", BSS_NAME.c_str(),
            (globalOffset + bssRegionOffset), (codeRegionOffset - bssRegionOffset),
            bssRegionAlign);

    Offset codeRegionAlign = computeAlignment(globalOffset + codeRegionOffset);
    target->addRegion(globalOffset + codeRegionOffset, 
            reinterpret_cast<void *>(&newTargetData[codeRegionOffset]),
            static_cast<unsigned int>(dataRegionOffset - codeRegionOffset),
            CODE_NAME, Region::RT_TEXT, true, codeRegionAlign);
    fprintf(stderr, "Placed region %s @ 0x%lx, size = 0x%lx, alignment = 0x%lx\n", CODE_NAME.c_str(),
            (globalOffset + codeRegionOffset), (dataRegionOffset - codeRegionOffset),
            codeRegionAlign);

    Offset dataRegionAlign = computeAlignment(globalOffset + dataRegionOffset);
    target->addRegion(globalOffset + dataRegionOffset, 
            reinterpret_cast<void *>(&newTargetData[dataRegionOffset]),
            static_cast<unsigned int>(tlsImageOffset - dataRegionOffset),
            DATA_NAME, Region::RT_DATA, true, dataRegionAlign);
    fprintf(stderr, "Placed region %s @ 0x%lx, size = 0x%lx, alignment = 0x%lx\n", DATA_NAME.c_str(),
            (globalOffset + dataRegionOffset), (tlsImageOffset - dataRegionOffset),
            dataRegionAlign);

    if( tlsRegions.size() > 0 ) {
        Offset tlsRegionAlign = computeAlignment(globalOffset + tlsImageOffset);
        target->addRegion(globalOffset + tlsImageOffset,
                reinterpret_cast<void *>(&newTargetData[tlsImageOffset]),
                static_cast<unsigned int>(gotOffset - tlsImageOffset),
                TLS_DATA_NAME, Region::RT_DATA, true, tlsRegionAlign, true);
        fprintf(stderr, "Placed region %s @ 0x%lx, size = 0x%lx, alignment = 0x%lx\n", TLS_DATA_NAME.c_str(),
                (globalOffset + tlsImageOffset), (gotOffset - tlsImageOffset),
                tlsRegionAlign);
    }

    if( gotSymbols.size() > 0 ) {
        Offset gotRegionAlign = computeAlignment(globalOffset + gotOffset);
        target->addRegion(globalOffset + gotOffset,
                reinterpret_cast<void *>(&newTargetData[gotOffset]),
                static_cast<unsigned int>(currentOffset - gotOffset),
                GOT_NAME, Region::RT_DATA, true, gotRegionAlign);
        fprintf(stderr, "Placed region %s @ 0x%lx, size = 0x%lx, alignment = 0x%lx\n", GOT_NAME.c_str(),
            (globalOffset + gotOffset), (currentOffset - gotOffset),
            gotRegionAlign);
    }

    return newTargetData;
}

/**
 * Copies the specified regions into the specified storage space
 *
 * @param targetData - the Regions are copied into this storage space
 * 
 * @param regions - the Regions to copy
 *
 * @param regionOffsets - populated by this function, a mapping of the Regions to 
 * their place in the specified storage space
 *
 * @param currentOffset - the starting Offset to place the specified Regions
 *
 * @return the offset at which the next set of regions should be placed
 */
Offset emitElf::allocateRegions(std::deque<Region *> &regions, 
        std::map<Region *, Offset> &regionOffsets, 
        std::map<Region *, Offset> &paddingMap, Offset currentOffset, Offset globalOffset) 
{
    Offset retOffset = currentOffset;

    std::deque<Region *>::iterator copyReg_it;
    for(copyReg_it = regions.begin(); copyReg_it != regions.end(); ++copyReg_it) {
        // Skip empty Regions
        if( (*copyReg_it)->getRegionSize() == 0 ) continue;

        // Make sure the Region is aligned correctly in the new aggregate Region
        Offset padding = computePadding(globalOffset + retOffset, (*copyReg_it)->getMemAlignment());
        retOffset += padding;

        // Store padding for later use
        std::pair<std::map<Region *, Offset>::iterator, bool> result;
        result = paddingMap.insert(std::make_pair(*copyReg_it, padding));

        // If the map already contains this Region, this is a logic error
        assert(result.second);

        // Set up mapping for a new Region in the specified storage space
        result = regionOffsets.insert(std::make_pair(*copyReg_it, retOffset));

        // If the map already contains this Region, this is a logic error
        assert(result.second);
                       
        retOffset += (*copyReg_it)->getRegionSize();
    }

    return retOffset;
}

void emitElf::copyRegions(char *targetData, std::map<Region *, Offset> &regionOffsets,
        std::map<Region *, Offset> &paddingMap) 
{
    std::map<Region *, Offset>::iterator regOff_it;
    for(regOff_it = regionOffsets.begin(); regOff_it != regionOffsets.end(); ++regOff_it) {
        Offset regionOffset = regOff_it->second;
        Region *depRegion = regOff_it->first;

        // Copy in the Region data
        char *rawRegionData = reinterpret_cast<char *>(depRegion->getPtrToRawData());

        // Currently, BSS is expanded
        if( depRegion->isBSS() ) {
            bzero(&targetData[regionOffset], depRegion->getRegionSize());
        }else{
            memcpy(&targetData[regionOffset], rawRegionData, depRegion->getRegionSize());
        }

        // Set the padded space to a meaningful value
        std::map<Region *, Offset>::iterator padding_it = paddingMap.find(depRegion);
        if( padding_it != paddingMap.end() && padding_it->second != 0) {
            memset(&targetData[regionOffset - padding_it->second], getPaddingValue(depRegion->getRegionType()), padding_it->second);
        }
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

Offset emitElf::computeAlignment(Offset location) {
    // The alignment is determined by the least significant
    // bit in the location that is set to 1
    // alignment of 0x00c = 4
    // alignment of 0x005 = 1
    const unsigned BYTES_TO_BITS = 8;
    Offset alignment = 1;
    for(unsigned i = 0; i < sizeof(Offset)*BYTES_TO_BITS; i++) {
        if( alignment & location ) break;
        alignment = alignment << 1;
    }
    return alignment;
}

/**
 * Given a collection of newly allocated regions in the specified storage space, 
 * computes relocations and places the values at the location specified by the
 * relocation entry (stored with the Regions)
 *
 * @param targetData - the storage space for the newly allocated Regions
 *
 * @param dependentObjects - the Symtab objects whose select Regions have been copied
 * into the targetData buffer
 *
 * @param regionOffsets - a mapping of Regions in dependentObjects to there offset in
 * the targetData buffer
 *
 * @param globalOffset - the Offset at which targetData will be placed in the target
 * Symtab object
 *
 * @param target - the target Symtab object, mainly used to compute relocations for new
 * Symbols that refer to Symtabs in the dependentObjects collection
 *
 * @return true, if sucessful; false otherwise
 */
bool emitElf::applyRelocations(char *targetData, std::vector<Symtab *> & dependentObjects, 
        std::map<Region *, Offset> & regionOffsets,
        std::map<Symbol *, Offset> & gotOffsets,
        Offset globalOffset, Offset gotOffset, Symtab *target, StaticLinkError &err,
        std::string &errMsg) 
{
    // Iterate over all relocations in all .o's
    std::vector<Symtab *>::iterator depObj_it;
    for(depObj_it = dependentObjects.begin(); depObj_it != dependentObjects.end(); ++depObj_it) {
        std::vector<Region *> allRegions;
        (*depObj_it)->getAllRegions(allRegions);

        fprintf(stderr, "\n*** Computing relocations for object: %s\n\n",
                (*depObj_it)->memberName().c_str());

        // Relocations are stored with the Region to which they will be applied
        // As an ELF example, .rel.text relocations are stored with the Region .text
        std::vector<Region *>::iterator region_it;
        for(region_it = allRegions.begin(); region_it != allRegions.end(); ++region_it) {
            std::vector<relocationEntry>::iterator rel_it;
            for(rel_it = (*region_it)->getRelocations().begin();
                rel_it != (*region_it)->getRelocations().end();
                ++rel_it)
            {
                // Only compute relocations for the new Regions
                if( regionOffsets.count(*region_it) ) {
                    // Compute destination of relocation
                    Offset dest = regionOffsets[*region_it] + rel_it->rel_addr();
                    Offset relOffset = globalOffset + dest;
                    if( !archSpecificRelocation(targetData, *rel_it, dest, relOffset,
                                globalOffset + gotOffset, gotOffsets) )
                    {
                        err = Relocation_Computation_Failure;
                        errMsg = "Failed to compute relocation";
                        return false;
                    }
                }
            }
        }
    }

    fprintf(stderr, "\n*** Computing relocations added to target.\n\n");

    // Compute relocations added to static target 
    std::vector<Region *> allRegions;
    target->getAllRegions(allRegions);

    std::vector<Region *>::iterator reg_it;
    for(reg_it = allRegions.begin(); reg_it != allRegions.end(); ++reg_it) {
        char *regionData = reinterpret_cast<char *>((*reg_it)->getPtrToRawData());
        
        std::vector<relocationEntry>::iterator rel_it;
        for(rel_it = (*reg_it)->getRelocations().begin();
            rel_it != (*reg_it)->getRelocations().end();
            ++rel_it)
        {
            if( !archSpecificRelocation(regionData, *rel_it,
                        rel_it->rel_addr() - (*reg_it)->getRegionAddr(),
                        rel_it->rel_addr(), gotOffset, gotOffsets) )
            {
                err = Relocation_Computation_Failure;
                errMsg = "failed to compute relocation";
                return false;
            }
        }
    }

    return true;
}

std::string emitElf::printStaticLinkError(StaticLinkError err) {
    switch(err) {
        CASE_RETURN_STR(No_Static_Link_Error);
        CASE_RETURN_STR(Symbol_Resolution_Failure);
        CASE_RETURN_STR(Relocation_Computation_Failure);
        CASE_RETURN_STR(Storage_Allocation_Failure);
        default:
            return "unknown error";
    }
}

/** The following functions are all architecture-specific */

/**
 * Computes the relocation value and copies it into the right location
 *
 * This routine is meant to be architecture dependent because relocations are
 * specific to a certain architecture.
 *
 * @param targetData - the destination for computed relocation value
 *
 * @param rel - the relocationEntry for this relocation
 *
 * @param dest - the Offset within targetData to place the relocation value
 *
 * @param relOffset - the Offset of the target of the relocation (needed for
 * PC relative relocations)
 *
 * @return true, if sucessful; false, otherwise
 */
bool emitElf::archSpecificRelocation(char *targetData, relocationEntry &rel,
        Offset dest, Offset relOffset, Offset gotOffset, std::map<Symbol *, Offset> &gotOffsets) 
{
#if defined(arch_x86)
    // All relocations on x86 are one word32 == Offset
    // Referring to the SYSV 386 supplement:
    // S = rel.getDynSym()->getOffset()
    // A = addend
    // P = relOffset
   
    Offset addend;
    if( rel.regionType() == Region::RT_REL ) {
        memcpy(&addend, &targetData[dest], sizeof(Offset));
    }else if( rel.regionType() == Region::RT_RELA ) {
        addend = rel.addend();
    }

    fprintf(stderr, "relocation for %s: S = %lx A = %lx P = %lx\n",
            rel.name().c_str(), rel.getDynSym()->getOffset(), addend, relOffset);

    Offset relocation = 0;
    switch(rel.getRelType()) {
        case R_386_32:
            relocation = rel.getDynSym()->getOffset() + addend;
            break;
        case R_386_PLT32: // this works because the address of the symbol is known at link time
        case R_386_PC32:
            relocation = rel.getDynSym()->getOffset() + addend - relOffset;
            break;
        case R_386_TLS_LE: // The necessary value is set during storage allocation
        case R_386_GLOB_DAT:
        case R_386_JMP_SLOT:
            relocation = rel.getDynSym()->getOffset();
            break;
        case R_386_GOT32:
            if( !gotOffsets.count(rel.getDynSym()) ) return false;
            relocation = gotOffsets[rel.getDynSym()] + addend - relOffset;
            break;
        case R_386_GOTOFF:
            relocation = rel.getDynSym()->getOffset() + addend - gotOffset;
            break;
        case R_386_GOTPC:
            relocation = gotOffset + addend - relOffset;
            break;
        case R_386_TLS_IE:
        case R_386_TLS_GOTIE:
            if( !gotOffsets.count(rel.getDynSym()) ) return false;
            relocation = gotOffsets[rel.getDynSym()] + gotOffset;
            break;
        case R_386_COPY:
        case R_386_RELATIVE:
            fprintf(stderr, "WARNING: encountered relocation type (%lu) that is meant for use during dynamic linking",
                    rel.getRelType());
            return false;
        default:
            fprintf(stderr, "Relocation type %lu currently unimplemented\n",
                    rel.getRelType());
            return false;
    }

    fprintf(stderr, "relocation = 0x%lx @ 0x%lx\n", relocation, relOffset);
    if( relocation == 0 ) fprintf(stderr, "WARNING: relocation value is 0x%lx\n", relocation);

    memcpy(&targetData[dest], &relocation, sizeof(Offset));

    return true;
#else
    fprintf(stderr, "currently, relocations cannot be calculated on this platform\n");
    return false;
#endif
}

Offset emitElf::allocateTLSImage(Offset tlsImageOffset, Offset globalOffset,
        std::map<Region *, Offset> &regionOffsets, std::map<Region *, Offset> &paddingMap,
        std::deque<Region *> &newRegions,
        Region *dataTLS, Region *bssTLS, std::vector<Symbol *> &tlsSyms)
{
    /*
     * This is pseudo-architecture dependent. The implementation of this function 
     * depends on the implementation of TLS on a specific architecture.
     *
     * This material comes from the "ELF Handling For TLS" white paper.
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
     * According to the paper, these are the two variants one would see when working
     * with ELF files. So, an architecture either implements variant 1 or 2.
     */

    // Variant 1
#if defined(arch_x86) 
    // The original init. image needs to remain in the image 1 slot because
    // the TLS data references are relative to that position
    unsigned long bssSize = 0;
    if( dataTLS != NULL ) newRegions.push_back(dataTLS);
    if( bssTLS != NULL ) bssSize = bssTLS->getRegionSize();

    // Create the image, note new BSS regions are expanded
    Offset endOffset = allocateRegions(newRegions,
            regionOffsets, paddingMap, tlsImageOffset, globalOffset);
    Offset adjustedEnd = endOffset - tlsImageOffset;

    // This is necessary so the offsets of existing TLS symbols can be updated
    // in a uniform, architecture independent way
    if( bssTLS != NULL ) {
        if( dataTLS != NULL ) {
            regionOffsets.insert(make_pair(bssTLS,regionOffsets[dataTLS]));
        }else{
            regionOffsets.insert(make_pair(bssTLS,endOffset));
        }
    }

    // Update the symbols with their offsets relative to the TCB address
    std::vector<Symbol *>::iterator sym_it;
    for(sym_it = tlsSyms.begin(); sym_it != tlsSyms.end(); ++sym_it) {
        // It is a programming error if the region for the symbol
        // was not passed to this function
        assert(regionOffsets.count((*sym_it)->getRegion()));

        Offset symbolOffset = (*sym_it)->getOffset();
        symbolOffset += (regionOffsets[(*sym_it)->getRegion()] - tlsImageOffset) -
            (adjustedEnd + bssSize);
        (*sym_it)->setOffset(symbolOffset);
    }

    return endOffset;
#else
    // Variant 2
    fprintf(stderr, "Variant 2 of TLS initialization image currently not implemented.\n");
    return tlsImageOffset;
#endif
}

Offset emitElf::adjustTLSOffset(Offset curOffset, Offset tlsSize) {
    // Variant 1
    Offset retOffset = curOffset;
#if defined(arch_x86)
    retOffset -= tlsSize;
#endif
    // Variant 2 does not require any modifications
    return retOffset;
}

char emitElf::getPaddingValue(Region::RegionType rtype) {
    const char X86_NOP = 0x90;
    char retChar = 0;
    if( rtype == Region::RT_TEXT || rtype == Region::RT_TEXTDATA ) {
        // This should be the value of a NOP (or equivalent instruction)
#if defined(arch_x86)
        retChar = X86_NOP;
#endif
    }

    return retChar;
}

void emitElf::cleanupTLSRegionOffsets(std::map<Region *, Offset> &regionOffsets,
        Region *, Region *bssTLS) 
{
#if defined(arch_x86)
    // Variant 1 - existing BSS TLS regions are not copied to the new target data
    regionOffsets.erase(bssTLS);
#else
    // Variant 2 TODO
#endif
}

void emitElf::getExcludedSymbolNames(std::set<std::string> &symNames) {
#if defined(arch_x86) 
    /*
     * It appears that some .o's have a reference to _GLOBAL_OFFSET_TABLE_
     * This reference is only needed for dynamic linking.
     */
    symNames.insert("_GLOBAL_OFFSET_TABLE_");
#endif
}

bool emitElf::isGOTRelocation(unsigned long relType) {
#if defined(arch_x86)
    switch(relType) {
        case R_386_GOT32:
        case R_386_TLS_IE:
        case R_386_TLS_GOTIE:
            return true;
            break;
        default:
            return false;
            break;
    }
#endif
    return false;
}

Offset emitElf::getGOTSize(std::set<Symbol *> &gotSymbols) {
    Offset size = 0;
#if defined(arch_x86)
    // According to the ELF abi, entries 0, 1, 2 are reserved in a GOT on x86
    if( gotSymbols.size() > 0 ) {
        assert(sizeof(Elf32_Addr) == sizeof(Offset));
        size = (gotSymbols.size()+3)*sizeof(Elf32_Addr);
    }
#endif
    return size;
}

bool emitElf::buildGOT(char *targetData,
        std::map<Symbol *, Offset> &gotOffsets,
        std::set<Symbol *> &gotSymbols)
{
#if defined(arch_x86)
    const int GOT_RESERVED = 3;
    // For each GOT symbol, allocate an entry and copy the value of the
    // symbol into the table
    Offset curOffset = GOT_RESERVED*sizeof(Offset);
    bzero(targetData, GOT_RESERVED*sizeof(Offset));
    std::set<Symbol *>::iterator sym_it;
    for(sym_it = gotSymbols.begin(); sym_it != gotSymbols.end(); ++sym_it) {
        Offset value = (*sym_it)->getOffset();
        memcpy(&targetData[curOffset], &value, sizeof(Offset));
        gotOffsets.insert(make_pair((*sym_it), curOffset));
        curOffset += sizeof(Offset);
    }

    return true;
#endif
    return false;
}
