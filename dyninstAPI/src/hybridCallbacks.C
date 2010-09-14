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
#include "BPatch_process.h"
#include "BPatch_function.h"
#include "BPatch_module.h"
#include "instPoint.h"
#include "function.h"


void newCodeCB(std::vector<BPatch_function*> &newFuncs, 
               std::vector<BPatch_function*> &modFuncs)
{
    printf("new functions %d mod functions %d %s[%d]\n", 
           (int)newFuncs.size(), (int)modFuncs.size(), FILE__, __LINE__);
}


/* Invoked twice for every signal handler function, the first time we
 * just adjust the value of the saved fault address to its unrelocated 
 * counterpart (in the EXCEPTION_RECORD), the second time we do this
 * translation for the CONTEXT structure, which is the PC that is used
 * when execution resumes, and we replace instrumentation at the handler's 
 * exit points because we didn't know the contextAddr before 
 */
static bool firstHandlerEntry = true;
void HybridAnalysis::signalHandlerEntryCB(BPatch_point *point, void *pcAddr)
{
    mal_printf("\nAt signalHandlerEntry(%lx , %lx)\n", 
                point->getAddress(), (Address)pcAddr);

    // save address of context information for the exit point handler
    BPatch_function *func = point->getFunction();
    if (firstHandlerEntry) {
        func->setHandlerFaultAddrAddr((Address)pcAddr,false);
    } else {
        func->setHandlerFaultAddrAddr((Address)pcAddr,true);
        handlerFunctions[(Address)func->getBaseAddr()] = (Address)pcAddr;
        // remove any exit-point instrumentation and add new instrumentation 
        // at exit points
        if ( instrumentedFuncs->end() != instrumentedFuncs->find(func) ) {
            std::map<BPatch_point*, BPatchSnippetHandle*>::iterator pit;
            for (pit  = (*instrumentedFuncs)[func]->begin(); 
                 pit != (*instrumentedFuncs)[func]->end(); 
                 pit++) 
            {
                if ( BPatch_exit == (*pit).first->getPointType() ) {
                    proc()->deleteSnippet((*pit).second);
                    (*instrumentedFuncs)[func]->erase( (*pit).first );
                }
            }
        }
        instrumentFunction(func, true, true);
    }
    firstHandlerEntry = !firstHandlerEntry;
}

/* If the context of the exception has been changed so that execution
 * will resume at a new address, parse and instrument the code at that
 * address
 */
void HybridAnalysis::signalHandlerExitCB(BPatch_point *point, void *returnAddr)
{
    mal_printf("\nAt signalHandlerExit(%lx , %lx)\n", 
        point->getAddress(), (Address)returnAddr);
    BPatch_function *func = proc()->findFunctionByAddr(returnAddr);
    if (!func) {
        analyzeNewFunction((Address)returnAddr, true);
    }
    point->getFunction()->fixHandlerReturnAddr((Address)returnAddr);
    mal_printf("Exception handler exiting at %lx will resume execution at "
                "%lx %s[%d]\n",
                point->getAddress(), returnAddr, FILE__,__LINE__);
}


#if !defined (os_windows)
void HybridAnalysis::signalHandlerCB(BPatch_point *,long,std::vector<Address>&)
{}
#else 

static void signalHandlerEntryCB_wrapper(BPatch_point *point, void *returnAddr) 
{ 
    dynamic_cast<BPatch_process*>(point->getFunction()->getProc())->
        getHybridAnalysis()->signalHandlerEntryCB(point,returnAddr); 
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

    // eliminate any handlers in system libraries, and handlers that have 
    // already been parsed, add new handlers to handler function list
    std::vector<Address>::iterator it=handlers.begin();
    while (it < handlers.end())
    {
        BPatch_function *func = proc()->findFunctionByAddr((void*)*it);
        BPatch_module *mod = proc()->findModuleByAddr(*it);
        if (func) {
            handlerAddrs.push_back((Address)func->getBaseAddr());
        }

        if (mod->isSystemLib()) {
            it = handlers.erase(it);
            continue;
        } 

        this->handlerFunctions[*it] = 0;
        if (func) {
            it = handlers.erase(it);
            continue;
        } 

        // parse and instrument handler
        mal_printf("found handler at %x %s[%d]\n", *it,FILE__,__LINE__);
        onlySysHandlers = false;
        analyzeNewFunction(*it,true);

        proc()->beginInsertionSet();
        // get relative position of fields in the EXCEPTION_RECORD struct
        BPatch_function *handlerFunc = proc()->findFunctionByAddr((void*)*it);
        EXCEPTION_RECORD *tmpRec = (EXCEPTION_RECORD*)mod; //bogus pointer, but I won't write to it
        CONTEXT *tmpCtxt         = (CONTEXT*)         mod; //bogus pointer, but I won't write to it
        Address excAddrPosition = (Address)(&(tmpRec->ExceptionAddress)) - (Address)tmpRec;
        Address eipPosition     = (Address)(&(tmpCtxt->Eip))             - (Address)tmpCtxt;

        BPatch_paramExpr contextAddr(2);
        BPatch_arithExpr contextPCaddr
            (BPatch_plus, contextAddr, BPatch_constExpr(eipPosition));
        BPatch_stopThreadExpr sThread1
            (signalHandlerEntryCB_wrapper,contextPCaddr,false,BPatch_noInterp);
        proc()->insertSnippet(sThread1, 
                               *(*handlerFunc->findPoint(BPatch_entry))[0]);

        BPatch_paramExpr excRecAddr(0);
        BPatch_arithExpr excSrcAddr
            (BPatch_plus, excRecAddr, BPatch_constExpr(excAddrPosition));
        BPatch_stopThreadExpr sThread2
            (signalHandlerEntryCB_wrapper,excSrcAddr,false,BPatch_noInterp);
        proc()->insertSnippet(sThread2, 
                               *(*handlerFunc->findPoint(BPatch_entry))[0]);


        proc()->finalizeInsertionSet(false);
        it++;
    }

    // trigger the signal-handler callback
    if (bpatchSignalHandlerCB) {
        bpatchSignalHandlerCB(point,signum,&handlerAddrs);
    }
}
#endif

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

    // add the new edge to the program, parseNewEdgeInFunction will figure
    // out whether to extend the current function or parse as a new one. 
    parseNewEdgeInFunction(point, nextInsn);

    //make sure we don't re-instrument
    point->setResolved();

    // re-instrument the module 
    instrumentModules();
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
 */
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
    mal_printf("badTransferCB %lx=>%lx %s\n\n", point->getAddress(), target, timeStr);

    // 1. the target address is in a shared library
    BPatch_module * targMod = proc()->findModuleByAddr(target);
    if (!targMod) {
        mal_printf( "ERROR, NO MODULE for target addr %lx %s[%d]\n", 
                target,FILE__,__LINE__);
        assert(0);
    }
    if ( targMod->isSharedLib() ) {
        vector<BPatch_function*> targFuncs;
        targMod->findFunctionByAddress((void*)target,targFuncs,false,true);
        char modName[16]; 
        char funcName[32];
        targMod->getName(modName,16);
        if (targFuncs.size()) {
            targFuncs[0]->getName(funcName,32);
            mal_printf("%lx => %lx, in module %s to known func %s\n",
                        point->getAddress(),target,modName,funcName);
        } else {
            funcName[0]= '\0';
            mal_printf("%lx => %lx, in module %s \n",
                        point->getAddress(),target,modName,funcName);
        }
        // 1.1 if targMod is a system library don't parse at target.  However, if the 
        //     transfer into the targMod is an unresolved indirect call, parse at the 
        //     call's fallthrough addr and return.
        if (targMod->isSystemLib()) 
        {
            if (point->getPointType() == BPatch_subroutine) {
                if (0 == strncmp(funcName,"ExitProcess",32) && 
                    0 == strncmp(modName,"kernel32.dll",16)) 
                {
                    fprintf(stderr,"Caught call to %s, should exit soon %s[%d]\n", 
                            funcName,FILE__,__LINE__);
                    return;
                }
                mal_printf("stopThread instrumentation found call %lx=>%lx, "
                          "target is in module %s, parsing at fallthrough %s[%d]\n",
                          (long)point->getAddress(), target, modName,FILE__,__LINE__);
                if (targFuncs.size()) {
                    parseAfterCallAndInstrument(point, target, targFuncs[0]);
                } else {
                    parseAfterCallAndInstrument(point, target, NULL);
                }
                return;
			} else if (point->getPointType() == BPatch_exit) {
				mal_printf("WARNING: stopThread instrumentation found return %lx=>%lx, "
                          "into module %s, this indicates obfuscation or that there was a "
						  "call from that module into our code %s[%d]\n",
                          (long)point->getAddress(), target, modName,FILE__,__LINE__);
			}
			else { // jump into system library
                // this is usually symptomatic of the following:
                // call thunk1
                //    ...
                // .thunk1
                // jump ptr
                mal_printf("WARNING: transfer into non-instrumented system module "
                            "%s at: %lx=>%lx %s[%d]\n", modName, 
                            (long)point->getAddress(), target,FILE__,__LINE__);

                // Instrument the return instructions of the system library 
                // function so we can find the code at the call instruction's
                // fallthrough address
				analyzeNewFunction(target,false);
				BPatch_function *targFunc = proc()->findFunctionByAddr((void*)target);
				assert(targFunc);
				instrumentFunction(targFunc, true, true);
                return;
            }
        }
        // 1.2 if targMod is a non-system library, then warn, and fall through into
        // handling the transfer as we would any other transfer
        else if ( targMod->isExploratoryModeOn() ) { 
            mal_printf("WARNING: Transfer into non-instrumented module %s "
                    "func %s at: %lx=>%lx %s[%d]\n", modName, funcName, (long)point->getAddress(), 
                    target, FILE__,__LINE__);
        } else { // jumped or called into module that's not recognized as a 
                 // system library and is not instrumented
            if (pointAddr != 0x77c39d78) {
                mal_printf("WARNING: Transfer into non-instrumented module "
                        "%s func %s that is not recognized as a system lib: "
                        "%lx=>%lx [%d]\n", modName, funcName, 
                        (long)point->getAddress(), target, FILE__,__LINE__);
            }
            return; // triggers for nspack's transfer into space that's 
                    // allocated at runtime
        }
    }

// 2. the point is a call: 
    if (point->getPointType() == BPatch_subroutine) {

        // 2.1 if the target is new, parse at the target
        BPatch_function *targFunc = proc()->findFunctionByAddr((void*)target);
        if ( ! targFunc ) {
            mal_printf("stopThread instrumentation found call %lx=>%lx, "
                      "parsing at call target %s[%d]\n",
                     (long)point->getAddress(), target,FILE__,__LINE__);
            if (analyzeNewFunction( target,true )) {
                //this happens for some single-instruction functions
                targFunc = proc()->findFunctionByAddr((void*)target);
            }
        }

        // if the target is a new entry point for the target function we'll
        // split the function and wind up with two or more functions that share
        // the target address, so make sure they're both instrumented
        if ( targFunc && (Address)targFunc->getBaseAddr() != target ) {
            // see if the function starting at the target address is 
            // truly unparsed, or if the lookup is just masking its presence
            vector<BPatch_function*> targFuncs;
            proc()->getImage()->findFunction(target,targFuncs);
            unsigned i=0;
            for(; i < targFuncs.size() && 
                  (Address)targFuncs[i]->getBaseAddr() != target; 
                i++);

            if (i == targFuncs.size()) //the function at target is REALLY unparsed
            {
                mal_printf("discovery instr. got new entry point for func\n");
                std::set<HybridAnalysisOW::owLoop*> loops;
                if ( hybridOW()->hasLoopInstrumentation(false, *targFunc, &loops) )
                {
                    /* Code sharing will change the loops, the appropriate response
                    is to trigger early exit analysis and remove the loops if 
                    the underlying code hasn't changed */
                    mal_printf("[%d] Removing loop instrumentation for func %lx\n", 
                                __LINE__,targFunc->getBaseAddr());
                    std::set<HybridAnalysisOW::owLoop*>::iterator lIter = 
                        loops.begin();
                    while (lIter != loops.end()) {
                        hybridOW()->deleteLoop(*lIter);
                        lIter++;
                    }
                }
                //KEVINCOMMENT: not sure why I thought the following was necessary or a good idea 5/26/2010
                ////remove instrumentation 
                //removeInstrumentation(targFunc);

                //analyze new function and re-instrument module
                analyzeNewFunction(target,true);
                proc()->finalizeInsertionSet(false);
            }
        }

        // 2. if the target is a returning function, parse at the fallthrough address
        if ( !targFunc || ParseAPI::RETURN == 
                          targFunc->lowlevel_func()->ifunc()->retstatus() ) 
        {
            //mal_printf("stopThread instrumentation found returning call %lx=>%lx, "
            //          "parsing after call site\n",
            //         (long)point->getAddress(), target);
            parseAfterCallAndInstrument(point, target, targFunc);
        } else {
            instrumentModules();
        }

        // 2. return
        return;
    }

// 3. the point is a return instruction:
    if ( point->getPointType() == BPatch_locExit ) {

        // 3.1 if the return address has been parsed as code, return
        if ( proc()->findFunctionByAddr( (void*)target ) ) {
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
        BPatch_function *caller = proc()->findFunctionByAddr( 
            (void*)( (Address)returnAddr - 1 ) );
        if ( !caller ) {
            mal_printf("hybridCallbacks.C[%d] Observed abuse of normal return "
                    "instruction semantics for insn at %lx target %lx\n",
                    __LINE__, point->getAddress(), returnAddr);
        }
        else {
            // find the call point whose fallthrough address matches the return address
            vector<BPatch_point *> *callPoints = caller->findPoint(BPatch_subroutine);
            for (int pIdx = callPoints->size() -1; pIdx >= 0; pIdx--) {
                if ((*callPoints)[pIdx]->getCallFallThroughAddr() == returnAddr) {
                    callPoint = (*callPoints)[pIdx];
                    break;
                }
            }
        }

        // 3.2.1 if the return addr follows a call, parse it as its fallthrough edge
        if ( callPoint ) {
            mal_printf("stopThread instrumentation found return at %lx, "
                      "parsing return addr %lx as fallthrough of call "
                      "instruction at %lx %s[%d]\n", (long)point->getAddress(), 
                      target,callPoint->getAddress(),FILE__,__LINE__);
            parseAfterCallAndInstrument( callPoint, returnAddr, point->getFunction() );
        }

        // 3.2.2 else parse the return addr as a new function
        else {
            mal_printf("stopThread instrumentation found return at %lx, "
                      "the return address %lx doesn't follow a call insn, "
                      "parsing it as a new function %s[%d]\n",
                      point->getAddress(),returnAddr,FILE__,__LINE__);
            analyzeNewFunction( returnAddr, true );
        }

        // 3. return
        return;
    }

// 4. else case: the point is a jump/branch 

    // 4.1 if the point is a direct branch, remove any instrumentation
    vector<Address> *targets = new vector<Address>;
    if ( point->getCFTargets(*targets) ) {
        BPatch_function *func = point->getFunction();
        if ((*instrumentedFuncs)[func]
            &&
            (*instrumentedFuncs)[func]->end() != 
            (*instrumentedFuncs)[func]->find(point))
        {
            proc()->deleteSnippet(
                (*(*instrumentedFuncs)[func])[point] );
            (*instrumentedFuncs)[func]->erase(point);
        }
        //KEVINTODO: currently don't need to resolve the point here, it happens in handleStopThread, what's the better place for it?
        //point->setResolved();
    } 
    delete(targets);

    BPatch_function *targfunc = proc()->findFunctionByAddr( (void*)target );
    if ( targfunc == NULL ) { 
        mal_printf("stopThread instrumentation found jump "
                "at 0x%lx leading to an unparsed target at 0x%lx\n",
                (long)point->getAddress(), target);
    } else {
        mal_printf("stopThread instrumentation added an edge for jump "
                " at 0x%lx leading to a previously parsed target at 0x%lx\n",
                (long)point->getAddress(), target);
    }

    // add the new edge to the program, parseNewEdgeInFunction will figure
    // out whether to extend the current function or parse as a new one. 
    parseNewEdgeInFunction(point, target);

    // re-instrument the module if the target hadn't been parsed
    if ( targfunc == NULL ) { 
        instrumentModules();
    }

} // end badTransferCB


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
