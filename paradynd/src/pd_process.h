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

// $Id: pd_process.h,v

// Put Paradyn specific process code in the object, not in
// dyninstAPI/src/process.[hC]

#ifndef __PD_PROCESS__
#define __PD_PROCESS__

#include "dyninstAPI/h/BPatch_thread.h"
#include "paradynd/src/threadMgr.h"
#include "paradynd/src/timeMgr.h"
#include "dyninstAPI/src/process.h"

class pd_image;

// Someday this class will inherit from BPatch_thread instead of
// having a process* member

class pd_process {
   BPatch_thread *dyninst_process;

   threadMgr thr_mgr;

   variableMgr *theVariableMgr;

   pd_image *img;
   resource *rid;

 public:
   // Paradyn daemon arguments, etc.
   static pdvector<pdstring> arg_list; // the arguments of paradynd
   // Paradyn RT name if set on the command line
   static pdstring defaultParadynRTname;

   unsigned numOfActCounters_is;
   unsigned numOfActProcTimers_is;
   unsigned numOfActWallTimers_is;
   
 public:
   // Creation constructor
   pd_process(const pdstring argv0, pdvector<pdstring> &argv,
              const pdstring dir, int stdin_fd, int stdout_fd, int stderr_fd);

   // Attach constructor
   pd_process(const pdstring &progpath, int pid);
  
   // fork constructor
   pd_process(const pd_process &parent, BPatch_thread *childDynProc);
   
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

   // handle to resource for this process
   resource *get_rid() { return rid; }

   void handleExit(int exitStatus);

   void FillInCallGraphStatic();
   void MonitorDynamicCallSites(pdstring function_name);

  // ==== Cpu time related functions and members =======================

  // called by process object constructor
  void initCpuTimeMgr();
  // called by initCpuTimeMgr, sets up platform specific aspects of cpuTimeMgr
  void initCpuTimeMgrPlt();

  // Call getCpuTime to get the current cpu time of process. Time conversion
  // from raw to primitive time units is done in relevant functions by using
  // the units ratio as defined in the cpuTimeMgr.  getCpuTime and getRawTime
  // use the best level as determined by the cpuTimeMgr.
  timeStamp getCpuTime(int lwp);
  timeStamp units2timeStamp(int64_t rawunits);
  timeLength units2timeLength(int64_t rawunits);
  rawTime64 getRawCpuTime(int lwp);

  // Verifies that the wall and cpu timer levels chosen by the daemon are
  // also available within the rtinst library.  This is an issue because the
  // daemon chooses the wall and cpu timer levels to use at daemon startup
  // and process object initialization respectively.  There is an outside
  // chance that the level would be determined unavailable by the rtinst
  // library upon application startup.  Asserts if there is a mismatch.
  void verifyTimerLevels();
  // Sets the wall and cpu time retrieval functions to use in the the rtinst
  // library by setting a function ptr in the rtinst library to the address
  // of the chosen function.
  void writeTimerLevels();
  private:
  // helper routines for writeTimerLevels
  void writeTimerFuncAddr(const char *rtinstVar, const char *rtinstHelperFPtr);
  bool writeTimerFuncAddr_(const char *rtinstVar,const char *rtinstHelperFPtr);

  // returns the address to assign to a function pointer that will
  // allow the time querying function to be called (in the rtinst library)
  // on AIX, this address returned will be the address of a structure which 
  //   has a field that points to the proper querying function (function 
  //   pointers are handled differently on AIX)
  // on other platforms, this address will be the address of the time
  // querying function in the rtinst library
  Address getTimerQueryFuncTransferAddress(const char *helperFPtr);

  // handles setting time retrieval functions for the case of a 64bit daemon
  // and 32bit application
  // see process.C definition for why being disabled
  //bool writeTimerFuncAddr_Force32(const char *rtinstVar, 
  //			  const char *rtinstFunc);

 private:
  // Platform dependent (ie. define in platform files) process time retrieval
  // function for daemon.  Use process::getCpuTime instead of calling these
  // functions directly.  If platform doesn't implement particular level,
  // still need to define a definition (albeit empty).  Ignores lwp arg if
  // lwp's are irrelevant for platform. The file descriptor argument "fd"
  // is used.

  // NOTE: It is the caller's responsibility to check for rollbacks!

  rawTime64 getRawCpuTime_hw(int lwp);
  rawTime64 getRawCpuTime_sw(int lwp);

  // function always returns true, used when timer level is always available
  bool yesAvail();
  // The process time time mgr.  This handles choosing the best timer level
  // to use.  Call getTime member with a process object and an integer lwp
  // as args.
  typedef timeMgr<pd_process, int> cpuTimeMgr_t;
  cpuTimeMgr_t *cpuTimeMgr;

#if defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4)
  bool isPapiAvail();           
#endif
#ifdef rs6000_ibm_aix4_1
  bool isPmapiAvail();
#endif
  public:
  
   // ========  PASS THROUGH FUNCTIONS ===============================

   BPatch_thread *get_dyn_process() {
      return dyninst_process;
   }

   processState status() const {
      return dyninst_process->lowlevel_process()->status(); }

   bool continueProc() {
      return dyninst_process->lowlevel_process()->continueProc();
   }
   bool pause() {
      return dyninst_process->lowlevel_process()->pause();
   }

   void continueAfterNextStop() {
      dyninst_process->lowlevel_process()->continueAfterNextStop();
   }

   bool detach(const bool paused) { // why the param?
      return dyninst_process->lowlevel_process()->detach(paused);
   }

   int getPid() const {
      return dyninst_process->lowlevel_process()->getPid();
   }

   bool cancelRPC(unsigned rpc_id) {
    return dyninst_process->lowlevel_process()->getRpcMgr()->cancelRPC(rpc_id);
   }

   shmMgr *getSharedMemMgr() {
      return dyninst_process->lowlevel_process()->getSharedMemMgr();
   }
   
   Address initSharedMetaData() {
      return dyninst_process->lowlevel_process()->initSharedMetaData();
   }

   bool findInternalSymbol(const pdstring &name, bool warn,
                           internalSym &ret_sym) const {
      return dyninst_process->lowlevel_process()->findInternalSymbol(name, warn, ret_sym);
   }

   Address findInternalAddress(const pdstring &name, bool warn,
                               bool &err) const {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->findInternalAddress(name, warn, err);
   }

   bool writeDataSpace(void *inTracedProcess, u_int amount,
		       const void *inSelf) {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->writeDataSpace(inTracedProcess, amount, inSelf);
   }

   bool isBootstrappedYet() const {
      return dyninst_process->lowlevel_process()->isBootstrappedYet();
   }

   bool hasExited() {
      return dyninst_process->lowlevel_process()->hasExited();
   }

   bool wasCreatedViaAttach() { 
      return dyninst_process->lowlevel_process()->wasCreatedViaAttach();
   }

   bool launchRPCs(bool wasRunning) {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->getRpcMgr()->launchRPCs(wasRunning);
   }

   bool readDataSpace(const void *inTracedProcess, u_int amount,
                      void *inSelf, bool displayErrMsg) {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->readDataSpace(inTracedProcess, amount, inSelf,
                                   displayErrMsg);
   }

   bool writeTextSpace(void *inTracedProcess, u_int amount,
                       const void *inSelf) {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->writeTextSpace(inTracedProcess, amount, inSelf);
   }

   bool isPARADYNBootstrappedYet() const {
       // Good enough approximation (should use a flag here)
       return reachedLibState(paradynRTState, libReady);       
   }

   bool catchupSideEffect(Frame &frame, instReqNode *inst) {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->catchupSideEffect(frame, inst);
   }

   int getTraceLink() {
      return dyninst_process->lowlevel_process()->traceLink;
   }
   void setTraceLink(int v) {
      dyninst_process->lowlevel_process()->traceLink = v;
   }
   dyn_lwp *getProcessLWP() const {
      return dyninst_process->lowlevel_process()->getProcessLWP();
   }

   void installInstrRequests(const pdvector<instMapping*> &requests) {
      dyninst_process->lowlevel_process()->installInstrRequests(requests);
   }

   unsigned postRPCtoDo(AstNode *action, bool noCost,
                        inferiorRPCcallbackFunc callbackFunc,
                        void *userData, bool lowmem,
                        dyn_thread *thr, dyn_lwp *lwp, Address aixHACK = 0) {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->getRpcMgr()->postRPCtoDo(action, noCost,
                                            callbackFunc, userData,
                                            lowmem,
                                            thr, lwp, aixHACK);
   }
   
   bool triggeredInStackFrame(Frame frame, instPoint *point,
                              callWhen when, callOrder order) {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->triggeredInStackFrame(frame, point, when, order);
   }
   
   bool walkStacks(pdvector<pdvector<Frame> > &stackWalks) {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->walkStacks(stackWalks);
   }
   
   function_base *findOnlyOneFunction(resource *func, resource *mod) {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->findOnlyOneFunction(func, mod);
   }

   function_base *findOnlyOneFunction(const pdstring &func_name) const {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->findOnlyOneFunction(func_name);
   }

   pdvector<module *> *getAllModules() {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->getAllModules();
   }

   bool findAllFuncsByName(const pdstring &func_name,
                           pdvector<function_base *> &res) {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->findAllFuncsByName(func_name, res);
   }
   bool findAllFuncsByName(resource *func, resource *mod,
                           pdvector<function_base *> &res) {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->findAllFuncsByName(func, mod, res);
   }

   bool getSymbolInfo(const pdstring &n, Symbol &info,
                      Address &baseAddr) const {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->getSymbolInfo(n, info, baseAddr);
   }

   pd_image *getImage() const {
      return img;
   }

   bool findCallee(instPoint &instr, function_base *&target) {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->findCallee(instr, target);
   }

   pdvector<function_base *> *getIncludedFunctions(module *mod) {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->getIncludedFunctions(mod);
   }

   pdvector<function_base *> *getIncludedFunctions() {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->getIncludedFunctions();
   }

   pdvector<module *> *getIncludedModules() {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->getIncludedModules();
   }

   module *findModule(const pdstring &mod_name,bool check_excluded) {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->findModule(mod_name, check_excluded);
   }

   bool isDynamicallyLinked() { 
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->isDynamicallyLinked();
   }

   pdvector<shared_object *> *sharedObjects() {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->sharedObjects();
   }

   unsigned maxNumberOfThreads() {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->maxNumberOfThreads();
   }

   function_base *getMainFunction() const {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->getMainFunction();
   }

   bool dumpCore(const pdstring coreFile) {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->dumpCore(coreFile);
   }

   pdstring getProcessStatus() const {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->getProcessStatus();
   }

   virtualTimer *getVirtualTimer(int pos) {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->getVirtualTimer(pos);
   }

#ifdef PAPI
   papiMgr* getPapiMgr() {  return dyninst_process->getPapiMgr();  }
#endif

   bool multithread_capable() {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->multithread_capable();
   }
   bool multithread_ready() {
      process *llproc = dyninst_process->lowlevel_process();
      return llproc->multithread_ready();
   }

   bool wasRunningWhenAttached() { 
      return dyninst_process->lowlevel_process()->wasRunningWhenAttached();
   }

   pd_thread *STthread() { 
		assert(! multithread_capable());
      assert(numThr() > 0);
      return (*beginThr());
   }

   void initAfterFork(pd_process *parentProc);
      
  /************************************
   *** Fork and Exec handling       ***
   ************************************/
  public:
   static void paradynPreForkDispatch(BPatch_thread *parent,
                                      BPatch_thread *child);
   static void paradynPostForkDispatch(BPatch_thread *parent,
                                       BPatch_thread *child);
   static void paradynExecDispatch(BPatch_thread *thread);
   static void paradynExitDispatch(BPatch_thread *thread, int exitCode);

  private:
   void preForkHandler();
   void postForkHandler(BPatch_thread *child);
   void execHandler();
   void exitHandler();
   
  /************************************
   *** Runtime library functions    ***
   ************************************/
  public:
  // Returns once paradyn lib is loaded and initialized
   typedef enum { create_load, attach_load, exec_load } load_cause_t;
   bool loadParadynLib(load_cause_t ldcause);
  
  private:
   libraryState_t paradynRTState;
   libraryState_t auxLibState; // Needed for solaris
   
   // We load via an inferior RPC, so we need a callback
   static void paradynLibLoadCallback(process *, unsigned /* rpc_id */,
                                      void *data, void *ret);
   void setParadynLibLoaded() { setLibState(paradynRTState, libLoaded); }
   
   // Replace with BPatch::loadLibrary
   bool loadAuxiliaryLibrary(pdstring libname);
   static void loadAuxiliaryLibraryCallback(process *, unsigned /* rpc_id */,
                                            void *data, void *ret);
   
   // Sets the parameters to paradynInit
   bool setParadynLibParams(load_cause_t ldcause);
   // And associated callback function
   static void setParadynLibParamsCallback(process *p, pdstring libname, 
                                           shared_object *libobj, void *data);
   
   // Handles final initialization
   bool finalizeParadynLib();
   // For when we can't call paradynInit from _init method
   bool iRPCParadynInit();
   static void paradynInitCompletionCallback(process *, unsigned /* rpc_id */,
                                             void *data, void *ret);
   
   bool extractBootstrapStruct(PARADYN_bootstrapStruct *bs_record);
   
   bool getParadynRTname();
   
   /*************************************************************
    **** Process state variables                             ****
    *************************************************************/
 private:
   
   bool inExec;
  
   Address sharedMetaDataOffset;
  
   pdstring paradynRTname;
};


// Shouldn't these be static members of class pd_process?
pd_process *pd_createProcess(pdvector<pdstring> &argv, pdstring dir);
pd_process *pd_attachProcess(const pdstring &progpath, int pid);

#endif













