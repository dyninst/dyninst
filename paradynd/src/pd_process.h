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

#include "dyninstAPI/src/process.h"
#include "paradynd/src/threadMgr.h"
#include "paradynd/src/timeMgr.h"

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

   bool cancelRPC(unsigned rpc_id) {
      return dyninst_process->getRpcMgr()->cancelRPC(rpc_id);
   }

   shmMgr *getSharedMemMgr() {  return dyninst_process->getSharedMemMgr();  }

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
       return dyninst_process->getRpcMgr()->launchRPCs(wasRunning);
   }

   bool isPARADYNBootstrappedYet() const {
       // Good enough approximation (should use a flag here)
       return reachedLibState(paradynRTState, libReady);       
   }

   bool catchupSideEffect(Frame &frame, instReqNode *inst) {
      return dyninst_process->catchupSideEffect(frame, inst);
   }

   int getTraceLink() { return dyninst_process->traceLink; }
   void setTraceLink(int v) { dyninst_process->traceLink = v; }
   dyn_lwp *getDefaultLWP() const {
      return dyninst_process->getDefaultLWP();
   }

   unsigned postRPCtoDo(AstNode *action, bool noCost,
                        inferiorRPCcallbackFunc callbackFunc,
                        void *userData, bool lowmem,
                        dyn_thread *thr, dyn_lwp *lwp, Address aixHACK = 0) {
       return dyninst_process->getRpcMgr()->postRPCtoDo(action, noCost,
                                                        callbackFunc, userData,
                                                        lowmem,
                                                        thr, lwp, aixHACK);
   }
   
   bool triggeredInStackFrame(instPoint* point, Frame frame,
                              pd_Function *&func,
                              callWhen when, callOrder order) {
      return dyninst_process->triggeredInStackFrame(point, frame, func, when, order);
   }
   
   bool walkStacks(pdvector<pdvector<Frame> > &stackWalks) {
       return dyninst_process->walkStacks(stackWalks);
   }
   
   function_base *findOnlyOneFunction(resource *func, resource *mod) {
      return dyninst_process->findOnlyOneFunction(func, mod);
   }

   function_base *findOnlyOneFunction(const string &func_name) const {
      return dyninst_process->findOnlyOneFunction(func_name);
   }

   bool findAllFuncsByName(const string &func_name, pdvector<function_base *> &res) {
     return dyninst_process->findAllFuncsByName(func_name, res);
   }
   bool findAllFuncsByName(resource *func, resource *mod, pdvector<function_base *> &res) {
     return dyninst_process->findAllFuncsByName(func, mod, res);
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

   virtualTimer *getVirtualTimer(int pos) {
      return dyninst_process->getVirtualTimer(pos);
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

   void initAfterFork(pd_process *parentProc);
      
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
  bool loadAuxiliaryLibrary(string libname);
  static void loadAuxiliaryLibraryCallback(process *, unsigned /* rpc_id */,
                                           void *data, void *ret);
  
  // Sets the parameters to paradynInit
  bool setParadynLibParams(load_cause_t ldcause);
  // And associated callback function
  static void setParadynLibParamsCallback(process *p, string libname, 
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

  string paradynRTname;
};


// Shouldn't these be static members of class pd_process?
pd_process *pd_createProcess(pdvector<string> &argv, pdvector<string> &envp, string dir);
pd_process *pd_attachProcess(const string &progpath, int pid);

#endif













