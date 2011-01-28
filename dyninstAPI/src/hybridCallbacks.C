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
#include "debug.h"
#include "BPatch_point.h"
#include "BPatch_process.h"
#include "BPatch_function.h"
#include "BPatch_module.h"
#include "instPoint.h"
#include "function.h"
#include "MemoryEmulator/memEmulator.h"
#include "mapped_object.h"

void newCodeCB(std::vector<BPatch_function*> &newFuncs, 
               std::vector<BPatch_function*> &modFuncs)
{
    printf("new functions %d mod functions %d %s[%d]\n", 
           (int)newFuncs.size(), (int)modFuncs.size(), FILE__, __LINE__);
}

void HybridAnalysis::synchShadowOrigCB(BPatch_point *point, bool toOrig)
{
    mal_printf("in synch callback for point 0x%lx toOrig=%d\n",
               (Address)point->getAddress(), (int) (long) toOrig);

    if ( toOrig && !proc()->getHybridAnalysis()->needsSynchronization(point) ) {
        return;
    }

    // synch the shadow pages
    BPatch_function *pfunc = point->getFunction();
    proc()->lowlevel_process()->getMemEm()->synchShadowOrig
        ( pfunc->lowlevel_func()->obj(), (bool) toOrig);

    // fix up page rights so that the program can proceed
    if (toOrig) {
        // instrument at fallthrough
        std::vector<int_block*> ftBlks;
        int_block *ftBlk = point->llpoint()->block()->getFallthrough();
        if (ftBlk) {
            ftBlks.push_back(ftBlk);
        }
        else {
            // the transfer is an inter-module jump, instrument at 
            // fallthroughs of callers to the function containing the jump
            std::vector<BPatch_point*> callerPoints;
            point->getFunction()->getCallerPoints(callerPoints);
            mal_printf("Caught inter-module jump at %lx, parsing at "
                       "fallthroughs of %d callers to the jump func\n",
                       point->getAddress(), callerPoints.size());
            assert(callerPoints.size()); // there has to be at least one
            for (vector<BPatch_point*>::iterator pit = callerPoints.begin();
                 pit != callerPoints.end(); 
                 pit++)
            {
                ftBlk = (*pit)->llpoint()->block()->getFallthrough();
                if (!ftBlk) {
                    parseAfterCallAndInstrument(*pit, pfunc);
                    ftBlk = (*pit)->llpoint()->block()->getFallthrough();
                }
                assert(ftBlk);
                ftBlks.push_back(ftBlk);
            }
        }
        // drop the instrumentation in 
        origToShadowInstrumentation(point, ftBlks);

        // remove write-protections from code pages
        pfunc->getModule()->setAnalyzedCodeWriteable(true);
    } 
    else {
        // restore write-protections to code pages
        pfunc->getModule()->setAnalyzedCodeWriteable(false);
        // but not to those of active overwrite loops
        std::set<Address> pages;
        hybridOW()->activeOverwritePages(pages);
        for (set<Address>::iterator pit = pages.begin(); 
             pit != pages.end(); 
             pit++) 
        {
            proc()->setMemoryAccessRights(*pit,
                proc()->lowlevel_process()->getMemoryPageSize(),
                getOrigPageRights(*pit));
        }
        // KEVINTODO: if there are other in edges to the fallthrough block it 
        //            might be cheaper to remove synch instrumentation at this point
    }
}


#if !defined (os_windows)
void HybridAnalysis::signalHandlerCB(BPatch_point *,long,std::vector<Address>&)
{}

bool HybridAnalysis::registerCodeDiscoveryCallback
        (BPatchCodeDiscoveryCallback )
{
    return true;
}

bool HybridAnalysis::registerSignalHandlerCallback
        (BPatchSignalHandlerCallback )
{
    return true;
}

bool HybridAnalysis::removeCodeDiscoveryCallback()
{
    return false;
}

bool HybridAnalysis::removeSignalHandlerCallback()
{
    return false;
}

#else 

static void signalHandlerEntryCB_wrapper(BPatch_point *point, void *excRecAddr) 
{ 
    dynamic_cast<BPatch_process*>(point->getFunction()->getProc())->
        getHybridAnalysis()->signalHandlerEntryCB(point,(Address)excRecAddr); 
}

bool HybridAnalysis::registerCodeDiscoveryCallback
        (BPatchCodeDiscoveryCallback cb)
{
    bpatchCodeDiscoveryCB = cb;
    return true;
}

bool HybridAnalysis::registerSignalHandlerCallback
        (BPatchSignalHandlerCallback cb)
{
    bpatchSignalHandlerCB = cb;
    return true;
}

bool HybridAnalysis::removeCodeDiscoveryCallback()
{
    if (bpatchCodeDiscoveryCB) {
        bpatchCodeDiscoveryCB = NULL;
        return true;
    }
    return false;
}

bool HybridAnalysis::removeSignalHandlerCallback()
{
    if (bpatchSignalHandlerCB) {
        bpatchSignalHandlerCB = NULL;
        return true;
    }
    return false;
}


/* Parse and instrument any signal handlers that have yet to be
 * analyzed (and instrumented), and that are not in system libraries
 */
void HybridAnalysis::signalHandlerCB(BPatch_point *point, long signum, 
                                     std::vector<Address> &handlers) 
{
    mal_printf("signalHandlerCB(%lx , %lx, %d handlers)\n", 
               point->getAddress(), signum, handlers.size());
    bool onlySysHandlers = true;
    std::vector<Address> handlerAddrs;
    proc()->beginInsertionSet();
    // eliminate any handlers in system libraries, and handlers that have 
    // already been parsed, add new handlers to handler function list
    std::vector<Address>::iterator it=handlers.begin();
    while (it < handlers.end())
    {
        BPatch_module *mod = proc()->findModuleByAddr(*it);
        if (!mod || mod->isSystemLib()) {
            it = handlers.erase(it);
            continue;
        } 

        BPatch_function *handlerFunc = proc()->findFunctionByEntry(*it);
        handlerFunctions[*it] = 0;
        if (handlerFunc) {
            it = handlers.erase(it);
            continue;
        } 

        // parse and instrument handler
        mal_printf("found handler at %x %s[%d]\n", *it,FILE__,__LINE__);
        onlySysHandlers = false;
        analyzeNewFunction(point,*it,true,false);
        handlerFunc = proc()->findFunctionByEntry(*it);
        assert(handlerFunc);
        handlerAddrs.push_back(*it);
        BPatch_point *entryPt =  (*handlerFunc->findPoint(BPatch_entry))[0];

        // instrument the handler at its entry and exit points
        proc()->beginInsertionSet();

        // instrument handler entry with callback that will deliver the stack 
        // address at which the fault addr is stored
        BPatch_paramExpr excRecAddr(0,BPatch_ploc_entry);
        BPatch_stopThreadExpr sThread2
            (signalHandlerEntryCB_wrapper,excRecAddr,false,BPatch_noInterp);
        proc()->insertSnippet(sThread2, *entryPt);

        // remove any exit-point instrumentation and add new instrumentation 
        // at exit points
        proc()->beginInsertionSet();
        std::map<BPatch_point*,BPatchSnippetHandle*> *funcPoints = 
            (*instrumentedFuncs)[handlerFunc];
        if ( funcPoints ) {
            std::map<BPatch_point*, BPatchSnippetHandle*>::iterator pit;
            pit = funcPoints->begin();
            while (pit != funcPoints->end())
            {
                if ( BPatch_exit == (*pit).first->getPointType() ) {
                    proc()->deleteSnippet((*pit).second);
                    funcPoints->erase( (*pit).first );
                    pit = funcPoints->begin();                    
                } else {
                    pit++;
                }
            }
        }
        instrumentFunction(handlerFunc, false, true);

        it++;
    }
    proc()->finalizeInsertionSet(false);

    // trigger the signal-handler callback
    if (bpatchSignalHandlerCB) {
        bpatchSignalHandlerCB(point,signum,&handlerAddrs);
    }
}
#endif


/* Invoked twice for every signal handler function, the first time we
 * just adjust the value of the saved fault address to its unrelocated 
 * counterpart (in the EXCEPTION_RECORD), the second time we do this
 * translation for the CONTEXT structure, containing the PC that is used
 * when execution resumes, and we replace instrumentation at the handler's 
 * exit points because we didn't know the contextAddr before 
 */
void HybridAnalysis::signalHandlerEntryCB(BPatch_point *point, Address excRecAddr)
{
    mal_printf("\nAt signalHandlerEntry(%lx , %lx)\n", 
               point->getAddress(), (Address)excRecAddr);

    // calculate the offset of the fault address in the EXCEPTION_RECORD
    EXCEPTION_RECORD *tmpRec = (EXCEPTION_RECORD*)excRecAddr; //bogus pointer, but I won't write to it
    Address pcAddr = excRecAddr + (Address)(&(tmpRec->ExceptionAddress)) - (Address)tmpRec;

    // save address of context information for the exit point handler
    BPatch_function *func = point->getFunction();

    func->setHandlerFaultAddrAddr((Address)pcAddr,true);
    handlerFunctions[(Address)func->getBaseAddr()] = (Address)excRecAddr;
}

/* If the context of the exception has been changed so that execution
 * will resume at a new address, parse and instrument the code at that
 * address
 */
void HybridAnalysis::signalHandlerExitCB(BPatch_point *point, void *dontcare)
{
    BPatch_function *func = point->getFunction();
    assert(handlerFunctions.end() != handlerFunctions.find((Address)func->getBaseAddr()) && 
           0 != handlerFunctions[(Address)func->getBaseAddr()]);
    Address erLoc = handlerFunctions[(Address)func->getBaseAddr()];

    mal_printf("\nAt signalHandlerExit(%lx)\n", point->getAddress());

    // figure out the address the program will resume at by reading in the stored EXCEPTION_RECORD
    EXCEPTION_RECORD er;
    proc()->lowlevel_process()->readDataSpace(
        (void*)erLoc, sizeof(EXCEPTION_RECORD), &er, true);

    Address resumePC = (Address) er.ExceptionAddress;
    if (er.ExceptionCode == EXCEPTION_BREAKPOINT) {
        resumePC += 1;
    }
    mal_printf("Program will resume at %lx\n", resumePC);

    // restore the effAddr of the 

    // parse at the resumePC address, if necessary
    vector<BPatch_function *> funcs;
    proc()->findFunctionsByAddr((Address)resumePC,funcs);
    if (funcs.empty()) {
        analyzeNewFunction(point, (Address)resumePC, true, true);
    }
    point->getFunction()->fixHandlerReturnAddr((Address)resumePC);
    mal_printf("Exception handler exiting at %lx will resume execution at "
                "%lx %s[%d]\n",
                point->getAddress(), resumePC, FILE__,__LINE__);
}

void HybridAnalysis::abruptEndCB(BPatch_point *point, void *) 
{
    Address pointAddr = (Address) point->getAddress();
    mal_printf("\nabruptEndCB at %lx in function at %lx\n", 
                pointAddr, point->getFunction()->getBaseAddr());

    // parse, immediately after the current block
    vector<Address> *targets = new vector<Address>;
    Address nextInsn =0;
    point->getCFTargets(*targets);
    assert(targets->size());
    nextInsn = (*targets)[0];
    delete(targets);

    proc()->beginInsertionSet();
    // add the new edge to the program, parseNewEdgeInFunction will figure
    // out whether to extend the current function or parse as a new one. 
    assert(0 && "KEVINTODO: turn on non-existent parsing flag to force parsing of overlapping blocks and weird instructions");
    parseNewEdgeInFunction(point, nextInsn, false);

    //make sure we don't re-instrument
    point->setResolved();

    // re-instrument the module 
    instrumentModules(false);
    proc()->finalizeInsertionSet(false);
}

// Look up the memory region, and unmap it if it corresponds to a mapped object

void HybridAnalysis::virtualFreeAddrCB(BPatch_point *, void *addr) {
	cerr << "Setting virtualFree addr to " << hex << (Address) addr << dec << endl;
	virtualFreeAddr_ = (Address) addr;
	return;
}

void HybridAnalysis::virtualFreeSizeCB(BPatch_point *, void *size) {
	cerr << "Setting virtualFree size to " << (unsigned) size << endl;
	virtualFreeSize_ = (unsigned) size;
	return;
}

void HybridAnalysis::virtualFreeCB(BPatch_point *, void *t) {
	assert(virtualFreeAddr_ != 0);
	unsigned type = (unsigned) t;
	cerr << "virtualSizeFree [" << hex << virtualFreeAddr_ << "," << virtualFreeAddr_ + (unsigned) virtualFreeSize_ << "], " << (unsigned) type << dec << endl;

	Address pageSize = proc()->lowlevel_process()->getMemoryPageSize();

	// Windows page-aligns everything.

	unsigned addrShift = virtualFreeAddr_ % pageSize;
	unsigned sizeShift = pageSize - (virtualFreeSize_ % pageSize);

	virtualFreeAddr_ -= addrShift;

	if (type != MEM_RELEASE)
	{
		virtualFreeSize_ += addrShift + sizeShift;
	}

	// We need to:
	// 1) Remove any function with a block in the deleted range
	// 2) Remove memory translation for that range
	// 3) Skip trying to set permissions for any page in the range.

	// DEBUG!
	if (1 || type == MEM_RELEASE)
	{
		mapped_object *obj = proc()->lowlevel_process()->findObject(virtualFreeAddr_);

		if (!obj) return;
		virtualFreeAddr_ = obj->codeBase();
		virtualFreeSize_ = obj->imageSize();
		// DEBUG!
		cerr << "Removing shared object " << obj->fileName() << endl;
		proc()->lowlevel_process()->removeASharedObject(obj);
		virtualFreeAddr_ = 0;
		return;
	}

	std::set<int_function *> deletedFuncs;
	for (Address i = virtualFreeAddr_; i < (virtualFreeAddr_ + virtualFreeSize_); ++i) {
		proc()->lowlevel_process()->findFuncsByAddr(i, deletedFuncs);
	}
	for (std::set<int_function *>::iterator iter = deletedFuncs.begin();
		iter != deletedFuncs.end(); ++iter)
	{
		BPatch_function * bpfunc = proc()->findOrCreateBPFunc(*iter, NULL);
		if (!bpfunc) continue;
		bpfunc->getModule()->removeFunction(bpfunc, true);
	}

	proc()->lowlevel_process()->getMemEm()->removeRegion(virtualFreeAddr_, virtualFreeSize_);
	// And nuke the RT cache

	proc()->lowlevel_process()->proc()->flushAddressCache_RT(virtualFreeAddr_, virtualFreeSize_);

	virtualFreeAddr_ = 0;
	return;
}

/* CASES (sub-numbering are cases too)
 * 1. the target address is in a shared library
 * 1.1 if it's a system library don't parse at the target, but if the point was marked 
 *     as a possibly non-returning indirect call, parse at its fallthrough addr
 *     and return.
 * 1.2 if the target is in the runtime library, translate to an unrelocated address
 *     and continue, but for now assert, since this translation is happening internally
 * 2. the point is an call: 
 * 2.1 if the target is new, parse at the target
 * 2. if the target is a returning function, parse at the fallthrough address
 * 2. return
 * 3. the point is a return instruction:
 * 3. find the call point 
 * 3.1 if the return address has been parsed as code, return
 * 3.2.1 if the return addr follows a call, parse it as its fallthrough edge
 * 3.2.2 else parse the return addr as a new function
 * 3. return
 * 4. else case: the point is a direct transfer or an indirect jump/branch.
 * 4.1 if the point is a direct transfer: 
 * 4.1.1 remove instrumentation
 * 4. parse at the target if it is code
 * KEVINTODO: split into phases: parse, instrument
 */
extern bool debug_blocks;
void HybridAnalysis::badTransferCB(BPatch_point *point, void *returnValue) 
{
    Address pointAddr = (Address) point->getAddress();
    Address target = (Address) returnValue;

    time_t tstruct;
    struct tm * tmstruct;
    char timeStr[64];
    time( &tstruct );
    tmstruct = localtime( &tstruct );
    strftime(timeStr, 64, "%X", tmstruct);
    printf("badTransferCB %lx=>%lx %s\n\n", point->getAddress(), target, timeStr);

    BPatch_module * targMod = proc()->findModuleByAddr(target);
    if (!targMod) {
        mal_printf( "ERROR, NO MODULE for target addr %lx %s[%d]\n", 
                target,FILE__,__LINE__);
        assert(0);
    }

// 1. the target address is in a shared library
    bool processTargMod = false;
    if ( targMod != point->getFunction()->getModule() ) {
        // process the edge, decide if we should instrument target function
        processTargMod = processInterModuleEdge(point, target, targMod);

        if (!processTargMod) {
            return;
        }
    }

// 2. the point is a call: 
    if (point->getPointType() == BPatch_subroutine) {

        proc()->beginInsertionSet();
        // if the target is in the body of an existing function we'll split 
        // the function and wind up with two or more functions that share
        // the target address, so make sure we're not in the middle of an
        // overwrite loop; if we are, check for overwrites immediately
        BPatch_function *targFunc = proc()->findFunctionByEntry(target);
        vector<BPatch_function*> targFuncs;
        proc()->findFunctionsByAddr(target, targFuncs);
        if (!targFunc && targFuncs.size()) {
            mal_printf("discovery instr. got new entry point for func\n");
            std::set<HybridAnalysisOW::owLoop*> loops;
            for (unsigned tidx=0; tidx < targFuncs.size(); tidx++) {
                BPatch_function *curFunc = targFuncs[tidx];
                if ( hybridOW()->hasLoopInstrumentation(false, *curFunc, &loops) )
                {
                    /* Code sharing will change the loops, the appropriate response
                    is to trigger early exit analysis and remove the loops if 
                    the underlying code hasn't changed */
                    mal_printf("[%d] Removing loop instrumentation for func %lx\n", 
                                __LINE__,curFunc->getBaseAddr());
                    std::set<HybridAnalysisOW::owLoop*>::iterator lIter = 
                        loops.begin();
                    while (lIter != loops.end()) {
                        hybridOW()->deleteLoop(*lIter,false);
                        lIter++;
                    }
                }
            }
        }

        // 2.1 if the target is new, parse at the target
        if ( ! targFunc ) {
            mal_printf("stopThread instrumentation found call %lx=>%lx, "
                      "parsing at call target %s[%d]\n",
                     (long)point->getAddress(), target,FILE__,__LINE__);
            if (!analyzeNewFunction( point,target,true,false )) {
                //this happens for some single-instruction functions
                mal_printf("WARNING: parse of call target %lx=>%lx failed %s[%d]\n",
                         (long)point->getAddress(), target, FILE__,__LINE__);
            }
            targFunc = proc()->findFunctionByEntry(target);
        }

        // 2.2 if the target is a returning function, parse at the fallthrough
        if ( ParseAPI::RETURN == 
             targFunc->lowlevel_func()->ifunc()->retstatus() ) 
        {
            //mal_printf("stopThread instrumentation found returning call %lx=>%lx, "
            //          "parsing after call site\n",
            //         (long)point->getAddress(), target);
            parseAfterCallAndInstrument(point, targFunc);
        } else {
            instrumentModules(false);
        }
        proc()->finalizeInsertionSet(false);
        // 2. return
        return;
    }

// 3. the point is a return instruction:
    if ( point->getPointType() == BPatch_locExit ) {

        // 3.1 if the return address has been parsed as code, return
        vector<BPatch_function*> callFuncs;
        if ( proc()->findFunctionsByAddr( target,callFuncs ) ) {
            return;
        }
        // 3.2 find the call point so we can parse after it
        //   ( In this case "point" is the return statement and 
        //   "target" is the fallthrough address of the call insn )
        //   in order to find the callPoint in the caller function that 
        //   corresponds to the non-returning call, we traverse list of
        //   the caller's points to find the callpoint that is nearest
        //   to the return address
        Address returnAddr = target;
        BPatch_point *callPoint = NULL;
        proc()->findFunctionsByAddr( ( (Address)returnAddr - 1 ) , callFuncs );
        if ( !callFuncs.empty() ) {
            // get the call point whose fallthrough addr matches the ret target
            vector<BPatch_point *> *callPoints = 
                callFuncs[0]->findPoint(BPatch_subroutine);
            for (int pIdx = callPoints->size() -1; pIdx >= 0; pIdx--) {
                if ((*callPoints)[pIdx]->getCallFallThroughAddr() == returnAddr) {
                    callPoint = (*callPoints)[pIdx];
                    break;
                }
            }
			if (!callPoint) {
				// It's possible that this was an entry point that overlapped with
				// the call site. Dyninst doesn't handle that well...
				vector<BPatch_point *> *entryPoints = callFuncs[0]->findPoint(BPatch_entry);
				for (int pIdx = entryPoints->size() - 1; pIdx >= 0; pIdx--) {
					if ((*entryPoints)[pIdx]->getCallFallThroughAddr() == returnAddr) {
						callPoint = (*entryPoints)[pIdx];
						break;
						}
					}
				}

            if (callPoint && callFuncs.size() > 1) {
                //KEVINTODO: implement this case
                mal_printf("ERROR: callPoint %lx is shared, test this case\n",
                           callPoint->getAddress());
            } 
        }

        // 3.2.1 if the return addr follows a call to this function, parse at 
        // its fallthrough edge
        bool parseAtFT = false;
        if (callPoint) {
            vector<Address> targs;
            callPoint->getSavedTargets(targs);
            // unfortunately, because of pc emulation, if the return point is 
            // shared we may have flipped between functions that share the 
            // return point, so check both
            vector<ParseAPI::Function*> retfuncs;
            point->llpoint()->block()->llb()->getFuncs(retfuncs);
            Address base = point->llpoint()->func()->obj()->codeBase();
            for (unsigned tidx=0; !parseAtFT && tidx < targs.size(); tidx++) {
                for (unsigned fidx=0; !parseAtFT && fidx < retfuncs.size(); fidx++) {
                    if (targs[tidx] == (base + retfuncs[fidx]->addr()) ) {
                        parseAtFT = true;
                    }
                }
            }
        }
        if ( parseAtFT ) {
            mal_printf("stopThread instrumentation found return at %lx, "
                      "parsing return addr %lx as fallthrough of call "
                      "instruction at %lx %s[%d]\n", (long)point->getAddress(), 
                      target,callPoint->getAddress(),FILE__,__LINE__);
            parseAfterCallAndInstrument( callPoint, point->getFunction() );
        }

        // 3.2.2 else parse the return addr as a new function
        else {
            if ( point->getFunction()->getModule()->isExploratoryModeOn() ) {
                // otherwise we've instrumented a function in trusted library
                // because we want to catch its callbacks into our code, but in
                // the process are catching calls into other modules
                mal_printf("hybridCallbacks.C[%d] Observed abuse of normal return "
                        "instruction semantics for insn at %lx target %lx\n",
                        __LINE__, point->getAddress(), returnAddr);
            }
            analyzeNewFunction( point, returnAddr, true , true );
        }

        // 3. return
        return;
    }
    else {
    // 4. else case: the point is a jump/branch 
        proc()->beginInsertionSet();
        // 4.1 if the point is a direct branch, remove any instrumentation
        vector<Address> *targets = new vector<Address>;
        if ( point->getCFTargets(*targets) ) {
            BPatch_function *func = point->getFunction();
            if (instrumentedFuncs->end() != instrumentedFuncs->find(func)
                &&
                (*instrumentedFuncs)[func]->end() != 
                (*instrumentedFuncs)[func]->find(point))
            {
                proc()->deleteSnippet(
                    (*(*instrumentedFuncs)[func])[point] );
                (*instrumentedFuncs)[func]->erase(point);
            }
            //point is set to resolved in handleStopThread
        } 
        delete(targets);

        bool newParsing;
        vector<BPatch_function*> targFuncs;
        proc()->findFunctionsByAddr(target, targFuncs);
        if ( 0 == targFuncs.size() ) { 
            newParsing = true;
            mal_printf("stopThread instrumentation found jump "
                    "at 0x%lx leading to an unparsed target at 0x%lx\n",
                    (long)point->getAddress(), target);
        } else {
            newParsing = false;
            mal_printf("stopThread instrumentation added an edge for jump "
                    " at 0x%lx leading to a previously parsed target at 0x%lx\n",
                    (long)point->getAddress(), target);
        }

        // add the new edge to the program, parseNewEdgeInFunction will figure
        // out whether to extend the current function or parse as a new one. 
        parseNewEdgeInFunction(point, target, false);
        if (0 == targFuncs.size()) {
            proc()->findFunctionsByAddr( target, targFuncs );
        }

        // manipulate init_retstatus so that we will instrument the function's 
        // return addresses, since this jump might be a tail call
        for (unsigned tidx=0; tidx < targFuncs.size(); tidx++) {
            image_func *imgfunc = targFuncs[tidx]->lowlevel_func()->ifunc();
            FuncReturnStatus initStatus = imgfunc->init_retstatus();
            if (ParseAPI::RETURN == initStatus) {
                imgfunc->setinit_retstatus(ParseAPI::UNKNOWN);
                removeInstrumentation(targFuncs[tidx],false);
                instrumentFunction(targFuncs[tidx],false,true);
            } 
        }

        // re-instrument the function or the whole module, as needed
        if (newParsing) {
            instrumentModules(false);
        }
        proc()->finalizeInsertionSet(false);
    }
} // end badTransferCB
