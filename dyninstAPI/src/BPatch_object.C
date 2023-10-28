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

#include <string>
#include <vector>

#if defined(cap_stack_mods)
#include "stackanalysis.h"
#endif

#include "BPatch_object.h"
#include "BPatch_image.h"
#include "mapped_object.h"
#include "BPatch_module.h"
#include "dyntypes.h"
#include "function.h"
#include "block.h"
#include "BPatch_function.h"
#include "debug.h"
#include "BPatch_point.h"
#include "registers/x86_64_regs.h"

const Dyninst::Address BPatch_object::E_OUT_OF_BOUNDS((Dyninst::Address) -1);

class BPatch_object_getMod {
  public:
   void operator()(mapped_module *m) { 
      BPatch_module *mod = img->findOrCreateModule(m);
      if (mod) mods.push_back(mod);
   }
   BPatch_object_getMod(BPatch_image *i, std::vector<BPatch_module *> &m) : img(i), mods(m) {}
   BPatch_image *img;
   std::vector<BPatch_module *> &mods;
};

BPatch_object::BPatch_object(mapped_object *o, BPatch_image *i) :
   img(i), obj(o) {
   // Fill in module list
   const std::vector<mapped_module *> &ll_mods = obj->getModules();
   BPatch_object_getMod gm(img, mods);
   std::for_each(ll_mods.begin(), ll_mods.end(), gm);
}

AddressSpace *BPatch_object::ll_as() { return obj->proc(); }

BPatch_addressSpace *BPatch_object::as() { return img->getAddressSpace(); }

std::string BPatch_object::name() {
   return obj->fileName();
}

std::string BPatch_object::pathName() {
   return obj->fullName();
}

Dyninst::Address BPatch_object::fileOffsetToAddr(const Dyninst::Offset fileOffset) {
   // File offset, so duck into SymtabAPI to turn it into a "mem offset" 
   // (aka ELF shifted) address

   Dyninst::SymtabAPI::Symtab *sym = Dyninst::SymtabAPI::convert(this);
   assert(sym);
   
   Dyninst::Offset memOffset = sym->fileToMemOffset(fileOffset);
   if (memOffset == (Dyninst::Offset) -1) return E_OUT_OF_BOUNDS;

   if (memOffset >= obj->imageOffset() &&
       memOffset < (obj->imageOffset() + obj->imageSize())) {
      return memOffset + obj->codeBase();
   }
   if (memOffset >= obj->dataOffset() &&
       memOffset < (obj->dataOffset() + obj->dataSize())) {
      return memOffset + obj->dataBase();
   }
   
   return E_OUT_OF_BOUNDS;
}

void BPatch_object::regions(std::vector<BPatch_object::Region> &regions) {
   regions.push_back(Region(obj->codeAbs(), obj->imageSize(), Region::CODE));
   regions.push_back(Region(obj->dataAbs(), obj->dataSize(), Region::DATA));
}

void BPatch_object::modules(std::vector<BPatch_module *> &modules) {
   modules.insert(modules.end(), mods.begin(), mods.end());
}

struct findFunc {
   void operator()(BPatch_module *mod) {
      mod->findFunction(name.c_str(), funcs, 
                        notify_on_failure, regex_case_sensitive,
                        incUninstrumentable, dont_use_regex);
   }

   findFunc(std::string n, 
            std::vector<BPatch_function *> &f,
            bool n_o_f, bool r, bool i, bool d) : 
      name(n), funcs(f), notify_on_failure(n_o_f),
      regex_case_sensitive(r), incUninstrumentable(i), dont_use_regex(d) {}

   std::string name;
   std::vector<BPatch_function *> &funcs;
   bool notify_on_failure;
   bool regex_case_sensitive;
   bool incUninstrumentable;
   bool dont_use_regex;
};
      
std::vector<BPatch_function *> *BPatch_object::findFunction(std::string name,
                                                               std::vector<BPatch_function *> &funcs,
                                                               bool notify_on_failure,
                                                               bool regex_case_sensitive,
                                                               bool incUninstrumentable,
                                                               bool dont_use_regex) {
   size_t num_funcs_in = funcs.size();
   findFunc f(name, funcs, false, regex_case_sensitive,
              incUninstrumentable, dont_use_regex);
   std::for_each(mods.begin(), mods.end(), f);
   if(notify_on_failure && (num_funcs_in == funcs.size()))
   {
       // notify once
      char msg[1024];
      sprintf(msg, "%s[%d]:  Object %s: unable to find function %s",
	      __FILE__, __LINE__, this->name().c_str(), name.c_str());
      BPatch_reportError(BPatchSerious, 100, msg);

   }
   return &funcs;
}


bool BPatch_object::findPoints(Dyninst::Address addr,
                                  std::vector<BPatch_point *> &points) {
   block_instance *blk = obj->findOneBlockByAddr(addr);
   if (!blk) return false;

   std::vector<func_instance *> funcs;
   blk->getFuncs(std::back_inserter(funcs));
   for (unsigned i = 0; i < funcs.size(); ++i) {
      BPatch_module *bpmod = img->findOrCreateModule(funcs[i]->mod());
      BPatch_function *bpfunc = as()->findOrCreateBPFunc(funcs[i], bpmod);
      if (!bpfunc) continue;
      instPoint *p = instPoint::preInsn(funcs[i], blk, addr);
      if (!p) continue;
      BPatch_point *pbp = as()->findOrCreateBPPoint(bpfunc, p, BPatch_locInstruction);
      if (pbp) points.push_back(pbp);
   }
   return true;
}

std::string BPatch_object::Region::format() {
   std::stringstream ret;
   
   ret << "[" << hex << base << "," << (base + size) << ","
       << ((type == CODE) ? "CODE" : "DATA") << "]";
   return ret.str();
}

Dyninst::ParseAPI::CodeObject *Dyninst::ParseAPI::convert(const BPatch_object *o) {
   if (!o->obj) return NULL;
   return o->obj->parse_img()->codeObject();
}

Dyninst::PatchAPI::PatchObject *Dyninst::PatchAPI::convert(const BPatch_object *o) {
   if (!o) return NULL;
   return o->obj;
}

Dyninst::SymtabAPI::Symtab *Dyninst::SymtabAPI::convert(const BPatch_object *o) {
   if (!o) return NULL;
   return o->obj->parse_img()->getObject();
}

BPatchSnippetHandle* BPatch_object::insertInitCallback(BPatch_snippet& callback)
{
    BPatch_Vector<BPatch_function*> init_funcs;
    findFunction("_init", init_funcs);    
    if(!init_funcs.empty())
    {
        assert(init_funcs[0]);
        BPatch_Vector<BPatch_point*>* init_entry = init_funcs[0]->findPoint(BPatch_entry);
        if(init_entry && !init_entry->empty() && (*init_entry)[0])
        {
            startup_printf("\tinserting init snippet at 0x%p\n", (*init_entry)[0]->getAddress());
            return as()->insertSnippet(callback, *((*init_entry)[0]));
        }
    }
    
    return NULL;
}

BPatchSnippetHandle* BPatch_object::insertFiniCallback(BPatch_snippet& callback)
{
    BPatch_Vector<BPatch_function*> fini_funcs;
    findFunction("_fini", fini_funcs);
    if(!fini_funcs.empty())
    {
        assert(fini_funcs[0]);
        BPatch_Vector<BPatch_point*>* fini_exit = fini_funcs[0]->findPoint(BPatch_exit);
        if(fini_exit && !fini_exit->empty() && (*fini_exit)[0])
        {
            startup_printf("\tinserting fini snippet at 0x%p\n", (*fini_exit)[0]->getAddress());
            return as()->insertSnippet(callback, *((*fini_exit)[0]));
        }
    }
    return NULL;
}


#if defined(cap_stack_mods)
namespace {
void getCalledFuncs(BPatch_function *func,
    std::set<BPatch_function *> &callSet) {
    const std::vector<BPatch_point *> *callPoints =
        func->findPoint(BPatch_subroutine);
    if (callPoints == NULL) return;
    for (auto callIter = callPoints->begin(); callIter != callPoints->end();
        callIter++) {
        BPatch_point *callPoint = *callIter;
        BPatch_function *f = callPoint->getCalledFunction();
        if (f != NULL) {
            callSet.insert(f);
        }
    }
}


void resolveCalls(BPatch_function *func,
    std::map<Address, Address> &callResolutionMap) {
    const std::vector<BPatch_point *> *callPoints =
        func->findPoint(BPatch_subroutine);
    if (callPoints == NULL) return;
    for (auto callIter = callPoints->begin(); callIter != callPoints->end();
        callIter++) {
        BPatch_point *callPoint = *callIter;
        Address callSite = (Address) callPoint->getAddress();
        BPatch_function *f = callPoint->getCalledFunction();
        if (f != NULL) {
            Address calledAddr = (Address) f->getBaseAddr();
            callResolutionMap[callSite] = calledAddr;
        }
    }
}


void DLDFS(unsigned depthLimit, std::set<BPatch_function *> allFuncsSet,
    std::set<BPatch_function *> &retSet) {
    if (depthLimit == 0) {
        retSet = allFuncsSet;
        return;
    }

    std::stack<std::pair<BPatch_function *, unsigned> > workstack;
    std::map<BPatch_function *, unsigned> donemap;
    while (!allFuncsSet.empty()) {
        BPatch_function *func = *allFuncsSet.begin();
        workstack.push(std::make_pair(func, depthLimit));
        while (!workstack.empty()) {
            BPatch_function *currFunc = workstack.top().first;
            unsigned currDepth = workstack.top().second;
            workstack.pop();

            // Add this function to the return set, and keep track of its depth
            // in case we later discover a shorter path to this function
            retSet.insert(currFunc);
            donemap[currFunc] = currDepth;
            allFuncsSet.erase(currFunc);

            // If we hit our depth limit, don't add children
            if (currDepth == 0) continue;

            std::set<BPatch_function *> calledSet;
            getCalledFuncs(currFunc, calledSet);
            for (auto iter = calledSet.begin(); iter != calledSet.end();
                iter++) {
                BPatch_function *calledFunc = *iter;
                if (allFuncsSet.find(calledFunc) != allFuncsSet.end()) {
                    // Child is not a library function, so add it with full
                    // depth limit
                    workstack.push(std::make_pair(calledFunc, depthLimit));
                } else if (donemap.find(calledFunc) != donemap.end()) {
                    // We've already visited this child.  Only add it if we now
                    // have a shorter path to it.
                    if (donemap[calledFunc] < currDepth - 1) {
                        workstack.push(std::make_pair(calledFunc,
                            currDepth - 1));
                    }
                } else {
                    // Child is a library function, so decrease its depth limit
                    workstack.push(std::make_pair(calledFunc, currDepth - 1));
                }
            }
        }
    }
}


class topoCycleVisitor {
public:
    topoCycleVisitor() : currCycle(0) {}

    void topoCycleVisit(BPatch_function *func,
        std::set<BPatch_function *> &unvisitedSet,
        std::queue<std::pair<std::set<BPatch_function *>, bool> >
        &sortedSetList) {
        // Detect cycles in call graph
        if (currCallSet.find(func) != currCallSet.end()) {
            markCycle(func);
            return;
        }

        currCallStack.push(func);
        currCallSet.insert(func);

        // Recurse until all descendants have been visited
        std::set<BPatch_function *> calledFuncs;
        getCalledFuncs(func, calledFuncs);
        for (auto iter = calledFuncs.begin(); iter != calledFuncs.end();
            iter++) {
            BPatch_function *childFunc = *iter;
            if (unvisitedSet.find(childFunc) != unvisitedSet.end()) {
                topoCycleVisit(childFunc, unvisitedSet, sortedSetList);
            } else if (cycleGroupMap.find(childFunc) != cycleGroupMap.end()) {
                // childFunc is part of a cycle.  Detect if we need to add
                // functions along our current call path to that cycle.
                BPatch_function *otherFuncInCycle = getFirstNodeByGroup(
                    currCallStack, cycleGroupMap[childFunc]);
                if (otherFuncInCycle != NULL) {
                    markCycle(otherFuncInCycle);
                }
            }
        }

        // Mark this node as visited
        unvisitedSet.erase(func);
        assert(currCallStack.top() == func);
        currCallStack.pop();
        currCallSet.erase(func);

        if (cycleGroupMap.find(func) != cycleGroupMap.end()) {
            // This node is part of a cycle
            if (!currCallStack.empty() &&
                cycleGroupMap.find(currCallStack.top()) !=
                cycleGroupMap.end() &&
                cycleGroupMap[currCallStack.top()] == cycleGroupMap[func]) {
                // The parent node is also part of this cycle.  We'll deal with
                // this cycle when we finish visiting the parent.
                return;
            } else {
                // This is the last node to be visited in this cycle.  Let's add
                // the merged cycle to the sorted list.
                const std::set<BPatch_function *> &mergedCycle =
                    cycleSetMap[cycleGroupMap[func]];
                sortedSetList.push(std::make_pair(mergedCycle, true));
            }
        } else {
            // This node is not part of a cycle.  Add the node to the sorted
            // list.
            std::set<BPatch_function *> funcSet;
            funcSet.insert(func);
            sortedSetList.push(std::make_pair(funcSet, false));
        }
    }

private:
    std::stack<BPatch_function *> currCallStack;
    std::set<BPatch_function *> currCallSet;
    std::map<BPatch_function *, unsigned> cycleGroupMap;
    std::map<unsigned, std::set<BPatch_function *> > cycleSetMap;
    unsigned currCycle;

    // Marks all nodes on the call stack up to func as part of the same cycle.
    // If any of those nodes are part of different cycles, those cycles are
    // merged into the new cycle.
    void markCycle(BPatch_function *func) {
        std::set<unsigned> mergeSet;
        std::stack<BPatch_function *> currCallStackCopy = currCallStack;

        // Update mappings for the passed-in func
        if (cycleGroupMap.find(func) != cycleGroupMap.end()) {
            mergeSet.insert(cycleGroupMap[func]);
        }
        cycleGroupMap[func] = currCycle;
        cycleSetMap[currCycle].insert(func);

        // Update our mappings with all other nodes in the cycle.  Keep track of
        // other cycles that we intersect.
        while (currCallStackCopy.top() != func) {
            BPatch_function *currFunc = currCallStackCopy.top();
            if (cycleGroupMap.find(currFunc) != cycleGroupMap.end()) {
                mergeSet.insert(cycleGroupMap[currFunc]);
            }
            cycleGroupMap[currFunc] = currCycle;
            cycleSetMap[currCycle].insert(currFunc);
            currCallStackCopy.pop();
        }

        // Update mappings for all intersecting cycles (merge those cycles into
        // this one).
        for (auto iter = mergeSet.begin(); iter != mergeSet.end(); iter++) {
            unsigned cycleNum = *iter;
            for (auto iter2 = cycleSetMap[cycleNum].begin();
                iter2 != cycleSetMap[cycleNum].end(); iter2++) {
                BPatch_function *f = *iter2;
                cycleGroupMap[f] = currCycle;
                cycleSetMap[currCycle].insert(f);
            }
            cycleSetMap.erase(cycleNum);
        }

        currCycle++;
    }

    // Returns the first node placed on callStack that is part of the cycle
    // specified by cycleNum.  Returns NULL if no matching node is found.
    BPatch_function *getFirstNodeByGroup(
        std::stack<BPatch_function *> callStack, unsigned cycleNum) {
        BPatch_function *firstNode = NULL;
        while (!callStack.empty()) {
            BPatch_function *currNode = callStack.top();
            callStack.pop();
            if (cycleGroupMap.find(currNode) != cycleGroupMap.end() &&
                cycleGroupMap[currNode] == cycleNum) {
                firstNode = currNode;
            }
        }
        return firstNode;
    }
};


void topoCycleSort(std::set<BPatch_function *> unvisitedSet,
    std::queue<std::pair<std::set<BPatch_function *>, bool> > &sortedSetList) {
    topoCycleVisitor tcv;
    while (!unvisitedSet.empty()) {
        BPatch_function *func = *unvisitedSet.begin();
        tcv.topoCycleVisit(func, unvisitedSet, sortedSetList);
    }
}


void funcSummaryFixpoint(std::set<BPatch_function *> funcSet,
    const std::map<Address, Address> &callResolutionMap,
    std::map<Address, StackAnalysis::TransferSet> &functionSummaries) {
    // Determine which functions in the set can be summarized
    std::set<Address> summarizableSet;
    for (auto iter = funcSet.begin(); iter != funcSet.end(); iter++) {
        BPatch_function *func = *iter;
        StackAnalysis sa(func->lowlevel_func()->ifunc());
        if (sa.canGetFunctionSummary()) {
            Address funcBaseAddr = (Address) func->getBaseAddr();
            summarizableSet.insert(funcBaseAddr);
        }
    }

    // Determine which functions in the set call each other
    std::map<BPatch_function *, std::set<BPatch_function *> > calleeToCallerMap;
    for (auto callerIter = funcSet.begin(); callerIter != funcSet.end();
        callerIter++) {
        BPatch_function *caller = *callerIter;
        std::set<BPatch_function *> calleeSet;
        getCalledFuncs(caller, calleeSet);
        for (auto calleeIter = calleeSet.begin(); calleeIter != calleeSet.end();
            calleeIter++) {
            BPatch_function *callee = *calleeIter;
            if (funcSet.find(callee) != funcSet.end()) {
               calleeToCallerMap[callee].insert(caller);
            }
        }
    }

    // Iteratively generate function summaries until a fixed point is reached
    std::queue<BPatch_function *> worklist;
    std::set<BPatch_function *> workset;
    for (auto iter = funcSet.begin(); iter != funcSet.end(); iter++) {
        BPatch_function *f = *iter;
        worklist.push(f);
        workset.insert(f);
    }
    while (!worklist.empty()) {
        BPatch_function *currFunc = worklist.front();
        worklist.pop();
        workset.erase(currFunc);

        // Delete any cached work and regenerate function summary
        currFunc->lowlevel_func()->freeStackMod();
        StackAnalysis sa(currFunc->lowlevel_func()->ifunc(), callResolutionMap,
            functionSummaries, summarizableSet);
        Address funcBaseAddr = (Address) currFunc->getBaseAddr();
        StackAnalysis::TransferSet summary;

        // Hard-coded function summaries for LibC
        bool summarySuccess;
        if (currFunc->getName() == "__libc_memalign") {
            stackmods_printf("Using hard-coded summary for __libc_memalign\n");
#if defined(arch_x86)
            Absloc eax(x86::eax);
            summary[eax] = StackAnalysis::TransferFunc::retopFunc(eax);
#elif defined(arch_x86_64)
            Absloc rax(x86_64::rax);
            Absloc eax(x86_64::eax);
            summary[rax] = StackAnalysis::TransferFunc::retopFunc(rax);
            summary[eax] = StackAnalysis::TransferFunc::retopFunc(eax);
#else
            assert(false);
#endif
            summarySuccess = true;
        } else {
            summarySuccess = sa.getFunctionSummary(summary);
        }

        // If summary has changed, add affected functions back to worklist
        if (summary != functionSummaries[funcBaseAddr]) {
            functionSummaries[funcBaseAddr] = summary;
            for (auto callerIter = calleeToCallerMap[currFunc].begin();
                callerIter != calleeToCallerMap[currFunc].end(); callerIter++) {
                BPatch_function *caller = *callerIter;
                // Only add to worklist if caller isn't already in the list
                if (workset.find(caller) == workset.end()) {
                    worklist.push(caller);
                    workset.insert(caller);
                }
            }
        }

        // If the summary failed, delete any default summary created
        if (!summarySuccess) {
            functionSummaries.erase(funcBaseAddr);
        }
    }
}


void addModsFuncSet(const std::set<StackMod *> &mods,
    const std::set<BPatch_function *> &addModFuncs,
    std::pair<std::set<BPatch_function *>, bool> &funcSetPair,
    std::map<Address, StackAnalysis::TransferSet> &functionSummaries,
    std::map<Address, Address> &callResolutionMap,
    std::vector<std::pair<BPatch_function *, bool> > &modResults) {
    std::set<BPatch_function *> funcSet = funcSetPair.first;
    bool isCycle = funcSetPair.second;

    if (isCycle) {
        // Iteratively generate function summaries for the cycle
        funcSummaryFixpoint(funcSet, callResolutionMap, functionSummaries);

        // Instrument the necessary functions in the cycle
        for (auto iter = funcSet.begin(); iter != funcSet.end(); iter++) {
            BPatch_function *func = *iter;
            if (addModFuncs.find(func) != addModFuncs.end()) {
               bool success = func->addMods(mods);
               modResults.push_back(std::make_pair(func, success));
            }
            func->lowlevel_func()->freeStackMod();
        }
    } else {
        // Generate a function summary and save for future analysis
        assert(funcSet.size() == 1);
        BPatch_function *func = *funcSet.begin();
        StackAnalysis sa(func->lowlevel_func()->ifunc(), callResolutionMap,
            functionSummaries);
        Address funcBaseAddr = (Address) func->getBaseAddr();
        StackAnalysis::TransferSet summary;
        if (sa.getFunctionSummary(summary)) {
            functionSummaries[funcBaseAddr] = summary;
        }

        if (addModFuncs.find(func) != addModFuncs.end()) {
            // This function also needs to be instrumented
            bool success = func->addMods(mods);
            modResults.push_back(std::make_pair(func, success));
        }
        func->lowlevel_func()->freeStackMod();
    }
}

}  // namespace
#endif

void BPatch_object::addModsAllFuncs(const std::set<StackMod *> &mods_,
    bool interproc,
    std::vector<std::pair<BPatch_function *, bool> > &modResults,
    unsigned depthLimit) {
#if defined(cap_stack_mods)
    // Get list of functions in this object
    std::set<BPatch_function *> allModFuncs;
    std::vector<BPatch_module *> objModules;
    modules(objModules);
    for (auto iter = objModules.begin(); iter != objModules.end(); iter++) {
        BPatch_module *module = *iter;
        vector<BPatch_function *> *funcs = module->getProcedures(true);
        for (auto funcIter = funcs->begin(); funcIter != funcs->end();
            funcIter++) {
            BPatch_function *func = *funcIter;
            if (!func->isInstrumentable()) {
                stackmods_printf("Function %s is uninstrumentable\n",
                    func->getName().c_str());
                continue;
            }
            allModFuncs.insert(func);
        }
    }

    if (interproc) {
        // Do a depth-limited DFS to get functions we need to consider from
        // other objects.
        std::set<BPatch_function *> allFuncs;
        DLDFS(depthLimit, allModFuncs, allFuncs);
        stackmods_printf("FUNCTIONS TO ANALYZE:\n");
        for (auto iter = allFuncs.begin(); iter != allFuncs.end(); iter++) {
            BPatch_function *f = *iter;
            stackmods_printf("%s\n", f->getName().c_str());
        }

        // Map function calls to PLT-resolved addresses (for use by
        // StackAnalysis).
        std::map<Address, Address> callResolutionMap;
        for (auto iter = allFuncs.begin(); iter != allFuncs.end(); iter++) {
            BPatch_function *f = *iter;
            resolveCalls(f, callResolutionMap);
        }

        // Determine order of function analysis via topological sort of call
        // graph.
        std::queue<std::pair<std::set<BPatch_function *>, bool> > sortedSetList;
        topoCycleSort(allFuncs, sortedSetList);

        std::queue<std::pair<std::set<BPatch_function *>, bool> > ssl2 =
            sortedSetList;
        stackmods_printf("Topological ordering:\n");
        while (!ssl2.empty()) {
            std::set<BPatch_function *> fSet = ssl2.front().first;
            bool cycle = ssl2.front().second;
            ssl2.pop();
            stackmods_printf("{");
            for (auto iter = fSet.begin(); iter != fSet.end(); iter++) {
                stackmods_printf("%s (%p), ", (*iter)->getName().c_str(),
                    (*iter)->getBaseAddr());
            }
            stackmods_printf("}, %s\n", cycle ? "TRUE" : "FALSE");
        }

        // Analyze functions in topological order
        std::map<Address, StackAnalysis::TransferSet> funcEntryToSummary;
        while (!sortedSetList.empty()) {
            addModsFuncSet(mods_, allModFuncs, sortedSetList.front(),
                funcEntryToSummary, callResolutionMap, modResults);
            sortedSetList.pop();
        }
    } else {
        for (auto iter = allModFuncs.begin(); iter != allModFuncs.end();
            iter++) {
            BPatch_function *func = *iter;
            modResults.push_back(std::make_pair(func, func->addMods(mods_)));
        }
    }
#else
    // Silence compiler warnings
    (void)mods_;
    (void)interproc;
    (void)modResults;
    (void)depthLimit;
#endif
}
