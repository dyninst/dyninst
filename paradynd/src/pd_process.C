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

#include "paradynd/src/pd_process.h"
#include "paradynd/src/pd_thread.h"
#include "dyninstAPI/src/signalhandler.h"
#include "paradynd/src/init.h"
#include "paradynd/src/metricFocusNode.h"
#include "paradynd/src/processMgr.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/perfStream.h"

// Set in main.C
extern int termWin_port;
extern string pd_machine;
extern PDSOCKET connect_Svr(string machine,int port);
extern pdRPC *tp;

// Exec callback
extern void pd_execCallback(process *proc);

pdvector<string> pd_process::arg_list;
string pd_process::defaultParadynRTname;

const string nullString("");

// Global "create a new pd_process object" functions

pd_process *pd_createProcess(pdvector<string> &argv, pdvector<string> &envp, string dir) {
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
    pd_process *proc = new pd_process(argv[0], argv, envp, dir, 0, stdout_fd, 2);
#else 
    pd_process *proc = new pd_process(argv[0], argv, envp, dir, 0, 1, 2);
#endif
    if (!proc) {
#if !defined(i386_unknown_nt4_0)
        CLOSEPDSOCKET(stdout_fd);
#endif
        return NULL;
    }
    // Load the paradyn runtime lib
    proc->loadParadynLib();
    
    // Run necessary initialization
    proc->init();
    
    // Lower batch mode
    tp->resourceBatchMode(false); 
    
	if (!costMetric::addProcessToAll(proc->get_dyn_process()))
        assert(false);
    
    getProcMgr().addProcess(proc);

    string buffer = string("PID=") + string(proc->getPid());
    buffer += string(", ready");
    statusLine(buffer.c_str());
    
    return proc;
}

pd_process *pd_attachProcess(const string &progpath, int pid) { 
    // Avoid deadlock
	tp->resourceBatchMode(true);

    pd_process *proc = new pd_process(progpath, pid);

    proc->loadParadynLib();
    proc->init();

    // Lower batch mode
    tp->resourceBatchMode(false);

	if (!costMetric::addProcessToAll(proc->get_dyn_process()))
		assert(false);

	getProcMgr().addProcess(proc);

    string buffer = string("PID=") + string(proc->getPid());
    buffer += string(", ready");
    statusLine(buffer.c_str());
	
    return proc;
}

void pd_process::init() {
    
    string buffer = string("PID=") + string(getPid());
    buffer += string(", initializing daemon-side data");
    statusLine(buffer.c_str());

    for(unsigned i=0; i<dyninst_process->threads.size(); i++) {
        pd_thread *thr = new pd_thread(dyninst_process->threads[i]);
        addThread(thr);
    }
    
    theVariableMgr = new variableMgr(this, getSharedMemMgr(),
                                     maxNumberOfThreads());
    buffer = string("PID=") + string(getPid());
    buffer += string(", posting call graph information");
    statusLine(buffer.c_str());
    
    dyninst_process->FillInCallGraphStatic();
    if (resource::num_outstanding_creates)
        dyninst_process->setWaitingForResources();
    
}

// Creation constructor
pd_process::pd_process(const string argv0, pdvector<string> &argv,
                       pdvector<string> envp, const string dir,
                       int stdin_fd, int stdout_fd, int stderr_fd) 
        : numOfActCounters_is(0), numOfActProcTimers_is(0),
          numOfActWallTimers_is(0), 
          paradynRTState(libUnloaded),
          wasCreated(true), wasAttached(false), 
          wasForked(false),
          wasExeced(false), inExec(false)
{
    dyninst_process = createProcess(argv0, argv, envp, dir, 
                                    stdin_fd, stdout_fd, stderr_fd);
    
    if (!dyninst_process) {
        // Ummm.... 
        cerr << "Error creating process" << endl;
        return;
    }
    // Dyninst process create currently also builds and attaches
    // to the shared segment. That should be moved here. In the
    // meantime....
    sharedMetaDataOffset = dyninst_process->initSharedMetaData();

    // Set the paradyn RT lib name
    if (!getParadynRTname())
        assert(0 && "Need to do cleanup");
}

// Attach constructor
pd_process::pd_process(const string &progpath, int pid)
        : numOfActCounters_is(0), numOfActProcTimers_is(0),
          numOfActWallTimers_is(0), 
          paradynRTState(libUnloaded),
          wasCreated(false), wasAttached(true), wasForked(false),
          wasExeced(false), inExec(false)
{
    
    dyninst_process = attachProcess(progpath, pid);
    
    if (!dyninst_process) {
        // Ummm.... 
        cerr << "Error creating process" << endl;
        return;
    }
    // Dyninst process create currently also builds and attaches
    // to the shared segment. That should be moved here. In the
    // meantime....
    sharedMetaDataOffset = dyninst_process->initSharedMetaData();
    // Set the paradyn RT lib name
    if (!getParadynRTname())
        assert(0 && "Need to do cleanup");
}


// fork constructor
pd_process::pd_process(const pd_process &parent, process *childDynProc) :
        dyninst_process(childDynProc), 
        paradynRTState(libLoaded),
        wasCreated(parent.wasCreated), wasAttached(parent.wasAttached),
        wasForked(true), wasExeced(false), inExec(false)
{
   setLibState(paradynRTState, libReady);
   for(unsigned i=0; i<childDynProc->threads.size(); i++) {
      pd_thread *pd_thr = new pd_thread(childDynProc->threads[i]);
      thr_mgr.addThread(pd_thr);
      dyn_thread *thr = pd_thr->get_dyn_thread();

      if(! multithread_ready()) continue;
      // computing resource id
      string buffer;
      string pretty_name = string(thr->get_start_func()->prettyName().c_str());
      buffer = string("thr_") + string(thr->get_tid()) + string("{")
               + pretty_name + string("}");
      resource *rid;
      rid = resource::newResource(get_rid(), (void *)thr, nullString, 
                                  buffer, timeStamp::ts1970(), "",
                                  MDL_T_STRING, true);
      pd_thr->get_dyn_thread()->update_rid(rid);
      // tell front-end about thread start function for newly created threads
      // We need the module, which could be anywhere (including a library)
      pd_Function *func = (pd_Function *)thr->get_start_func();
      pdmodule *foundMod = func->file();
      assert(foundMod != NULL);
      resource *modRes = foundMod->getResource();
      string start_func_str = thr->get_start_func()->prettyName();
      string res_string = modRes->full_name() + "/" + start_func_str;
      CallGraphSetEntryFuncCallback(getImage()->file(), res_string, 
                                    thr->get_tid());
   }

   theVariableMgr = new variableMgr(*parent.theVariableMgr, this,
                                    getSharedMemMgr());
   theVariableMgr->initializeVarsAfterFork();
}

pd_process::~pd_process() {
   delete theVariableMgr;
   delete dyninst_process;
}

bool pd_process::doMajorShmSample() {
   if( !isBootstrappedYet() || !isPARADYNBootstrappedYet()) { //SPLIT ccw 4 jun 2002
      return false;
   }

   bool result = true; // will be set to false if any processAll() doesn't complete
                       // successfully.
   process *dyn_proc = get_dyn_process();

   if(! getVariableMgr().doMajorSample())
      result = false;
   
   if(status() == exited) {
      return false;
   }

   // inferiorProcessTimers used to take in a non-dummy process time as the
   // 2d arg, but it looks like that we need to re-read the process time for
   // each proc timer, at the time of sampling the timer's value, to avoid
   // ugly jagged spikes in histogram (i.e. to avoid incorrect sampled 
   // values).  Come to think of it: the same may have to be done for the 
   // wall time too!!!

   const timeStamp theProcTime = dyn_proc->getCpuTime(0);
   const timeStamp curWallTime = getWallTime();
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

void pd_process::handleExit(int exitStatus) {
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
   metricFocusNode::handleDeletedProcess(this);

   if(multithread_ready()) {
      // retire any thread resources which haven't been retired yet
      threadMgr::thrIter itr = beginThr();
      while(itr != endThrMark()) {
         pd_thread *thr = *itr;
         itr++;
         assert(thr->get_dyn_thread()->get_rid() != NULL);
         tp->retiredResource(thr->get_dyn_thread()->get_rid()->full_name());
      }
   }

   assert(get_rid() != NULL);
   tp->retiredResource(get_rid()->full_name());
   tp->processStatus(getPid(), procExited);
}

void pd_process::initAfterFork(pd_process *parentProc) {
   process *childproc = get_dyn_process();
   process *parentproc = parentProc->get_dyn_process();
   int pid = getPid();

   childproc->initCpuTimeMgr();

   string buff = string(pid); // + string("_") + getHostName();
   childproc->rid = resource::newResource(machineResource, // parent
                               (void*)this, // handle
                               nullString, // abstraction
                               parentproc->getImage()->name(),
                               timeStamp::ts1970(), // creation time
                               buff, // unique name (?)
                               MDL_T_STRING, // mdl type (?)
                               true
                               );
}

/********************************************************************
 **** Fork/Exec handling code                                    ****
 ********************************************************************/

void pd_process::paradynPreForkDispatch(process *p, void *data) {
    ((pd_process *)data)->preForkHandler(p);
}

void pd_process::paradynPostForkDispatch(process *p, void *data, process *c) {
    ((pd_process *)data)->postForkHandler(p, c);
}

void pd_process::paradynPreExecDispatch(process *p, void *data, char *arg0) {
    ((pd_process *)data)->preExecHandler(p, arg0);
}

void pd_process::paradynPostExecDispatch(process *p, void *data) {
    fprintf(stderr, "postExecDispatch called\n");
    ((pd_process *)data)->postExecHandler(p);
}

void pd_process::paradynPreExitDispatch(process *p, void *data, int code) {
    ((pd_process *)data)->preExitHandler(p, code);
}

void pd_process::preForkHandler(process *p) {
}

void pd_process::postForkHandler(process *p, process *c) {
}

void pd_process::preExecHandler(process *p, char *arg0) {
}

void pd_process::postExecHandler(process *p) {
    // We need to reload the Paradyn library
    paradynRTState = libUnloaded; // It was removed when we execed
    inExec = true;
    wasExeced = true;

    // Renew the metadata: it's been scribbled on
    //sharedMetaDataOffset = dyninst_process->initSharedMetaData();

    loadParadynLib();
}

void pd_process::preExitHandler(process *p, int code) {
}


/********************************************************************
 **** Paradyn runtime library code                               ****    
 ********************************************************************/

// Load and initialize the paradyn runtime library.

bool pd_process::loadParadynLib() {
    // Force a load of the paradyn runtime library. We use
    // the dyninst runtime DYNINSTloadLibrary call, as in
    // BPatch_thread::loadLibrary
    
    assert(dyninst_process->status() == stopped);

    string buffer = string("PID=") + string(getPid());
    buffer += string(", loading Paradyn RT lib via iRPC");       
    statusLine(buffer.c_str());

#if defined(i386_unknown_nt4_0)
    // Another FIXME: NT strips the path from the loaded
    // library for recognition purposes. 
    char dllFilename[_MAX_FNAME];
    _splitpath (paradynRTname.c_str(),
                NULL, NULL, dllFilename, NULL);
    // Set the callback to run setParams
    dyninst_process->registerLoadLibraryCallback(string(dllFilename),
                                                 setParadynLibParamsCallback,
                                                 (void *)this);
#else
    dyninst_process->registerLoadLibraryCallback(paradynRTname,
                                                 setParadynLibParamsCallback,
                                                 (void *)this);
#endif
#if defined(sparc_sun_solaris2_4)
    // Sparc requires libsocket to be loaded before the paradyn lib can be :/
    loadAuxiliaryLibrary("libsocket.so");
#endif

    pdvector<AstNode*> loadLibAstArgs(1);
    loadLibAstArgs[0] = new AstNode(AstNode::ConstantString, 
                                    (void *)paradynRTname.c_str());
    AstNode *loadLib = new AstNode("DYNINSTloadLibrary", loadLibAstArgs);
    removeAst(loadLibAstArgs[0]);

    // We've built a call to loadLibrary, now run it via inferior RPC
    postRPCtoDo(loadLib, true, // Don't update cost
                pd_process::paradynLibLoadCallback,
                (void *)this); // User data

    setLibState(paradynRTState, libLoading);
    // .. run RPC

    // We block on paradynRTState, which is set to libLoaded
    // via the inferior RPC callback
    while (!reachedLibState(paradynRTState, libLoaded)) {
        launchRPCs(false);
        decodeAndHandleProcessEvent(true);
    }
    removeAst(loadLib);

    // Unregister callback now that the library is loaded
    dyninst_process->unregisterLoadLibraryCallback(paradynRTname);

    buffer = string("PID=") + string(getPid());
    buffer += string(", finalizing Paradyn RT lib");       
    statusLine(buffer.c_str());

    // Now call finalizeParadynLib which will handle any initialization
    if (!finalizeParadynLib()) {
        buffer = string("PID=") + string(getPid());
        buffer += string(", finalizing Paradyn RT lib via iRPC");       
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
bool pd_process::setParadynLibParams()
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

    int creationMethod = 0;
    if (wasAttached) creationMethod = 1;
    if (wasForked) creationMethod = 2;
    
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
    

    int theKey = dyninst_process->getShmKeyUsed();
    if (!getSymbolInfo("libparadynRT_init_localtheKey", sym, baseAddr))
        if (!getSymbolInfo("_libparadynRT_init_localtheKey", sym, baseAddr))
            assert(0 && "Could not find symbol libparadynRT_init_localtheKey");
    assert(sym.type() != Symbol::PDST_FUNCTION);
    writeDataSpace((void*)(sym.addr() + baseAddr), sizeof(int), (void *)&theKey);
    //fprintf(stderr, "Set localTheKey to %d\n", theKey);


    int shmSegNumBytes = dyninst_process->getShmHeapTotalNumBytes();
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
                                             string /*ignored*/,
                                             shared_object * /*libobj*/,
                                             void *data) {
    pd_process *p = (pd_process *) data;
    p->setParadynLibParams();
}

bool pd_process::finalizeParadynLib() {
    // Check to see if paradyn init has been run, and if
    // not run it manually via inferior RPC
    string str;

    if (reachedLibState(paradynRTState, libReady))
        return true;
    
    PARADYN_bootstrapStruct bs_record;
    if (!extractBootstrapStruct(&bs_record)){
        assert(false);
    }
    
    // Read the structure; if event 0 then it's undefined! (not yet written)
    if (bs_record.event == 0) return false;

    assert(bs_record.event == 1 || bs_record.event == 2 || bs_record.event==3);
    assert(bs_record.pid == getPid());
    
    const bool calledFromFork   = (bs_record.event == 2);
    const bool calledFromExec   = (bs_record.event == 1 && wasExeced);
    const bool calledFromAttach = (bs_record.event == 3);

    // Override tramp guard address
    dyninst_process->setTrampGuardAddr((Address) bs_record.tramp_guard_base);

    dyninst_process->registerInferiorAttachedSegs(bs_record.appl_attachedAtPtr.ptr);

    if (!calledFromFork) {
        // MT: need to set Paradyn's bootstrap state or the instrumentation
        // basetramps will be created in ST mode
        dyninst_process->setParadynBootstrap();

        // Install initial instrumentation requests
        extern pdvector<instMapping*> initialRequestsPARADYN; // init.C //ccw 18 apr 2002 : SPLIT
        dyninst_process->installInstrRequests(initialRequestsPARADYN); 
        str=string("PID=") + string(bs_record.pid) + ", propagating mi's...";
        statusLine(str.c_str());
    }
    
    if (calledFromExec) {
        pd_execCallback(dyninst_process);
    }
    else { // Not called from exec
        metricFocusNode::handleNewProcess(dyninst_process);
    }
    
    str=string("PID=") + string(bs_record.pid) + ", executing new-prog callback...";
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
                               get_dyn_process()->wasRunningWhenAttached());
    // in paradyn, this will call paradynDaemon::addRunningProgram().
    // If the state of the application as a whole is 'running' paradyn will
    // soon issue an igen call to us that'll continue this process.
    if (!calledFromExec) {
        tp->setDaemonStartTime(getPid(), currWallTime.getD(timeUnit::sec(), 
                                                           timeBase::bStd()));
    }
    // verify that the wall and cpu timer levels chosen by the daemon
    // are available in the rt library
    dyninst_process->verifyTimerLevels();
    
    dyninst_process->writeTimerLevels();
    
    if (calledFromFork) {
        // the parent proc has been waiting patiently at the start of DYNINSTfork
        // (i.e. the fork syscall executed but that's it).  We can continue it now.
        process *parentProcess = process::findProcess(bs_record.ppid);
        
        if (parentProcess) {
            if (parentProcess->status() == stopped) {
                if (!parentProcess->continueProc())
                    assert(false);
            }
            else
                parentProcess->continueAfterNextStop();
        }
    }

    // Set library state to "ready"
    setLibState(paradynRTState, libReady);

    // Add callbacks for events we care about
    /*
      dyninst_process->registerPreForkCallback(paradynPreForkDispatch,
      (void *)this);
      dyninst_process->registerPreExecCallback(paradynPreExecDispatch,
      (void *)this);
    */
    dyninst_process->registerPostExecCallback(paradynPostExecDispatch,
                                              (void *)this);
    extern void paradyn_forkCallback(process *parentDynProc, 
                                     void *parentDynProcData,
                                     process *childDynProc);
    dyninst_process->registerPostForkCallback(paradyn_forkCallback,
                                              (void *)this);
    
    /*
      dyninst_process->registerPreExitCallback(paradynPreExitDispatch,
      (void *)this);
    */
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
                (void *)this); // User data

    // And force a flush...
    
    while(!reachedLibState(paradynRTState, libReady)) {
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
    const string vrbleName = "PARADYN_bootstrap_info";
    internalSym sym;
    bool flag = findInternalSymbol(vrbleName, true, sym);
    assert(flag);
    Address symAddr = sym.getAddr();
    // bulk read of bootstrap structure
    
    if (!dyninst_process->readDataSpace((const void*)symAddr, sizeof(*bs_record), 
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
    ret2 = dyninst_process->readDataSpace((void *)sym2.getAddr(), sizeof(int32_t), 
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
#ifdef MT_THREAD
    const char ParadynEnvVar[]="PARADYN_LIB_MT";
#else
    const char ParadynEnvVar[]="PARADYN_LIB";
#endif // MT_THREAD
    
    // If there is a default set, use it
    if (defaultParadynRTname.length())
        paradynRTname = defaultParadynRTname;
    else {
        // check the environment variable
        if (getenv(ParadynEnvVar) != NULL) {
            paradynRTname = getenv(ParadynEnvVar);
        } else {
            string msg = string("Environment variable " + string(ParadynEnvVar)
                                + " has not been defined for process "
                                + string(getPid()));
            showErrorCallback(101, msg);
            cerr << "Environment variable " << ParadynEnvVar << " not set!" << endl;
            return false;
        }
    }

#if !defined(i386_unknown_nt4_0)
    // TODO: make equivalent for NT
    // Check to see if the library given exists.
    if (access(paradynRTname.c_str(), R_OK)) {
        string msg = string("Runtime library ") + paradynRTname
        + string(" does not exist or cannot be accessed!");
        showErrorCallback(101, msg);
        cerr << "Paradyn lib is not accessible!" << endl;
        return false;
    }
#endif
    return true;
}


bool pd_process::loadAuxiliaryLibrary(string libname) {
    auxLibState = libUnloaded;

    pdvector<AstNode*> loadLibAstArgs(1);
    loadLibAstArgs[0] = new AstNode(AstNode::ConstantString, 
                                    (void *)libname.c_str());
    AstNode *loadLib = new AstNode("DYNINSTloadLibrary", loadLibAstArgs);
    removeAst(loadLibAstArgs[0]);

    // We've built a call to loadLibrary, now run it via inferior RPC
    postRPCtoDo(loadLib, true, // Don't update cost
                pd_process::loadAuxiliaryLibraryCallback,
                (void *)this); // User data

    setLibState(auxLibState, libLoading);
    // .. run RPC

    // We block on paradynRTState, which is set to libLoaded
    // via the inferior RPC callback
    while (!reachedLibState(auxLibState, libLoaded)) {
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

