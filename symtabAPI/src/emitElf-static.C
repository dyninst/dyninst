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

#include <stdio.h>
#include <stdlib.h>

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

static const std::string CODE_NAME = ".depCode";
static const std::string DATA_NAME = ".depData";
static const std::string BSS_NAME = ".depBss";

char *emitElf::linkStaticCode(Symtab *binary, 
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
    Region *instrumentRegion;
    Offset globalOffset = 0;
    std::vector<Region *> newRegs;
    if( binary->getAllNewRegions(newRegs) ) {
        // This is true if instrumentation Region is added
        std::vector<Region *>::iterator newReg_it;
        for(newReg_it = newRegs.begin(); newReg_it != newRegs.end(); ++newReg_it) {
            Offset newRegEndAddr = (*newReg_it)->getRegionSize() + (*newReg_it)->getRegionAddr();
            if( newRegEndAddr > globalOffset ) {
                globalOffset = newRegEndAddr;
                instrumentRegion = *newReg_it;
            }
        }
    }else{
        err = Instrument_Location_Error;
        errMsg = "failed to locate .dyninstInst instrumentation section";
        return NULL;
    }

    fprintf(stderr, "globalOffset = 0x%lx\n", globalOffset);

    // Holds location information for Regions copied into the static executable
    std::map<Region *, Offset> regionOffsets;
    char *rawDependencyData = allocateStorage(binary, dependentObjects, 
            regionOffsets, globalOffset, err, errMsg);

    if( rawDependencyData == NULL ) {
        return NULL;
    }

    if( !computeRelocations(rawDependencyData, dependentObjects, regionOffsets, globalOffset, err, errMsg) ) {
        delete rawDependencyData;
        return NULL;
    }

    computeInstrumentRelocations(instrumentRegion, binary->interModuleSymRefs_);

    err = No_Static_Link_Error;
    errMsg = "";
    return rawDependencyData;
}

bool emitElf::resolveSymbols(Symtab *binary, 
        std::vector<Symtab *> & dependentObjects,
        StaticLinkError &err, std::string &errMsg) 
{
    std::set<Symtab *> workSet;

    // Automatically add all object files needed by instrumentation
    std::map<Symbol *, std::vector<Address> >::iterator objfile_it;
    for(objfile_it = binary->interModuleSymRefs_.begin();
        objfile_it != binary->interModuleSymRefs_.end();
        ++objfile_it) 
    {
        dependentObjects.push_back(objfile_it->first->getSymtab());
        workSet.insert(objfile_it->first->getSymtab());
    }

    std::set<Symtab *>::iterator curObjFilePtr = workSet.begin();
    while( curObjFilePtr != workSet.end() ) {
        // Take an object file off the working set
        Symtab *curObjFile = *curObjFilePtr;
        workSet.erase(curObjFile);

        std::map<Symbol *, std::vector<ELFRelocation> > curRels;
        curObjFile->getObject()->getELFRelocations(curRels);

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
                 binary->findSymbol(foundSyms, curUndefSym->getPrettyName(),
                    curUndefSym->getType()) )
            {
                // Get the first found symbol TODO make this more exact
                extSymbol = foundSyms[0];

                fprintf(stderr, "Found external symbol %s with addr = 0x%lx\n", 
                        extSymbol->getPrettyName().c_str(), extSymbol->getOffset());
            }else{
                //TODO search loaded libraries and system libraries and add
                //any new object files as dependent objects
                err = Symbol_Resolution_Failure;
                errMsg = "failed to locate symbol: " + 
                    curUndefSym->getPrettyName();
                return false;
            }

            assert(extSymbol != NULL);

            // Store the found symbol with the related relocations
            std::map<Symbol *, std::vector<ELFRelocation> >::iterator relMap_it;
            relMap_it = curRels.find(curUndefSym);
            if( relMap_it != curRels.end() ) {
                std::vector<ELFRelocation>::iterator rel_it;
                for(rel_it = relMap_it->second.begin(); rel_it != relMap_it->second.end();
                        ++rel_it)
                {
                    rel_it->addDynSym(extSymbol);
                }
            }else{
                err = Symbol_Resolution_Failure;
                errMsg = "failed to find relocation for symbol: " +
                    curUndefSym->getPrettyName();
                return false;
            }
        }

        // Store changes to relocations
        curObjFile->getObject()->setELFRelocations(curRels);   

        curObjFilePtr = workSet.begin();
    }
    
    return true;
}

char *emitElf::allocateStorage(Symtab *binary,
        std::vector<Symtab *> & dependentObjects, 
        std::map<Region *, Offset> & regionOffsets, 
        Offset globalOffset,
        StaticLinkError &err,
        std::string &errMsg) 
{

    unsigned long totalSize = 0;
    std::vector<Region *> codeRegions;
    std::vector<Region *> dataRegions;
    std::vector<Region *> bssRegions;

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
                        dataRegions.push_back(*reg_it);
                        break;
                    default:
                        // skip other regions
                        continue;
                }
            }
        }
    }

    char *rawDependencyData = new char[totalSize];
    Offset currentOffset = 0;
    
    // Copy in code regions 
    Offset codeRegionOffset = currentOffset;
    currentOffset = copyRegions(rawDependencyData, codeRegions, regionOffsets, 
            currentOffset);

    // Copy in data regions 
    Offset dataRegionOffset = currentOffset;
    currentOffset = copyRegions(rawDependencyData, dataRegions, regionOffsets, 
            currentOffset);

    // Copy in bss regions 
    Offset bssRegionOffset = currentOffset;
    currentOffset = copyRegions(rawDependencyData, bssRegions, regionOffsets, 
            currentOffset);

    // Add new Regions to the binary
    binary->addRegion(globalOffset + codeRegionOffset, 
            reinterpret_cast<void *>(&rawDependencyData[codeRegionOffset]),
            static_cast<unsigned int>((dataRegionOffset - codeRegionOffset)),
            CODE_NAME, Region::RT_TEXT, true);
    binary->addRegion(globalOffset + dataRegionOffset, 
            reinterpret_cast<void *>(&rawDependencyData[dataRegionOffset]),
            static_cast<unsigned int>((bssRegionOffset - dataRegionOffset)),
            DATA_NAME, Region::RT_DATA, true);
    binary->addRegion(globalOffset + bssRegionOffset, reinterpret_cast<void *>(&rawDependencyData[bssRegionOffset]),
            static_cast<unsigned int>((currentOffset - bssRegionOffset)),
            BSS_NAME, Region::RT_BSS, true);

    for(obj_it = dependentObjects.begin(); obj_it != dependentObjects.end(); ++obj_it) {
        // Update all symbols with their new offsets in the binary
        std::vector<Symbol *> definedSyms;
        if( (*obj_it)->getAllDefinedSymbols(definedSyms) ) {
            std::vector<Symbol *>::iterator sym_it;
            for(sym_it = definedSyms.begin(); sym_it != definedSyms.end(); ++sym_it) {
                if( regionOffsets.count((*sym_it)->getRegion()) ) {
                    Offset symbolOffset = (*sym_it)->getOffset();
                    symbolOffset += globalOffset + regionOffsets[(*sym_it)->getRegion()];
                    (*sym_it)->setOffset(symbolOffset);
                }
            }
        }
    }

    return rawDependencyData;
}

Offset emitElf::copyRegions(char * rawDependencyData, std::vector<Region *> &regions, 
        std::map<Region *, Offset> &regionOffsets, Offset currentOffset) {
    Offset retOffset = currentOffset;

    std::vector<Region *>::iterator copyReg_it;
    for(copyReg_it = regions.begin(); copyReg_it != regions.end(); ++copyReg_it) {
        // Set up mapping for a new Region in the static binary
        std::pair<std::map<Region *, Offset>::iterator, bool> result;
        result = regionOffsets.insert(std::make_pair(*copyReg_it, retOffset));

        // If the map already contains this Region, this is a logic error
        assert(result.second);

        fprintf(stderr, "Region %s placed at 0x%lx\n", (*copyReg_it)->getRegionName().c_str(),
                retOffset);

        // Copy in the Region data
        char *rawRegionData = reinterpret_cast<char *>((*copyReg_it)->getPtrToRawData());
        memcpy(&rawDependencyData[retOffset], rawRegionData, (*copyReg_it)->getRegionSize());
        retOffset += (*copyReg_it)->getRegionSize();
    }

    return retOffset;
}

bool emitElf::computeRelocations(char *newRawData, std::vector<Symtab *> & dependentObjects, 
        std::map<Region *, Offset> & regionOffsets, Offset globalOffset, StaticLinkError &err,
        std::string &errMsg) 
{
    // Iterate over all relocations in all .o's
    std::vector<Symtab *>::iterator depObj_it;
    for(depObj_it = dependentObjects.begin(); depObj_it != dependentObjects.end(); ++depObj_it) {

        std::map<Symbol *, std::vector<ELFRelocation> > rels;
        (*depObj_it)->getObject()->getELFRelocations(rels);

        std::map<Symbol *, std::vector<ELFRelocation> >::iterator relVec_it;
        for(relVec_it = rels.begin(); relVec_it != rels.end(); ++relVec_it) {

            std::vector<ELFRelocation>::iterator rel_it;
            for(rel_it = relVec_it->second.begin(); rel_it != relVec_it->second.end(); ++rel_it) {

                // Only compute relocations for the new Regions
                if( regionOffsets.count(rel_it->getTargetRegion()) ) {

                    // Compute destination of relocation
                    Offset dest = regionOffsets[rel_it->getTargetRegion()] + rel_it->rel_addr();
                    Offset relOffset = globalOffset + 
                        regionOffsets[rel_it->getTargetRegion()] + rel_it->rel_addr();
                    if( !archSpecificRelocation(newRawData, *rel_it, dest, relOffset) ) {
                        err = Relocation_Computation_Failure;
                        errMsg = "Failed to compute relocation";
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

bool emitElf::archSpecificRelocation(char *newRawData, ELFRelocation &rel, Offset dest, Offset relOffset) {
// This routine is meant to be architecture dependent because relocations are
// specific to a certain architecture
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
            rel.name().c_str(), 
            rel.getDynSym()->getOffset(),
            addend, relOffset);

    Offset relocation;
    switch(rel.getRelType()) {
        case R_386_32:
            relocation = rel.getDynSym()->getOffset() + addend;
            break;
        case R_386_PC32:
            relocation = rel.getDynSym()->getOffset() + addend - relOffset;
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

void emitElf::computeInstrumentRelocations(Region *instrumentRegion,
        std::map<Symbol *, std::vector<Address> > &interModSymRefs) 
{
    char *regionData = reinterpret_cast<char *>(instrumentRegion->getPtrToRawData());
    std::map<Symbol *, std::vector<Address> >::iterator symRef_it;
    for(symRef_it = interModSymRefs.begin(); 
        symRef_it != interModSymRefs.end(); ++symRef_it) 
    {
        std::vector<Address>::iterator addr_it;
        for(addr_it = symRef_it->second.begin(); 
            addr_it != symRef_it->second.end(); ++addr_it) 
        {
            // TODO this probably needs to be architecture dependent
            Offset symbolOffset = symRef_it->first->getOffset();
            memcpy(&regionData[*addr_it - instrumentRegion->getRegionAddr()],
                   &symbolOffset, sizeof(symbolOffset));
        }    
    }
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
