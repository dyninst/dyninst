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
#include "BPatch_module.h"
#include "function.h"
#include "instPoint.h"
#include "debug.h"
#include "process.h"
#include "MemoryEmulator/memEmulator.h"
#include "mapped_module.h"

using namespace Dyninst;

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
    BPatch_process *proc = dynamic_cast<BPatch_process*>(
       point->getFunction()->getAddSpace());
    int_function *func = point->llpoint()->func();
    proc->lowlevel_process()->getMemEm()->synchShadowOrig(func->obj(), (bool) toOrig);
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
    if (NULL == (*instrumentedFuncs)[func]) {
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

// returns false if no new instrumentation was added to the module
// Iterates through all unresolved instrumentation points in the 
// function and adds control-flow instrumentation at each type: 
// unresolved, abruptEnds, and return instructions
bool HybridAnalysis::instrumentFunction(BPatch_function *func, 
						bool useInsertionSet, 
						bool instrumentReturns) 
{
   Address funcAddr = (Address) func->getBaseAddr();
    vector<BPatch_function*>dontcare;
    mal_printf("instfunc at %lx\n", funcAddr);
    if (proc()->lowlevel_process()->isMemoryEmulated()) {
        proc()->lowlevel_process()->addModifiedFunction(func->lowlevel_func());
    }
    int pointCount = 0;

    if (!(*instrumentedFuncs)[func]) {
        (*instrumentedFuncs)[func] = new 
            std::map<BPatch_point*,BPatchSnippetHandle*>();
    }

    // grab all unresolved control transfer points in the function
    vector<BPatch_point*> points;
    func->getUnresolvedControlTransfers(points);
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
            if (curPoint->getPointType() == BPatch_locSubroutine &&
                ! proc()->findFunctionsByAddr(
                    curPoint->getCallFallThroughAddr(),dontcare)) 
            {   // If this indirect control transfer is a function call whose 
                // return status is unknown, don't allow its instrumentation to 
                // use the address cache, use the unconditional DYNINST_stopThread
                dynamicTransferSnippet = new BPatch_stopThreadExpr(
                    badTransferCB_wrapper, dynTarget, false,BPatch_interpAsTarget);
                mal_printf("hybridInstrumentation[%d] unconditional monitoring at 0x%lx:"
                            " call indirect\n", __LINE__,(long)curPoint->getAddress());
            }
            else {
                dynamicTransferSnippet = new BPatch_stopThreadExpr(
                    badTransferCB_wrapper, dynTarget, true,BPatch_interpAsTarget);
                mal_printf("hybridInstrumentation[%d] monitoring at 0x%lx: indirect\n", 
                            __LINE__,(long) curPoint->getAddress());
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
#if 0
            // if memory is emulated, and we don't know that it doesn't go to 
            // a non-instrumented library, add a callback to synchShadowOrig_wrapper
            if (proc()->lowlevel_process()->isMemoryEmulated()) {
                BPatch_module *mod = proc()->findModuleByAddr(curPoint->getSavedTarget());
                if (memHandles.end() == memHandles.find(curPoint) &&
                     ( !mod || 
                        mod->lowlevel_mod()->obj() != 
                        curPoint->llpoint()->func()->obj() ))
                {
                    std::pair<BPatchSnippetHandle*,BPatchSnippetHandle*> handles;
                    handles.first = proc()->insertSnippet
                        (BPatch_stopThreadExpr(synchShadowOrigCB_wrapper, BPatch_constExpr(1)), 
                         *curPoint, 
                         BPatch_lastSnippet);
                    handles.second = proc()->insertSnippet
                        (BPatch_stopThreadExpr(synchShadowOrigCB_wrapper, BPatch_constExpr(0)), 
                         *curPoint,
                         BPatch_callAfter, 
                         BPatch_firstSnippet);
                    memHandles[curPoint] = handles;
                    pointCount++;
                }
            }
#endif   
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
                 true,BPatch_interpAsTarget);
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
    if (retPoints && retPoints->size() && 
        (instrumentReturns || 
         ParseAPI::RETURN != func->lowlevel_func()->ifunc()->init_retstatus() || 
         ParseAPI::TAMPER_NONZERO == func->lowlevel_func()->ifunc()->tampersStack() ||
         handlerFunctions.end() != 
         handlerFunctions.find((Address)funcAddr) ))
    {
        for (unsigned int j=0; j < retPoints->size(); j++) {
			BPatch_point *curPoint = (*retPoints)[j];
            BPatchSnippetHandle *handle;

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
            bool isHandler = false;
            if (handlerFunctions.end() != 
                handlerFunctions.find((Address)func->getBaseAddr()) &&
                0 != handlerFunctions[(Address)func->getBaseAddr()])
            {
                // case 0: signal handler return address
                // for handlers, instrument their exit point with a snippet 
                // that reads the address to which the program will return 
                // from the CONTEXT of the exception, which is the 3rd argument
                // to windows structured exception handlers
                Address contextPCaddr = 
                    handlerFunctions[ (Address)func->getBaseAddr() ];
                calcSnippet = new BPatch_arithExpr
                    ( BPatch_deref, BPatch_constExpr(contextPCaddr) );
                interp = BPatch_interpAsTarget;
                isHandler = true;
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
                    ( signalHandlerExitCB_wrapper, *calcSnippet, true, interp ); 
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
        mod->protectAnalyzedCode();
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
                           (Address)((*cIter)->getAddress()), 
                           curFallThroughAddr) &&
                dupFuncCheck.find((*cIter)->getFunction()) == dupFuncCheck.end())
            {
                mal_printf("%s[%d] Function call at 0x%lx is returning, adding edge "
                          "after calls to the function at %lx\n", __FILE__,__LINE__,
                          callPoint->getAddress(), (long)(*cIter)->getAddress());
                assert(0);// KEVINTEST, this case has never executed

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
    }

    // make sure that the edge hasn't already been parsed 
    Address fallThroughAddr = callPoint->getCallFallThroughAddr();
    if (fallThroughAddr == 0x933647) {
        cerr << "DEBUG BREAKPOINT" << endl;
        }
    vector<BPatch_function *> fallThroughFuncs;
    proc()->findFunctionsByAddr(fallThroughAddr,fallThroughFuncs);

    if (! parsedAfterCallPoint && 
        ! hasEdge(callPoint->getFunction(), 
                  (Address)callPoint->getAddress(), 
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
    for (unsigned ftidx=0; 
         !parsedAfterCallPoint && ftidx < fallThroughFuncs.size(); 
         ftidx++) 
    {
        BPatch_function *fallThroughFunc = fallThroughFuncs[ftidx];
        if ( hybridOW() &&
             ! hybridOW()->hasLoopInstrumentation(true, *fallThroughFunc) )
        {
            BPatch_function *callFunc = callPoint->getFunction();
            // remove the ctrl-transfer instrumentation for this point 
            if ((*instrumentedFuncs)[callFunc] && 
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
                BPatch_stopThreadExpr newSnippet (
                        badTransferCB_wrapper, BPatch_dynamicTargetExpr(), 
                        true, BPatch_interpAsTarget);
                BPatchSnippetHandle *handle = proc()->insertSnippet
                    (newSnippet, *callPoint, BPatch_lastSnippet);
                saveInstrumentationHandle(callPoint, handle);
            }
        }
    }

    bool success = false;
    if (didSomeParsing) { // called parseNewEdge
        if (callPoint->llpoint()->block()->llb()->isShared()) {
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
                    !hasEdge(func,(Address)callPoint->getAddress(),fallThroughAddr))
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

// parse, instrument, and write-protect code found by seeding at a new target address
bool HybridAnalysis::analyzeNewFunction( Address target , 
                                         bool doInstrumentation , 
                                         bool useInsertionSet )
{
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

    // instrument all of the new modules and protect their code
    for (unsigned i=0; i < affectedMods.size(); i++) {
        if ( doInstrumentation  && affectedMods[i]->isExploratoryModeOn() ) {
            instrumentModule(affectedMods[i],useInsertionSet);//also protects the code
        }
        else if (BPatch_defensiveMode == affectedMods[i]->getHybridMode()) {
            affectedMods[i]->protectAnalyzedCode(); 
        }
    }
    return parsed;
}


bool HybridAnalysis::hasEdge(BPatch_function *func, Address source, Address target)
{
if (target == 0x933647) {
    cerr << "TEMP BREAKPOINT" << endl;
    }
// 0. first see if the edge needs to be parsed
    int_basicBlock *block = func->lowlevel_func()->findBlockByAddr(source);
    pdvector<int_basicBlock *> targBlocks; 
    block->getTargets(targBlocks);
    for (unsigned bidx=0; bidx < targBlocks.size(); bidx++) {
        if (target == targBlocks[bidx]->origInstance()->firstInsnAddr()) {
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
                (Address)sourcePoint->getAddress(),
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
    if ( ! sourceFunc->parseNewEdge( (Address)sourcePoint->getAddress() , 
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

void HybridAnalysis::deleteSynchSnippet(BPatch_point *point)
{
    assert(memHandles.end() != memHandles.find(point));
    proc()->deleteSnippet(memHandles[point].first);
    proc()->deleteSnippet(memHandles[point].second);
    memHandles.erase(point);
}

