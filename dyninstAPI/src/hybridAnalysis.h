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

#ifndef _HYBRIDANALYSIS_H_
#define _HYBRIDANALYSIS_H_

#include <stdio.h>
#include <string>
#include <utility>
#include <set>
#include <map>
#include <vector>
#include "dyntypes.h"
#include "BPatch_enums.h"
#include "BPatch_callbacks.h"
#include "function.h"
#include "block.h"
#include "BPatch_process.h"

class BPatch_module;
class BPatch_function;
class BPatch_process;
class BPatch_point;
class BPatch_thread;
class HybridAnalysisOW;
class DefenseReport;
class BPatchSnippetHandle;
class BPatch_basicBlock;
class BPatch_basicBlockLoop;
class block_instance;

#if !defined(os_windows)
#endif

class HybridAnalysis {
    friend class HybridAnalysisOW;

private:
    class SynchHandle {
      public:
        SynchHandle(BPatch_point* prePt, BPatchSnippetHandle* preHandle);
        void setPostHandle(BPatch_point* postPt, BPatchSnippetHandle* postHandle);

        BPatch_point *prePt_;
        BPatch_point *postPt_;
        BPatchSnippetHandle *preHandle_;
        BPatchSnippetHandle *postHandle_;
   };
    typedef struct {
        Dyninst::Address faultPCaddr;
        bool isInterrupt;
    } ExceptionDetails; 

public:

    class AnalysisStats {
      public: 
        AnalysisStats() {
            exceptions = 0; 
            winApiCallbacks = 0;
            unpackCount = 0;
            owCount = 0; 
            owBytes = 0; 
            owExecFunc = 0; 
            owFalseAlarm = 0; 
        }
        int exceptions;
        int winApiCallbacks;
        int unpackCount;
        int owCount;
        int owBytes;
        int owExecFunc;
        int owFalseAlarm;
    };


    HybridAnalysis(BPatch_hybridMode mode, BPatch_process *proc);
    ~HybridAnalysis();

    bool init(); 

    bool setMode(BPatch_hybridMode mode);

    const HybridAnalysis::AnalysisStats & getStats();

    HybridAnalysisOW * hybridOW() { return hybridow_; }
    BPatch_process *proc() { return proc_; }
    static InternalSignalHandlerCallback getSignalHandlerCB();
    BPatch_module *getRuntimeLib() { return sharedlib_runtime; }
    void deleteSynchSnippet(SynchHandle *handle);
    Dyninst::ProcControlAPI::Process::mem_perm getOrigPageRights(Dyninst::Address addr);
    void addReplacedFuncs(std::vector<std::pair<BPatch_function*,BPatch_function*> > &repFs);

    void getCallBlocks(Dyninst::Address retAddr, 
                       func_instance *retFunc,
                       block_instance *retBlock,
                       pair<ParseAPI::Block*, Dyninst::Address> & returningCallB,
                       set<ParseAPI::Block*> & callBlocks);

    std::map< BPatch_point* , SynchHandle* > & synchMap_pre();
    std::map< BPatch_point* , SynchHandle* > & synchMap_post();

    bool isInLoop(Dyninst::Address blockAddr, bool activeOnly);
    void netFuncCB(BPatch_point *point, void *);
    void abruptEndCB(BPatch_point *point, void *);
	void virtualFreeAddrCB(BPatch_point *point, void *);
	void virtualFreeSizeCB(BPatch_point *point, void *);
	void virtualFreeCB(BPatch_point *point, void *);
	void badTransferCB(BPatch_point *point, void *returnValue);
    void signalHandlerEntryCB(BPatch_point *point, Dyninst::Address excRecAddr);
    void signalHandlerEntryCB2(BPatch_point *point, Dyninst::Address excCtxtAddr);
    void signalHandlerCB(BPatch_point *pt, long snum, std::vector<Dyninst::Address> &handlers);
    void signalHandlerExitCB(BPatch_point *point, void *dontcare);
    void synchShadowOrigCB(BPatch_point *point, bool toOrig);
    void overwriteSignalCB(Dyninst::Address faultInsnAddr, Dyninst::Address writeTarget);

    bool registerCodeDiscoveryCallback(BPatchCodeDiscoveryCallback cb);
    bool registerSignalHandlerCallback(BPatchSignalHandlerCallback cb);
    bool removeCodeDiscoveryCallback();
    bool removeSignalHandlerCallback();

    struct blockcmp {
        bool operator()(const BPatch_basicBlock *b1, 
                    const BPatch_basicBlock *b2) const;
    };

private:

    bool instrumentModules(bool useInsertionSet); 
    bool instrumentModule(BPatch_module *mod, bool useInsertionSet); 
    bool instrumentFunction(BPatch_function *func, 
                            bool useInsertionSet, 
                            bool instrumentReturns=false,
                            bool syncShadow = false);
    bool parseAfterCallAndInstrument(BPatch_point *callPoint, 
                        BPatch_function *calledFunc,
                        bool foundByRet) ;
    void removeInstrumentation(BPatch_function *func, 
                               bool useInsertionSet, 
                               bool handlesWereDeleted = false);
    int saveInstrumentationHandle(BPatch_point *point, 
                                  BPatchSnippetHandle *handle);
    bool hasEdge(BPatch_function *func, Dyninst::Address source, Dyninst::Address target);
    bool processInterModuleEdge(BPatch_point *point, 
                                Dyninst::Address target, 
                                BPatch_module *targMod);
    bool canUseCache(BPatch_point *pt);

    void parseNewEdgeInFunction(BPatch_point *sourcePoint, 
                                Dyninst::Address target,
                                bool useInsertionSet);
    bool analyzeNewFunction( BPatch_point *source, 
                             Dyninst::Address target , 
                             bool doInstrumentation , 
                             bool useInsertionSet );
    bool addIndirectEdgeIfNeeded(BPatch_point *srcPt, Dyninst::Address target);

    bool getCallAndBranchTargets(block_instance *block, std::vector<Address> & targs);
    bool getCFTargets(BPatch_point *point, vector<Address> &targets);

    friend void BPatch_process::overwriteAnalysisUpdate
       ( std::map<Dyninst::Address,unsigned char*>& owPages, 
         std::vector<std::pair<Dyninst::Address,int> >& deadBlocks,
         std::vector<BPatch_function*>& owFuncs,     
         std::set<BPatch_function *> &monitorFuncs, 
         bool &changedPages, bool &changedCode ); 

    std::map<Dyninst::Address, ExceptionDetails> handlerFunctions; 
    std::map< BPatch_function*, 
              std::map<BPatch_point*,BPatchSnippetHandle*> *> * instrumentedFuncs;
    std::map< BPatch_point* , SynchHandle* > synchMap_pre_;
    std::map< BPatch_point* , SynchHandle* > synchMap_post_;
    std::set< BPatch_function *> instShadowFuncs_;
    std::set< std::string > skipShadowFuncs_;
    std::map< BPatch_function *, BPatch_function *> replacedFuncs_;
    std::set< BPatch_point* > cachePoints_;
    BPatch_module *sharedlib_runtime;
    BPatch_hybridMode mode_;
    BPatch_process *proc_;
    HybridAnalysisOW *hybridow_;
    AnalysisStats stats_;

    BPatchCodeDiscoveryCallback bpatchCodeDiscoveryCB;
    BPatchSignalHandlerCallback bpatchSignalHandlerCB;

        Dyninst::Address virtualFreeAddr_;
	unsigned virtualFreeSize_;
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
        static int getNextLoopId() { return ++IDcounter_; }
        bool isActive() const { return activeStatus_; }
        bool writesOwnPage() const { return writesOwnPage_; }
        bool isRealLoop() const { return realLoop_; }
        int getID() const { return loopID_; }
        Dyninst::Address getWriteTarget() { return writeTarget_; }
        void setWriteTarget(Dyninst::Address targ);
        void setWritesOwnPage(bool wop);
        void setActive(bool act);

        void instrumentOverwriteLoop(Dyninst::Address writeInsnAddr);

        void instrumentOneWrite(Dyninst::Address writeInsnAddr, 
                                std::vector<BPatch_function*> writeFuncs);

        void instrumentLoopWritesWithBoundsCheck();

        std::set<BPatchSnippetHandle*> snippets;
        std::map<Dyninst::Address,unsigned char *> shadowMap;
        std::set<Dyninst::Address> writeInsns;
        std::set<BPatch_basicBlock*,HybridAnalysis::blockcmp> blocks;
        std::set<BPatch_point*> unresExits_;
    private:
        Dyninst::Address writeTarget_;
        bool activeStatus_;
        bool writesOwnPage_;
        bool realLoop_;

        HybridAnalysisOW *hybridow_;

        int loopID_;
        static int IDcounter_;
    };


    HybridAnalysisOW(HybridAnalysis *hybrid);
    ~HybridAnalysisOW();
    HybridAnalysis *hybrid() { return hybrid_; }
    BPatch_process *proc() { return hybrid_->proc(); }
    HybridAnalysisOW::owLoop* findLoop(Dyninst::Address blockStart);
    bool isInLoop(Dyninst::Address blockAddr, bool activeOnly);

    bool registerCodeOverwriteCallbacks(BPatchCodeOverwriteBeginCallback cbBegin,
                                  BPatchCodeOverwriteEndCallback cbEnd);
    bool removeCodeOverwriteCallbacks();
    bool codeChangeCB(std::vector<BPatch_function*> &modfuncs);

    bool hasLoopInstrumentation
        (bool activeOnly, BPatch_function &func, 
         std::set<HybridAnalysisOW::owLoop*> *loops=NULL);
    bool getActiveLoops(std::vector<HybridAnalysisOW::owLoop*> &active);
    bool activeOverwritePages(std::set<Dyninst::Address> &pages);

    bool deleteLoop(HybridAnalysisOW::owLoop *loop, 
                    bool useInsertionSet,
                    BPatch_point *writePoint=NULL,
                    bool uninstrument=true);

    void overwriteSignalCB
    (Dyninst::Address faultInsnAddr, Dyninst::Address writeTarget);

    void overwriteAnalysis(BPatch_point *point, void *loopID_);
    static InternalCodeOverwriteCallback getCodeOverwriteCB();

private:
    bool removeLoop(HybridAnalysisOW::owLoop *loop, 
                    bool useInsertionSet,
                    BPatch_point *writePoint=NULL,
                    bool uninstrument=true);
   
    BPatch_basicBlockLoop* getWriteLoop(BPatch_function &func, 
                                        Dyninst::Address writeAddr, 
                                        bool allowParentLoop = true);

    BPatch_basicBlockLoop* getParentLoop(BPatch_function &func, Dyninst::Address writeAddr);

    bool addFuncBlocks(owLoop *loop, std::set<BPatch_function*> &addFuncs, 
                       std::set<BPatch_function*> &seenFuncs,
                       std::set<int> &overlappingLoops);

    bool setLoopBlocks(owLoop *loop, 
                       BPatch_basicBlockLoop *writeLoop,
                       std::set<int> &overlappingLoops);

    bool removeOverlappingLoops(owLoop *loop, std::set<int> &overlappingLoops);

    void makeShadow_setRights(Dyninst::Address pageAddr,
                              owLoop *loop);

    bool isRealStore(Dyninst::Address insnAddr, 
                     block_instance *blk, 
                     BPatch_function *func);

    HybridAnalysis *hybrid_;
    std::set<owLoop*> loops;
    std::map<Dyninst::Address,int> blockToLoop;
    std::map<int, owLoop*> idToLoop;

    map<Dyninst::Address,int> writeHits;
    static const int HIT_THRESHOLD = 0;

    BPatchCodeOverwriteBeginCallback bpatchBeginCB;
    BPatchCodeOverwriteEndCallback bpatchEndCB;

};


#endif /* _HYBRIDANALYSIS_H_ */
