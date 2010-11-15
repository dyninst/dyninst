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
#include "BPatch_process.h"
#include "BPatch_function.h"
#include "BPatch_edge.h"
#include "BPatch_module.h"
#include "process.h"
#include "function.h"
#include "debug.h"
#include "dynutil/h/AST.h"
#include "Absloc.h"
#include "AbslocInterface.h"
#include "InstructionDecoder.h"
#include "instPoint.h"

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
}

HybridAnalysisOW::owLoop::~owLoop()
{
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
                    findFuncByInternalFunc(dynamic_cast<image_func*>(*fiter))->
                    getAddress();
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
        owLoop *loop = idToLoop[blockToLoop[blockStart]];
        // because of blocks being overwritten, sometimes we can't tear blocks
        // out because the internal blocks have been purged and we can't figure
        // out the block address.  Eventually, if the block is reconstituted 
        // we may be able to find it here with a reference to a defunct loop,
        // make sure that this is not the case, if it is, tear the block out
        if (NULL == loop) {
            blockToLoop.erase(blockStart);
            loop = NULL;
        }
        return loop;
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
bool HybridAnalysisOW::deleteLoop(owLoop *loop, bool useInsertionSet, BPatch_point *writePoint)
{
    bool isLoopActive = loop->isActive();
    mal_printf("deleteLoop: loopID =%d active=%d %s[%d]\n",
              loop->getID(),isLoopActive,FILE__,__LINE__);

    if (isLoopActive) {
        // make sure the underlying code hasn't changed
        bool changedPages = false;
        bool changedCode = false;
        std::vector<Address> deadBlockAddrs;
        std::vector<BPatch_function*> modFuncs;
        if (writePoint) {
            overwriteAnalysis(writePoint,(void*)loop->getID());
        } else {
    	    proc()->overwriteAnalysisUpdate(loop->shadowMap,
                                            deadBlockAddrs,
                                            modFuncs,
                                            changedPages,
                                            changedCode);
            assert(!changedCode && "bug, overwrite loops should not contain "
                   "instructions that could trigger analysis update callbacks");
        }
        proc()->protectAnalyzedCode();
    }

    // remove loop instrumentation
    if (useInsertionSet) {
        proc()->beginInsertionSet();
    }
    std::set<BPatchSnippetHandle*>::iterator sIter = loop->snippets.begin();
    for (; sIter != loop->snippets.end(); sIter++) {
        proc()->deleteSnippet(*sIter);
    }

    // clear loop from blockToLoop and idToLoop datastructures
    std::set<BPatch_basicBlock*,HybridAnalysis::blockcmp>::iterator bIter 
	    = loop->blocks.begin();
    for (; bIter != loop->blocks.end(); bIter++) {
        if ( !(*bIter)->lowlevel_block() ) {
            mal_printf("WARNING: Can't remove overwritten loop block, the "
                       "internal block was deleted so we can't ascertain its "
                       "address: loopID %d block %lx %s[%d]\n", 
                       loop->getID(), *bIter,FILE__,__LINE__);
        } else {
            blockToLoop.erase((*bIter)->getStartAddress());
        }
    }

    // delete loop (destructor deletes shadow pages)
    assert(idToLoop.end() != idToLoop.find(loop->getID()));
    idToLoop.erase(idToLoop.find(loop->getID()));
    delete loop;

    if (useInsertionSet) {
        proc()->finalizeInsertionSet(false);
    }
    return isLoopActive;
}


bool HybridAnalysisOW::hasLoopInstrumentation
    (bool activeOnly, BPatch_function &func, std::set<owLoop*> *loops)
{
    //_ASSERTE(_CrtCheckMemory());
    // NEED TO BE CAREFUL BECAUSE WHEN WE OVERWRITE A BLOCK WE DON'T INVALIDATE
    // THE FLOWGRAPH, BUT FROM THE INT-LAYER ON DOWN THINGS ARE INVALIDATED.
    // ANOTHER PROBLEM IS THAT WE MAY BE KEEPING THE BLOCK AROUND AS A PART 
    // OF A BLOCK LOOP
    bool foundLoop = false;

    // get function's blocks
    BPatch_Set<BPatch_basicBlock*> blocks;
    BPatch_flowGraph *cfg = func.getCFG();
    if (!cfg) {
        assert(0);
        return false;
    }
    cfg->getAllBasicBlocks(blocks);

    // find loops matching the function's blocks
    BPatch_Set<BPatch_basicBlock*>::iterator bIter = blocks.begin();
    for (; bIter != blocks.end(); bIter++) 
    {
        owLoop *loop = findLoop((*bIter)->getStartAddress());
        if (loop && (loop->isActive() || !activeOnly) ) {
            foundLoop = true;
            if (loops) {
                loops->insert(loop);
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
void HybridAnalysisOW::owLoop::instrumentOverwriteLoop
    (Address writeInsn, std::set<BPatch_point*> &unresExits)
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
            }
        }
        outEdges.clear();
        bIter++;
    }

    // 2. Instrument exit edges and unresolved points with callbacks to 
    //    the analysis update routine
    hybridow_->proc()->beginInsertionSet();
    BPatch_stopThreadExpr stopForAnalysis(overwriteAnalysis_wrapper, 
                                          BPatch_constExpr(getID()),
                                          false,
                                          BPatch_noInterp);
    // 2a.Instrument at loop exit edges
    std::set<BPatch_edge*>::iterator eIter = instEdges.begin();
    BPatchSnippetHandle *snippetHandle = NULL;
    while (eIter != instEdges.end()) {
        BPatch_point *edgePoint = (*eIter)->getPoint();
        mal_printf(" instr edge: 0x%x -> 0x%x\n", 
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
    std::set<BPatch_point*>::iterator uIter = unresExits.begin();
    while (uIter != unresExits.end()) {
        Address uAddr = (Address)(*uIter)->getAddress();
        mal_printf(" instr unresolved: 0x%x in func at 0x%lx\n", 
                  uAddr, (*uIter)->getFunction()->getBaseAddr());
        if ((*uIter)->isDynamic()) {
            long st = (*uIter)->getSavedTarget();
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

    hybridow_->proc()->finalizeInsertionSet(false);
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

    hybridow_->proc()->beginInsertionSet();
    for (unsigned fidx =0; fidx < writeFuncs.size(); fidx++) 
    {
        // create instrumentation points
        BPatch_point * writePoint = hybridow_->proc()->getImage()->
                createInstPointAtAddr((void*)writeInsnAddr, 
                                      NULL, 
                                      writeFuncs[fidx]);

        // instrument and store the snippet handle
        BPatchSnippetHandle *snippetHandle = hybridow_->proc()->insertSnippet
            (stopForAnalysis, *writePoint, BPatch_callAfter);
	    assert(snippetHandle);
        snippets.insert(snippetHandle);
    }
    hybridow_->proc()->finalizeInsertionSet(false);
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
    assert(blocks.size());

    // build the set that describes the type of accesses we're looking for
    BPatch_Set<BPatch_opCode> insnTypes;
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
        if (boundsArray[boundsIdx-1] == (*bIter)->getStartAddress()) {
            boundsArray[boundsIdx-1] = (*bIter)->getEndAddress();
        } else {
            if (boundsIdx > 0) {
                mal_printf("BA[%d] = [%lx %lx]\n", boundsIdx-2, 
                           boundsArray[boundsIdx-2],boundsArray[boundsIdx-1]);
            }
            boundsArray[boundsIdx]   = (*bIter)->getStartAddress();
            boundsArray[boundsIdx+1] = (*bIter)->getEndAddress();
            boundsIdx+=2;
        }
        bIter++; 
    }
    if (boundsIdx > 0) {
        mal_printf("BA[%d] = [%lx %lx]\n", 
                   boundsIdx-2, boundsArray[boundsIdx-2], boundsArray[boundsIdx-1]);
    }
    mal_printf("instrumenting %d store instructions in loop %d %s[%d]\n",
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
    hybridow_->proc()->beginInsertionSet();
    for (unsigned wIdx=0; wIdx < loopWrites.size(); wIdx++) {
        // create the stopthread expression
        BPatch_stopThreadExpr stopForAnalysis
            (overwriteAnalysis_wrapper, 
             BPatch_constExpr(-1*loopID_), 
             false,BPatch_noInterp);
        mal_printf("BoundsCheck Call at %lx\n",loopWrites[wIdx]->getAddress());
        // create the if expression
        BPatch_ifExpr ifBoundsThenStop(condition, stopForAnalysis);
        // insert the snippet 
        BPatchSnippetHandle *handle = hybridow_->proc()->insertSnippet
            (ifBoundsThenStop, *loopWrites[wIdx], BPatch_callAfter);
        snippets.insert(handle);
    }
    hybridow_->proc()->finalizeInsertionSet(false);
}


// gets biggest loop without unresolved/multiply resolved indirect ctrl flow that it can find
BPatch_basicBlockLoop* HybridAnalysisOW::getWriteLoop(BPatch_function &func, Address writeAddr)
{
    BPatch_flowGraph *graph = func.getCFG();
    vector<BPatch_basicBlockLoop*> loops;
    vector<BPatch_point*> blockPoints;
    vector<BPatch_basicBlock *>loopBlocks;
    graph->getLoops(loops);
    vector<BPatch_basicBlockLoop*>::iterator lIter = loops.begin();
    BPatch_basicBlockLoop *writeLoop = NULL;
    while (lIter != loops.end()) {

        mal_printf("found nat'l loop w/ block[0] at 0x%x, back edge at 0%x\n",
                   (*lIter)->getLoopHead()->getStartAddress(),
                   (*lIter)->getBackEdge()->getSource()->getLastInsnAddress());

        if ((*lIter)->containsAddressInclusive(writeAddr) && 
            (!writeLoop || writeLoop->hasAncestor(*lIter))) 
        {
            // set writeLoop if the curloop has no indirect control transfers
            bool hasIndirect = false;
            (*lIter)->getLoopBasicBlocks(loopBlocks);
            for (vector<BPatch_basicBlock*>::iterator bIter= loopBlocks.begin(); 
                bIter != loopBlocks.end() && !hasIndirect; 
                bIter++) 
            {
                Address blockStart = (*bIter)->getStartAddress();
                if (blockToLoop.find(blockStart) != blockToLoop.end()) {
                    // this block corresponds to another instrumented loop!
                    fprintf(stderr,"WARNING: block [%lx %lx] in loop overlaps with "
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
                        if (0 == (*pIter)->getSavedTarget()) {
                            // for now, warn, but allow to proceed
                            mal_printf("loop has an unresolved indirect transfer at %lx\n", 
                                    (*pIter)->getAddress());
                            hasIndirect = true;
                        } else if (-1 == (long)(*pIter)->getSavedTarget()) {
                            mal_printf("loop has an ambiguously resolved indirect transfer at %lx\n", 
                                    (*pIter)->getAddress());
                            hasIndirect = true;
                        }
                    }
                }
                blockPoints.clear();
            }
            loopBlocks.clear();
            
            if (!hasIndirect) {
                writeLoop = *lIter;
            }
        }
        lIter++;
    }
    if (writeLoop) {
        mal_printf("CHOSE nat'l loop with: block[0] at 0x%x, back edge at 0%x\n",
		    writeLoop->getLoopHead()->getStartAddress(),
		    writeLoop->getBackEdge()->getSource()->getLastInsnAddress());
    }
    return writeLoop;
}

// recursively add all functions that contain calls, 
// return true if the function contains no unresolved control flow
// and the function returns normally
bool HybridAnalysisOW::addFuncBlocks(owLoop *loop, 
                   std::set<BPatch_function*> &addFuncs, 
                   std::set<BPatch_function*> &seenFuncs,
                   std::set<BPatch_point*> &exitPoints,
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
        BPatch_Set<BPatch_basicBlock*> fBlocks;
        (*fIter)->getCFG()->getAllBasicBlocks(fBlocks);
        for (BPatch_Set<BPatch_basicBlock*>::iterator bIter= fBlocks.begin(); 
            bIter != fBlocks.end(); 
            bIter++) 
        {
            // add the block to the new loop's datastructures
            Address blockStart = (*bIter)->getStartAddress();
            if (blockToLoop.find(blockStart) != blockToLoop.end() && 
                loop->getID() != blockToLoop[blockStart]) 
            {
                mal_printf("block [%lx %lx] in loop %d overlaps with existing loop %d %s[%d]\n",
                        blockStart, (*bIter)->getEndAddress(),
                        loop->getID(), blockToLoop[blockStart],FILE__,__LINE__);
                overlappingLoops.insert(blockToLoop[blockStart]);
            } else {
                blockToLoop[blockStart] = loop->getID();
            }
            loop->blocks.insert(*bIter);

            // if call is to unseen function then add it for next iteration
            BPatch_function *targFunc = (*bIter)->getCallTarget();
            if ( targFunc && seenFuncs.find(targFunc) == seenFuncs.end() ) {
                seenFuncs.insert(targFunc);
                if (/*buggy*/ targFunc->getModule()->isExploratoryModeOn() ) {
                    nextAddFuncs.insert(targFunc);
                    //KEVINTODO: this 0x50000000 test is disgusting
                } else if ((Address)targFunc->getBaseAddr() < 0x50000000) {
                    mal_printf("revbug targfunc at=%lx %d\n",
                               (Address)targFunc->getBaseAddr(),__LINE__);
                    const int nameLen = 32;
                    char modName[nameLen];
                    targFunc->getModule()->getName(modName,nameLen);
                    fprintf(stderr,"ERROR: overwrite loop calls into func at "
                            "0x%lx in module %s that is not marked with "
                            "malware mode %s[%d]\n",
                            (Address)targFunc->getBaseAddr(),
                            modName,FILE__,__LINE__);
                 } else {
                    mal_printf("revbug targfunc at=%lx %d\n",targFunc->getBaseAddr(),__LINE__);
                }
            }
        }
        // if func contains ambiguously resolved control flow, or truly 
        // unresolved ctrl flow, then set flag and clear vector
        (*fIter)->getUnresolvedControlTransfers(unresolvedCF);
        for (vector<BPatch_point*>::iterator pIter = unresolvedCF.begin(); 
            pIter != unresolvedCF.end(); 
            pIter++) 
        {
            Address curTarg = (*pIter)->getSavedTarget();
            if (-1 == (long)curTarg) {
                hasUnresolved = true;
                mal_printf("loop %d calls func %lx which has an ambiguously "
                          "resolved indirect transfer at %lx savedtarg %lx "
                          "%s[%d]\n", loop->getID(), (*fIter)->getBaseAddr(), 
                          (*pIter)->getAddress(), curTarg, FILE__,__LINE__);
            } else {
                exitPoints.insert(*pIter);
            }
        }
        if (unresolvedCF.size()) {
            unresolvedCF.clear();
        }
        // also want to make sure this function is returning
        if ( ParseAPI::RETURN != 
             (*fIter)->lowlevel_func()->ifunc()->retstatus() ) 
        {
            mal_printf("loop %d calls func %lx which appears to be "
                      "non-returning %s[%d]\n", loop->getID(), 
                      (*fIter)->getBaseAddr(), FILE__,__LINE__);
            hasUnresolved = true;
        }
    }
    // if these functions called additional functions
    if (nextAddFuncs.size()) {
        if (false == addFuncBlocks(loop, 
                                   nextAddFuncs, 
                                   seenFuncs, 
                                   exitPoints,
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
                                     std::set<BPatch_point*> &exitPoints,
                                     std::set<int> &overlappingLoops)
{
    bool hasUnresolvedCF = false;
    std::set<BPatch_function*> loopFuncs;
    vector<BPatch_point*> blockPoints;
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
        } else {
            blockToLoop[(*bIter)->getStartAddress()] = loop->getID();
        }
        loop->blocks.insert(*bIter);

        // if the block has an indirect control transfer
        (*bIter)->getAllPoints(blockPoints);
        for (vector<BPatch_point*>::iterator pIter= blockPoints.begin(); 
            pIter != blockPoints.end(); 
            pIter++) 
        {
            if ((*pIter)->isDynamic()) {
                Address target = (*pIter)->getSavedTarget();
                if (-1 == (long)target) {
                    // if the transfer's target is not uniquely resolved, 
                    // we won't use this loop
                    mal_printf("loop %d has an indirect transfer with "
                              "ambiguously unresolved target at %lx\n", loop->getID(), 
                              (*pIter)->getAddress());
                    hasUnresolvedCF = true;
                } else if (0 == target) {
                    // haven't seen its target yet, mark it as a loop exit
                    exitPoints.insert(*pIter);
                } else {
                    // if the transfer IS uniquely resolved, add the target
                    mal_printf("loop %d has an indirect transfer at %lx with "
                              "target %lx\n", loop->getID(), (*pIter)->getAddress(), 
                              target);
                    vector<BPatch_function *> targFuncs;
                    proc()->findFunctionsByAddr(target,targFuncs);
                    if (targFuncs.size() && 
                        targFuncs[0]->getModule()->isExploratoryModeOn()) 
                    {
                        loopFuncs.insert(targFuncs.begin(),targFuncs.end());
                    } else if (targFuncs.size()) {
                        mal_printf("loop contains call to non-mal-func:%lx %d\n",
                                   targFuncs[0]->getBaseAddr(), __LINE__);
                    }
                }
            }
        }
        blockPoints.clear();

        // if the block has a call, add it to the list of called funcs
        BPatch_function *targFunc = (*bIter)->getCallTarget();
        if ( targFunc && targFunc->getModule()->isExploratoryModeOn()) { 
            mal_printf("loop has a function call %lx->%lx\n", 
                      (*bIter)->getLastInsnAddress(), targFunc->getBaseAddr());
            loopFuncs.insert(targFunc);
        } else if (targFunc) {
            mal_printf("loop calls non-mal-func:%lx [%d]\n",targFunc->getBaseAddr(), __LINE__);
        }
    }
    //recursively add blocks in called functions to the loop
    if (loopFuncs.size()) {
        std::set<BPatch_function*> loopFuncCopy(loopFuncs);
        if (false == addFuncBlocks(loop, 
                                   loopFuncs, 
                                   loopFuncCopy, 
                                   exitPoints, 
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
        if (0 == otherLoop->blocks.size()) { // if cleanup is only partially done
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
        if (loop->shadowMap.size()) {
            std::map<Address,unsigned char*>::iterator siter = loop->shadowMap.begin();
            for (; siter != loop->shadowMap.end(); siter++) {
    	        proc()->setMemoryAccessRights
                    ((*siter).first, 1, PAGE_EXECUTE_READWRITE);
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

	// Restore write permissions to the written page
    proc()->setMemoryAccessRights(pageAddr, 1, PAGE_EXECUTE_READWRITE);
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
    std::vector<Address> deadBlockAddrs;
    std::vector<BPatch_function*> newFuncs;
    std::vector<BPatch_function*> modFuncs;
    const unsigned int pageSize = proc()->lowlevel_process()->getMemoryPageSize();
	Address pageAddress = (((Address)pointAddr) / pageSize) * pageSize;
    bool overwroteLoop = false;
    long loopID = (long)loopID_;

    //if this is the exit of a bounds check exit:
    if (loopID < 0) {
        loopID *= -1;
		overwroteLoop = true;
//        assert(0 && "KEVINTODO: test this, overwrite loop modified itself, triggering bounds check instrumentation");
    }

    owLoop *loop = idToLoop[loopID]; 

    // find the loop corresponding to the loopID, and if there is none, it
    // means we tried to delete the instrumentation earlier, but failed
    if (NULL == loop) {
        fprintf(stderr,"executed instrumentation for old loop %d at point %lx "
                  "%s[%d]\n", loopID, pointAddr, FILE__,__LINE__);
        // this should not happen now that instrumentation removal unlinks 
        // multiTramps and can only fail to remove multis with calls in them
        assert(0);
        return; 
    }

    loop->setActive(false);

    if (overwroteLoop) {
        proc()->beginInsertionSet();
        std::set<BPatchSnippetHandle*>::iterator sIter = loop->snippets.begin();
        for (; sIter != loop->snippets.end(); sIter++) {
            proc()->deleteSnippet(*sIter);
        }
        loop->snippets.clear();
        proc()->finalizeInsertionSet(false);
    }

/* 1. Identify overwritten blocks and update the analysis */
    // if writeTarget is non-zero, only one byte was overwritten
    proc()->overwriteAnalysisUpdate(loop->shadowMap, 
                                    deadBlockAddrs,
                                    owFuncs, 
                                    changedPages,
                                    changedCode);

    proc()->beginInsertionSet();

/* 2. if the code changed, remove loop & overwritten instrumentation, 
      then re-instrument */
    if (changedCode) {

        // build up list of mods to instrument by traversing the functions, or
        // if there aren't any, by traversing the block list
        std::set<BPatch_module*> mods;
        if (owFuncs.size()) {
            for(vector<BPatch_function*>::iterator fIter = owFuncs.begin();
                fIter != owFuncs.end(); 
                fIter++) 
            {
                mal_printf("modified function at %lx %s[%d]\n", 
                          (*fIter)->getBaseAddr(),FILE__,__LINE__);
    		    mods.insert( (*fIter)->getModule() );
            }
        } else {
            // this case only arises if an overwrite eliminated a function 
            // altogether and we chose not to add it to the modified
            // functions list for that reason (e.g., MEW func 0x40e000)
	        for (std::map<Address, unsigned char *>::iterator pIter 
		            = loop->shadowMap.begin();
                 pIter != loop->shadowMap.end();
                 pIter++) 
            {
                mods.insert( proc()->findModuleByAddr( (*pIter).first ) );
            }
        }

        // remove loop and function instrumentation
        deleteLoop(loop,false);
        vector<BPatch_function*>::iterator fIter = owFuncs.begin();
        for(;fIter != owFuncs.end(); fIter++) {   
            // this will already have happened internally, here we clear out 
            // our instrumentation datastructures
            hybrid_->removeInstrumentation( *fIter,false );
            (*fIter)->getCFG()->invalidate();
        }

        // re-instate code discovery instrumentation
        for(set<BPatch_module*>::iterator mIter = mods.begin();
            mIter != mods.end(); 
            mIter++) 
        {
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
        bpatchEndCB(deadBlockAddrs, owFuncs, modFuncs, newFuncs); 
        hybrid_->proc()->getImage()->clearNewCodeRegions();

    } 
    else { // no changes to the code

        // if this is a dynamic point whose target we will have resolved, 
        // remove the loop instrumentation and return, as next time we 
        // may be able to build a more complete loop
        // 
        // Also remove single block loops since we instrument right after
        // the fault instruction and not at the end of the block, meaning
        // that two faults in the block will cause us to treat the 2nd
        // as occurring in the existing inactive loop
        if (point->isDynamic() || 1 == loop->blocks.size()) {
            deleteLoop(loop,false);
        }

        else {
        /* 4. Free shadow pages */
	        std::map<Address, unsigned char *>::iterator spIter = 
                loop->shadowMap.begin();
	        while (spIter != loop->shadowMap.end()) {

                // overwriteSignalCB will renew the shadow pages.
                // free the page now, but leave an entry for it
                unsigned char *oldShadow = (*spIter).second;
                ::free(oldShadow);
                (*spIter).second = NULL;

		        // . Re-instate write protections for the written pages,
                //   if they still contain code
    	        proc()->setMemoryAccessRights(
                    (*spIter).first, 1, PAGE_EXECUTE_READ);
    	        spIter++;
	        }
            loop->shadowMap.clear();

            // reset some fields for this inactive loop in case we want to re-use it
            // hasn't written to its own page... yet.
            loop->setWritesOwnPage(false);
            // clear loopWriteTarget so it can be reset
            loop->setWriteTarget(0);
        }
    }
    proc()->finalizeInsertionSet(false);
    proc()->protectAnalyzedCode();
}
#endif


bool HybridAnalysisOW::isRealStore(Address insnAddr, 
                                   BPatch_function *func) 
{
    using namespace InstructionAPI;
    const unsigned char* buf = reinterpret_cast<const unsigned char*>
        (proc()->lowlevel_process()->getPtrToInstruction(insnAddr));
    InstructionDecoder decoder(buf,
			                   InstructionDecoder::maxInstructionLength,
            			       proc()->lowlevel_process()->getArch());
    Instruction::Ptr insn = decoder.decode();
    assert(insn != NULL);
    image_func *imgfunc = func->lowlevel_func()->ifunc(); 
    Address image_addr = func->lowlevel_func()->addrToOffset(insnAddr);

    std::vector<Assignment::Ptr> assignments;
    AssignmentConverter aConverter(false);
    aConverter.convert(insn, image_addr, imgfunc, assignments);

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

/* 1. If this is an already instrumented instruction that has now moved onto 
      an adjacent page or is in a subsequent iteration of the instrumented loop: */
    owLoop *loop = findLoop(faultBlocks[0]->getStartAddress());
    if ( loop ) {
        //make sure we haven't added this page to the loop's shadows already
        assert(loop->shadowMap.end() == loop->shadowMap.find(pageAddress));

        // if this loop has written to more than one location since it
        // last exited (or will), indicate it by zeroing out loopWriteTarget
        if (loop->getWriteTarget() != writeTarget || loop->blocks.size() > 1) {
            loop->setWriteTarget(0);
        } 

        // if this is not a real loop, this may be a new write instruction 
        // in the existing loop and we will need to instrument after that 
        // write instruction.  
        // While we're at it, we check to see if the function has been
        // updated, in which case we can swith to loop-based instrumentation
        if ( ! loop->isRealLoop() ) {
            // see if the function's analysis has been updated, and if there
            // is now a loop surrounding the write instruction, scrap our
            // current instrumentation and instrument the loop
            if ( loop->codeChanged && 1 == faultFuncs.size()) {

                BPatch_basicBlockLoop * bpLoop = 
                    getWriteLoop(*faultFuncs[0], faultInsnAddr);
                if (bpLoop) {
                    // scrap the current loop instrumentation and instrument 
                    // the new loop instead
                    writeTarget = loop->getWriteTarget();
                    deleteLoop(loop,false);
                    loop = new owLoop(this,writeTarget);
                    set<int> overlappingLoops;
                    set<BPatch_point*> unresExits;
                    // use current counter for now as we may choose not to 
                    // use the new loop 
                    if (setLoopBlocks(loop, 
                                      bpLoop, 
                                      unresExits, 
                                      overlappingLoops) 
                        &&
                        ( 0 == overlappingLoops.size() || 
                          removeOverlappingLoops(loop, overlappingLoops)))
                    {   // use the new loop
                        loop->instrumentOverwriteLoop(faultInsnAddr,unresExits);
                        mal_printf("new overwrite loop %d %d[%d]\n", 
                                   loop->getID(), FILE__,__LINE__);
                    } else {
                        // this loop is unsafe to instrument, go back to 
                        // single block instrumentation
                        loop->instrumentOneWrite(faultInsnAddr,faultFuncs);
                        fprintf(stderr,"failed to expand to loop-based "
                                "instrumentation %s[%d]\n",
                                FILE__,__LINE__);
                    }
                }
            }
            // if there is a new write instruction in the block that hasn't 
            // been instrumented, instrument after the new write instruction
            else if (loop->writeInsns.end() == 
                     loop->writeInsns.find(faultInsnAddr))
            {
                loop->writeInsns.insert(faultInsnAddr);
                loop->instrumentOneWrite(faultInsnAddr,faultFuncs);
            } 
        }

        // if the write is to same page as the write instruction and we didn't
        // already know that it does this, instrument the loop with bounds 
        // checks
        else if( pageAddress <= faultInsnAddr &&
                 faultInsnAddr < (pageAddress + pageSize) &&
                 ! loop->writesOwnPage() )
        {
            loop->setWritesOwnPage(true);
            if ( loop->isRealLoop() ) {
                //re-invoke overwriteSignalCB
                int_function *llfunc = faultFuncs[0]->lowlevel_func();
                instPoint *pt = llfunc->findInstPByAddr(faultInsnAddr);
                if (!pt) {
                    llfunc->funcCalls();
                    llfunc->funcEntries();
                    llfunc->funcExits();
                    llfunc->funcUnresolvedControlFlow();
                    pt = llfunc->findInstPByAddr(faultInsnAddr);
                    if (!pt) {
                        pt = instPoint::createArbitraryInstPoint(
                            faultInsnAddr, proc()->lowlevel_process(), llfunc);
                    }
                }
                assert(pt);
                BPatch_point *writePoint = proc()->findOrCreateBPPoint(
                    faultFuncs[0], 
                    pt, 
                    BPatch_point::convertInstPointType_t(pt->getPointType()));
                deleteLoop(loop, true, writePoint);
                overwriteSignalCB(faultInsnAddr, writeTarget);
                return;
            }
        }

        makeShadow_setRights(writeTarget, loop);
        loop->setActive(true);
        return;
    }

    // grab the next available loopID 
    loop = new owLoop(this, writeTarget);
    mal_printf("new overwrite loop %d %d[%d]\n", loop->getID(), FILE__,__LINE__);

    // find loops surrounding the write insn
    BPatch_basicBlockLoop *bblLoop = getWriteLoop(*faultFuncs[0],faultInsnAddr);
    set<BPatch_point*> unresExits;

    if ( bblLoop) 
    { 
        // set loop blocks, returns false if there's indirect control flow in the loop, 
        // in which case we default to immediate instrumentation
        set<int> overlappingLoops;
        if (1 == faultFuncs.size() &&
            setLoopBlocks(loop, bblLoop, unresExits, overlappingLoops) &&
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
                        assert(0 && "KEVINTODO: test what happens when "
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
            // KEVINTODO: shift to instrumenting a smaller loop if there are 
            // any and they don't contain indirect ctrl flow, instead of
            // reverting to single block instrumentation, as we currently are
            mal_printf("Found loop, but it contains indirect control flow, "
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
        loop->instrumentOverwriteLoop(faultInsnAddr,unresExits);
        mal_printf("Instrumenting loop for write at %lx, loop has %d blocks, "
                   "loopID=%d %s[%d]\n", faultInsnAddr, 
                   loop->blocks.size(), loop->getID(), FILE__, __LINE__);
    }

    // make a shadow page and restore write privileges to the page
    makeShadow_setRights(writeTarget, loop);
    loop->setActive(true);
}

bool HybridAnalysisOW::registerCodeOverwriteCallbacks
        (BPatchCodeOverwriteBeginCallback cbBegin,
         BPatchCodeOverwriteEndCallback cbEnd)
{
    bpatchBeginCB = cbBegin;
    bpatchEndCB = cbEnd;
    return true;//KEVINTODO: these functions should all fail if the hybrid mode is wrong
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
//KEVINTODO: figure out why I'm tracking writeInsnAddrs, I'm not tracking it properly
