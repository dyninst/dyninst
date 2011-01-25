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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "hybridAnalysis.h"
#include "BPatch.h"
#include "BPatch_process.h"
#include "BPatch_function.h"
#include "BPatch_flowGraph.h"
#include "BPatch_module.h"
#include "BPatch_point.h"
#include "debug.h"
#include "function.h"
#include "instPoint.h"
#include "mapped_object.h"
#include "MemoryEmulator/memEmulator.h"
#include "mapped_module.h"
#include <iostream>
#include <fstream>

using namespace Dyninst;

/* Callback wrapper funcs */
static void virtualFreeCB_wrapper(BPatch_point *point, void *calc)
{
	dynamic_cast<BPatch_process *>(point->getFunction()->getProc())->
		getHybridAnalysis()->virtualFreeCB(point, calc);
}

static void badTransferCB_wrapper(BPatch_point *point, void *calc) 
{ 
    dynamic_cast<BPatch_process*>(point->getFunction()->getProc())->
        getHybridAnalysis()->badTransferCB(point,calc); 
}
static void abruptEndCB_wrapper(BPatch_point *point, void *calc) 
{ 
    dynamic_cast<BPatch_process*>(point->getFunction()->getProc())->
        getHybridAnalysis()->abruptEndCB(point,calc); 
}
static void signalHandlerCB_wrapper
    (BPatch_point *point, long snum, std::vector<Dyninst::Address> &handlers)
{ 
    dynamic_cast<BPatch_process*>(point->getFunction()->getProc())->
        getHybridAnalysis()->signalHandlerCB(point,snum,handlers); 
}
static void signalHandlerExitCB_wrapper(BPatch_point *point, void *returnAddr) 
{ 
    dynamic_cast<BPatch_process*>(point->getFunction()->getProc())->
        getHybridAnalysis()->signalHandlerExitCB(point,returnAddr); 
}
static void synchShadowOrigCB_wrapper(BPatch_point *point, void *toOrig) 
{
    dynamic_cast<BPatch_process*>(point->getFunction()->getProc())->
        getHybridAnalysis()->synchShadowOrigCB(point, (bool) toOrig);
}

InternalSignalHandlerCallback HybridAnalysis::getSignalHandlerCB()
{ return signalHandlerCB_wrapper; }


HybridAnalysis::HybridAnalysis(BPatch_hybridMode mode, BPatch_process* proc) 
{
    mode_ = mode;
    proc_ = proc;
    hybridow_ = NULL;
    bpatchCodeDiscoveryCB = NULL;
    bpatchSignalHandlerCB = NULL;
    sharedlib_runtime = 
        proc_->getImage()->findModule("libdyninstAPI_RT", true);
    assert(sharedlib_runtime);
}

bool HybridAnalysis::init()
{
    bool ret = true;

    proc()->hideDebugger();

#if defined (os_windows)
	// Instrument VirtualFree to inform us when to unmap a code region
	BPatch_stopThreadExpr virtualFreeSnippet = BPatch_stopThreadExpr(virtualFreeCB_wrapper,
		BPatch_paramExpr(0), // Address getting unloaded
		false, // No cache!
		BPatch_interpAsTarget);
	proc()->beginInsertionSet();
	std::vector<BPatch_function *> virtualFreeFuncs;
	proc()->getImage()->findFunction("VirtualFree", virtualFreeFuncs);
	for (unsigned i = 0; i < virtualFreeFuncs.size(); ++i) {
		std::vector<BPatch_point *> *entryPoints = virtualFreeFuncs[i]->findPoint(BPatch_locEntry);
		proc()->insertSnippet(virtualFreeSnippet, *entryPoints);
	}
	proc()->finalizeInsertionSet(false);
	// Done instrumenting VirtualFree


    if (proc()->lowlevel_process()->isMemoryEmulated()) {
        // read in the list of whitelisted Windows API functions (those that 
        // don't use pointers) so we save time by not to synchronizing around
        // them
        char *dyn_root = getenv("DYNINST_ROOT");
        if (!dyn_root) {
            fprintf(stderr, "ERROR: DYNINST_ROOT environment variable was not "
                    "declared, couldn't find the list of non-pointer Windows API "
                    "functions, will synchronize memory at all inter-library calls"
                    " %s[%d]\n",FILE__,__LINE__);
        } 
        else {
            string fname = string(dyn_root) + "\\dyninst\\dyninstAPI\\nosynchfuncs.txt";
            ifstream nsf_file(fname.c_str());
            if ( ! nsf_file.is_open() ) {
                fprintf(stderr, "ERROR: failed to open file %s, which should "
                        "contain the list of non-pointer Windows API functions, "
                        "will synchronize memory at all inter-library calls "
                        "%s[%d]\n",fname.c_str(),FILE__,__LINE__);
            }
            else {
                std::string curfunc;
                while (nsf_file.good()) {
                    getline(nsf_file, curfunc);
                    nonPtrAPIs_.insert(curfunc);
                }
            }
        }
    }
#endif

    //mal_printf("   pre-inst  "); proc()->printKTimer();

    // instrument a.out module & protect analyzed code
    instrumentedFuncs = new map<BPatch_function*,map<BPatch_point*,BPatchSnippetHandle*>*>();
    vector<BPatch_module *> *allmods = proc()->getImage()->getModules();
    for (unsigned midx =0; midx < allmods->size(); midx++) {

        char namebuf[64];
        (*allmods)[midx]->getName(namebuf,64);

        // instrument the a.out
        if ( (*allmods)[midx]->isExploratoryModeOn() ) 
        {
            mal_printf("\nINSTRUMENTING MOD %s\n",namebuf);
            if (false == instrumentModule((*allmods)[midx],true)) {
                fprintf(stderr, "%s[%d] Applied no instrumentation to mod %s\n",
                        __FILE__,__LINE__,namebuf);
                ret = false;
            }
        } 
#if 0
        else if (!strncmp(namebuf,"msvcrt.dll",64)) {
            // instrument msvcrt initterm, since it calls into the application
            vector<BPatch_function*> funcs;
            (*allmods)[midx]->findFunction("initterm",funcs,false,false);
            proc()->beginInsertionSet();
            for(unsigned fidx=0; fidx < funcs.size(); fidx++) {
	            instrumentFunction(funcs[fidx],false,false);
            }
            proc()->finalizeInsertionSet(false);
        }
#endif
    }
    mal_printf("   post-inst ");
    //proc()->printKTimer();

    proc()->getImage()->clearNewCodeRegions();
    if (BPatch_defensiveMode == mode_) {
        hybridow_ = new HybridAnalysisOW(this);
    }

    if (BPatch_defensiveMode == mode_) {
        proc()->protectAnalyzedCode();
    }

    return ret;
}

HybridAnalysis::~HybridAnalysis() 
{
    if (hybridow_) {
        delete hybridow_;
    }
}

bool HybridAnalysis::setMode(BPatch_hybridMode mode)
{ 
    // can't move to (or from) defensive mode at runtime,
    // as parsing has already taken place in the wrong mode
    if (BPatch_defensiveMode == mode || 
        BPatch_defensiveMode == mode_  )
    {
        return false;
    }
    if (mode_ == mode) {
        return false;
    }
    if (BPatch_exploratoryMode == mode) {
        init();
    }
    mode_ = mode;
    return true;
}

// return number of instrumented points, 1 or 0, if the handle is NULL
int HybridAnalysis::saveInstrumentationHandle(BPatch_point *point, 
                                              BPatchSnippetHandle *handle) 
{
    BPatch_function *func = point->getFunction();
    if (instrumentedFuncs->end() == instrumentedFuncs->find(func)) {
        (*instrumentedFuncs)[func] = new map<BPatch_point*,BPatchSnippetHandle*>();
    }
    assert((*instrumentedFuncs)[func]->end() == // don't add point twice
           (*instrumentedFuncs)[func]->find(point));

    if (handle != NULL) {
        (*(*instrumentedFuncs)[func])[point] = handle;
        return 1;
    }

    mal_printf("FAILED TO INSTRUMENT at point %lx %s[%d]\n", 
               (long) point->getAddress(),FILE__,__LINE__);
    return 0;
}

// we can use the cache in memoryEmulation mode since it will
// call a modified version of stopThreadExpr that excludes 
// inter-modular calls from cache lookups
bool HybridAnalysis::canUseCache(BPatch_point *pt) 
{
    bool ret = true;

    // can't use cache for call points w/ no fallthrough addr
    if (BPatch_subroutine == pt->getPointType()) {
        vector<BPatch_function*>tmp;
        if (!proc()->findFunctionsByAddr(pt->getCallFallThroughAddr(),tmp)) {
            ret = false;
        }
    }

#if 0
    if (ret && proc()->lowlevel_process()->isMemoryEmulated()) {
        vector<Address> targs;
        pt->getSavedTargets(targs);
        if (1 == targs.size() && 
            pt->llpoint()->func()->obj() == 
            proc()->lowlevel_process()->findObject(targs[0]))
        {
            ret = false;
        }
    }
#endif
    return ret;
}

// returns false if no new instrumentation was added to the module
// Iterates through all unresolved instrumentation points in the 
// function and adds control-flow instrumentation at each type: 
// unresolved, abruptEnds, and return instructions
bool HybridAnalysis::instrumentFunction(BPatch_function *func, 
						bool useInsertionSet, 
						bool instrumentReturns) 
{
    Address funcAddr = (Address) func->getBaseAddr();
    int pointCount = 0;
    bool isHandler = false;
    if (handlerFunctions.end() != 
        handlerFunctions.find((Address)func->getBaseAddr())) 
    {
        isHandler = true;
    }

    mal_printf("instfunc at %lx\n", funcAddr);
	if (funcAddr == 0x401014) {
		DebugBreak();
	}

	assert(func);
	assert(func->lowlevel_func());
	assert(proc());
	assert(proc()->lowlevel_process());

    if (proc()->lowlevel_process()->isMemoryEmulated() && 
        BPatch_defensiveMode == func->lowlevel_func()->obj()->hybridMode()) 
    {   // we have to relocate all functions to emulate their memory accesses
        proc()->lowlevel_process()->addModifiedFunction(func->lowlevel_func());
    }

    if (instrumentedFuncs->end() == instrumentedFuncs->find(func)) {
        (*instrumentedFuncs)[func] = new 
            std::map<BPatch_point*,BPatchSnippetHandle*>();
    }

    // grab all unresolved control transfer points in the function
    vector<BPatch_point*> points;
    if (BPatch_defensiveMode == func->lowlevel_func()->obj()->hybridMode()) {
        func->getUnresolvedControlTransfers(points);
    }
    //iterate through all the points and instrument them
    BPatch_dynamicTargetExpr dynTarget;
    for (unsigned pidx=0; pidx < points.size(); pidx++) {
        BPatch_point *curPoint = points[pidx];
        BPatchSnippetHandle *handle = NULL;

        // check that we don't instrument the same point multiple times
        if ( (*instrumentedFuncs)[func]->end() != 
             (*instrumentedFuncs)[func]->find(curPoint) ) 
        {
            continue;
        }

        // if dynamic control transfer set up & install dynamicTransferSnippet 
        if(curPoint->isDynamic()) { 

            //choose the type of snippet
            BPatch_stopThreadExpr *dynamicTransferSnippet;
            bool useCache = canUseCache(curPoint);
            mal_printf("hybridInstrumentation[%d] monitoring unresolved at 0x%lx: "
                       "indirect, useCache=%d\n", __LINE__,(long)curPoint->getAddress(),(int)useCache);
            if (useCache) {
                dynamicTransferSnippet = new BPatch_stopThreadExpr(badTransferCB_wrapper, 
                    dynTarget, *curPoint->llpoint()->func()->obj(), useCache, BPatch_interpAsTarget);
            } else {
                dynamicTransferSnippet = new BPatch_stopThreadExpr(badTransferCB_wrapper, 
                    dynTarget, useCache, BPatch_interpAsTarget);
            }

            // instrument the point
            if (useInsertionSet && 0 == pointCount) {
                proc()->beginInsertionSet();
            }
            if ( curPoint->getFunction()->getModule()->isExploratoryModeOn() ) {
                handle = proc()->insertSnippet
                    (*dynamicTransferSnippet, *curPoint, BPatch_lastSnippet);
            } else { // e.g., libc's initterm function KEVINTODO: make this more robust
                BPatch_boolExpr condition(BPatch_lt, dynTarget, 
                                          BPatch_constExpr((long)(unsigned long)0x50000000));
                BPatch_ifExpr ifSmallThenStop(condition, *dynamicTransferSnippet);
                handle = proc()->insertSnippet
                    (ifSmallThenStop, *curPoint, BPatch_lastSnippet);
            }
            // if memory is emulated, and we don't know that it doesn't go to 
            // a non-instrumented library, add a callback to synchShadowOrigCB_wrapper
            if ( proc()->lowlevel_process()->isMemoryEmulated() && 
                 needsSynchronization(curPoint) )
            {
                mal_printf("Adding pre- and post- synch instrumentation to 0x%lx\n",
                           (Address)curPoint->getAddress());
                BPatchSnippetHandle *handle = proc()->insertSnippet
                    (BPatch_stopThreadExpr(synchShadowOrigCB_wrapper, BPatch_constExpr(1)), 
                     *curPoint,
                     BPatch_lastSnippet);
                synchMap_pre_[curPoint] = new SynchHandle(curPoint, handle);
            }
        } 
        else { // static ctrl flow

            // IAT entries wind up as static points but have no target, we 
            // don't need to instrument them
            vector<Address> targets;
            if (!curPoint->getCFTargets(targets)) {
                mal_printf("ERROR: Could not get target for static point[%d] "
                       "[%lx] -> [?]\n", pidx, (long)curPoint->getAddress());
                continue;
            }

            //output message
            Address target = 0;
            if (targets.size()) { 
                target = targets[0]; 
            } else {
                mal_printf("WARNING: instrumenting static transfer to "
                           "unknown target %s[%d]\n",FILE__,__LINE__);
            }
            if (curPoint->getPointType() == BPatch_locSubroutine) {
                mal_printf("hybridInstrumentation[%d] monitoring at 0x%lx: call 0x%lx\n",
                            __LINE__,(long)curPoint->getAddress(), 
                            target);
            } else {
                mal_printf("hybridInstrumentation[%d] monitoring at 0x%lx: jump 0x%lx\n",
                            __LINE__,(long)curPoint->getAddress(), 
                            target);
            }

            // instrument the point, 
            // don't need the cache because we'll remove the instrumentation 
            // once it executes, but the cache is safe to use
            if (useInsertionSet && 0 == pointCount) {
                proc()->beginInsertionSet();
            }
            BPatch_stopThreadExpr staticTransferSnippet
                (badTransferCB_wrapper,
                 BPatch_constExpr(target), 
                 false, BPatch_interpAsTarget);
            handle = proc()->insertSnippet
                (staticTransferSnippet, *curPoint, BPatch_lastSnippet);
        }
        if (handle != NULL) {
            pointCount += saveInstrumentationHandle(curPoint, handle);
        }

    } // end point loop
    points.clear();

    // get abrupt end points
    func->getAbruptEndPoints(points);

    for (unsigned pidx=0; pidx < points.size(); pidx++) {
        if ( (*instrumentedFuncs)[func]->end() != 
             (*instrumentedFuncs)[func]->find(points[pidx]) ) 
        {
            continue;
        }
        BPatch_point *curPoint = points[pidx];
        mal_printf("hybridInstrumentation[%d]monitoring at 0x%lx: abruptEnd point\n",
                    __LINE__,(long)curPoint->getAddress());

        // set up args and instrument
        if (useInsertionSet && 0 == pointCount) {
            proc()->beginInsertionSet();
        }
        BPatch_stopThreadExpr staticTransferSnippet// don't need cache here, 
            (abruptEndCB_wrapper, // will remove snippet after it executes
             BPatch_constExpr(0), 
             false,BPatch_noInterp);
        BPatchSnippetHandle *handle = proc()->insertSnippet
            (staticTransferSnippet, *curPoint, BPatch_lastSnippet);
        pointCount += saveInstrumentationHandle(curPoint,handle);
    }
    points.clear();

    // find all returns and instrument the returns of possibly non-returning 
    // functions
    vector<Address> targets;
    vector<BPatch_point *> *retPoints = func->findPoint(BPatch_exit);
    BPatch_retAddrExpr retAddrSnippet;

    if (retPoints && retPoints->size()) {
		for (unsigned int j=0; j < retPoints->size(); j++) {
			BPatch_point *curPoint = (*retPoints)[j];
			BPatchSnippetHandle *handle;

			// Workaround for a parsing inconsistency - instrument this with a snippet
			// if _any_ of the functions that contain a retblock have unknown return status...
			bool instrument = false;
			std::vector<ParseAPI::Function *> funcs;
			curPoint->llpoint()->block()->llb()->getFuncs(funcs);
			for (unsigned f_iter = 0; f_iter < funcs.size(); ++f_iter) {
				if (((image_func *)funcs[f_iter])->init_retstatus() != ParseAPI::RETURN ||
					funcs[f_iter]->tampersStack() == ParseAPI::TAMPER_NONZERO) {
						instrument = true;
				}
			}
			if (instrument || instrumentReturns || (handlerFunctions.find((Address)funcAddr) != handlerFunctions.end())) {
				// check that we don't instrument the same point multiple times
				// and that we don't instrument the return instruction if it's got 
				// a fixed, known target, e.g., it's a static target push-return 
				if ( (*instrumentedFuncs)[func]->end() != 
					(*instrumentedFuncs)[func]->find(curPoint) 
					||
					( ( curPoint->isReturnInstruction() || curPoint->isDynamic()) &&
					! curPoint->getCFTargets(targets) ) ) 
				{
					continue;
				}
			}
		// instrument the point, start insertion set, set interpretation 
		// type according to the type of instruction we're instrumenting,
		// and insert the instrumentation
		if (useInsertionSet && 0 == pointCount) {
			proc()->beginInsertionSet();
		}

		// create instrumentation snippet
            BPatch_stopThreadExpr *returnSnippet;
            BPatch_snippet * calcSnippet = NULL;
            BPatch_stInterpret interp;
            if (isHandler) {
                // case 0: signal handler return address
                // for handlers, instrument their exit point with a snippet 
                // that reads the address to which the program will return 
                // from the CONTEXT of the exception, which is the 3rd argument
                // to windows structured exception handlers
                Address contextPCaddr = 
                    handlerFunctions[ (Address)func->getBaseAddr() ];
                calcSnippet = new BPatch_constExpr(0xbaadc0de);
                interp = BPatch_noInterp;
            }
            else if (curPoint->isReturnInstruction()) {
                // case 1: the point is at a return instruction
                interp = BPatch_interpAsReturnAddr;
                calcSnippet = & retAddrSnippet;
                mal_printf("monitoring return from func[%lx %lx] at %lx\n", 
                            (long)func->getBaseAddr(), 
                            (long)func->getBaseAddr() + func->getSize(), 
                            (long)curPoint->getAddress());
            }
            else if (curPoint->isDynamic()) {
                // case 2: above check ensures that this is not a return 
                // instruction but that it is dynamic and therefore it's a jump 
                // that leaves the region housing the rest of the function
                    interp = BPatch_interpAsTarget;
                    calcSnippet = & dynTarget;
                    mal_printf("instrumenting indirect non-return exit "
                               "at 0x%lx %s[%d]\n", curPoint->getAddress(), 
                               FILE__,__LINE__);
            }
            else { // tail call, do nothing?
                calcSnippet = & dynTarget;
                fprintf(stderr,"WARNING: exit point at %lx that isn't "
                        "a return or indirect control transfer, what "
                        "kind of point is this? not instrumenting %s[%d]\n", 
                        (Address)curPoint->getAddress(), FILE__,__LINE__);
                continue;
            }

            if (!isHandler) {
                returnSnippet = new BPatch_stopThreadExpr
                    ( badTransferCB_wrapper, *calcSnippet, true, interp ); 
            } else {
                returnSnippet = new BPatch_stopThreadExpr
                    ( signalHandlerExitCB_wrapper, *calcSnippet, false, interp ); 
            }

            // insert the instrumentation
            handle = proc()->insertSnippet
                (*returnSnippet, *(curPoint), BPatch_lastSnippet);
            pointCount += saveInstrumentationHandle(curPoint,handle);

            // clean up
            delete returnSnippet;
            if (dynamic_cast<BPatch_arithExpr*>(calcSnippet)) {
                delete calcSnippet;
            }
        }
	}
    
    // close insertion set
    if (proc()->lowlevel_process()->isMemoryEmulated() || pointCount) {
        mal_printf("instrumented %d points in function at %lx\n", 
                    pointCount,func->getBaseAddr());
        if (useInsertionSet) {
            proc()->finalizeInsertionSet(false);
        }
        return true;
    }
    return false;
}// end instrumentFunction

// 1. Removes elements from instrumentedFuncs
// 2. Relegates actual work to BPatch_function::removeInstrumentation(), which:
//    saves live tramps
//    calls BPatch_point::deleteAllSnippets() for all function points, which:
//        calls BPatch_addressSpace::deleteSnippet
//    and then invalidates relocations of the function
void HybridAnalysis::removeInstrumentation(BPatch_function *func,
                                           bool useInsertionSet) 
{
    if (useInsertionSet) {
        proc()->beginInsertionSet();
    }
    // remove overwrite loops
    std::set<HybridAnalysisOW::owLoop*> loops;
    if ( hybridOW() && hybridOW()->hasLoopInstrumentation(false, *func, &loops) ) {
        mal_printf("Removing active loop instrumentation for func "
                  "%lx [%d]\n", func->getBaseAddr(), __LINE__);
        std::set<HybridAnalysisOW::owLoop*>::iterator lIter= loops.begin();
        for(; lIter != loops.end(); lIter++) {
            hybridOW()->deleteLoop(*lIter,false);
        }
    }

    // 1. Remove elements from instrumentedFuncs
    if (instrumentedFuncs->end() != instrumentedFuncs->find(func)) {
        if (proc()->lowlevel_process()->isMemoryEmulated()) {
            map<BPatch_point*,BPatchSnippetHandle*>::iterator 
                pit = (*instrumentedFuncs)[func]->begin();
            for (; pit != (*instrumentedFuncs)[func]->end(); pit++) {
                if (synchMap_pre_.end() != synchMap_pre_.find(pit->first)) 
                {
                    SynchHandle *shandle = synchMap_pre_[pit->first];
                    synchMap_pre_.erase(shandle->prePt_);
                    synchMap_post_.erase(shandle->postPt_);
                    delete shandle;
                }
                else if (synchMap_post_.end() != synchMap_post_.find(pit->first))
                {
                    SynchHandle *shandle = synchMap_post_[pit->first];
                    synchMap_pre_.erase(shandle->prePt_);
                    synchMap_post_.erase(shandle->postPt_);
                    delete shandle;
                }
            }
        }
        (*instrumentedFuncs)[func]->clear();
        delete (*instrumentedFuncs)[func];
        instrumentedFuncs->erase(func);
    }

// 2. Relegate actual work to BPatch_function::removeInstrumentation()
    func->removeInstrumentation(false);
    if (useInsertionSet) {
        proc()->finalizeInsertionSet(false);
    }
}

// Returns false if no new instrumentation was added to the module.  
// Relegates all instrumentation work to instrumentFunction
// Protects the code in the module
bool HybridAnalysis::instrumentModule(BPatch_module *mod, bool useInsertionSet) 
{
    assert(proc() && mod);
    if (false == mod->isExploratoryModeOn()) {
        return true;
    }

    if (useInsertionSet) {
        proc()->beginInsertionSet();
    }

    bool didInstrument = false;
    // if this not the default module, instrument it as well
    //if ( false == mod->isSharedLib() ) {
    //    BPatch_module* defaultModule = mod->getObjectDefaultModule();
    //    if (defaultModule != mod) {
    //        didInstrument = instrumentModule(defaultModule,false) || didInstrument;
    //    }
    //}

    // instrument functions
    vector<BPatch_function*> *modFuncs = mod->getProcedures(false);
    vector<BPatch_function*>::iterator fIter = modFuncs->begin();
    for (; fIter != modFuncs->end(); fIter++) 
    {
        if ( instrumentedFuncs->find(*fIter) == instrumentedFuncs->end() ) {
            didInstrument = instrumentFunction(*fIter,false) || didInstrument;
        }
    }
    
    if (useInsertionSet) {
        proc()->finalizeInsertionSet(false);
    }

    // protect the code in the module
    if (BPatch_defensiveMode == mod->getHybridMode()) {
        mod->setAnalyzedCodeWriteable(false);
    }

    return didInstrument;
}

// Returns false if no new instrumentation was added to the module.  
// Relegates all instrumentation work to instrumentFunction
// Protects the code in the module
bool HybridAnalysis::instrumentModules(bool useInsertionSet) 
{
    bool didInstrument = false;
    if (useInsertionSet) {
        proc()->beginInsertionSet();
    }
    std::vector<BPatch_module*> *mods = proc()->getImage()->getModules();
    for (unsigned midx=0; midx < mods->size(); midx++) {
        if ((*mods)[midx]->isExploratoryModeOn()) {
            didInstrument = instrumentModule((*mods)[midx], false) || didInstrument;
        }
    }
    if (useInsertionSet) {
        proc()->finalizeInsertionSet(false);
    }
    return didInstrument;
}

// add instrumentation to trigger an original memory to shadow memory copy 
// at blks, associated with an intermodule call at callPt
void HybridAnalysis::origToShadowInstrumentation(BPatch_point *callPt, 
                                                 const vector<int_block*> &blks)
{
    for (vector<int_block*>::const_iterator bit = blks.begin();
         bit != blks.end();
         bit++) 
    {
        BPatch_function *bpFunc = proc()->findOrCreateBPFunc((*bit)->func(),NULL);
        BPatch_point *ftPt = bpFunc->getPoint((*bit)->start());
        assert(ftPt);
        if (synchMap_post().end() == synchMap_post().find(ftPt)) {
            BPatchSnippetHandle *handle = proc()->insertSnippet
                (BPatch_stopThreadExpr(synchShadowOrigCB_wrapper, BPatch_constExpr(0)), 
                 *ftPt,
                 BPatch_callBefore,
                 BPatch_firstSnippet);
            synchMap_pre()[callPt]->setPostHandle(ftPt, handle);
            synchMap_post()[ftPt] = synchMap_pre()[callPt];
        }
    }
}

/* Takes a point corresponding to a function call and continues the parse in 
 * the calling function after the call.  If there are other points that call
 * into this function resume parsing after those call functions as well. 
 * Also need to parse any functions that are discovered thanks to the better parsing
 * 
 * Return true if instrumentation of new or modified functions occurs
 */ 
bool HybridAnalysis::parseAfterCallAndInstrument(BPatch_point *callPoint, 
                                                 BPatch_function *calledFunc)
{
    assert(callPoint);
    std::set<BPatch_module*> callerMods; 
    bool parsedAfterCallPoint = false;//ensures we parse after the call point only once
    bool didSomeParsing = false; // if we parsed after another call point
    proc()->beginInsertionSet();

    if (calledFunc) {
        // add fallthrough for any other calls to this function, iterate 
        // through call points to the called function
        std::vector<BPatch_point*> callerPoints;
        calledFunc->getCallerPoints(callerPoints);
        vector<BPatch_point*>::iterator cIter = callerPoints.begin();
        std::set<BPatch_function *> dupFuncCheck; //add one fallthrough edge per func
        while (cIter != callerPoints.end()) 
        {
            Address curFallThroughAddr = (*cIter)->getCallFallThroughAddr();
            if ( ! hasEdge((*cIter)->getFunction(), 
                           (Address)((*cIter)->llpoint()->block()->start()), 
                           curFallThroughAddr) &&
                dupFuncCheck.find((*cIter)->getFunction()) == dupFuncCheck.end())
            {
                mal_printf("%s[%d] Function call at 0x%lx is returning, adding edge "
                          "after calls to the function at %lx\n", __FILE__,__LINE__,
                          callPoint->getAddress(), (long)(*cIter)->getAddress());
                //assert(0);// KEVINTEST, this case has never executed

                parseNewEdgeInFunction( *cIter , curFallThroughAddr , false );

                callerMods.insert((*cIter)->getFunction()->getModule());
                dupFuncCheck.insert((*cIter)->getFunction());
                
                // We have to make sure we parse after the original call, it 
                // won't be listed here if it's an indirect call
                if ( ! parsedAfterCallPoint && (*cIter) == callPoint ) {
                    parsedAfterCallPoint = true;
                }
                didSomeParsing = true;
            }
            cIter++;
        }
        // parse the call target edge if needed
        if (callPoint->isDynamic()) {
            addIndirectEdgeIfNeeded(callPoint,
                                    (Address)calledFunc->getBaseAddr());
        }
    }

    // make sure that the edge hasn't already been parsed 
    Address fallThroughAddr = callPoint->getCallFallThroughAddr();
    vector<BPatch_function *> fallThroughFuncs;
    proc()->findFunctionsByAddr(fallThroughAddr,fallThroughFuncs);

    if (! parsedAfterCallPoint && 
        ! hasEdge(callPoint->getFunction(), 
                  (Address)callPoint->llpoint()->block()->start(), 
                  fallThroughAddr)) 
    {
        mal_printf("Function call at 0x%lx is returning, "
                    "adding edge to fallthrough at %lx %s[%d]\n", 
                    callPoint->getAddress(), fallThroughAddr, 
                    FILE__, __LINE__);

        parseNewEdgeInFunction( callPoint , fallThroughAddr , false );
        parsedAfterCallPoint = true;
        didSomeParsing = true;

        callerMods.insert(callPoint->getFunction()->getModule());
    }

    // if we're not re-instrumenting the function because we'd already
    // parsed the fallthrough addr and there is no active loop
    // instrumentation in this function (in which case changing the 
    // instrumentation would cause changes to the code, resulting in 
    // a false positive code overwrite) re-instrument the point to use
    // the cache, if it's an indirect transfer, or remove it altogether
    // if it's a static transfer. 
    bool reInstrument = false;
    if (!parsedAfterCallPoint) {
        vector<Address> targs;
        if (!proc()->lowlevel_process()->isMemoryEmulated()) {
            reInstrument = true;
        } else if (callPoint->getSavedTargets(targs)) {
            reInstrument = true;
            Address objStart = callPoint->llpoint()->func()->obj()->codeBase();
            Address objEnd = objStart 
                + callPoint->llpoint()->func()->obj()->imageSize();
            for (vector<Address>::iterator iter=targs.begin(); 
                 iter != targs.end(); 
                 iter++) 
            {
                if ((*iter) < objStart || (*iter) >= objEnd) {
                    reInstrument = false;
                    break;
                }
            }
        }
    }
    if (reInstrument) {
      for (unsigned ftidx=0; ftidx < fallThroughFuncs.size(); ftidx++) 
      {
        BPatch_function *fallThroughFunc = fallThroughFuncs[ftidx];
        if ( hybridOW() &&
             ! hybridOW()->hasLoopInstrumentation(true, *fallThroughFunc) )
        {
            BPatch_function *callFunc = callPoint->getFunction();
            // remove the ctrl-transfer instrumentation for this point 
            if ((*instrumentedFuncs).end() != (*instrumentedFuncs).find(callFunc) && 
                (*instrumentedFuncs)[callFunc]->end() !=
                (*instrumentedFuncs)[callFunc]->find(callPoint))
            {
                proc()->deleteSnippet( (*(*instrumentedFuncs)[callFunc])[callPoint] );
                (*instrumentedFuncs)[callFunc]->erase(callPoint);
            }
            // if the point is dynamic, re-instrument it to use the cache
            if (callPoint->isDynamic()) {
                mal_printf("replacing instrumentation at indirect call point "
                            "%lx with instrumentation that uses the cache "
                            "%s[%d]\n", callPoint->getAddress(),FILE__,__LINE__);
                BPatch_stopThreadExpr newSnippet (badTransferCB_wrapper, 
                        BPatch_dynamicTargetExpr(), 
                        *callPoint->llpoint()->func()->obj(), 
                        true, BPatch_interpAsTarget);
                BPatchSnippetHandle *handle = proc()->insertSnippet
                    (newSnippet, *callPoint, BPatch_lastSnippet);
                saveInstrumentationHandle(callPoint, handle);
            }
        }
      }
    }

    bool success = false;
    if (didSomeParsing) { // called parseNewEdge
        vector<ParseAPI::Function*> imgFuncs;
        callPoint->llpoint()->block()->llb()->getFuncs(imgFuncs);
        for (unsigned fidx=0; fidx < imgFuncs.size(); fidx++) 
        {
            BPatch_function *func = proc()->findOrCreateBPFunc(
                proc()->lowlevel_process()->findFuncByInternalFunc((image_func*)imgFuncs[fidx]),
                NULL);
            removeInstrumentation(func, false);
            func->getCFG()->invalidate();

            if (func != callPoint->getFunction() &&
                !hasEdge(func,(Address)callPoint->llpoint()->block()->start(),fallThroughAddr))
            {
                BPatch_point *sharedCallPoint = NULL;
                std::vector<BPatch_point *> *points = func->findPoint(BPatch_subroutine);
                for (unsigned i = 0; i < points->size(); ++i) {
                    if ((*points)[i]->getAddress() == callPoint->getAddress()) {
                        sharedCallPoint = (*points)[i];
                        break;
                    }
                }
                assert(sharedCallPoint);
                parseNewEdgeInFunction(sharedCallPoint, fallThroughAddr, false);
            }
        }
        // Ensure we relocate the re-parsed function
        proc()->beginInsertionSet();
        callPoint->getFunction()->relocateFunction();

        // instrument all modules that have modified functions
        success = instrumentModules(false);
    }

    // fill in the post-call area with a patch 
    // (even if we didn't parse, we have to do this to get rid of the illegal instructions in the pad)
    proc()->finalizeInsertionSet(false);
    success = callPoint->patchPostCallArea() && success;

    return success;
}

bool HybridAnalysis::addIndirectEdgeIfNeeded(BPatch_point *sourcePt, 
                                             Address target)
{
    // see if the edge already exists
    using namespace ParseAPI;
    Block::edgelist &edges = sourcePt->llpoint()->block()->llb()->targets();
    Block::edgelist::iterator eit = edges.begin();
    mapped_object *targObj = proc()->lowlevel_process()->findObject(target);
    if (targObj) {
        for (; eit != edges.end(); eit++)
            if ( ! (*eit)->sinkEdge() && 
                 (*eit)->trg()->start() == (target - targObj->codeBase()) )
                break;
    }
    if (targObj && eit == edges.end()) {

        mal_printf("Adding indirect edge %lx->%lx", 
                   (Address)sourcePt->getAddress(), target);

        // edge does not exist, determine desired edge type
		EdgeTypeEnum etype;
        if (BPatch_subroutine == sourcePt->getPointType()) {
            etype = CALL;
            mal_printf(" of type CALL\n");
        } else if (BPatch_exit == sourcePt->getPointType()) {
            etype = RET;
            mal_printf(" of type RET\n");
        } else {
            etype = INDIRECT;
            mal_printf(" of type INDIRECT\n");
        }

        // make sure we're not adding a call to a function 
        // that looks like garbage, since the edge would make us 
        // update our analysis of the function over and over 
        // rather than just blowing the function away of it
        // gets stomped in its entry block
        if (CALL == etype) {
            int_function *tFunc = targObj->findFuncByEntry(target);
            assert(tFunc);
            if ( ! tFunc->funcAbruptEnds().empty() ) {
                malware_cerr << "Ignoring request as target function "
                    << "has abrupt end points and will probably get "
                    << "overwritten before it executes, if ever" << endl;
                // clear cache of the target function address
                //proc()->lowlevel_process()->flushAddressCache_RT(tFunc);
                return false;
            }
        }

        // the function looks good, add the edge
		vector<CodeObject::NewEdgeToParse> worklist;
		worklist.push_back(CodeObject::NewEdgeToParse(sourcePt->llpoint()->block()->llb(), target - targObj->codeBase(), etype));
        targObj->parse_img()->codeObject()->parseNewEdges(worklist);
        return true;
    }
    return false;
}


// parse, instrument, and write-protect code found by seeding at a new target address
bool HybridAnalysis::analyzeNewFunction( BPatch_point *source, 
                                         Address target, 
                                         bool doInstrumentation, 
                                         bool useInsertionSet )
{
    // parse at the target
    bool parsed = true;
    vector<BPatch_module *> affectedMods;
    vector<Address> targVec; // has only one element, used to conform to interface
    targVec.push_back(target);
    if (!proc()->getImage()->parseNewFunctions(affectedMods, targVec)) {
        fprintf(stderr, "WARNING: call to parseNewFunctions failed to parse region "
                "containing target addr %lx  %s[%d]\n", (long)target, FILE__, __LINE__);
        parsed = false;
    }
    
    // inform the mutator of the new code
    if (bpatchCodeDiscoveryCB) {
        std::vector<BPatch_function *> newfuncs;
        std::vector<BPatch_function *> modfuncs;
        proc()->getImage()->getNewCodeRegions(newfuncs,modfuncs);
        if (newfuncs.size() || modfuncs.size()) {
            if (hybridow_ && modfuncs.size()) {
                hybridow_->codeChangeCB(modfuncs);
            }
            if (BPatch_defensiveMode == mode_) {
                proc()->protectAnalyzedCode();
            }
            bpatchCodeDiscoveryCB(newfuncs,modfuncs);
        }
    }
    proc()->getImage()->clearNewCodeRegions();

    if (source->isDynamic() || BPatch_exit == source->getPointType()) {
        addIndirectEdgeIfNeeded(source,target);
    }

    // instrument all of the new modules and protect their code
    for (unsigned i=0; i < affectedMods.size(); i++) {
        if ( doInstrumentation  && affectedMods[i]->isExploratoryModeOn() ) {
            instrumentModule(affectedMods[i],useInsertionSet);//also protects the code
        }
        else if (BPatch_defensiveMode == affectedMods[i]->getHybridMode()) {
            affectedMods[i]->setAnalyzedCodeWriteable(false); 
        }
    }
    return parsed;
}


bool HybridAnalysis::hasEdge(BPatch_function *func, Address source, Address target)
{
// 0. first see if the edge needs to be parsed
    int_block *block = func->lowlevel_func()->findBlockByEntry(source);
    pdvector<int_block *> targBlocks; 
    block->getTargets(targBlocks);
    for (unsigned bidx=0; bidx < targBlocks.size(); bidx++) {
        if (target == targBlocks[bidx]->start()) {
            return true; // already parsed this edge, we're done!
        }
    }
    return false;
}

// Does not reinsert instrumentation.  
//
// Adds new edge to the parse of the function, removes existing instrumentation
// from the function if it is relocated, removes func from instrumentedFuncs
//
// 1. if the target is in the same section as the source func, 
//    remove instrumentation from the source function 
// 2. parse the new edge
void HybridAnalysis::parseNewEdgeInFunction(BPatch_point *sourcePoint, 
                                            Address target,
                                            bool useInsertionSet)
{
    // 0. first see if the edge needs to be parsed
    if (hasEdge(sourcePoint->getFunction(),
                (Address)sourcePoint->llpoint()->block()->start(),
                target)) 
    {
        return;
    }

    BPatch_function *sourceFunc = sourcePoint->getFunction();

    // 1. if the target is in the same section as the source func, 
    //    remove instrumentation from the source function 

    if (useInsertionSet) {
        proc()->beginInsertionSet();
    }

    // remove loop instrumentation, if any
    std::set<HybridAnalysisOW::owLoop*> loops;
    if (hybridOW() && 
        hybridOW()->hasLoopInstrumentation(false, *sourceFunc, &loops)) 
    {
        std::set<HybridAnalysisOW::owLoop*>::iterator lIter= loops.begin();
        while (lIter != loops.end())
        {
            hybridOW()->deleteLoop(*lIter,false);
            lIter++;
        }
    } 

    // remove the function's instrumentation (and from shared funcs)
    removeInstrumentation(sourceFunc,false);
    set<BPatch_function*> sharedFuncs;
    if (sourceFunc->getSharedFuncs(sharedFuncs)) {
        set<BPatch_function*>::iterator fit;
        for (fit = sharedFuncs.begin(); fit != sharedFuncs.end(); fit++) {
            if ( *fit != sourceFunc) {
                removeInstrumentation(*fit,false);
            }
        }
    }
    if (useInsertionSet) {
        proc()->finalizeInsertionSet(false);
    }

    // 2. parse the new edge
    if ( ! sourceFunc->parseNewEdge( (Address)sourcePoint->llpoint()->block()->start() , 
                                     target ) ) 
    {
        assert(0);//this case should be ruled out by the call to sameRegion
    }

    // inform the mutator of the new code
    if (bpatchCodeDiscoveryCB) {
        std::vector<BPatch_function *> newfuncs;
        std::vector<BPatch_function *> modfuncs;
        proc()->getImage()->getNewCodeRegions(newfuncs,modfuncs);
        // add sourceFunc to modfuncs, since we removed its instrumentation
        bool foundSrcFunc = false;
        for(unsigned midx=0; midx < modfuncs.size(); midx++) {
            if (sourceFunc == modfuncs[midx]) {
                foundSrcFunc =true;
                break;
            }
        }
        if (newfuncs.size() || modfuncs.size()) {
            if (hybridow_ && modfuncs.size()) {
                hybridow_->codeChangeCB(modfuncs);
            }
            if (BPatch_defensiveMode == mode_) {
                proc()->protectAnalyzedCode();
            }
        }
        // if the code didn't change, add sourceFunc to modfuncs, since
        // we removed its instrumentation and the user has to re-instrument
        if (!foundSrcFunc) {
            modfuncs.push_back(sourceFunc);
        }
        // invoke callback
        bpatchCodeDiscoveryCB(newfuncs,modfuncs);
    }

    proc()->getImage()->clearNewCodeRegions();
}

bool HybridAnalysis::processInterModuleEdge(BPatch_point *point, 
                                            Address target, 
                                            BPatch_module *targMod)
{
    bool processTargMod = true;
    BPatch_function* targFunc = targMod->findFunctionByEntry(target);
    char modName[16]; 
    char funcName[32];
    targMod->getName(modName,16);
    if (targFunc) {
        targFunc->getName(funcName,32);
        mal_printf("%lx => %lx, in module %s to known func %s\n",
                    point->getAddress(),target,modName,funcName);
    } else {
        funcName[0]= '\0';
        mal_printf("%lx => %lx, in module %s \n",
                    point->getAddress(),target,modName,funcName);
    }
	if (target == 0x401014) {
		int i = 3;
	}
    // 1.1 if targMod is a system library don't parse at target.  However, if the 
    //     transfer into the targMod is an unresolved indirect call, parse at the 
    //     call's fallthrough addr and return.
    if (targMod->isSystemLib() && BPatch_defensiveMode != targMod->getHybridMode()) 
    {
        if (point->getPointType() == BPatch_subroutine) {
            if (0 == strncmp(funcName,"ExitProcess",32) && 
                NULL == strstr(modName,"kernel32.dll")) 
            {
                fprintf(stderr,"Caught call to %s, should exit soon %s[%d]\n", 
                        funcName,FILE__,__LINE__);
            }
            else if (0 == strncmp(funcName,"ExitProcess",32) && 
                     NULL == strstr(modName,"kernel32.dll")) 
            {

            }
            else {
                mal_printf("stopThread instrumentation found call %lx=>%lx, "
                          "target is in module %s, parsing at fallthrough %s[%d]\n",
                          (long)point->getAddress(), target, modName,FILE__,__LINE__);
                parseAfterCallAndInstrument(point, targFunc);
            }
            processTargMod = false;
		} else if (point->getPointType() == BPatch_exit) {
			mal_printf("WARNING: stopThread instrumentation found return %lx=>%lx, "
                      "into module %s, this indicates obfuscation or that there was a "
					  "call from that module into our code %s[%d]\n",
                      (long)point->getAddress(), target, modName,FILE__,__LINE__);
		}
		else { // jump into system library
            // this is usually symptomatic of the following:
            // call tail1
            //    ...
            // .tail1
            // jump ptr
            mal_printf("WARNING: transfer into non-instrumented system module "
                        "%s at: %lx=>%lx %s[%d]\n", modName, 
                        (long)point->getAddress(), target,FILE__,__LINE__);

            // Instrument the return instructions of the system library 
            // function so we can find the code at the call instruction's
            // fallthrough address
            proc()->beginInsertionSet();
			BPatch_function *targFunc = proc()->findFunctionByEntry(target);
            if (!targFunc) {
                analyzeNewFunction(point,target,false,false);
                targFunc = proc()->findFunctionByEntry(target);
            }
			instrumentFunction(targFunc, false, true);
            proc()->finalizeInsertionSet(false);
            processTargMod = false;
        }
    }
    // 1.2 if targMod is a non-system library, then warn, and fall through into
    // handling the transfer as we would any other transfer
    else if ( targMod->isExploratoryModeOn() ) { 
        mal_printf("WARNING: Transfer into instrumented module %s "
                "func %s at: %lx=>%lx %s[%d]\n", modName, funcName, 
                (long)point->getAddress(), target, FILE__,__LINE__);
    } else { // jumped or called into module that's not recognized as a 
             // system library and is not instrumented
        if ((Address)point->getAddress() != 0x77c39d78) {
            mal_printf("WARNING: Transfer into non-instrumented module "
                    "%s func %s that is not recognized as a system lib: "
                    "%lx=>%lx [%d]\n", modName, funcName, 
                    (long)point->getAddress(), target, FILE__,__LINE__);
        } else {
            processTargMod = false;
                    // triggers for nspack's transfer into space that's 
                    // allocated at runtime
        }
    }
    return processTargMod;
}

// returns false if current evidence suggests that an indirect control 
// transfer is always intramodular or if it is to a function that does
// take pointers as parameters or produce them as a return value
bool HybridAnalysis::needsSynchronization(BPatch_point *point)
{
    vector<Address> targs;
    point->getSavedTargets(targs);

    if (targs.empty()) { 
        ParseAPI::Block::edgelist & outEdges = point->llpoint()->block()->llb()->targets();
        if (outEdges.size() <= 1) {
            return true; // the point has not been resolved yet
        } else {
            return false; // the point has been resolved
        }
    }

    for (vector<Address>::iterator tit= targs.begin();
         tit != targs.end(); 
         tit++) 
    {
        BPatch_module *targMod = proc()->findModuleByAddr(*tit);
        if (targMod != point->getFunction()->getModule()) {
            BPatch_function *tfunc = targMod->findFunctionByEntry(*tit);
            std::set<string>::iterator fit = nonPtrAPIs_.find(tfunc->getName());
            if (nonPtrAPIs_.end() == fit) {
                return true;
            } else {
                mal_printf("Not synchronizing around call to non-ptr func %s\n", 
                           tfunc->getName());
            }
        }
    }

    return false;
}

bool HybridAnalysis::blockcmp::operator () (const BPatch_basicBlock *b1, 
                    const BPatch_basicBlock *b2) const
{
    if (const_cast<BPatch_basicBlock*>(b1)->getStartAddress() 
        < 
        const_cast<BPatch_basicBlock*>(b2)->getStartAddress()) 
    {
        return true;
    } 
    return false; 
}

HybridAnalysis::SynchHandle::SynchHandle(BPatch_point *prePt, 
                                         BPatchSnippetHandle *preHandle)
{
    prePt_ = prePt;
    preHandle_ = preHandle;
}

void 
HybridAnalysis::SynchHandle::setPostHandle(BPatch_point *postPt, 
                                           BPatchSnippetHandle *postHandle)
{
    postPt_ = postPt;
    postHandle_ = postHandle;
}

void HybridAnalysis::deleteSynchSnippet(SynchHandle *handle)
{
    proc()->deleteSnippet(handle->preHandle_);
    if (handle->postHandle_) {
        proc()->deleteSnippet(handle->postHandle_);
    }
}

std::map< BPatch_point* , HybridAnalysis::SynchHandle* > & 
HybridAnalysis::synchMap_pre()
{ 
    return synchMap_pre_;
}

std::map< BPatch_point* , HybridAnalysis::SynchHandle* > & 
HybridAnalysis::synchMap_post()
{ 
    return synchMap_post_;
}

int
HybridAnalysis::getOrigPageRights(Address addr)
{
    int origPerms;
    using namespace SymtabAPI;
    mapped_object *obj = proc()->lowlevel_process()->findObject(addr);
    Region * reg = obj->parse_img()->getObject()->
        findEnclosingRegion(addr - obj->codeBase());
    Region::perm_t perms = reg->getRegionPermissions();
    switch (perms) {
    case (Region::RP_R):
        origPerms = PAGE_READONLY;
        break;
    case (Region::RP_RW):
        origPerms = PAGE_READWRITE;
        break;
    case (Region::RP_RX):
        origPerms = PAGE_EXECUTE_READ;
        break;
    case (Region::RP_RWX):
        origPerms = PAGE_EXECUTE_READWRITE;
        break;
    default:
        assert(0);
    }
    return origPerms;
}
