/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// Put Paradyn specific process code in the object, not in
// dyninstAPI/src/process.[hC]

#ifndef __PD_PROCESS__
#define __PD_PROCESS__

#include "dyninstAPI/src/process.h"
#include "paradynd/src/threadMgr.h"

// Someday this class will inherit from BPatch_thread instead of
// having a process* member

class pd_process {
   process *dyninst_process;

   threadMgr thr_mgr;

   variableMgr *theVariableMgr;

 public:
   unsigned numOfActCounters_is;
   unsigned numOfActProcTimers_is;
   unsigned numOfActWallTimers_is;

 public:
   pd_process(process *p) : dyninst_process(p),
      numOfActCounters_is(0), numOfActProcTimers_is(0),
      numOfActWallTimers_is(0)
   {
      init();
   }

   // fork constructor
   pd_process(const pd_process &parent, process *childDynProc);

   ~pd_process();

   void init();

   threadMgr &thrMgr() { return thr_mgr; }
   void addThread(pd_thread *thr) { thr_mgr.addThread(thr); }
   void removeThread(pd_thread *thr) { thr_mgr.removeThread(thr); }
   void removeThread(int tid) { thr_mgr.removeThread(tid); }
   threadMgr::thrIter beginThr() { return thr_mgr.begin(); }
   threadMgr::thrIter endThrMark() { return thr_mgr.end(); }
   unsigned numThr() const { return thr_mgr.size(); }

   bool doMajorShmSample();
   bool doMinorShmSample();

   const variableMgr &getVariableMgr() const {
      return *theVariableMgr;
   }
   variableMgr &getVariableMgr() {
      return *theVariableMgr;
   }

   // ========  PASS THROUGH FUNCTIONS ===============================

   process *get_dyn_process() { return dyninst_process; }


   processState status() const { return dyninst_process->status(); }

   // handle to resource for this process
   resource *get_rid() { return dyninst_process->rid; }

#ifdef DETACH_ON_THE_FLY
   int reattachAndPause() { return dyninst_process->reattachAndPause(); }
   int detachAndContinue() { return dyninst_process->detachAndContinue(); }
   int detach() { return dyninst_process->detach(); }
#endif
   bool continueProc() { return dyninst_process->continueProc(); }
   bool pause() { return dyninst_process->pause(); }

   void continueAfterNextStop() { dyninst_process->continueAfterNextStop(); }

   bool detach(const bool paused) { // why the param?
      return dyninst_process->detach(paused);
   }

   int getPid() const { return dyninst_process->getPid();  }
   bool existsRPCreadyToLaunch() const { 
      return dyninst_process->existsRPCreadyToLaunch();
   }

   shmMgr *getSharedMemMgr() {  return dyninst_process->getSharedMemMgr();  }

   void cleanRPCreadyToLaunch(int mid) { 
      dyninst_process->cleanRPCreadyToLaunch(mid);
   }

   bool findInternalSymbol(const string &name, bool warn, internalSym &ret_sym)
      const {
      return dyninst_process->findInternalSymbol(name, warn, ret_sym);
   }

   bool writeDataSpace(void *inTracedProcess, u_int amount,
		       const void *inSelf) {
      return dyninst_process->writeDataSpace(inTracedProcess, amount, inSelf);
   }

   void MonitorDynamicCallSites(string function_name) {
      dyninst_process->MonitorDynamicCallSites(function_name);
   }

   bool isBootstrappedYet() const {
      return dyninst_process->isBootstrappedYet();
   }

   bool hasExited() { return dyninst_process->hasExited(); }

   bool wasCreatedViaAttach() { 
      return dyninst_process->wasCreatedViaAttach();
   }

   bool launchRPCifAppropriate(bool wasRunning, bool finishingSysCall) {
      return dyninst_process->launchRPCifAppropriate(wasRunning, 
						     finishingSysCall);
   }

   bool isPARADYNBootstrappedYet() const {
      return dyninst_process->isPARADYNBootstrappedYet();
   }

   bool existsRPCinProgress() {
      return dyninst_process->existsRPCinProgress();
   }

   bool catchupSideEffect(Frame &frame, instReqNode *inst) {
      return dyninst_process->catchupSideEffect(frame, inst);
   }

   int getTraceLink() { return dyninst_process->traceLink; }
	dyn_lwp *getDefaultLWP() const {
		return dyninst_process->getDefaultLWP();
	}

	void postRPCtoDo(AstNode *action, bool noCost,
						  inferiorRPCcallbackFunc callbackFunc,
						  void *userData, int mid, dyn_thread *thr,
						  dyn_lwp *lwp, bool lowmem = false) {
		dyninst_process->postRPCtoDo(action, noCost, callbackFunc, userData,
											  mid, thr, lwp, lowmem);
	}

   bool triggeredInStackFrame(instPoint* point, Frame frame,
			      callWhen when, callOrder order) {
      return dyninst_process->triggeredInStackFrame(point, frame, when, order);
   }

	bool walkStacks(vector<vector<Frame> > &stackWalks) {
		return dyninst_process->walkStacks(stackWalks);
	}

   function_base *findOneFunction(resource *func, resource *mod) {
      return dyninst_process->findOneFunction(func, mod);
   }

   function_base *findOneFunction(const string &func_name) const {
      return dyninst_process->findOneFunction(func_name);
   }

   bool getSymbolInfo(const string &n, Symbol &info, Address &baseAddr) const {
      return dyninst_process->getSymbolInfo(n, info, baseAddr);
   }

   bool findCallee(instPoint &instr, function_base *&target) {
      return dyninst_process->findCallee(instr, target);
   }

   vector<function_base *> *getIncludedFunctions(module *mod) {
      return dyninst_process->getIncludedFunctions(mod);
   }

   vector<function_base *> *getIncludedFunctions() {
      return dyninst_process->getIncludedFunctions();
   }

   vector<module *> *getIncludedModules() {
      return dyninst_process->getIncludedModules();
   }

   module *findModule(const string &mod_name,bool check_excluded) {
      return dyninst_process->findModule(mod_name, check_excluded);
   }

   unsigned maxNumberOfThreads() {
      return dyninst_process->maxNumberOfThreads();
   }

	dyn_thread *getThreadByPOS(unsigned pos) {
		return dyninst_process->getThreadByPOS(pos);
	}

   function_base *getMainFunction() const {
      return dyninst_process->getMainFunction();
   }

   bool dumpCore(const string coreFile) {
      return dyninst_process->dumpCore(coreFile);
   }

   string getProcessStatus() const {
      return dyninst_process->getProcessStatus();
   }

   rawTime64 getRawCpuTime(int lwp_id = -1) {
      return dyninst_process->getRawCpuTime(lwp_id);
   }

   virtualTimer *getVirtualTimerBase() {
      return dyninst_process->getVirtualTimerBase();
   }

   timeLength units2timeLength(int64_t rawunits) {
      return dyninst_process->units2timeLength(rawunits);
   }

#ifdef PAPI
   papiMgr* getPapiMgr() {  return dyninst_process->getPapiMgr();  }
#endif

   bool multithread_capable() {
      return dyninst_process->multithread_capable();
   }
	bool multithread_ready() {
      return dyninst_process->multithread_ready();
	}

   pd_thread *STthread() { 
		assert(! multithread_capable());
      assert(numThr() > 0);
      return (*beginThr());
   }
      
   bool isInSyscall() { return dyninst_process->isInSyscall(); }
   bool thrInSyscall() { return dyninst_process->thrInSyscall(); }

};



#endif

