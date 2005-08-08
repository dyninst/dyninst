/*
 * Copyright (c) 1996-2004 Barton P. Miller
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
#include "paradynd/src/init.h"
#include "paradynd/src/metricFocusNode.h"
#include "paradynd/src/processMgr.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/pd_image.h"
#include "paradynd/src/pd_module.h"

#include "dyninstAPI/src/function.h"

#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/InstrucIter.h"
#include "dyninstAPI/src/instPoint.h"
#if defined(i386_unknown_nt4_0)
#include <Windows.h>
#endif

#include "paradynd/src/debug.h"

// Set in main.C
extern int termWin_port;
extern pdstring pd_machine;
extern PDSOCKET connect_Svr(pdstring machine,int port);
extern pdRPC *tp;

// Exec callback
extern void pd_execCallback(pd_process *proc);

pdvector<pdstring> pd_process::arg_list;
pdstring pd_process::defaultParadynRTname;
pdstring pd_process::pdFlavor;
pdstring pd_process::programName;

extern resource *machineResource;

bool should_report_loops = false;

void addLibraryCallback(BPatch_thread *thr,
                        BPatch_module *mod,
                        bool load)
{
  if (!mod) {
    fprintf(stderr, "%s[%d]:  addModuleCallback called w/out module!\n",
            __FILE__, __LINE__);
    return;
  }

  BPatch_thread *appThread = NULL;
  pd_process *parent_proc = NULL;
  for (unsigned int i = 0; i < pd_image::all_pd_images.size(); ++i) {
    parent_proc = pd_image::all_pd_images[i]->getParentProc();
    appThread = parent_proc->get_dyn_process();
    if (appThread  == thr) {
      break; 
    }
    parent_proc = NULL;
  }

  if (!parent_proc) {
    fprintf(stderr, "%s[%d]:  Could not find parent process for new module\n",
            __FILE__, __LINE__);
    return;
   }

  pd_image *img = parent_proc->getImage();

  if (!img) {
       fprintf(stderr, "%s[%d]:  this should never happen!\n", __FILE__, __LINE__);
       return;
  }

  if (load) {

    if (img->hasModule(mod)) {
      fprintf(stderr, "%s[%d]:  WARN:  addModule ignoring duplicate!\n",
              __FILE__, __LINE__);
    }
    else {
      img->addModule(mod);
      return;
    }
  }
  else {
    fprintf(stderr, "%s[%d]:  Non-load call of BPatchDynLibraryCallback!\n",
            __FILE__, __LINE__);
    fprintf(stderr, "%s[%d]:  Not anticipated, not fatal, but FIXME.\n",
            __FILE__, __LINE__);
  }
}

// Global "create a new pd_process object" functions

pd_process *pd_createProcess(pdvector<pdstring> &argv, pdstring dir,
                             bool parse_loops) {
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
   should_report_loops = parse_loops;
#if !defined(i386_unknown_nt4_0)
    pd_process *proc = new pd_process(argv[0], argv, dir, 0, stdout_fd, 2, 
                                      parse_loops);
#else 
    pd_process *proc = new pd_process(argv[0], argv, dir, 0, 1, 2, 
                                      parse_loops);
#endif
    if ( (proc == NULL) || (proc->get_dyn_process() == NULL) ) {
#if !defined(i386_unknown_nt4_0)
        CLOSEPDSOCKET(stdout_fd);
#endif
        return NULL;
    }

    // Load the paradyn runtime lib
    if (!proc->getSharedMemMgr()->initialize()) {
      fprintf(stderr, "%s[%d]:  failed to init shared mem mgr, fatal...\n", __FILE__, __LINE__);
	tp->resourceBatchMode(false);
      return NULL;
    }
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

pd_process *pd_attachProcess(const pdstring &progpath, int pid, bool parse_loops)
{ 
    // Avoid deadlock
	tp->resourceBatchMode(true);

   should_report_loops = parse_loops;
    pd_process *proc = new pd_process(progpath, pid, parse_loops);

    if (!proc || !proc->get_dyn_process()) return NULL;

    if (!proc->getSharedMemMgr()->initialize()) {
      fprintf(stderr, "%s[%d]:  failed to init shared mem mgr, fatal...\n", __FILE__, __LINE__);
	tp->resourceBatchMode(false);
      return NULL;
    }

    proc->loadParadynLib(pd_process::attach_load);
    proc->init();

    // Lower batch mode
    tp->resourceBatchMode(false);

    process *llproc = proc->get_dyn_process()->lowlevel_process();
    if (!costMetric::addProcessToAll(llproc))
       assert(false);

    getProcMgr().addProcess(proc);

    llproc->recognize_threads(NULL);

    pdstring buffer = pdstring("PID=") + pdstring(proc->getPid());
    buffer += pdstring(", ready");
    statusLine(buffer.c_str());
	
    return proc;
}

pd_process *pd_attachToCreatedProcess(const pdstring &progpath,
                                      int pid,
                                      bool examineLoops) {
    // This has atrophied and needs to be fixed;
    // instead of horrible behavior I'm going to have it
    // break.
    
    // We need a Dyninst-level attach to created mechanism...

    return NULL;
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
                             timeStamp::ts1970(), "", 
                             CategoryResourceType,
                             MDL_T_STRING,
                             false);
       resource::newResource(syncRoot, NULL, nullString, "RwLock", 
                             timeStamp::ts1970(), "",
                             CategoryResourceType,
                             MDL_T_STRING,
                             false);
       resource::newResource(syncRoot, NULL, nullString, "CondVar", 
                             timeStamp::ts1970(), "",
                             CategoryResourceType,
                             MDL_T_STRING,
                             false);
       has_mt_resource_heirarchies_been_defined = true;
    }
    FillInCallGraphStatic();
}

// Creation constructor
pd_process::pd_process(const pdstring argv0, pdvector<pdstring> &argv,
                       const pdstring dir, int stdin_fd, int stdout_fd,
                       int stderr_fd, bool loops) 
        : monitorFunc(NULL),
          use_loops(loops),
          numOfActCounters_is(0), numOfActProcTimers_is(0),
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
               strerror(errno));
       logLine(errorLine);
       P__exit(-1);
    }

    char **argv_array = new char*[argv.size()+1];
    for(unsigned i=0; i<argv.size(); i++)
       argv_array[i] = const_cast<char *>(argv[i].c_str());
    argv_array[argv.size()] = NULL;

    char *path = new char[  argv0.length() + 5];
    strcpy(path, argv0.c_str());
    getBPatch().setTypeChecking(false);
    dyninst_process = getBPatch().createProcess(path, 
        (const char **) argv_array, NULL, 
        stdin_fd, stdout_fd, stderr_fd);
    if (dyninst_process == 0) {
       // createProcess will print proper error message in the paradyn msg box
       P__exit(-1);
    }
    bproc = dyninst_process->getProcess();

    delete []argv_array;
    delete path;

    created_via_attach = false;

    img = new pd_image(dyninst_process->getImage(), this);

    pdstring img_name = img->name();
    if (img_name == (char *) NULL) {
      //  this will cause an assertion failure in newResource()
      fprintf(stderr, "%s[%d]:  unnamed image!\n", __FILE__, __LINE__);
    }

    pdstring buff = pdstring(getPid()); // + pdstring("_") + getHostName();
    rid = resource::newResource(machineResource, // parent
				(void*)this, // handle
				nullString, // abstraction
				img->name(), // process name
				timeStamp::ts1970(), // creation time
				buff, // unique name (?)
                ProcessResourceType,
				MDL_T_STRING, // mdl type (?)
				true
				);
    
    if (!dyninst_process) {
        // Ummm.... 
        return;
    }

    initCpuTimeMgr();

    // Initialize the shared memory segment
    sharedMemManager = new shmMgr(dyninst_process,
                                  7000, // Arbitrary constant for shared key
                                  SHARED_SEGMENT_SIZE,
                                  false // Don't leave around -- if we're attached, then the
                                  // inferior process will have the segment as long is it 
                                  // exists
                                  );
    shmMetaData = new sharedMetaData(sharedMemManager, MAX_NUMBER_OF_THREADS);
    
    // Set the paradyn RT lib name
    if (!getParadynRTname())
        assert(0 && "Need to do cleanup");
}

// Attach constructor
pd_process::pd_process(const pdstring &progpath, int pid, bool loops)
        : monitorFunc(NULL),
          use_loops(loops),
          numOfActCounters_is(0), numOfActProcTimers_is(0),
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
    bproc = dyninst_process->getProcess();    
    img = new pd_image(dyninst_process->getImage(), this);
    pdstring img_name = img->name();
    if (img_name == (char *) NULL) {
      //  this will cause an assertion failure in newResource()
      fprintf(stderr, "%s[%d]:  unnamed image!\n", __FILE__, __LINE__);
    }

    pdstring buff = pdstring(getPid()); // + pdstring("_") + getHostName();
    rid = resource::newResource(machineResource, // parent
                                (void*)this, // handle
                                nullString, // abstraction
                                img->name(),
                                timeStamp::ts1970(), // creation time
                                buff, // unique name (?)
                                ProcessResourceType,
                                MDL_T_STRING, // mdl type (?)
                                true
                                );

    if (!dyninst_process) {
        // Ummm.... 
        return;
    }

    created_via_attach = true;

    initCpuTimeMgr();

    // Initialize the shared memory segment
    sharedMemManager = new shmMgr(dyninst_process,
                                  7000, // Arbitrary constant for shared key
                                  SHARED_SEGMENT_SIZE,
                                  false // Don't leave around -- if we're attached, then the
                                  // inferior process will have the segment as long is it 
                                  // exists
                                  );
    shmMetaData = new sharedMetaData(sharedMemManager, MAX_NUMBER_OF_THREADS);

    // Set the paradyn RT lib name
    if (!getParadynRTname())
        assert(0 && "Need to do cleanup");

}

extern void CallGraphSetEntryFuncCallback(pdstring exe_name, pdstring r, int tid);

// fork constructor
pd_process::pd_process(const pd_process &parent, BPatch_thread *childDynProc) :
        dyninst_process(childDynProc), 
        monitorFunc(NULL),
        use_loops(parent.use_loops),
        cpuTimeMgr(NULL),
#ifdef PAPI
        papi(NULL),
#endif
        paradynRTState(libLoaded), inExec(false),
        paradynRTname(parent.paradynRTname)
{
   bproc = childDynProc->getProcess();
   img = new pd_image(dyninst_process->getImage(), this);

   // Call fork initialization code
   BPatch_Vector<BPatch_snippet *>fork_init_args;
   BPatch_Vector<BPatch_function *>fork_init_func;

   if ((NULL == dyninst_process->getImage()->findFunction("PARADYN_init_child_after_fork",
                                                    fork_init_func)) ||
       fork_init_func.size() == 0) {
       assert(0 && "Failed to find post-fork initialization function");
   }
   BPatch_funcCallExpr fork_init_expr(*(fork_init_func[0]), fork_init_args);

   if (dyninst_process->oneTimeCode(fork_init_expr) != (void *)123)
       fprintf(stderr, "Error running forked child init function\n");
   
   pdstring img_name = img->name();
   if (img_name == (char *) NULL) {
     //  this will cause an assertion failure in newResource()
     fprintf(stderr, "%s[%d]:  unnamed image!\n", __FILE__, __LINE__);
   }

   pdstring buff = pdstring(getPid()); // + pdstring("_") + getHostName();
   rid = resource::newResource(machineResource, // parent
                               (void*)this, // handle
                               nullString, // abstraction
                               img->name(),
                               timeStamp::ts1970(), // creation time
                               buff, // unique name (?)
                               ProcessResourceType,
                               MDL_T_STRING, // mdl type (?)
                               true
                               );

   setLibState(paradynRTState, libReady);

   // Okay, time to rock and roll... that is, make a copy of the parent's
   // shared memory
   sharedMemManager = new shmMgr(parent.sharedMemManager, childDynProc,
                                 true /* Attach at same addr as in parent */);
   // Already initialized
   shmMetaData = new sharedMetaData(parent.shmMetaData, sharedMemManager);
   // Don't need to update observed cost addr -- the application side (that
   // the process class cares about) hasn't changed.

   theVariableMgr = new variableMgr(*parent.theVariableMgr, this,
                                    getSharedMemMgr());
   theVariableMgr->initializeVarsAfterFork();

   created_via_attach = parent.wasCreatedViaAttach();

   // And the time manager...
   initCpuTimeMgr();
   tp->newProgramCallbackFunc(getPid(), arg_list, 
                              machineResource->part_name(),
                              false, wasRunningWhenAttached());
   
   // Thread time. We keep our own list of threads in the process.

   // Back off the paradyn bootstrap level

   // DYNINST SEP
   process *cp = childDynProc->PDSEP_process();
   if (parent.dyninst_process->PDSEP_process()->multithread_ready()) {
     for (unsigned i = 0; i < cp->threads.size(); i++) {
       dyn_thread *thr = cp->threads[i];
       pd_thread *pd_thr = new pd_thread(thr, this);
       addThread(pd_thr);
       // This is mostly duplicated from context.C::createThread.
       // Problem is, in this case we _have_ threads already...     
       resource *rid;
       pdstring buffer;
       buffer = pdstring("thr_") + 
	 pdstring(thr->get_tid()) + 
	 pdstring("{") + 
	 thr->get_start_func()->prettyName() +
	 pdstring("}");
       rid = resource::newResource(get_rid(),
				   (void *)cp->threads[i],
				   nullString, 
				   buffer,
				   timeStamp::ts1970(),
				   "",
				   ThreadResourceType,
				   MDL_T_STRING,
				   true);
       pd_thr->update_rid(rid);
       pd_thr->resetInferiorVtime(getVirtualTimer(thr->get_index()));
     }
   }
   else {
      // Make the "dummy" primary thread
      for (unsigned j = 0; j < cp->threads.size(); j++) {
         pd_thread *thr = new pd_thread(cp->threads[j], this);
         addThread(thr);
      }
   }
}

pd_process::~pd_process() {
   cpuTimeMgr->destroyMechTimers(this);

   delete dyninst_process;
   dyninst_process = NULL;

   delete sharedMemManager;
   sharedMemManager = NULL;

   delete theVariableMgr;
   theVariableMgr = NULL;
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

   if(isTerminated()) {
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

   const timeStamp theProcTime = getCpuTime();
   const timeStamp curWallTime = getWallTime();

   // need to check this again, process could have execed doMajorSample
   // and it may be midway through setting up for the exec
   if( !isBootstrappedYet() || !isPARADYNBootstrappedYet()) {
      return false;
   }

   // Now sample the observed cost.
   const unsigned theCost = *(shmMetaData->getObservedCost());

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

void pd_process::paradynExitDispatch(BPatch_thread *thread, 
                                     BPatch_exitType exit_type) {
   pd_process *matching_pd_process = getProcMgr().find_pd_process(thread);
   int code = 0;
   assert(thread->terminationStatus() == exit_type);

   if(exit_type == ExitedNormally) {      
      code = thread->getExitCode();
   } else if(exit_type == ExitedViaSignal) {
      code = - (thread->getExitSignal());
   } else   assert(false);

   if(matching_pd_process)
      matching_pd_process->handleExit(code);
}

void pd_process::preForkHandler() {
  // Nothing to do here...
}

// this is the parent
void pd_process::postForkHandler(BPatch_thread *child) {
   BPatch_thread *parent = dyninst_process;
   process *parentDynProc = parent->lowlevel_process();
   process *childDynProc  = child->lowlevel_process();
   assert(childDynProc->status() == stopped);

   pd_process *parentProc = 
     getProcMgr().find_pd_process(parentDynProc->getPid());
   if (!parentProc) {
     logLine("Error in forkProcess: could not find parent process\n");
     return;
   }

   pd_process *childProc = new pd_process(*parentProc, child);
   getProcMgr().addProcess(childProc);
   metricFocusNode::handleFork(parentProc, childProc);
   childDynProc->setParadynBootstrap();

   // I don't think we want to continue the process here... hand it back
   // to Dyninst -- bernat, 28APR04
   childProc->continueProc();
   // parent process will get continued by unix.C/handleSyscallExit
}

void pd_process::execHandler() {
    // We need to reload the Paradyn library
    paradynRTState = libUnloaded; // It was removed when we execed
    inExec = true;

    // create a new pd_image because there is a new dyninst image
    delete img;
    img = new pd_image(dyninst_process->getImage(), this);

    // The shared segment is gone... so delete and remake
    delete shmMetaData;
    delete sharedMemManager;

    // Sigh... the library's gone as well

    sharedMemManager = new shmMgr(dyninst_process,
                                  7000, // Arbitrary constant for shared key
                                  SHARED_SEGMENT_SIZE,
                                  false // Don't leave around -- if we're attached, then the
                                  // inferior process will have the segment as long is it 
                                  // exists
                                  );
    shmMetaData = new sharedMetaData(sharedMemManager, MAX_NUMBER_OF_THREADS);

    // Initialize shared memory before we re-load the paradyn lib
    if (!sharedMemManager->initialize()) {
      fprintf(stderr, "%s[%d]:  failed to init shared mem mgr, fatal...\n", __FILE__, __LINE__);
      return;
    }

    loadParadynLib(exec_load);
}

/********************************************************************
 **** Paradyn runtime library code                               ****    
 ********************************************************************/

// Load and initialize the paradyn runtime library.
bool pd_process::loadParadynLib(load_cause_t ldcause) 
{
#if defined(sparc_sun_solaris2_4)
    // Sparc requires libsocket to be loaded before the paradyn lib can be :/
    loadAuxiliaryLibrary("libsocket.so");
#endif

    assert(isStopped());

    pdstring buffer = pdstring("PID=") + pdstring(getPid());
    buffer += pdstring(", loading Paradyn RT lib via iRPC");
    statusLine(buffer.c_str());

    setLibState(paradynRTState, libLoading);
#if defined (i386_unknown_nt4_0)
     // Another FIXME: NT strips the path from the loaded
     // library for recognition purposes.
     char dllFilename[_MAX_FNAME];
     _splitpath (paradynRTname.c_str(),
            NULL, NULL, dllFilename, NULL);
    if (!dyninst_process->loadLibrary(dllFilename)) {
      fprintf(stderr, "%s[%d]:  failed to load %s, fatal...\n",
              __FILE__, __LINE__, dllFilename);
      return false;
    }
    fprintf(stderr, "%s[%d]:  loaded %s\n", __FILE__, __LINE__, dllFilename);
#else

    if (!dyninst_process->loadLibrary(paradynRTname.c_str())) {
      fprintf(stderr, "%s[%d]:  failed to load %s, fatal...\n",
              __FILE__, __LINE__, paradynRTname.c_str());
      return false;
    }
#endif
    if (!setParadynLibParams(ldcause)) {
      fprintf(stderr, "%s[%d]:  failed set lib params for %s, fatal...\n",
              __FILE__, __LINE__, paradynRTname.c_str());
      return false;
    }

    setLibState(paradynRTState, libLoaded);

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
    Symbol sym;
    // TODO: this shouldn't be paradynPid, it should be traceConnectInfo
    int paradynPid = traceConnectInfo;
    // UNIX: the daemon's PID. NT: a socket.

    // We can now initialize the shared metadata, since the library exists
    if (!shmMetaData->initialize()) {
        assert(0 && "Failed to allocate required shared metadata variables");
    }
    
    BPatch_image *appImage = dyninst_process->getImage();
    assert(appImage); 

    const char *vname = "libparadynRT_init_localparadynPid";
    BPatch_variableExpr *v_pid = appImage->findVariable(vname);
    if (! v_pid) {
      fprintf(stderr, "%s[%d]:  could not find var named %s\n", __FILE__, __LINE__, vname);
      assert(0  && "fatal init error");
    }
    if (! v_pid->writeValue((void *) &paradynPid)) {
      fprintf(stderr, "%s[%d]:  could not write var named %s\n", __FILE__, __LINE__, vname);
      assert(0  && "fatal init error");
    }

    int creationMethod;
    if(ldcause == create_load)      creationMethod = 0;
    else if(ldcause == attach_load) creationMethod = 1;
    else if(ldcause == exec_load)   creationMethod = 4;
    else assert(0);

    vname = "libparadynRT_init_localcreationMethod";
    BPatch_variableExpr *v_cm = appImage->findVariable(vname);
    if (! v_cm) {
      fprintf(stderr, "%s[%d]:  could not find var named %s\n", __FILE__, __LINE__, vname);
      assert(0  && "fatal init error");
    }
    if (! v_cm->writeValue((void *) &creationMethod)) {
      fprintf(stderr, "%s[%d]:  could not write var named %s\n", __FILE__, __LINE__, vname);
      assert(0  && "fatal init error");
    }


    int maxThreads = maxNumberOfThreads();

    vname = "libparadynRT_init_localmaxThreads";
    BPatch_variableExpr *v_maxt = appImage->findVariable(vname);
    if (! v_maxt) {
      fprintf(stderr, "%s[%d]:  could not find var named %s\n", __FILE__, __LINE__, vname);
      assert(0  && "fatal init error");
    }
    if (! v_maxt->writeValue((void *) &maxThreads)) {
      fprintf(stderr, "%s[%d]:  could not write var named %s\n", __FILE__, __LINE__, vname);
      assert(0  && "fatal init error");
    }

    
    Address virtTimers = getSharedMemMgr()->daemonToApplic((Address)shmMetaData->getVirtualTimers());

    vname = "libparadynRT_init_localVirtualTimers";
    BPatch_variableExpr *v_vts = appImage->findVariable(vname);
    if (! v_vts) {
      fprintf(stderr, "%s[%d]:  could not find var named %s\n", __FILE__, __LINE__, vname);
      assert(0  && "fatal init error");
    }
    if (! v_vts->writeValue((void *) &virtTimers)) {
      fprintf(stderr, "%s[%d]:  could not write var named %s\n", __FILE__, __LINE__, vname);
      assert(0  && "fatal init error");
    }

    // And now that we have our new (shared) observed cost address, update the
    // internal process one
    Address obsCostInApplic = getSharedMemMgr()->daemonToApplic((Address)shmMetaData->getObservedCost());

    dyninst_process->lowlevel_process()->updateObservedCostAddr(obsCostInApplic);

    vname = "libparadynRT_init_localObservedCost";
    BPatch_variableExpr *v_obsCost = appImage->findVariable(vname);
    if (! v_obsCost) {
      fprintf(stderr, "%s[%d]:  could not find var named %s\n", __FILE__, __LINE__, vname);
      assert(0  && "fatal init error");
    }
    if (! v_obsCost->writeValue((void *) &obsCostInApplic)) {
      fprintf(stderr, "%s[%d]:  could not write var named %s\n", __FILE__, __LINE__, vname);
      assert(0  && "fatal init error");
    }

    return true;
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
    if (bs_record.event == 0) {
        return false;
    }
    
    assert(bs_record.pid == getPid());
    
    const bool calledFromFork   = (bs_record.event == 2);
    const bool calledFromExec   = (bs_record.event == 4);
    //const bool calledFromAttach = (bs_record.event == 3);

    // Override tramp guard address
    process *llproc = dyninst_process->lowlevel_process();
    llproc->setTrampGuardBase((Address) bs_record.tramp_guard_base);

    // Override thread index address
    llproc->updateThreadIndexAddr((Address) bs_record.thread_index_base);

    if (!calledFromFork) {
        // MT: need to set Paradyn's bootstrap state or the instrumentation
        // basetramps will be created in ST mode
        llproc->setParadynBootstrap();

        // Install initial instrumentation requests
        extern pdvector<pdinstMapping*> initialRequestsPARADYN; // init.C //ccw 18 apr 2002 : SPLIT
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
    assert(isStopped());
    
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
       if(hasExited()) {
           fprintf(stderr, "%s[%d]:  bootstrap aborted, appl. terminated\n", __FILE__, __LINE__);
           return false;
       }
        launchRPCs(false);
        if (!reachedLibState(paradynRTState, libReady))
          getSH()->checkForAndHandleProcessEvents(true);
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

    BPatch_image *appImage = dyninst_process->getImage();
    assert(appImage);

    const char *vname = "PARADYN_bootstrap_info";
    BPatch_variableExpr *v_bs = appImage->findVariable(vname);
    if (! v_bs) {
      fprintf(stderr, "%s[%d]:  could not find var named %s\n", 
              __FILE__, __LINE__, vname);
      assert(0  && "fatal init error");
    }
    if (! v_bs->readValue((void *) bs_record, sizeof(*bs_record))) {
      fprintf(stderr, "%s[%d]:  could not read var named %s\n", 
              __FILE__, __LINE__, vname);
      return false;
    }

#if 0 
    // DEPRECATED

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
    }
#endif    
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
        getSH()->checkForAndHandleProcessEvents(true);
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
   if(isTerminated()) {
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
   if(lwp == time_for_whole_program) {
      // get cpu time for whole process
      return getAllLwpRawCpuTime_hw();
   } else {
      dyn_lwp *thelwp = dyninst_process->lowlevel_process()->lookupLWP(lwp);
      return thelwp->getRawCpuTime_hw();
   }
}

rawTime64 pd_process::getRawCpuTime_sw(int lwp)
{
   if(lwp == time_for_whole_program) {
      // get cpu time for whole process
      return getAllLwpRawCpuTime_sw();      
   } else {
      dyn_lwp *thelwp = dyninst_process->lowlevel_process()->lookupLWP(lwp);
      return thelwp->getRawCpuTime_sw();
   }
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
   int appAddrWidth = getImage()->getAddressWidth();

   BPatch_image *appImage = dyninst_process->getImage();
   assert(appImage);

   const char *vname = "hintBestCpuTimerLevel";
   BPatch_variableExpr *v_hint = appImage->findVariable(vname);
   if (! v_hint) {
     fprintf(stderr, "%s[%d]:  could not find var named %s\n", 
             __FILE__, __LINE__, vname);
     assert(0  && "fatal init error");
   }
   if (! v_hint->readValue((void *) &hintBestCpuTimerLevel, appAddrWidth)) {
      fprintf(stderr, "%s[%d]:  could not read var named %s\n", 
              __FILE__, __LINE__, vname);
      //return;
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

   vname = "hintBestWallTimerLevel";
   v_hint = appImage->findVariable(vname);
   if (! v_hint) {
     fprintf(stderr, "%s[%d]:  could not find var named %s\n", __FILE__, __LINE__, vname);
     assert(0  && "fatal init error");
   }
   if (! v_hint->readValue((void *) &hintBestWallTimerLevel, appAddrWidth)) {
      fprintf(stderr, "%s[%d]:  could not read var named %s\n", __FILE__, 
              __LINE__, vname);
      //return;
   }

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
   Address transferAddr = 0;
   int appAddrWidth = getImage()->getAddressWidth();

   BPatch_image *appImage = dyninst_process->getImage();
   assert(appImage);

   const char *vname = helperFPtr;
   BPatch_variableExpr *v_hint = appImage->findVariable(vname);
   if (! v_hint) {
     fprintf(stderr, "%s[%d]:  could not find var named %s\n",
             __FILE__, __LINE__, vname);
     assert(0  && "fatal internal error");
   }
   if (! v_hint->readValue((void *) &transferAddr, appAddrWidth)) {
      fprintf(stderr, "%s[%d]:  could not read var named %s\n",
              __FILE__, __LINE__, vname);
      //return;
   }

   return transferAddr;
}

bool pd_process::writeTimerFuncAddr_(const char *rtinstVar,
				   const char *rtinstHelperFPtr)
{
   Address rtfuncAddr = getTimerQueryFuncTransferAddress(rtinstHelperFPtr);
   BPatch_image *appImage = dyninst_process->getImage();
   assert(appImage);

   const char *vname = rtinstVar;
   BPatch_variableExpr *v_funcaddr = appImage->findVariable(vname);
   if (! v_funcaddr) {
     fprintf(stderr, "%s[%d]:  could not find var named %s\n",
             __FILE__, __LINE__, vname);
     assert(0  && "fatal internal error");
   }
   if (! v_funcaddr->writeValue((void *) &rtfuncAddr, sizeof(rtfuncAddr),
                                 false /*saveWorld*/)) {
      fprintf(stderr, "%s[%d]:  could not write var named %s\n",
              __FILE__, __LINE__, vname);
      return false;
   }

   return true;
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
extern void AddCallGraphStaticChildrenCallback(pdstring exe_name, pdstring r,
					       const pdvector<pdstring> children);

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

  BPatch_Vector<BPatch_function *> entry_bpfs;
  BPatch_function *entry_bpf;
  if ((!img->get_dyn_image()->findFunction("main", entry_bpfs))
      || !entry_bpfs.size()) abort();
  if (entry_bpfs.size() > 1) {
    //  maybe we should warn here?
  }
  entry_bpf = entry_bpfs[0];

  
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

  
  resource *entry_res = pd_module::getFunctionResource(entry_bpf);
  if (entry_res)
     CallGraphSetEntryFuncCallback(img->get_file(), entry_res->full_name(), thr);
    
  // build call graph for executable
  img->FillInCallGraphStatic(this);
  // build call graph for module containing entry point
  // ("main" is not always defined in the executable)

  pd_image *pd_main_img = pd_image::get_pd_image(entry_bpf->getModule());
  if (pd_main_img != img)
     pd_main_img->FillInCallGraphStatic(this);

  // TODO: build call graph for all shared objects?


  CallGraphFillDone(img->get_file());
}

void pd_process::MonitorDynamicCallSites(pdstring function_name) {
   if (!monitorFunc) {
     BPatch_Vector<BPatch_function *> monFuncs;
     if ((!img->get_dyn_image()->findFunction("DYNINSTRegisterCallee",monFuncs))
        || !monFuncs.size()) {
       fprintf(stderr, "%s[%d]:  cannot find function DYNINSTRegisterCallee\n",
              __FILE__, __LINE__);
       return;
     }
      if (monFuncs.size() > 1) {
        //  maybe we should warn here?
      }
      monitorFunc = monFuncs[0];
   }
   assert(monitorFunc);
   resource *r, *p;
   BPatch_module *mod;
   r = resource::findResource(function_name);
   assert(r);
   p = r->parent();
   assert(p);

   mod = findModule(p->name(), true);
   if(!mod) {
      //Must be the weird case where main() isn't in the executable

      BPatch_Vector<BPatch_function *> entry_bpfs;
      BPatch_function *entry_bpf;
      if ((!img->get_dyn_image()->findFunction("main", entry_bpfs))
        || !entry_bpfs.size()) abort();
      if (entry_bpfs.size() > 1) {
        //  maybe we should warn here?
      }
      entry_bpf = entry_bpfs[0];
      mod = entry_bpf->getModule();
   }
   assert(mod);
  
   BPatch_function *func;
   BPatch_Vector<BPatch_function *> fbv;
   if (NULL == mod->findFunction(r->name().c_str(), fbv) || !fbv.size()) {
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

   BPatch_Vector<BPatch_point*> *callPoints;
   callPoints = func->findPoint(BPatch_subroutine);

   bool needToCont = false;  
   if(!(isStopped() || isTerminated()) && callPoints->size() > 0) {
      // going to insert instrumentation, pause it from up here.
      // pausing at lower levels can cause performance problems, particularly
      // on ptrace systems, where pauses are slow
      if(pause() == true)
         needToCont = true;
   }
  
   for(unsigned i = 0; i < callPoints->size(); i++) {

      BPatch_function *called_func;
      if (NULL == (called_func = (*callPoints)[i]->getCalledFunction())){

        if ((*callPoints)[i]->isDynamic())
          if (!(*callPoints)[i]->monitorCalls(monitorFunc)) {
            fprintf(stderr,
              "%s[%d]:ERROR in daemon, unable to monitorCallSite for function :%s\n",
                    __FILE__, __LINE__,function_name.c_str());
          }

#ifdef NOTDEF // PDSEP
         process *llproc = get_dyn_process()->lowlevel_process();
         if(! llproc->MonitorCallSite((*callPoints)[i]->PDSEP_instPoint())) {
            fprintf(stderr,
              "ERROR in daemon, unable to monitorCallSite for function :%s\n",
                    function_name.c_str());
         }
#endif
      }
   }

   if(needToCont)
      continueProc();
}

bool reachedLibState(libraryState_t lib, libraryState_t state) {
   return (lib >= state);
}

virtualTimer *pd_process::getVirtualTimer(unsigned index) {
    virtualTimer *virt_base = (virtualTimer *)shmMetaData->getVirtualTimers();
    return &(virt_base[index]);
}

      // First, define a numeric mapping on top of instrumentation
      // points:

      // 0: Before base tramp
      // 1: In the jump to base tramp
      // 2: base tramp between entry and the call to preInsn
      //    minitramps 
      // 3: preInsn minitramps 
      // 4: base tramp between preInsn minitramps and postInsn
      //    minitramps
      // 5: postInsn minitramps 
      // 6: base tramp between postInsn minitramps and the end of
      //    the base tramp
      // 7: after base tramp

      // This simplification works because we only allow adding
      // minitramps to the beginning or end of the current tramp
      // chain (between 1 and 2 or 2 and 3... but not within 2)

typedef enum {beforeInstru, baseEntry, preInsn, 
	      emulInsns, postInsn, baseExit, afterInstru, nowhere} logicalPCLocation_t;

bool pd_process::triggeredInStackFrame(Frame &frame,
                                       BPatch_point *bpPoint,
                                       BPatch_callWhen when,
                                       BPatch_snippetOrder order) {
    fprintf(stderr, "WARNING: skipping catchup!\n");
    return false;

#if 0
  
    // Use a previous lookup if it's there.
    codeRange *range = frame.getRange();
    
    // We still use int_function and instPoint. TODO: replace frame
    // with the BPatch frame, int_function with BPatch_function, etc.

    instPoint *point = bpPoint->PDSEP_instPoint();
    

    // Most of the checking is based on "Am I before, after, or during the
    // instrumentation point". We check that first to see if we need to care.
    Address collapsedFrameAddr;

    // Test 1: if the function associated with the frame
    // is not the instPoint function, return false immediately.
    int_function *func_ptr = range->is_function();
    miniTrampHandle *minitramp_ptr = range->is_minitramp();
    trampTemplate *basetramp_ptr = range->is_basetramp();
    relocatedFuncInfo *reloc_ptr = range->is_relocated_func();
    edgeTrampTemplate *edge_ptr = range->is_edge_tramp();
    multitrampTemplate *multitramp_ptr = range->is_multitramp();

    int_function *frame_func;

    if(func_ptr) {
        collapsedFrameAddr = frame.getPC();
	frame_func = func_ptr;
        if(pd_debug_catchup)
           fprintf(stderr, "     PC in function %s...",
                   func_ptr->prettyName().c_str());
    }
    else if(minitramp_ptr) {
        // Again, quick check for function matching
        trampTemplate *baseT = minitramp_ptr->baseTramp;
        const instPoint *instP = baseT->location;
	
        collapsedFrameAddr = instP->absPointAddr(dyninst_process->lowlevel_process());
	frame_func = instP->pointFunc();

        if(pd_debug_catchup)
           fprintf(stderr, "     PC in minitramp at 0x%lx (%s)...",
                   collapsedFrameAddr,
                   instP->pointFunc()->prettyName().c_str());
    }
    else if(basetramp_ptr) {
        const instPoint *instP = basetramp_ptr->location;
        collapsedFrameAddr = instP->absPointAddr(dyninst_process->lowlevel_process());
	frame_func = instP->pointFunc();

        if(pd_debug_catchup)
           fprintf(stderr,"     PC in base tramp at 0x%lx (%s)...",
                   collapsedFrameAddr,
                   instP->pointFunc()->prettyName().c_str());
    }
    else if (reloc_ptr) {
       frame_func = reloc_ptr->func();
        collapsedFrameAddr = frame.getPC();

       if(pd_debug_catchup)
          fprintf(stderr, "      PC in relocated function (%s)...",
                  frame_func->prettyName().c_str());
    }
    else if (edge_ptr) {
      frame_func = dyninst_process->lowlevel_process()->findFuncByAddr(edge_ptr->addrInFunc);
      collapsedFrameAddr = edge_ptr->addrInFunc;
       if(pd_debug_catchup)
          fprintf(stderr, "      PC in edge tramp...");
    }
    else if (multitramp_ptr) {
        const instPoint *instP = multitramp_ptr->location;
        collapsedFrameAddr = instP->absPointAddr(dyninst_process->lowlevel_process());
	frame_func = instP->pointFunc();

        if(pd_debug_catchup)
           fprintf(stderr,"     PC in multitramp at 0x%lx (%s)...",
                   collapsedFrameAddr,
                   instP->pointFunc()->prettyName().c_str());
    }
    else {
        // Uhh... no function, no point... murph?
        // Could be top of the stack -- often address 0x0 or -1
        if (frame.getPC() && pd_debug_catchup) 
            fprintf(stderr, "     Couldn't find match for address 0x%lx!\n",
                    frame.getPC());
        return false;
    }

    if (frame_func != point->pointFunc()) {
      if(pd_debug_catchup)
         fprintf(stderr, "     Current function %s not instPoint function, returning false\n======\n",
		frame_func->prettyName().c_str());
      return false;
    }

    bool catchupNeeded = false;
    
    if (pd_debug_catchup)
      fprintf(stderr, " CFA 0x%lx, ", collapsedFrameAddr);

    if (bpPoint->getPointType() != BPatch_locLoopEntry &&
	bpPoint->getPointType() != BPatch_locLoopStartIter) {
      // We have a "collapsed" (i.e. within the function area) version
      // of the current PC. Do the same to the minitramp (instPoint/when/order)
      // passed in, and compare. 
      
      // We do handling a little differently depending on the type of inst
      // point. This takes advantage of the behavior of our inst points. So we 
      // break it up here.
      
      Address pointAddr = point->absPointAddr(dyninst_process->lowlevel_process());
      
      // addr in edge tramp then use addr which jumps to edge tramp
      if (point->addrInFunc != 0)
	pointAddr = point->addrInFunc;

      if (pd_debug_catchup)
         fprintf(stderr, "PA 0x%lx, ", pointAddr);
      
      logicalPCLocation_t location = nowhere;
      
      if (collapsedFrameAddr < pointAddr) {
        // Haven't reached the point yet
	location = beforeInstru;
      }
      else if (collapsedFrameAddr > pointAddr) {
	// If this instPoint is for function _exit_ and we're after the point, DO NOT
	// return true -- it is possible for an exit point to be in the middle of the
	// function
	location = afterInstru;
      }
      else {
	// They're both in the same point... break down further
	if (func_ptr) {
	  // We're not in a base or minitramp, but addresses match. Therefore
	  // we're at the jump to the base tramp
	  location = baseEntry;
	}
	else if (basetramp_ptr) {
	  if (frame.getPC() <= (basetramp_ptr->get_address() +
				basetramp_ptr->localPreOffset)) {
	    // From start of base tramp to beginning of instrumentation
	    location = baseEntry;
	  }
	  else if (frame.getPC() >= (basetramp_ptr->get_address() +
				     basetramp_ptr->localPreReturnOffset) &&
		   frame.getPC() <= (basetramp_ptr->get_address() +
				     basetramp_ptr->localPostOffset)) {
	    // Between pre and post...
	    location = emulInsns;
	  }
	  else if (frame.getPC() >= (basetramp_ptr->get_address() +
				     basetramp_ptr->localPostReturnOffset)) {
	    // After post
	    location = baseExit;
	  }
	  else {
	    assert(0 && "Unknown address in base tramp");
	  }
	}
	else if (minitramp_ptr) {
	  if (minitramp_ptr->when == callPreInsn) {
	    // Middle of pre instrumentation
	    location = preInsn;
	  }
	  else if (minitramp_ptr->when == callPostInsn) {
	    // Middle of post instrumentation
	    location = postInsn;
	  }
	  else
	    assert(0 && "Unknown minitramp callWhen");
	}
      } // addrs equal
      
      if (pd_debug_catchup) {
         switch(location) {
            case beforeInstru:
               fprintf(stderr, "beforeInstru...");
               break;
            case baseEntry:
               fprintf(stderr, "baseEntry...");
               break;
            case preInsn:
               fprintf(stderr, "preInsn...");
               break;
            case emulInsns:
               fprintf(stderr, "emulInsns...");
               break;
            case postInsn:
               fprintf(stderr, "postInsn...");
               break;
            case baseExit:
               fprintf(stderr, "baseExit...");
               break;
            case afterInstru:
               fprintf(stderr, "afterInstru...");
               break;
            case nowhere:
               fprintf(stderr, "serious problem with the compiler...");
               break;
         }    
      }
      // And now determine if we're before or after the point
      if (location == afterInstru) {
	// If we're looking at entry instrumentation,
	// we _want_ to do catchup -- we're in the function.
	// If we're dealing with an arbitrary point, assume that 
	// functions are linear.
	// If we're at anything else, we just missed the point
	// and don't do catchup.
	if (bpPoint->getPointType() == BPatch_locEntry)
	  catchupNeeded = true;
	else 
	  catchupNeeded = false;
      }
      // Otherwise, check by when/order/location
      else if (when == BPatch_callBefore) {
	if (order == BPatch_firstSnippet) {
	  // If we're after baseEntry, we missed it
	  catchupNeeded = (location > baseEntry);
	}
	else {
	  // If we're after preInsn, we missed it
	  catchupNeeded = (location > preInsn);
	}
      }
      else {
	// when == BPatch_callAfter
	if (order == BPatch_firstSnippet) {
	  catchupNeeded = (location > emulInsns);
	}
	else {
	  catchupNeeded = (location > postInsn);
	}
      } // callPostInsn
    }
    else {
      // LOOP CODE
      // Problem with loops is that they're non-contiguous. At least, 
      // we can't assume the easy case. So instead of a simple
      // "Before/After" split, we have an "In/Not In" split. 
      // First... get the loop body that we just instrumented.

      BPatch_basicBlockLoop *loop = bpPoint->getLoop();

      // Luckily, we can ask the loop if it contains the addr
      if (loop->containsAddressInclusive(collapsedFrameAddr))
	catchupNeeded = true;
      else
	catchupNeeded = false;
    }
      

    if (pd_debug_catchup) {
       if (catchupNeeded)
          fprintf(stderr, "catchup needed, ret true\n========\n");
       else
          fprintf(stderr, "catchup not needed, ret false\n=======\n");
    }
#endif
    bool catchupNeeded = false;
    return catchupNeeded;
}

BPatch_Vector<BPatch_function *> *pd_process::getIncludedFunctions(BPatch_module *mod)
{
    if (!img) return NULL;
    return img->getIncludedFunctions(mod);
}


BPatch_Vector<BPatch_function *> *pd_process::getIncludedFunctions()
{
   if (!img) return NULL;
   return img->getIncludedFunctions();
}

pdvector<BPatch_module *> *pd_process::getIncludedModules(pdvector<BPatch_module *> *buf)
{
   if (!img) return buf;
   return img->getIncludedModules(buf);
}

BPatch_module *pd_process::findModule(const pdstring &mod_name,bool check_excluded)
{
   if (!img) return NULL;
   return img->findModule(mod_name, check_excluded);

}

bool pd_process::installInstrRequests(const pdvector<pdinstMapping*> &requests)
{
  BPatch_image *appImage = img->get_dyn_image();
  BPatch_thread *appThread = get_dyn_process();
  bool err = false;

  for (unsigned i = 0; i < requests.size(); ++i) {
    pdinstMapping *req = requests[i];

    if (!multithread_capable() && req->is_MTonly())
       continue;
   
    //  used to split the func name into lib and func in some cases,
    //  not sure this is necessary anymore.
    BPatch_Vector<BPatch_function *> bpfv;
    if (!appImage->findFunction(req->func.c_str(), bpfv, !req->quiet_fail)
        || !bpfv.size()) {
      if (!req->quiet_fail) {
        fprintf(stderr, "%s[%d]:  cannot find function %s, instrRequest skipped\n",
               __FILE__, __LINE__, req->func.c_str());
      }
      err = true;
      continue;
    }    
    for (unsigned int j = 0; j < bpfv.size(); ++j) {
      BPatch_function *bpf = bpfv[j];
      if (!bpf) {
        fprintf(stderr, "%s[%d]:  BAD NEWS, got NULL BPatch_function for %s\n",
                __FILE__, __LINE__, req->func.c_str());
        err = true;
        continue;
      }
      BPatch_Vector<BPatch_function *> bpfv2;
      if (!appImage->findFunction(req->inst.c_str(), bpfv2)
          || !bpfv2.size()) {
         if (!req->quiet_fail)
           fprintf(stderr, "%s[%d]:  cannot find function %s, instrRequest skipped\n",
                   __FILE__, __LINE__, req->inst.c_str());
         err = true;
         continue;
      }
      if (bpfv2.size() > 1) {
        if (!req->quiet_fail)
          fprintf(stderr, "%s[%d]: %d matches for function %s, using the first\n",
                 __FILE__, __LINE__, bpfv2.size(), req->inst.c_str());
      }
      BPatch_function *bpf_inst = bpfv2[0];
      BPatch_snippet *snip;

      if ((req->where & FUNC_ARG) && req->args.size()) {
         snip = new BPatch_funcCallExpr(*bpf_inst, req->args);
      } else {
         BPatch_constExpr *tmp = new BPatch_constExpr(0);
         BPatch_Vector<BPatch_snippet *> tmp_args;
         tmp_args.push_back(tmp);
         snip = new BPatch_funcCallExpr(*bpf_inst, tmp_args);
         delete tmp; // is this safe ?
      }

      if (req->where & FUNC_EXIT) {
         BPatch_Vector<BPatch_point *> *exit_points = bpf->findPoint(BPatch_exit);
         if ((!exit_points) || !exit_points->size()) {
           if (!req->quiet_fail)
             fprintf(stderr, "%s[%d]:  function %s has no exit points, %s\n",
                     __FILE__, __LINE__, req->func.c_str(),
                     "cannot perform instrumentation request.");
           err = true;
         }
         else {
           for (unsigned k = 0; k < exit_points->size(); ++k) {
             BPatch_point *exit_pt = (*exit_points)[k];
             BPatchSnippetHandle *snipHandle;
             //  allow_trap is always true with insertSnippet
             //  useTrampGuard is generally controlled by BPatch::isTrampRecursive
             snipHandle = appThread->insertSnippet(*snip, *exit_pt,
                                                   req->when, req->order);
             if (NULL == snipHandle) {
               //  this request failed, but keep going...
               //  QUESTION:  should we add the NULL to req->snippetHandles so that
               //  a 1-1 mapping between requests and results is maintained?
               //if (!req->quiet_fail) 
                 fprintf(stderr, "%s[%d]:  failed to insert inst request for %s exit\n",
                        __FILE__, __LINE__, req->func.c_str());
               err = true;
             }
             else {
               //  add returned handle to the request class and keep going...
               req->snippetHandles.push_back(snipHandle);
             }
           } // for k
        }
      }

      if (req->where & FUNC_ENTRY) {
         BPatch_Vector<BPatch_point *> *entry_points = bpf->findPoint(BPatch_entry);
         if ((!entry_points) || !entry_points->size()) {
           if (!req->quiet_fail) 
             fprintf(stderr, "%s[%d]:  function %s has no entry points, %s\n",
                     __FILE__, __LINE__, req->func.c_str(),
                     "cannot perform instrumentation request.");
           err = true;
         }
         else {
           BPatch_point *entry_pt = (*entry_points)[0];
           BPatchSnippetHandle *snipHandle;
           //  allow_trap is always true with insertSnippet
           //  useTrampGuard is generally controlled by BPatch::isTrampRecursive
           snipHandle = appThread->insertSnippet(*snip, *entry_pt,
                                                 req->when, req->order);
           if (NULL == snipHandle) {
             //  this request failed, but keep going...
             //  QUESTION:  should we add the NULL to req->snippetHandles so that
             //  a 1-1 mapping between requests and results is maintained?
             if (!req->quiet_fail)
               fprintf(stderr, "%s[%d]:  failed to insert inst request for %s entry\n",
                      __FILE__, __LINE__, req->func.c_str());
             err = true;
           }
           else {
             //  add returned handle to the request class and keep going...
             req->snippetHandles.push_back(snipHandle);
           }
        }
      }

      if (req->where & FUNC_CALL) {
         BPatch_Vector<BPatch_point *> *call_points = bpf->findPoint(BPatch_subroutine);
         if ((!call_points) || !call_points->size()) {
           if (!req->quiet_fail)
             fprintf(stderr, "%s[%d]:  function %s has no call points, %s\n",
                     __FILE__, __LINE__, req->func.c_str(),
                     "cannot perform instrumentation request.");
           err = true;
         }
         else {
           for (unsigned k = 0; k < call_points->size(); ++k) {
             BPatch_point *call_pt = (*call_points)[k];
             BPatchSnippetHandle *snipHandle;
             //  allow_trap is always true with insertSnippet
             //  useTrampGuard is generally controlled by BPatch::isTrampRecursive
             snipHandle = appThread->insertSnippet(*snip, *call_pt,
                                                   req->when, req->order);
             if (NULL == snipHandle) {
               //  this request failed, but keep going...
               //  QUESTION:  should we add the NULL to req->snippetHandles so that
               //  a 1-1 mapping between requests and results is maintained?
               if (!req->quiet_fail)
                 fprintf(stderr, "%s[%d]:  failed to insert inst request for %s call (%s)\n",
                        __FILE__, __LINE__, req->func.c_str(), req->inst.c_str());
               err = true;
             }
             else {
               //  add returned handle to the request class and keep going...
               req->snippetHandles.push_back(snipHandle);
             }
           } // for k
        }
      }

      delete snip;
    } // for j
  } // for i

  return err;
}

bool pd_process::pause() 
{
  do {
     if (!dyninst_process->stopExecution())
        return false;
     if (dyninst_process->isTerminated())
        return false;
  } while (!dyninst_process->isStopped());

  return true;
}

bool pd_process::findAllFuncsByName(resource *func, resource *mod,
                           BPatch_Vector<BPatch_function *> &res) {
     const pdvector<pdstring> &f_names = func->names();
     const pdvector<pdstring> &m_names = mod->names();
     pdstring func_name = f_names[f_names.size() -1];
     pdstring mod_name = m_names[m_names.size() -1];
     BPatch_Vector<BPatch_module *> *mods = dyninst_process->getImage()->getModules();
     assert(mods);
     for (unsigned int i = 0; i < mods->size(); ++i) {
       char nbuf[512];
       (*mods)[i]->getName(nbuf, 512);
       if (!strcmp(nbuf, mod_name.c_str())) {
         BPatch_module *target_mod = (*mods)[i];
         BPatch_Vector<BPatch_function *> modfuncs;
         if (NULL == target_mod->findFunction(func_name.c_str(), res, 
                                              false, false, true, true)
            || !res.size()) {
            return false;
         }
         return true;
       }
     }

     BPatch_image *appImage = dyninst_process->getImage();
     if (NULL == appImage->findFunction(func_name.c_str(), res, false)
        || !res.size()) {
          //fprintf(stderr, "%s[%d]: function %s not found in image\n",
          //        __FILE__, __LINE__, func_name.c_str());
       return false;
     }
     return true;
}

bool pd_process::isStopped() const {return dyninst_process->isStopped();}
bool pd_process::isTerminated() const {return dyninst_process->isTerminated();}
bool pd_process::isDetached() const {return dyninst_process->isDetached();}
bool pd_process::continueProc() { 
  return dyninst_process->continueExecution(); 
}
