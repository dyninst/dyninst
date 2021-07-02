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

#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_flowGraph.h"
#include "BPatch_basicBlockLoop.h"
#include "BPatch_object.h"
#include "BPatch_point.h"

#if defined(arch_x86) || defined(arch_x86_64)
#include "inst-x86.h"
#elif defined(arch_power)
#include "inst-power.h"
#else
#error "Unknown architecture, expected x86, x86_64, or power"
#endif

#include "stackanalysis.h"

#include "debug.h"
#include "StackMod.h"
#include "StackModExpr.h"
#include "StackModChecker.h"


StackModChecker::~StackModChecker() {
    // Free the height vectors in blockHeights
    for (auto bhIter = blockHeights.begin(); bhIter != blockHeights.end();
        bhIter++) {
        delete bhIter->second;
    }
}

/* Locate the libc-provided canary failure function __stack_chk_fail */
static BPatch_function* findCanaryFailureFunction(BPatch_image* image)
{
    BPatch_function* failFunction = NULL;

    /* Locate libc stack_chk_fail function for failure case */
    std::vector<BPatch_object*> objects;
    image->getObjects(objects);
    if (!objects.size()) {
        stackmods_printf("findCanaryFailureFunction: could not find any BPatch_objects\n");
        return failFunction;
    }

    std::string name = "__stack_chk_fail";

    for (auto iter = objects.begin(); iter != objects.end(); ++iter) {
        BPatch_object* obj = *iter;

        std::vector<BPatch_function*> funcs;
        obj->findFunction(name, funcs, false);
        if (funcs.size()) {
            failFunction = funcs[0];
            return failFunction;
        }
    }

    return failFunction;
}

bool StackModChecker::addModsInternal(std::set<StackMod*> mods)
{
    // Check architecture
    Architecture arch = func->ifunc()->isrc()->getArch();
    if ( (arch != Arch_x86) && (arch != Arch_x86_64)) {
        stackmods_printf("Stack modifications only implemented for x86 and x86_64!\n");
        return false;
    }

    if (!func->proc()->edit()) {
        stackmods_printf("Stack modifications only implemented for binary rewriting mode\n");
        return false;
    }

    if (func->hasRandomize()) {
        stackmods_printf("RANDOMIZE has already been applied to this function; additional modifications not allowed\n");
        return false;
    }

    stackmods_printf("addMods called with:\n");
    for (auto iter = mods.begin(); iter != mods.end(); ++iter) {
        stackmods_printf("\t %s\n", (*iter)->format().c_str());
    }

    if (!func->hasOffsetVector()) {
        // Add DWARF information to the low-level function
        BPatch_Vector<BPatch_localVar*>* params = bfunc->getParams();
        for (auto iter = params->begin(); iter != params->end(); ++iter) {
            BPatch_localVar* v = *iter;
            if (v) {
                func->addParam(v->getSymtabVar());
            }
        }
        BPatch_Vector<BPatch_localVar*>* vars  = bfunc->getVars();
        for (auto iter = vars->begin(); iter != vars->end(); ++iter) {
            BPatch_localVar* v = *iter;    
            if (v) {
                func->addVar(v->getSymtabVar());
            }
        }

        // Generate offset vector
        // This only needs to happen once per function
        func->createOffsetVector();
    }

    if (!func->hasValidOffsetVector()) {
        // Function cannot be modified
        stackmods_printf("Function %s cannot be modified, addMods returning false\n", getName().c_str());
        func->freeStackMod();
        return false;
    }

    // Create a local set to update
    std::vector<StackMod*> allMods;

    // Some modifications require a safety check to ensure that the stack only grows/shrinks once during the function
    bool requiresCheckStackGrowth = false;

    // Calculate any paired modification requirements (before alignment)
    //      TODO: Good way to check if the user is doing this cleanup already?
    //      Although canaries have 2 snippets, they don't require a second
    //      modification to be added (but two snippets will be generated)
    // Also check that modifications make sense
    //      Only one canary modification per function
    //      Randomize may not be used with any other modifications
    for (auto iter = mods.begin(); iter != mods.end(); ++iter) {
        StackMod* m = *iter;
        StackMod::MType type = m->type();
        stackmods_printf("Checking paired modifications for %s\n", m->format().c_str());

        switch(type) {
            case StackMod::INSERT:
                {
                    // Add paired REMOVE at function exit
                    Insert* insertMod = dynamic_cast<Insert*>(m);
                    Remove* removeMod = new Remove(StackMod::CLEANUP, insertMod->low(), insertMod->high());
                    stackmods_printf("\t Adding paired %s\n", removeMod->format().c_str());
                    allMods.push_back(removeMod);

                    // Will require a check for how the stack grows/shrinks
                    requiresCheckStackGrowth = true;
                    break;

                }
            case StackMod::REMOVE:
                {
                    // Add paired INSERT at function exit
                    Remove* removeMod = dynamic_cast<Remove*>(m);
                    Insert* insertMod = new Insert(StackMod::CLEANUP, removeMod->low(), removeMod->high());
                    stackmods_printf("\t Adding paired %s\n", insertMod->format().c_str());
                    allMods.push_back(insertMod);

                    // Will require a check for how the stack grows/shrinks
                    requiresCheckStackGrowth = true;
                    break;

                }
            case StackMod::MOVE:
                {
                    // Will require a check for how the stack grows/shrinks
                    requiresCheckStackGrowth = true;
                    break;
                }
            case StackMod::CANARY:
                {
#if !defined(os_linux)
                    // Safety check: canaries are Linux-only
                    func->freeStackMod();
                    return false;
#endif

                    // Some initialization now required
                    Canary* canaryMod = dynamic_cast<Canary*>(m);
                    if (arch == Arch_x86) {
                        canaryMod->init(-8, -4);
                    } else if (arch == Arch_x86_64) {
                        canaryMod->init(-8 - AMD64_STACK_ALIGNMENT, -8);
                    }

                    // Safety check: Only one CANARY modification may be performed

                    // Check remainder of mods
                    auto remainderIter = iter;
                    remainderIter++;
                    for ( ; remainderIter != mods.end(); ++remainderIter) {
                        if ((*remainderIter)->type() == StackMod::CANARY) {
                            stackmods_printf("Found 2 CANARY modifications, not allowed\n");
                            func->freeStackMod();
                            return false;
                        }
                    }

                    // Check existing modifications
                    if (func->hasCanary()) {
                        stackmods_printf("Found existing CANARY modification, not allowed\n");
                        func->freeStackMod();
                        return false;
                    }
                    break;
                }
            case StackMod::RANDOMIZE:
                {
                    // Randomize may not be performed with any other stack modifcations
                    if (mods.size() != 1 || func->hasStackMod()) {
                        stackmods_printf("Found RANDOMIZE with other modifications, not allowed\n");
                        func->freeStackMod();
                        return false;
                    }
                    break;
                }
            default:
                {
                    stackmods_printf("addMods called with unknown stack modification type, returning false\n");
                    func->freeStackMod();
                    return false;
                }
        }
    }

    // The canary has to be the "middle" modification because the snippets are inserted together
    for (auto iter = mods.rbegin(); iter != mods.rend(); ++iter) {
        if ((*iter)->type() == StackMod::CANARY) {
            allMods.push_back(*iter);
            break;
        }
    }

    // Add in original mods
    for (auto iter = mods.rbegin(); iter != mods.rend(); ++iter) {
        if ((*iter)->type() != StackMod::CANARY) {
            allMods.push_back(*iter);
        }
    }

    // Calculate any alignment requirements
    for (auto iter = allMods.begin(); iter != allMods.end(); ++iter) {
        if (!accumulateStackRanges(*iter)) {
            func->freeStackMod();
            return false;
        }
    }
    int alignment = 1;
    if (arch == Arch_x86_64) { alignment = AMD64_STACK_ALIGNMENT; }
    if (!alignStackRanges(alignment, StackMod::NEW, allMods)) {
        func->freeStackMod();
        return false;
    }
    if (!alignStackRanges(alignment, StackMod::CLEANUP, allMods)) {
        func->freeStackMod();
        return false;
    }

    stackmods_printf("final set of modifications:\n");
    for (auto iter = allMods.begin(); iter != allMods.end(); ++iter) {
        stackmods_printf("\t %s\n", (*iter)->format().c_str());
    }

    // Perform stack analysis so we have the information ready
    StackAnalysis sa(func->ifunc());

    // Other safety checks:
    //  - Does the stack grow/shrink more than once? For canaries, randomize this might be fine; 
    //    for the others, this could lead to ambiguity about where the modifications should happen
    if (requiresCheckStackGrowth) {
        if (_unsafeStackGrowth || !checkStackGrowth(sa)) {
            _unsafeStackGrowth = true;
            stackmods_printf("Found that the stack grows and shrinks multiple times in this function; INSERT,REMOVE,MOVE disallowed\n");
            func->freeStackMod();
            return false;
        } 
    }

    // Record the old Transformation mapping, in case we need to reset it
    TMap* oldTMap = func->getTMap(); 
    TMap* newTMap = new TMap(*oldTMap);

    // Keep track of inserted snippets, in case we need to remove them...
    std::vector<BPatchSnippetHandle*> snippets;
    
    // We may add an insert for the canary check; we need to then add this to allMods outside the loop
    StackMod* canaryCheckInsert = NULL;

    // Add modifications to the mod set and generate snippets...
    bool cleanupAndReturn = false;
    for (auto iter = allMods.begin(); iter != allMods.end(); ++iter) {
        // Analysis time!
        StackMod* m = *iter;
        stackmods_printf("Generating snippet for %s\n", m->format().c_str());

        StackMod::MType type = m->type();

        switch(type) {
            case StackMod::INSERT :
                {
                    long dispFromRSP = 0;
                    std::vector<BPatch_point*>* points;
                    if (!findInsertOrRemovePoints(sa, m, points, dispFromRSP)) {
                        stackmods_printf("\t findInsertOrRemovePoints failed\n");
                        func->freeStackMod();
                        return false;
                    }
                    
                    Insert* insert = dynamic_cast<Insert*>(m);
                    
                    for (auto point : *points) {
                        Address addr = (Address)(point->getAddress()); 
                        BPatch_snippet* snip = new BPatch_stackInsertExpr(insert->size());
                        stackmods_printf("\t Generating INSERT of size %u at 0x%lx\n", insert->size(), addr);
                        BPatchSnippetHandle* handle = bfunc->getAddSpace()->insertSnippet(*snip, *point, BPatch_callBefore, BPatch_lastSnippet);
                        if (!handle) {
                            stackmods_printf("\t\t insertSnippet failed\n");
                            cleanupAndReturn = true;
                            break;
                        } else {
                            snippets.push_back(handle);
                        }
                    }

                    if (m->order() == StackMod::NEW) {
                        func->addMod(m, newTMap);
                    } else {
                        // Cleanup operations don't get added to the TMap
                    }

                    break;
                }
            case StackMod::REMOVE :
                {
                    long dispFromRSP = 0;
                    std::vector<BPatch_point*>* points;
                    if (!findInsertOrRemovePoints(sa, m, points, dispFromRSP)) {
                        stackmods_printf("\t findInsertOrRemovePoints failed\n");
                        cleanupAndReturn = true;
                        break;
                    }

                    Remove* remove = dynamic_cast<Remove*>(m);
                    
                    for (auto point : *points) {
                        Address addr = (Address)(point->getAddress()); 
                        BPatch_snippet* snip = new BPatch_stackRemoveExpr(remove->size());
                        stackmods_printf("\t Generating REMOVE of size %u at 0x%lx\n", remove->size(), addr);
                        BPatchSnippetHandle* handle = bfunc->getAddSpace()->insertSnippet(*snip, *point);
                        if (!handle) {
                            stackmods_printf("\t\t insertSnippet failed\n");
                            cleanupAndReturn = true;
                            break;
                        } else {
                            snippets.push_back(handle);
                        }
                    }
                    
                    if (m->order() == StackMod::NEW) {
                        func->addMod(m, newTMap);
                    } else {
                        // Cleanup operations don't get added to the TMap
                    }

                    break;
                }
            case StackMod::MOVE :
                {
                    // Doesn't matter where this snippet is inserted, so pick entry
                    BPatch_point* point = NULL;
                    std::vector<BPatch_point*>* entryPoints = bfunc->findPoint(BPatch_entry);
                    if (entryPoints && entryPoints->size()) {
                        point = entryPoints->at(0);
                    } else {
                        cleanupAndReturn = true;
                        break;
                    }
                    BPatch_snippet* snip = new BPatch_stackMoveExpr();
                    stackmods_printf("\t Generating MOVE at function entry (0x%lx)\n", (Address)(point->getAddress()));
                    BPatchSnippetHandle* handle = bfunc->getAddSpace()->insertSnippet(*snip, *point);
                    if (!handle) {
                        stackmods_printf("\t\t insertSnippet failed\n");
                        cleanupAndReturn = true;
                        break;
                    }
                    
                    if (m->order() == StackMod::NEW) {
                        func->addMod(m, newTMap);
                    }

                    break;
                }
            case StackMod::CANARY :
                {
                    std::vector<BPatch_point*> insertPoints;
                    std::vector<BPatch_point*> checkPoints;
                    if (!findCanaryPoints(&insertPoints, &checkPoints)) {
                        cleanupAndReturn = true;
                        break;
                    }
                    
                    // If the insert and check point are the same, don't add the canary
                    // This isn't a failure, it's just unnecessary
                    if ( (insertPoints.size() == 1) && (checkPoints.size() == 1) &&
                            ((Address)(insertPoints.at(0)->getAddress()) == ((Address)(checkPoints.at(0)->getAddress())))) {
                        continue;
                    }
                    // Generate check snippets before insert snippets; set gets processed in LIFO order
                    Canary* canaryMod = dynamic_cast<Canary*>(m);
                    BPatch_function* failFunc = canaryMod->failFunc();
                   
                    // If a failure function wasn't provided, use __stack_chk_fail from libc
                    if (!failFunc) {
                        failFunc = findCanaryFailureFunction(bfunc->getAddSpace()->getImage());
                    }
                    
                    // If no failure function has been found, fail
                    if (!failFunc) {
                        cleanupAndReturn = true;
                        break;
                    }
                    for (auto point : checkPoints) {
                        Address addr = (Address)(point->getAddress());
                   
                        // We always insert a canary at entry
                        // To insert a canary elsewhere, set to true and update canaryHeight
                        bool canaryAfterPrologue = false;
                        long canaryHeight = 0;
                        if (canaryAfterPrologue) {
                            // This is the old handling code for non-entry canaries; needs to be updated
#if 0
                            ParseAPI::Function* pf = ParseAPI::convert(func);
                            StackAnalysis sa(pf);

                            ParseAPI::Block* pb = ParseAPI::convert(block);
                            assert(pb);
                            StackAnalysis::Height spHeight = sa.findSP(pb, addr);
                            if (spHeight.isBottom()) {
                                assert(0 && "spHeight is bottom and prologue_canary; this shouldn't happen");
                            }

                            fprintf(stderr, "BPatch_canaryCheckExpr: prologue_canary, spHeight = %s (decimal)\n", spHeight.format().c_str());

                            // Calculate the stack height of the canary
                            size_t width = func->getModule()->getAddressWidth();
                            canaryHeight = spHeight + width + width; // spHeight - RA - saved FP  
#endif
                        }
                        stackmods_printf("\t Generating CANARY CHECK at 0x%lx\n", addr);
                        BPatch_snippet* snip = new BPatch_canaryCheckExpr(failFunc, canaryAfterPrologue, canaryHeight);
                        BPatchSnippetHandle* handle = bfunc->getAddSpace()->insertSnippet(*snip, *point, BPatch_callAfter, BPatch_firstSnippet);
                        if (!handle) {
                            stackmods_printf("\t\t insertSnippet failed\n");
                            cleanupAndReturn = true;
                            break;
                        } else {
                            snippets.push_back(handle);
                        }
                    }

                    for (auto point : insertPoints) {
                        Address addr = (Address)(point->getAddress());
                        BPatch_snippet* snip = new BPatch_canaryExpr();
                        stackmods_printf("\t Generating CANARY at 0x%lx\n", addr);
                        BPatchSnippetHandle* handle = bfunc->getAddSpace()->insertSnippet(*snip, *point, BPatch_callBefore);
                        if (!handle) {
                            stackmods_printf("\t\t insertSnippet failed\n");
                            cleanupAndReturn = true;
                            break;
                        } else {
                            snippets.push_back(handle);
                        }
                    }

                    // This modification needs to get added to allMods
                    canaryCheckInsert = new Insert(canaryMod->low(), canaryMod->high());
                    func->addMod(canaryCheckInsert, newTMap);
                    func->setCanary(true);

                    break;
                }
            case StackMod::RANDOMIZE :
                {
                    BPatch_snippet* snip = new BPatch_stackRandomizeExpr();
                    // Doesn't matter where this snippet is inserted, so pick entry
                    BPatch_point* point = NULL;
                    std::vector<BPatch_point*>* entryPoints = bfunc->findPoint(BPatch_entry);
                    if (entryPoints && entryPoints->size()) {
                        point = entryPoints->at(0);
                    } else {
                        cleanupAndReturn = true;
                        break;
                    }
                    stackmods_printf("\t Generating RANDOMIZE at function entry (0x%lx)\n", (Address)(point->getAddress()));
                    BPatchSnippetHandle* handle = bfunc->getAddSpace()->insertSnippet(*snip, *point);
                    if (!handle) {
                        stackmods_printf("\t\t insertSnippet failed\n");
                        cleanupAndReturn = true;
                        break;
                    } else {
                        snippets.push_back(handle);
                    }

                    // Add modifications to support randomization
                    Randomize* randomizeMod = dynamic_cast<Randomize*>(m);
                    if (randomizeMod->isSeeded()) {
                        if (!func->randomize(newTMap, true, randomizeMod->seed())) {
                            stackmods_printf("Function %s notrandom\n", getName().c_str());
                            cleanupAndReturn = true;
                            break;
                        }
                    } else {
                        if (!func->randomize(newTMap)) {
                            stackmods_printf("Function %s notrandom\n", getName().c_str());
                            cleanupAndReturn = true;
                            break;
                        }
                    }
                    stackmods_printf("Function %s randomized\n", getName().c_str());
                    break;
                }
            default:
                {
                    // Shouldn't get here--this was checked earlier
                    cleanupAndReturn = true;
                    break;
                }
        }
    }

    if (canaryCheckInsert) {
        // This will be out-of-order in allMods, but the snippet insertion happened in the right order, so it's OK
        allMods.push_back(canaryCheckInsert);
    }

    // Unsafety analysis
    // TMap has been built as func_instance::addMod has been invoked for each modification; now, we just need to run the analysis
    if (!cleanupAndReturn && !areModificationsSafe()) {
        stackmods_printf("Modifications are not safe. Will not be added.\n");
        cleanupAndReturn = true;
    }

    // addMods should be atomic, so if there was a failure, we need to revert all changes performed by these modifications
    if (cleanupAndReturn) {
        stackmods_printf("One or more modifications were not successfully added. All modifications from the current set will be removed.\n");
        
        // Remove snippets
        for (auto iter = snippets.begin(); iter != snippets.end(); ++iter) {
            BPatchSnippetHandle* handle = *iter;
            if (!bfunc->getAddSpace()->deleteSnippet(handle)) {
                stackmods_printf("deleteSnippet failed; this is odd.\n");
            }
        }

        // Remove mods from the modification set (reverse func->addMods)
        for (auto iter = allMods.begin(); iter != allMods.end(); ++iter) {
            func->removeMod(*iter);
        }

        // The original TMap remained unchanged, so no reveision is necessary

        func->freeStackMod();
        return false; 
    }

    // Update the TMap
    func->replaceTMap(newTMap);

    // Free old TMap
    for (auto tmapIter = oldTMap->begin(); tmapIter != oldTMap->end();
        tmapIter++) {
        delete tmapIter->first;
        delete tmapIter->second;
    }
    delete oldTMap;

    // Free stack mod data structures (lots of memory)
    func->freeStackMod();

    func->setStackMod(true);
    return true;
}

/* Get the name of the function we're checking */
std::string StackModChecker::getName() const {
    return func->ifunc()->name();
}

/* Accumulate the effects of the current stack modification on the _stackRanges;
 * this gets used to determine if additional modification need to be added to 
 * preserve alignment requirements on the current platform.
 */
bool StackModChecker::accumulateStackRanges(StackMod* m)
{
    StackMod::MType curType = m->type();
    
    // Handle MOVE = INSERT + REMOVE
    if (curType == StackMod::MOVE) {
        Move* mod = dynamic_cast<Move*>(m);
        StackMod* tmpInsert = new Insert(mod->destLow(), mod->destHigh());
        if (!accumulateStackRanges(tmpInsert)) { return false; }
        StackMod* tmpRemove = new Remove(mod->srcLow(), mod->srcHigh());
        return accumulateStackRanges(tmpRemove);
    }

    // Handle CANARY = INSERT
    if (curType == StackMod::CANARY) {
        Canary* mod = dynamic_cast<Canary*>(m);
        StackMod* tmpInsert = new Insert(mod->low(), mod->high());
        return accumulateStackRanges(tmpInsert);
    }

    // RANDOMIZE has no effect on the stack ranges
    if (curType == StackMod::RANDOMIZE) {
        return true;
    }

    bool found = false;
    int removeRangeAt = -1;
    std::set<std::pair<int, pair<int, StackMod::MType> > > newRanges;

    rangeIterator begin, end;
    if (m->order() == StackMod::NEW) {
        begin = _stackRanges.begin();
        end = _stackRanges.end();
    } else if (m->order() == StackMod::CLEANUP) {
        begin = _stackRangesCleanup.begin();
        end = _stackRangesCleanup.end();
    }

    int low, high;
    if (curType == StackMod::INSERT) {
        Insert* insertMod = dynamic_cast<Insert*>(m);
        low = insertMod->low();
        high = insertMod->high();
    } else if (curType == StackMod::REMOVE) {
        Remove* removeMod = dynamic_cast<Remove*>(m);
        low = removeMod->low();
        high = removeMod->high();
    } else {
        // Shouldn't reach here
        return false;
    }

    for (auto rangeIter = begin; rangeIter != end; ++rangeIter) {
        int first = (*rangeIter).first;
        int last = (*rangeIter).second.first;
        StackMod::MType type = (*rangeIter).second.second;

        if (type == curType) {
            if ( (first <= low) && (low <= last) ) {
                found = true;
                removeRangeAt = first; // This is inefficient...
                newRanges.insert(make_pair(first, make_pair(high, type)));
                break;
            } else if ( (first <= high) && (high <= last) ) {
                found = true;
                removeRangeAt = first;
                newRanges.insert(make_pair(low, make_pair(last, type)));
                break;
            }
        } else {
            // Cases:
            // 1. m's range is contained inside existing range
            //      separate existing range into two disjoint ranges with same type,
            //      insert new range for m
            if ( (first <= low) && (high <= last)) {
                found = true;
                removeRangeAt = first;
                if (!(first == low)) {
                    newRanges.insert(make_pair(first, make_pair(low, type)));
                } 

                if (!(last == high)) {
                    newRanges.insert(make_pair(high, make_pair(last, type)));
                }

                newRanges.insert(make_pair(low, make_pair(high, curType)));
            }
            // 2. m's range encloses an existing range
            //      remove existing ranges
            //      insert a new range for m
            if ( (low < first) && (high > last) ) {
                found = true;
                removeRangeAt = first;
                newRanges.insert(make_pair(low, make_pair(high, curType)));
            }
            // 3. m's range overlaps with first
            if ( (low < first) && (high > first) && (high < last) ) {
                found = true;
                removeRangeAt = first;
                newRanges.insert(make_pair(low, make_pair(high, curType)));
                newRanges.insert(make_pair(high, make_pair(last, type)));
            }
            // 4. m's range overlaps with last
            if ( (low > first) && (low < last) && (high > last) ) {
                found = true;
                removeRangeAt = first;
                newRanges.insert(make_pair(first, make_pair(low, type)));
                newRanges.insert(make_pair(low, make_pair(high, curType)));
            }
        }
    }
    if (found) {
        if (removeRangeAt != -1) {
            if (m->order() == StackMod::NEW) {
                _stackRanges.erase(removeRangeAt);
            } else if (m->order() == StackMod::CLEANUP) {
                _stackRangesCleanup.erase(removeRangeAt);
            }
        }
        for (auto rangeIter = newRanges.begin(); rangeIter != newRanges.end(); ++rangeIter) {
            if (m->order() == StackMod::NEW) {
                _stackRanges.insert(*rangeIter);
            } else if (m->order() == StackMod::CLEANUP) {
                _stackRangesCleanup.insert(*rangeIter);
            }
        }
    } else {
        if (m->order() == StackMod::NEW) {
            _stackRanges.insert(make_pair(low, make_pair(high, curType)));
        } else if (m->order() == StackMod::CLEANUP) {
            _stackRangesCleanup.insert(make_pair(low, make_pair(high, curType)));
        }
    }

    return true;
}

/*
 * WARNING: The methods alignStackRanges and findContiguousRange function
 *          are fundamentally broken and need to be rewritten.  There is
 *          much wrong here, but it may work in limited circumstances.
 *
 */
/* Conform to alignment requirements of the current platform */
bool StackModChecker::alignStackRanges(int alignment, StackMod::MOrder order, std::vector<StackMod*>& mods)
{
    rangeIterator begin, end;
    if (order == StackMod::NEW) {
        begin = _stackRanges.begin();
        end = _stackRanges.end();
    } else if (order == StackMod::CLEANUP) {
        begin = _stackRangesCleanup.begin();
        end = _stackRangesCleanup.end();
    } else {
        // Shouldn't reach here
        return false;
    }

    for (rangeIterator rIter = begin; rIter != end; ) {
	/* endXXX renamed from end to unshadow original end, no other
	 * brokenness was fixed in this function */
        rangeIterator endXXX = rIter;
        findContiguousRange(rIter, endXXX);

        // Need to account for INSERTs vs. REMOVEs
        int size = 0;
        if (rIter == endXXX) {
            size = (*rIter).second.first - (*rIter).first;
        } else {
            for (auto iter = rIter; iter != endXXX; ++iter) {
                int curSize = (*iter).second.first - (*iter).first;
                StackMod::MType type = (*iter).second.second;
                if (type == StackMod::INSERT) {
                    size += curSize;
                } else if (type == StackMod::REMOVE) {
                    size -= curSize;
                }
            }
        }
            
        // Fix alignment
        std::div_t d = std::div(size, alignment);
        if (d.rem) {
            // Pad out to alignment requirements; for now, we'll pad above (WLOG) 
            int fixup = alignment - d.rem;
            if (order == StackMod::NEW) {
                mods.push_back(new Insert(StackMod::NEW, (*endXXX).second.first - fixup, (*endXXX).second.first));
            } else if (order == StackMod::CLEANUP) {
                mods.insert(mods.begin(), new Remove(StackMod::CLEANUP, (*endXXX).second.first - fixup, (*endXXX).second.first));
            }
        }

        rIter = ++endXXX;
    } 
    return true;
}

void StackModChecker::findContiguousRange(rangeIterator iter, rangeIterator& end)
{
    if (end == _stackRanges.end()) {
        return;
    }
    end = iter;
    rangeIterator next = iter;
    next++;
    if (next != _stackRanges.end()) {
        if ((*end).second.first == (*next).first) {
            findContiguousRange(next, end);
        }
    }
}

/* This function determines whether the stack frame grows and shrinks multiple
 * times throughout the function.  Currently, we do not allow stack
 * modifications (INSERT, REMOVE, MOVE) for functions where this occurs,
 * because the stack offsets specified by the modifications may be valid for
 * multiple address ranges in the function.  In the future, stack modifications
 * could be extended to handle these functions.
 */
bool StackModChecker::checkStackGrowth(StackAnalysis& sa)
{
    bool ret = true;
   
    ParseAPI::Function* pf = func->ifunc();
    ParseAPI::Function::blocklist blocks = pf->blocks();

    // Record stack heights in each function
    for (auto bIter = blocks.begin(); bIter != blocks.end(); ++bIter) {
        processBlock(sa, *bIter);
    }
   
    // Quick check: if the stack only changes in the entry and exit blocks(s), 
    // assume its safe to modify 

    // Accumulate exit blocks
    ParseAPI::Function::const_blocklist exitBlocklist = pf->exitBlocks();
    std::set<ParseAPI::Block*> exitBlocks;
    for (auto iter = exitBlocklist.begin(); iter != exitBlocklist.end(); ++iter) {
        exitBlocks.insert(*iter);
    }

    bool requiresFurtherCheck = false;
    for (auto bIter = blocks.begin(); bIter != blocks.end(); ++bIter) {
        ParseAPI::Block* block = *bIter;
        if (block == pf->entry()) {
            // Entry block
            continue;
        } else if (exitBlocks.find(block) != exitBlocks.end()) {
            // Exit block
            continue;
        } else {
            // Internal block
            auto found = blockHeights.find(block);

            if (found == blockHeights.end()) {
                return false;
            }
            std::vector<StackAnalysis::Height>* heights = found->second;
            if (heights->size() == 1) {
                continue;
            } else {
                requiresFurtherCheck = true;
            }
        }
    } 

    // If the stack changes in internal blocks, perform more comprehensive checks
    if (requiresFurtherCheck) {
        ret = checkAllPaths(sa);
    }

    return ret;
}

/* Cache all SP heights for each block in the function */
void StackModChecker::processBlock(StackAnalysis& sa, ParseAPI::Block* block)
{
    std::vector<StackAnalysis::Height>* heightVec = new std::vector<StackAnalysis::Height>();
    ParseAPI::Block::Insns insns;
    block->getInsns(insns);
    heightVec->push_back(sa.findSP(block, block->start()));
    for (auto iter = insns.begin(); iter != insns.end(); ++iter) {
        Offset off = (*iter).first;
        InstructionAPI::Instruction insn = (*iter).second;
        StackAnalysis::Height curHeight = sa.findSP(block, off);
        if (curHeight != heightVec->back()) {
            heightVec->push_back(curHeight);
        }
    }

    blockHeights.insert(make_pair(block, heightVec));
}

/* Check the stack growth for all paths in the function */
bool StackModChecker::checkAllPaths(StackAnalysis& sa)
{
    ParseAPI::Function::const_blocklist exitBlocklist = func->ifunc()->exitBlocks();
    std::set<ParseAPI::Block*> exitBlocks;
    for (auto iter = exitBlocklist.begin(); iter != exitBlocklist.end(); ++iter) {
        exitBlocks.insert(*iter);
    }
    std::set<ParseAPI::Block*> state;
    std::vector<ParseAPI::Block*> path;
    return checkAllPathsInternal(func->ifunc()->entry(), state, path, exitBlocks, sa);
}

/* Recursively build all control flow paths through the function;
 * when the end of path is found (at an exit block), check for safe
 * stack growth on that path. 
 */
bool StackModChecker::checkAllPathsInternal(ParseAPI::Block* block, 
                                            std::set<ParseAPI::Block*>& state,
                                            std::vector<ParseAPI::Block*>& path,
                                            std::set<ParseAPI::Block*>& exitBlocks,
                                            StackAnalysis& sa)
{
    bool ret = true;

    // Record this block on the current path; use this to track cycles
    auto stateFound = state.find(block);
    if (stateFound == state.end()) {
        state.insert(block);
    } else {
        // Found a cycle; return
        return ret;
    }

    path.push_back(block);

    if (exitBlocks.find(block) != exitBlocks.end()) {
        // Found the end of a path
        if (!checkPath(path)) {
            ret = false;
        }
    } else {
        // Recurse
        for (auto iter = block->targets().begin(); iter != block->targets().end(); ++iter) {
            ParseAPI::Edge* edge = *iter;
            // Don't follow interprocedural edges; we're creating an intraprocedural path
            if (edge->sinkEdge() || edge->interproc()) continue;
            ParseAPI::Block* target = edge->trg();

            if (!checkAllPathsInternal(target, state, path, exitBlocks, sa)) {
                ret = false;
                break;
            }
        }
    }

    // Clean up the state
    state.erase(block);
    path.pop_back();

    return ret;
}

/* Check for unsafe stack growth on the current path */
bool StackModChecker::checkPath(std::vector<ParseAPI::Block*>& path)
{
    bool foundShrink = false;

    ParseAPI::Block* entryBlock = path.front();
    auto found = blockHeights.find(entryBlock);
    if (found == blockHeights.end()) {
        return false;
    }
    std::vector<StackAnalysis::Height>* heights = found->second;

    // Height at function entry
    StackAnalysis::Height curHeight = heights->front(); 

    for (auto iter = path.begin(); iter != path.end(); ++iter) {
        ParseAPI::Block* curBlock = *iter;
        found = blockHeights.find(curBlock);

        if (found == blockHeights.end()) {
            return false;
        }
        heights = found->second;

        for (auto heightIter = heights->begin(); heightIter != heights->end(); ++heightIter) {
            StackAnalysis::Height height = *heightIter;
            if (height < curHeight) {
                // Stack grows
                if (foundShrink) {
                    // Found grow after shrink; not safe
                    return false;
                }
                curHeight = height;
            } else if (height > curHeight) {
                // Stack shrinks
                foundShrink = true;
                curHeight = height;
            }
        }
    }

    return true;
}

bool StackModChecker::findInsertOrRemovePoints(StackAnalysis& sa, StackMod* m, std::vector<BPatch_point*>*& points, long& dispFromRSP)
{
    // Insert and Remove need to be applied when the stack height is at the location of stack memory to be modified;
    // for Insert and StackMod::CLEANUP operations, this is directly above the region in qusetion; 
    // otherwise, directly below
    int loc = 0;
    if (m->type() == StackMod::INSERT || m->order() == StackMod::CLEANUP) {
        if (m->type() == StackMod::INSERT) {
            loc = dynamic_cast<Insert*>(m)->high();
        } else if (m->type() == StackMod::REMOVE) {
            loc = dynamic_cast<Remove*>(m)->high();
        } else {
            // Shouldn't reach here
            return false;
        }
    } else if (m->type() == StackMod::REMOVE) {
        loc = dynamic_cast<Remove*>(m)->low();
    } else {
        // Shouldn't reach here
        return false;
    }

    // Record some loop information for a safety check
    BPatch_flowGraph* cfg = bfunc->getCFG();
    BPatch_Vector<BPatch_basicBlockLoop*> loops;
    cfg->getLoops(loops);

    if (m->order() == StackMod::NEW) {
        // Find the first insn(s) at this height

        // Shortcut: is this height at function entry?
        ParseAPI::Block* entryBlock = func->ifunc()->entry();
        Address entryAddr = entryBlock->start();
        BPatch_point* point;
        if (checkInsn(entryBlock, entryAddr, loc, sa, point, dispFromRSP)) {
            // Yes! Return BPatch_entry
            std::vector<BPatch_point*>* entryPoints = bfunc->findPoint(BPatch_entry);
            if (entryPoints && entryPoints->size()) {
                points = entryPoints;
                return true;
            }
        }

        points = new std::vector<BPatch_point*>();

        // Normal path: work forward from entry
        std::vector<ParseAPI::Block*> worklist;
        worklist.push_back(entryBlock);

        std::set<BPatch_point*> tmpPoints;

        while (worklist.size()) {
            ParseAPI::Block* block = worklist.back();
            worklist.pop_back();

            bool found = false;
            ParseAPI::Block::Insns insns;
            block->getInsns(insns);
            BPatch_point* tmpPoint;
            for (auto iIter = insns.begin(); iIter != insns.end(); ++iIter) {
                Address addr = (*iIter).first;
                if (checkInsn(block, addr, loc, sa, tmpPoint, dispFromRSP)) {
                    // If this insn is part of a loop, not safe
                    for (auto iter = loops.begin(); iter != loops.end(); ++iter) {
                        BPatch_basicBlockLoop* bbl = *iter;
                        if (bbl->containsAddress(addr)) {
                            // Corner case: is stack height at the last insn in the loop header the same?
                            //   stack analysis calculates before height, not after.
                            //   Future: try to move to loop header
                            // Not safe to insert here
                            // But, inserting at a later instruction would not meet the expectations of the user
                            return false;
                        }
                    }

                    tmpPoints.insert(tmpPoint);
                    found = true;
                }
                if (found) break;
            }

            if (found) {
                // If this block contains an insn at the right height, add it to the set and continue through the worklist
            } else {
                // If this block does not contain an insn at the right height, push successors onto the worklist
                ParseAPI::Block::edgelist edges = block->targets();
                for (auto eIter = edges.begin(); eIter != edges.end(); ++eIter) {
                    ParseAPI::Edge* edge = *eIter;
                    if (edge->interproc()) {
                        // We shouldn't follow an interproc edge
                        // Also, this shouldn't be possible, I think
                        return false;
                    }
                    ParseAPI::Block* srcBlock = edge->src();
                    worklist.push_back(srcBlock);
                }
            }
        }

        // If we found a point for each path, return true
        // TODO: Is it possible not to have found the right point for each path?
        if (tmpPoints.size()) {
            for (auto iter = tmpPoints.begin(); iter != tmpPoints.end(); ++iter) {
                points->push_back(*iter);
            }
            return true;
        }
    } else if (m->order() == StackMod::CLEANUP) {
        // Find the last insn at this height from each function return/tailcall
        std::vector<ParseAPI::Block*> worklist;

        ParseAPI::Function::const_blocklist retBlocks = func->ifunc()->returnBlocks();
        // Accumulate returns
        for (auto iter = retBlocks.begin(); iter != retBlocks.end(); ++iter) {
            worklist.push_back(*iter);
        }
        // Accumulate tailcalls
        ParseAPI::Function::edgelist callEdges = func->ifunc()->callEdges();
        for (auto iter = callEdges.begin(); iter != callEdges.end(); ++iter) {
            ParseAPI::Edge* edge = *iter;
            if ( ( (edge->type() == ParseAPI::DIRECT) || (edge->type() == ParseAPI::INDIRECT)) &&
                    (edge->interproc())) {
                ParseAPI::Block* block = edge->src();
                worklist.push_back(block);
            }
        }

        // Use a set to combine when we find the same point twice
        std::set<BPatch_point*> tmpPoints;

        while (worklist.size()) {
            // Grab the next block from the worklist
            ParseAPI::Block* block = worklist.back();
            worklist.pop_back();

            // Look for insn at the specified height
            bool found = false;
            ParseAPI::Block::Insns insns;
            block->getInsns(insns);
            BPatch_point* point;
            for (auto iIter = insns.rbegin(); iIter != insns.rend(); ++iIter) {
                Address addr = (*iIter).first;
                if (checkInsn(block, addr, loc, sa, point, dispFromRSP)) {
                    // If this insn is part of a loop, not safe
                    for (auto iter = loops.begin(); iter != loops.end(); ++iter) {
                        BPatch_basicBlockLoop* bbl = *iter;
                        if (bbl->containsAddress(addr)) {
                            // Corner case: is stack height at the last insn in the loop header the same?
                            //   stack analysis calculates before height, not after.
                            //   Future: try to move to loop header
                            // Not safe to insert here
                            // But, inserting at a later instruction would not meet the expectations of the user
                            return false;
                        }
                    }

                    tmpPoints.insert(point);
                    found = true;
                }
                if (found) break;
            }

            if (found) {
                // If this block contains an insn at the right height, add it to the set and continue through the worklist
            } else {
                // If this block does not contain an insn at the right height, push predecessors onto the worklist
                ParseAPI::Block::edgelist edges = block->sources();
                for (auto eIter = edges.begin(); eIter != edges.end(); ++eIter) {
                    ParseAPI::Edge* edge = *eIter;
                    if (edge->interproc()) {
                        // We shouldn't follow an interproc edge
                        // Also, this shouldn't be possible, I think
                        return false;
                    }
                    ParseAPI::Block* srcBlock = edge->src();
                    worklist.push_back(srcBlock);
                }
            }
        }

        // If we found a point for each path, return true
        // TODO: Is it possible not to have found the right point for each path?
        if (tmpPoints.size()) {
            for (auto iter = tmpPoints.begin(); iter != tmpPoints.end(); ++iter) {
                points->push_back(*iter);
            }
            return true;
        }
    } else {
        // Shouldn't reach here
        return false;
    }
    
    return false;
}

bool StackModChecker::checkInsn(ParseAPI::Block* block, Offset off, int loc, StackAnalysis& sa, BPatch_point*& point, long& dispFromRSP)
{
    // Get the current stack height at loc
    StackAnalysis::Height h = sa.findSP(block, off);

    if (!h.isBottom() || h.height() == loc || h.height() < loc) {
        // Height matches

        // Find point
        std::vector<BPatch_point*> points;
        if (!bfunc->getAddSpace()->getImage()->findPoints(off, points)) return false;

        if (h.height() < loc) {
            // The difference gets stored in dispFromRSP
            // This is because we're actually inserting below our target height
            dispFromRSP = loc - h.height();
        }

        if (points.size() > 1) {
            for (auto iter = points.begin(); iter != points.end(); ++iter) {
                BPatch_point* curPoint = *iter;
                BPatch_function* curFunc = curPoint->getFunction();
                if (curFunc == bfunc) {
                    point = curPoint; 
                    return true;
                } else { 
                    continue;
                }
            }
        } else {
            point = points[0];
            return true;
        }
    }

    return false;
}

bool StackModChecker::findCanaryPoints(std::vector<BPatch_point*>* insertPoints,
        std::vector<BPatch_point*>* checkPoints)
{
    // Insert at function entry
    std::vector<BPatch_point*>* entryPoints = bfunc->findPoint(BPatch_entry);
    if (!entryPoints || !entryPoints->size()) return false;
    *insertPoints = *entryPoints;

    // Build set of check points by finding all function returns and tailcalls
    // Non-returning calls ARE NOT check points
    std::set<Address> checkAddrs;

    ParseAPI::Function::const_blocklist exitBlocks = func->ifunc()->exitBlocks();
    for (auto iter = exitBlocks.begin(); iter != exitBlocks.end(); iter++) {
        ParseAPI::Block::edgelist exitEdges = (*iter)->targets();
        for (auto eiter = exitEdges.begin(); eiter != exitEdges.end(); eiter++) {
            if ((*eiter)->interproc() && ((*eiter)->type() == ParseAPI::RET ||
                (*eiter)->type() == ParseAPI::DIRECT ||
                (*eiter)->type() == ParseAPI::INDIRECT)) {
                checkAddrs.insert((*iter)->last());
            }
        }
    }

    // Add points corresponding to this accumulated set of check addrs
    std::vector<BPatch_point*>* exitPoints = bfunc->findPoint(BPatch_exit);
    if (!exitPoints || !exitPoints->size()) return false;
    for (auto iter = exitPoints->begin(); iter != exitPoints->end(); ++iter) {
        Address ptAddr = (Address)((*iter)->getAddress());
        if (checkAddrs.find(ptAddr) != checkAddrs.end()) {
            checkPoints->push_back(*iter);
        }
    }
    return true;
}

bool StackModChecker::areModificationsSafe()
{
    ParseAPI::Function* pf = func->ifunc();
    ParseAPI::Function::blocklist blocks = pf->blocks();

    for (auto bIter = blocks.begin(); bIter != blocks.end(); ++bIter) {
        ParseAPI::Block* block = *bIter;
        ParseAPI::Block::Insns insns;
        block->getInsns(insns);
        for (auto iIter = insns.begin(); iIter != insns.end(); ++iIter) {
            Offset off = (*iIter).first;
            InstructionAPI::Instruction insn = (*iIter).second;
            Accesses* accesses = func->getAccesses(off);
            for (auto aIter = accesses->begin(); aIter != accesses->end();
                ++aIter) {
                const std::set<StackAccess *> &accessSet = aIter->second;
                for (auto setIter = accessSet.begin();
                    setIter != accessSet.end(); setIter++) {
                    if (!isAccessSafe(insn, *setIter)) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

/* Check if a particular stack access in instruction insn is safe, given the current set of
 * stack modifications. Accesses are unsafe if a stack modification perturbs the access range, 
 * which can happen by:
 *  - inserting into the accessed range, 
 *  - removing any part of the accessed range, or 
 *  - moving a portion (but not the whole) accessed range.
 */
bool StackModChecker::isAccessSafe(InstructionAPI::Instruction insn, StackAccess *access)
{
    OffsetVector* offVec = func->getOffsetVector();
    TMap* tMap = func->getTMap();

    StackAnalysis::Height accessLow = access->readHeight();
        int accessSize = -1;
        StackLocation* curLoc = NULL;
        offVec->find(accessLow, curLoc);
        if (curLoc) {
            accessLow = curLoc->off();
            accessSize = curLoc->size();
        } else {
            stackmods_printf("\t could not find entry in offVec for this access to %ld, defaulting to getAccessSize\n", accessLow.height());
            accessSize = getAccessSize(insn);
        }

        StackAnalysis::Height accessHigh = accessLow + accessSize;

        // Is this access also being moved?
        StackAnalysis::Height newAccessLow = accessLow;
        StackAnalysis::Height newAccessHigh = accessHigh;
        StackLocation* dest = NULL;
        if (curLoc) {
            dest = tMap->findInMap(curLoc);
            if (dest && curLoc != dest) {
                newAccessLow = dest->off();
                newAccessHigh = newAccessLow + dest->size();
            } 
        }
        
        // For now, we re-iterate through the modifications; in the future, it would be better to use the tMap somehow
        std::set<StackMod*>* modifications = func->getMods();

        for (auto modIter = modifications->begin(); modIter != modifications->end(); ++modIter) {
            StackMod* mod = *modIter;

            switch(mod->type()) {
                case(StackMod::INSERT): {
                                  Insert* insertMod = dynamic_cast<Insert*>(mod);
                                  StackAnalysis::Height c(insertMod->low());
                                  StackAnalysis::Height d(insertMod->high());

                                  if ((newAccessLow < c && c < newAccessHigh) || (newAccessLow < d && d < newAccessHigh)) {
                                      stackmods_printf("\t Checking modification %s\n", mod->format().c_str());
                                      stackmods_printf("\t\t c = %d\n", (int)c.height());
                                      stackmods_printf("\t\t d = %d\n", (int)d.height());
                                      stackmods_printf("\t\t Insert inside the access range. Unsafe.\n");    
                                      return false;
                                  }
                                  break;
                              }
                case(StackMod::REMOVE): {
                                  Remove* removeMod = dynamic_cast<Remove*>(mod);
                                  StackAnalysis::Height c(removeMod->low());
                                  StackAnalysis::Height d(removeMod->high());
                                  if ((newAccessLow < c && c < newAccessHigh) || (newAccessLow < d && d < newAccessHigh)) {
                                      stackmods_printf("\t Checking modification %s\n", mod->format().c_str());
                                      stackmods_printf("\t c = %d\n", (int)c.height());
                                      stackmods_printf("\t d = %d\n", (int)d.height());
                                      stackmods_printf("\t\t Remove inside the access range. Unsafe.\n");    
                                      return false;
                                  }
                                  break;
                              }
                case(StackMod::MOVE): {
                                      Move* moveMod = dynamic_cast<Move*>(mod);
                                      StackAnalysis::Height c(moveMod->srcLow());

                                      int size = moveMod->size();
                                      StackAnalysis::Height m(moveMod->destLow());

                                      // Possibilities: Both of the above sets
                                      if ((accessLow < c && c < accessHigh) || (accessLow < c+size && c+size < accessHigh)) {
                                          stackmods_printf("\t Checking modification %s\n", mod->format().c_str());
                                          stackmods_printf("\t c = %d, size = %d\n", (int)c.height(), size);
                                          stackmods_printf("\t m = %d\n", (int)m.height());
                                          stackmods_printf("\t\t Move source inside the access range. Unsafe.\n");    
                                          return false;
                                      }

                                      if ((newAccessLow < m && m < newAccessHigh) || 
                                              ( newAccessLow < m+size && 
                                                m+size < newAccessHigh)) {
                                          stackmods_printf("\t Checking modification %s\n", mod->format().c_str());
                                          stackmods_printf("\t c = %d, size = %d\n", (int)c.height(), size);
                                          stackmods_printf("\t m = %d\n", (int)m.height());
                                          stackmods_printf("\t\t Move dest inside the access range. Unsafe.\n");    
                                          return false;
                                      }
                                      break;
                                  }
                default: {
                            stackmods_printf("\t\t Unknown modification type.\n");
                            return false;
                         }
            }
        }
        
        return true;
}



