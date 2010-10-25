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

#ifndef _HYBRIDANALYSIS_H_
#define _HYBRIDANALYSIS_H_

#include <stdio.h>
#include <set>
#include <map>
#include <vector>
#include "dyntypes.h"
#include "BPatch_hybridAnalysis.h"
#include "BPatch_callbacks.h"

class BPatch_module;
class BPatch_function;
class BPatch_process;
class BPatch_point;
class BPatch_thread;
class HybridAnalysisOW;
class BPatchSnippetHandle;
class BPatch_basicBlock;
class BPatch_basicBlockLoop;

/* There should only be one instance of this class, as for the BPatch class */
class HybridAnalysis {
    friend class HybridAnalysisOW;

public:

    HybridAnalysis(BPatch_hybridMode mode, BPatch_process *proc);
    ~HybridAnalysis();

    // sets up instrumentation, if there will be any
    bool init(); 
    // returns false if conversion has no effect or is not possible
    bool setMode(BPatch_hybridMode mode);

    HybridAnalysisOW * hybridOW() { return hybridow_; };
    BPatch_process *proc() { return proc_; };
    static InternalSignalHandlerCallback getSignalHandlerCB();
    BPatch_module *getRuntimeLib() { return sharedlib_runtime; }

    // callbacks
    bool isInLoop(Dyninst::Address blockAddr, bool activeOnly);
    void netFuncCB(BPatch_point *point, void *);
    void abruptEndCB(BPatch_point *point, void *);
    void badTransferCB(BPatch_point *point, void *returnValue);
    void signalHandlerEntryCB(BPatch_point *point, void *pcAddr);
    void signalHandlerCB(BPatch_point *pt, long snum, std::vector<Dyninst::Address> &handlers);
    void signalHandlerExitCB(BPatch_point *point, void *returnAddr);
    void overwriteSignalCB(Dyninst::Address faultInsnAddr, Dyninst::Address writeTarget);

    // callback registering functions
    bool registerCodeDiscoveryCallback(BPatchCodeDiscoveryCallback cb);
    bool registerSignalHandlerCallback(BPatchSignalHandlerCallback cb);
    bool removeCodeDiscoveryCallback();
    bool removeSignalHandlerCallback();

    // this function ought to be in BPatch_basicBlock.h, but we 
    // can't include it from this header file without strongly 
    // increasing the likelihood of circular dependencies
    // 
    // no possibility of equality, return true if b1 < b2
    struct blockcmp {
        bool operator()(const BPatch_basicBlock *b1, 
                    const BPatch_basicBlock *b2) const;
    };

private:

    // instrumentation functions 
    bool instrumentModules(bool useInsertionSet); 
    bool instrumentModule(BPatch_module *mod, bool useInsertionSet); 
    bool instrumentFunction(BPatch_function *func, 
                            bool useInsertionSet, 
                            bool instrumentReturns=false);
    bool parseAfterCallAndInstrument(BPatch_point *callPoint, 
                        Dyninst::Address calledAddr, 
                        BPatch_function *calledFunc) ;
    void removeInstrumentation(BPatch_function *func, bool useInsertionSet);
    int saveInstrumentationHandle(BPatch_point *point, 
                                  BPatchSnippetHandle *handle);
    bool hasEdge(BPatch_point *sourcePoint, Dyninst::Address target);

    // parsing
    void parseNewEdgeInFunction(BPatch_point *sourcePoint, 
                                Dyninst::Address target,
                                bool useInsertionSet);
    bool analyzeNewFunction( Dyninst::Address target , 
                             bool doInstrumentation , 
                             bool useInsertionSet );

    // variables
    std::map<Dyninst::Address,Dyninst::Address> handlerFunctions; 
    std::map < BPatch_function*, 
               std::map<BPatch_point*,BPatchSnippetHandle*> *> * instrumentedFuncs;
    BPatch_module *sharedlib_runtime;
    BPatch_hybridMode mode_;
    BPatch_process *proc_;
    HybridAnalysisOW *hybridow_;

    BPatchCodeDiscoveryCallback bpatchCodeDiscoveryCB;
    BPatchSignalHandlerCallback bpatchSignalHandlerCB;

};


class HybridAnalysisOW {
    friend class HybridAnalysis;
public:

public:

    class owLoop {
    public:
        owLoop(HybridAnalysisOW *hybridow, 
               Dyninst::Address writeTarg);
        ~owLoop();
        static int getNextLoopId() { return ++IDcounter_; };
        bool isActive() { return activeStatus_; };
        bool writesOwnPage() { return writesOwnPage_; }
        int getID() { return loopID_; }
        Dyninst::Address getWriteTarget() { return writeTarget_; }
        void setWriteTarget(Dyninst::Address targ);
        void setWritesOwnPage(bool wop);
        void setActive(bool act);

        /* 1. Gather up all instrumentation sites that need to be monitored:
           1a. The edges of all instrumented blocks that leave the block set
           1b. Unresolved points in instrumented blocks
           2. Instrument exit edges and unresolved points with callbacks to 
              the analysis update routine
           2a.Instrument at loop exit edges
           2b.Instrument at unresolved edges in the loop 
         */
        void instrumentOverwriteLoop(Dyninst::Address writeInsnAddr, 
                                     std::set<BPatch_point*> &unresExits);

        void instrumentOneWrite(Dyninst::Address writeInsnAddr, 
                                BPatch_function *writeFunc);

        /*1. initialize necessary variables
          2. create bounds array for all blocks in the loop
          3. create the bounds check function call snippet
          4. instrument each write point
        */
        void instrumentLoopWritesWithBoundsCheck();

        // variables

        bool codeChanged;
        //snippets
        std::set<BPatchSnippetHandle*> snippets;
        //shadows
        std::map<Dyninst::Address,unsigned char *> shadowMap;
        //write instructions
        std::set<Dyninst::Address> writeInsns;
        //loopblocks
        std::set<BPatch_basicBlock*,HybridAnalysis::blockcmp> blocks;
    private:
        //write target, set to 0 if loop has multiple write targets
        Dyninst::Address writeTarget_;
        //loop active status
        bool activeStatus_;
        //loop writes own page
        bool writesOwnPage_;
        HybridAnalysisOW *hybridow_;

        int loopID_;
        static int IDcounter_;
    };


    HybridAnalysisOW(HybridAnalysis *hybrid);
    ~HybridAnalysisOW();
    HybridAnalysis *hybrid() { return hybrid_; };
    BPatch_process *proc() { return hybrid_->proc(); }
    HybridAnalysisOW::owLoop* findLoop(Dyninst::Address blockStart);
    bool isInLoop(Dyninst::Address blockAddr, bool activeOnly);

    bool registerCodeOverwriteCallbacks(BPatchCodeOverwriteBeginCallback cbBegin,
                                  BPatchCodeOverwriteEndCallback cbEnd);
    bool removeCodeOverwriteCallbacks();
    bool codeChangeCB(std::vector<BPatch_function*> &modfuncs);

    // overwrite loop functions
    bool hasLoopInstrumentation
        (bool activeOnly, BPatch_function &func, 
        std::set<HybridAnalysisOW::owLoop*> *loops=NULL);

    /* 1. Check for changes to the underlying code to see if this is safe to do
     * 2. If the loop is active, check for changes to the underlying data, and 
     *    if no changes have occurred, we can just remove the loop instrumentation
     *    and everything will be hunky dory once we re-instate the write 
     *    protections for the loop's pages
     * return true if the loop was active
     */ 
    bool deleteLoop(HybridAnalysisOW::owLoop *loop, 
                    bool useInsertionSet,
                    BPatch_point *writePoint=NULL);

    /* Informs the mutator that an instruction will write to a page
    ` * that contains analyzed code.  
     * This function decides where to put the instrumentation that will mark
     * the end of the overwriting phase
     */
    void overwriteSignalCB
    (Dyninst::Address faultInsnAddr, Dyninst::Address writeTarget);

    void overwriteAnalysis(BPatch_point *point, void *loopID_);
    static InternalCodeOverwriteCallback getCodeOverwriteCB();

private:
    // gets biggest loop without unresolved/multiply resolved indirect ctrl flow that it can find
    BPatch_basicBlockLoop* getWriteLoop(BPatch_function &func, Dyninst::Address writeAddr);

    // recursively add all functions that contain calls, 
    // return true if the function contains no unresolved control flow
    // and the function returns normally
    bool addFuncBlocks(owLoop *loop, std::set<BPatch_function*> &addFuncs, 
                       std::set<BPatch_function*> &seenFuncs,
                       std::set<BPatch_point*> &exitPoints,
                       std::set<int> &overlappingLoops);

    // if writeLoop is null, return the whole function in the loop. 
    // returns true if we were able to identify all code in the loop
    bool setLoopBlocks(owLoop *loop, 
                       BPatch_basicBlockLoop *writeLoop,
                       std::set<BPatch_point*> &exitPoints,
                       std::set<int> &overlappingLoops);

    //returns true if the loop blocks are a superset of the loop(s) it overlaps
    bool removeOverlappingLoops(owLoop *loop, std::set<int> &overlappingLoops);

    // remove any coverage instrumentation
    // make a shadow page, 
    // restore write privileges to the page, 
    void makeShadow_setRights(Dyninst::Address pageAddr,
                              owLoop *loop);

    bool isRealStore(Dyninst::Address insnAddr, 
                     BPatch_function *func);
    // variables

    HybridAnalysis *hybrid_;
    std::set<owLoop*> loops;
#if 0
    //loopid to snippets
    std::map<int,std::set<BPatchSnippetHandle*>*> loopSnippets;
    //loopid to shadows
    std::map<int,std::map<Dyninst::Address,unsigned char *>*> loopShadowMap;
    //loopid to write target
    std::map<int,Dyninst::Address> loopWriteTarget;
    //loopid to write instructions
    std::map<int,std::set<Dyninst::Address>*> loopWriteInsns;
    //loopid to loopblocks
    std::map<int,std::set<BPatch_basicBlock*,HybridAnalysis::blockcmp>*> loopBlockMap;
    //loopblockstarts to loopid: 
    std::map<Dyninst::Address,int> blockToLoop;//KEVINCOMMENT: makes non-guaranteed assumption that only one loop per block, would it be better to use the last instruction address?
    //loop active status
    std::map<int,bool> loopActiveStatus;
    //loop writes own page
    std::map<int,bool> loopWritesOwnPage;
#endif
    std::map<Dyninst::Address,int> blockToLoop;
    std::map<int, owLoop*> idToLoop;

    BPatchCodeOverwriteBeginCallback bpatchBeginCB;
    BPatchCodeOverwriteEndCallback bpatchEndCB;

};


#endif /* _HYBRIDANALYSIS_H_ */
