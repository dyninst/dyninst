/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: pd_process.C,v

#include "paradynd/src/pd_process.h"
#include "paradynd/src/pd_thread.h"
#include "dyninstAPI/src/signalhandler.h"
#include "paradynd/src/init.h"
#include "paradynd/src/metricFocusNode.h"
#include "paradynd/src/processMgr.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/pd_image.h"

#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/instPoint.h"



// Set in main.C
extern int termWin_port;
extern pdstring pd_machine;
extern PDSOCKET connect_Svr(pdstring machine,int port);
extern pdRPC *tp;

// Exec callback
extern void pd_execCallback(pd_process *proc);

pdvector<pdstring> pd_process::arg_list;
pdstring pd_process::defaultParadynRTname;

const pdstring nullString("");

extern resource *machineResource;

// Global "create a new pd_process object" functions

pd_process *pd_createProcess(pdvector<pdstring> &argv, pdstring dir) {
#if !defined(i386_unknown_nt4_0)
    if (termWin_port == -1)
        return NULL;
    
    PDSOCKET stdout_fd = INVALID_PDSOCKET;
    if ((stdout_fd = connect_Svr(pd_machine,termWin_port)) == INVALID_PDSOCKET)
        return NULL;
    if (write(stdout_fd,"from_app\n",strlen("from_app\n")) <= 0)
    {
        CLOSEPDSOCKET(stdout_fd);
        return NULL;
    }
#endif
    
	// NEW: We bump up batch mode here; the matching bump-down occurs after
	// shared objects are processed (after receiving the SIGSTOP indicating
	// the end of running DYNINSTinit; more specifically,
	// procStopFromDYNINSTinit().  Prevents a diabolical w/w deadlock on
	// solaris --ari
	tp->resourceBatchMode(true);
    
#if !defined(i386_unknown_nt4_0)
    pd_process *proc = new pd_process(argv[0], argv, dir, 0, stdout_fd, 2);
#else 
    pd_process *proc = new pd_process(argv[0], argv, dir, 0, 1, 2);
#endif
    if ( (proc == NULL) || (proc->get_dyn_process() == NULL) ) {
#if !defined(i386_unknown_nt4_0)
        CLOSEPDSOCKET(stdout_fd);
#endif
        return NULL;
    }
    // Load the paradyn runtime lib
    proc->loadParadynLib(pd_process::create_load);
    
    // Run necessary initialization
    proc->init();
    
    // Lower batch mode
    tp->resourceBatchMode(false); 
    
    process *llproc = proc->get_dyn_process()->lowlevel_process();
    if(!costMetric::addProcessToAll(llproc))
        assert(false);
    
    getProcMgr().addProcess(proc);

    pdstring buffer = pdstring("PID=") + pdstring(proc->getPid());
    buffer += pdstring(", ready");
    statusLine(buffer.c_str());
    
    return proc;
}

pd_process *pd_attachProcess(const pdstring &progpath, int pid) { 
    // Avoid deadlock
	tp->resourceBatchMode(true);

    pd_process *proc = new pd_process(progpath, pid);

    if (!proc || !proc->get_dyn_process()) return NULL;

    proc->loadParadynLib(pd_process::attach_load);
    proc->init();

    // Lower batch mode
    tp->resourceBatchMode(false);

    process *llproc = proc->get_dyn_process()->lowlevel_process();
    if (!costMetric::addProcessToAll(llproc))
       assert(false);

    getProcMgr().addProcess(proc);

    pdstring buffer = pdstring("PID=") + pdstring(proc->getPid());
    buffer += pdstring(", ready");
    statusLine(buffer.c_str());
	
    return proc;
}

void pd_process::init() {
    static bool has_mt_resource_heirarchies_been_defined = false;
    pdstring buffer = pdstring("PID=") + pdstring(getPid());
    buffer += pdstring(", initializing daemon-side data");
    statusLine(buffer.c_str());

    process *lowlevel_proc = get_dyn_process()->lowlevel_process();
    for(unsigned i=0; i<lowlevel_proc->threads.size(); i++) {
        pd_thread *thr = new pd_thread(lowlevel_proc->threads[i], this);
        addThread(thr);
    }
    
    theVariableMgr = new variableMgr(this, getSharedMemMgr(),
                                     maxNumberOfThreads());
    buffer = pdstring("PID=") + pdstring(getPid());
    buffer += pdstring(", posting call graph information");
    statusLine(buffer.c_str());

    if(multithread_capable() && !has_mt_resource_heirarchies_been_defined) {
       resource::newResource(syncRoot, NULL, nullString, "Mutex", 
                             timeStamp::ts1970(), "", MDL_T_STRING, false);
       resource::newResource(syncRoot, NULL, nullString, "RwLock", 
                             timeStamp::ts1970(), "", MDL_T_STRING, false);
       resource::newResource(syncRoot, NULL, nullString, "CondVar", 
                             timeStamp::ts1970(), "", MDL_T_STRING, false);
       has_mt_resource_heirarchies_been_defined = true;
    }
    
    FillInCallGraphStatic();
}

// Creation constructor
pd_process::pd_process(const pdstring argv0, pdvector<pdstring> &argv,
                       const pdstring dir, int stdin_fd, int stdout_fd,
                       int stderr_fd) 
        : numOfActCounters_is(0), numOfActProcTimers_is(0),
          numOfActWallTimers_is(0), 
          cpuTimeMgr(NULL),
#ifdef PAPI
          papi(NULL),
#endif
          paradynRTState(libUnloaded),
          inExec(false)
{
    if ((dir.length() > 0) && (P_chdir(dir.c_str()) < 0)) {
       sprintf(errorLine, "cannot chdir to '%s': %s\n", dir.c_str(), 
               sys_errlist[errno]);
       logLine(errorLine);
       P__exit(-1);
    }

    char **argv_array = new char*[argv.size()+1];
    for(unsigned i=0; i<argv.size(); i++)
       argv_array[i] = const_cast<char *>(argv[i].c_str());
    argv_array[argv.size()] = NULL;

    char *path = new char[  argv0.length() + 5];
    strcpy(path, argv0.c_str());
    char *ignore[1] = { NULL };
    getBPatch().setTypeChecking(false);
    dyninst_process = getBPatch().createProcess(path, argv_array, ignore,
                                               stdin_fd, stdout_fd, stderr_fd);

    delete []argv_array;
    delete path;

    process *llproc = dyninst_process->lowlevel_process();
    img = new pd_image(llproc->getImage(), this);

    pdstring buff = pdstring(getPid()); // + pdstring("_") + getHostName();
    rid = resource::newResource(machineResource, // parent
				(void*)this, // handle
				nullString, // abstraction
				img->name(), // process name
				timeStamp::ts1970(), // creation time
				buff, // unique name (?)
				MDL_T_STRING, // mdl type (?)
				true
				);
    
    if (!dyninst_process) {
        // Ummm.... 
        return;
    }

    initCpuTimeMgr();

    // Dyninst process create currently also builds and attaches
    // to the shared segment. That should be moved here. In the
    // meantime....
    sharedMetaDataOffset = llproc->initSharedMetaData();

    // Set the paradyn RT lib name
    if (!getParadynRTname())
        assert(0 && "Need to do cleanup");
}

// Attach constructor
pd_process::pd_process(const pdstring &progpath, int pid)
        : numOfActCounters_is(0), numOfActProcTimers_is(0),
          numOfActWallTimers_is(0), 
          cpuTimeMgr(NULL),
#ifdef PAPI
          papi(NULL),
#endif
          paradynRTState(libUnloaded),
          inExec(false)
{
    getBPatch().setTypeChecking(false);
    dyninst_process = getBPatch().attachProcess(progpath.c_str(), pid);
    
    process *llproc = dyninst_process->lowlevel_process();
    img = new pd_image(llproc->getImage(), this);

    pdstring buff = pdstring(getPid()); // + pdstring("_") + getHostName();
    rid = resource::newResource(machineResource, // parent
                                (void*)this, // handle
                                nullString, // abstraction
                                img->name(),
                                timeStamp::ts1970(), // creation time
                                buff, // unique name (?)
                                MDL_T_STRING, // mdl type (?)
                                true
                                );

    if (!dyninst_process) {
        // Ummm.... 
        return;
    }

    initCpuTimeMgr();

    // Dyninst process create currently also builds and attaches
    // to the shared segment. That should be moved here. In the
    // meantime....
    sharedMetaDataOffset = llproc->initSharedMetaData();
    // Set the paradyn RT lib name
    if (!getParadynRTname())
        assert(0 && "Need to do cleanup");
}

extern void CallGraphSetEntryFuncCallback(pdstring exe_name, pdstring r, int tid);


// fork constructor
pd_process::pd_process(const pd_process &parent, BPatch_thread *childDynProc) :
        dyninst_process(childDynProc), 
        cpuTimeMgr(NULL),
#ifdef PAPI
        papi(NULL),
#endif
        paradynRTState(libLoaded), inExec(false),
        paradynRTname(parent.paradynRTname)
{
   process *llproc = dyninst_process->lowlevel_process();
   img = new pd_image(llproc->getImage(), this);

   pdstring buff = pdstring(getPid()); // + pdstring("_") + getHostName();
   rid = resource::newResource(machineResource, // parent
                               (void*)this, // handle
                               nullString, // abstraction
                               img->name(),
                               timeStamp::ts1970(), // creation time
                               buff, // unique name (?)
                               MDL_T_STRING, // mdl type (?)
                               true
                               );

   setLibState(paradynRTState, libReady);
   for(unsigned i=0; i<llproc->threads.size(); i++) {
      pd_thread *pd_thr = new pd_thread(llproc->threads[i], this);
      thr_mgr.addThread(pd_thr);
      dyn_thread *thr = pd_thr->get_dyn_thread();

      if(! multithread_ready()) continue;
      // computing resource id
      pdstring buffer;
      pdstring pretty_name = pdstring(thr->get_start_func()->prettyName().c_str());
      buffer = pdstring("thr_") + pdstring(thr->get_tid()) + pdstring("{")
               + pretty_name + pdstring("}");
      resource *rid;
      rid = resource::newResource(get_rid(), (void *)thr, nullString, 
                                  buffer, timeStamp::ts1970(), "",
                                  MDL_T_STRING, true);
      pd_thr->update_rid(rid);
      // tell front-end about thread start function for newly created threads
      // We need the module, which could be anywhere (including a library)
      pd_Function *func = (pd_Function *)thr->get_start_func();
      pdmodule *foundMod = func->file();
      assert(foundMod != NULL);
      resource *modRes = foundMod->getResource();
      pdstring start_func_str = thr->get_start_func()->prettyName();
      pdstring res_string = modRes->full_name() + "/" + start_func_str;
      CallGraphSetEntryFuncCallback(getImage()->get_file(), res_string, 
                                    thr->get_tid());
   }

   theVariableMgr = new variableMgr(*parent.theVariableMgr, this,
                                    getSharedMemMgr());
   theVariableMgr->initializeVarsAfterFork();
}

pd_process::~pd_process() {
   cpuTimeMgr->destroyMechTimers(this);

   delete theVariableMgr;
   //delete dyninst_process;
}

bool pd_process::doMajorShmSample() {
   if( !isBootstrappedYet() || !isPARADYNBootstrappedYet()) {
      return false;
   }

   bool result = true; // will be set to false if any processAll() doesn't complete
                       // successfully.
   process *dyn_proc = get_dyn_process()->lowlevel_process();

   if(! getVariableMgr().doMajorSample())
      result = false;

   if(status() == exited) {
      return false;
   }

   // need to check this again, process could have execed doMajorSample
   // and it may be midway through setting up for the exec
   if( !isBootstrappedYet() || !isPARADYNBootstrappedYet()) {
      return false;
   }

   // inferiorProcessTimers used to take in a non-dummy process time as the
   // 2d arg, but it looks like that we need to re-read the process time for
   // each proc timer, at the time of sampling the timer's value, to avoid
   // ugly jagged spikes in histogram (i.e. to avoid incorrect sampled 
   // values).  Come to think of it: the same may have to be done for the 
   // wall time too!!!

   const timeStamp theProcTime = getCpuTime(0);
   const timeStamp curWallTime = getWallTime();

   // need to check this again, process could have execed doMajorSample
   // and it may be midway through setting up for the exec
   if( !isBootstrappedYet() || !isPARADYNBootstrappedYet()) {
      return false;
   }

   // Now sample the observed cost.
   unsigned *costAddr = (unsigned *)dyn_proc->getObsCostLowAddrInParadyndSpace();
   const unsigned theCost = *costAddr; // WARNING: shouldn't we be using a mutex?!

   dyn_proc->processCost(theCost, curWallTime, theProcTime);

   return result;
}

bool pd_process::doMinorShmSample() {
   // Returns true if the minor sample has successfully completed all
   // outstanding samplings.
   bool result = true; // so far...
   
   if(! getVariableMgr().doMinorSample())
      result = false;

   return result;
}

extern pdRPC *tp;
extern unsigned activeProcesses; // process.C (same as processVec.size())
extern void disableAllInternalMetrics();

void pd_process::handleExit(int exitStatus) {
   // The below vector is a kludge put in as a "safer" method of handling
   // this issue just before the release.  After the release, this method
   // shouldn't be used.  We're using this to delete the pd_process when
   // Paradyn exits.  The better appoach would be to delete the pd_process
   // when the process actually exits, as we find out in
   // paradyn_handleProcessExit.  We don't have opportunity so close to the
   // release to flush out any bugs related to this, so that's why we're
   // deleting all pd_processes when Paradyn exits.
   getProcMgr().exitedProcess(this);

   // don't do a final sample for terminated processes
   // this is because there could still be active process timers
   // we can't get a current process time since the process no longer
   // exists, so can't sample these active process timers   
   if(exitStatus == 0) {
      doMajorShmSample();
   }

   reportInternalMetrics(true);

   // close down the trace stream:
   if(getTraceLink() >= 0) {
      //processTraceStream(proc); // can't do since it's a blocking read 
                                  // (deadlock)
      P_close(getTraceLink());      
      setTraceLink(-1);
   }
   metricFocusNode::handleExitedProcess(this);

   if(multithread_ready()) {
      // retire any thread resources which haven't been retired yet
      threadMgr::thrIter itr = beginThr();
      while(itr != endThrMark()) {
         pd_thread *thr = *itr;
         itr++;
         assert(thr->get_rid() != NULL);
         tp->retiredResource(thr->get_rid()->full_name());
      }
   }

   assert(get_rid() != NULL);
   tp->retiredResource(get_rid()->full_name());
   tp->processStatus(getPid(), procExited);

   if (activeProcesses == 0)
      disableAllInternalMetrics();
}

void pd_process::initAfterFork(pd_process * /*parentProc*/) {
   initCpuTimeMgr();

   tp->newProgramCallbackFunc(getPid(), arg_list, 
                              machineResource->part_name(),
                              false, wasRunningWhenAttached());
}

/********************************************************************
 **** Fork/Exec handling code                                    ****
 ********************************************************************/

void pd_process::paradynPreForkDispatch(BPatch_thread *parent,
                                        BPatch_thread * /*child*/) {
   pd_process *matching_pd_process = getProcMgr().find_pd_process(parent);
   if(matching_pd_process)
      matching_pd_process->preForkHandler();
}

void pd_process::paradynPostForkDispatch(BPatch_thread *parent,
                                         BPatch_thread *child) {
   pd_process *matching_pd_process = getProcMgr().find_pd_process(parent);
   if(matching_pd_process)
      matching_pd_process->postForkHandler(child);
}

void pd_process::paradynExecDispatch(BPatch_thread *dyn_proc) {
   pd_process *matching_pd_process = getProcMgr().find_pd_process(dyn_proc);
   if(matching_pd_process) {
      matching_pd_process->execHandler();
   }
}

void pd_process::paradynExitDispatch(BPatch_thread *thread, int exitCode) {
   pd_process *matching_pd_process = getProcMgr().find_pd_process(thread);
   if(matching_pd_process)
      matching_pd_process->handleExit(exitCode);
}

void pd_process::preForkHandler() {
}

extern void MT_lwp_setup(process *parentDynProc, process *childDynProc);

// this is the parent
void pd_process::postForkHandler(BPatch_thread *child) {
   BPatch_thread *parent = dyninst_process;
   process *parentDynProc = parent->lowlevel_process();
   process *childDynProc  = child->lowlevel_process();
   assert(childDynProc->status() == stopped);

   if(childDynProc->multithread_capable())
      MT_lwp_setup(parentDynProc, childDynProc);

   childDynProc->setParadynBootstrap();
   assert(childDynProc->status() == stopped);

   pd_process *parentProc = 
      getProcMgr().find_pd_process(parentDynProc->getPid());
   if (!parentProc) {
      logLine("Error in forkProcess: could not find parent process\n");
      return;
   }

   pd_process *childProc = new pd_process(*parentProc, child);
   getProcMgr().addProcess(childProc);

   childProc->initAfterFork(parentProc);
   metricFocusNode::handleFork(parentProc, childProc);

   if (childProc->status() == stopped)
       childProc->continueProc();
   // parent process will get continued by unix.C/handleSyscallExit
}

void pd_process::execHandler() {
    // We need to reload the Paradyn library
    paradynRTState = libUnloaded; // It was removed when we execed
    inExec = true;

    // create a new pd_image because there is a new dyninst image
    delete img;
    process *llproc = dyninst_process->lowlevel_process();
    img = new pd_image(llproc->getImage(), this);

    // Renew the metadata: it's been scribbled on
    sharedMetaDataOffset = initSharedMetaData();
    loadParadynLib(exec_load);
}

/********************************************************************
 **** Paradyn runtime library code                               ****    
 ********************************************************************/

typedef struct {
   pd_process *proc;
   pd_process::load_cause_t load_cause;
} loadLibCallbackInfo_t;

// Load and initialize the paradyn runtime library.

bool pd_process::loadParadynLib(load_cause_t ldcause) {
    // Force a load of the paradyn runtime library. We use
    // the dyninst runtime DYNINSTloadLibrary call, as in
    // BPatch_thread::loadLibrary

    process *llproc = dyninst_process->lowlevel_process();
    assert(status() == stopped);

    pdstring buffer = pdstring("PID=") + pdstring(getPid());
    buffer += pdstring(", loading Paradyn RT lib via iRPC");       
    statusLine(buffer.c_str());

    loadLibCallbackInfo_t *cbInfo = new loadLibCallbackInfo_t;
    cbInfo->proc = this;
    cbInfo->load_cause = ldcause;

#if defined(i386_unknown_nt4_0)
    // Another FIXME: NT strips the path from the loaded
    // library for recognition purposes. 
    char dllFilename[_MAX_FNAME];
    _splitpath (paradynRTname.c_str(),
                NULL, NULL, dllFilename, NULL);
    // Set the callback to run setParams
    llproc->registerLoadLibraryCallback(pdstring(dllFilename),
                                        setParadynLibParamsCallback,
                                        (void *)cbInfo);
#else
    llproc->registerLoadLibraryCallback(paradynRTname,
                                        setParadynLibParamsCallback,
                                        (void *)cbInfo);
#endif
#if defined(sparc_sun_solaris2_4)
    // Sparc requires libsocket to be loaded before the paradyn lib can be :/
    loadAuxiliaryLibrary("libsocket.so");
#endif

    pdvector<AstNode*> loadLibAstArgs(1);
    loadLibAstArgs[0] = new AstNode(AstNode::ConstantString, 
          reinterpret_cast<void *>(const_cast<char *>(paradynRTname.c_str())));
    AstNode *loadLib = new AstNode("DYNINSTloadLibrary", loadLibAstArgs);
    removeAst(loadLibAstArgs[0]);

    // We've built a call to loadLibrary, now run it via inferior RPC
    postRPCtoDo(loadLib, true, // Don't update cost
                pd_process::paradynLibLoadCallback,
                (void *)this, // User data
                false,
                NULL, NULL); // Not metric definition

    setLibState(paradynRTState, libLoading);
    // .. run RPC

    // We block on paradynRTState, which is set to libLoaded
    // via the inferior RPC callback
    while (!reachedLibState(paradynRTState, libLoaded)) {
        if(hasExited()) return false;
        launchRPCs(false);
        
        decodeAndHandleProcessEvent(true);
    }
    removeAst(loadLib);

    // Unregister callback now that the library is loaded
    llproc->unregisterLoadLibraryCallback(paradynRTname);

    buffer = pdstring("PID=") + pdstring(getPid());
    buffer += pdstring(", finalizing Paradyn RT lib");       
    statusLine(buffer.c_str());

    // Now call finalizeParadynLib which will handle any initialization
    if (!finalizeParadynLib()) {
        buffer = pdstring("PID=") + pdstring(getPid());
        buffer += pdstring(", finalizing Paradyn RT lib via iRPC");       
        statusLine(buffer.c_str());
        // Paradyn init probably hasn't run, so trigger it with an
        // inferior RPC
        iRPCParadynInit();
    }
    assert(reachedLibState(paradynRTState, libReady));

    return true;
}

void pd_process::paradynLibLoadCallback(process * /*p*/, unsigned /* rpc_id */,
                                        void *data, void * /*ret*/)
{
    // Paradyn library has been loaded (via inferior RPC).
    pd_process *pd_p = (pd_process *)data;
    pd_p->setParadynLibLoaded();
}

// Set the parameters to the RT lib via address space writes
// Mimics the argument creation in iRPC... below
bool pd_process::setParadynLibParams(load_cause_t ldcause)
{
    // Now we write these variables into the following global vrbles
    // in the dyninst library:
    
    /*
      int libparadynRT_init_localparadynPid=-1;
      int libparadynRT_init_localCreationMethod=-1;
      int libparadynRT_init_localmaxThreads=-1;
      int libparadynRT_init_localtheKey=-1;
      int libparadynRT_init_localshmSegNumBytes=-1;
      int libparadynRT_init_localoffset=-1;
    */

    Symbol sym;
    Address baseAddr;
    // TODO: this shouldn't be paradynPid, it should be traceConnectInfo
    int paradynPid = traceConnectInfo;
    // UNIX: the daemon's PID. NT: a socket.

    if (!getSymbolInfo("libparadynRT_init_localparadynPid", sym, baseAddr))
        if (!getSymbolInfo("_libparadynRT_init_localparadynPid", sym, baseAddr))
            assert(0 && "Could not find symbol libparadynRT_init_localparadynPid");
    assert(sym.type() != Symbol::PDST_FUNCTION);
    writeDataSpace((void*)(sym.addr() + baseAddr), sizeof(int), (void *)&paradynPid);
    //fprintf(stderr, "Set localParadynPid to %d\n", paradynPid);

    int creationMethod;
    if(ldcause == create_load)      creationMethod = 0;
    else if(ldcause == attach_load) creationMethod = 1;
    else if(ldcause == exec_load)   creationMethod = 4;
    else assert(0);

    if (!getSymbolInfo("libparadynRT_init_localcreationMethod", sym, baseAddr))
        if (!getSymbolInfo("_libparadynRT_init_localcreationMethod", sym, baseAddr))
            assert(0 && "Could not find symbol libparadynRT_init_localcreationMethod");
    assert(sym.type() != Symbol::PDST_FUNCTION);
    writeDataSpace((void*)(sym.addr() + baseAddr), sizeof(int), (void *)&creationMethod);
    //fprintf(stderr, "Set localCreationMethod to %d\n", creationMethod);

    int maxThreads = maxNumberOfThreads();
    if (!getSymbolInfo("libparadynRT_init_localmaxThreads", sym, baseAddr))
        if (!getSymbolInfo("_libparadynRT_init_localmaxThreads", sym, baseAddr))
            assert(0 && "Could not find symbol libparadynRT_init_localmaxThreads");
    assert(sym.type() != Symbol::PDST_FUNCTION);
    writeDataSpace((void*)(sym.addr() + baseAddr), sizeof(int), (void *)&maxThreads);
    //fprintf(stderr, "Set localMaxThreads to %d\n", maxThreads);
    
    process *llproc = dyninst_process->lowlevel_process();
    int theKey = llproc->getShmKeyUsed();
    if (!getSymbolInfo("libparadynRT_init_localtheKey", sym, baseAddr))
        if (!getSymbolInfo("_libparadynRT_init_localtheKey", sym, baseAddr))
            assert(0 && "Could not find symbol libparadynRT_init_localtheKey");
    assert(sym.type() != Symbol::PDST_FUNCTION);
    writeDataSpace((void*)(sym.addr() + baseAddr), sizeof(int), (void *)&theKey);
    //fprintf(stderr, "Set localTheKey to %d\n", theKey);


    int shmSegNumBytes = llproc->getShmHeapTotalNumBytes();
    if (!getSymbolInfo("libparadynRT_init_localshmSegNumBytes", sym, baseAddr))
        if (!getSymbolInfo("_libparadynRT_init_localshmSegNumBytes", sym, baseAddr))
            assert(0 && "Could not find symbol libparadynRT_init_localshmSegNumBytes");
    assert(sym.type() != Symbol::PDST_FUNCTION);
    writeDataSpace((void*)(sym.addr() + baseAddr), sizeof(int), (void *)&shmSegNumBytes);
    //fprintf(stderr, "Set localShmSegNumBytes to %d\n", shmSegNumBytes);


    int offset = (int) sharedMetaDataOffset;
    if (!getSymbolInfo("libparadynRT_init_localoffset", sym, baseAddr))
        if (!getSymbolInfo("_libparadynRT_init_localoffset", sym, baseAddr))
            assert(0 && "Could not find symbol libparadynRT_init_localoffset");
    assert(sym.type() != Symbol::PDST_FUNCTION);
    writeDataSpace((void*)(sym.addr() + baseAddr), sizeof(int), (void *)&offset);
    //fprintf(stderr, "Set localOffset to %d\n", offset);

    return true;
}

// Callback from dyninst's loadLibrary callbacks
void pd_process::setParadynLibParamsCallback(process* /*ignored*/,
                                             pdstring /*ignored*/,
                                             shared_object * /*libobj*/,
                                             void *data) {
   loadLibCallbackInfo_t *info = (loadLibCallbackInfo_t *)data;
   pd_process *p = info->proc;
   load_cause_t ldcause = info->load_cause;
   p->setParadynLibParams(ldcause);
   delete info;
}

bool pd_process::finalizeParadynLib() {
    // Check to see if paradyn init has been run, and if
    // not run it manually via inferior RPC
    pdstring str;

    if (reachedLibState(paradynRTState, libReady))
        return true;
    
    PARADYN_bootstrapStruct bs_record;
    if (!extractBootstrapStruct(&bs_record)){
        assert(false);
    }
    
    // Read the structure; if event 0 then it's undefined! (not yet written)
    if (bs_record.event == 0) return false;

    assert(bs_record.pid == getPid());
    
    const bool calledFromFork   = (bs_record.event == 2);
    const bool calledFromExec   = (bs_record.event == 4);
    //const bool calledFromAttach = (bs_record.event == 3);

    // Override tramp guard address
    process *llproc = dyninst_process->lowlevel_process();
    llproc->setTrampGuardAddr((Address) bs_record.tramp_guard_base);
    llproc->registerInferiorAttachedSegs(bs_record.appl_attachedAtPtr.ptr);

    if (!calledFromFork) {
        // MT: need to set Paradyn's bootstrap state or the instrumentation
        // basetramps will be created in ST mode
        llproc->setParadynBootstrap();

        // Install initial instrumentation requests
        extern pdvector<instMapping*> initialRequestsPARADYN; // init.C //ccw 18 apr 2002 : SPLIT
        installInstrRequests(initialRequestsPARADYN); 
        str=pdstring("PID=") + pdstring(bs_record.pid) + ", propagating mi's...";
        statusLine(str.c_str());
    }

    if (calledFromExec) {
        pd_execCallback(this);
    }

    str=pdstring("PID=") + pdstring(bs_record.pid) + ", executing new-prog callback...";
    statusLine(str.c_str());
    
    timeStamp currWallTime = calledFromExec ? timeStamp::ts1970():getWallTime();
    if (!calledFromExec && !calledFromFork) {
        // The following must be done before any samples are sent to
        // paradyn; otherwise, prepare for an assert fail.
        
        if (!isInitFirstRecordTime())
            setFirstRecordTime(currWallTime);
    }
    assert(status() == stopped);
    
    tp->newProgramCallbackFunc(bs_record.pid, arg_list, 
                               machineResource->part_name(),
                               calledFromExec,
                               wasRunningWhenAttached());
    // in paradyn, this will call paradynDaemon::addRunningProgram().
    // If the state of the application as a whole is 'running' paradyn will
    // soon issue an igen call to us that'll continue this process.
    if (!calledFromExec) {
        tp->setDaemonStartTime(getPid(), currWallTime.getD(timeUnit::sec(), 
                                                           timeBase::bStd()));
    }
    // verify that the wall and cpu timer levels chosen by the daemon
    // are available in the rt library
    verifyTimerLevels();
    writeTimerLevels();
    
    // Set library state to "ready"
    setLibState(paradynRTState, libReady);

    // Add callbacks for events we care about
    /*
      dyninst_process->registerPreForkCallback(paradynPreForkDispatch);
    */
    getBPatch().registerExecCallback(paradynExecDispatch);
    getBPatch().registerPostForkCallback(paradynPostForkDispatch);
    getBPatch().registerExitCallback(paradynExitDispatch);

    return true;
}



// Run paradyn init via an inferior RPC
bool pd_process::iRPCParadynInit() {
   pdvector<AstNode *> the_args;

    // We're using the set-globals-and-call-wrapper method now,
    // to get around problems with argument list size and
    // instrumentation generation.
/*    
    // Argument 1: Paradyd PID (or socket ID on NT)
    the_args[0] = new AstNode(AstNode::Constant, (void *)traceConnectInfo); // Global var set in perfStream.C

    // Argument 2: creation method (created/attached/forked/other)
    // need to switch from wasForked, wasCreated, and wasAttached  to
    // using ldcause, as in pd_process::loadParadynLib(load_cause_t ldcause)
    if (wasForked) // Forked overrides (?)
        the_args[1] = new AstNode(AstNode::Constant, (void *)2);
    else if (wasCreated) 
        the_args[1] = new AstNode(AstNode::Constant, (void *)0);
    else if (wasAttached)
        the_args[1] = new AstNode(AstNode::Constant, (void *)1);

    // Argument 3: number of threads to support
    the_args[2] = new AstNode(AstNode::Constant, (void *)maxNumberOfThreads());
    
    // Argument 4: shared segment key
    the_args[3] = new AstNode(AstNode::Constant, (void *)dyninst_process->getShmKeyUsed());
    
    // Argument 5: shared segment size
    the_args[4] = new AstNode(AstNode::Constant, (void *)dyninst_process->getShmHeapTotalNumBytes());
    
    // Argument 6: Offset to the metadata in the shared segment
    // FIXME: initialization should take place elsewhere
    the_args[5] = new AstNode(AstNode::Constant, (void *)sharedMetaDataOffset);
*/  

    // That's a lot of arguments. Now construct the call to paradyn init
    AstNode *paradyn_init = new AstNode("libparadynRT_init", the_args);
    postRPCtoDo(paradyn_init, false, // noCost
                pd_process::paradynInitCompletionCallback,
                (void *)this, // User data
                false,
                NULL, NULL); // Not metric definition

    // And force a flush...
    
    while(!reachedLibState(paradynRTState, libReady)) {
       if(hasExited()) return false;
        launchRPCs(false);
        decodeAndHandleProcessEvent(true);
    }
    return true;
}

void pd_process::paradynInitCompletionCallback(process* /*p*/,
                                               unsigned /* rpc_id */,
                                               void* data, 
                                               void* /*ret*/) {
    // Run finalizeParadynLib on the appropriate 
    // pd_process class
    pd_process *pd_p = (pd_process *) data;
    pd_p->finalizeParadynLib();
}

bool pd_process::extractBootstrapStruct(PARADYN_bootstrapStruct *bs_record)
{
    const pdstring vrbleName = "PARADYN_bootstrap_info";
    internalSym sym;
    bool flag = findInternalSymbol(vrbleName, true, sym);
    assert(flag);
    Address symAddr = sym.getAddr();
    // bulk read of bootstrap structure
    
    if (! readDataSpace((const void*)symAddr, sizeof(*bs_record), 
                        bs_record, true)) {
        cerr << "extractBootstrapStruct failed because readDataSpace failed" 
             << endl;
        return false;
    }
    
    // address-in-memory: re-read pointer field with proper alignment
    // (see rtinst/h/trace.h)
    assert(sizeof(int64_t) == 8); // sanity check
    assert(sizeof(int32_t) == 4); // sanity check
    
    // read pointer size
    int32_t ptr_size;
    internalSym sym2;
    bool ret2;
    ret2 = findInternalSymbol("PARADYN_attachPtrSize", true, sym2);
    if (!ret2) return false;
    ret2 = readDataSpace((void *)sym2.getAddr(), sizeof(int32_t), 
                         &ptr_size, true);
    if (!ret2) return false;
    // problem scenario: 64-bit application, 32-bit paradynd
    assert((size_t)ptr_size <= sizeof(bs_record->appl_attachedAtPtr.ptr));
    
    // re-align pointer if necessary
    if ((size_t)ptr_size < sizeof(bs_record->appl_attachedAtPtr.ptr)) {
        // assumption: 32-bit application, 64-bit paradynd
        printf(" ERROR %d != %d \n", ptr_size, (int) sizeof(int32_t));  //ccw 5 jun 2002 SPLIT
        assert(ptr_size == sizeof(int32_t));
        assert(sizeof(bs_record->appl_attachedAtPtr.ptr) == sizeof(int64_t));
        assert(sizeof(bs_record->appl_attachedAtPtr.words.hi) == sizeof(int32_t));
        // read 32-bit pointer from high word
        Address val_a = (unsigned)bs_record->appl_attachedAtPtr.words.hi;
        void *val_p = (void *)val_a;
        bs_record->appl_attachedAtPtr.ptr = val_p;
        fprintf(stderr, "    %p ptr *\n", bs_record->appl_attachedAtPtr.ptr);
    }
    
    return true;
}

// There is no reason why we need to use the same library for all
// inferior processes (MT vs ST)
bool pd_process::getParadynRTname() {
    
    // Replace with better test for MT-ness
   char ParadynEnvVar[20];
   if(multithread_capable())      
      strcpy(ParadynEnvVar, "PARADYN_LIB_MT");
   else
      strcpy(ParadynEnvVar, "PARADYN_LIB");
    
    // If there is a default set, use it
    if (defaultParadynRTname.length())
        paradynRTname = defaultParadynRTname;
    else {
        // check the environment variable
        if (getenv(ParadynEnvVar) != NULL) {
            paradynRTname = getenv(ParadynEnvVar);
        } else {
            pdstring msg = pdstring("Environment variable " + pdstring(ParadynEnvVar)
                                + " has not been defined for process "
                                + pdstring(getPid()));
            showErrorCallback(101, msg);
            cerr << "Environment variable " << ParadynEnvVar << " not set!" << endl;
            return false;
        }
    }

#if !defined(i386_unknown_nt4_0)
    // TODO: make equivalent for NT
    // Check to see if the library given exists.
    if (access(paradynRTname.c_str(), R_OK)) {
        pdstring msg = pdstring("Runtime library ") + paradynRTname
        + pdstring(" does not exist or cannot be accessed!");
        showErrorCallback(101, msg);
        cerr << "Paradyn lib is not accessible!" << endl;
        return false;
    }
#endif
    return true;
}


bool pd_process::loadAuxiliaryLibrary(pdstring libname) {
    auxLibState = libUnloaded;

    pdvector<AstNode*> loadLibAstArgs(1);
    loadLibAstArgs[0] = new AstNode(AstNode::ConstantString, 
                reinterpret_cast<void *>(const_cast<char *>(libname.c_str())));
    AstNode *loadLib = new AstNode("DYNINSTloadLibrary", loadLibAstArgs);
    removeAst(loadLibAstArgs[0]);

    // We've built a call to loadLibrary, now run it via inferior RPC
    postRPCtoDo(loadLib, true, // Don't update cost
                pd_process::loadAuxiliaryLibraryCallback,
                (void *)this, // User data
                false,
                NULL, NULL);

    setLibState(auxLibState, libLoading);
    // .. run RPC

    // We block on paradynRTState, which is set to libLoaded
    // via the inferior RPC callback
    while (!reachedLibState(auxLibState, libLoaded)) {
        if(hasExited()) return false;
        launchRPCs(false);
        decodeAndHandleProcessEvent(true);
    }
    removeAst(loadLib);
    return true;
}

void pd_process::loadAuxiliaryLibraryCallback(process* /*ignored*/,
                                              unsigned /* rpc_id */,
                                              void *data, void* /*ignored*/) {
    pd_process *p = (pd_process *)data;
    setLibState(p->auxLibState, libLoaded);
}

bool bForceSoftwareLevelCpuTimer() {
   char *pdkill;
   pdkill = getenv("PD_SOFTWARE_LEVEL_CPU_TIMER");
   if( pdkill )
      return true;
   else
      return false;
}

void pd_process::initCpuTimeMgr() {
   if(cpuTimeMgr != NULL)  delete cpuTimeMgr;
   cpuTimeMgr = new cpuTimeMgr_t();
   initCpuTimeMgrPlt();
   
   if(bForceSoftwareLevelCpuTimer()) {
      cpuTimeMgr_t::mech_t *tm =
         cpuTimeMgr->getMechLevel(cpuTimeMgr_t::LEVEL_TWO);
      cpuTimeMgr->installMechLevel(cpuTimeMgr_t::LEVEL_BEST, tm);    
      if(bShowTimerInfo())
         cerr << "Forcing to software level cpu timer\n";
   } else {
      cpuTimeMgr->determineBestLevels(this);
   }
   cpuTimeMgr_t::timeMechLevel ml = cpuTimeMgr->getBestLevel();
   //cerr << "Chosen cpu timer level: " << int(ml)+1 << "  "
   //     << *cpuTimeMgr->getMechLevel(ml)
   //     << "(timeBase is irrelevant for cpu time)\n\n";
   if(bShowTimerInfo()) {
      cerr << "Chosen cpu timer level: " << int(ml)+1 << "  "
           << *cpuTimeMgr->getMechLevel(ml)
           << "(timeBase is irrelevant for cpu time)\n\n";    
   }
}

timeStamp pd_process::getCpuTime(int lwp_id) {
   if(status() == exited) {
      return timeStamp::tsLongAgoTime();
   }
   
   return cpuTimeMgr->getTime(this, lwp_id, cpuTimeMgr_t::LEVEL_BEST);
   /* can nicely handle case when we allow exceptions
      } catch(LevelNotInstalled &) {
      cerr << "getCpuTime: timer level not installed\n";
      assert(0);
      }
   */
}

bool pd_process::yesAvail() {
   return true; 
}

rawTime64 pd_process::getRawCpuTime_hw(int lwp)
{
   return dyninst_process->lowlevel_process()->lwps[lwp]->getRawCpuTime_hw();
}

rawTime64 pd_process::getRawCpuTime_sw(int lwp)
{
   return dyninst_process->lowlevel_process()->lwps[lwp]->getRawCpuTime_sw();
}

rawTime64 pd_process::getRawCpuTime(int lwp) {
   return cpuTimeMgr->getRawTime(this, lwp, cpuTimeMgr_t::LEVEL_BEST);
   /* can nicely handle case when we allow exceptions
      } catch(LevelNotInstalled &) {
      cerr << "getRawCpuTime: timer level not installed\n";
      assert(0);
      }
   */
}

timeStamp pd_process::units2timeStamp(int64_t rawunits) {
   return cpuTimeMgr->units2timeStamp(rawunits, cpuTimeMgr_t::LEVEL_BEST);
   /* can nicely handle case when we allow exceptions
      } catch(LevelNotInstalled &) {
      cerr << "units2timeStamp: timer level not installed\n";
      assert(0);
      }
   */
}

timeLength pd_process::units2timeLength(int64_t rawunits) {
   return cpuTimeMgr->units2timeLength(rawunits, cpuTimeMgr_t::LEVEL_BEST);

   /* can nicely handle case when we allow exceptions
      } catch(LevelNotInstalled &) {
      cerr << "units2timeStamp: timer level not installed\n";
      assert(0);
      }
   */
}

void pd_process::verifyTimerLevels() {
   int hintBestCpuTimerLevel, hintBestWallTimerLevel;
   bool err = false;
   int appAddrWidth = getImage()->getAddressWidth();
   Address addr = findInternalAddress("hintBestCpuTimerLevel", true, err);
   assert(err==false);

   if (! readDataSpace((caddr_t)addr, appAddrWidth, &hintBestCpuTimerLevel,
                       true)) {
      return;  // readDataSpace has it's own error reporting
   }

   int curCpuTimerLevel = int(cpuTimeMgr->getBestLevel())+1;
   if(curCpuTimerLevel < hintBestCpuTimerLevel) {
      char errLine[150];
      sprintf(errLine, "Chosen cpu timer level (%d) is not available in the rt"
              " library (%d is best).\n", curCpuTimerLevel,
              hintBestCpuTimerLevel);
      fprintf(stderr, errLine);
      assert(0);
   }

   addr = findInternalAddress("hintBestWallTimerLevel",
                                               true, err);
   assert(err==false);
   if(! readDataSpace((caddr_t)addr, appAddrWidth, &hintBestWallTimerLevel,
                      true))
      return;  // readDataSpace has it's own error reporting

   int curWallTimerLevel = int(getWallTimeMgr().getBestLevel())+1;
   if(curWallTimerLevel < hintBestWallTimerLevel) {
      char errLine[150];
      sprintf(errLine, "Chosen wall timer level (%d) is not available in the"
              " rt library (%d is best).\n", curWallTimerLevel,
              hintBestWallTimerLevel);
      fprintf(stderr, errLine);
      assert(0);
   }
}

// being disabled since written for IRIX platform, now that don't support
// this platform, don't have way to test changes needed in this feature
// feel free to bring back to life if the need arises again
/*
bool pd_process::writeTimerFuncAddr_Force32(const char *rtinstVar,
                                            const char *rtinstFunc)
{
   bool err = false;
   int rtfuncAddr = findInternalAddress(rtinstFunc, true, err);
   assert(err==false);

   err = false;
   int timeFuncVarAddr = findInternalAddress(rtinstVar, true, err);
   assert(err==false);

   return writeTextSpace((void *)(timeFuncVarAddr),
			 sizeof(rtfuncAddr), (void *)(&rtfuncAddr));
}
*/

/* That is, get the address of the thing to set the function pointer to.  In
   most cases, this will be the address of the desired function, however, on
   AIX it is the address of a structure which in turn points to the desired
   function. 
*/
Address pd_process::getTimerQueryFuncTransferAddress(const char *helperFPtr) {
   bool err = false;
   Address transferAddrVar = findInternalAddress(helperFPtr, true, err);
   
   //logStream << "address of var " << helperFPtr << " = " << hex 
   //    << transferAddrVar <<"\n";

   int appAddrWidth = getImage()->getAddressWidth();

   Address transferAddr = 0;
   assert(err==false);
   if (! readDataSpace((caddr_t)transferAddrVar, appAddrWidth, &transferAddr,
                       true)) {
      cerr << "getTransferAddress: can't read var " << helperFPtr << "\n";
      return 0;
   }
   return transferAddr;
}

bool pd_process::writeTimerFuncAddr_(const char *rtinstVar,
				   const char *rtinstHelperFPtr)
{
   Address rtfuncAddr = getTimerQueryFuncTransferAddress(rtinstHelperFPtr);
   //logStream << "transfer address at var " << rtinstHelperFPtr << " = " 
   //     << hex << rtfuncAddr <<"\n";
   bool err = false;
   Address timeFuncVarAddr = findInternalAddress(rtinstVar, true, err);
   //logStream << "timeFuncVarAddr (" << rtinstVar << "): " << hex
   //   << timeFuncVarAddr << "\n";
   assert(err==false);
   return writeTextSpace((void *)(timeFuncVarAddr), sizeof(rtfuncAddr),
                         (void *)(&rtfuncAddr));
}

void pd_process::writeTimerFuncAddr(const char *rtinstVar, 
				 const char *rtinstHelperFPtr)
{   
   bool result;
   // being disabled since written for IRIX platform, now that don't support
   // this platform, don't have way to test changes needed in this feature
   // feel free to bring back to life if the need arises again
   //int appAddrWidth = getImage()->getObject().getAddressWidth();
   //if(sizeof(Address)==8 && appAddrWidth==4)
   //result = writeTimerFuncAddr_Force32(rtinstVar, rtinstFunc);     
   //else
   result = writeTimerFuncAddr_(rtinstVar, rtinstHelperFPtr);          

   if(result == false) {
     cerr << "!!!  Couldn't write timer func address into rt library !!\n";
   }
}

void pd_process::writeTimerLevels() {
   char rtTimerStr[61];
   rtTimerStr[60] = 0;
   pdstring cStr = cpuTimeMgr->get_rtTimeQueryFuncName(cpuTimeMgr_t::LEVEL_BEST);
   strncpy(rtTimerStr, cStr.c_str(), 59);
   writeTimerFuncAddr("PARADYNgetCPUtime", rtTimerStr);
   //logStream << "Setting cpu time retrieval function in rtinst to " 
   //     << rtTimerStr << "\n" << flush;
   
   pdstring wStr=wallTimeMgr->get_rtTimeQueryFuncName(wallTimeMgr_t::LEVEL_BEST);
   strncpy(rtTimerStr, wStr.c_str(), 59);
   writeTimerFuncAddr("PARADYNgetWalltime", rtTimerStr);
   //logStream << "Setting wall time retrieval function in rtinst to " 
   //     << rtTimerStr << "\n" << flush;
}


//
// Fill in the statically determinable components of the call
//  graph for process.  "statically determinable" refers to
//  the problem that some call destinations cannot be determined
//  statically, but rather instrumentation must be inserted to
//  determine the actual target (which may change depending on when 
//  the call is executed).  For example conmsider the assembly code 
//  fragment:
//   ....
//   call <random>  // puts random number (in some range) in g1
//   nop
//   call %g1
//   nop
//   ....
//  Code where the call target cannot be statically determined has
//   been observed w/ pointers to functions, switch statements, and 
//   some heavily optimized SPARC code.
//  Parameters:
//   Called just after an image is parsed and added to process
//   (image can represent either a.out or shared object).  
//   img - pointer to image just parsed and added.
//   shared_object - boolean inidcating whether img refers to an
//   a.out or shared object.
//
//  NOTE : Paradynd keeps 1 copy of each image, even when that image
//   appears in multiple processes (e.g. when that image represents
//   a shared object).  However, for keeping track of call graphs,
//   we want to keep a SEPERATE call graph for every process - this
//   includes the images which may be shared by multiple processes.
//   The reason for this is that when adding dynamically determined
//   call destinations, we want them to apply ONLY to the process
//   in which they are observed, NOT to other processes which may share
//   e.g. the same shared library. 

extern void CallGraphAddProgramCallback(pdstring name);
extern void CallGraphFillDone(pdstring exe_name);

void pd_process::FillInCallGraphStatic()
{
  // specify entry point (location in code hierarchy to begin call 
  //  graph searches) for call graph.  Currently, begin searches at
  //  "main" - note that main is usually NOT the actual entry point
  //  there is usually a function which does env specific initialization
  //  and sets up exit handling (at least w/ gcc on solaris).  However,
  //  this function is typically in an excluded module.  Anyway, setting
  //  main as the entry point should usually work fairly well, except
  //  that call graph PC searches will NOT catch time spent in the
  //  environment specific setup of _start.

  pd_Function *entry_pdf = (pd_Function *)findOnlyOneFunction("main");
  assert(entry_pdf);
  
  CallGraphAddProgramCallback(img->get_file());

  int thr = 0;
  // MT: forward the ID of the first thread.
  if(thr_mgr.size()) {
     threadMgr::thrIter begThrIter = beginThr();
     pd_thread *begThr = *(begThrIter);
     thr = begThr->get_tid();
  }

  if(multithread_capable()) {
     // Temporary hack -- ordering problem
     thr = 1;
  }

  CallGraphSetEntryFuncCallback(img->get_file(), 
                                entry_pdf->ResourceFullName(), thr);
    
  // build call graph for executable
  img->FillInCallGraphStatic(this);
  // build call graph for module containing entry point
  // ("main" is not always defined in the executable)
  image *main_img = entry_pdf->file()->exec();
  pd_image *pd_main_img = pd_image::get_pd_image(main_img);
  if (pd_main_img != img) 
     pd_main_img->FillInCallGraphStatic(this);

  // TODO: build call graph for all shared objects?
  CallGraphFillDone(img->get_file());
}

void pd_process::MonitorDynamicCallSites(pdstring function_name) {
   resource *r, *p;
   pdmodule *mod;
   r = resource::findResource(function_name);
   assert(r);
   p = r->parent();
   assert(p);
   mod = img->get_dyn_image()->findModule(p->name());
   if(!mod) {
      //Must be the weird case where main() isn't in the executable
      pd_Function *entry_pdf = (pd_Function *)findOnlyOneFunction("main");
      assert(entry_pdf);
      image *main_img = entry_pdf->file()->exec();
      assert(main_img);
      mod = main_img->findModule(p->name());
   }
   assert(mod);
  
   function_base *func, *temp;
   pdvector<function_base *> fbv;
   if (NULL == mod->findFunction(r->name(), &fbv) || !fbv.size()) {
      fprintf(stderr, "%s[%d]: Cannot find %s, currently fatal\n", __FILE__,
              __LINE__, r->name().c_str());
      abort();
   }
   if (fbv.size() > 1) {
      fprintf(stderr, "%s[%d]: Warning, found %d %s()\n", __FILE__, __LINE__,
              fbv.size(), r->name().c_str());
   }
   func = fbv[0];

   //Should I just be using a resource::handle here instead of going through
   //all of this crap to find a pointer to the function???
   pdstring exe_name = getImage()->get_file();
   pdvector<instPoint*> callPoints;
   process *llproc = get_dyn_process()->lowlevel_process();
   callPoints = func->funcCalls(llproc);
  
   for(unsigned i = 0; i < callPoints.size(); i++) {
      if(!findCallee(*(callPoints[i]), temp)) {

         if(! llproc->MonitorCallSite(callPoints[i])) {
            fprintf(stderr, 
              "ERROR in daemon, unable to monitorCallSite for function :%s\n",
                    function_name.c_str());
         }
      }
   }
}

bool reachedLibState(libraryState_t lib, libraryState_t state) {
   return (lib >= state);
}


// triggeredInStackFrame is used to determine whether instrumentation
//   added at the specified instPoint/callWhen/callOrder would have been
//   executed based on the supplied pc.
//
// If the pc is within instrumentation for the instPoint, the callWhen
//   and callOrder must be examined.  triggeredInStackFrame will return
//   true if the pc is located after the identified point of
//   instrumentation.
//   
// If the pc is not in instrumentation for the supplied instPoint, and
//   the instPoint is located at the function entry or if the instPoint
//   is for preInsn at a function call and the pc is located at the
//   return address of the callsite (indicating that the call is
//   currently executing), triggeredInStackFrame will return true.

bool process::triggeredInStackFrame(instPoint* point,  Frame &frame, 
                                    pd_Function *&func,
                                    callWhen when, callOrder order)
{
    //this->print(stderr, ">>> triggeredInStackFrame(): ");
    trampTemplate *tempTramp;
    bool retVal = false;
    pd_Function *instPoint_fn = dynamic_cast<pd_Function *>
      (const_cast<function_base *>(point->iPgetFunction()));
    pd_Function *stack_fn = func;
    if (!func) {
        stack_fn = findAddressInFuncsAndTramps(frame.getPC());
        func = stack_fn;
    }
    
    if (pd_debug_catchup) {
        char line[200];
        bool didA = false;
        if (stack_fn) {
            pdvector<pdstring> name = stack_fn->prettyNameVector();
            if (name.size()) {
                sprintf(line, "stack_func: %-20.20s ", name[0].c_str());
                didA = true;
            }
        }
        if(!didA)  sprintf(line, "stack_func: %-20.20s ", "");
        
        if (instPoint_fn) {
            pdvector<pdstring> name = instPoint_fn->prettyNameVector();
            strcat(line, "instP_func: ");
            if (name.size())
                strcat(line, name[0].c_str());
        }
        cerr << "triggeredInStackFrame- " << line << endl;
    }
    if (stack_fn != instPoint_fn) {
        if (stack_fn && instPoint_fn &&
            (stack_fn->prettyName() == instPoint_fn->prettyName()))
            if (pd_debug_catchup)
                fprintf(stderr, "Equal names, ptr %p != %p\n",
                        stack_fn, instPoint_fn);
        return false;
    }
  Address pc = frame.getPC();
  if ( pd_debug_catchup )
     cerr << "  Stack function matches function containing instPoint" << endl;

  if (pc == point->iPgetAddress()) {
      if (pd_debug_catchup) {
          fprintf(stderr, "Found pc at start of instpoint, returning false\n");
      }
      return false;
  }
  
  
  //  Is the pc within the instPoint instrumentation?
  instPoint* currentIp = findInstPointFromAddress(pc);

  if ( currentIp && currentIp == point )
  {
    tempTramp = findBaseTramp(currentIp, this);

    if ( tempTramp )
    {
      //  Check if pc in basetramp
        if ( tempTramp->inBasetramp(pc) )
      {
        if ( pd_debug_catchup )
        {
          cerr << "  Found pc in BaseTramp" << endl;
          fprintf(stderr, "    baseTramp range is (%lx - %lx)\n",
                  tempTramp->baseAddr,
                  (tempTramp->baseAddr + tempTramp->size));
          fprintf(stderr, "    localPreReturnOffset is %lx\n",
                  tempTramp->baseAddr+tempTramp->localPreReturnOffset);
          fprintf(stderr, "    localPostReturnOffset is %lx\n",
                  tempTramp->baseAddr+tempTramp->localPostReturnOffset);
        }
        
        if ( (when == callPreInsn && 
            pc >= tempTramp->baseAddr+tempTramp->localPreReturnOffset) ||
          (when == callPostInsn &&
            pc >= tempTramp->baseAddr+tempTramp->localPostReturnOffset) )
        {
          if ( pd_debug_catchup )
            cerr << "  pc is after requested instrumentation point, returning true." << endl;
          retVal = true;
        }
        else
        {
          if ( pd_debug_catchup )
            cerr << "  pc is before requested instrumentation point, returning false." << endl;
        }
      }
      else //  pc is in a mini-tramp
      {
	installed_miniTramps_list* mtList = getMiniTrampList(currentIp, when);
    if( mtList == NULL )
    {
        newMiniTrampList( currentIp, when, &mtList );
    }
	List<instInstance*>::iterator curMT = mtList->get_begin_iter();
	List<instInstance*>::iterator endMT = mtList->get_end_iter();	 

        bool pcInTramp = false;

	for(; curMT != endMT && !pcInTramp; curMT++)
        {
	  instInstance *currInstance = *curMT;
          if ( pd_debug_catchup )
          {
            fprintf(stderr, "  Checking for pc in mini-tramp (%lx - %lx)\n",
                    currInstance->trampBase, currInstance->returnAddr);
          }
          
          if ( pc >= currInstance->trampBase &&
                pc <= currInstance->returnAddr )
          {
            // We have found the mini-tramp that is currently being executed
            pcInTramp = true;

            if ( pd_debug_catchup )
            {
              cerr << "  Found pc in mini-tramp" << endl;
              cerr << "    Requested instrumentation is for ";
              switch(when) {
                case callPreInsn:
                  cerr << "PreInsn ";
                  break;
                case callPostInsn:
                  cerr << "PostInsn ";
                  break;
              }
              switch(order) {
                case orderFirstAtPoint:
                  cerr << "prepend ";
                  break;
                case orderLastAtPoint:
                  cerr << "append ";
                  break;
              }
              cerr << endl;

              cerr << "    The pc is in ";
              switch(when) {
                case callPreInsn:
                  cerr << "PreInsn ";
                  break;
                case callPostInsn:
                  cerr << "PostInsn ";
                  break;
              }
              cerr << "instrumentation\n";
            }
            
            // The request should be triggered if it is for:
            //   1)  pre-instruction instrumentation to prepend
            //   2)  pre-instruction instrumentation to append
            //         and the pc is in PostInsn instrumentation
            //   3)  post-instruction instrumentation to prepend
            //         and the pc is in PostInsn instrumentation
            if ( (when == callPreInsn && (order == orderFirstAtPoint || 
                  (order == orderLastAtPoint &&
                    when == callPostInsn))) ||
                 (when == callPostInsn && order == orderFirstAtPoint &&
		    when == callPostInsn) )
            {
              if ( pd_debug_catchup )
                cerr << "  pc is after requested instrumentation point, returning true." << endl;
              retVal = true;
            }
            else
            {
              if ( pd_debug_catchup )
                cerr << "  pc is before requested instrumentation point, returning false." << endl;
            }
          }
        }
      }
    } 
  }
  else  // pc not in instrumentation
  {
    //  If the instrumentation point is located at the entry of the
    //    function, it would be triggered.
    //  If the instrumentation point is a call site, the instrumentation
    //    is preInsn and the pc points to the return address of the call,
    //    the instrumentation should be triggered as any postInsn instrumentation
    //    will be executed.
#if defined(mips_sgi_irix6_4) || defined(mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
    if (point->ipType_ == IPT_ENTRY) {
      if ( pd_debug_catchup )
        cerr << "  pc not in instrumentation, requested instrumentation for function entry, returning true." << endl;
      retVal = true;
    } else if (point->ipType_ == IPT_CALL && when == callPreInsn) {
      // check if the $pc corresponds to the native call insn
      Address base;
      getBaseAddress(stack_fn->file()->exec(), base);
      Address native_ra = base + stack_fn->getAddress(0) + point->offset_ + point->size_;
      if (pc == native_ra)
      {
        if ( pd_debug_catchup )
          cerr << "  Requested instrumentation is preInsn for callsite being executed.  Returning true." << endl;
        retVal = true;
      }
      else
      {
        if ( pd_debug_catchup )
          cerr << "  Function at requested preInsn callsite is not being executed.  Returning false." << endl;
      }
    }
    else
    {
      if ( pd_debug_catchup )
        cerr << "  Requested instrumentation point is not appropriate for catchup, returning false." << endl;
    }      
#elif defined(sparc_sun_solaris2_4) || defined(alpha_dec_osf4_0)

    if (point->ipType == functionEntry) {
      if ( pd_debug_catchup )
        cerr << "  pc not in instrumentation, requested instrumentation for function entry, returning true." << endl;
      retVal = true;
    } else if (point->ipType == callSite && when == callPreInsn) {
      // looking at gdb, sparc-solaris seems to record PC of the 
      //  call site + 8, as opposed to the PC of the call site.
      Address base, target;
      getBaseAddress( stack_fn->file()->exec(), base );
      target = base + point->addr + 2 * sizeof(instruction);
      if (pc == target) {
        if ( pd_debug_catchup )
          cerr << "  Requested instrumentation is preInsn for callsite being executed.  Returning true." << endl;
        retVal = true;
      } else {
        trampTemplate *bt = findBaseTramp( point, this );
        Address target = bt->baseAddr + bt->emulateInsOffset + 2 * sizeof(instruction);
        if( pc == target )
        {
          if ( pd_debug_catchup )
            cerr << "  Requested instrumentation is preInsn for callsite being executed.  Returning true." << endl;
          retVal = true;
        }
        else
        {
          if ( pd_debug_catchup )
            cerr << "  Function at requested preInsn callsite is not being executed.  Returning false." << endl;
        }
      }
    }
    else
    {
      if ( pd_debug_catchup )
        cerr << "  Requested instrumentation point is not appropriate for catchup, returning false." << endl;
    }      
#elif defined(rs6000_ibm_aix4_1)
    if ( point->ipLoc == ipFuncEntry ) {
      if ( pd_debug_catchup )
        cerr << "  pc not in instrumentation, requested instrumentation for function entry, returning true." << endl;
      retVal = true;
    } else if ( point->ipLoc == ipFuncCallPoint && when == callPreInsn ) {
      // check if the stack_pc points to the instruction after the call site
      Address base, target;
      getBaseAddress( stack_fn->file()->exec(), base );
      target = base + point->addr + sizeof(instruction);
      //cerr << " stack_pc should be " << (void*)target;
      if ( pc == target ) {
        if ( pd_debug_catchup )
          cerr << "  Requested instrumentation is preInsn for callsite being executed.  Returning true." << endl;
        retVal = true;
      }
      else
      {
        if ( pd_debug_catchup )
          cerr << "  Function at requested preInsn callsite is not being executed.  Returning false." << endl;
      }
      //cerr << endl;
    }
    else
    {
      //if ( pd_debug_catchup )
        //cerr << "  Requested instrumentation point is not appropriate for catchup, returning false." << endl;
    }      
#elif defined(i386_unknown_nt4_0) || defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0)
    if ( point->address() == point->func()->getAddress( this ) ) {
      if ( pd_debug_catchup )
        cerr << "  pc not in instrumentation, requested instrumentation for function entry, returning true." << endl;
      retVal = true;
    } else if ( point->insnAtPoint().isCall() && when == callPreInsn ) {
      // check if the pc points to the instruction after the call site
      Address base, target;
      getBaseAddress( stack_fn->file()->exec(), base );
      target = base + point->address() + point->insnAtPoint().size();
      //cerr << " pc should be " << (void*)target;
      if ( pc == target ) {
        if ( pd_debug_catchup )
          cerr << "  Requested instrumentation is preInsn for callsite being executed." << endl;
        //cerr << " -- HIT";
        retVal = true;
      }
      else
      {
        if ( pd_debug_catchup )
          cerr << "  Function at requested preInsn callsite is not being executed.  Returning false." << endl;
      }
      //cerr << endl;
    }
    else
    {
      if ( pd_debug_catchup )
        cerr << "  Requested instrumentation point is not appropriate for catchup, returning false." << endl;
    }      
#endif
  }

  return retVal;
}

