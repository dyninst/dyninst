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
#include "BPatch_process.h"
#include "BPatch_function.h"
#include "BPatch_basicBlock.h"
#include "BPatch_flowGraph.h"
#include "BPatch_edge.h"
#include "BPatch_module.h"
#include "dynProcess.h"
#include "BPatch_point.h"
#include "function.h"
#include "debug.h"
#include "common/h/DynAST.h"
#include "dataflowAPI/h/Absloc.h"
#include "dataflowAPI/h/AbslocInterface.h"
#include "instructionAPI/h/InstructionDecoder.h"
#include "instPoint.h"
#include "mapped_object.h"
#include "mapped_module.h"

using namespace Dyninst;

static void overwriteAnalysis_wrapper(BPatch_point *point, void *calc)
{
    dynamic_cast<BPatch_process*>(point->getFunction()->getProc())->
        getHybridAnalysis()->hybridOW()->overwriteAnalysis(point,calc); 
}


static void overwriteSignalCB_wrapper(BPatch_point *viol_instruc, Address viol_target) 
{ 
    dynamic_cast<BPatch_process*>(viol_instruc->getFunction()->getProc())->
        getHybridAnalysis()->hybridOW()->overwriteSignalCB
            ((Address)viol_instruc->getAddress(),viol_target); 
}
InternalCodeOverwriteCallback HybridAnalysisOW::getCodeOverwriteCB()
{ 
    return overwriteSignalCB_wrapper; 
}


int HybridAnalysisOW::owLoop::IDcounter_ = 0;

HybridAnalysisOW::owLoop::owLoop(HybridAnalysisOW *hybridow, 
                                 Address writeTarg)
{
    hybridow_ = hybridow;
    writeTarget_ = writeTarg;
    activeStatus_ = true;
    writesOwnPage_ = false;
    realLoop_ = true;
    loopID_ = owLoop::getNextLoopId();
    hybridow_->idToLoop[loopID_] = this;
    hybridow_->loops.insert(this);
}

HybridAnalysisOW::owLoop::~owLoop()
{
    hybridow_->idToLoop.erase(loopID_);
    hybridow_->loops.erase(this);
    std::map<Dyninst::Address,unsigned char *>::iterator sIter = shadowMap.begin();
    for(; sIter != shadowMap.end(); sIter++) {
        if ((*sIter).second) {
            ::free((*sIter).second);
        }
    }
}

// returns true if any of the modfuncs contains loop code
bool HybridAnalysisOW::codeChangeCB
     (std::vector<BPatch_function*> &modfuncs)
{
    for (std::map<int, owLoop*>::iterator lIter= idToLoop.begin();
         lIter != idToLoop.end();
         lIter++) 
    {
        for (std::set<BPatch_basicBlock*,HybridAnalysis::blockcmp>::iterator 
                biter = ((*lIter).second)->blocks.begin(); 
             biter != ((*lIter).second)->blocks.end(); 
             biter++) 
        {
            vector<ParseAPI::Function *> funcs;
            (*biter)->lowlevel_block()->llb()->getFuncs(funcs);
            vector<ParseAPI::Function*>::iterator fiter = funcs.begin();
            for ( ; fiter != funcs.end(); fiter++) {
                Address fAddr = proc()->lowlevel_process()->
                   findFunction(dynamic_cast<parse_func*>(*fiter))->
                   addr();
                std::vector<BPatch_function*>::iterator bfiter=modfuncs.begin();
                for (; bfiter != modfuncs.end(); bfiter++) {
                    if (fAddr == (Address)(*bfiter)->getBaseAddr()) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

HybridAnalysisOW::HybridAnalysisOW(HybridAnalysis *hybrid)
   : bpatchBeginCB(NULL), bpatchEndCB(NULL)
{
    hybrid_ = hybrid;
}

HybridAnalysisOW::~HybridAnalysisOW()
{
    std::set<owLoop*>::iterator lIter = loops.begin();
    for (; lIter != loops.end(); lIter++) {
        delete *lIter;
    }
}

void HybridAnalysisOW::owLoop::setWriteTarget(Address targ)
{
    writeTarget_ = targ;
}

void HybridAnalysisOW::owLoop::setWritesOwnPage(bool wop)
{
    writesOwnPage_ = wop;
}
void HybridAnalysisOW::owLoop::setActive(bool act)
{
    activeStatus_ = act;
}


// return value of 0 means not in loop
HybridAnalysisOW::owLoop *HybridAnalysisOW::findLoop(Address blockStart)
{
    if (blockToLoop.find(blockStart) != blockToLoop.end()) {
		std::map<int, owLoop*>::iterator iter = idToLoop.find(blockToLoop[blockStart]);
		if (iter == idToLoop.end()) {
	        // because of blocks being overwritten, sometimes we can't tear blocks
		    // out because the internal blocks have been purged and we can't figure
			// out the block address.  Eventually, if the block is reconstituted 
			// we may be able to find it here with a reference to a defunct loop,
			// make sure that this is not the case, if it is, tear the block out
            blockToLoop.erase(blockStart);
            return NULL;
        }
		return iter->second;
    }
    return NULL;
}

bool HybridAnalysisOW::isInLoop(Address blockAddr, bool activeOnly)
{
    owLoop *loop = findLoop(blockAddr);
    if ( !loop ) {
        return false;
    }
    if (activeOnly && loop->isActive()) {
        return false;
    }
    return true;
}

/* 1. Check for changes to the underlying code to see if this is safe to do
 * 2. If the loop is active, check for changes to the underlying data, and 
 *    if no changes have occurred, we can just remove the loop instrumentation
 *    and everything will be hunky dory once we re-instate the write 
 *    protections for the loop's pages
 * return true if the loop was active
 */ 
bool HybridAnalysisOW::deleteLoop(owLoop *loop, 
                                  bool useInsertionSet, 
                                  BPatch_point *writePoint,
                                  bool uninstrument)
{
   mal_printf("deleteLoop: loopID =%d active=%d %s[%d]\n",
              loop->getID(),loop->isActive(),FILE__,__LINE__);

   bool ret = removeLoop(loop, useInsertionSet, writePoint, uninstrument);
   // anything added after this call must also be added to overwriteAnalysis's
   // handling of the "overwroteLoop==true" case. 

   delete loop; 
   return ret;
}


/* 1. Check for changes to the underlying code to see if this is safe to do
 * 2. If the loop is active, check for changes to the underlying data, and 
 *    if no changes have occurred, we can just remove the loop instrumentation
 *    and everything will be hunky dory once we re-instate the write 
 *    protections for the loop's pages
 * return true if the loop was active
 */ 
bool HybridAnalysisOW::removeLoop(owLoop *loop, 
                                  bool useInsertionSet, 
                                  BPatch_point *writePoint,
                                  bool uninstrument)
{
    bool isLoopActive = loop->isActive();

    if (isLoopActive) {
        // make sure the underlying code hasn't changed
        bool changedPages = false;
        bool changedCode = false;
        std::vector<pair<Address,int> > deadBlocks;
        std::vector<BPatch_function*> modFuncs;
        if (writePoint) {
   			cerr << "Calling overwriteAnalysis with point @ " << hex << writePoint->getAddress() << dec << endl;
            overwriteAnalysis(writePoint,(void*)(intptr_t)loop->getID());
        } else {
            std::set<BPatch_function *> funcsToInstrument;
            proc()->overwriteAnalysisUpdate(loop->shadowMap,
                                            deadBlocks,
                                            modFuncs,
                                            funcsToInstrument,
                                            changedPages,
                                            changedCode);
            assert(!changedCode && "bug, overwrite loops should not contain "
                   "instructions that could trigger analysis update callbacks");
        }
        proc()->protectAnalyzedCode();
    }

    // remove loop instrumentation
    if (uninstrument) {
        if (useInsertionSet) {
            proc()->beginInsertionSet();
        }
        std::set<BPatchSnippetHandle*>::iterator sIter = loop->snippets.begin();
        mal_printf("deleting snippets from loop %d\n",loop->getID());
        for (; sIter != loop->snippets.end(); sIter++) {
            proc()->deleteSnippet(*sIter);
        }
        loop->snippets.clear();
        if (useInsertionSet) {
            proc()->finalizeInsertionSet(false);
        }
    }

    // clear loop from blockToLoop, idToLoop, and loops datastructures
    std::set<BPatch_basicBlock*,HybridAnalysis::blockcmp>::iterator bIter 
	    = loop->blocks.begin();
    for (; bIter != loop->blocks.end(); bIter++) {
        if ( !(*bIter)->lowlevel_block() ) {
            mal_printf("WARNING: Can't remove overwritten loop block, the "
                       "internal block was deleted so we can't ascertain its "
                       "address: loopID %d block %p %s[%d]\n", 
                       loop->getID(), (void*)*bIter,FILE__,__LINE__);
        } else {
            blockToLoop.erase((*bIter)->getStartAddress());
        }
    }
    assert(idToLoop.end() != idToLoop.find(loop->getID()));
    idToLoop.erase(idToLoop.find(loop->getID()));
    loops.erase(loop);

    return isLoopActive;
}


bool HybridAnalysisOW::hasLoopInstrumentation
    (bool activeOnly, BPatch_function &func, std::set<owLoop*> *loops_)
{
    //_ASSERTE(_CrtCheckMemory());
    // NEED TO BE CAREFUL BECAUSE WHEN WE OVERWRITE A BLOCK WE DON'T INVALIDATE
    // THE FLOWGRAPH, BUT FROM THE INT-LAYER ON DOWN THINGS ARE INVALIDATED.
    // ANOTHER PROBLEM IS THAT WE MAY BE KEEPING THE BLOCK AROUND AS A PART 
    // OF A BLOCK LOOP
    bool foundLoop = false;

    // get function's blocks
    std::set<BPatch_basicBlock*> blocks;
    BPatch_flowGraph *cfg = func.getCFG();
    if (!cfg) {
        assert(0);
        return false;
    }
    cfg->getAllBasicBlocks(blocks);

    // find loops matching the function's blocks
    std::set<BPatch_basicBlock*>::iterator bIter = blocks.begin();
    for (; bIter != blocks.end(); bIter++) 
    {
        owLoop *loop = findLoop((*bIter)->getStartAddress());
        if (loop && (loop->isActive() || !activeOnly) ) {
            foundLoop = true;
            if (loops_) {
                loops_->insert(loop);
            }
        }
    }
    return foundLoop;
}


/* 1. Gather up all instrumentation sites that need to be monitored:
   1a. The edges of all instrumented blocks that leave the block set
   1b. Unresolved points in instrumented blocks
   2. Instrument exit edges and unresolved points with callbacks to 
      the analysis update routine
   2a.Instrument at loop exit edges
   2b.Instrument at unresolved edges in the loop 
 */
void HybridAnalysisOW::owLoop::instrumentOverwriteLoop(Address writeInsn)
{
    assert(blocks.size());
    writeInsns.insert(writeInsn);

    // call user callback before instrumenting, 
    // convert loop's block set into a vector
    std::vector<BPatch_basicBlock*> loopBlocks;
    std::set<BPatch_basicBlock*,HybridAnalysis::blockcmp>::iterator bIter;
    for (bIter = blocks.begin(); bIter != blocks.end(); bIter++) {
        loopBlocks.push_back(*bIter);
    }
    hybridow_->bpatchBeginCB(loopBlocks);

    // 1. Gather up all instrumentation sites that need to be monitored:
    std::set<BPatch_edge*> instEdges;
    vector<BPatch_edge*> outEdges;
    bIter = blocks.begin();
    while (bIter != blocks.end()) {
        mal_printf("loop block [%lx %lx]\n", (*bIter)->getStartAddress(),
               (*bIter)->getEndAddress());
        // 1a. The edges of all instrumented blocks that leave the block set
        (*bIter)->getOutgoingEdges(outEdges);
        for (unsigned eIdx=0; eIdx < outEdges.size(); eIdx++) {
            if ( blocks.end() == 
                 blocks.find(outEdges[eIdx]->getTarget()) ) 
            {
                mal_printf("instrumenting loop out edge %lx -> %lx\n", 
                       (*bIter)->getLastInsnAddress(),
                       outEdges[eIdx]->getTarget()->getStartAddress());
                instEdges.insert(outEdges[eIdx]);

                BPatch_basicBlock *src = outEdges[eIdx]->getSource();
                if (src->lowlevel_block()->isShared()) {
                   vector<func_instance*> funcs;
                   src->lowlevel_block()->getFuncs(std::back_inserter(funcs));
                   for (unsigned int fidx=0; fidx < outEdges.size(); fidx++) {
                      if (funcs[fidx] != src->getFlowGraph()->getFunction()->lowlevel_func()) {
                         BPatch_function *bpf = hybridow_->proc()->
                            findOrCreateBPFunc(funcs[fidx],src->getFlowGraph()->getModule());

                         BPatch_basicBlock *shared = bpf->getCFG()->findBlockByAddr(src->getStartAddress());
                         vector<BPatch_edge*> sharedOutEdges;
                         shared->getOutgoingEdges(sharedOutEdges);
                         for (unsigned sIdx=0; sIdx < sharedOutEdges.size(); sIdx++) {
                            if (sharedOutEdges[sIdx]->getTarget()->getStartAddress() 
                                == outEdges[eIdx]->getTarget()->getStartAddress()) 
                            {
                               instEdges.insert(sharedOutEdges[sIdx]);
                            }
                         }
                      }
                   }
                }
            }
        }
        outEdges.clear();
        bIter++;
    }

    // 2. Instrument exit edges and unresolved points with callbacks to 
    //    the analysis update routine
    BPatch_stopThreadExpr stopForAnalysis(overwriteAnalysis_wrapper, 
                                          BPatch_constExpr(getID()),
                                          false,
                                          BPatch_noInterp);
    // 2a.Instrument at loop exit edges
    std::set<BPatch_edge*>::iterator eIter = instEdges.begin();
    BPatchSnippetHandle *snippetHandle = NULL;
    while (eIter != instEdges.end()) {
        BPatch_point *edgePoint = (*eIter)->getPoint();
        mal_printf(" instr edge: 0x%lx -> 0x%lx\n", 
            (*eIter)->getSource()->getLastInsnAddress(),
            (*eIter)->getTarget()->getStartAddress());
        snippetHandle = hybridow_->proc()->insertSnippet
            (stopForAnalysis, *edgePoint, BPatch_firstSnippet);
		assert(snippetHandle);
        snippets.insert(snippetHandle);
        eIter++;
    }
    // 2b.Instrument at unresolved edges in the loop
    BPatch_dynamicTargetExpr dynTarg;
    std::set<BPatch_point*>::iterator uIter = unresExits_.begin();
    while (uIter != unresExits_.end()) {
        Address uAddr = (Address)(*uIter)->getAddress();
        mal_printf(" instr unresolved: 0x%p in func at 0x%p\n", 
                  (void*)uAddr, (*uIter)->getFunction()->getBaseAddr());
        if ((*uIter)->isDynamic()) {
            long st = 0;
            vector<Address> targs;
            if (hybridow_->hybrid_->getCallAndBranchTargets
                ((*uIter)->llpoint()->block(), targs)) 
            {
                st = * targs.begin();
            }
            BPatch_constExpr stSnip(st);
            BPatch_ifExpr stopIfNewTarg
                ( BPatch_boolExpr(BPatch_ne, dynTarg, stSnip),
                  stopForAnalysis );
            snippetHandle = hybridow_->proc()->insertSnippet
                (stopIfNewTarg, *(*uIter), BPatch_firstSnippet);
        } else {
            snippetHandle = hybridow_->proc()->insertSnippet
                (stopForAnalysis, *(*uIter), BPatch_firstSnippet);
        }
		assert(snippetHandle);
        snippets.insert(snippetHandle);
        uIter++;
    }
}


void HybridAnalysisOW::owLoop::instrumentOneWrite(Address writeInsnAddr, 
                                                  vector<BPatch_function*> writeFuncs)
{
    realLoop_ = false;
    writeInsns.insert(writeInsnAddr);

    // create the stopthread expression
    BPatch_stopThreadExpr stopForAnalysis
            (overwriteAnalysis_wrapper, 
             BPatch_constExpr(loopID_), 
             false,BPatch_noInterp);

    for (unsigned fidx =0; fidx < writeFuncs.size(); fidx++) 
    {
       // We can afford to be really slow and precise in the lookup, as this is the
       // very, very, very uncommon case.
       block_instance *block = writeFuncs[fidx]->lowlevel_func()->obj()->findOneBlockByAddr(writeInsnAddr);
       if (!block) continue;
       instPoint *ip = instPoint::postInsn(writeFuncs[fidx]->lowlevel_func(),
                                           block,
                                           writeInsnAddr);
       BPatch_point *writePoint = hybridow_->proc()->findOrCreateBPPoint(writeFuncs[fidx],
                                                                         ip, 
                                                                         BPatch_locInstruction);

        // instrument and store the snippet handle
        BPatchSnippetHandle *snippetHandle = hybridow_->proc()->insertSnippet
            (stopForAnalysis, *writePoint, BPatch_callAfter);
        assert(snippetHandle);
        snippets.insert(snippetHandle);
    }
}



/*1. initialize necessary variables
 * get loop blocks
 * build the set that describes the type of accesses we're looking for
2. create bounds array for all blocks in the loop
 * for each block
     * create instrumentation points
     * store block bounds
 * create and initialize the snippet for the array of bounds
3. create the bounds check function call snippet
   checkBounds(boundsArray, arrayLen, target)
 * arg0: bounds array
 * arg1: array size
 * arg2: effective address of write instruction
 * arg3: call
 * find DYNINST_checkBounds function
 * create the conditional expression based on bounds check's return value
4. instrument each write point
     * create the stopthread expression
     * create the if expression
     * insert the snippet 
*/
void HybridAnalysisOW::owLoop::instrumentLoopWritesWithBoundsCheck()
{
    assert(!blocks.empty());

    // build the set that describes the type of accesses we're looking for
    std::set<BPatch_opCode> insnTypes;
    insnTypes.insert(BPatch_opStore);

    // 2. create bounds array for all blocks in the loop
    int array_length = 2 * blocks.size(); // entries (not bytes) in array
    Address *boundsArray = new Address[array_length]; 

    // for each block
    std::vector<BPatch_point*> loopWrites;
    std::set<BPatch_basicBlock*,HybridAnalysis::blockcmp>::iterator bIter = blocks.begin();
    unsigned boundsIdx=0; 
    while (bIter != blocks.end()) {
        // create instrumentation points
        Address blockAddr = (*bIter)->getStartAddress();
        ParseAPI::Block *blk = (*bIter)->lowlevel_block()->llb();
        Address base = blockAddr - blk->start();
        assert(blockAddr == base+blk->start() && 
               (*bIter)->getEndAddress() == base+blk->end());
        std::vector<BPatch_point*>* blockWrites = (*bIter)->findPoint(insnTypes);
        for (unsigned widx = 0; widx < (*blockWrites).size(); widx++) {
            if (BPatch_locSubroutine == (*blockWrites)[widx]->getPointType() ||
                !hybridow_->isRealStore(
                    (Address)(*blockWrites)[widx]->getAddress(), 
                    (*bIter)->lowlevel_block(),
                    (*blockWrites)[widx]->getFunction())) 
            {
                (*blockWrites)[widx] = (*blockWrites)[blockWrites->size()-1];
                blockWrites->pop_back();
                widx--; // there's an unexamined point at this vector location
            }
        }
        loopWrites.insert(loopWrites.end(), 
                          blockWrites->begin(), 
                          blockWrites->end());
        // store block bounds or alter previous entry if blocks are contiguous
        if (boundsIdx > 0 && boundsArray[boundsIdx-1] == (*bIter)->getStartAddress()) {
            boundsArray[boundsIdx-1] = (*bIter)->getEndAddress();
        } else {
            if (boundsIdx > 0) {
                mal_printf("BA[%u] = [%lx %lx]\n", boundsIdx-2, 
                           boundsArray[boundsIdx-2],boundsArray[boundsIdx-1]);
            }
            boundsArray[boundsIdx]   = (*bIter)->getStartAddress();
            boundsArray[boundsIdx+1] = (*bIter)->getEndAddress();
            boundsIdx+=2;
        }
        bIter++; 
    }
    if (boundsIdx > 0) {
        mal_printf("BA[%u] = [%lx %lx]\n", 
                   boundsIdx-2, boundsArray[boundsIdx-2], boundsArray[boundsIdx-1]);
    }
    mal_printf("instrumenting %lu store instructions in loop %d %s[%d]\n",
               loopWrites.size(),loopID_,FILE__,__LINE__);
    // array length is now probably smaller than earlier worst case value
    array_length = boundsIdx;
    // create and initialize the snippet for the array of bounds
    BPatch_variableExpr *varBA = hybridow_->proc()->malloc
        (array_length *sizeof(Address));
    varBA->writeValue(boundsArray, array_length * sizeof(Address), false);

    //3. create the bounds check function call snippet
    //   checkBounds(boundsArray, arrayLen, target)
    BPatch_Vector<BPatch_snippet*> args;
    // arg0: bounds array
    BPatch_constExpr varArrayBase(varBA->getBaseAddr());
    args.push_back(&varArrayBase);
    // arg1: array length
    BPatch_constExpr varAS(array_length);
    args.push_back(&varAS);
    // arg2: effective address of write instruction
    BPatch_effectiveAddressExpr eae;
    args.push_back(&eae);

    // create function call snippet to DYNINST_checkBounds 
    vector<BPatch_function *> funcs;
    hybridow_->hybrid()->getRuntimeLib()->findFunction
        ("DYNINST_boundsCheck", funcs);
    BPatch_function *boundsCheckFunc = funcs[0];
    BPatch_funcCallExpr bcCall(*boundsCheckFunc, args);

    // create the conditional expression based on bounds check's return value
    BPatch_boolExpr condition(BPatch_ne, bcCall, BPatch_constExpr(0));

    // create the conditional expression based on bounds check's return value
    BPatch_constExpr ce_lowAddr(boundsArray[0]);
    BPatch_constExpr ce_highAddr(boundsArray[array_length-1]);
    BPatch_boolExpr cond_lb(BPatch_le, ce_lowAddr, eae);
    BPatch_boolExpr cond_hb(BPatch_lt, eae, ce_highAddr);
    BPatch_boolExpr cond_bounds(BPatch_and, cond_lb, cond_hb);

    //4. instrument each write point
    for (unsigned wIdx=0; wIdx < loopWrites.size(); wIdx++) {
        // create the stopthread expression
        BPatch_stopThreadExpr stopForAnalysis
            (overwriteAnalysis_wrapper, 
             BPatch_constExpr(-1*loopID_), 
             false,BPatch_noInterp);
        mal_printf("BoundsCheck Call at %p\n",loopWrites[wIdx]->getAddress());
        // create the if expression
        BPatch_ifExpr ifBoundsThenStop(condition, stopForAnalysis);
        // insert the snippet 
        BPatchSnippetHandle *handle = hybridow_->proc()->insertSnippet
            (ifBoundsThenStop, *loopWrites[wIdx], BPatch_callAfter);
        snippets.insert(handle);
    }
}
// Returns a loop if all callers to the writeAddr function are in a single 
// function and there is a loop that contains them all.
// (otherwise we would have to do a stackwalk and that's too expensive)
// If more than one loop satisfies this criteria, choose the largest one
BPatch_basicBlockLoop* HybridAnalysisOW::getParentLoop(BPatch_function &func, Address writeAddr)
{
    set<BPatch_function*> callFs;
    vector<BPatch_point*> callPs;
    func.getCallerPoints(callPs);
    for (vector<BPatch_point*>::iterator pit = callPs.begin(); pit != callPs.end(); pit++) {
        callFs.insert((*pit)->getFunction());
    }
    if (callFs.empty()) {
        return NULL;
    }
    BPatch_function *callF = NULL;
    if (callFs.size() > 1) {
        if (writeHits.end() == writeHits.find(writeAddr)) {
            writeHits[writeAddr] = 0;
        }
        writeHits[writeAddr] += 1;
        if (writeHits[writeAddr] >= HIT_THRESHOLD) {
            vector<vector<Frame> > stacks;
            proc()->lowlevel_process()->walkStacks(stacks);
            unsigned writeStackIdx = stacks.size();
            for (unsigned sidx=0; sidx < stacks.size(); sidx++) {
                if (stacks[sidx].front().getFunc() == func.lowlevel_func()) {
                    writeStackIdx = sidx;
                    break;
                }
            }
            if (writeStackIdx < stacks.size()) {
                // pick a loop to return, we choose the deepest function with
                // a loop that calls the writeFunction directly, or if there
                // is none, the first loop we find in any function 
                vector<BPatch_basicBlockLoop*> callWriteLoops;
                BPatch_basicBlockLoop* firstNoCallLoop = NULL;
                for (unsigned fidx=1; fidx < stacks[writeStackIdx].size(); fidx++) {
                    func_instance *curIntF = stacks[writeStackIdx][fidx].getFunc();
                    if (!curIntF) {
                        break; // we haven't parsed at the function's call
                               // fallthrough address, if there is a loop here
                               // we haven't parsed it yet, later on we may 
                               // complete the parse, discovering the loop
                    }
                    BPatch_function *curF = proc()->
                        findOrCreateBPFunc(curIntF, NULL);
                    // translate PC to unrelocated version
                    Address origPC = stacks[writeStackIdx][fidx].getPC();
                    vector<func_instance*> tmp1;
                    baseTramp *tmp2;
                    proc()->lowlevel_process()->getAddrInfo
                        (stacks[writeStackIdx][fidx].getPC(),origPC,tmp1,tmp2);
                    // get loop
                    BPatch_basicBlockLoop *curL = getWriteLoop
                        (*curF, origPC, false);
                    if (curL) {
                        // curF calls the write loop directly
                        if (callFs.end() != callFs.find(curF)) {
                            callWriteLoops.push_back(curL);
                        }
                        else if (NULL == firstNoCallLoop) {
                            firstNoCallLoop = curL;
                        }
                    }
                }
                if (!callWriteLoops.empty()) {
                    return callWriteLoops.back();
                } else if (firstNoCallLoop) {
                    return firstNoCallLoop;
                }
            }
        }
        return NULL;
    } 
    else {
        callF = * callFs.begin();
        map<int,BPatch_basicBlockLoop*> goodLoops;
        for (vector<BPatch_point*>::iterator pit = callPs.begin(); pit != callPs.end(); pit++) {
            BPatch_basicBlockLoop *curL = getWriteLoop(*callF, (Address)(*pit)->getAddress(), false);
            if (curL) {
                bool containsAllCalls = true;
                for (vector<BPatch_point*>::iterator pit2 = callPs.begin(); 
                     pit2 != callPs.end(); pit2++) 
                {
                    if ( ! curL->containsAddressInclusive((Address)(*pit2)->getAddress()) ) {
                        containsAllCalls = false;
                    }
                }
                if (containsAllCalls) {
                    vector<BPatch_basicBlock*> loopBs;
                    curL->getLoopBasicBlocks(loopBs);
                    goodLoops[loopBs.size()] = curL;
                }
            }
        }
        if (goodLoops.empty()) {
            return NULL;
        }
        return goodLoops.rbegin()->second;
    }
}


// gets biggest loop without unresolved/multiply resolved indirect ctrl flow that it can find
BPatch_basicBlockLoop* HybridAnalysisOW::getWriteLoop
(BPatch_function &func, Address writeAddr, bool allowCallerLoop)
{
    BPatch_flowGraph *graph = func.getCFG();
    vector<BPatch_basicBlockLoop*> loops_;
    vector<BPatch_point*> blockPoints;
    vector<BPatch_basicBlock *>loopBlocks;
    graph->getLoops(loops_);
    if (allowCallerLoop && loops_.empty()) {
        BPatch_basicBlockLoop *pLoop = getParentLoop(func,writeAddr);
        if (pLoop) {
            return pLoop;
        }
    }
    vector<BPatch_basicBlockLoop*>::iterator lIter = loops_.begin();
    BPatch_basicBlockLoop *writeLoop = NULL;
    while (lIter != loops_.end()) {

//        mal_printf("found nat'l loop w/ block[0] at 0x%x, back edge at 0%x\n",
//                   (*lIter)->getLoopHead()->getStartAddress(),
//                   (*lIter)->getBackEdge()->getSource()->getLastInsnAddress());

        if ((*lIter)->containsAddressInclusive(writeAddr) && 
            (!writeLoop || writeLoop->hasAncestor(*lIter))) 
        {
            // set writeLoop if the curloop has no indirect or non-returning control transfers
            bool hasUnresolved = false;
            (*lIter)->getLoopBasicBlocks(loopBlocks);
            for (vector<BPatch_basicBlock*>::iterator bIter= loopBlocks.begin(); 
                bIter != loopBlocks.end() && !hasUnresolved; 
                bIter++) 
            {
                Address blockStart = (*bIter)->getStartAddress();
                if (blockToLoop.find(blockStart) != blockToLoop.end()) {
                    // this block corresponds to another instrumented loop!
                    mal_printf("WARNING: block [%lx %lx] in loop overlaps with "
                            "existing loop %d %s[%d]\n", blockStart, 
                            (*bIter)->getEndAddress(), 
                            blockToLoop[blockStart], FILE__,__LINE__);
                }

                (*bIter)->getAllPoints(blockPoints);
                for (vector<BPatch_point*>::iterator pIter= blockPoints.begin(); 
                    pIter != blockPoints.end(); 
                    pIter++) 
                {
                    if ((*pIter)->isDynamic()) {
                        vector<Address> targs;
                        if (!hybrid_->getCallAndBranchTargets((*pIter)->llpoint()->block(),targs)) {
                            mal_printf("loop has an unresolved indirect transfer at %p\n", 
                                    (*pIter)->getAddress());
                            hasUnresolved = true;
                        } else if (targs.size() > 1) {
                            mal_printf("loop has an indirect transfer resolving to multiple targets at %p\n", 
                                    (*pIter)->getAddress());
                            hasUnresolved = true;
                        } 
                    }
                }
                blockPoints.clear();
            }
            loopBlocks.clear();
            
            if (!hasUnresolved) {
                writeLoop = *lIter;
            }
        }
        lIter++;
    }
    if (writeLoop) {
//        mal_printf("CHOSE nat'l loop with: block[0] at 0x%x, back edge at 0%x\n",
//		    writeLoop->getLoopHead()->getStartAddress(),
//		    writeLoop->getBackEdge()->getSource()->getLastInsnAddress());
    }
    return writeLoop;
}

// adds to visitMe if the library is in a non-system library in 
// exploratory or defensive mode
static void addLoopFunc(BPatch_function *func, 
                        set<BPatch_function*> &visited, 
                        set<BPatch_function*> &visitMe)
{   
    if (!func) {
        return;
    }

    // if we've visited the func, return
    if (visited.end() != visited.find(func)) {
        return;
    }

    // add to visited
    visited.insert(func);

    // add to visitMe, if func is in a defensive binary
    if ( BPatch_defensiveMode == func->getModule()->getHybridMode() ) {
        mal_printf("new loop func at=%lx %d\n",
                   (Address)func->getBaseAddr(),__LINE__);
        visitMe.insert(func);
    } 
    else if ( ! func->getModule()->isSystemLib() ) {
        const int nameLen = 32;
        char modName[nameLen];
        func->getModule()->getName(modName,nameLen);
        fprintf(stderr,"ERROR: overwrite loop calls into func at "
                "0x%lx in module %s that is not marked with "
                "malware mode %s[%d]\n",
                (Address)func->getBaseAddr(),
                modName,FILE__,__LINE__);
    }
}


// recursively add all functions that contain calls, 
// return true if the function contains no unresolved control flow
// and the function returns normally
bool HybridAnalysisOW::addFuncBlocks(owLoop *loop, 
                   std::set<BPatch_function*> &addFuncs, 
                   std::set<BPatch_function*> &seenFuncs,
                   std::set<int> &overlappingLoops)
{
    bool hasUnresolved = false;
    std::set<BPatch_function*> nextAddFuncs;
    std::vector<BPatch_point*> unresolvedCF;

    // for each function in addFuncs
    for (set<BPatch_function*>::iterator fIter= addFuncs.begin(); 
         fIter != addFuncs.end(); 
         fIter++) 
    {
        // for each of its blocks
        std::set<BPatch_basicBlock*> fBlocks;
        (*fIter)->getCFG()->getAllBasicBlocks(fBlocks);
        for (std::set<BPatch_basicBlock*>::iterator bIter= fBlocks.begin(); 
            bIter != fBlocks.end(); 
            bIter++) 
        {
            // check to see if the block has a non-returning call
            if ((*bIter)->lowlevel_block()->llb()->isCallBlock() && 
                NULL == (*bIter)->lowlevel_block()->getFallthrough()) 
            {
                mal_printf("loop %d calls func %p which has a non-returning "
                           "call at %lx %s[%d]\n", loop->getID(), 
                           (*fIter)->getBaseAddr(), 
                           (*bIter)->getLastInsnAddress(), FILE__,__LINE__);
                hasUnresolved = true;
            }
            // add the block to the new loop's datastructures
            Address blockStart = (*bIter)->getStartAddress();
            if (blockToLoop.find(blockStart) != blockToLoop.end() && 
                loop->getID() != blockToLoop[blockStart]) 
            {
                mal_printf("block [%lx %lx] in loop %d overlaps with existing loop %d %s[%d]\n",
                        blockStart, (*bIter)->getEndAddress(),
                        loop->getID(), blockToLoop[blockStart],FILE__,__LINE__);
                overlappingLoops.insert(blockToLoop[blockStart]);
            }
            blockToLoop[blockStart] = loop->getID();
            loop->blocks.insert(*bIter);

            // if the block has a call to an unseen function, add it for next iteration
            if ((*bIter)->lowlevel_block()->containsCall()) {
               addLoopFunc((*bIter)->getCallTarget(), seenFuncs, nextAddFuncs);
            }
        }
        // if func contains ambiguously resolved control flow, or truly 
        // unresolved ctrl flow, then set flag and clear vector
        (*fIter)->getUnresolvedControlTransfers(unresolvedCF);
        for (vector<BPatch_point*>::iterator pIter = unresolvedCF.begin(); 
             !hasUnresolved && pIter != unresolvedCF.end(); 
             pIter++) 
        {
            vector<Address> targs;
            hybrid_->getCallAndBranchTargets((*pIter)->llpoint()->block(), targs);
            if (1 != targs.size()) {
                loop->unresExits_.insert(*pIter);
                hasUnresolved = true;
                mal_printf("loop %d calls func %p which has an indirect "
                           "transfer at %p that resolves to %lu targets "
                           "%s[%d]\n", loop->getID(), (*fIter)->getBaseAddr(), 
                           (*pIter)->getAddress(), targs.size(), FILE__,__LINE__);
            } else {
                // add target function if it's not in seenFuncs
                addLoopFunc(proc()->findFunctionByEntry(targs[0]), seenFuncs, nextAddFuncs);
            }
        }
        if (unresolvedCF.size()) {
            unresolvedCF.clear();
        }
        // also want to make sure this function is returning
        if ( ParseAPI::RETURN != 
             (*fIter)->lowlevel_func()->ifunc()->retstatus() ) 
        {
            mal_printf("loop %d calls func %p which could be "
                       "non-returning %s[%d]\n", loop->getID(), 
                       (*fIter)->getBaseAddr(), FILE__,__LINE__);
            hasUnresolved = true;
        }
    }
    // if these functions called additional functions
    if (!hasUnresolved && nextAddFuncs.size()) {
        if (false == addFuncBlocks(loop, 
                                   nextAddFuncs, 
                                   seenFuncs, 
                                   overlappingLoops)) 
        {
            hasUnresolved = true;
        }
    }
    return ! hasUnresolved;
}

// if writeLoop is null, return the whole function in the loop. 
// returns true if we were able to identify all code in the loop
// 
// the checks on the basicBlockLoop's immediate blocks are not strictly 
// necessary as they should have been checked by getWriteLoop
// 
// considering the loop save to instrument if its indirect control transfers
// have so far always been resolved to a single control flow target
//
// Does not set the block->loop map for overlapping blocks
bool HybridAnalysisOW::setLoopBlocks(owLoop *loop, 
                                     BPatch_basicBlockLoop *writeLoop,
                                     std::set<int> &overlappingLoops)
{
    bool hasUnresolvedCF = false;
    vector<BPatch_point*> blockPoints;
    std::set<BPatch_function*> loopFuncs;// functions called by loop insns
    vector<BPatch_basicBlock *>loopBlocks;
    writeLoop->getLoopBasicBlocks(loopBlocks);
    // for each block in the loop
    for (vector<BPatch_basicBlock*>::iterator bIter= loopBlocks.begin(); 
         bIter != loopBlocks.end(); 
         bIter++) 
    {
        // add the block to the loop's block-list datastructure
        Address blockStart = (*bIter)->getStartAddress();
        if (blockToLoop.find(blockStart) != blockToLoop.end()) {
            // deal w/ overlapping blocks in the caller
            // this block corresponds to another instrumented loop!
            fprintf(stderr,"WARNING: block [%lx %lx] in loop %d overlaps with "
                    "existing loop %d %s[%d]\n", blockStart, 
                    (*bIter)->getEndAddress(), loop->getID(), 
                    blockToLoop[blockStart], FILE__,__LINE__);
            overlappingLoops.insert(blockToLoop[blockStart]);
        }
        blockToLoop[(*bIter)->getStartAddress()] = loop->getID();
        loop->blocks.insert(*bIter);

        // if the block has an indirect control transfer
        (*bIter)->getAllPoints(blockPoints);
        for (vector<BPatch_point*>::iterator pIter= blockPoints.begin(); 
            pIter != blockPoints.end(); 
            pIter++) 
        {
            if ((*pIter)->isDynamic()) {
                // We've already checked that the transfer is uniquely 
                // resolved, add the target.
                vector<Address> targs;
                hybrid_->getCallAndBranchTargets((*pIter)->llpoint()->block(), targs);
                assert(targs.size() == 1); 
                mal_printf("loop %d has a resolved indirect transfer at %p with "
                          "target %lx\n", loop->getID(), (*pIter)->getAddress(), 
                          *targs.begin());

                vector<BPatch_function *> targFuncs;
                proc()->findFunctionsByAddr(*targs.begin(),targFuncs);
                if (!targFuncs.empty() && 
                    targFuncs[0]->getModule()->isExploratoryModeOn()) 
                {
                    loopFuncs.insert(targFuncs.begin(),targFuncs.end());
                } else if (!targFuncs.empty()) {
                    mal_printf("loop contains call to non-mal-func:%p %d\n",
                               targFuncs[0]->getBaseAddr(), __LINE__);
                }
            }
        }
        blockPoints.clear();

        // if the block has a call, add it to the list of called funcs
        BPatch_function *targFunc = NULL;
        if ((*bIter)->lowlevel_block()->containsCall()) {
           targFunc = (*bIter)->getCallTarget();
        }
        if (targFunc && 
            (*bIter)->getFlowGraph()->getModule() == targFunc->getModule()) 
        { 
            mal_printf("loop has a function call %lx->%p\n", 
                      (*bIter)->getLastInsnAddress(), targFunc->getBaseAddr());
            loopFuncs.insert(targFunc);

        } else if (targFunc) {
            mal_printf("loop calls func at %p in different module [%d]\n",
                       targFunc->getBaseAddr(), __LINE__);
        }
    }
    //recursively add blocks in called functions to the loop
    if (!loopFuncs.empty()) {
        std::set<BPatch_function*> loopFuncCopy(loopFuncs);
        if (false == addFuncBlocks(loop, 
                                   loopFuncs, 
                                   loopFuncCopy, 
                                   overlappingLoops)) 
        {
            hasUnresolvedCF = true;
        }
    }

    // if the loop has bad ctrl flow, clear up the loop-block datastructures
    if (hasUnresolvedCF == true) {
        std::set<BPatch_basicBlock*,HybridAnalysis::blockcmp>::iterator bIter 
            = loop->blocks.begin();
        while (bIter != loop->blocks.end()) {
            blockToLoop.erase((*bIter)->getStartAddress());
            bIter++;
        }
        loop->blocks.clear();
    }

    return ! hasUnresolvedCF;
}

#if !defined(os_windows)

void HybridAnalysisOW::makeShadow_setRights(Address , owLoop *) {}
void HybridAnalysisOW::overwriteAnalysis(BPatch_point *, void *) {}
bool HybridAnalysisOW::removeOverlappingLoops
(owLoop *, std::set<int> &) { return false; }

#else 


// return true if the loop's blocks are a superset of the loop(s) it
// overlaps with
bool HybridAnalysisOW::removeOverlappingLoops(owLoop *loop,
                                              std::set<int> &overlappingLoops)
{
    bool isSuperset = true;

    // iterate to ensure that overlappingLoops are contained in this loop
    std::set<int>::iterator lIter = overlappingLoops.begin();
    std::set<int> removeLoops;
    for (; lIter != overlappingLoops.end(); lIter++) 
    {
        // get blocks for other loops
        owLoop *otherLoop = idToLoop[*lIter];
        if (otherLoop->blocks.empty()) { // if cleanup is only partially done
            deleteLoop(otherLoop,false);
            removeLoops.insert(*lIter);
        } else {// iterate through otherLoop's blocks to ensure they're a subset
            std::set<BPatch_basicBlock*,HybridAnalysis::blockcmp>::iterator 
                obIter = otherLoop->blocks.begin();
            for (; obIter != otherLoop->blocks.end(); obIter++) {
                if (loop->blocks.end() == loop->blocks.find(*obIter))
                {
                    fprintf(stderr,"WARNING: write loop %d is not a subset of "
                            "current loop, block [%lx %lx] not in curloop "
                            "%s[%d]\n", *lIter, (*obIter)->getStartAddress(), 
                            (*obIter)->getEndAddress(), FILE__,__LINE__);
                    isSuperset = false;
                }
            }
        }
    }

    // remove from overlappingLoops the loops that are overlapping
    for (lIter = removeLoops.begin(); lIter != removeLoops.end(); lIter++) {
        overlappingLoops.erase( overlappingLoops.find(*lIter) );
    }

    // if the overlapping loops are contained in this loop
    if (isSuperset) {
        // remove instrumentation for other loops
        for (lIter = overlappingLoops.begin(); 
             lIter != overlappingLoops.end(); 
             lIter++) 
        {
            deleteLoop(idToLoop[*lIter],false); //CAREFUL! removes write perm's from loop pages
        }
        // restore write permissions to pages that are currently shadowed
        if (!loop->shadowMap.empty()) {
            Dyninst::ProcControlAPI::Process::mem_perm rights(true, true, true);
            std::map<Address,unsigned char*>::iterator siter = loop->shadowMap.begin();
            for (; siter != loop->shadowMap.end(); siter++) {
    	        proc()->setMemoryAccessRights(siter->first, 
                    proc()->lowlevel_process()->getMemoryPageSize(), 
                    rights /*PAGE_EXECUTE_READWRITE*/);
                mal_printf("revbug restoring write to %lx %s[%d]\n",(*siter).first, 
                        FILE__,__LINE__);
            }
        }
        // add all blocks to block -> loop map (overlap blocks won't be)
        std::set<BPatch_basicBlock*,HybridAnalysis::blockcmp>::iterator bIter;
        for(bIter = loop->blocks.begin(); bIter != loop->blocks.end(); bIter++) {
            Address blockStart = (*bIter)->getStartAddress();
            if (blockToLoop.end() == blockToLoop.find(blockStart)) {
                blockToLoop[blockStart] = loop->getID();
            } else {
                assert(loop->getID() == blockToLoop[blockStart]);
            }
        }
    }

    return isSuperset;
}

// remove any coverage instrumentation
// make a shadow page, 
// restore write privileges to the page, 
void HybridAnalysisOW::makeShadow_setRights
    (Address pageAddr, // addr on the page, not necessarily the start of the page
     owLoop *loop)
{

    const unsigned int pageSize = proc()->lowlevel_process()->getMemoryPageSize();
    pageAddr = (pageAddr / pageSize ) * pageSize;

    // . Make a shadow copy of the block that is about to be overwritten
    loop->shadowMap[pageAddr] = proc()->makeShadowPage(pageAddr);

    Dyninst::ProcControlAPI::Process::mem_perm rights(true, true, true);
	// Restore write permissions to the written page
    proc()->setMemoryAccessRights(pageAddr, pageSize,
                                  rights /* PAGE_EXECUTE_READWRITE */);
}


/* 
 * 1. Identify overwritten blocks and update the analysis
 * 2. Update instrumentation
 * 3. Remove loop instrumentation
 * 4. Free shadow pages 
 */
void HybridAnalysisOW::overwriteAnalysis(BPatch_point *point, void *loopID_)
{
    Address pointAddr = (Address) point->getAddress();
    mal_printf("\noverwriteAnalysis(trigger=%lx, loopID=%d)\n", 
		      pointAddr,(long)loopID_);

    // setup
    bool changedPages = false;
    bool changedCode = false;
    std::vector<BPatch_function*> owFuncs;
    std::vector<pair<Address,int> > deadBlocks;
    std::vector<BPatch_function*> newFuncs;
    std::vector<BPatch_function*> modFuncs;
    std::set<BPatch_module*> modModules;
    std::set<BPatch_function*> funcsToInstrument;
    const unsigned int pageSize = proc()->lowlevel_process()->getMemoryPageSize();
    Address pageAddress = (((Address)pointAddr) / pageSize) * pageSize;
    bool overwroteLoop = false;
    long loopID = (long)loopID_;

    //if we quit early because we failed a bounds check on a write instruction:
    if (loopID < 0) {
        loopID *= -1;
        overwroteLoop = true;
    }

    // find the loop corresponding to the loopID, and if there is none, it
    // means we tried to delete the instrumentation earlier, but failed
    if (idToLoop.end() == idToLoop.find(loopID)) {
        fprintf(stderr,"executed instrumentation for old loop %d at point %lx "
                  "%s[%d]\n", loopID, pointAddr, FILE__,__LINE__);
        assert(0);
        return; 
    }

    owLoop *loop = idToLoop[loopID]; 
    loop->setActive(false);
    loop->unresExits_.clear();

    if (overwroteLoop) {
        mal_printf("Overwrite loop modified its own code\n");
        for (std::map<Address, unsigned char *>::iterator pIter 
	            = loop->shadowMap.begin();
             pIter != loop->shadowMap.end();
             pIter++) 
        {
            modModules.insert( proc()->findModuleByAddr( (*pIter).first ) );
        }
        removeLoop(loop, true, NULL, true);
    }

/* 1. Identify overwritten blocks and update the analysis */
    // if writeTarget is non-zero, only one byte was overwritten
    proc()->overwriteAnalysisUpdate(loop->shadowMap, 
                                    deadBlocks,
                                    owFuncs, 
                                    funcsToInstrument,
                                    changedPages,
                                    changedCode);

    if (overwroteLoop) {
       delete(loop);
       loop = NULL;
    }

    proc()->beginInsertionSet();

/* 2. if the code changed, remove loop & overwritten instrumentation, 
      then re-instrument */
    for (std::set<BPatch_function *>::iterator iter = funcsToInstrument.begin();
        iter != funcsToInstrument.end(); ++iter)
    {
        hybrid_->instrumentFunction(*iter,
            false, // Already in an insertion set, so don't start another
            false, // Only need unresolved instrumentation, I think.
            false); // And don't add shadow synchronization.
    }

    if (changedCode) {

        // overwrite stats
        hybrid_->stats_.owCount++;
        unsigned long owBytes = 0;
        for (vector<pair<Address,int> >::iterator bit = deadBlocks.begin();
             bit != deadBlocks.end();
             bit++)
        {
            owBytes += bit->second;
        }
        hybrid_->stats_.owBytes = owBytes;

        // build up list of modified modules 
        if (!overwroteLoop) {
           for (std::map<Address, unsigned char *>::iterator pIter 
	               = loop->shadowMap.begin();
                pIter != loop->shadowMap.end();
                pIter++) 
           {
               modModules.insert( proc()->findModuleByAddr( (*pIter).first ) );
           }
        }

        // debugging output
        for(vector<BPatch_function*>::iterator fIter = owFuncs.begin();
            fIter != owFuncs.end(); 
            fIter++) 
        {
            mal_printf("modified function at %lx %s[%d]\n", 
                      (*fIter)->getBaseAddr(),FILE__,__LINE__);
        }

        // remove function instrumentation datastructures
        vector<BPatch_function*>::iterator fIter = owFuncs.begin();
        for(;fIter != owFuncs.end(); fIter++) {   
            // this will already have happened internally, here we clear out 
            // our instrumentation datastructures
            hybrid_->removeInstrumentation( *fIter,false,false );
            (*fIter)->getCFG()->invalidate();
        }

        // Clean up datastructures for pages that had code but don't anymore
        // Re-instate code discovery instrumentation
        for(set<BPatch_module*>::iterator mIter = modModules.begin();
            mIter != modModules.end(); 
            mIter++) 
        {
            (*mIter)->lowlevel_mod()->obj()->removeEmptyPages();
            hybrid_->instrumentModule(*mIter,false);
        }

        // call user-level callback, but first set up its arguments
        hybrid_->proc()->getImage()->getNewCodeRegions(newFuncs,modFuncs);
        for(fIter = owFuncs.begin(); fIter != owFuncs.end(); fIter++) {   
            std::vector<BPatch_function*>::iterator fIter2 =newFuncs.begin();
            for (;fIter2 != newFuncs.end(); fIter2++) {
                if ((*fIter2) == (*fIter)) {
                    newFuncs.erase(fIter2);
                    break;
                }
            }
            fIter2 =modFuncs.begin();
            for (;fIter2 != modFuncs.end(); fIter2++) {
                if ((*fIter2) == (*fIter)) {
                    modFuncs.erase(fIter2);
                    break;
                }
            }
        }
        bpatchEndCB(deadBlocks, owFuncs, modFuncs, newFuncs); 
        hybrid_->proc()->getImage()->clearNewCodeRegions();

    } // if the code changed 
    else {
        hybrid_->stats_.owFalseAlarm++;
    }

    // delete the loop, but if the code changed, we've probably already done so
    if (!overwroteLoop && idToLoop.end() != idToLoop.find(loopID)) { 
        deleteLoop(loop,false);
    }
    for (map<Address,int>::iterator lit = blockToLoop.begin();
         lit != blockToLoop.end();) 
    {
        if (lit->second == loopID) {
            lit = blockToLoop.erase(lit);
        } else {
            lit++;
        }
    }
    proc()->finalizeInsertionSet(false);
    proc()->protectAnalyzedCode();
    malware_cerr << "overWriteAnalysis returns" << endl;
}
#endif


bool HybridAnalysisOW::isRealStore(Address insnAddr, block_instance *block, 
                                   BPatch_function *func) 
{
    using namespace InstructionAPI;
    const unsigned char* buf = reinterpret_cast<const unsigned char*>
        (proc()->lowlevel_process()->getPtrToInstruction(insnAddr));
    InstructionDecoder decoder(buf,
			                   InstructionDecoder::maxInstructionLength,
            			       proc()->lowlevel_process()->getArch());
    Instruction insn = decoder.decode();
    assert(insn.isValid());
    parse_func *imgfunc = func->lowlevel_func()->ifunc(); 
    Address image_addr = func->lowlevel_func()->addrToOffset(insnAddr);

    std::vector<Assignment::Ptr> assignments;
    AssignmentConverter aConverter(false, false);
    aConverter.convert(insn, image_addr, imgfunc, block->llb(), assignments);

    for (std::vector<Assignment::Ptr>::const_iterator a_iter = assignments.begin();
         a_iter != assignments.end(); ++a_iter) 
    {
        if ((*a_iter)->out().contains(Absloc::Heap)) {
            return true;
        }
    }
    return false;
}


/* Informs the mutator that an instruction will write to a page
` * that contains analyzed code.  
 * This function decides where to put the instrumentation that will mark
 * the end of the overwriting phase
 * 
 * 1. If this is an already instrumented instruction that has now moved onto 
 *     an adjacent page or is in a subsequent iteration of the instrumented loop:
 * 1a.Make a shadow copy of the overwritten page and restore write permissions 
 * . Instrument the loop
 * . Make a shadow copy of the block that is about to be overwritten
 * . Restore write permissions to the written page
 */
void HybridAnalysisOW::overwriteSignalCB
(Address faultInsnAddr, Address writeTarget) 
{
    using namespace std;
    // debugging output
    mal_printf("\noverwriteSignalCB(%lx , %lx)\n", faultInsnAddr, writeTarget);
    // setup
    vector<BPatch_function*> faultFuncs;
    proc()->findFunctionsByAddr(faultInsnAddr,faultFuncs);
    assert(!faultFuncs.empty());
    vector<BPatch_basicBlock*> faultBlocks;
    for (unsigned fidx=0; fidx < faultFuncs.size(); fidx++) {
	    faultBlocks.push_back(faultFuncs[fidx]->getCFG()->findBlockByAddr(faultInsnAddr));
    }
    assert(faultBlocks.size() == faultFuncs.size());
    const unsigned int pageSize = proc()->lowlevel_process()->getMemoryPageSize();
    Address pageAddress = (writeTarget / pageSize) * pageSize;
    proc()->beginInsertionSet();

/* 1. If this is an already instrumented instruction that has now moved onto 
      an adjacent page or is in a subsequent iteration of the instrumented loop: */
    owLoop *loop = findLoop(faultBlocks[0]->getStartAddress());
    if ( loop ) {
        mal_printf("matches existing loop %d of %lu blocks\n", loop->getID(), loop->blocks.size());
        //make sure we haven't added this page to the loop's shadows already
        assert(loop->shadowMap.end() == loop->shadowMap.find(pageAddress));

        // if this loop has written to more than one location since it
        // last exited (or will), indicate it by zeroing out loopWriteTarget
        if (loop->getWriteTarget() != writeTarget || loop->blocks.size() > 1) {
            loop->setWriteTarget(0);
        } 

        // if the write is to same page as the write instruction and we didn't
        // already know that it does this, instrument the loop with bounds 
        // checks
        if( loop->isRealLoop() && 
            pageAddress <= faultInsnAddr &&
            faultInsnAddr < (pageAddress + pageSize) &&
            ! loop->writesOwnPage() )
        {
            loop->setWritesOwnPage(true);
            mal_printf("discovered that loop %d writes to one of its code pages, "
                       "will add write-bounds instrumentation\n", loop->getID());
            loop->instrumentLoopWritesWithBoundsCheck();
        }

        makeShadow_setRights(writeTarget, loop);
        loop->setActive(true);
        proc()->finalizeInsertionSet(false);
        return;
    }

    // grab the next available loopID 
    loop = new owLoop(this, writeTarget);
    mal_printf("new overwrite loop %d %s[%d]\n", loop->getID(), FILE__,__LINE__);

    // find loops surrounding the write insn
    BPatch_basicBlockLoop *bblLoop = getWriteLoop(*faultFuncs[0],faultInsnAddr,true);

    if ( bblLoop) 
    { 
        // set loop blocks, returns false if there's indirect control flow in the loop, 
        // in which case we default to immediate instrumentation
        set<int> overlappingLoops;
        if (1 == faultFuncs.size() &&
            setLoopBlocks(loop, bblLoop, overlappingLoops) &&
            ( 0 == overlappingLoops.size() || 
            removeOverlappingLoops(loop,overlappingLoops))) 
        {
            // add bounds checks to the loop if it writes to its own page
            if (pageAddress <= faultInsnAddr && 
                               faultInsnAddr < (pageAddress + pageSize)) 
            {   
                loop->instrumentLoopWritesWithBoundsCheck();
                loop->setWritesOwnPage(true);
                for (unsigned bidx=0; bidx < faultBlocks.size(); bidx++) {
                    // If the fault block contains the write target,
                    // we will have to single-step the write operation
                    if (writeTarget >= faultBlocks[bidx]->getStartAddress() && 
                        writeTarget < faultBlocks[bidx]->getEndAddress()) 
                    {
                        assert(0 && "KEVINTEST: test what happens when "
                               "program writes to its own basic block");
                        // change PC and context to next instruction to be
                        // executed, then single step the write instruction 
                        // how does Olly do this?
                    }
                }
            }
        } 
        else { 
            // clear out loop block datastructures
            // KEVINOPTIMIZE: shift to instrumenting a smaller loop if there are 
            // any and they don't contain indirect ctrl flow, instead of
            // reverting to single block instrumentation, as we currently are
            mal_printf("Found loop, but it contains unresolved control flow "
                    "functions that may not return, shared code, or overlaps "
                    "with another loop in a strange way "
                    "will instrument after write %s[%d]\n",FILE__,__LINE__);
            bblLoop = NULL; 
        }
    }

    // found no loop! instrument after write operation
    if ( !bblLoop ) { 
        mal_printf("Instrumenting after overwrite instruction at %lx in block "
                   "[%lx %lx), loopID=%d %s[%d]\n",faultInsnAddr,
                   faultBlocks[0]->getStartAddress(), faultBlocks[0]->getEndAddress(),
                   loop->getID(), FILE__, __LINE__);
        assert(blockToLoop.find(faultBlocks[0]->getStartAddress()) == blockToLoop.end());
        for (unsigned bidx= 0; bidx < faultBlocks.size(); bidx++) {
            loop->blocks.insert(faultBlocks[bidx]);
            blockToLoop[faultBlocks[bidx]->getStartAddress()] = loop->getID();
        }
        loop->instrumentOneWrite(faultInsnAddr,faultFuncs);
        // we're only going to write to one spot, set write target and writeInsns
    }

    else {
        // . Instrument the loop
        loop->instrumentOverwriteLoop(faultInsnAddr);
        mal_printf("Instrumenting loop for write at %lx, loop has %lx blocks, "
                   "loopID=%d %s[%d]\n", faultInsnAddr, 
                   loop->blocks.size(), loop->getID(), FILE__, __LINE__);
    }

    // make a shadow page and restore write privileges to the page
    makeShadow_setRights(writeTarget, loop);
    loop->setActive(true);
    proc()->finalizeInsertionSet(false);
}

bool HybridAnalysisOW::registerCodeOverwriteCallbacks
        (BPatchCodeOverwriteBeginCallback cbBegin,
         BPatchCodeOverwriteEndCallback cbEnd)
{
    bpatchBeginCB = cbBegin;
    bpatchEndCB = cbEnd;
    return true;
}

bool HybridAnalysisOW::removeCodeOverwriteCallbacks()
{
    bool ret = false;
    if (bpatchBeginCB && bpatchEndCB) {
        ret = true;
    }
    bpatchBeginCB = NULL;
    bpatchEndCB = NULL;
    return ret;
}

bool HybridAnalysisOW::getActiveLoops
(std::vector<HybridAnalysisOW::owLoop*> &active)
{
    std::set<HybridAnalysisOW::owLoop*>::iterator lit;
    for (lit = loops.begin(); lit != loops.end(); lit++) {
        if ((*lit)->isActive()) {
            active.push_back(*lit);
        }
    }
    return !active.empty();
}

bool HybridAnalysisOW::activeOverwritePages(std::set<Address> &pages)
{
    std::vector<HybridAnalysisOW::owLoop*> active;
    getActiveLoops(active);
    for (vector<HybridAnalysisOW::owLoop*>::iterator lit = active.begin(); 
         lit != active.end(); 
         lit++)
    {
        for (map<Address,unsigned char*>::iterator pit = (*lit)->shadowMap.begin();
             pit != (*lit)->shadowMap.end();
             pit++)
        {
            pages.insert(pit->first);
        }
    }
    return !pages.empty();
}

