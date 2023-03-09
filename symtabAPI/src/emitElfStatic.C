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

#include <cstdlib>
#include <cstdio>
#include <iostream>

#include <algorithm>

#include "emitElfStatic.h"
#include "debug.h"
#include "Object-elf.h"
#include "unaligned_memory_access.h"

#if defined(os_freebsd)
#define R_X86_64_JUMP_SLOT R_X86_64_JMP_SLOT
#endif
#if !defined(R_X86_64_IRELATIVE)
#define R_X86_64_IRELATIVE 37
#endif
#if !defined(R_386_IRELATIVE)
#define R_386_IRELATIVE 42
#endif

using namespace Dyninst;
using namespace SymtabAPI;

// Section names
static const string CODE_NAME(".dyninstCode");
static const string STUB_NAME(".dyninstStub");
static const string DATA_NAME(".dyninstData");
static const string BSS_NAME(".dyninstBss");
static const string GOT_NAME(".dyninstGot");
static const string CTOR_NAME(".dyninstCtors");
static const string DTOR_NAME(".dyninstDtors");
static const string TLS_DATA_NAME(".dyninstTdata");
static const string DEFAULT_COM_NAME(".common");
static const string PLT_NAME(".dyninstPLT");
static const string REL_NAME(".dyninstRELA");
static const string REL_GOT_NAME(".dyninstRELAgot");

/* Used by architecture specific functions, but not architecture specific */
const string Dyninst::SymtabAPI::SYMTAB_CTOR_LIST_REL("__SYMTABAPI_CTOR_LIST__");
const string Dyninst::SymtabAPI::SYMTAB_DTOR_LIST_REL("__SYMTABAPI_DTOR_LIST__");
const string Dyninst::SymtabAPI::SYMTAB_IREL_START("__SYMTABAPI_IREL_START__");
const string Dyninst::SymtabAPI::SYMTAB_IREL_END("__SYMTABAPI_IREL_END__");

emitElfStatic::emitElfStatic(unsigned addressWidth, bool isStripped) :
    addressWidth_(addressWidth),
    isStripped_(isStripped),
    hasRewrittenTLS_(false)
{

}

/**
 * NOTE:
 * Most of these functions take a reference to a StaticLinkError and a string
 * for error reporting purposes. These should prove useful in identifying the
 * cause of an error
 */

/**
 * Statically links relocatable files into the specified Symtab object,
 * as specified by state in the Symtab object
 *
 * target       relocatable files will be linked into this Symtab
 *
 * Returns a pointer to a block of data containing the results of the link
 * The caller is responsible for delete'ing this block of data
 */
char *emitElfStatic::linkStatic(Symtab *target,
        StaticLinkError &err, string &errMsg)
{
    /*
     * Basic algorithm is:
     * - locate defined versions of symbols for undefined symbols in
     *   either the target or relocatable code, produces a list
     *   of Symtab objects that contain the defined versions
     *
     * - Allocate space for all new code and data to be linked into
     *   the target, populates a LinkMap with all allocation
     *   information
     *
     * - Compute relocations and set their targets to these values
     */

    // Holds all information necessary to work with the block of data created by createLinkMap
    LinkMap lmap;

    rewrite_printf("START link map output\n");

    // Determine starting location of new Regions
    Offset globalOffset = 0;
    vector<Region *> newRegs;
    if( target->getAllNewRegions(newRegs) ) {
        // This is true, only if other regions have already been added
        vector<Region *>::iterator newReg_it;
        for(newReg_it = newRegs.begin(); newReg_it != newRegs.end(); ++newReg_it) {
            Offset newRegEndAddr = (*newReg_it)->getMemSize() + (*newReg_it)->getDiskOffset();
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

    // Holds all necessary dependencies, as determined by resolveSymbols
    vector<Symtab *> relocatableObjects;
    if( !resolveSymbols(target, relocatableObjects, lmap, err, errMsg) ) {
        return NULL;
    }

    // Lays out all relocatable files into a single contiguous block of data
    if( !createLinkMap(target, relocatableObjects, globalOffset, lmap, err, errMsg) ) {
        return NULL;
    }

    // Copies the data from the relocatable files into the new block of data
    if( !addNewRegions(target, globalOffset, lmap) ) {
        err = Storage_Allocation_Failure;
        errMsg = "Failed to create new Regions in original binary";
        return NULL;
    }

    if (!updateTOC(target, lmap, globalOffset)) {
      return NULL;
    }

    // Print out the link map for debugging
    rewrite_printf("Global Offset = 0x%lx\n", globalOffset);
    if( sym_debug_rewrite ) {
        lmap.printAll(cerr, globalOffset);
        lmap.printBySymtab(cerr, relocatableObjects, globalOffset);
    }

    // Now that all symbols are at their final locations, compute and apply relocations
    if( !applyRelocations(target, relocatableObjects, globalOffset, lmap, err, errMsg) ) {
        if( lmap.allocatedData ) delete[] lmap.allocatedData;
        return NULL;
    }

    if (!buildPLT(target, globalOffset, lmap, err, errMsg)) {
      if (lmap.allocatedData) delete[] lmap.allocatedData;
      return NULL;
    }

    if (!buildRela(target, globalOffset, lmap, err, errMsg)) {
      if (lmap.allocatedData) delete[] lmap.allocatedData;
      return NULL;
    }

    // Restore the offset of the modified symbols
    vector< pair<Symbol *, Offset> >::iterator symOff_it;
    for(symOff_it = lmap.origSymbols.begin();
        symOff_it != lmap.origSymbols.end();
        ++symOff_it)
    {
        symOff_it->first->setOffset(symOff_it->second);
    }

    // Restore the symbols of the modified relocations
   vector< pair<relocationEntry *, Symbol *> >::iterator relSym_it;
   for(relSym_it = lmap.origRels.begin();
       relSym_it != lmap.origRels.end();
       ++relSym_it)
   {
       relSym_it->first->addDynSym(relSym_it->second);
   }

    rewrite_printf("\n*** Finished static linking\n\n");

    rewrite_printf("END link map output\n");

    err = No_Static_Link_Error;
    errMsg = "";
    return lmap.allocatedData;
}

/**
 * Resolves undefined symbols in the specified Symtab object, usually due
 * to the addition of new Symbols to the Symtab object. The target Symtab
 * object must have a collection of Archives associated with it. These
 * Archives will be searched for the defined versions of the undefined
 * symbols in the specified Symtab objects.
 *
 * target               the Symtab containing the undefined symbols
 * relocatableObjects   populated by this function, this collection specifies
 *                      all the Symtabs needed to ensure that the target has
 *                      no undefined Symbols once the link is performed
 */
bool emitElfStatic::resolveSymbols(Symtab *target,
        vector<Symtab *> &relocatableObjects,
        LinkMap &lmap,
        StaticLinkError &err, string &errMsg)
{
    // Collection of Symtabs that currently need their symbols resolved
    set<Symtab *> workSet;

    // Collection of Symtabs that have already had their symbols resolved
    // this is necessary to avoid errors related to circular dependencies
    set<Symtab *> linkedSet;

    set<string> excludeSymNames;
    getExcludedSymbolNames(excludeSymNames);

    // Establish list of libraries to search for symbols
    vector<Archive *> libraries;
    target->getLinkingResources(libraries);

    // Add all relocatable files explicitly referenced by new symbols, these
    // essentially fuel the symbol resolution process
    set<Symtab *> explicitRefs;
    target->getExplicitSymtabRefs(explicitRefs);

    set<Symtab *>::iterator expObj_it;
    for(expObj_it = explicitRefs.begin(); expObj_it != explicitRefs.end();
            ++expObj_it)
    {
        relocatableObjects.push_back(*expObj_it);
        workSet.insert(*expObj_it);
        linkedSet.insert(*expObj_it);
    }

    set<Symtab *>::iterator curObjFilePtr = workSet.begin();
    while( curObjFilePtr != workSet.end() ) {
        // Take a relocatable file off the working set
        Symtab *curObjFile = *curObjFilePtr;
        workSet.erase(curObjFile);

        rewrite_printf("\n*** Resolving symbols for: %s\n\n",
                curObjFile->memberName().c_str());

        // Build the map of symbols to their relocations
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
                lmap.origRels.push_back(make_pair(&(*rel_it), rel_it->getDynSym()));
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

            if( !checkSpecialCaseSymbols(curObjFile, curUndefSym) ) {
                err = Symbol_Resolution_Failure;
                errMsg = "special case check failed for symbol: " +
                    curUndefSym->getMangledName();
                return false;
            }

            Symbol *extSymbol = NULL;

            // First, attempt to search the target for the symbol
            if( !isStripped_ ) {
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
                // search loaded libraries and add any new reloctable files
                vector<Symtab *> possibleSymtabs;

                for(auto lib_it = libraries.begin(); lib_it != libraries.end(); ++lib_it) {
                   (*lib_it)->getMembersBySymbol(curUndefSym->getMangledName(), possibleSymtabs);
                }

                // Look through the possibleSymtabs for symbol hits
                // Options are:
                // 1) A lot of weak symbols; pick one
                // 2) A lot of weak symbols and a strong symbol: pick strong
                // 3) Multiple strong symbols: fail
                // 4) ... no symbols. Also fail.

                for (auto iter = possibleSymtabs.begin(); iter != possibleSymtabs.end(); ++iter) {
                   std::vector<Symbol *> syms;
                   (*iter)->findSymbol(syms, curUndefSym->getMangledName(), curUndefSym->getType());
                   if (syms.empty()) {
                      // Problem: mismatch between the Archive table and Symtab
                      err = Symbol_Resolution_Failure;
                      errMsg = "inconsistency found between archive's symbol table and member's symbol table";
                      return false;
                   }

                   for (auto iter2 = syms.begin(); iter2 != syms.end(); ++iter2) {
		     // We include UNKNOWN symbols in the find; unfortunately that's
		     // not what we want here. Skip.
		     if ((*iter2)->getType() == Symbol::ST_UNKNOWN) {
		       continue;
		     }

                      if (extSymbol == NULL || extSymbol->getLinkage() == Symbol::SL_WEAK) {
                         extSymbol = *iter2;
                      }
                      else if ((*iter2)->getLinkage() == Symbol::SL_WEAK) {
                         continue;
                      }
                      else {
                         // Two strong symbols: bad
                         err = Symbol_Resolution_Failure;
                         errMsg = "ambiguous symbol definition: " +
                            curUndefSym->getMangledName();
                         return false;
                      }
                   }
                }

                if( extSymbol == NULL ) {
                   // If it is a weak symbol, it isn't an error that the symbol wasn't resolved
                   if( curUndefSym->getLinkage() == Symbol::SL_WEAK ) {
                      continue;
                   }
		   if (curUndefSym->getMangledName().compare(".TOC.") == 0) {
		      continue;
		   }

                   err = Symbol_Resolution_Failure;
                   errMsg = "failed to locate symbol '" + curUndefSym->getMangledName()
                      + "' for object '" + curObjFile->memberName() + "'";
                   rewrite_printf(" failed to locate symbol %s for %s  \n" , curUndefSym->getMangledName().c_str(), curObjFile->memberName().c_str());
                   return false;
                }
                Symtab *containingSymtab = extSymbol->getSymtab();

                if( !linkedSet.count(containingSymtab) ) {
                   // Consistency check
                   if( containingSymtab->getAddressWidth() != addressWidth_ ) {
                      err = Symbol_Resolution_Failure;
                      errMsg = "symbol (" + curUndefSym->getMangledName() +
                         ") found in relocatable file that is not compatible"
                         + " with the target binary";
                      return false;
                   }

                   relocatableObjects.push_back(containingSymtab);

                   // Resolve all symbols for this new Symtab, if it hasn't already
                   // been done

                   workSet.insert(containingSymtab);
                   linkedSet.insert(containingSymtab);
                }

                rewrite_printf("Found external symbol %s in object %s(%s)\n",
                               extSymbol->getPrettyName().c_str(),
                               containingSymtab->getParentArchive()->name().c_str(),
                               containingSymtab->memberName().c_str());
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
                 * use it in order to ensure that the object that defines the
                 * symbol is linked. Therefore, it is not an error if a symbol doesn't
                 * have a relocation.
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
 * Allocates COMMON symbols in the collection of Symtab objects
 * as bss
 *
 * Creates a new TLS initialization image, combining the target image
 * and the image that exists in the collection of Symtab objects
 *
 * Creates a GOT used for indirect memory accesses that is required by some
 * relocations
 *
 * Creates a new global constructor and/or destructor table if necessary,
 * combining tables from the target and collection of Symtab objects
 *
 * target               New code/data/etc. will be linked into this Symtab
 * relocatableObjects   The new code/data/etc.
 * globalOffset         The location of the new block of data in the target
 * lmap                 The LinkMap to be populated by this function
 */
bool emitElfStatic::createLinkMap(Symtab *target,
        vector<Symtab *> &relocatableObjects,
        Offset & globalOffset,
        LinkMap &lmap,
        StaticLinkError &err, string &errMsg)
{
    rewrite_printf("\n*** Allocating storage for relocatable objects\n\n");

    // Used to create a new COMMON block
    multimap<Offset, Symbol *> commonAlignments;
    Offset commonRegionAlign = 0;

    // Collect all Regions that should be allocated in the new data block
    vector<Symtab *>::iterator obj_it;
    for(obj_it = relocatableObjects.begin(); obj_it != relocatableObjects.end(); ++obj_it) {
        vector<Region *> regs;
        if( !(*obj_it)->getAllRegions(regs) ) {
            err = Storage_Allocation_Failure;
            errMsg = "failed to locate regions in relocatable object";
            return false;
        }

        vector<Region *>::iterator reg_it;
        for(reg_it = regs.begin(); reg_it != regs.end(); ++reg_it) {
            string regionName = (*reg_it)->getRegionName();
            if( (*reg_it)->isLoadable() && (*reg_it)->getMemSize() > 0) {
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
                }else if( isConstructorRegion(*reg_it) ) {
                    lmap.newCtorRegions.insert(*reg_it);
                    if( (*reg_it)->getMemAlignment() > lmap.ctorRegionAlign ) {
                        lmap.ctorRegionAlign = (*reg_it)->getMemAlignment();
                    }
                }else if( isDestructorRegion(*reg_it) ) {
                    lmap.newDtorRegions.insert(*reg_it);
                    if( (*reg_it)->getMemAlignment() > lmap.dtorRegionAlign ) {
                        lmap.dtorRegionAlign = (*reg_it)->getMemAlignment();
                    }
		} else if( isGOTRegion(*reg_it) ) {
		  lmap.gotRegions.push_back(*reg_it);
		  if( (*reg_it)->getMemAlignment() > lmap.gotRegionAlign ) {
		    lmap.gotRegionAlign = (*reg_it)->getMemAlignment();
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

                // Find symbols that need to be put in the GOT

#if defined(arch_power) && defined(arch_64bit)
		// If statically linked binary, we need to put all the entries in region toc to GOT
		if(target->isStaticBinary() && (regionName.compare(".toc") == 0)) {
                   // For every symbol in toc
                   // Get data of the region and split it into symbols
                   char *rawRegionData = reinterpret_cast<char *>((*reg_it)->getPtrToRawData());
                   unsigned long numSymbols = (*reg_it)->getMemSize()/8;
                   for (Offset entry = 0; entry < numSymbols; entry++ ) {
                      // If symbol has relocation, add that
                      bool relExist = false;
                      vector<relocationEntry> &region_rels = (*reg_it)->getRelocations();
                      vector<relocationEntry>::iterator rel_it;
                      for( rel_it = region_rels.begin(); rel_it != region_rels.end(); ++rel_it) {
                         if(entry*8 == rel_it->rel_addr()) {
                            lmap.gotSymbolTable.push_back(make_pair(rel_it->getDynSym(), rel_it->addend()));
                            lmap.gotSymbols.insert(make_pair(rel_it->getDynSym(), rel_it->addend()));
                            relExist = true;

                            break;
                         }
                      }
                      if(!relExist) {
                         // else create a new symbol
                         // Offset and name is all we care - offset should be value of the symbol
                         Offset soffset = *((unsigned long *) (rawRegionData + entry*8));
                         Symbol *newsym = new Symbol("dyntoc_entry",
                                                     Symbol::ST_UNKNOWN,
                                                     Symbol::SL_UNKNOWN,
                                                     Symbol::SV_UNKNOWN,
                                                     soffset);

                         lmap.gotSymbolTable.push_back(make_pair(newsym, 0));
                         lmap.gotSymbols.insert(make_pair(newsym, 0));

                      }
                   }

                }
#endif
                vector<relocationEntry> &region_rels = (*reg_it)->getRelocations();
                vector<relocationEntry>::iterator rel_it;
                for( rel_it = region_rels.begin(); rel_it != region_rels.end(); ++rel_it) {

		  if(isGOTRelocation(rel_it->getRelType()) ) {
		    lmap.gotSymbolTable.push_back(make_pair(rel_it->getDynSym(), rel_it->addend()));
		    lmap.gotSymbols.insert(make_pair(rel_it->getDynSym(), 0));

		  }

                }
            } // isLoadable

        } //for regions

        vector<Symbol *> definedSyms;
        if( (*obj_it)->getAllDefinedSymbols(definedSyms) ) {
            vector<Symbol *>::iterator sym_it;
            for(sym_it = definedSyms.begin(); sym_it != definedSyms.end(); ++sym_it) {
                // Note: the assumption is made that a Symbol cannot
                // be both TLS and COMMON
                if( (*sym_it)->isCommonStorage() ) {
                    // For a symbol in common storage, the offset/value is the alignment
                    commonAlignments.insert(make_pair((*sym_it)->getOffset(), *sym_it));

                    // The alignment of a COMMON Region is the maximum alignment
                    // needed by one of its symbols
                    if( (*sym_it)->getOffset() > commonRegionAlign ) {
                        commonRegionAlign = (*sym_it)->getOffset();
                    }
                }else if( (*sym_it)->getType() == Symbol::ST_TLS ) {
                    lmap.tlsSymbols.push_back(*sym_it);
                }
            }
        }
    }

    // Determine how all the Regions in the relocatable objects will be
    // placed in the rewritten target
    Offset currentOffset = 0;

    // Allocate code regions

    // Since this is the first new Region, the actual globalOffset should be
    // the passed globalOffset plus the padding for this Region

    Offset maxAlign = getGOTAlign(lmap);
    if(maxAlign < lmap.codeRegionAlign) maxAlign = lmap.codeRegionAlign;

    globalOffset += computePadding(globalOffset + currentOffset,
            maxAlign);

    // Handle PLT entries generated by GNU indirect functions
    lmap.pltRegionAlign = lmap.codeRegionAlign;
    lmap.pltRegionOffset = currentOffset;
    lmap.pltRegionOffset += computePadding(globalOffset + lmap.pltRegionOffset,
					   lmap.pltRegionAlign);
    currentOffset = allocatePLTEntries(lmap.pltEntries, lmap.pltRegionOffset, lmap.pltSize);

    lmap.relRegionAlign = lmap.pltRegionAlign;
    lmap.relRegionOffset = currentOffset;
    lmap.relRegionOffset += computePadding(globalOffset + lmap.relRegionOffset,
					  lmap.relRegionAlign);
    currentOffset = allocateRelocationSection(lmap.pltEntries, lmap.relRegionOffset,
					      lmap.relSize, target);

    lmap.relGotRegionAlign = lmap.pltRegionAlign;
    lmap.relGotRegionOffset = currentOffset;
    lmap.relGotRegionOffset += computePadding(globalOffset + lmap.relGotRegionOffset,
					      lmap.relGotRegionAlign);
    currentOffset = allocateRelGOTSection(lmap.pltEntries, lmap.relGotRegionOffset,
					  lmap.relGotSize);

    // Allocate space for a GOT Region, if necessary
    Offset gotLayoutOffset = 0;
    lmap.gotSize = getGOTSize(target, lmap, gotLayoutOffset);
    if( lmap.gotSize > 0 ) {
      // We may copy the original GOT over; in this case, we have to be sure we skip it
      // when we lay out new GOT entries. That's the gotLayoutOffset value.
	lmap.gotRegionAlign = getGOTAlign(lmap);
        lmap.gotRegionOffset = currentOffset;
        currentOffset += lmap.gotSize;
        layoutRegions(lmap.gotRegions, lmap.regionAllocs,
		      lmap.gotRegionOffset + gotLayoutOffset, globalOffset);
	calculateTOCs(target, lmap.gotRegions, lmap.gotRegionOffset, gotLayoutOffset, globalOffset);
    }

    // Calculate the space necessary for stub code; normally this will be 0,
    // but we may need to add code later on. Make room for it now.
    // Note we use code region alignment here.
    lmap.stubRegionOffset = currentOffset;
    lmap.stubRegionOffset += computePadding(globalOffset + lmap.stubRegionOffset,
					    lmap.codeRegionAlign);
    rewrite_printf("Before allocStubRegions, currentOffset is 0x%lx\n", currentOffset);
    currentOffset = allocStubRegions(lmap, globalOffset);
    rewrite_printf("After allocStubRegions, currentOffset is 0x%lx\n", currentOffset);
    if( currentOffset == ~0UL ) {
        err = Storage_Allocation_Failure;
        errMsg = "assumption failed while creating link map";
        return false;
    }
    lmap.stubSize = currentOffset - lmap.stubRegionOffset;

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

    /*
     * Find current TLS Regions in target, also check for multiple TLS Regions
     * of the same type => knowing how to construct the TLS image would be
     * undefined for multiple TLS Regions of the same type
     *
     * Find current constructor Regions and destructor Regions.
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
                lmap.tlsSize += dataTLS->getMemSize();
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
                lmap.tlsSize += bssTLS->getMemSize();
                if( bssTLS->getMemAlignment() > lmap.tlsRegionAlign ) {
                    lmap.tlsRegionAlign = bssTLS->getMemAlignment();
                }
            }
        }else if( isConstructorRegion(*reg_it) ) {
            lmap.originalCtorRegion = *reg_it;
        }else if( isDestructorRegion(*reg_it) ){
            lmap.originalDtorRegion = *reg_it;
        }
    }
#if defined(arch_x86) || defined(arch_x86_64) || defined(arch_power)
    // Allocate the new TLS region, if necessary
    if( lmap.tlsRegions.size() > 0 ) {
        lmap.tlsRegionOffset = currentOffset;

        lmap.tlsRegionOffset += computePadding(lmap.tlsRegionOffset + globalOffset,
                lmap.tlsRegionAlign);
        currentOffset = layoutTLSImage(globalOffset, dataTLS, bssTLS, lmap);

        if( currentOffset == lmap.tlsRegionOffset ) {
            err = Storage_Allocation_Failure;
            errMsg = "failed to create TLS initialization image";
            return false;
        }

        /*
         *  The size of this Region is counted twice: once when collecting the
         *  original TLS Regions and once when laying out the image. This is
         *  necessary to maintain consistency with the case where there are no
         *  new TLS regions in the relocatable objects but existing TLS symbols
         *  are used.
         */
        if( dataTLS != NULL ) {
            lmap.tlsSize -= dataTLS->getMemSize();
        }

        lmap.tlsSize += currentOffset - lmap.tlsRegionOffset;

        // Adjust offsets for all existing TLS symbols, as their offset
        // in the TLS block could have changed
        vector<Symbol *> definedSyms;
        if( target->getAllDefinedSymbols(definedSyms) ) {
            vector<Symbol *>::iterator sym_it;
            for(sym_it = definedSyms.begin(); sym_it != definedSyms.end(); ++sym_it) {
                if( (*sym_it)->getType() == Symbol::ST_TLS ) {

                    map<Region *, LinkMap::AllocPair>::iterator result;
                    result = lmap.regionAllocs.find((*sym_it)->getRegion());

                    if( result != lmap.regionAllocs.end() ) {
                        Offset regionOffset = result->second.second;
                        Offset symbolOffset = (*sym_it)->getOffset();
                        lmap.origSymbols.push_back(make_pair(*sym_it, symbolOffset));

                        symbolOffset += regionOffset - lmap.tlsRegionOffset;
                        symbolOffset = adjustTLSOffset(symbolOffset, lmap.tlsSize);

                        (*sym_it)->setOffset(symbolOffset);
                    }
                }
            }
        }

        cleanupTLSRegionOffsets(lmap.regionAllocs, dataTLS, bssTLS);
        if( bssTLS != NULL ) lmap.tlsSize -= bssTLS->getMemSize();

        hasRewrittenTLS_ = true;
    }else{
        // Adjust offsets for all existing TLS symbols, as the offsets required
        // by relocation processing are TLS variant dependent
        vector<Symbol *> definedSyms;
        if( target->getAllDefinedSymbols(definedSyms) ) {
            vector<Symbol *>::iterator sym_it;
            for(sym_it = definedSyms.begin(); sym_it != definedSyms.end(); ++sym_it) {
                if( (*sym_it)->getType() == Symbol::ST_TLS ) {
                    Offset symbolOffset = (*sym_it)->getOffset();
                    lmap.origSymbols.push_back(make_pair(*sym_it, symbolOffset));

                    symbolOffset = adjustTLSOffset(symbolOffset, lmap.tlsSize);
                    (*sym_it)->setOffset(symbolOffset);
                }
            }
        }

        // The size of the original TLS image is no longer needed
        lmap.tlsSize = 0;
    }

#endif
    // Allocate space for a new constructor region, if necessary
    if( lmap.newCtorRegions.size() > 0 ) {
        lmap.ctorRegionOffset = currentOffset;
        lmap.ctorRegionOffset += computePadding(globalOffset + lmap.ctorRegionOffset,
                lmap.ctorRegionAlign);
        currentOffset = layoutNewCtorRegion(lmap);
        if( currentOffset == ~0UL ) {
            err = Storage_Allocation_Failure;
            errMsg = "assumption failed while creating link map";
            return false;
        }
        lmap.ctorSize = currentOffset - lmap.ctorRegionOffset;
    }

    // Allocate space for a new destructor region, if necessary
    if( lmap.newDtorRegions.size() > 0 ) {
        lmap.dtorRegionOffset = currentOffset;
        lmap.dtorRegionOffset += computePadding(globalOffset + lmap.dtorRegionOffset,
                lmap.dtorRegionAlign);
        currentOffset = layoutNewDtorRegion(lmap);
        if( currentOffset == ~0UL ) {
            err = Storage_Allocation_Failure;
            errMsg = "assumption failed while creating link map";
            return false;
        }
        lmap.dtorSize = currentOffset - lmap.dtorRegionOffset;
    }

    /* Combine all COMMON symbols into a single block */
    if( commonAlignments.size() > 0 ) {
        // The alignment of the COMMON Region could be the alignment
        // for the bss Region
        if( commonRegionAlign > lmap.bssRegionAlign ) {
            lmap.bssRegionAlign = commonRegionAlign;
        }

        // The following approach to laying out the COMMON symbols is greedy and
        // suboptimal (in terms of space in the target), but it's quick...
        Offset commonOffset = currentOffset;

        // Make sure the COMMON Region's alignment is satisfied
        commonOffset += computePadding(globalOffset + commonOffset, lmap.bssRegionAlign);
        Offset commonStartOffset = commonOffset;

        multimap<Offset, Symbol *>::iterator align_it;
        for(align_it = commonAlignments.begin();
            align_it != commonAlignments.end(); ++align_it)
        {
            commonOffset += computePadding(commonOffset, align_it->first);
            lmap.origSymbols.push_back(make_pair(align_it->second, align_it->first));

            // Update symbol with place in new linked data
            align_it->second->setOffset(globalOffset + commonOffset);
            commonOffset += align_it->second->getSize();
        }

        // Update the size of COMMON storage
        if( commonAlignments.size() > 0 ) {
            // A COMMON region is not really complete
            lmap.commonStorage = Region::createRegion(0, Region::RP_RW,
                    Region::RT_BSS, commonOffset - commonStartOffset, 0,
                    commonOffset - commonStartOffset,
                    DEFAULT_COM_NAME, NULL, true, false, commonRegionAlign);
            lmap.bssRegions.push_front(lmap.commonStorage);
        }
    }

    // Allocate bss regions
    lmap.bssRegionOffset = currentOffset;
    lmap.bssRegionOffset += computePadding(globalOffset + lmap.bssRegionOffset,
            lmap.bssRegionAlign);
    currentOffset = layoutRegions(lmap.bssRegions, lmap.regionAllocs,
            lmap.bssRegionOffset, globalOffset);
    if( currentOffset == ~0UL ) {
        err = Storage_Allocation_Failure;
        errMsg = "assumption failed while creating link map";
        return false;
    }

    lmap.bssSize = currentOffset - lmap.bssRegionOffset;

    // Update all relevant symbols with their offsets in the new target
    for(obj_it = relocatableObjects.begin(); obj_it != relocatableObjects.end(); ++obj_it) {
        vector<Symbol *> definedSyms;
        if( (*obj_it)->getAllDefinedSymbols(definedSyms) ) {
            vector<Symbol *>::iterator sym_it;
            for(sym_it = definedSyms.begin(); sym_it != definedSyms.end(); ++sym_it) {
                if(    !(*sym_it)->isCommonStorage()
                    && (*sym_it)->getType() != Symbol::ST_TLS)
                {
                    map<Region *, LinkMap::AllocPair>::iterator result;
                    result = lmap.regionAllocs.find((*sym_it)->getRegion());
                    if( result != lmap.regionAllocs.end() ) {
                        Offset regionOffset = result->second.second;
                        Offset symbolOffset = (*sym_it)->getOffset();
                        lmap.origSymbols.push_back(make_pair(*sym_it, symbolOffset));

                        symbolOffset += globalOffset + regionOffset;
// If region has relocations, set offset to got
			vector<relocationEntry> region_rels = result->first->getRelocations();
			if(region_rels.size() > 0) {
//			printf(" region %s has relocations %d \n", result->first->getRegionName().c_str(), region_rels.size());
/*
                    		result = lmap.regionAllocs.find((*sym_it)->getRegion());
                        	regionOffset = result->second.second;
*/
			}


                        (*sym_it)->setOffset(symbolOffset);
                    }
                }
            }
        }
    }

    /* ASSUMPTION
     * At this point, the layout of the new regions is fixed, and
     * addresses of all symbols are known (excluding Constructor/Destructor Regions)
     */

    // Allocate storage space
    lmap.allocatedData = new char[currentOffset];
    lmap.allocatedSize = currentOffset;

    // Copy the Regions from the relocatable objects into the new storage space
    copyRegions(lmap);

    return true;
}

/**
 * Lays out the specified regions, storing the layout info in the
 * passed map.
 *
 * regions              A collection of Regions to layout
 * regionAllocs         A map of Regions to their layout information
 * currentOffset        The starting offset for the passed Regions in
 *                      the new storage space
 * globalOffset         The location of the new storage space in the
 *                      target (used for padding calculation)
 */
Offset emitElfStatic::layoutRegions(deque<Region *> &regions,
                                    map<Region *, LinkMap::AllocPair> &regionAllocs,
                                    Offset currentOffset, Offset globalOffset)
{
    Offset retOffset = currentOffset;

    deque<Region *>::iterator copyReg_it;
    for(copyReg_it = regions.begin(); copyReg_it != regions.end(); ++copyReg_it) {
        // Skip empty Regions
        if( (*copyReg_it)->getMemSize() == 0 ) continue;

        // Make sure the Region is aligned correctly in the new aggregate Region
        Offset padding = computePadding(globalOffset + retOffset, (*copyReg_it)->getMemAlignment());
        retOffset += padding;

        // Set up mapping for a new Region in the specified storage space
        pair<map<Region *, LinkMap::AllocPair>::iterator, bool> result;
        result = regionAllocs.insert(make_pair(*copyReg_it, make_pair(padding, retOffset)));

        // If the map already contains this Region, this is a logic error
        if( !result.second ) {
            retOffset = ~0UL;
            break;
        }

        retOffset += (*copyReg_it)->getMemSize();
    }

    return retOffset;
}

/*
 * Adds new combined Regions to the target at the specified globalOffset
 *
 * target       The Symtab object to which the new Regions will be added
 * globalOffset The offset of the first new Region in the target
 * lmap         Contains all the information about the LinkMap
 */
bool emitElfStatic::addNewRegions(Symtab *target, Offset globalOffset, LinkMap &lmap) {
    char *newTargetData = lmap.allocatedData;

#if defined(arch_x86) || defined(arch_x86_64) || \
    defined(arch_aarch64) || (defined(arch_power) && defined(arch_64bit))
    if( lmap.gotSize > 0 ) {
       buildGOT(target, lmap);
        target->addRegion(globalOffset + lmap.gotRegionOffset,
                reinterpret_cast<void *>(&newTargetData[lmap.gotRegionOffset]),
                static_cast<unsigned int>(lmap.gotSize),
                GOT_NAME, Region::RT_DATA, true, lmap.gotRegionAlign);
    }
#endif
    if( lmap.pltSize > 0 ) {
      target->addRegion(globalOffset + lmap.pltRegionOffset,
			reinterpret_cast<void *>(&newTargetData[lmap.pltRegionOffset]),
			static_cast<unsigned int>(lmap.pltSize),
			PLT_NAME, Region::RT_TEXTDATA, true, lmap.pltRegionAlign);
    }

    if (lmap.relSize > 0) {
      Region::RegionType type;
      if (addressWidth_ == 8) {
	type = Region::RT_RELA;
      }
      else {
	type = Region::RT_REL;
      }
      target->addRegion(globalOffset + lmap.relRegionOffset,
			reinterpret_cast<void *>(&newTargetData[lmap.relRegionOffset]),
			static_cast<unsigned int>(lmap.relSize),
			REL_NAME, type, true, lmap.relRegionAlign);

    }

    if (lmap.relGotSize > 0) {
      target->addRegion(globalOffset + lmap.relGotRegionOffset,
			reinterpret_cast<void *>(&newTargetData[lmap.relGotRegionOffset]),
			static_cast<unsigned int>(lmap.relGotSize),
			REL_GOT_NAME, Region::RT_TEXTDATA, true, lmap.relGotRegionAlign);
    }

    if( lmap.codeSize > 0 ) {
        target->addRegion(globalOffset + lmap.codeRegionOffset,
                reinterpret_cast<void *>(&newTargetData[lmap.codeRegionOffset]),
                static_cast<unsigned int>(lmap.codeSize),
                CODE_NAME, Region::RT_TEXTDATA, true, lmap.codeRegionAlign);
    }
    if (lmap.stubSize > 0) {
        target->addRegion(globalOffset + lmap.stubRegionOffset,
			  reinterpret_cast<void *>(&newTargetData[lmap.stubRegionOffset]),
			  static_cast<unsigned int>(lmap.stubSize),
			  STUB_NAME, Region::RT_TEXTDATA, true, lmap.codeRegionAlign);
    }

    if( lmap.dataSize > 0 ) {
        target->addRegion(globalOffset + lmap.dataRegionOffset,
                reinterpret_cast<void *>(&newTargetData[lmap.dataRegionOffset]),
                static_cast<unsigned int>(lmap.dataSize),
                DATA_NAME, Region::RT_DATA, true, lmap.dataRegionAlign);
    }

#if defined(arch_x86) || defined(arch_x86_64)  || defined(arch_power)
    if( lmap.tlsSize > 0 ) {
        target->addRegion(globalOffset + lmap.tlsRegionOffset,
                reinterpret_cast<void *>(&newTargetData[lmap.tlsRegionOffset]),
                static_cast<unsigned int>(lmap.tlsSize),
                TLS_DATA_NAME, Region::RT_DATA, true, lmap.tlsRegionAlign, true);
    }
#endif
    if( lmap.newCtorRegions.size() > 0 ) {
        if( !createNewCtorRegion(lmap) ) {
            return false;
        }

        target->addRegion(globalOffset + lmap.ctorRegionOffset,
                reinterpret_cast<void *>(&newTargetData[lmap.ctorRegionOffset]),
                static_cast<unsigned int>(lmap.ctorSize),
                CTOR_NAME, Region::RT_DATA, true,
                lmap.ctorRegionAlign);
    }

    if( lmap.newDtorRegions.size() > 0 ) {
        if( !createNewDtorRegion(lmap) ) {
            return false;
        }

        target->addRegion(globalOffset + lmap.dtorRegionOffset,
                reinterpret_cast<void *>(&newTargetData[lmap.dtorRegionOffset]),
                static_cast<unsigned int>(lmap.dtorSize),
                DTOR_NAME, Region::RT_DATA, true,
                lmap.dtorRegionAlign);
    }

    if( lmap.bssSize > 0 ) {
        target->addRegion(globalOffset + lmap.bssRegionOffset,
                reinterpret_cast<void *>(&newTargetData[lmap.bssRegionOffset]),
                static_cast<unsigned int>(lmap.bssSize),
                BSS_NAME, Region::RT_BSS, true, lmap.bssRegionAlign);
    }

    return true;
}

/**
 * Copys the new Regions, as indicated by the LinkMap, into the
 * allocated storage space
 *
 * lmap         Contains all the information necessary to perform the copy
 */
void emitElfStatic::copyRegions(LinkMap &lmap) {
    char *targetData = lmap.allocatedData;

    map<Region *, LinkMap::AllocPair>::iterator regOff_it;
    for(regOff_it = lmap.regionAllocs.begin(); regOff_it != lmap.regionAllocs.end(); ++regOff_it) {
        Region *depRegion = regOff_it->first;
        Offset regionOffset = regOff_it->second.second;
        Offset padding = regOff_it->second.first;

        // Copy in the Region data
        char *rawRegionData = reinterpret_cast<char *>(depRegion->getPtrToRawData());

        if( !depRegion->isBSS() ) {
            memcpy(&targetData[regionOffset], rawRegionData, depRegion->getMemSize());
        }

        // Set the padded space to a meaningful value
        memset(&targetData[regionOffset - padding], getPaddingValue(depRegion->getRegionType()), padding);
    }
}

/**
 * Computes the padding necessary to satisfy the specified alignment
 *
 * candidateOffset      A possible offset for an item
 * alignment            The alignment for an item
 */
inline
Offset emitElfStatic::computePadding(Offset candidateOffset, Offset alignment) {
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
 *
 * target               The Symtab object being rewritten
 * relocatableObjects   A list of relocatable files being linked into target
 * globalOffset         The location of the new storage space in target
 * lmap                 Contains all the information necessary to apply relocations
 */
bool emitElfStatic::applyRelocations(Symtab *target, vector<Symtab *> &relocatableObjects,
				     Offset globalOffset, LinkMap &lmap,
				     StaticLinkError &err, string &errMsg)
{
    // Iterate over all relocations in all relocatable files
    vector<Symtab *>::iterator depObj_it;
    for(depObj_it = relocatableObjects.begin(); depObj_it != relocatableObjects.end(); ++depObj_it) {
        vector<Region *> allRegions;
        (*depObj_it)->getAllRegions(allRegions);

        // Relocations are stored with the Region to which they will be applied
        // As an ELF example, .rel.text relocations are stored with the Region .text
        vector<Region *>::iterator region_it;
        for(region_it = allRegions.begin(); region_it != allRegions.end(); ++region_it) {
           // Only compute relocations for the new Regions
           bool isText = ((*region_it)->getRegionType() == Region::RT_TEXT);
           bool isTOC = false;

           string regionName = (*region_it)->getRegionName();
           if (regionName.compare(0, 4, ".toc") == 0) isTOC = true;

           if (!isText && !isTOC) {
	     // Just process all regions; if there are still relocations lurking it had
	     // better be because something got modified...
	     //   continue;
           }
        map<Region *, LinkMap::AllocPair>::iterator result;
        result = lmap.regionAllocs.find(*region_it);
	if( result != lmap.regionAllocs.end() ) {

		Offset regionOffset = result->second.second;
                vector<relocationEntry> region_rels = (*region_it)->getRelocations();

                vector<relocationEntry>::iterator rel_it;
                for(rel_it = region_rels.begin(); rel_it != region_rels.end(); ++rel_it) {
                    // Compute destination of relocation
                    Offset dest = regionOffset + rel_it->rel_addr();
                    Offset relOffset = globalOffset + dest;

		    rewrite_printf("Computing relocations to apply to region: %s (%s) @ 0x%lx reloffset 0x%lx dest 0x%lx  \n\n",
				   (*region_it)->getRegionName().c_str(),
				   (*region_it)->symtab()->file().c_str(),
				   regionOffset,
				   relOffset,
				   dest);
		    rewrite_printf("\t RelOffset computed as region 0x%lx + rel_addr 0x%lx + globalOffset 0x%lx\n",
				   regionOffset, rel_it->rel_addr(), globalOffset);

                    char *targetData = lmap.allocatedData;

                    if( !archSpecificRelocation(target, *depObj_it,
						targetData, *rel_it, dest,
						relOffset, globalOffset, lmap, errMsg) )
                    {
                        err = Relocation_Computation_Failure;
                        errMsg = "Failed to compute relocation: " + errMsg;
                        return false;
                    }
/*
		   if((*region_it)->getRegionName().compare(".toc") == 0){
			Offset regOffset = lmap.gotRegionOffset;
                        dest = regOffset + rel_it->rel_addr();
                        relOffset = globalOffset + dest;

		    if((*rel_it).name().compare("__SYMTABAPI_CTOR_LIST__") == 0)
			{
                   	printf("\n\tComputing relocations to apply to region: %s @ 0x%lx reloffset %d 0x%lx dest %d 0x%lx  \n\n",
                        (*region_it)->getRegionName().c_str(), regOffset, relOffset,relOffset, dest,dest);
                    if( !archSpecificRelocation(target, *depObj_it, targetData, *rel_it, dest,
                                relOffset, globalOffset, lmap, errMsg) )
                    {
                        err = Relocation_Computation_Failure;
                        errMsg = "Failed to compute relocation: " + errMsg;
                        return false;
                    }
			}

		   }
*/
                }
            }
        }
    }

    rewrite_printf("\n*** Computing relocations added to target.\n\n");

    // Compute relocations added to target
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
	    // Don't process relocations for other sections; those get handled by the
	    // stub code.
	    if ((rel_it->rel_addr() < (*reg_it)->getMemOffset()) ||
		(rel_it->rel_addr() >= ((*reg_it)->getMemOffset() + (*reg_it)->getMemSize()))) continue;
            if( !archSpecificRelocation(target, target, regionData, *rel_it,
                        rel_it->rel_addr() - (*reg_it)->getDiskOffset(),
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

/**
 * A string representation of the StaticLinkError returned by
 * other functions
 */
string emitElfStatic::printStaticLinkError(StaticLinkError err) {
    switch(err) {
        CASE_RETURN_STR(No_Static_Link_Error);
        CASE_RETURN_STR(Symbol_Resolution_Failure);
        CASE_RETURN_STR(Relocation_Computation_Failure);
        CASE_RETURN_STR(Storage_Allocation_Failure);
        default:
            return "unknown error";
    }
}

/**
 * Indicates if a new TLS initialization image has been created
 */
bool emitElfStatic::hasRewrittenTLS() const {
    return hasRewrittenTLS_;
}

/** The following functions are all somewhat architecture-specific */

/* TLS Info
 *
 * TLS handling is pseudo-architecture dependent. The implementation of the TLS
 * functions depend on the implementation of TLS on a specific architecture.
 *
 * The following material is documented in more detail in the
 * "ELF Handling For TLS" white paper. According to this paper, their are
 * two variants w.r.t. creating a TLS initialization image.
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
 * TLS initialization image for an object (in this context an executable or
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
 * An image is:
 *
 * +--------+--------+
 * | DATA   |  BSS   |
 * +--------+--------+
 *
 * New TLS data and bss is added to the original initialization image as follows:
 *
 * +----------+------------------+-------------+------------+-----+
 * | NEW DATA | EXPANDED NEW BSS | TARGET DATA | TARGET BSS | TCB |
 * +----------+------------------+-------------+------------+-----+
 *
 * It is important to note that the TARGET DATA and TARGET BSS blocks are not moved.
 * This ensures that the modifications to the TLS image are safe.
 *
 * ==========================
 *
 * These are the two variants one would see when working with ELF files. So, an
 * architecture either uses variant 1 or 2.
 *
 */

/* See architecture specific functions that call these for descriptions of function interface */

Offset emitElfStatic::tlsLayoutVariant1(Offset globalOffset, Region *dataTLS, Region *bssTLS, LinkMap &lmap)
{
    // The original init. image needs to remain in the image 1 slot because
    // the TLS data references are relative to that position
    if( dataTLS != NULL ) {
	    lmap.tlsRegions.push_front(dataTLS);
    }
    deque<Region *> tlsRegionsVar;

    deque<Region *>::iterator copyReg_it;
    for(copyReg_it = lmap.tlsRegions.begin(); copyReg_it != lmap.tlsRegions.end(); ++copyReg_it) {
         tlsRegionsVar.push_front(*copyReg_it);
    }

    // Create the image, note new BSS regions are expanded
    Offset endOffset = layoutRegions(lmap.tlsRegions,
            lmap.regionAllocs, lmap.tlsRegionOffset, globalOffset);
    if( endOffset == ~0UL ) return lmap.tlsRegionOffset;

    // This is necessary so the offsets of existing TLS symbols can be updated
    // in a uniform, architecture independent way
    if( bssTLS != NULL ) {
        if( dataTLS != NULL ) {
            lmap.regionAllocs.insert(make_pair(bssTLS, lmap.regionAllocs[dataTLS]));
        }else{
            lmap.regionAllocs.insert(make_pair(bssTLS, make_pair(0, endOffset)));
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
        lmap.origSymbols.push_back(make_pair((*sym_it), symbolOffset));
        symbolOffset += (regionOffset - lmap.tlsRegionOffset);
        (*sym_it)->setOffset(symbolOffset);
    }

    return endOffset;
}

Offset emitElfStatic::tlsLayoutVariant2(Offset globalOffset, Region *dataTLS, Region *bssTLS,
        LinkMap &lmap)
{
    // The original init. image needs to remain in the image 1 slot because
    // the TLS data references are relative to that position
    unsigned long tlsBssSize = 0;
    if( dataTLS != NULL ) lmap.tlsRegions.push_back(dataTLS);
    if( bssTLS != NULL ) tlsBssSize = bssTLS->getMemSize();

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
            lmap.regionAllocs.insert(make_pair(bssTLS, make_pair(0, endOffset)));
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
        lmap.origSymbols.push_back(make_pair((*sym_it), symbolOffset));

        symbolOffset += (regionOffset - lmap.tlsRegionOffset) - (adjustedEnd + tlsBssSize);
        (*sym_it)->setOffset(symbolOffset);
    }

    return endOffset;
}

// Note: Variant 1 does not require any modifications, so a separate
// function is not necessary
Offset emitElfStatic::tlsAdjustVariant2(Offset curOffset, Offset tlsSize) {
    // For Variant 2, offsets relative to the TCB need to be negative
    Offset retOffset = curOffset;
    retOffset -= tlsSize;
    return retOffset;
}

void emitElfStatic::tlsCleanupVariant1(map<Region *, LinkMap::AllocPair> &regionAllocs,
        Region *, Region *bssTLS)
{
    // Existing BSS TLS region is not copied to the new target data
    if( bssTLS != NULL ) regionAllocs.erase(bssTLS);
}

void emitElfStatic::tlsCleanupVariant2(map<Region *, LinkMap::AllocPair> &regionAllocs,
        Region *, Region *bssTLS)
{
    // Existing BSS TLS region is not copied to the new target data
    if( bssTLS != NULL ) regionAllocs.erase(bssTLS);
}

bool
emitElfUtils::sort_reg(const Region* a, const Region* b)
{
    if (a->getMemOffset() == b->getMemOffset())
        return a->getMemSize() < b->getMemSize();
    return a->getMemOffset() < b->getMemOffset();
}

/*
 * Sort the sections array so that sections with a non-zero
 * memory offset come first (and are sorted in increasing
 * order of offset). Preserves the ordering of zero-offset
 * sections.
 *
 * If no section has a non-zero offset, the return value will
 * be an address in the virtual memory space big enough to
 * hold all the loadable sections. Otherwise it will be the
 * address of the first non-zero offset section.
 *
 * NB if we need to create a new segment to hold these sections,
 * it needs to be clear up to the next page boundary to avoid
 * potentially clobbering other loadable segments.
 */
Address
emitElfUtils::orderLoadableSections(Symtab *obj, vector<Region*> & sections)
{
    Address ret = 0;
    Address sz = 0;
    vector<Region*> nonzero;
    vector<Region*> copy(sections);
    unsigned insert = sections.size()-1;

    for(int i=copy.size()-1;i>=0;--i) {
        if(!copy[i]->getMemOffset())
            sections[insert--] = copy[i];
        else
            nonzero.push_back(copy[i]);
        sz += copy[i]->getMemSize();
    }

    assert(nonzero.size() == (insert+1));

    std::sort(nonzero.begin(),nonzero.end(),sort_reg);
    for(unsigned i=0;i<nonzero.size();++i)
        sections[i] = nonzero[i];

    if(nonzero.size() > 0)
        ret = nonzero[0]->getMemOffset();
    else {
        // find a `hole' of appropriate size
        unsigned pgSize = P_getpagesize();
        sz = (sz + pgSize - 1) & ~(pgSize-1);
        ret = obj->getFreeOffset(sz);
    }
    return ret;
}

// There are also some known variables that point to the heap
bool emitElfUtils::updateHeapVariables(Symtab *obj, unsigned long newSecsEndAddress ) {
    unsigned pgSize = (unsigned)getpagesize();
    const std::string minbrk(".minbrk");
    const std::string curbrk(".curbrk");
    std::vector<Symbol *> heapSyms;
    obj->findSymbol(heapSyms, minbrk, Symbol::ST_NOTYPE);
    obj->findSymbol(heapSyms, curbrk, Symbol::ST_NOTYPE);

    std::vector<Symbol *>::iterator heapSymsIter;
    for(heapSymsIter = heapSyms.begin();
      heapSymsIter != heapSyms.end();
      ++heapSymsIter)
    {
        Region *symRegion = (*heapSymsIter)->getRegion();
        Offset symOffset = (*heapSymsIter)->getOffset();
        if( symOffset < symRegion->getDiskOffset() ) continue;

        Offset regOffset = symOffset - symRegion->getDiskOffset();
        unsigned char *regData = reinterpret_cast<unsigned char *>(symRegion->getPtrToRawData());
        Offset heapAddr;
        memcpy(&heapAddr, &regData[regOffset], sizeof(Offset));

        heapAddr = (newSecsEndAddress & ~(pgSize-1)) + pgSize;

        if( !symRegion->patchData(regOffset, &heapAddr, sizeof(Offset)) ) {
            rewrite_printf("Failed to update heap address\n");
            return false;
        }
    }

    return true;
}


bool emitElfStatic::calculateTOCs(Symtab *target, deque<Region *> &regions, Offset GOTstart, Offset newStart, Offset globalOffset) {
  rewrite_printf("Calculating TOCs for merged GOT sections, base is 0x%lx, new regions at 0x%lx\n",
		 GOTstart + globalOffset,
		 GOTstart + newStart + globalOffset);

  Offset GOTbase = GOTstart + globalOffset;
  Offset current = GOTbase + newStart;
  Offset currentTOC = target->getTOCoffset((Offset) 0);
  rewrite_printf("\tBase TOC is 0x%lx\n", currentTOC);

  for (deque<Region *>::iterator iter = regions.begin(); iter != regions.end(); ++iter) {
    Region *reg = *iter;
    Symtab *symtab = reg->symtab();

    Offset end = current + reg->getDiskSize();

    // We'd like the TOC to be GOTbase + 0x8000, so long as that can address the end of the current
    // region.
    if ((currentTOC + 0x7ff0) < end) {
      // Crud...
      // OTOH, we can't reference anything outside the current GOT, so just rebase it
      currentTOC = current + 0x8000;
    }
    // Recheck
    if ((currentTOC + 0x7ff0) < end) {
      assert(0 && "Need to implement -bbigtoc equivalent to rewrite this binary!");
    }
    symtab->setTOCOffset(currentTOC);
    current = end;
  }
  return true;
}

Offset emitElfStatic::allocatePLTEntries(std::map<Symbol *, std::pair<Offset, Offset> > &pltEntries,
				       Offset pltOffset,
				       Offset &size) {
  // For each Symtab
  //   For each indirect symbol
  //     Allocate a PLT entry for it
  //     Add it to entries
#if defined(arch_x86)
  unsigned entry_size = 16;
#elif defined(arch_x86_64)
  unsigned entry_size = 16;
#elif defined(arch_power)
  unsigned entry_size = 0;
#else
	unsigned entry_size = 0;
//#warning "AArch64:"
//#warning "steve: entry_size is tmp 0, need to be defined later"
//#error "Unknown architecture"
#endif

  Offset cur = pltOffset;
  for (auto iter = pltEntries.begin(); iter != pltEntries.end(); ++iter) {
    iter->second.first = cur;
    cur += entry_size;
  }

  size = cur - pltOffset;
  return cur;
}

bool emitElfStatic::addIndirectSymbol(Symbol *sym, LinkMap &lmap) {
  // Existence is what we care about at this point
  lmap.pltEntries[sym] = make_pair(0,0);
  return true;
}


bool emitElfStatic::buildPLT(Symtab *target, Offset globalOffset,
			     LinkMap &lmap, StaticLinkError &err,
			     string &errMsg) {
  (void)target; (void)globalOffset; (void)err; (void)errMsg; // unused
#if !defined(arch_x86_64)
  return lmap.pltEntries.empty();
#else
  unsigned char pltEntry[] = {0xff, 0x25, 0x00, 0x00, 0x00, 0x00, // jmp *<offset>, %rip
		     0x68, 0x00, 0x00, 0x00, 0x00, // pushq $0x0
		     0xe9, 0x00, 0x00, 0x00, 0x00}; // jmpq
  unsigned pltEntrySize = 16;
  unsigned gotOffset = 2;
  // Size of the first instruction for x86 platforms so the math works out right
  unsigned magicOffset = 6;

  char *data = lmap.allocatedData;

  unsigned index = 0;
  for (auto iter = lmap.pltEntries.begin(); iter != lmap.pltEntries.end(); ++iter) {
    char *entry = &(data[iter->second.first]);

    memcpy(entry, pltEntry, pltEntrySize);

    iter->second.second = lmap.relGotRegionOffset + (index * sizeof(void *)) + globalOffset;
    Offset curOffset = globalOffset + iter->second.first;
    int val = (int) (iter->second.second - curOffset - magicOffset);

    memcpy(entry+gotOffset, &val, sizeof(int));
    ++index;
  }
  return true;
#endif
}

Offset emitElfStatic::allocateRelocationSection(std::map<Symbol *, std::pair<Offset, Offset> > &entries,
						Offset relocOffset, Offset &size,
						Symtab *target) {
#if defined(arch_x86_64)
  unsigned relocSize;
  if (addressWidth_ == 8)
    relocSize = sizeof(Elf64_Rela);
  else
    relocSize = sizeof(Elf32_Rel);
#elif defined(arch_x86)
  // 32-bit only uses REL types
  unsigned relocSize = sizeof(Elf32_Rel);
#else
  size = 0;
  unsigned relocSize = 0;
  return relocOffset;
#endif

  Offset cur = relocOffset;

  Region *rela = NULL;
  if (addressWidth_ == 8)
    target->findRegion(rela, ".rela.plt");
  else
    target->findRegion(rela, ".rel.plt");

  unsigned relocEntries = entries.size() + (rela ? rela->getRelocations().size() : 0);
  cur += relocEntries * relocSize;

  size = cur - relocOffset;
  return cur;
}

Offset emitElfStatic::allocateRelGOTSection(const std::map<Symbol *, std::pair<Offset, Offset> > &entries,
					    Offset relocOffset, Offset &size) {
#if defined(arch_x86_64)
  unsigned relocSize = sizeof(Elf64_Rela);
  (void)relocSize; // unused?!
#else
  size = 0;
  return relocOffset;
#endif

  size = entries.size() * sizeof(void *);
  return relocOffset + size;
}


bool emitElfStatic::buildRela(Symtab *target, Offset globalOffset,
			     LinkMap &lmap, StaticLinkError &err,
			     string &errMsg) {
  (void)err; (void)errMsg; // unused
  if (lmap.relSize == 0) return true;

#if !defined(arch_x86_64)
  // TODO: implementation
  return false;
#endif

  if (addressWidth_ == 8) {
    // 64-bit uses RELA
    unsigned copied = 0;

    char *data = lmap.allocatedData;
    auto relas = alignas_cast<Elf64_Rela>(&(data[lmap.relRegionOffset]));

    Region *rela = NULL;
    target->findRegion(rela, ".rela.plt");
    if (rela) {
      memcpy(relas, rela->getPtrToRawData(), rela->getDiskSize());
      copied += rela->getDiskSize();
      relas = alignas_cast<Elf64_Rela>(&(data[lmap.relRegionOffset + rela->getDiskSize()]));
    }

    unsigned index = 0;
    for (auto iter = lmap.pltEntries.begin(); iter != lmap.pltEntries.end(); ++iter) {
      // Grab a GOT location
      relas[index].r_offset = iter->second.second;
      relas[index].r_info = ELF64_R_INFO((unsigned long) STN_UNDEF, R_X86_64_IRELATIVE);
      relas[index].r_addend = iter->first->getOffset();
      copied += sizeof(Elf64_Rela);
      ++index;
    }
    assert(copied == lmap.relSize);
  }
  else {
    // 32 bit uses REL
    unsigned copied = 0;

    char *data = lmap.allocatedData;
    auto *rels = alignas_cast<Elf32_Rel>(&(data[lmap.relRegionOffset]));

    Region *rel = NULL;
    target->findRegion(rel, ".rel.plt");
    if (rel) {
      memcpy(rels, rel->getPtrToRawData(), rel->getDiskSize());
      copied += rel->getDiskSize();
      rels = alignas_cast<Elf32_Rel>(&(data[lmap.relRegionOffset + rel->getDiskSize()]));
    }

    unsigned index = 0;
    for (auto iter = lmap.pltEntries.begin(); iter != lmap.pltEntries.end(); ++iter) {
      // Grab a GOT location
      rels[index].r_offset = iter->second.second;
      rels[index].r_info = ELF32_R_INFO((unsigned long) STN_UNDEF, R_386_IRELATIVE);
      // For a REL relocation, the addend goes in *offset instead of its own
      // slot...

      // So find the data address for rels[index].r_offset
      // and set it to iter->first->getOffset(), AKA the symbol address

      // We should be writing into .dyninstRELAgot; just have to figure out how.
      auto *got = alignas_cast<long>(&(data[rels[index].r_offset - globalOffset]));
      *got = iter->first->getOffset();

      copied += sizeof(Elf32_Rel);
      ++index;
    }
    assert(copied == lmap.relSize);
  }

  return true;
}
