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

#include "hybridAnalysis.h"
#include "debug.h"
#include "BPatch_point.h"
#include "BPatch_process.h"
#include "BPatch_function.h"
#include "BPatch_module.h"
#include "instPoint.h"
#include "function.h"
#include "dynProcess.h"
#include "PatchModifier.h"
#include "BPatch_image.h"
#include "mapped_object.h"

extern std::vector<image*> allImages;

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

    std::vector<BPatch_module *> *mods = proc()->getImage()->getModules();

    // fix up page rights so that the program can proceed
    for (unsigned i = 0; i < mods->size(); ++i) {
        (*mods)[i]->setAnalyzedCodeWriteable(toOrig);
    }

    if (!toOrig) {
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

void HybridAnalysis::signalHandlerEntryCB(BPatch_point *, Address) { }
void HybridAnalysis::signalHandlerEntryCB2(BPatch_point *, Address) {}
void HybridAnalysis::signalHandlerExitCB(BPatch_point *, void *) {}

void HybridAnalysis::virtualFreeAddrCB(BPatch_point *, void *) { }
void HybridAnalysis::virtualFreeSizeCB(BPatch_point *, void *) { }
void HybridAnalysis::virtualFreeCB(BPatch_point *, void *) { }

void HybridAnalysis::abruptEndCB(BPatch_point *, void *) { }
#else 

static void signalHandlerEntryCB_wrapper(BPatch_point *point, void *excRecAddr) 
{ 
    dynamic_cast<BPatch_process*>(point->getFunction()->getProc())->
        getHybridAnalysis()->signalHandlerEntryCB(point,(Address)excRecAddr); 
}

static void signalHandlerEntryCB2_wrapper(BPatch_point *point, void *excCtxtAddr) 
{ 
    dynamic_cast<BPatch_process*>(point->getFunction()->getProc())->
        getHybridAnalysis()->signalHandlerEntryCB2(point,(Address)excCtxtAddr); 
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
        handlerFunctions[*it] = ExceptionDetails();
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

        // instrument handler entry with callbacks that will deliver the stack 
        // addresses at which the fault addr is stored
        BPatch_paramExpr excRecAddr(0,BPatch_ploc_entry);
        BPatch_paramExpr excCtxtAddr(2,BPatch_ploc_entry);
        BPatch_stopThreadExpr sThread1
            (signalHandlerEntryCB_wrapper,excRecAddr,false,BPatch_noInterp);
        BPatch_stopThreadExpr sThread2
            (signalHandlerEntryCB2_wrapper,excCtxtAddr,false,BPatch_noInterp);
        proc()->insertSnippet(sThread2, *entryPt);
        proc()->insertSnippet(sThread1, *entryPt);

        // remove any exit-point instrumentation and add new instrumentation 
        // at exit points
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


/* Invoked for every signal handler function, adjusts the value of the saved 
 * fault address to its unrelocated counterpart in the EXCEPTION_RECORD
 */
void HybridAnalysis::signalHandlerEntryCB(BPatch_point *point, Address excRecAddr)
{
    mal_printf("\nAt signalHandlerEntry(%lx , %lx)\n", 
               point->getAddress(), (Address)excRecAddr);
    stats_.exceptions++;
    // calculate the offset of the fault address in the EXCEPTION_RECORD
    EXCEPTION_RECORD record;
    proc()->lowlevel_process()->readDataSpace(
        (void*)excRecAddr, sizeof(EXCEPTION_RECORD), &record, true);
    Address pcAddr = excRecAddr 
        + (Address) &(record.ExceptionAddress) 
        - (Address) &record;

    // set fault address to the unrelocated address of that instruction
    BPatch_function *func = point->getFunction();
    func->setHandlerFaultAddrAddr((Address)pcAddr,false);
    handlerFunctions[(Address)func->getBaseAddr()].isInterrupt = 
        (record.ExceptionCode == EXCEPTION_BREAKPOINT);
}

/* Invoked for every signal handler function, adjusts the value of the saved 
 * fault address to its unrelocated counterpart in the CONTEXT structure,
 * which contains the PC that is used when execution resumes
 */
void HybridAnalysis::signalHandlerEntryCB2(BPatch_point *point, Address excCtxtAddr)
{
    mal_printf("\nAt signalHandlerEntry2(%lx , %lx)\n", 
               point->getAddress(), (Address)excCtxtAddr);

    // calculate the offset of the fault address in the EXCEPTION_RECORD
    CONTEXT *cont= (CONTEXT*)excCtxtAddr; //bogus pointer, but I won't write to it
#ifdef _WIN64
	Address pcAddr = excCtxtAddr + (Address)(&(cont->Rip)) - (Address)cont;
#else
    Address pcAddr = excCtxtAddr + (Address)(&(cont->Eip)) - (Address)cont;
#endif

    // set fault address to the unrelocated address of that instruction
    // and save the PC address in the CONTEXT structure so the exit handler 
    // can read it
    BPatch_function *func = point->getFunction();
    func->setHandlerFaultAddrAddr((Address)pcAddr,true);
    handlerFunctions[(Address)func->getBaseAddr()].faultPCaddr = pcAddr;
}

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
	cerr << "virtualFree [" << hex << virtualFreeAddr_ << "," << virtualFreeAddr_ + (unsigned) virtualFreeSize_ << "], " << (unsigned) type << dec << endl;

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
		cerr << "Removing VirtualAlloc'ed shared object " << obj->fileName() << endl;
      image *img = obj->parse_img();
		proc()->lowlevel_process()->removeASharedObject(obj);
		virtualFreeAddr_ = 0;
      // Since removeASharedObject doesn't actually delete the object, 
      // or its image (even if its refCount==0), make sure the image
      // goes away from global datastructure allImages
      for (unsigned int i=0; i < allImages.size(); i++) {
         if (img == allImages[i]) {
            allImages[i] = allImages.back();
            allImages.pop_back();
         }
      }
		return;
	}

	std::set<func_instance *> deletedFuncs;
	for (Address i = virtualFreeAddr_; i < (virtualFreeAddr_ + virtualFreeSize_); ++i) {
		proc()->lowlevel_process()->findFuncsByAddr(i, deletedFuncs);
	}
	for (std::set<func_instance *>::iterator iter = deletedFuncs.begin();
		iter != deletedFuncs.end(); ++iter)
	{
		BPatch_function * bpfunc = proc()->findOrCreateBPFunc(*iter, NULL);
		if (!bpfunc) continue;
        PatchAPI::PatchModifier::remove(bpfunc->lowlevel_func());
	}
	// And nuke the RT cache

	proc()->lowlevel_process()->proc()->flushAddressCache_RT(virtualFreeAddr_, virtualFreeSize_);

	virtualFreeAddr_ = 0;
	return;
}


/* If the context of the exception has been changed so that execution
 * will resume at a new address, parse and instrument the code at that
 * address; then add a springboard at that address if it is not the 
 * entry point of a function
 */
void HybridAnalysis::signalHandlerExitCB(BPatch_point *point, void *)
{
    BPatch_function *func = point->getFunction();
    std::map<Dyninst::Address, ExceptionDetails>::iterator diter = 
        handlerFunctions.find((Address)func->getBaseAddr());
    assert(handlerFunctions.end() != diter && 
           0 != diter->second.faultPCaddr);
    Address pcLoc = diter->second.faultPCaddr;

    mal_printf("\nAt signalHandlerExit(%lx)\n", point->getAddress());

    // figure out the address the program will resume at by reading 
    // in the stored CONTEXT structure
    Address resumePC;
    assert(sizeof(Address) == proc()->getAddressWidth());
    proc()->lowlevel_process()->readDataSpace(
        (void*)pcLoc, sizeof(resumePC), &resumePC, true);
    if (diter->second.isInterrupt) {
        resumePC += 1;
    }

    // parse at the resumePC address, if necessary
    vector<BPatch_function *> funcs;
    proc()->findFunctionsByAddr((Address)resumePC,funcs);
    if (funcs.empty()) {
        mal_printf("Program will resume in new function at %lx\n", resumePC);
    }
    else {
        mal_printf("Program will resume at %lx in %d existing functions, "
                   "will add shared function starting at %lx\n", 
                   resumePC, funcs.size(), resumePC);
    }
    analyzeNewFunction(point, (Address)resumePC, true, true);

    mal_printf("Exception handler exiting at %lx will resume execution at "
                "%lx %s[%d]\n",
                point->getAddress(), resumePC, FILE__,__LINE__);
}

void HybridAnalysis::abruptEndCB(BPatch_point *point, void *) 
{
    Address pointAddr = (Address) point->getAddress();
    mal_printf("\nabruptEndCB at %lx in function at %lx\n", 
                pointAddr, point->getFunction()->getBaseAddr());
    // before we trigger further parsing, make sure the function is 
    // not just a big chunk of zeroes, in which case the first
    // 00 00 instruction will probably raise an exception
    using namespace ParseAPI;
    func_instance *pfunc = point->llpoint()->func();
    CodeRegion *reg = pfunc->ifunc()->region();
    unsigned char * regptr = (unsigned char *) reg->getPtrToInstruction(reg->offset());
    Address regSize = reg->high() - reg->offset();
    unsigned firstNonzero = point->llpoint()->block()->end() 
        - reg->offset() 
        - pfunc->obj()->codeBase();
    for (; firstNonzero < regSize; firstNonzero++) {
        if (0 != regptr[firstNonzero]) {
            break;
        }
    }
    if (firstNonzero == regSize) {
        mal_printf("Not extending parse after abruptEnd; the code byes are "
                   "zeroes through the end of the binary section and the "
                   "0x0000 instruction is idempotent\n");
        return; // don't do any parsing, there's nothing but 0's to the end 
                // of the section
    }

    // parse, immediately after the current block
    vector<Address> *targets = new vector<Address>;
    Address nextInsn =0;
    point->llpoint()->block()->setNotAbruptEnd();
    getCFTargets(point,*targets);
    if (targets->empty()) {
       nextInsn = point->llpoint()->block()->end();
    } else {
       nextInsn = (*targets)[0];
    }
    delete(targets);

    proc()->beginInsertionSet();
    // add the new edge to the program, parseNewEdgeInFunction will figure
    // out whether to extend the current function or parse as a new one. 
    parseNewEdgeInFunction(point, nextInsn, false);

    // re-instrument the module 
    instrumentModules(false);
    proc()->finalizeInsertionSet(false);
}
#endif

static int getPreCallPoints(ParseAPI::Block* blk, 
                         BPatch_process *proc, 
                         vector<BPatch_point*> &points) 
{
    if (!((parse_block*)blk)->isCallBlock()) {
        return 0;
    }

    using namespace ParseAPI;
    PCProcess *llproc = proc->lowlevel_process();
    vector<ParseAPI::Function*> pFuncs;

    blk->getFuncs(pFuncs); 
    for (vector<Function*>::iterator fit = pFuncs.begin(); 
         fit != pFuncs.end(); 
         fit++) 
    {
        func_instance *iFunc = llproc->findFunction((parse_func*)(*fit));
        instPoint *iPoint = instPoint::preCall(iFunc, iFunc->obj()->findBlock(blk));
        if (iPoint) {
            BPatch_function *bpFunc = proc->findOrCreateBPFunc(iFunc,NULL);
            BPatch_point *bpPoint = proc->findOrCreateBPPoint(
                bpFunc, iPoint, BPatch_subroutine);
            points.push_back(bpPoint);
        }
    }
    return points.size();
}

// Find the call blocks preceding the address that we're returning 
// past, but only set returningCallB if we can be sure that 
// that we've found a call block that actually called the function
// we're returning from 
void HybridAnalysis::getCallBlocks(Address retAddr, 
                   func_instance *retFunc,
                   block_instance *retBlock,
                   pair<ParseAPI::Block*, Address> & returningCallB, // output
                   set<ParseAPI::Block*> & callBlocks) // output
{
   // find blocks at returnAddr -1 
   using namespace ParseAPI;
   PCProcess *llproc = retFunc->proc()->proc();
   mapped_object *callObj = llproc->findObject((Address)retAddr - 1);
   std::set<CodeRegion*> callRegs;
   if (callObj) {
      callObj->parse_img()->codeObject()->cs()->
          findRegions(retAddr - 1 - callObj->codeBase(),callRegs);
   }
   if (!callRegs.empty()) {
      callObj->parse_img()->codeObject()->
          findBlocks(*callRegs.begin(), 
                     retAddr - 1 - callObj->codeBase(),
                     callBlocks);
   }

   // remove blocks at returnAddr -1 that either have no call from callBlocks,
   // or that don't end at the correct address
   for (set<Block*>::iterator bit = callBlocks.begin();
       bit != callBlocks.end();)
   {
      if ((*bit)->end() == (retAddr - callObj->codeBase()) &&
          ((parse_block*)(*bit))->isCallBlock()) 
      {
          bit++;
      }
      else {
          callBlocks.erase(bit);
          bit = callBlocks.begin();
      }
   }

   // set returningCallB, which must call the func that has the return
   // ins'n, or call a function that tail-calls to the return func
   for (set<Block*>::iterator bit = callBlocks.begin();
       returningCallB.first == NULL && bit != callBlocks.end();
       bit++) 
   {
      if (BPatch_defensiveMode != retFunc->obj()->hybridMode()) {
          const func_instance *origF = llproc->isFunctionReplacement(retFunc);
          if (origF) {
              retFunc = const_cast<func_instance*>(origF);
          }
      }

      // check that the target function contains the return insn, 
      // or that it tail-calls to the function containing the ret, 
      // or that the return insn is in a replacement for the called 
      // or tail-called func
      const Block::edgelist & trgs = (*bit)->targets();
      for (Block::edgelist::const_iterator eit = trgs.begin();
           eit != trgs.end(); 
           eit++)
      {
          if ((*eit)->type() == ParseAPI::CALL && 
              !(*eit)->sinkEdge()) 
          {
              Block *calledB = (*eit)->trg();
              Function *calledF = calledB->obj()->
                  findFuncByEntry(calledB->region(), calledB->start());
              if (calledF == retFunc->ifunc() || 
                  calledF->contains(retBlock->llb()))
              {
                  returningCallB.first = *bit;
                  returningCallB.second = calledB->start() + 
                      llproc->findObject(calledB->obj())->codeBase();
                  break; // calledF contains return instruction
              }
              // see if calledF tail-calls to func that has ret point
              // or if calledF has been replaced, by hideDebugger
              const Function::edgelist & calls = calledF->callEdges();
              for (Function::edgelist::const_iterator cit = calls.begin();
                   cit != calls.end();
                   cit++)
              {
                  if (!(*cit)->sinkEdge()) {
                      Block *tailCalledB = (*cit)->trg();
                      Function *tCalledF = tailCalledB->obj()->
                          findFuncByEntry(tailCalledB->region(), 
                                          tailCalledB->start());
                      if (retFunc->ifunc() == tCalledF || 
                          tCalledF->contains(retBlock->llb()))
                      {
                          // make sure it really is a tail call
                          // (i.e., the src has no CALL_FT)
                          returningCallB.first = *bit;
                          returningCallB.second = tailCalledB->start() + 
                              llproc->findObject(tailCalledB->obj())->
                              codeBase();
                          break;
                      }
                  }
              }
          }
      }
   }
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

    mal_printf("badTransferCB %lx=>%lx %s\n\n", pointAddr, target, timeStr);
    BPatch_module * targMod = proc()->findModuleByAddr(target);
    if (!targMod) {
        mal_printf( "ERROR, NO MODULE for target addr %lx %s[%d]\n", 
                target,FILE__,__LINE__);
        assert(0);
    }

    if (targMod == point->getFunction()->getModule() && targMod->isSystemLib()) {
        return;
    }

// 1. the target address is in a shared library
    if ( targMod != point->getFunction()->getModule()) 
    {
        // process the edge, decide if we should instrument target function
        bool doMoreProcessing = processInterModuleEdge(point, target, targMod);

        if (!doMoreProcessing) {
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
                    mal_printf("[%d] Removing loop instrumentation for func %p\n", 
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
            mal_printf("stopThread instrumentation found call %p=>%lx, "
                      "parsing at call target %s[%d]\n",
                     point->getAddress(), target,FILE__,__LINE__);
            if (!analyzeNewFunction( point,target,false,false )) {
                //this happens for some single-instruction functions
                mal_printf("ERROR: parse of call target %p=>%lx failed %s[%d]\n",
                         point->getAddress(), target, FILE__,__LINE__);
                assert(0);
                instrumentModules(false);
                proc()->finalizeInsertionSet(false);
                return;
            }
            targFunc = proc()->findFunctionByEntry(target);
        }

        // 2.2 if the target is a returning function, parse at the fallthrough
        bool instrument = true;
        if ( ParseAPI::RETURN == 
             targFunc->lowlevel_func()->ifunc()->retstatus() ) 
        {
            //mal_printf("stopThread instrumentation found returning call %lx=>%lx, "
            //          "parsing after call site\n",
            //         (long)point->getAddress(), target);
            if (parseAfterCallAndInstrument(point, targFunc, false)) {
               instrument = false;
            }
        } 
        if (instrument) {
            instrumentModules(false);
        }
        proc()->finalizeInsertionSet(false);
        // 2. return
        return;
    }

// 3. the point is a return instruction:
    if ( point->getPointType() == BPatch_locExit ) {

        // 3.2 find the call point so we can parse after it
        //   ( In this case "point" is the return statement and 
        //   "target" is the fallthrough address of the call insn )
        //   in order to find the callPoint in the caller function that 
        //   corresponds to the non-returning call, we traverse list of
        //   the caller's points to find the callpoint that is nearest
        //   to the return address

        Address returnAddr = target;
        using namespace ParseAPI;


        // Find the call blocks preceding the address that we're returning 
        // past, but only set set returningCallB if we can be sure that 
        // that we've found a call block that actually called the function
        // we're returning from 
        pair<Block*, Address> returningCallB((Block*)NULL,0); 
        set<Block*> callBlocks;
        getCallBlocks(returnAddr, point->llpoint()->func(), 
                      point->llpoint()->block(), returningCallB, callBlocks);

        // 3.2.1 parse at returnAddr as the fallthrough of the preceding
        // call block, if there is one 
        if (!callBlocks.empty()) {

            // we don't know if the function we called returns, so 
            // invoke parseAfterCallAndInstrument with NULL as the
            // called func, so we won't try to parse after other 
            // callers to the called func, as it may not actually
            // return in a normal fashion
            if (NULL == returningCallB.first) {
                vector<BPatch_point*> callPts; 
                for (set<Block*>::iterator bit = callBlocks.begin(); 
                     bit != callBlocks.end(); 
                     bit++)
                {
                    getPreCallPoints(*bit, proc(), callPts);
                }
                for (vector<BPatch_point*>::iterator pit = callPts.begin(); 
                     pit != callPts.end(); 
                     pit++)
                {
                    parseAfterCallAndInstrument( *pit, NULL, true );
                }
            }

            // if the return address has been parsed as the entry point
            // of a block, patch post-call areas and return
            else if ( returningCallB.first->obj()->findBlockByEntry(
                          returningCallB.first->region(), 
                          target))
            {
                vector<BPatch_point*> callPts; 
                getPreCallPoints(returningCallB.first, proc(), callPts);
                for (unsigned j=0; j < callPts.size(); j++) {
                    callPts[j]->patchPostCallArea();
                }
            }

            else { // parse at the call fallthrough

                // find one callPoint, any other ones will 
                // be found by parseAfterCallAndInstrument
                vector<BPatch_point*> callPoints;
                getPreCallPoints(returningCallB.first, proc(), callPoints);
                assert(!callPoints.empty());

                mal_printf("stopThread instrumentation found return at %p, "
                          "parsing return addr %lx as fallthrough of call "
                          "instruction at %p %s[%d]\n", point->getAddress(), 
                          target,callPoints[0]->getAddress(),FILE__,__LINE__);

                if (point->llpoint()->block()->llb()->isShared()) {
                    // because of pc emulation, if the return point is shared, 
                    // we may have flipped between functions that share the 
                    // return point, so use the call target function
                    BPatch_function *calledFunc = proc()->
                        findFunctionByEntry(returningCallB.second);
                    parseAfterCallAndInstrument( callPoints[0], calledFunc, true );
                }
                else {
                    parseAfterCallAndInstrument( callPoints[0], 
                                                 point->getFunction(),
                                                 true);
                }
            }
        }

        // 3.2.2 no call blocks, parse the return addr as a new function
        else {
            if ( point->getFunction()->getModule()->isExploratoryModeOn() ) {
                // otherwise we've instrumented a function in trusted library
                // because we want to catch its callbacks into our code, but in
                // the process are catching calls into other modules
                mal_printf("hybridCallbacks.C[%d] Observed abuse of normal return "
                        "instruction semantics for insn at %p target %lx\n",
                        __LINE__, point->getAddress(), returnAddr);
            }
            analyzeNewFunction( point, returnAddr, true , true );

            // there are no call blocks, so we don't have any post-call pads to patch
        }

        // 3. return
        return;
    }

    // 4. else case: the point is a jump/branch 
    proc()->beginInsertionSet();
    // 4.1 if the point is a direct branch, remove any instrumentation
    if (!point->isDynamic()) {
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

    bool newParsing;
    vector<BPatch_function*> targFuncs;
    proc()->findFunctionsByAddr(target, targFuncs);
    if ( 0 == targFuncs.size() ) { 
        newParsing = true;
        mal_printf("stopThread instrumentation found jump "
                "at 0x%p leading to an unparsed target at 0x%lx\n",
                point->getAddress(), target);
    } else {
        newParsing = false;
        mal_printf("stopThread instrumentation added an edge for jump "
                " at 0x%p leading to a previously parsed target at 0x%lx\n",
                point->getAddress(), target);
    }

    // add the new edge to the program, parseNewEdgeInFunction will figure
    // out whether to extend the current function or parse as a new one. 
    if (targMod != point->getFunction()->getModule()) {
        // Don't put in inter-module branches
        if (newParsing)
            analyzeNewFunction(point, target, true, false);
    }
    else
    {
        parseNewEdgeInFunction(point, target, false);
    }

    if (0 == targFuncs.size()) {
        proc()->findFunctionsByAddr( target, targFuncs );
    }

    // manipulate init_retstatus so that we will instrument the function's 
    // return addresses, since this jump might be a tail call
    for (unsigned tidx=0; tidx < targFuncs.size(); tidx++) {
        parse_func *imgfunc = targFuncs[tidx]->lowlevel_func()->ifunc();
        FuncReturnStatus initStatus = imgfunc->init_retstatus();
        if (ParseAPI::RETURN == initStatus) {
            imgfunc->setinit_retstatus(ParseAPI::UNKNOWN);
            removeInstrumentation(targFuncs[tidx],false,false);
            instrumentFunction(targFuncs[tidx],false,true);
        } 
    }

    // re-instrument the function or the whole module, as needed
    if (newParsing) {
        instrumentModules(false);
    }
    proc()->finalizeInsertionSet(false);
} // end badTransferCB
