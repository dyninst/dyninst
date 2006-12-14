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

#ifdef sparc_sun_solaris2_4
#include <dlfcn.h>
#endif

#define BPATCH_FILE


#include "process.h"
#include "EventHandler.h"
#include "mailbox.h"
#include "signalgenerator.h"
#include "inst.h"
#include "instP.h"
#include "instPoint.h"
#include "function.h" // int_function
#include "codeRange.h"
#include "dyn_thread.h"
#include "miniTramp.h"

#include "mapped_module.h"

#include "BPatch_libInfo.h"
#include "BPatch_asyncEventHandler.h"
#include "BPatch.h"
#include "BPatch_thread.h"
#include "LineInformation.h"
#include "BPatch_function.h"
#include "callbacks.h"

#include "ast.h"

void BPatch_process::PDSEP_updateObservedCostAddr(unsigned long a)
{
  if (llproc)
    llproc->updateObservedCostAddr(a);
}

/*
 * BPatch_process::getImage
 *
 * Return the BPatch_image this object.
 */
BPatch_image *BPatch_process::getImageInt()
{
   return image;
}

int BPatch_process::getAddressWidthInt(){ 
	return llproc->getAddressWidth();
}

/*
 * BPatch_process::getPid
 *
 * Return the process ID of the thread associated with this object.
 */
int BPatch_process::getPidInt()
{
   return llproc ? (llproc->sh ? llproc->getPid()  : -1 ) : -1;
}

/*
 * BPatch_process::BPatch_process
 *
 * Starts a new process and associates it with the BPatch_process being
 * constructed.  The new process is placed into a stopped state before
 * executing any code.
 *
 * path		Pathname of the executable to start.
 * argv		A list of pointers to character strings which are the
 *              arguments for the new process, terminated by a NULL pointer.
 * envp		A list of pointers to character strings which are the
 *              environment variables for the new process, terminated by a
 *              NULL pointer.  If NULL, the default environment will be used.
 */
BPatch_process::BPatch_process(const char *path, const char *argv[], const char **envp,
                               int stdin_fd, int stdout_fd, int stderr_fd)
   : llproc(NULL), image(NULL), lastSignal(-1), exitCode(-1), 
     exitedNormally(false), exitedViaSignal(false), mutationsActive(true), 
     createdViaAttach(false), detached(false), unreportedStop(false), 
     unreportedTermination(false), terminated(false), unstartedRPC(false),
     activeOneTimeCodes_(0),
     resumeAfterCompleted_(false),
     pendingInsertions(NULL)
{
   func_map = new BPatch_funcMap();
   instp_map = new BPatch_instpMap();

   isVisiblyStopped = true;

   pdvector<pdstring> argv_vec;
   pdvector<pdstring> envp_vec;
   // Contruct a vector out of the contents of argv
   if (argv) {
      for(int i = 0; argv[i] != NULL; i++)
         argv_vec.push_back(argv[i]);
   }
    
   // Construct a vector out of the contents of envp
   if(envp) {
      for(int i = 0; envp[i] != NULL; ++i)
         envp_vec.push_back(envp[i]);
   }
   
   pdstring directoryName = "";

#if !defined(os_windows)
   // this fixes a problem on linux and alpha platforms where pathless
   // filenames are searched for either in a standard, predefined path, or
   // in $PATH by execvp.  thus paths that should resolve to "./" are
   // missed.  Note that the previous use of getcwd() here for the alpha
   // platform was dangerous since this is an API and we don't know where
   // the user's code will leave the cwd pointing.

   if (NULL == strchr(path, '/')) {
      const char *pathenv = getenv("PATH");
      char *pathenv_copy = strdup(pathenv);
      char *ptrptr;
      char *nextpath = strtok_r(pathenv_copy, ":", &ptrptr);
      while (nextpath) {
         struct stat statbuf;
         
         char *fullpath = new char[strlen(nextpath)+strlen(path)+2];
         strcpy(fullpath,nextpath);
         strcat(fullpath,"/");
         strcat(fullpath,path);
         
         if (!stat(fullpath,&statbuf)) {
            directoryName = nextpath;
            delete[] fullpath;
            break;
         }
         delete[] fullpath;
         nextpath = strtok_r(NULL,":", &ptrptr);
      }
      ::free(pathenv_copy);

      if (nextpath == NULL) {
         const char *dotslash = "./";
         directoryName = dotslash;
      }
   }
#endif

   /*
    * Set directoryName if a current working directory can be found in
    * the new process' environment (and override any previous settings).
    */
   if (envp) {
       for (int i = 0; envp[i] != NULL; ++i) {
           if (strncmp(envp[i], "PWD=", 4) == 0) {
               directoryName = envp[i] + 4;
               break;
           }
       }
   }
   
   llproc = ll_createProcess(path, &argv_vec, (envp ? &envp_vec : NULL), 
                             directoryName, stdin_fd, stdout_fd, stderr_fd);
   if (llproc == NULL) { 
      BPatch::bpatch->reportError(BPatchFatal, 68, 
           "Dyninst was unable to create the specified process");
      return;
   }
   startup_cerr << "Registering function callback..." << endl;
   llproc->registerFunctionCallback(createBPFuncCB);
   
   startup_cerr << "Registering instPoint callback..." << endl;
   llproc->registerInstPointCallback(createBPPointCB);
   llproc->container_proc = this;

   // Add this object to the list of processes
   assert(BPatch::bpatch != NULL);
   startup_cerr << "Registering process..." << endl;
   BPatch::bpatch->registerProcess(this);

   // Create an initial thread
   startup_cerr << "Getting initial thread..." << endl;
   dyn_thread *dynthr = llproc->getInitialThread();
   BPatch_thread *initial_thread = new BPatch_thread(this, dynthr);
   threads.push_back(initial_thread);

   startup_cerr << "Creating new BPatch_image..." << endl;
   image = new BPatch_image(this);

   assert(llproc->isBootstrappedYet());

   // Let's try to profile memory usage
#if defined(PROFILE_MEM_USAGE)
   void *mem_usage = sbrk(0);
   fprintf(stderr, "Post BPatch_process: sbrk %p\n", mem_usage);
#endif

   startup_cerr << "BPatch_process::BPatch_process, completed." << endl;
   isAttemptingAStop = false;
}

#if defined(os_linux)
/* Particular linux kernels running dyninst in particular patterns
   (namely, with a single process having spawned the mutator and the
   mutatee) are susceptible to a kernel bug that will cause a panic
   if the mutator exits before the mutatee. See the comment above
   class ForkNewProcessCallback : public DBICallbackBase in 
   debuggerinterface.h for details.
*/
bool LinuxConsideredHarmful(pid_t pid)
{
    int major, minor, sub, subsub; // version numbers
    pid_t my_ppid, my_pid, mutatee_ppid = 0;
    FILE *fd;
    char buf[1024];
    char filename[64];

    get_linux_version(major,minor,sub,subsub); 

    if( major == 2 && minor == 6 &&
        (sub < 11 || (sub == 11 && subsub <= 11)) )
    {
        my_ppid = getppid();
        my_pid = getpid();
        // If anybody knows a better way to get the parent pid, be my 
        // guest to change this.
        snprintf(filename, 64, "/proc/%d/status", pid);
        fd = fopen(filename, "r");
        if (!fd) {
            startup_printf("Failed to open %s, assuming no linux kernel bug\n",
                            filename);
            return false;
        }
        while (fgets(buf, 1024, fd)) { 
            if (strncmp(buf, "PPid", 4) == 0) {
                sscanf(buf, "%*s %d", &mutatee_ppid);
                break;
            }
        }
        fclose(fd);

        if(my_ppid == mutatee_ppid ||
           my_pid == mutatee_ppid)
            return true;
    }

    return false;
}
#endif
/*
 * BPatch_process::BPatch_process
 *
 * Constructs a new BPatch_process and associates it with a running process.
 * Stops execution of the process.
 *
 * path		Pathname of the executable file for the process.
 * pid		Process ID of the target process.
 */
BPatch_process::BPatch_process(const char *path, int pid)
   : llproc(NULL), image(NULL), lastSignal(-1), exitCode(-1), 
     exitedNormally(false), exitedViaSignal(false), mutationsActive(true), 
     createdViaAttach(true), detached(false), unreportedStop(false), 
     unreportedTermination(false), terminated(false), unstartedRPC(false),
     activeOneTimeCodes_(0),
     resumeAfterCompleted_(false),
     pendingInsertions(NULL)
{
   func_map = new BPatch_funcMap();
   instp_map = new BPatch_instpMap();

   isVisiblyStopped = true;

#if defined(os_linux)
    /* We need to test whether we are in kernel 2.6.9 - 2.6.11.11 (inclusive).
       If so, and if the mutatee's parent and our parent are one and the same,
       we are exposing the user to a potential kernel panic.
    */
    startup_printf("Checking for potential Linux kernel bug...\n");
    if(LinuxConsideredHarmful(pid))
    {
        fprintf(stderr,
            "\nWARNING: You are running a Linux kernel between 2.6.9 and \n"
            "2.6.11.11 (inclusive). Executing Dyninst under this kernel \n"
            "may exercise a bug in the Linux kernel and lead to a panic \n"
            "under some conditions. We STRONGLY suggest that you upgrade \n"
            "your kernel to 2.6.11.12 or higher.\n\n");
    }
#endif

   // Add this object to the list of threads
   assert(BPatch::bpatch != NULL);
   BPatch::bpatch->registerProcess(this, pid);

   image = new BPatch_image(this);

   llproc = ll_attachProcess(path, pid, this);
   if (!llproc) {
      BPatch::bpatch->unRegisterProcess(pid, this);
      BPatch::bpatch->reportError(BPatchFatal, 68, 
             "Dyninst was unable to attach to the specified process");
      return;
   }

   // Create an initial thread
   dyn_thread *dynthr = llproc->getInitialThread();
   BPatch_thread *initial_thread = new BPatch_thread(this, dynthr);
   threads.push_back(initial_thread);

   llproc->registerFunctionCallback(createBPFuncCB);
   llproc->registerInstPointCallback(createBPPointCB);
   llproc->container_proc = this;

   assert(llproc->isBootstrappedYet());
   assert(llproc->status() == stopped);

   isAttemptingAStop = false;
}

/*
 * BPatch_process::BPatch_process
 *
 * Constructs a new BPatch_process and associates it with a forked process.
 *
 * parentPid          Pathname of the executable file for the process.
 * childPid           Process ID of the target process.
 */
BPatch_process::BPatch_process(process *nProc)
   : llproc(nProc), image(NULL), lastSignal(-1), exitCode(-1),
     exitedNormally(false), exitedViaSignal(false), mutationsActive(true), 
     createdViaAttach(true), detached(false),
     unreportedStop(false), unreportedTermination(false), terminated(false),
     unstartedRPC(false), activeOneTimeCodes_(0),
     resumeAfterCompleted_(false),
     pendingInsertions(NULL)
{
   // Add this object to the list of threads
   assert(BPatch::bpatch != NULL);
   BPatch::bpatch->registerProcess(this);

   func_map = new BPatch_funcMap();
   instp_map = new BPatch_instpMap();

   // Create an initial thread
   dyn_thread *dynthr = llproc->getInitialThread();
   BPatch_thread *initial_thread = new BPatch_thread(this, dynthr);
   threads.push_back(initial_thread);

   llproc->registerFunctionCallback(createBPFuncCB);
   llproc->registerInstPointCallback(createBPPointCB);
   llproc->container_proc = this;

   image = new BPatch_image(this);
   isVisiblyStopped = true;
   isAttemptingAStop = false;
}

/*
 * BPatch_process::~BPatch_process
 *
 * Destructor for BPatch_process.  Detaches from the running thread.
 */
void BPatch_process::BPatch_process_dtor()
{
    
   if (!detached &&
       !getAsync()->detachFromProcess(this)) {
      bperr("%s[%d]:  trouble decoupling async event handler for process %d\n",
            __FILE__, __LINE__, getPid());
   }

   for (int i=threads.size()-1; i>=0; i--)
   {
      deleteBPThread(threads[i]);
   }

   if (image) 
      delete image;
   
   image = NULL;

   if (func_map)
      delete func_map;
   func_map = NULL;
   if (instp_map)
      delete instp_map;
   instp_map = NULL;

   if (pendingInsertions) {
       for (unsigned f = 0; f < pendingInsertions->size(); f++) {
           delete (*pendingInsertions)[f];
       }
       delete pendingInsertions;
       pendingInsertions = NULL;
   }

   if (!llproc) { 
      return; 
   }

   //  unRegister process before doing detach
   BPatch::bpatch->unRegisterProcess(getPid(), this);   

   /**
    * If we attached to the process, then we detach and leave it be,
    * otherwise we'll terminate it
    **/
   if (createdViaAttach) {
       llproc->detachProcess(true);
   }else  {
       if (llproc->isAttached()) {
           proccontrol_printf("%s[%d]:  about to terminate execution\n", __FILE__, __LINE__);
           terminateExecutionInt();
       }
   }
   
   delete llproc;
   llproc = NULL;
   assert(BPatch::bpatch != NULL);
}


/*
 * BPatch_process::stopExecution
 *
 * Puts the thread into the stopped state.
 */
bool BPatch_process::stopExecutionInt()
{

    if (isTerminated()) return true;

    if (isVisiblyStopped) return true;

    // We go to stop and get a callback in the middle...
    isAttemptingAStop = true;

   signal_printf("%s[%d]: entry to stopExecution, lock depth %d\n", FILE__, __LINE__, global_mutex->depth());

   while (lowlevel_process()->sh->isActivelyProcessing()) {
       lowlevel_process()->sh->waitForEvent(evtAnyEvent);
   }
  
   getMailbox()->executeCallbacks(FILE__, __LINE__);

   if (llproc->sh->pauseProcessBlocking()) {
       isVisiblyStopped = true;
       isAttemptingAStop = false;
       signal_printf("%s[%d]: exit of stopExecution, lock depth %d\n", FILE__, __LINE__, global_mutex->depth());
       return true;
   }
   else {
       return false;
       isAttemptingAStop = false;
   }
}

/*
 * BPatch_process::continueExecution
 *
 * Puts the thread into the running state.
 */
bool BPatch_process::continueExecutionInt()
{

    if (isTerminated()) {
        return true;
    }
    
    if (!llproc->reachedBootstrapState(bootstrapped_bs)) {
        return false;
    }

   //  maybe executeCallbacks led to the process execution status changing
   if (!statusIsStopped()) {
       isVisiblyStopped = false;
       llproc->sh->overrideSyncContinueState(runRequest);
       return true;
   }

   if (unstartedRPC) {
      //This shouldn't actually continue the process.  The BPatch state
      // should be stopped right now, and the low level code won't over-write
      // that.
      bool needsToRun = false;
      llproc->getRpcMgr()->launchRPCs(needsToRun, false);
      unstartedRPC = false;
   }

   //  DON'T let the user continue the process if we have potentially active 
   //  signal handling going on:
   // You know... this should really never happen. 

   // Just let them know we care...

   // Set isVisiblyStopped first... due to races (and the fact that CPBlocking gives
   // up the lock) we can hit a signal handler before this function returns...

   isVisiblyStopped = false;
   setUnreportedStop(false);

   bool ret =  llproc->sh->continueProcessBlocking();

   // Now here's amusing for you... we can hit a DyninstDebugBreakpoint
   // while continuing. That's handled in signalhandler.C
   return ret;
}


/*
 * BPatch_process::terminateExecution
 *
 * Kill the thread.
 */
bool BPatch_process::terminateExecutionInt()
{
   proccontrol_printf("%s[%d]:  about to terminate proc\n", FILE__, __LINE__);
   if (!llproc || !llproc->terminateProc())
      return false;
   while (!isTerminated()) {
       BPatch::bpatch->waitForStatusChangeInt();
   }
   
   return true;
}

/*
 * BPatch_process::statusIsStopped
 *
 * Returns true if the thread is stopped, and false if it is not.
 */
bool BPatch_process::statusIsStopped()
{
   return llproc->status() == stopped;
}

/*
 * BPatch_process::isStopped
 *
 * Returns true if the thread has stopped, and false if it has not.  This may
 * involve checking for thread events that may have recently changed this
 * thread's status.  This function also updates the unreportedStop flag if a
 * stop is detected, in order to indicate that the stop has been reported to
 * the user.
 */
bool BPatch_process::isStoppedInt()
{
    return isVisiblyStopped;

#if 0

   assert(BPatch::bpatch);
   if (statusIsStopped()) {
      //  if there are signal handler threads that are acting on this process
      //  that are not idle, we may not really be stopped from the end-user
      //  perspective (ie a continue might be imminent).
      if (llproc->sh->activeHandlerForProcess(llproc)) {
        signal_printf("%s[%d]:  pending events for proc %d, assuming still running\n",
                      FILE__, __LINE__, llproc->getPid());
        return false;
      }
      setUnreportedStop(false);
      return true;
   }
   

   return false;
#endif
}

/*
 * BPatch_process::stopSignal
 *
 * Returns the number of the signal which caused the thread to stop.
 */
int BPatch_process::stopSignalInt()
{
   if (llproc->status() != neonatal && llproc->status() != stopped) {
      fprintf(stderr, "%s[%d]:  request for stopSignal when process is %s\n",
              FILE__, __LINE__, llproc->getStatusAsString().c_str());
      return -1;
   } else
      return lastSignal;
}

/*
 * BPatch_process::statusIsTerminated
 *
 * Returns true if the process has terminated, false if it has not.
 */
bool BPatch_process::statusIsTerminated()
{
   if (llproc == NULL) {
     fprintf(stderr, "%s[%d]:  status is terminated becuase llproc is NULL\n", 
             FILE__, __LINE__);
     return true;
   }
   return llproc->hasExited();
}

/*
 * BPatch_process::isTerminated
 *
 * Returns true if the thread has terminated, and false if it has not.  This
 * may involve checking for thread events that may have recently changed this
 * thread's status.  This function also updates the unreportedTermination flag
 * if the program terminated, in order to indicate that the termination has
 * been reported to the user.
 */
bool BPatch_process::isTerminatedInt()
{
    getMailbox()->executeCallbacks(FILE__, __LINE__);
    // First see if we've already terminated to avoid 
    // checking process status too often.
    if (statusIsTerminated()) {
        proccontrol_printf("%s[%d]:  about to terminate proc\n", FILE__, __LINE__); 
        llproc->terminateProc();
        setUnreportedTermination(false);
        return true;
    }

    return false;
}

/*
 * BPatch_process::terminationStatus
 *
 * Indicates how the program exited.  Returns one of NoExit, ExitedNormally,
 * or ExitedViaSignal.
 *
 */
BPatch_exitType BPatch_process::terminationStatusInt() {
   if(exitedNormally)
      return ExitedNormally;
   else if(exitedViaSignal)
      return ExitedViaSignal;   
   return NoExit;
}

/*
 * BPatch_process::getExitCode
 *
 * Returns exit code of applications
 *
 */
int BPatch_process::getExitCodeInt() 
{
   return exitCode;
}

/*
 * BPatch_process::getExitSignal
 *
 * Returns signal number that caused application to exit.
 *
 */
int BPatch_process::getExitSignalInt()
{
   return lastSignal;
}

bool BPatch_process::wasRunningWhenAttachedInt()
{
  if (!llproc) return false;
  return llproc->wasRunningWhenAttached();
}

/*
 * BPatch_process::detach
 *
 * Detach from the thread represented by this object.
 *
 * cont		True if the thread should be continued as the result of the
 * 		detach, false if it should not.
 */
bool BPatch_process::detachInt(bool cont)
{
   //__UNLOCK;
   if (!getAsync()->detachFromProcess(this)) {
      bperr("%s[%d]:  trouble decoupling async event handler for process %d\n",
            __FILE__, __LINE__, getPid());
   }
  // __LOCK;
   detached = llproc->detachProcess(cont);
   return detached;
}

/*
 * BPatch_process::isDetaced
 *
 * Returns whether dyninstAPI is detached from this mutatee
 *
 */
bool BPatch_process::isDetachedInt()
{
   return detached;
}

/*
 * BPatch_process::dumpCore
 *
 * Causes the process to dump its state to a file, and optionally to terminate.
 * Returns true upon success, and false upon failure.
 *
 * file		The name of the file to which the state should be written.
 * terminate	Indicates whether or not the thread should be terminated after
 *		dumping core.  True indicates that it should, false that is
 *		should not.
 */
bool BPatch_process::dumpCoreInt(const char *file, bool terminate)
{
   bool had_unreportedStop = unreportedStop;
   bool was_stopped = isStopped();

   stopExecution();

   bool ret = llproc->dumpCore(file);
   if (ret && terminate) {
      fprintf(stderr, "%s[%d]:  about to terminate execution\n", __FILE__, __LINE__);
      terminateExecutionInt();
   } else if (was_stopped) {
    	unreportedStop = had_unreportedStop;
   } else {
      continueExecutionInt();
   }
    
   return ret;
}

/*
 * BPatch_process::dumpPatchedImage
 *
 * Writes the mutated file back to disk,
 * in ELF format.
 */
#if defined(os_solaris) || (defined(os_linux) && defined(arch_x86)) || defined(os_aix)
char* BPatch_process::dumpPatchedImageInt(const char* file)
{
   bool was_stopped = isStopped();
   bool had_unreportedStop = unreportedStop;
   
   stopExecution();
   char* ret = llproc->dumpPatchedImage(file);
   if (was_stopped) 
      unreportedStop = had_unreportedStop;
   else 
      continueExecutionInt();

   return ret;
   return NULL;
}
#else
char* BPatch_process::dumpPatchedImageInt(const char*)
{
   return NULL;
}
#endif

/*
 * BPatch_process::dumpImage
 *
 * Writes the contents of memory into a file.
 * Returns true upon success, and false upon failure.
 *
 * file		The name of the file to which the image should be written.
 */
bool BPatch_process::dumpImageInt(const char *file)
{
#if defined(os_windows)
   return false;
#else
   bool was_stopped;
   bool had_unreportedStop = unreportedStop;
   if (isStopped()) was_stopped = true;
   else was_stopped = false;

   stopExecutionInt();

   bool ret = llproc->dumpImage(file);
   if (was_stopped) 
      unreportedStop = had_unreportedStop;
   else 
      continueExecutionInt();

   return ret;
#endif
}

/*
 * BPatch_process::malloc
 *
 * Allocate memory in the thread's address space.
 *
 * n	The number of bytes to allocate.
 *
 * Returns:
 * 	A pointer to a BPatch_variableExpr representing the memory.
 *
 */
BPatch_variableExpr *BPatch_process::mallocInt(int n)
{
   assert(BPatch::bpatch != NULL);
   void *ptr = (void *) llproc->inferiorMalloc(n, dataHeap);
   if (!ptr) return NULL;
   return new BPatch_variableExpr(this, ptr, Null_Register, 
                                  BPatch::bpatch->type_Untyped);
}


/*
 * BPatch_process::malloc
 *
 * Allocate memory in the thread's address space for a variable of the given
 * type.
 *
 * type		The type of variable for which to allocate space.
 *
 * Returns:
 * 	A pointer to a BPatch_variableExpr representing the memory.
 *
 * XXX Should return NULL on failure, but the function which it calls,
 *     inferiorMalloc, calls exit rather than returning an error, so this
 *     is not currently possible.
 */
BPatch_variableExpr *BPatch_process::mallocByType(const BPatch_type &type)
{
   assert(BPatch::bpatch != NULL);
   BPatch_type &t = const_cast<BPatch_type &>(type);
   void *mem = (void *)llproc->inferiorMalloc(t.getSize(), dataHeap);
   if (!mem) return NULL;
   return new BPatch_variableExpr(this, mem, Null_Register, &t);
}


/*
 * BPatch_process::free
 *
 * Free memory that was allocated with BPatch_process::malloc.
 *
 * ptr		A BPatch_variableExpr representing the memory to free.
 */
bool BPatch_process::freeInt(BPatch_variableExpr &ptr)
{
   llproc->inferiorFree((Address)ptr.getBaseAddr());
   return true;
}

/*
 * BPatch_process::getInheritedVariable
 *
 * Allows one to retrieve a variable which exists in a child process that 
 * was inherited from and originally created in the parent process.
 * Function is invoked on the child BPatch_process (created from a fork in 
 * the application).
 *
 * parentVar   A BPatch_variableExpr created in the parent thread
 *
 * Returns:    The corresponding BPatch_variableExpr from the child thread
 *             or NULL if the variable argument hasn't been malloced
 *             in a parent process.
 */
BPatch_variableExpr *BPatch_process::getInheritedVariableInt(
                                                             BPatch_variableExpr &parentVar)
{
   if(! isInferiorAllocated(llproc, (Address)parentVar.getBaseAddr())) {
      // isn't defined in this process so must not have been defined in a
      // parent process
      return NULL;
   }
   return new BPatch_variableExpr(this, parentVar.getBaseAddr(), Null_Register,
                                  const_cast<BPatch_type *>(parentVar.getType()));
}


/*
 * BPatch_process::getInheritedSnippet
 *
 * Allows one to retrieve a snippet which exists in a child process which 
 * was inherited from and originally created in the parent process.
 * Function is invoked on the child BPatch_process (created from a fork in 
 * the application).
 *
 * Allows one to retrieve a snippet which exists in a child process which
 * was inherited from and originally created in the parent process.
 * Function is invoked on the child BPatch_process (created from a fork in
 * the application).
 *
 * parentSnippet: A BPatchSnippetHandle created in the parent thread
 *
 * Returns:       The corresponding BPatchSnippetHandle from the child thread.
 *
 */
BPatchSnippetHandle *BPatch_process::getInheritedSnippetInt(BPatchSnippetHandle &parentSnippet)
{
    // a BPatchSnippetHandle has an miniTramp for each point that
    // the instrumentation is inserted at
    const BPatch_Vector<miniTramp *> &parent_mtHandles = parentSnippet.mtHandles_;

    BPatchSnippetHandle *childSnippet = new BPatchSnippetHandle(this);
    for(unsigned i=0; i<parent_mtHandles.size(); i++) {
        miniTramp *childMT = NULL;
        if(!getInheritedMiniTramp(parent_mtHandles[i], childMT, llproc)) {
            fprintf(stderr, "Failed to get inherited mini tramp\n");
            return NULL;
        }
        childSnippet->addMiniTramp(childMT);
    }
    return childSnippet;
}

/*
 * BPatch_process::insertSnippet
 *
 * Insert a code snippet at a given instrumentation point.  Upon success,
 * returns a handle to the created instance of the snippet, which can be used
 * to delete it.  Otherwise returns NULL.
 *
 * expr		The snippet to insert.
 * point	The point at which to insert it.
 */
BPatchSnippetHandle *BPatch_process::insertSnippetInt(const BPatch_snippet &expr, 
						      BPatch_point &point, 
						      BPatch_snippetOrder order)
{
   BPatch_callWhen when;
   if (point.getPointType() == BPatch_exit)
      when = BPatch_callAfter;
   else
      when = BPatch_callBefore;

   return insertSnippetWhen(expr, point, when, order);
}

/*
 * BPatch_process::insertSnippet
 *
 * Insert a code snippet at a given instrumentation point.  Upon succes,
 * returns a handle to the created instance of the snippet, which can be used
 * to delete it.  Otherwise returns NULL.
 *
 * expr		The snippet to insert.
 * point	The point at which to insert it.
 */
// This handles conversion without requiring inst.h in a header file...
extern bool BPatchToInternalArgs(BPatch_point *point,
                                 BPatch_callWhen when,
                                 BPatch_snippetOrder order,
                                 callWhen &ipWhen,
                                 callOrder &ipOrder);
                           

BPatchSnippetHandle *BPatch_process::insertSnippetWhen(const BPatch_snippet &expr,
						       BPatch_point &point,
						       BPatch_callWhen when,
						       BPatch_snippetOrder order)
{
  BPatch_Vector<BPatch_point *> points;
  points.push_back(&point);
  return insertSnippetAtPointsWhen(expr,
				   points,
				   when,
				   order);
 
}

/*
 * BPatch_process::insertSnippet
 *
 * Insert a code snippet at each of a list of instrumentation points.  Upon
 * success, Returns a handle to the created instances of the snippet, which
 * can be used to delete them (as a unit).  Otherwise returns NULL.
 *
 * expr		The snippet to insert.
 * points	The list of points at which to insert it.
 */
// A lot duplicated from the single-point version. This is unfortunate.

BPatchSnippetHandle *BPatch_process::insertSnippetAtPointsWhen(const BPatch_snippet &expr,
                                                               const BPatch_Vector<BPatch_point *> &points,
                                                               BPatch_callWhen when,
                                                               BPatch_snippetOrder order)
{
    if (BPatch::bpatch->isTypeChecked()) {
        assert(expr.ast);
        if (expr.ast->checkType() == BPatch::bpatch->type_Error) {
            return false;
        }
    }

    if (!points.size()) {
      fprintf(stderr, "%s[%d]:  request to insert snippet at zero points!\n", FILE__, __LINE__);
      return false;
    }
    
    
    batchInsertionRecord *rec = new batchInsertionRecord;
    rec->thread_ = NULL;
    rec->snip = expr;
    rec->trampRecursive_ = BPatch::bpatch->isTrampRecursive();

    BPatchSnippetHandle *ret = new BPatchSnippetHandle(this);
    rec->handle_ = ret;
    
    for (unsigned i = 0; i < points.size(); i++) {
        BPatch_point *point = points[i];
        
#if defined(os_aix)
        if(llproc->collectSaveWorldData){
            // Apparently we have problems with main....
            // The things I do to not grab the name as a strcopy operation...
	  if (point->getFunction()->lowlevel_func()->symTabName().c_str() == "main") {
                rec->trampRecursive_ = true;
            }
        }

#endif

#if defined(os_aix) || defined(arch_x86_64)
        // Toss the const; the function _pointer_ doesn't though.
        BPatch_function *func = point->getFunction();
        func->calc_liveness(point);
#endif 
        
        callWhen ipWhen;
        callOrder ipOrder;
        
        if (!BPatchToInternalArgs(point, when, order, ipWhen, ipOrder)) {
            return NULL;
        }

        rec->points_.push_back(point);
        rec->when_.push_back(ipWhen);
        rec->order_ = ipOrder;

        point->recordSnippet(when, order, ret);
    }

    assert(rec->points_.size() == rec->when_.size());

    // Okey dokey... now see if we just tack it on, or insert now.
    if (pendingInsertions) {
        pendingInsertions->push_back(rec);
    }
    else {
        beginInsertionSetInt();
        pendingInsertions->push_back(rec);
        // All the insertion work was moved here...
        finalizeInsertionSetInt(false);
    }
    return ret;
}

/*
 * BPatch_process::insertSnippet
 *
 * Insert a code snippet at each of a list of instrumentation points.  Upon
 * success, Returns a handle to the created instances of the snippet, which
 * can be used to delete them (as a unit).  Otherwise returns NULL.
 *
 * expr		The snippet to insert.
 * points	The list of points at which to insert it.
 */
BPatchSnippetHandle *BPatch_process::insertSnippetAtPoints(
                 const BPatch_snippet &expr,
                 const BPatch_Vector<BPatch_point *> &points,
                 BPatch_snippetOrder order)
{
    return insertSnippetAtPointsWhen(expr,
                                     points,
                                     BPatch_callUnset,
                                     order);
}


/*
 * BPatch_process::beginInsertionSet
 * 
 * Starts a batch insertion set; that is, all calls to insertSnippet until
 * finalizeInsertionSet are delayed.
 *
 */

void BPatch_process::beginInsertionSetInt() 
{
    if (pendingInsertions == NULL)
        pendingInsertions = new BPatch_Vector<batchInsertionRecord *>;
    // Nothing else to do...
}

/*
 * BPatch_process::finalizeInsertionSet
 * 
 * Installs all instrumentation specified since the last beginInsertionSet call.
 *
 * modified gets set as a result of the catchup/fixup logic and is helpful in
 * interpreting a false return value...  if finalizeInsertionSet returns false,
 * but modified comes back true, then something horrible happened, because, if
 * we go thru the trouble to modify the process state to make everything work
 * then the function really should work.
 */
bool doingCatchup = false;
/*static*/ pdvector<pdvector<Frame> >  stacks;
/*static*/ pdvector<Address> pcs;

bool BPatch_process::finalizeInsertionSetInt(bool atomic, bool *modified) 
{
    // Can't insert code when mutations are not active.
    if (!mutationsActive)
        return false;
    
    if (pendingInsertions == NULL)
        return false;

    // Check where the application is _right_now_ to make sure that its "legal"
    // to insert code.  If any thread is executing instrumentation at the
    // requested inst point, then its _not_ ok to insert code.  So we have to do
    // a stack walk for each of the threads and check each frame to see if its
    // currently inside instrumentation at this point.
    //
    //  This info _might_ become available by simply examining the BPatch_frame
    //  structures, but we'll just do it the old way for now.
    //
    //  This check was imported from higher level code in paradyn.
    //
    //  To make this more efficient, we should cache the current set of stackwalks
    //  and derived info, but don't yet.  Need to find a good refresh condition.

    // Suggestion: when the process is continued...

    stacks.clear();
    pcs.clear();

    if (!llproc->walkStacks(stacks)) {
       fprintf(stderr, "%s[%d]:  walkStacks failed\n", FILE__, __LINE__);
       return false;
    }

    //  extract all PCs from all frames in our stack walks
    for (unsigned int i = 0; i < stacks.size(); ++i) {
       pdvector<Frame> &stack = stacks[i];
       for (unsigned int j = 0; j < stack.size(); ++j) {
         pcs.push_back( (Address) stack[j].getPC());
       }
    }

    // now extract all BPatch_point's from our set of pending insertions
    //  (need to check them all to do legal insertion atomically)
    pdvector<instPoint *> pts;

    for (unsigned int i = 0; i < pendingInsertions->size(); ++i) 
    {
       pdvector<BPatch_point *> &candidate_pts =  (*pendingInsertions)[i]->points_;
       for (unsigned int j = 0; j < candidate_pts.size(); ++j) 
       {
           instPoint *candidate_point = candidate_pts[j]->point;

           assert(candidate_point);
           bool found = false;

           //  check for duplicates...  
           for (unsigned int k = 0; k < pts.size(); ++k) 
           {
              if (pts[k] == candidate_point) {
                  //  already have this point, ignore it
                  found = true;
                  break;
              }
           }
           if (!found)  {
              pts.push_back(candidate_point);
           }
       }
    }

    //  Now...  for each instPoint in this insertion set, check the installed
    //  instrumentation vs. the current stack frames to make sure that we're not
    //  doing anything crazy...

    for (unsigned int i = 0; i < pts.size(); ++i) 
    {
       instPoint *pt = pts[i];

       if (!pt->checkInst(pcs)) {
           fprintf(stderr, "%s[%d]:  CANNOT perform code insertion while in instrumentation\n", 
                  FILE__, __LINE__);
           return false;
       }  
    }

    // Two loops: first addInst, then generate/install/link
    pdvector<miniTramp *> workDone;
    bool err = false;

    for (unsigned i = 0; i < pendingInsertions->size(); i++) {
        batchInsertionRecord *&bir = (*pendingInsertions)[i];
        assert(bir);

        // Don't handle thread inst yet...
        assert(!bir->thread_);

        if (!bir->points_.size()) {
          fprintf(stderr, "%s[%d]:  WARN:  zero points for insertion record\n", FILE__, __LINE__);
          fprintf(stderr, "%s[%d]:  failing to addInst\n", FILE__, __LINE__);
        }

        for (unsigned j = 0; j < bir->points_.size(); j++) {
            BPatch_point *bppoint = bir->points_[j];
            instPoint *point = bppoint->point;
            callWhen when = bir->when_[j];
            
            miniTramp *mini = point->addInst(bir->snip.ast,
                                             when,
                                             bir->order_,
                                             bir->trampRecursive_,
                                             false);
            if (mini) {
                workDone.push_back(mini);
                // Add to snippet handle
                bir->handle_->addMiniTramp(mini);
            }
            else {
                fprintf(stderr, "ERROR: failed to insert instrumentation: no minitramp\n");
                err = true;
                if (atomic) break;
            }
        }
        if (atomic && err)
            break;
    }
    
   if (atomic && err) goto cleanup;

   // All generation first. Actually, all generation per function...
   // but this is close enough.
   for (unsigned int i = 0; i < pendingInsertions->size(); i++) {
       batchInsertionRecord *&bir = (*pendingInsertions)[i];
       assert(bir);
        if (!bir->points_.size()) {
          fprintf(stderr, "%s[%d]:  WARN:  zero points for insertion record\n", FILE__, __LINE__);
          fprintf(stderr, "%s[%d]:  failing to generateInst\n", FILE__, __LINE__);
        }
       for (unsigned j = 0; j < bir->points_.size(); j++) {
           BPatch_point *bppoint = bir->points_[j];
           instPoint *point = bppoint->point;

           point->optimizeBaseTramps(bir->when_[j]);
           if (!point->generateInst()) {
               fprintf(stderr, "%s[%d]: ERROR: failed to insert instrumentation: generate\n",
                       FILE__, __LINE__);
               err = true;
               if (atomic && err) break;
           }
       }
       if (atomic && err) break;
   }

   if (atomic && err) goto cleanup;

   //  next, all installing 
   for (unsigned int i = 0; i < pendingInsertions->size(); i++) {
       batchInsertionRecord *&bir = (*pendingInsertions)[i];
       assert(bir);
        if (!bir->points_.size()) {
          fprintf(stderr, "%s[%d]:  WARN:  zero points for insertion record\n", FILE__, __LINE__);
          fprintf(stderr, "%s[%d]:  failing to installInst\n", FILE__, __LINE__);
        }
       for (unsigned j = 0; j < bir->points_.size(); j++) {
           BPatch_point *bppoint = bir->points_[j];
           instPoint *point = bppoint->point;
             
           if (!point->installInst()) {
               fprintf(stderr, "%s[%d]: ERROR: failed to insert instrumentation: install\n",
                      FILE__, __LINE__);
              err = true;
           }

           if (atomic && err) break;
       }
       if (atomic && err) break;
   }

   if (atomic && err) goto cleanup;

   //  Before we link it all together, we have to do some final checks and 
   //  fixes....  this is imported from paradyn's original ketchup logic

   // We may need to modify certain pieces of the process state to ensure
   // that instrumentation runs properly. Two known cases:
   // 1) If an active call site (call site on the stack) is instrumented,
   //    we need to modify the return address to be in instrumentation
   //    and not at the original return addr.
   // 2) AIX only: if we instrument with an entry/exit pair, modify the
   //    return address of the function to point into the exit tramp rather
   //    than the return addr. 
   //    Note that #2 overwrites #1; but if we perform the fixes in this order
   //    then everything works.

   // Note that stackWalks can be changed in catchupSideEffect...

    if (modified) *modified = false; 
    for (unsigned ptIter = 0; ptIter < pts.size(); ptIter++) {
        instPoint *pt = pts[ptIter];
        for (unsigned thrIter = 0; thrIter < stacks.size(); thrIter++) {
            for (unsigned sIter = 0; sIter < stacks[thrIter].size(); sIter++) {
                if (pt->instrSideEffect(stacks[thrIter][sIter]))
                    if (modified) *modified = true;
             }
        }
    }

   //  finally, do all linking 
   for (unsigned int i = 0; i < pendingInsertions->size(); i++) {
       batchInsertionRecord *&bir = (*pendingInsertions)[i];
       assert(bir);
        if (!bir->points_.size()) {
          fprintf(stderr, "%s[%d]:  WARN:  zero points for insertion record\n", FILE__, __LINE__);
          fprintf(stderr, "%s[%d]:  failing to linklInst\n", FILE__, __LINE__);
        }
       for (unsigned j = 0; j < bir->points_.size(); j++) {
           BPatch_point *bppoint = bir->points_[j];
           instPoint *point = bppoint->point;
             
          if (!point->linkInst()) {
               fprintf(stderr, "%s[%d]: ERROR: failed to insert instrumentation: link\n",
                       FILE__, __LINE__);
               err = true;
           }

           if (atomic && err) break;
       }
       if (atomic && err) break;
   }

   if (atomic && err) 
      goto cleanup;

  cleanup:
    bool ret = true;

    if (atomic && err) {
        // Something failed...   Cleanup...
        for (unsigned k = 0; k < workDone.size(); k++) {
            workDone[k]->uninstrument();
        }
        ret = false;
    }

    if (!doingCatchup) {
      //  if we're doing catchup, we need to keep these around (delete them later
      //    after catchup).
      for (unsigned int i = 0; i < pendingInsertions->size(); i++) {
         batchInsertionRecord *&bir = (*pendingInsertions)[i];
         assert(bir);
         delete(bir);
      }

      delete pendingInsertions;
      pendingInsertions = NULL;
    }
    catchup_printf("%s[%d]:  leaving finalizeInsertionSet -- CATCHUP DONE\n", FILE__, __LINE__);
    return ret;
}

bool BPatch_process::finalizeInsertionSetWithCatchupInt(bool atomic, bool *modified, 
                                               BPatch_Vector<BPatch_catchupInfo> &catchup_handles)
{
   //  set the doingCatchup flag so that finalizeInsertionSet knows to leave the insertion
   //  records around for us (we delete them at the end of this function)

   doingCatchup = true;
   if (!finalizeInsertionSetInt(atomic, modified)) {
      fprintf(stderr, "%s[%d]:  finalizeInsertionSet failed!\n", FILE__, __LINE__);
      return false;
   }
   doingCatchup = false;

   //  Now, flag insertions as needing catchup, if requested.
   //  This, again, is imported from higher level code in paradyn and should
   //  probably be modified from this first-stab importation...  which is more
   //  concerned with keeping it logically exact to what paradyn was doing, 
   //  rather than elegant, or even efficient (tho it is just about as efficient
   //  as the original paradyn catchup logic.

   //  Store the need-to-catchup flag in the BPatchSnippetHandle for the time being

   if (dyn_debug_catchup) {
      fprintf(stderr, "%s[%d]:  BEGIN CATCHUP ANALYSIS:  num inst req: %d\n", 
              FILE__, __LINE__, pendingInsertions->size());
      for (unsigned int i = 0; i < stacks.size(); ++i) {
        fprintf(stderr, "%s[%d]:Stack for thread %d\n", FILE__, __LINE__, i);
        pdvector<Frame> &one_stack = stacks[i];
        for (unsigned int j = 0; j < one_stack.size(); ++j) {
          int_function *my_f = one_stack[j].getFunc();
          const char *fname = my_f ? my_f->prettyName().c_str() : "no function";
          fprintf(stderr, "\t\tPC: 0x%lx\tFP: 0x%lx\t [%s]\n",  
                  one_stack[j].getPC(), one_stack[j].getFP(), fname);
        }
      }
   }

   //  For each stack frame, check to see if our just-inserted instrumentation
   //  is before or after the point that we will return to in the calling
   //  order

   // Iterate by threads first, to sort the list by thread. Paradyn needs this,
   // and it's a logical model - all the instrumentation missed on thread 1, then
   // missed on thread 2...

   catchup_printf("Checking to see if I work; stacks.size() == %d\n", stacks.size());

   for (unsigned int i = 0; i < stacks.size(); i++) {
       pdvector<Frame> &one_stack = stacks[i];

       catchup_printf("%s[%d]: examining stack %d with %d frames\n",
                      FILE__, __LINE__, i, one_stack.size());
       
       for (int j = one_stack.size()-1; j >= 0; j--) {
           Frame &frame = one_stack[j];

           catchup_printf("%s[%d]: examining frame %d\n", FILE__, __LINE__, j);

           if (frame.getPC() == 0) continue;

           for (unsigned int k = 0; k < pendingInsertions->size(); k++) {
               batchInsertionRecord *bir = (*pendingInsertions)[k];
               assert(bir);

               catchup_printf("%s[%d]: looking at insertion record %d\n", FILE__, __LINE__, k);
               
               if (dyn_debug_catchup) {
                   assert(bir->points_.size() == 1);
                   BPatch_point *bppoint = bir->points_[0];
                   instPoint *pt = bppoint->point;
                   assert(pt);
                   char *point_type = "no type";
                   switch(pt->getPointType()) {
                   case noneType: point_type = "noneType"; break;
                   case functionEntry: point_type = "funcEntry"; break;
                   case functionExit: point_type = "funcExit"; break;
                   case callSite: point_type = "callSite"; break;
                   case otherPoint: point_type = "otherPoint"; break;
                   }
                   int_function *f = pt->func();
                   const char *point_func = f->prettyName().c_str();
                   fprintf(stderr, "%s[%d]:  Catchup for instPoint %p [ %s ], func = %s\n",
                           FILE__, __LINE__, (void *)pt->addr(), point_type, point_func);
               }
               ///*static*/ pdvector<pdvector<Frame> > > stacks;
               
               // A subtlety - iterate _backwards_ down the frames. We
               // get stacks delivered to us with entry 0 being the
               // active function and entry (n-1) being main. However,
               // we want catchup to be considered in the opposite
               // direction - from time 0 (that is, entry n-1) to the
               // current time. As an example, take the call path main
               // -> foo -> bar. We instrument bar with an "if flag,
               // then" snippet, and foo with a "set flag" snippet. If
               // we execute catchup on bar first, then foo, the
               // snippets won't execute - the flag won't be set, and
               // so the condition will fail. If we (corrently)
               // execute foo first, the flag will be set. The snippet
               // in bar will then execute correctly.
               
               // Note: we need to iterate over a signed int, because
               // checking if an unsigned is ">= 0"... heh.
               
               
               //  Things we have:
               //  frame:  currentFrame
               //  bir->handle_:  current BPatchSnippetHandle (with mtHandles)
               //  bir->point_:  matching insertion point
               
               // First, if we're not even in the right _function_, then break out.
               //assert(bir->points_.size());
               if (!bir->points_.size()) {
                   //  how can this happen?
                   fprintf(stderr, "%s[%d]:  WARN:  insertion record w/o any points!\n", FILE__, __LINE__);
                   continue;
               }
               if (bir->points_.size() > 1) {
                   fprintf(stderr, "%s[%d]:  WARNING:  have more than one point!\n", FILE__, __LINE__);
               }
               BPatch_point *bppoint = bir->points_[0];
               assert(bppoint);
               instPoint *iP = bppoint->point;
               assert(iP);
               if (frame.getFunc() != iP->func()) {
                   if (dyn_debug_catchup) {
                       const char *f1 =  frame.getFunc() ? frame.getFunc()->prettyName().c_str()
                           :"no function";
                       const char *f2 = iP->func()->prettyName().c_str();
                       catchup_printf("%s[%d]: skipping frame, funcs don't match [%s, %s]\n",
                                      FILE__, __LINE__, f1 ? f1 : "<NULL>", f2 ? f2 : "<NULL>");
                   }
                   continue;
               }
               
               BPatchSnippetHandle *&sh = bir->handle_;
               dyn_thread *thr = frame.getThread();
               assert(thr);
               dynthread_t tid = thr->get_tid();
               BPatch_process *bpproc = sh->getProcess();
               assert(bpproc);
               BPatch_thread *bpthread = bpproc->getThread(tid);
               assert(bpthread);
               // I guess for the sake of absolute correctness, we need
               // to iterate over possibly more than one mtHandle:
               
               //  Actually NO...  this is incorrect -- disable catchup for
               //  snippet handles that have more than one mtHandle
               BPatch_Vector<miniTramp *> &mtHandles = bir->handle_->mtHandles_;
               assert(mtHandles.size() == 1);
               for (unsigned int m = 0; m < mtHandles.size(); ++m) {
                   miniTramp *&mtHandle = mtHandles[m];
                   bool &catchupNeeded  = bir->handle_->catchupNeeded;
                   catchupNeeded = false;
                   
                   //  Before we do any analysis at all, check a couple things:
                   //  (1)  If this snippet accesses function parameters, then
                   //       we just skip it for catchup (function parameters live on
                   //       the stack too)
                   
                   if (bir->snip.ast->accessesParam())
                       continue;
                   
#if 0
                   //  (2)  If this is a function entry, make sure that we only
                   //       register it once -- why??-- don't know -- from paradyn
                   //  (3)  If this is a loop entry, make sure that we only
                   //       register it once -- why??-- don't know -- from paradyn
                   if ((bir->points_[0]->getPointType() == BPatch_locEntry)
                       && (bir->somethings_wrong_here))
                       continue;
                   
                   if ((bir->point_->getPointType() == BPatch_locLoopEntry)
                       && (bir->somethings_wrong_here))
                       continue;
#endif
                   
                   // If we're inside the function, find whether we're before, 
                   // inside, or after the point.
                   // This is done by address comparison and used to demultiplex 
                   // the logic below.
                   
                   typedef enum {
                       nowhere_l = 1,
                       beforePoint_l = 2,
                       notMissed_l = 3,
                       missed_l = 4,
                       afterPoint_l =5
                   } logicalPCLocation_t;
                   
                   logicalPCLocation_t location;
                   
                   assert(iP);
                   instPoint::catchup_result_t iPresult = iP->catchupRequired(frame.getPC(), 
                                                                              mtHandle);
                   
                   
                   if (iPresult == instPoint::notMissed_c)
                       location = notMissed_l;
                   else if (iPresult == instPoint::missed_c)
                       location = missed_l;
                   else
                       location = nowhere_l;
                   
                   // We check for the instPoint before this because we use instrumentation
                   // that may cover multiple instructions.
                   // USE THE UNINSTRUMENTED ADDR :)
                   if (location == nowhere_l) {
                       // Uninstrumented, and mapped back from function relocation...
                       // otherwise we'll get all sorts of weird.
                       // Commented out; with non-contiguous functions, we must go only
                       // on known information.

                       // Back off to address comparison
                       if ((Address)iP->addr() < frame.getUninstAddr()) {
                           catchup_printf("%s[%d]: comparing instPoint addr 0x%lx to uninst addr 0x%lx (inst 0x%lx), setting afterPoint\n",
                                          FILE__, __LINE__, iP->addr(), frame.getUninstAddr(), frame.getPC());
                           location = afterPoint_l;
                       }
                       else {
                           catchup_printf("%s[%d]: comparing instPoint addr 0x%lx to uninst addr 0x%lx (inst 0x%lx), setting beforePoint\n",
                                          FILE__, __LINE__, iP->addr(), frame.getUninstAddr(), frame.getPC());
                           location = beforePoint_l;
                       }
                   }
                   
                   if (dyn_debug_catchup) {
                       char *str_iPresult = "error";
                       switch(location) {
                       case nowhere_l: str_iPresult = "nowhere_l"; break;
                       case beforePoint_l: str_iPresult = "beforePoint_l"; break;
                       case notMissed_l: str_iPresult = "notMissed_l"; break;
                       case missed_l: str_iPresult = "missed_l"; break;
                       case afterPoint_l: str_iPresult = "afterPoint_l"; break;
                       default: break;
                       };
                       fprintf(stderr, "\t\tFor PC = 0x%lx, iPresult = %s ", 
                               frame.getPC(), str_iPresult);
                   }
                   
                   BPatch_catchupInfo catchup_info;
                   
                   // We split cases out by the point type
                   // All of these must fit the following criteria:
                   // An object with a well-defined entry and exit;
                   // An object where we can tell if a PC is "within".
                   // Examples: functions, basic blocks, loops
                   switch(bppoint->getPointType()) {
                   case BPatch_locEntry:
                       // Entry is special, since it's one of the rare
                       // cases where "after" is good enough. TODO:
                       // check whether we're "in" a function in a manner
                       // similar to loops.
                       // We know we can get away with >= because we're in the
                       // function; if not we'd have already returned.
                       if ((location >= missed_l) ||
                           (location == nowhere_l)) {
                           catchupNeeded = true;
                           catchup_info.snip = bir->snip;
                           catchup_info.sh = sh;
                           catchup_info.thread = bpthread;
                           catchup_handles.push_back(catchup_info);
                       }
                       break;
                   case BPatch_locExit:
                       // Only do this if we triggered "missed". If we're
                       // after, we might well be later in the function.
                       // If this is true, we're cancelling an earlier entry
                       // catchup.
                       if (location == missed_l) {
                           catchupNeeded = true;
                           catchup_info.snip = bir->snip;
                           catchup_info.sh = sh;
                           catchup_info.thread = bpthread;
                           catchup_handles.push_back(catchup_info);
                       }
                       break;
                   case BPatch_subroutine:
                       // Call sites. Again, only if missed; otherwise we may
                       // just be elsewhere
                       if (location == missed_l) {
                           catchupNeeded = true;
                           catchup_info.snip = bir->snip;
                           catchup_info.sh = sh;
                           catchup_info.thread = bpthread;
                           catchup_handles.push_back(catchup_info);
                       }
                       break;
                   case BPatch_locLoopEntry:
                   case BPatch_locLoopStartIter:
                       if (location == missed_l) {
                           catchupNeeded = true;
                           catchup_info.snip = bir->snip;
                           catchup_info.sh = sh;
                           catchup_info.thread = bpthread;
                           catchup_handles.push_back(catchup_info);
                       }
                       if (location == afterPoint_l || location == nowhere_l) {
                           BPatch_basicBlockLoop *loop = bppoint->getLoop();
                           if (loop->containsAddressInclusive(frame.getUninstAddr())) {
                               catchupNeeded = true;
                               catchup_info.snip = bir->snip;
                               catchup_info.sh = sh;
                               catchup_info.thread = bpthread;
                               catchup_handles.push_back(catchup_info);
                           }
                       }
                       break;
                   case BPatch_locLoopExit:
                   case BPatch_locLoopEndIter:
                       // See earlier treatment of, well, everything else
                       if (location == missed_l) {
                           catchupNeeded = true;
                           catchup_info.snip = bir->snip;
                           catchup_info.sh = sh;
                           catchup_info.thread = bpthread;
                           catchup_handles.push_back(catchup_info);
                       }
                       break;
                       
                   case BPatch_locBasicBlockEntry:
                   case BPatch_locBasicBlockExit:
                   default:
                       // Nothing here
                       break;
                   }
                   
                   if (dyn_debug_catchup) {
                       if (catchupNeeded) {
                           fprintf(stderr, "catchup needed, ret true\n========\n");
                           if (!bir->handle_->catchupNeeded) {
                               fprintf(stderr, "%s[%d]:  SERIOUS MISTAKE with reference\n", FILE__, __LINE__);
                           }
                       } else
                           fprintf(stderr, "catchup not needed, ret false\n=======\n");
                   }
               } // Over minitramps (always 1)
           } // Over BPatch_points
       } // Over stack frames
   } // Over threads
   
   //cleanup:
   bool ret = true;
   
   for (unsigned int i = 0; i < pendingInsertions->size(); i++) {
       batchInsertionRecord *&bir = (*pendingInsertions)[i];
       assert(bir);
       delete(bir);
   }
   
   delete pendingInsertions;
   pendingInsertions = NULL;
   catchup_printf("%s[%d]:  leaving finalizeInsertionSet -- CATCHUP DONE\n", FILE__, __LINE__);

   catchup_printf("%s[%d]: %d returned catchup requests\n", FILE__, __LINE__, catchup_handles.size());

   // Postcondition: catchup_handles contains a list of <snippet,
   // snippetHandle, thread> tuples. There is an entry if a given
   // snippet was "missed" on that particular thread. The list is
   // sorted from the "top" of the stack (main) down.

   return ret;
}

/*
 * BPatch_process::deleteSnippet
 * 
 * Deletes an instance of a snippet.
 *
 * handle	The handle returned by insertSnippet when the instance to
 *		deleted was created.
 */
bool BPatch_process::deleteSnippetInt(BPatchSnippetHandle *handle)
{   
    if (terminated) return true;

    if (handle->proc_ == this) {
        for (unsigned int i=0; i < handle->mtHandles_.size(); i++)
            handle->mtHandles_[i]->uninstrument();
        delete handle;
        return true;
    } 
    // Handle isn't to a snippet instance in this process
    cerr << "Error: wrong process in deleteSnippet" << endl;     
    return false;
}

/*
 * BPatch_process::replaceCode
 *
 * Replace a given instruction with a BPatch_snippet.
 *
 * point       Represents the instruction to be replaced
 * snippet     The replacing snippet
 */

bool BPatch_process::replaceCodeInt(BPatch_point *point,
                                    BPatch_snippet *snippet) {
   if (!mutationsActive)
      return false;

    if (!point) {
        return false;
    }
    if (terminated) {
        return true;
    }

    if (point->edge_) {
        return false;
    }

    // Calculate liveness to make things cheaper

#if defined(os_aix) || defined(arch_x86_64)
        // Toss the const; the function _pointer_ doesn't though.
        BPatch_function *func = point->getFunction();
        func->calc_liveness(point);
#endif 


    return point->point->replaceCode(snippet->ast);
}


/*
 * BPatch_process::setMutationsActive
 *
 * Enable or disable the execution of all snippets for the thread.
 * 
 * activate	If set to true, execution of snippets is enabled.  If false,
 *		execution is disabled.
 */
bool BPatch_process::setMutationsActiveInt(bool activate)
{
   // If not activating or deactivating, just return.
   if ((activate && mutationsActive) || (!activate && !mutationsActive))
      return true;
   
   if (activate)
      llproc->reinstallMutations();
   else
      llproc->uninstallMutations();
   
   mutationsActive = activate;
   return true;
}

/*
 * BPatch_process::replaceFunctionCall
 *
 * Replace a function call with a call to a different function.  Returns true
 * upon success, false upon failure.
 * 
 * point	The call site that is to be changed.
 * newFunc	The function that the call site will now call.
 */
bool BPatch_process::replaceFunctionCallInt(BPatch_point &point,
                                            BPatch_function &newFunc)
{
   // Can't make changes to code when mutations are not active.
   if (!mutationsActive)
      return false;
   assert(point.point && newFunc.lowlevel_func());
   return llproc->replaceFunctionCall(point.point, newFunc.lowlevel_func());
}

/*
 * BPatch_process::removeFunctionCall
 *
 * Replace a function call with a NOOP.  Returns true upon success, false upon
 * failure.
 * 
 * point	The call site that is to be NOOPed out.
 */
bool BPatch_process::removeFunctionCallInt(BPatch_point &point)
{
   // Can't make changes to code when mutations are not active.
   if (!mutationsActive)
      return false;   
   assert(point.point);
   return llproc->replaceFunctionCall(point.point, NULL);
}


/*
 * BPatch_process::replaceFunction
 *
 * Replace all calls to function OLDFUNC with calls to NEWFUNC.
 * Returns true upon success, false upon failure.
 * 
 * oldFunc	The function to replace
 * newFunc      The replacement function
 */
bool BPatch_process::replaceFunctionInt(BPatch_function &oldFunc,
                                        BPatch_function &newFunc)
{
#if defined(os_solaris) || defined(os_osf) || defined(os_linux) || \
    defined(os_windows) \

    assert(oldFunc.lowlevel_func() && newFunc.lowlevel_func());
    if (!mutationsActive)
        return false;
    
    // Self replacement is a nop
    // We should just test direct equivalence here...
    if (oldFunc.lowlevel_func() == newFunc.lowlevel_func()) {
        return true;
    }

   bool old_recursion_flag = BPatch::bpatch->isTrampRecursive();
   BPatch::bpatch->setTrampRecursive( true );
   
   // We replace functions by instrumenting the entry of OLDFUNC with
   // a non-linking jump to NEWFUNC.  Calls to OLDFUNC do actually
   // transfer to OLDFUNC, but then our jump shunts them to NEWFUNC.
   // The non-linking jump ensures that when NEWFUNC returns, it
   // returns directly to the caller of OLDFUNC.
   BPatch_Vector<BPatch_point *> *pts = oldFunc.findPoint(BPatch_entry);
   if (! pts || ! pts->size()) {
      BPatch::bpatch->setTrampRecursive( old_recursion_flag );
      return false;
   }
   BPatch_funcJumpExpr fje(newFunc);
   BPatchSnippetHandle * result = insertSnippetAtPointsWhen(fje, *pts, BPatch_callBefore);
   
   BPatch::bpatch->setTrampRecursive( old_recursion_flag );
   
   return (NULL != result);
#else
   char msg[2048];
   char buf1[512], buf2[512];
   sprintf(msg, "cannot replace func %s with func %s, not implemented",
           oldFunc.getName(buf1, 512), newFunc.getName(buf2, 512));
    
   BPatch_reportError(BPatchSerious, 109, msg);
   BPatch_reportError(BPatchSerious, 109,
                      "replaceFunction is not implemented on this platform");
   return false;
#endif
}

/*
 * BPatch_process::oneTimeCode
 *
 * execute argument <expr> once.
 *
 */
void *BPatch_process::oneTimeCodeInt(const BPatch_snippet &expr, bool *err)
{
    return oneTimeCodeInternal(expr, NULL, NULL, NULL, true, err);
}

/*
 * BPatch_process::oneTimeCodeCallbackDispatch
 *
 * This function is registered with the lower-level code as the callback for
 * inferior RPC completion.  It determines what thread the RPC was executed on
 * and then calls the API's higher-level callback routine for that thread.
 *
 * theProc	The process in which the RPC completed.
 * userData	This is a value that can be set when we invoke an inferior RPC
 *		and which will be returned to us in this callback.
 * returnValue	The value returned by the RPC.
 */

int BPatch_process::oneTimeCodeCallbackDispatch(process *theProc,
                                                 unsigned /* rpcid */, 
                                                 void *userData,
                                                 void *returnValue)
{
    // Don't care what the process state is...
    int retval = RPC_LEAVE_AS_IS;

   assert(BPatch::bpatch != NULL);
   bool need_to_unlock = true;
   global_mutex->_Lock(FILE__, __LINE__);
   if (global_mutex->depth() > 1) {
     global_mutex->_Unlock(FILE__, __LINE__);
     need_to_unlock = false;
   }

   assert(global_mutex->depth());
   
   OneTimeCodeInfo *info = (OneTimeCodeInfo *)userData;
   
   BPatch_process *bproc =
      BPatch::bpatch->getProcessByPid(theProc->getPid());

   assert(bproc != NULL);

   assert(info && !info->isCompleted());

   if (returnValue == (void *) -1L)
     fprintf(stderr, "%s[%d]:  WARNING:  no return value for rpc\n", FILE__, __LINE__);
   info->setReturnValue(returnValue);
   info->setCompleted(true);

   bool synchronous = info->isSynchronous();
   
   if (!synchronous) {
       // Asynchronous RPCs: if we're running, then hint to run the process
       if (bproc->isVisiblyStopped)
           retval = RPC_STOP_WHEN_DONE;
       else
           retval = RPC_RUN_WHEN_DONE;
       
      //  if we have a specific callback for (just) this oneTimeCode, call it
      OneTimeCodeCallback *specific_cb = info->getCallback();
      if (specific_cb) {
          specific_cb->setTargetThread(TARGET_UI_THREAD);
          specific_cb->setSynchronous(true);
          (*specific_cb)(bproc->threads[0], info->getUserData(), returnValue);
      }

      //  get global oneTimeCode callbacks
      pdvector<CallbackBase *> cbs;
      getCBManager()->dispenseCallbacksMatching(evtOneTimeCode, cbs);
      BPatch::bpatch->signalNotificationFD();
      
      for (unsigned int i = 0; i < cbs.size(); ++i) {
          BPatch::bpatch->signalNotificationFD();
          
          OneTimeCodeCallback *cb = dynamic_cast<OneTimeCodeCallback *>(cbs[i]);
          if (cb) {
              cb->setTargetThread(TARGET_UI_THREAD);
              cb->setSynchronous(false);
              (*cb)(bproc->threads[0], info->getUserData(), returnValue);
          }
          
      }
      
      delete info;
   }

   bproc->oneTimeCodeCompleted(synchronous);

  if (need_to_unlock)
     global_mutex->_Unlock(FILE__, __LINE__);

  return retval;
}

/*
 * BPatch_process::oneTimeCodeInternal
 *
 * Causes a snippet expression to be evaluated once in the mutatee at the next
 * available opportunity.  Optionally, Dyninst will call a callback function
 * when the snippet has executed in the mutatee, and can wait until the
 * snippet has executed to return.
 *
 * expr		The snippet to evaluate.
 * userData	This value is given to the callback function along with the
 *		return value for the snippet.  Can be used by the caller to
 *		store per-oneTimeCode information.
 * synchronous	True means wait until the snippet has executed, false means
 *		return immediately.
 */
void *BPatch_process::oneTimeCodeInternal(const BPatch_snippet &expr,
                                          BPatch_thread *thread, 
                                          void *userData,
                                          BPatchOneTimeCodeCallback cb,
                                          bool synchronous,
                                          bool *err)
{
    if (statusIsTerminated()) {
       fprintf(stderr, "%s[%d]:  oneTimeCode failing because process is terminated\n", FILE__, __LINE__);
       if (err) *err = true;
       return NULL;
    }
    if (!isVisiblyStopped && synchronous) resumeAfterCompleted_ = true;

   inferiorrpc_printf("%s[%d]: UI top of oneTimeCode...\n", FILE__, __LINE__);
   while (llproc->sh->isActivelyProcessing()) {
       inferiorrpc_printf("%s[%d]:  waiting before doing user stop for process %d\n", FILE__, __LINE__, llproc->getPid());
       llproc->sh->waitForEvent(evtAnyEvent);
   }

    if (statusIsTerminated()) {
       fprintf(stderr, "%s[%d]:  oneTimeCode failing because process is terminated\n", FILE__, __LINE__);
       if (err) *err = true;
       return NULL;
    }

   inferiorrpc_printf("%s[%d]: oneTimeCode, handlers quiet, sync %d, statusIsStopped %d, resumeAfterCompleted %d\n",
                      FILE__, __LINE__, synchronous, statusIsStopped(), resumeAfterCompleted_);

   OneTimeCodeCallback *otc_cb =  cb ? new OneTimeCodeCallback(cb) : NULL;
   OneTimeCodeInfo *info = new OneTimeCodeInfo(synchronous, userData, otc_cb,
                                                 (thread) ? thread->index : 0);

   // inferior RPCs are a bit of a pain; we need to hand off control of process pause/continue
   // to the internal layers. In general BPatch takes control of the process _because_ we can't
   // predict what the user will do; if there is a BPatch-pause it overrides internal pauses. However,
   // here we give back control to the internals so that the rpc will complete.

   inferiorrpc_printf("%s[%d]: launching RPC on process pid %d\n",
                      FILE__, __LINE__, llproc->getPid());

   llproc->getRpcMgr()->postRPCtoDo(expr.ast,
                                    false, 
                                    BPatch_process::oneTimeCodeCallbackDispatch,
                                    (void *)info,
                                    false, // We'll determine later
                                    false, // don't use lowmem heap...
                                    (thread) ? (thread->llthread) : NULL,
                                    NULL); 
   activeOneTimeCodes_++;

   // We override while the inferiorRPC runs...
   if (synchronous) {
       // If we're waiting around make sure the iRPC runs. Otherwise,
       // it runs as the process does.
       llproc->sh->overrideSyncContinueState(ignoreRequest);
   }

   if (!synchronous && isVisiblyStopped) {
      unstartedRPC = true;
      return NULL;
   }

   inferiorrpc_printf("%s[%d]: calling launchRPCs\n", FILE__, __LINE__);
   bool needsToRun = false;
   llproc->getRpcMgr()->launchRPCs(needsToRun, false);

   if (!synchronous) return NULL;

   while (!info->isCompleted()) {
       inferiorrpc_printf("%s[%d]: waiting for RPC to complete\n",
                          FILE__, __LINE__);
       if (statusIsTerminated()) {
           fprintf(stderr, "%s[%d]:  process terminated with outstanding oneTimeCode\n", FILE__, __LINE__);
           if (err) *err = true;
           return NULL;
       }
       
       eventType ev = llproc->sh->waitForEvent(evtRPCSignal, llproc, NULL /*lwp*/, 
                                               statusRPCDone);
       inferiorrpc_printf("%s[%d]: got RPC event from system: terminated %d\n",
                          FILE__, __LINE__, statusIsTerminated());
       if (statusIsTerminated()) {
           fprintf(stderr, "%s[%d]:  process terminated with outstanding oneTimeCode\n", FILE__, __LINE__);
           if (err) *err = true;
           return NULL;
       }

       if (ev == evtProcessExit) {
           fprintf(stderr, "%s[%d]:  process terminated with outstanding oneTimeCode\n", FILE__, __LINE__);
           fprintf(stderr, "Process exited, returning NULL\n");
           if (err) *err = true;
           return NULL;
       }

       inferiorrpc_printf("%s[%d]: executing callbacks\n", FILE__, __LINE__);
       getMailbox()->executeCallbacks(FILE__, __LINE__);
   }

   void *ret = info->getReturnValue();

   inferiorrpc_printf("%s[%d]: RPC completed, process status %s\n",
                      FILE__, __LINE__, statusIsStopped() ? "stopped" : "running");
   
   if (err) *err = false;
   delete info;
   return ret;
}

void BPatch_process::oneTimeCodeCompleted(bool isSynchronous) {
    assert(activeOneTimeCodes_ > 0);
    activeOneTimeCodes_--;
    
    if (activeOneTimeCodes_ == 0 && isSynchronous) {
        inferiorrpc_printf("%s[%d]: oneTimeCodes outstanding reached 0, isVisiblyStopped %d, completing: %s\n",
                           FILE__, __LINE__, 
                           isVisiblyStopped,
                           resumeAfterCompleted_ ? "setting running" : "leaving stopped");
        if (resumeAfterCompleted_) {
            llproc->sh->overrideSyncContinueState(runRequest);
            llproc->sh->continueProcessAsync();
        }
        else {
            llproc->sh->overrideSyncContinueState(stopRequest);
        }
        resumeAfterCompleted_ = false;
    }
}

//  BPatch_process::oneTimeCodeAsync
//
//  Have the specified code be executed by the mutatee once.  Don't wait 
//  until done.
bool BPatch_process::oneTimeCodeAsyncInt(const BPatch_snippet &expr, 
                                         void *userData, BPatchOneTimeCodeCallback cb) 
{
    if (NULL == oneTimeCodeInternal(expr, NULL, userData,  cb, false, NULL)) {
      //fprintf(stderr, "%s[%d]:  oneTimeCodeInternal failed\n", FILE__, __LINE__);
      return false;
   }
   return true;
}

/*
 * BPatch_process::loadLibrary
 *
 * Load a dynamically linked library into the address space of the mutatee.
 *
 * libname	The name of the library to load.
 */
#if defined(cap_save_the_world)
bool BPatch_process::loadLibraryInt(const char *libname, bool reload)
#else
bool BPatch_process::loadLibraryInt(const char *libname, bool)
#endif
{
   stopExecutionInt();
   if (!statusIsStopped()) {
      fprintf(stderr, "%s[%d]:  Process not stopped in loadLibrary\n", FILE__, __LINE__);
      return false;
   }
   
   if (!libname) {
      fprintf(stderr, "[%s:%u] - loadLibrary called with NULL library name\n",
              __FILE__, __LINE__);
      return false;
   }

   /**
    * Find the DYNINSTloadLibrary function
    **/
   BPatch_Vector<BPatch_function *> bpfv;
   image->findFunction("DYNINSTloadLibrary", bpfv);
   if (!bpfv.size()) {
      cerr << __FILE__ << ":" << __LINE__ << ": FATAL:  Cannot find Internal"
           << "Function DYNINSTloadLibrary" << endl;
      return false;
   }
   if (bpfv.size() > 1) {
      pdstring msg = pdstring("Found ") + pdstring(bpfv.size()) + 
         pdstring("functions called DYNINSTloadLibrary -- not fatal but weird");
      BPatch_reportError(BPatchSerious, 100, msg.c_str());
   }
   BPatch_function *dlopen_func = bpfv[0]; 
   if (dlopen_func == NULL) return false;

   /**
    * Generate a call to DYNINSTloadLibrary, and then run the generated code.
    **/
   BPatch_Vector<BPatch_snippet *> args;   
   BPatch_constExpr nameArg(libname);
   args.push_back(&nameArg);   
   BPatch_funcCallExpr call_dlopen(*dlopen_func, args);
    
   if (!oneTimeCodeInternal(call_dlopen, NULL, NULL, NULL, true)) {
      BPatch_variableExpr *dlerror_str_var = 
         image->findVariable("gLoadLibraryErrorString");
      assert(NULL != dlerror_str_var);      
      char dlerror_str[256];
      dlerror_str_var->readValue((void *)dlerror_str, 256);
      BPatch_reportError(BPatchSerious, 124, dlerror_str);
      return false;
   }
   BPatch_variableExpr *brk_ptr_var = 
      image->findVariable("gBRKptr");
   assert(NULL != brk_ptr_var);
   void *brk_ptr;
   brk_ptr_var->readValue(&brk_ptr, sizeof(void *));

#if defined(cap_save_the_world) 
	if(llproc->collectSaveWorldData && reload){
		llproc->saveWorldloadLibrary(libname, brk_ptr);	
	}
#endif
   return true;
}

bool BPatch_process::getAddressRangesInt( const char * fileName, unsigned int lineNo, std::vector< std::pair< unsigned long, unsigned long > > & ranges ) {
	unsigned int originalSize = ranges.size();

	/* Iteratate over the modules, looking for addr in each. */
	BPatch_Vector< BPatch_module * > * modules = image->getModules();
	for( unsigned int i = 0; i < modules->size(); i++ ) {
		LineInformation & lineInformation = (* modules)[i]->getLineInformation();		
		lineInformation.getAddressRanges( fileName, lineNo, ranges );
		} /* end iteration over modules */
	if( ranges.size() != originalSize ) { return true; }
	
	return false;
	} /* end getAddressRangesInt() */

bool BPatch_process::getSourceLinesInt( unsigned long addr, std::vector< LineInformation::LineNoTuple > & lines ) {
	unsigned int originalSize = lines.size();

	/* Iteratate over the modules, looking for addr in each. */
	BPatch_Vector< BPatch_module * > * modules = image->getModules();
	for( unsigned int i = 0; i < modules->size(); i++ ) {
		LineInformation & lineInformation = (* modules)[i]->getLineInformation();		
		lineInformation.getSourceLines( addr, lines );
		} /* end iteration over modules */
	if( lines.size() != originalSize ) { return true; }
	
	return false;
	} /* end getLineAndFile() */

/*
 * BPatch_process::findFunctionByAddr
 *
 * Returns the function that contains the specified address, or NULL if the
 * address is not within a function.
 *
 * addr		The address to use for the lookup.
 */
BPatch_function *BPatch_process::findFunctionByAddrInt(void *addr)
{
   int_function *func;
   
   codeRange *range = llproc->findCodeRangeByAddress((Address) addr);
   if (!range)
      return NULL;

   func = range->is_function();
    
   if (!func)
      return NULL;
    
   return findOrCreateBPFunc(func, NULL);
}


/* 
 *	this function sets a flag in process that 
 *	forces the collection of data for saveworld.
 */
void BPatch_process::enableDumpPatchedImageInt(){
	llproc->collectSaveWorldData=true;
}

void BPatch_process::setExitedViaSignal(int signalnumber) 
{
   exitedViaSignal = true;
   lastSignal = signalnumber;
}

void BPatch_process::setExitedNormally() 
{
   exitedNormally = true;
}

void BPatch_process::getThreadsInt(BPatch_Vector<BPatch_thread *> &thrds)
{
   for (unsigned i=0; i<threads.size(); i++)
      thrds.push_back(threads[i]);
}

bool BPatch_process::isMultithreadedInt()
{
   return (threads.size() > 1);
}

bool BPatch_process::isMultithreadCapableInt()
{
   if (!llproc) return false;
   return llproc->multithread_capable();
}

BPatch_thread *BPatch_process::getThreadInt(dynthread_t tid)
{
   for (unsigned i=0; i<threads.size(); i++)
      if (threads[i]->getTid() == tid)
         return threads[i];
   return NULL;
}

BPatch_thread *BPatch_process::getThreadByIndexInt(unsigned index)
{
   for (unsigned i=0; i<threads.size(); i++)
      if (threads[i]->getBPatchID() == index)
         return threads[i];
   return NULL;
}

BPatch_function *BPatch_process::findOrCreateBPFunc(int_function* ifunc,
                                                    BPatch_module *bpmod)
{
  if( func_map->defines(ifunc) ) {
    assert( func_map->get(ifunc) != NULL );
    return func_map->get(ifunc);
  }
  
  // Find the module that contains the function
  if (bpmod == NULL && ifunc->mod() != NULL) {
      bpmod = getImage()->findModule(ifunc->mod()->fileName().c_str());
  }

  // findModule has a tendency to make new function objects... so
  // check the map again
  if (func_map->defines(ifunc)) {
    assert( func_map->get(ifunc) != NULL );
    return func_map->get(ifunc);
  }

  BPatch_function *ret = new BPatch_function(this, ifunc, bpmod);
  assert( ret != NULL );
  return ret;
}

BPatch_point *BPatch_process::findOrCreateBPPoint(BPatch_function *bpfunc, 
						  instPoint *ip, 
						  BPatch_procedureLocation pointType)
{
    assert(ip);
   if (instp_map->defines(ip)) 
      return instp_map->get(ip);

   if (bpfunc == NULL) 
       bpfunc = findOrCreateBPFunc(ip->func(), NULL);
   
   BPatch_point *pt = new BPatch_point(this, bpfunc, ip, pointType);

   instp_map->add(ip, pt);

   return pt;
}

BPatch_function *BPatch_process::createBPFuncCB(process *p, int_function *f)
{
   bool found;
   BPatch_process *proc = BPatch::bpatch->getProcessByPid(p->getPid(), &found);
   assert(found);
   return proc->findOrCreateBPFunc(f, NULL);
}

BPatch_point *BPatch_process::createBPPointCB(process *p, int_function *f, 
                                              instPoint *ip, int type)
{
   bool found;
   BPatch_process *proc = BPatch::bpatch->getProcessByPid(p->getPid(), &found);
   assert(found);
   BPatch_function *func = proc->func_map->get(f);
   return proc->findOrCreateBPPoint(func, ip, (BPatch_procedureLocation) type);
}

BPatch_thread *BPatch_process::createOrUpdateBPThread(
                         int lwp, dynthread_t tid, unsigned index, 
                         unsigned long stack_start,  
                         unsigned long start_addr)
{
   //fprintf(stderr, "%s[%d][%s]:  welcome to createOrUpdateBPThread(tid = %lu)\n",
   //      FILE__, __LINE__, getThreadStr(getExecThreadID()), tid);
   BPatch_thread *bpthr = this->getThread(tid);
   if (!bpthr)
      bpthr = this->getThreadByIndex(index);

   if (!bpthr)
   {
      bpthr = BPatch_thread::createNewThread(this, index, lwp, tid);

      if (bpthr->doa) {
             bpthr->getProcess()->llproc->removeThreadIndexMapping(tid, index);
          return bpthr;
      }         
   }

   bool found = false;
   for (unsigned i=0; i<threads.size(); i++)
      if (threads[i] == bpthr) {
         found = true;
         break;
      }
   if (!found)
      threads.push_back(bpthr);

   BPatch_function *initial_func = NULL;
#if defined(arch_ia64)
   bpthr->llthread->update_sfunc_indir(start_addr);
#else
   initial_func = getImage()->findFunction(start_addr);
#endif

   if (!initial_func) {
     //fprintf(stderr, "%s[%d][%s]:  WARNING:  no function at %p found for thread\n",
     //        FILE__, __LINE__, getThreadStr(getExecThreadID()), start_addr);
   }
   bpthr->updateValues(tid, stack_start, initial_func, lwp);   
   return bpthr;
}

/**
 * Called when a delete thread event is read out of the event queue
 **/
void BPatch_process::deleteBPThread(BPatch_thread *thrd)
{
   if (!thrd || !thrd->getBPatchID()) 
   {
      //Don't delete if this is the initial thread.  Some Dyninst programs
      // may use the initial BPatch_thread as a handle instead of the 
      // BPatch_process, and we don't want to delete that handle out from
      // under the users.
      return;
   }

   if (thrd->getTid() == 0)
     fprintf(stderr, "%s[%d]:  about to delete thread %lu: DOA: %s\n", FILE__, __LINE__, thrd->getTid(), thrd->isDeadOnArrival() ? "true" : "false");
   thrd->deleteThread();
}

#ifdef IBM_BPATCH_COMPAT
/**
 * In IBM's code, this is a wrapper for _BPatch_thread->addSharedObject (linux)
 * which is in turn a wrapper for creating a new 
 * ibmBpatchElf32Teader(name, addr)
 **/
bool BPatch_process::addSharedObjectInt(const char *name, 
                                        const unsigned long loadaddr)
{
   return loadLibraryInt(name);
}
#endif


/***************************************************************************
 * BPatch_snippetHandle
 ***************************************************************************/

/*
 * BPatchSnippetHandle::BPatchSnippetHandle
 *
 * Constructor for BPatchSnippetHandle.  Delete the snippet instance(s)
 * associated with the BPatchSnippetHandle.
 */
BPatchSnippetHandle::BPatchSnippetHandle(BPatch_process *proc) :
    proc_(proc)
{
}

/*
 * BPatchSnippetHandle::~BPatchSnippetHandle
 *
 * Destructor for BPatchSnippetHandle.  Delete the snippet instance(s)
 * associated with the BPatchSnippetHandle.
 */
void BPatchSnippetHandle::BPatchSnippetHandle_dtor()
{
   // don't delete inst instances since they are might have been copied
}

BPatch_process *BPatchSnippetHandle::getProcessInt()
{
  return proc_;
}

BPatch_Vector<BPatch_thread *> &BPatchSnippetHandle::getCatchupThreadsInt()
{
  return catchup_threads;
}

BPatch_function *BPatch_process::get_function(int_function *f) 
{ 
   if (!func_map->defines(f))
      return NULL;
   return func_map->get(f); 
}

extern void dyninst_yield();
void BPatch_process::updateThreadInfo()
{
   if (!llproc->multithread_capable())
      return;
   
   llproc->recognize_threads(NULL);
   
   //We want to startup the event handler thread even if there's
   // no registered handlers so we can start getting MT events.
   if (!getAsync()->startupThread())
       return;
}

/**
 * This function continues a stopped process, letting it execute in single step mode,
 * and printing the current instruction as it executes.
 **/
void BPatch_process::debugSuicideInt() {
    llproc->debugSuicide();
}

BPatch_thread *BPatch_process::handleThreadCreate(unsigned index, int lwpid, 
                                                  dynthread_t threadid, 
                                                  unsigned long stack_top, 
                                                  unsigned long start_pc, process *proc_)
{
   //bool thread_exists = (getThread(threadid) != NULL);
  if (!llproc && proc_) llproc = proc_;
  BPatch_thread *newthr = 
      createOrUpdateBPThread(lwpid, threadid, index, stack_top, start_pc);
  if (newthr->reported_to_user) {
     async_printf("%s[%d]:  NOT ISSUING CALLBACK:  thread %lu exists\n", 
                  FILE__, __LINE__, (long) threadid);
     return newthr;
  }

  pdvector<CallbackBase *> cbs;
  getCBManager()->dispenseCallbacksMatching(evtThreadCreate, cbs);
  
  for (unsigned int i = 0; i < cbs.size(); ++i) {
      BPatch::bpatch->signalNotificationFD();

     AsyncThreadEventCallback &cb = * ((AsyncThreadEventCallback *) cbs[i]);
     async_printf("%s[%d]:  before issuing thread create callback: tid %lu\n", 
                 FILE__, __LINE__, newthr->getTid());
     cb(this, newthr);
  }

  newthr->reported_to_user = true;
  BPatch::bpatch->mutateeStatusChange = true;
  llproc->sh->signalEvent(evtThreadCreate);

  if (newthr->isDeadOnArrival()) {
    //  thread was created, yes, but it also already exited...  set up and 
    //  execute thread exit callbacks too... (this thread will not trigger
    //  other thread events since we never attached to it)
    //  it is up to the user to check deadOnArrival() before doing anything
    //  with the thread object.
    pdvector<CallbackBase *> cbs;
    getCBManager()->dispenseCallbacksMatching(evtThreadExit, cbs);
    for (unsigned int i = 0; i < cbs.size(); ++i) {
        BPatch::bpatch->signalNotificationFD();
        BPatch::bpatch->mutateeStatusChange = true;
        llproc->sh->signalEvent(evtThreadExit);
        AsyncThreadEventCallback &cb = * ((AsyncThreadEventCallback *) cbs[i]);
        async_printf("%s[%d]:  before issuing thread exit callback: tid %lu\n", 
                     FILE__, __LINE__, newthr->getTid());
        cb(this, newthr);
    }
    
  }
  return newthr;
}

// Return true if any sub-minitramp uses a trap? Other option
// is "if all"...
bool BPatchSnippetHandle::usesTrapInt() {
    for (unsigned i = 0; i < mtHandles_.size(); i++) {
        if (mtHandles_[i]->instrumentedViaTrap())
            return true;
    }
    return false;
}
