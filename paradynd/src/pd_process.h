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
   // Paradyn daemon arguments, etc.
   static pdvector<string> arg_list; // the arguments of paradynd
   // Paradyn RT name if set on the command line
   static string defaultParadynRTname;

   unsigned numOfActCounters_is;
   unsigned numOfActProcTimers_is;
   unsigned numOfActWallTimers_is;

   // the following 3 are used in perfStream.C
   char buffer[2048];
   unsigned bufStart;
   unsigned bufEnd;
   
 public:
  // Creation constructor
  pd_process(const string argv0, pdvector<string> &argv,
             pdvector<string> envp, const string dir,
             int stdin_fd, int stdout_fd, int stderr_fd);

  // Attach constructor
  pd_process(const string &progpath, int pid);
  
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
   void handleExit(int exitStatus);

   // ========  PASS THROUGH FUNCTIONS ===============================

   process *get_dyn_process() { return dyninst_process; }


   processState status() const { return dyninst_process->status(); }

   // handle to resource for this process
   resource *get_rid() { return dyninst_process->rid; }

   bool continueProc() { return dyninst_process->continueProc(); }
   bool pause() { return dyninst_process->pause(); }

   void continueAfterNextStop() { dyninst_process->continueAfterNextStop(); }

   bool detach(const bool paused) { // why the param?
      return dyninst_process->detach(paused);
   }

   int getPid() const { return dyninst_process->getPid();  }

   bool existsRPCPending() const {
       return dyninst_process->existsRPCPending();
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

   bool launchRPCs(bool wasRunning) {
       return dyninst_process->launchRPCs(wasRunning);
   }

   bool isPARADYNBootstrappedYet() const {
       // Good enough approximation (should use a flag here)
       return reachedLibState(paradynRTState, libReady);       
   }

   bool existsRPCinProgress() {
      return dyninst_process->existsRPCinProgress();
   }

   bool catchupSideEffect(Frame &frame, instReqNode *inst) {
      return dyninst_process->catchupSideEffect(frame, inst);
   }

   int getTraceLink() { return dyninst_process->traceLink; }
   void setTraceLink(int v) { dyninst_process->traceLink = v; }
   dyn_lwp *getDefaultLWP() const {
      return dyninst_process->getDefaultLWP();
   }

   void postRPCtoDo(AstNode *action, bool noCost,
                    inferiorRPCcallbackFunc callbackFunc,
                    void *userData, int mid, bool lowmem = false) {
      dyninst_process->postRPCtoDo(action, noCost, callbackFunc, userData,
                                   mid, lowmem);      
   }

   void postRPCtoDo(AstNode *action, bool noCost,
                    inferiorRPCcallbackFunc callbackFunc,
                    void *userData, int mid, dyn_lwp *lwp,
                    bool lowmem = false) {
       dyninst_process->postRPCtoDo(action, noCost, callbackFunc, userData,
                                    mid, lwp, lowmem);
   }
   
   bool triggeredInStackFrame(instPoint* point, Frame frame,
                              callWhen when, callOrder order) {
      return dyninst_process->triggeredInStackFrame(point, frame, when, order);
   }
   
   bool walkStacks(pdvector<pdvector<Frame> > &stackWalks) {
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

   image *getImage() const {
      return dyninst_process->getImage();
   }

   bool findCallee(instPoint &instr, function_base *&target) {
      return dyninst_process->findCallee(instr, target);
   }

   pdvector<function_base *> *getIncludedFunctions(module *mod) {
      return dyninst_process->getIncludedFunctions(mod);
   }

   pdvector<function_base *> *getIncludedFunctions() {
      return dyninst_process->getIncludedFunctions();
   }

   pdvector<module *> *getIncludedModules() {
      return dyninst_process->getIncludedModules();
   }

   module *findModule(const string &mod_name,bool check_excluded) {
      return dyninst_process->findModule(mod_name, check_excluded);
   }

   bool isDynamicallyLinked() { 
      return dyninst_process->isDynamicallyLinked();
   }

   pdvector<shared_object *> *sharedObjects() {
      return dyninst_process->sharedObjects();
   }

   unsigned maxNumberOfThreads() {
      return dyninst_process->maxNumberOfThreads();
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

   virtualTimer *getVirtualTimer(int pos) {
      return dyninst_process->getVirtualTimer(pos);
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

  /************************************
   *** Fork and Exec handling       ***
   ************************************/
  public:
   static void paradynPreForkDispatch(process *p, void *data);
   static void paradynPostForkDispatch(process *p, void *data, process *c);
   static void paradynPreExecDispatch(process *p, void *data, char *arg0);
   static void paradynPostExecDispatch(process *p, void *data);
   static void paradynPreExitDispatch(process *p, void *data, int code);

  private:
   void preForkHandler(process *p);
   void postForkHandler(process *p, process *c);
   void preExecHandler(process *p, char *arg0);
   void postExecHandler(process *p);
   void preExitHandler(process *p, int code);
   
  /************************************
   *** Runtime library functions    ***
   ************************************/
  public:
  // Returns once paradyn lib is loaded and initialized
  bool loadParadynLib();
  
  private:
  libraryState_t paradynRTState;
  libraryState_t auxLibState; // Needed for solaris
  
  // We load via an inferior RPC, so we need a callback
  static void paradynLibLoadCallback(process *, void *data, void *ret);
  void setParadynLibLoaded() { setLibState(paradynRTState, libLoaded); }

  // Replace with BPatch::loadLibrary
  bool loadAuxiliaryLibrary(string libname);
  static void loadAuxiliaryLibraryCallback(process *, void *data, void *ret);
  
  // Sets the parameters to paradynInit
  bool setParadynLibParams();
  // And associated callback function
  static void setParadynLibParamsCallback(process *p, string libname, void *data);

  // Handles final initialization
  bool finalizeParadynLib();
  // For when we can't call paradynInit from _init method
  bool iRPCParadynInit();
  static void paradynInitCompletionCallback(process *, void *data, void *ret);
  
  bool extractBootstrapStruct(PARADYN_bootstrapStruct *bs_record);

  bool getParadynRTname();

  /*************************************************************
   **** Process state variables                             ****
   *************************************************************/
  private:
  
  bool wasCreated;
  bool wasAttached;
  bool wasForked;
  bool wasExeced;
  bool inExec;

  Address sharedMetaDataOffset;

  string paradynRTname;
};


// Shouldn't these be static members of class pd_process?
pd_process *pd_createProcess(pdvector<string> &argv, pdvector<string> &envp, string dir);
pd_process *pd_attachProcess(const string &progpath, int pid);

#endif













