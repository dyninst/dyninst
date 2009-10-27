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
static const int LARGE_FREE_SPACE = 50*1024*1024;

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
 * @param binary - the Symtab object being rewritten
 *
 * @return a pointer to the newly allocated space, to be written
 *         out during the rewrite process, NULL, if there is an error
 */
char *emitElf::linkStatic(Symtab *binary, 
        StaticLinkError &err, std::string &errMsg) 
{
    // Basic algorithm is:
    // * locate defined versions of symbols for undefined symbols in 
    //   either instrumentation or dependent code, produces a vector
    //   of Symtab objects that contain the defined versions
    // * Allocate storage for all new code and data to be linked into 
    //   the binary, produces a map of old regions to their new offsets
    //   in the binary
    // * Compute relocations and set their targets to these values. 
    // * The passed symtab object will reflect all these changes

    // Errors are set by the methods

    // Holds all necessary dependencies, as determined by resolveSymbols
    std::vector<Symtab *> dependentObjects;
    if( !resolveSymbols(binary, dependentObjects, err, errMsg) ) {
        return NULL;
    }

    // Determine starting location of new Regions
    Offset globalOffset = 0;
    std::vector<Region *> newRegs;
    if( binary->getAllNewRegions(newRegs) ) {
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


    // Create a temporary region for COMMON storage
    Region commonStorage;

    // Stores each common symbol by its alignment
    std::multimap<Offset, Symbol *> commonAlignments;

    // Holds location information for Regions copied into the static executable
    std::map<Region *, Offset> regionOffsets;
    
    char *rawDependencyData = allocateStorage(binary, dependentObjects, 
            regionOffsets, globalOffset, &commonStorage, commonAlignments, err, errMsg);

    if( rawDependencyData == NULL ) {
        return NULL;
    }

    if( !computeRelocations(rawDependencyData, dependentObjects, 
                regionOffsets, globalOffset, binary, err, errMsg) ) 
    {
        delete rawDependencyData;
        return NULL;
    }

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

    err = No_Static_Link_Error;
    errMsg = "";
    return rawDependencyData;
}

/**
 * Resolves undefined symbols in the specified Symtab object, usually due
 * to the addition of new Symbols to the Symtab object
 *
 * @param binary - the Symtab object being rewritten
 *
 * @param dependentObjects - on success, will hold all Symtab objects which
 * were used to resolve symbols i.e. they need to be copied into the binary
 * 
 * @return true, if sucessful, false, otherwise
 */
bool emitElf::resolveSymbols(Symtab *binary, 
        std::vector<Symtab *> &dependentObjects,
        StaticLinkError &err, std::string &errMsg) 
{
    // Collection of objects that currently need their symbols resolved
    std::set<Symtab *> workSet;

    // Collection of objects that have already had their symbols resolved
    // this is necessary to avoid errors related to circular dependencies
    std::set<Symtab *> visitedSet;

    // Add all object files explicitly referenced by new symbols, these
    // essentially fuel the linking process
    std::vector<Symtab *> explicitRefs;
    binary->getExplicitSymtabRefs(explicitRefs);

    std::vector<Symtab *>::iterator expObj_it;
    for(expObj_it = explicitRefs.begin(); expObj_it != explicitRefs.end();
            ++expObj_it) 
    {
        dependentObjects.push_back(*expObj_it);
        workSet.insert(*expObj_it);
    }

    // Establish list of libraries to search for symbols
    std::vector<Archive *> libraries;
    binary->getLinkingResources(libraries);

    std::set<Symtab *>::iterator curObjFilePtr = workSet.begin();
    while( curObjFilePtr != workSet.end() ) {
        // Take an object file off the working set
        Symtab *curObjFile = *curObjFilePtr;
        workSet.erase(curObjFile);
        visitedSet.insert(curObjFile);

        fprintf(stderr, "\n*** Resolving symbols for object: %s\n\n",
                curObjFile->memberName().c_str());

        // Build the map of Symbols to relocations
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

            // First, search the binary for the symbol
            std::vector<Symbol *> foundSyms;
            Symbol *extSymbol = NULL;
            if( !binary->isStripped() && 
                 binary->findSymbol(foundSyms, curUndefSym->getMangledName(),
                    curUndefSym->getType()) )
            {
                if( foundSyms.size() > 1 ) {
                    err = Symbol_Resolution_Failure;
                    errMsg = "ambiguous symbol definition: " + 
                        curUndefSym->getMangledName();
                    return false;
                }

                extSymbol = foundSyms[0];

                fprintf(stderr, "Found external symbol %s with addr = 0x%lx\n", 
                    extSymbol->getPrettyName().c_str(), extSymbol->getOffset());
            }else{
                //search loaded libraries and system libraries and add
                //any new object files as dependent objects
                std::vector<Archive *>::iterator lib_it;
                Symtab *containingSymtab = NULL;
                for(lib_it = libraries.begin(); lib_it != libraries.end(); ++lib_it) {
                    Symtab *tmpSymtab;
                    if( (*lib_it)->getMemberByGlobalSymbol(tmpSymtab, 
                                const_cast<std::string&>(curUndefSym->getMangledName())) ) 
                    {
                        if( containingSymtab != NULL ) {
                            err = Symbol_Resolution_Failure;
                            errMsg = "symbol defined in multiple libraries: " +
                                curUndefSym->getMangledName();
                            return false;
                        }

                        containingSymtab = tmpSymtab;
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
                        if( !visitedSet.count(containingSymtab) ) {
                            dependentObjects.push_back(containingSymtab);

                            // Resolve all symbols for this new Symtab, if it hasn't already
                            // been done

                            workSet.insert(containingSymtab);
                        }

                        fprintf(stderr, "Found external symbol %s in module %s\n",
                                extSymbol->getPrettyName().c_str(), 
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
                // There are some weird cases where an undefined symbol doesn't
                // have a relocation in its respective .o
                // An example case 'libc.a:ioungetwc.o' with the symbol __gcc_personality_v0
                fprintf(stderr, 
                        "WARNING: LINK ERROR: failed to find relocation for undefined symbol '%s'.\n",
                        curUndefSym->getPrettyName().c_str());
                
                /* old behavior
                err = Symbol_Resolution_Failure;
                errMsg = "failed to find relocation for symbol: " +
                    curUndefSym->getPrettyName();
                return false;
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
 * @param binary - the Symtab object being rewritten
 *
 * @param dependentObjects - the Symtab objects that need to be copied
 * into the binary
 *
 * @param regionOffset - populated by this function, this map will provide
 * the offsets of Regions in the dependentObjects in the new storage space
 *
 * @param globalOffset - the Offset in the binary where new code will be
 * placed
 *
 * @param commonStorage - a dummy region to use for combining COMMON blocks
 * => allows for uniform treatment of Regions later in the linking process
 *
 * @return commonAlignments - populated by this function, this map saves
 * the original alignments associated with COMMON symbols, this can be used
 * to rewind the changes made to Symbols during the linking process
 */
char *emitElf::allocateStorage(Symtab *binary,
        std::vector<Symtab *> & dependentObjects, 
        std::map<Region *, Offset> & regionOffsets, 
        Offset globalOffset, Region *commonStorage,
        std::multimap<Offset, Symbol *> &commonAlignments,
        StaticLinkError &err,
        std::string &errMsg) 
{

    unsigned long totalSize = 0;
    std::deque<Region *> codeRegions;
    std::deque<Region *> dataRegions;
    std::deque<Region *> bssRegions;

    fprintf(stderr, "\n*** Allocating storage for new objects\n\n");

    fprintf(stderr, "globalOffset = 0x%lx\n", globalOffset);

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
                totalSize += (*reg_it)->getRegionSize();
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

        std::vector<Symbol *> definedSyms;
        if( (*obj_it)->getAllDefinedSymbols(definedSyms) ) {
            std::vector<Symbol *>::iterator sym_it;
            for(sym_it = definedSyms.begin(); sym_it != definedSyms.end(); ++sym_it) {
                if( (*sym_it)->isCommonStorage() ) {
                    // For a symbol in common storage, the offset/value is the alignment

                    // This should never occur and probably indicates a bad object file
                    if( (*sym_it)->getOffset() == 0 ) {
                        err = Storage_Allocation_Failure;
                        errMsg = string("common symbol ") + (*sym_it)->getPrettyName() +
                            string(" has 0 alignment value");
                        return NULL;
                    }

                    commonAlignments.insert(make_pair((*sym_it)->getOffset(), *sym_it));
                }
            }
        }
    }

    /* START common block processing */

    // Combine all common symbols into a single block

    // The following approach is greedy and suboptimal
    // (in terms of space in the binary), but it's quick...
    Offset commonOffset = globalOffset;
    std::multimap<Offset, Symbol *>::iterator align_it;
    for(align_it = commonAlignments.begin(); 
        align_it != commonAlignments.end(); ++align_it)
    {
        if( (commonOffset % align_it->first) != 0 ) {
            Offset padding = align_it->first - (commonOffset % align_it->first);
            commonOffset += padding;
        }
        align_it->second->setOffset(commonOffset);
        commonOffset += align_it->second->getSize();
    }

    // Update the size of common storage
    if( commonAlignments.size() > 0 ) {
        // Region doesn't have a public constructor, but it does have a public
        // copy constructor...this isn't clean
        *commonStorage = *(*bssRegions.begin());
        commonStorage->setPtrToRawData(NULL, commonOffset - globalOffset);
        bssRegions.push_front(commonStorage);
        totalSize += commonOffset - globalOffset;
    }

    /* END common block processing */

    char *rawDependencyData = new char[totalSize];
    Offset currentOffset = 0;

    // Copy in bss regions 
    Offset bssRegionOffset = currentOffset;
    currentOffset = copyRegions(rawDependencyData, bssRegions, regionOffsets, 
            currentOffset);

    // Copy in code regions 
    Offset codeRegionOffset = currentOffset;
    currentOffset = copyRegions(rawDependencyData, codeRegions, regionOffsets, 
            currentOffset);

    // Copy in data regions 
    Offset dataRegionOffset = currentOffset;
    currentOffset = copyRegions(rawDependencyData, dataRegions, regionOffsets, 
            currentOffset);
    
    // Add new Regions to the binary
    binary->addRegion(globalOffset + bssRegionOffset, 
            reinterpret_cast<void *>(&rawDependencyData[bssRegionOffset]),
            static_cast<unsigned int>((codeRegionOffset - bssRegionOffset)),
            BSS_NAME, Region::RT_DATA, true);
    fprintf(stderr, "placed region %s @ %lx, size = %lx\n", BSS_NAME.c_str(),
            (globalOffset + bssRegionOffset), (codeRegionOffset - bssRegionOffset));

    binary->addRegion(globalOffset + codeRegionOffset, 
            reinterpret_cast<void *>(&rawDependencyData[codeRegionOffset]),
            static_cast<unsigned int>((dataRegionOffset - codeRegionOffset)),
            CODE_NAME, Region::RT_TEXT, true);
    fprintf(stderr, "placed region %s @ %lx, size = %lx\n", CODE_NAME.c_str(),
            (globalOffset + codeRegionOffset), (dataRegionOffset - codeRegionOffset));

    binary->addRegion(globalOffset + dataRegionOffset, 
            reinterpret_cast<void *>(&rawDependencyData[dataRegionOffset]),
            static_cast<unsigned int>((currentOffset - dataRegionOffset)),
            DATA_NAME, Region::RT_DATA, true);
    fprintf(stderr, "placed region %s @ %lx, size = %lx\n", DATA_NAME.c_str(),
            (globalOffset + dataRegionOffset), (currentOffset - dataRegionOffset));
   
    for(obj_it = dependentObjects.begin(); obj_it != dependentObjects.end(); ++obj_it) {
        // Update all relevant symbols with their offsets in the new binary
        std::vector<Symbol *> definedSyms;
        if( (*obj_it)->getAllDefinedSymbols(definedSyms) ) {
            std::vector<Symbol *>::iterator sym_it;
            for(sym_it = definedSyms.begin(); sym_it != definedSyms.end(); ++sym_it) {
                if(    regionOffsets.count((*sym_it)->getRegion())
                    && !(*sym_it)->isCommonStorage()) 
                {
                    Offset symbolOffset = (*sym_it)->getOffset();
                    symbolOffset += globalOffset + regionOffsets[(*sym_it)->getRegion()];
                    (*sym_it)->setOffset(symbolOffset);
                }
            }
        }
    }

    return rawDependencyData;
}

/**
 * Copies the specified regions into the specified storage space
 *
 * @param rawDependencyData - the Regions are copied into this storage space
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
Offset emitElf::copyRegions(char * rawDependencyData, std::deque<Region *> &regions, 
        std::map<Region *, Offset> &regionOffsets, Offset currentOffset) {
    Offset retOffset = currentOffset;

    std::deque<Region *>::iterator copyReg_it;
    for(copyReg_it = regions.begin(); copyReg_it != regions.end(); ++copyReg_it) {
        // Set up mapping for a new Region in the specified storage space
        std::pair<std::map<Region *, Offset>::iterator, bool> result;
        result = regionOffsets.insert(std::make_pair(*copyReg_it, retOffset));

        // If the map already contains this Region, this is a logic error
        assert(result.second);

        fprintf(stderr, "Region %s placed at 0x%lx, size = 0x%lx\n", (*copyReg_it)->getRegionName().c_str(),
                retOffset, (*copyReg_it)->getRegionSize());

        // Copy in the Region data
        char *rawRegionData = reinterpret_cast<char *>((*copyReg_it)->getPtrToRawData());

        // Currently, BSS is expanded
        if( (*copyReg_it)->isBSS() ) {
            bzero(&rawDependencyData[retOffset], (*copyReg_it)->getRegionSize());
        }else{
            memcpy(&rawDependencyData[retOffset], rawRegionData, (*copyReg_it)->getRegionSize());
        }
            
        retOffset += (*copyReg_it)->getRegionSize();
    }

    return retOffset;
}

/**
 * Given a collection of newly allocated regions in the specified storage space, 
 * computes relocations and places the values at the location specified by the
 * relocation entry (stored with the Regions)
 *
 * @param newRawData - the storage space for the newly allocated Regions
 *
 * @param dependentObjects - the Symtab objects whose select Regions have been copied
 * into the newRawData buffer
 *
 * @param regionOffsets - a mapping of Regions in dependentObjects to there offset in
 * the newRawData buffer
 *
 * @param globalOffset - the Offset at which newRawData will be placed in the target
 * Symtab object
 *
 * @param binary - the target Symtab object, mainly used to compute relocations for new
 * Symbols that refer to Symtabs in the dependentObjects collection
 *
 * @return true, if sucessful; false otherwise
 */
bool emitElf::computeRelocations(char *newRawData, std::vector<Symtab *> & dependentObjects, 
        std::map<Region *, Offset> & regionOffsets,
        Offset globalOffset, Symtab *binary, StaticLinkError &err,
        std::string &errMsg) 
{
    // Iterate over all relocations in all .o's
    std::vector<Symtab *>::iterator depObj_it;
    for(depObj_it = dependentObjects.begin(); depObj_it != dependentObjects.end(); ++depObj_it) {
        std::vector<Region *> allRegions;
        (*depObj_it)->getAllRegions(allRegions);

        fprintf(stderr, "\n*** Computing relocations for object: %s\n\n",
                (*depObj_it)->memberName().c_str());

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
                    if( !archSpecificRelocation(newRawData, *rel_it, dest, relOffset) ) {
                        err = Relocation_Computation_Failure;
                        errMsg = "Failed to compute relocation";
                        return false;
                    }
                }
            }
        }
    }

    fprintf(stderr, "\n*** Computing relocations added to binary.\n\n");

    // Compute relocations added to static binary 
    std::vector<Region *> allRegions;
    binary->getAllRegions(allRegions);

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
                        rel_it->rel_addr()) )
            {
                err = Relocation_Computation_Failure;
                errMsg = "failed to compute relocation";
                return false;
            }
        }
    }

    return true;
}

/**
 * Computes the relocation value and copies it into the right location
 *
 * This routine is meant to be architecture dependent because relocations are
 * specific to a certain architecture.
 *
 * @param newRawData - the destination for computed relocation value
 *
 * @param rel - the relocationEntry for this relocation
 *
 * @param dest - the Offset within newRawData to place the relocation value
 *
 * @param relOffset - the Offset within newRawData
 *
 * @return true, if sucessful; false, otherwise
 */
bool emitElf::archSpecificRelocation(char *newRawData, relocationEntry &rel,
        Offset dest, Offset relOffset) 
{
#if defined(arch_x86)
    // All relocations on x86 are one word32 == Offset
    // Referring to the SYSV 386 supplement:
    // S = rel.getDynSym()->getOffset()
    // A = addend
    // P = relOffset
   
    Offset addend;
    if( rel.regionType() == Region::RT_REL ) {
        memcpy(&addend, &newRawData[dest], sizeof(Offset));
    }else if( rel.regionType() == Region::RT_RELA ) {
        addend = rel.addend();
    }

    fprintf(stderr, "relocation for %s: S = %lx A = %lx P = %lx\n",
            rel.name().c_str(), rel.getDynSym()->getOffset(), addend, relOffset);

    Offset relocation;
    switch(rel.getRelType()) {
        case R_386_32:
            relocation = rel.getDynSym()->getOffset() + addend;
            break;
        case R_386_PC32:
            relocation = rel.getDynSym()->getOffset() + addend - relOffset;
            break;
        case R_386_GLOB_DAT:
        case R_386_JMP_SLOT:
            relocation = rel.getDynSym()->getOffset();
            break;
        default:
            fprintf(stderr, "Relocation type %lu currently unimplemented\n",
                    rel.getRelType());
            return false;
    }

    fprintf(stderr, "relocation = 0x%lx @ 0x%lx\n", relocation, relOffset);

    memcpy(&newRawData[dest], &relocation, sizeof(Offset));

    return true;
#else
    fprintf(stderr, "currently, relocations can only be calculated on x86\n");
    return false;
#endif
}

/*
bool emitElf::archSpecificInstrumentRelocation(char *regionData, Offset regionAddr,
        Symbol *targetSym, Address targetAddr) 
{
#if defined(arch_x86)
    Offset symbolOffset = targetSym->getOffset();
    memcpy(&regionData[targetAddr - regionAddr],
            &symbolOffset, sizeof(Offset));
    fprintf(stderr, "instrument relocation = 0x%lx @ 0x%lx\n", symbolOffset,
            targetAddr);
    return true;
#else
    fprintf(stderr, "currently, relocations can only be calculated on x86\n");
    return false;
#endif
}
*/

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

/* findDotOs - finds the list of .o files from archive 'arf' which
 * when added satisfy all the symbol references.
 *
 * We start with two different sets of symbols defined, undefined set
 * of symbols and a set of .o's that we need. Initially all the symbols
 * defined in the static executable fall under defined set and undefined fall
 * under the undefined set. We iterate over all the undefined symbols and try to find
 * the member in the archive which has the definition and add it to the list of .o's.
 * All the undefined symbols in this .o are added to undefined set and defined symbols
 * are added to the defined set. This process goes on until we do not have any undefined symbols.
 * Otherwise we return an error
 */
bool emitElf::findDotOs(Symtab *obj, std::vector<Archive *> &arfs, std::vector<Symtab *>&members){
    std::vector<Symbol *> undefSyms;

    //Initialize undefSyms with all the undefined members in the static executable
    obj->getAllUndefinedSymbols(undefSyms);

    while(undefSyms.size() != 0){
        //get the first undefined symbol
        Symbol *sym = undefSyms[0];
        undefSyms.erase(undefSyms.begin());
        Symtab *tab; // Symtab object for member that contains the definition
        vector<Symbol *> foundsyms;
        //first check if the symbol is already defined in the static executable
        if(obj->findSymbolByType(foundsyms, sym->getName(), Symbol::ST_UNKNOWN, true))
            continue;
        //check all archives starting from the beginning
        for(unsigned i=0;i<arfs.size();i++){
            std::string symName = sym->getName();
            if(arfs[i]->findMemberWithDefinition(tab, symName)){
                members.push_back(tab);
                vector<Symbol *>undefs;
                if(tab->getAllUndefinedSymbols(undefs))
                    undefSyms.insert(undefSyms.end(), undefs.begin(), undefs.end());
                continue;
            }
        }
        //reached if there are undefined symbols that are not defined in any of the archives
        return false;
    }
    return true;
}
