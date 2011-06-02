/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

#define BPATCH_FILE

#include <string>

#include "process.h"
#include "EventHandler.h"
#include "mailbox.h"
#include "signalgenerator.h"
#include "inst.h"
#include "instP.h"
#include "instPoint.h"
#include "function.h" // func_instance
#include "codeRange.h"
#include "dyn_thread.h"
#include "miniTramp.h"

#include "mapped_module.h"
#include "mapped_object.h"

#include "BPatch_libInfo.h"
#include "BPatch_asyncEventHandler.h"
#include "BPatch.h"
#include "BPatch_point.h"
#include "BPatch_thread.h"
#include "BPatch_function.h"
#include "BPatch_basicBlock.h"
#include "callbacks.h"
#include "BPatch_module.h"
#include "hybridAnalysis.h"
#include "BPatch_private.h"
#include "parseAPI/h/CFG.h"
#include "ast.h"
#include "debug.h"
#include "MemoryEmulator/memEmulator.h"
#include <boost/tuple/tuple.hpp>

#include "PatchMgr.h"
#include "Relocation/DynAddrSpace.h"
#include "Relocation/DynPointMaker.h"
#include "Relocation/DynObject.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using PatchAPI::DynObjectPtr;
using PatchAPI::DynObject;
using PatchAPI::DynAddrSpace;
using PatchAPI::PatchMgr;
using PatchAPI::PointMakerPtr;

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
BPatch_process::BPatch_process(const char *path, const char *argv[],
                               BPatch_hybridMode mode, const char **envp,
                               int stdin_fd, int stdout_fd, int stderr_fd)
   : llproc(NULL), lastSignal(-1), exitCode(-1),
     exitedNormally(false), exitedViaSignal(false), mutationsActive(true),
     createdViaAttach(false), detached(false), unreportedStop(false),
     unreportedTermination(false), terminated(false), reportedExit(false),
     unstartedRPC(false), activeOneTimeCodes_(0),
     resumeAfterCompleted_(false), hybridAnalysis_(NULL)
{
   image = NULL;
   pendingInsertions = NULL;

   isVisiblyStopped = true;

   pdvector<std::string> argv_vec;
   pdvector<std::string> envp_vec;
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

   std::string directoryName = "";

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

   std::string spath(path);
   llproc = ll_createProcess(spath, &argv_vec, mode, this,
                             (envp ? &envp_vec : NULL),
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
   llproc->set_up_ptr(this);


   assert(BPatch::bpatch != NULL);

   // Create an initial thread
   startup_cerr << "Getting initial thread..." << endl;
   dyn_thread *dynthr = llproc->getInitialThread();
   BPatch_thread *initial_thread = new BPatch_thread(this, dynthr);
   threads.push_back(initial_thread);

   startup_cerr << "Creating new BPatch_image..." << endl;
   image = new BPatch_image(this);

   assert(llproc->isBootstrappedYet());

   assert(BPatch_heuristicMode != llproc->getHybridMode());
   if ( BPatch_normalMode != mode ) {
       BPatch::bpatch->setInstrStackFrames(true);
       hybridAnalysis_ = new HybridAnalysis(llproc->getHybridMode(),this);
   }

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
BPatch_process::BPatch_process
(const char *path, int pid, BPatch_hybridMode mode)
   : llproc(NULL), lastSignal(-1), exitCode(-1),
     exitedNormally(false), exitedViaSignal(false), mutationsActive(true),
     createdViaAttach(true), detached(false), unreportedStop(false),
     unreportedTermination(false), terminated(false), reportedExit(false),
     unstartedRPC(false), activeOneTimeCodes_(0), resumeAfterCompleted_(false),
     hybridAnalysis_(NULL)
{
   image = NULL;
   pendingInsertions = NULL;

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

    startup_printf("%s[%d]:  creating new BPatch_image...\n", FILE__, __LINE__);
   image = new BPatch_image(this);
    startup_printf("%s[%d]:  created new BPatch_image...\n", FILE__, __LINE__);
   std::string spath = path ? std::string(path) : std::string();
    startup_printf("%s[%d]:  attaching to process %s/%d\n", FILE__, __LINE__,
          path ? path : "no_path", pid);
   llproc = ll_attachProcess(spath, pid, this, mode);
   if (!llproc) {
      BPatch::bpatch->unRegisterProcess(pid, this);
      BPatch::bpatch->reportError(BPatchFatal, 68,
             "Dyninst was unable to attach to the specified process");
      return;
   }
    startup_printf("%s[%d]:  attached to process %s/%d\n", FILE__, __LINE__, path ? path : "no_path", pid);

   // Create an initial thread
   dyn_thread *dynthr = llproc->getInitialThread();
   BPatch_thread *initial_thread = new BPatch_thread(this, dynthr);
   threads.push_back(initial_thread);

   llproc->registerFunctionCallback(createBPFuncCB);
   llproc->registerInstPointCallback(createBPPointCB);
   llproc->set_up_ptr(this);

   assert(llproc->isBootstrappedYet());
   assert(llproc->status() == stopped);

   isAttemptingAStop = false;

   assert(BPatch_heuristicMode != llproc->getHybridMode());
   if ( BPatch_normalMode != mode ) {
       hybridAnalysis_ = new HybridAnalysis(llproc->getHybridMode(),this);
   }
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
   : llproc(nProc), lastSignal(-1), exitCode(-1),
     exitedNormally(false), exitedViaSignal(false), mutationsActive(true),
     createdViaAttach(true), detached(false),
     unreportedStop(false), unreportedTermination(false), terminated(false),
     reportedExit(false), unstartedRPC(false), activeOneTimeCodes_(0),
     resumeAfterCompleted_(false), hybridAnalysis_(NULL)
{
   // Add this object to the list of threads
   assert(BPatch::bpatch != NULL);
   image = NULL;
   pendingInsertions = NULL;

   BPatch::bpatch->registerProcess(this);

   // Create an initial thread
   for (unsigned i=0; i<llproc->threads.size(); i++)
   {
      dyn_thread *dynthr = llproc->threads[i];
      BPatch_thread *thrd = new BPatch_thread(this, dynthr);
      threads.push_back(thrd);
      BPatch::bpatch->registerThreadCreate(this, thrd);
   }

   llproc->registerFunctionCallback(createBPFuncCB);
   llproc->registerInstPointCallback(createBPPointCB);
   llproc->set_up_ptr(this);

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
       !getAsync()->detachFromProcess(llproc))
   {
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

   if (pendingInsertions)
   {
       for (unsigned f = 0; f < pendingInsertions->size(); f++)
	   {
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

   if (createdViaAttach)
   {
       llproc->detachProcess(true);
   }
   else
   {
       if (llproc->isAttached())
	   {
           proccontrol_printf("%s[%d]:  about to terminate execution\n", __FILE__, __LINE__);
           terminateExecutionInt();
       }
   }

   if (NULL != hybridAnalysis_) {
       delete hybridAnalysis_;
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
    if (statusIsTerminated()) return false;

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
       isAttemptingAStop = false;
       return false;
   }
}

/*
 * BPatch_process::continueExecution
 *
 * Puts the thread into the running state.
 */
bool BPatch_process::continueExecutionInt()
{
    if (statusIsTerminated()) {
        return false;
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
    // USER LEVEL CALL! BPatch_process should use
    // statusIsTerminated.

    // This call considers a process terminated if it has reached
    // or passed the entry to exit. The process may still exist,
    // but we no longer let the user modify it; hence, terminated.

    getMailbox()->executeCallbacks(FILE__, __LINE__);

    if (exitedNormally || exitedViaSignal) return true;

    // First see if we've already terminated to avoid
    // checking process status too often.
    if (reportedExit)
       return true;
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
   if (!getAsync()->detachFromProcess(llproc)) {
      bperr("%s[%d]:  trouble decoupling async event handler for process %d\n",
            __FILE__, __LINE__, getPid());
   }
  // __LOCK;
   if (image)
      image->removeAllModules();
   detached = llproc->detachProcess(cont);
   BPatch::bpatch->unRegisterProcess(getPid(), this);
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
   if(! llproc->isInferiorAllocated((Address)parentVar.getBaseAddr())) {
      // isn't defined in this process so must not have been defined in a
      // parent process
      return NULL;
   }

   return new BPatch_variableExpr(this, llproc, parentVar.getBaseAddr(), Null_Register,
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
        childMT = parent_mtHandles[i]->getInheritedMiniTramp(llproc);
        if (!childMT) {
            fprintf(stderr, "Failed to get inherited mini tramp\n");
            return NULL;
        }
        childSnippet->addMiniTramp(childMT);
    }
    return childSnippet;
}

/*
 * BPatch_addressSpace::beginInsertionSet
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
bool BPatch_process::finalizeInsertionSetInt(bool, bool *)
{
  // Can't insert code when mutations are not active.
  bool shouldContinue = false;
  if (!mutationsActive) {
    return false;
  }

  if ( ! statusIsStopped() ) {
    shouldContinue = true;
    stopExecutionInt();
  }

  /* PatchAPI stuffs */
  bool ret = AddressSpace::patch(llproc);
  /* End of PatchAPI stuffs */

  // bool ret = llproc->relocate();

  llproc->trapMapping.flush();

  if (shouldContinue)
    continueExecutionInt();

  if (pendingInsertions) {
    delete pendingInsertions;
    pendingInsertions = NULL;
  }

  return ret;
}


bool BPatch_process::finalizeInsertionSetWithCatchupInt(bool, bool *,
							BPatch_Vector<BPatch_catchupInfo> &)
{
   return false;
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

      BPatch::bpatch->signalNotificationFD();

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

      for (unsigned int i = 0; i < cbs.size(); ++i) {

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

   llproc->getRpcMgr()->postRPCtoDo(expr.ast_wrapper,
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
   if (statusIsTerminated()) {
      return false;
   }
   oneTimeCodeInternal(expr, NULL, userData,  cb, false, NULL);
   return true;
}

/*
 * BPatch_process::loadLibrary
 *
 * Load a dynamically linked library into the address space of the mutatee.
 *
 * libname	The name of the library to load.
 */
bool BPatch_process::loadLibraryInt(const char *libname, bool)
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
   BPatch_module* dyn_rt_lib = image->findModule("dyninstAPI_RT", true);
   if(dyn_rt_lib == NULL)
   {
      cerr << __FILE__ << ":" << __LINE__ << ": FATAL:  Cannot find module for "
           << "DyninstAPI Runtime Library" << endl;
      return false;
   }
   dyn_rt_lib->findFunction("DYNINSTloadLibrary", bpfv);
   if (!bpfv.size()) {
      cerr << __FILE__ << ":" << __LINE__ << ": FATAL:  Cannot find Internal"
           << "Function DYNINSTloadLibrary" << endl;
      return false;
   }
   if (bpfv.size() > 1) {
      std::string msg = std::string("Found ") + utos(bpfv.size()) +
         std::string("functions called DYNINSTloadLibrary -- not fatal but weird");
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
         dyn_rt_lib->findVariable("gLoadLibraryErrorString");
      assert(NULL != dlerror_str_var);
      char dlerror_str[256];
      dlerror_str_var->readValue((void *)dlerror_str, 256);
      BPatch_reportError(BPatchSerious, 124, dlerror_str);
      return false;
   }

   /* PatchAPI stuffs */
   mapped_object* plib = llproc->findObject(libname);
   if (plib) DYN_CAST(DynAddrSpace, llproc->mgr()->as())->loadLibrary(plib);
   /* End of PatchAPi stuffs */

   return true;
}

void BPatch_process::enableDumpPatchedImageInt(){
    // deprecated; saveTheWorld is dead. Do nothing for now; kill later.
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

bool BPatch_process::getType()
{
  return TRADITIONAL_PROCESS;
}

void BPatch_process::getAS(std::vector<AddressSpace *> &as)
{
   as.push_back(static_cast<AddressSpace*>(llproc));
}

BPatch_thread *BPatch_process::createOrUpdateBPThread(
                         int lwp, dynthread_t tid, unsigned index,
                         unsigned long stack_start,
                         unsigned long start_addr)
{
   async_printf("%s[%d]:  welcome to createOrUpdateBPThread(tid = %lu)\n",
         FILE__, __LINE__, tid);

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
      if (threads[i] == bpthr)
	  {
         found = true;
         break;
      }

   if (!found)
      threads.push_back(bpthr);

   BPatch_function *initial_func = NULL;
   initial_func = getImage()->findFunction(start_addr);

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

extern void dyninst_yield();
bool BPatch_process::updateThreadInfo()
{
   if (!llproc->multithread_capable())
      return true;

   if (!llproc->recognize_threads(NULL))
       return false;

   async_printf("%s[%d]:  about to startup async thread\n", FILE__, __LINE__);

   //We want to startup the event handler thread even if there's
   // no registered handlers so we can start getting MT events.
   if (!getAsync()->startupThread())
   {
	   async_printf("%s[%d]:  startup async thread failed\n", FILE__, __LINE__);
       return false;
   }

   async_printf("%s[%d]:  startup async thread: ok\n", FILE__, __LINE__);
   return true;
}

/**
 * This function continues a stopped process, letting it execute in single step mode,
 * and printing the current instruction as it executes.
 **/

void BPatch_process::debugSuicideInt()
{
    llproc->debugSuicide();
}

BPatch_thread *BPatch_process::handleThreadCreate(unsigned index, int lwpid,
                                                  dynthread_t threadid,
                                                  unsigned long stack_top,
                                                  unsigned long start_pc, process *proc_)
{
	async_printf("%s[%d]:  welcome to handleThreadCreate\n", FILE__, __LINE__);
   //bool thread_exists = (getThread(threadid) != NULL);

  if (!llproc && proc_)
	  llproc = proc_;

  BPatch_thread *newthr =
      createOrUpdateBPThread(lwpid, threadid, index, stack_top, start_pc);

  bool result = BPatch::bpatch->registerThreadCreate(this, newthr);

  if (!result)
     return newthr;

  if (newthr->isDeadOnArrival())
  {
    //  thread was created, yes, but it also already exited...  set up and
    //  execute thread exit callbacks too... (this thread will not trigger
    //  other thread events since we never attached to it)
    //  it is up to the user to check deadOnArrival() before doing anything
    //  with the thread object.
    BPatch::bpatch->signalNotificationFD();

    pdvector<CallbackBase *> cbs;
    getCBManager()->dispenseCallbacksMatching(evtThreadExit, cbs);

    for (unsigned int i = 0; i < cbs.size(); ++i)
	{
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
   return false;
}

/* BPatch::triggerStopThread
 *
 * Causes the execution of a callback in the mutator that was
 * triggered for the evtStopThread event. As BPatch_stopThreadExpr
 * snippets allow a different callback to be triggered to each
 * snippet instance, the cb_ID is used to find the right callback to
 * trigger. This code had to be in a BPatch-level class so that we
 * could utilize the findOrCreateBPFunc and findOrCreateBPPoint
 * functions.
 *
 * @intPoint: the instPoint at which the event occurred, will be
 *    wrapped in a BPatch_point and sent to the callback as a parameter
 * @intFunc: the function in which the event occurred, will be wrapped
 *    in a BPatch_function and sent to the callback as a parameter
 * @proc: the process is needed for the creation of BPatch level objects
 * @cb_ID: helps us identify the correct call
 * @retVal: the return value of a parameter snippet that gets passed
 *    down in the stopThread snippet and evaluated.
 *
 * Return Value: Will always be true if code unless an error occurs, a
 *    callback is triggered for every stopThread snippet instance.
 */
bool BPatch_process::triggerStopThread(instPoint *intPoint,
         func_instance *intFunc, int cb_ID, void *retVal)
{
    // find the BPatch_point corresponding to the instrumentation point
    BPatch_function *bpFunc = findOrCreateBPFunc(intFunc, NULL);
    BPatch_procedureLocation bpPointType =
        BPatch_point::convertInstPointType_t(intPoint->type());
    BPatch_point *bpPoint = findOrCreateBPPoint(bpFunc, intPoint, bpPointType);
    if (!bpPoint) {
        return false;
    }
    isVisiblyStopped = true;
    // trigger all callbacks matching the snippet and event type
    pdvector<CallbackBase *> cbs;
    getCBManager()->dispenseCallbacksMatching(evtStopThread,cbs);
    BPatch::bpatch->signalNotificationFD();//KEVINTODO: is this necessary for synchronous callbacks?
    StopThreadCallback *cb;
    for (unsigned i = 0; i < cbs.size(); ++i) {
        cb = dynamic_cast<StopThreadCallback *>(cbs[i]);
        if ( cb && cb_ID == llproc->getStopThreadCB_ID((Address)(cb->getFunc()))) {
            (*cb)(bpPoint, retVal);
        }
    }
    isVisiblyStopped = false;
    return true;
}


/* BPatch::triggerSignalHandlerCB
 *
 * Grabs BPatch level objects for the instPoint and enclosing function
 * and triggers any registered callbacks for this signal/exception
 *
 * @intPoint: the instPoint at which the event occurred, will be
 * wrapped in a BPatch_point and sent to the callback as a parameter
 * @intFunc: the function in which the event occurred, will be
 * wrapped in a BPatch_function and sent to the callback as a parameter
 *
 * Return Value: true if a matching callback was found and no error occurred
 *
 */
bool BPatch_process::triggerSignalHandlerCB(instPoint *intPoint,
        func_instance *intFunc, long signum, BPatch_Vector<Address> *handlers)
{
    // find the BPatch_point corresponding to the exception-raising instruction
    BPatch_function *bpFunc = findOrCreateBPFunc(intFunc, NULL);
    BPatch_procedureLocation bpPointType =
        BPatch_point::convertInstPointType_t(intPoint->type());
    BPatch_point *bpPoint = findOrCreateBPPoint(bpFunc, intPoint, bpPointType);
    if (!bpPoint) { return false; }
    // trigger all callbacks for this signal
    pdvector<CallbackBase *> cbs;
    getCBManager()->dispenseCallbacksMatching(evtSignalHandlerCB,cbs);
    BPatch::bpatch->signalNotificationFD();
    bool foundCallback = false;
    for (unsigned int i = 0; i < cbs.size(); ++i) {
        SignalHandlerCallback *cb =
            dynamic_cast<SignalHandlerCallback *>(cbs[i]);
        if (cb && cb->handlesSignal(signum)) {
            (*cb)(bpPoint, signum, handlers);
            foundCallback = true;
        }
    }
    return foundCallback;
}

/* BPatch::triggerCodeOverwriteCB
 *
 * Grabs BPatch level objects for the instPoint and enclosing function
 * and triggers a registered callback if there is one
 *
 * @intPoint: the instPoint at which the event occurred, will be
 * wrapped in a BPatch_point and sent to the callback as a parameter
 *
 * Return Value: true if a matching callback was found and no error occurred
 */
bool BPatch_process::triggerCodeOverwriteCB(instPoint *faultPoint,
                                            Address faultTarget)
{
    // does the callback exist?
    pdvector<CallbackBase *> cbs;
    if ( ! getCBManager()->dispenseCallbacksMatching(evtCodeOverwrite,cbs) ) {
        return false;
    }

    // find the matching callbacks and trigger them
    BPatch_function *bpFunc = findOrCreateBPFunc(faultPoint->func(),NULL);
    BPatch_point *bpPoint = findOrCreateBPPoint(
        bpFunc,
        faultPoint,
        BPatch_point::convertInstPointType_t(faultPoint->type()));
    BPatch::bpatch->signalNotificationFD();
    bool foundCallback = false;
    for (unsigned int i = 0; i < cbs.size(); ++i)
    {
        CodeOverwriteCallback *cb =
            dynamic_cast<CodeOverwriteCallback *>(cbs[i]);
        if (cb) {
            foundCallback = true;

            (*cb)(bpPoint, faultTarget, lowlevel_process());

        }
    }
    return foundCallback;
}

/* This is a Windows only function that sets the user-space
 * debuggerPresent flag to 0 or 1, 0 meaning that the process is not
 * being debugged.  The debugging process will still have debug
 * access, but system calls that ask if the process is being debugged
 * will say that it is not because they merely return the value of the
 * user-space beingDebugged flag.
 */
bool BPatch_process::hideDebuggerInt()
{
    // do non-instrumentation related hiding
    bool retval = llproc->hideDebugger();

    // disable API calls //
    vector<pair<BPatch_function *,BPatch_function *> > disabledFuncs;
    BPatch_module *user = image->findModule("user32.dll",true);
    BPatch_module *kern = image->findModule("*kernel32.dll",true);

    if (user) {
        // BlockInput
        using namespace SymtabAPI;
        vector<BPatch_function*> funcs;
        user->findFunction(
            "BlockInput",
            funcs, false, false, false, true);
        assert (funcs.size());
        BPatch_module *rtlib = this->image->findOrCreateModule(
            (*llproc->runtime_lib.begin())->getModules().front());
        vector<BPatch_function*> repfuncs;
        rtlib->findFunction("DYNINST_FakeBlockInput", repfuncs, false);
        assert(!repfuncs.empty());
        replaceFunction(*funcs[0],*repfuncs[0]);
        disabledFuncs.push_back(pair<BPatch_function*,BPatch_function*>(
                                funcs[0],repfuncs[0]));
    }

    if (kern) {
        // SuspendThread
        // KEVINTODO: condition the function replacement on its thread ID parameter matching a Dyninst thread
        using namespace SymtabAPI;
        vector<BPatch_function*> funcs;
        kern->findFunction(
            "SuspendThread",
            funcs, false, false, false, true);
        assert (funcs.size());
        BPatch_module *rtlib = this->image->findOrCreateModule(
            (*llproc->runtime_lib.begin())->getModules().front());
        vector<BPatch_function*> repfuncs;
        rtlib->findFunction("DYNINST_FakeSuspendThread", repfuncs, false);
        assert(!repfuncs.empty());
        replaceFunction(*funcs[0],*repfuncs[0]);
        disabledFuncs.push_back(pair<BPatch_function*,BPatch_function*>(
                                funcs[0],repfuncs[0]));
    }

    if (kern) {
        // getTickCount
        using namespace SymtabAPI;
        vector<BPatch_function*> funcs;
        kern->findFunction(
            "GetTickCount",
            funcs, false, false, false, true);
        assert (!funcs.empty());
        BPatch_module *rtlib = this->image->findOrCreateModule(
            (*llproc->runtime_lib.begin())->getModules().front());
        vector<BPatch_function*> repfuncs;
        rtlib->findFunction("DYNINST_FakeTickCount", repfuncs, false);
        assert(!repfuncs.empty());
        replaceFunction(*funcs[0],*repfuncs[0]);
        disabledFuncs.push_back(pair<BPatch_function*,BPatch_function*>(
                                funcs[0],repfuncs[0]));
    }

    if (kern) {
        // getSystemTime
        using namespace SymtabAPI;
        vector<BPatch_function*> funcs;
        kern->findFunction(
            "GetSystemTime",
            funcs, false, false, false, true);
        assert (!funcs.empty());
        BPatch_module *rtlib = this->image->findOrCreateModule(
            (*llproc->runtime_lib.begin())->getModules().front());
        vector<BPatch_function*> repfuncs;
        rtlib->findFunction("DYNINST_FakeGetSystemTime", repfuncs, false);
        assert(!repfuncs.empty());
        replaceFunction(*funcs[0],*repfuncs[0]);
        disabledFuncs.push_back(pair<BPatch_function*,BPatch_function*>(
                                funcs[0],repfuncs[0]));
    }

    if (kern) {
        // CheckRemoteDebuggerPresent
        vector<BPatch_function*> funcs;
        kern->findFunction(
            "CheckRemoteDebuggerPresent",
            funcs, false, false, true);
        assert (funcs.size());
        BPatch_module *rtlib = this->image->findOrCreateModule(
            (*llproc->runtime_lib.begin())->getModules().front());
        vector<BPatch_function*> repfuncs;
        rtlib->findFunction("DYNINST_FakeCheckRemoteDebuggerPresent", repfuncs, false);
        assert(!repfuncs.empty());
        replaceFunction(*funcs[0],*repfuncs[0]);
        disabledFuncs.push_back(pair<BPatch_function*,BPatch_function*>(
                                funcs[0],repfuncs[0]));
    }

    if (kern && user) {
        // OutputDebugStringA
        vector<BPatch_function*> funcs;
        kern->findFunction("OutputDebugStringA",
            funcs, false, false, true);
        assert(funcs.size());
        vector<BPatch_function*> sle_funcs;
        user->findFunction("SetLastErrorEx", sle_funcs,
                           false, false, true, true);
        assert(!sle_funcs.empty());
        vector<BPatch_snippet*> args;
        BPatch_constExpr lasterr(1);
        args.push_back(&lasterr);
        args.push_back(&lasterr); // need a second parameter, but it goes unused by windows
        BPatch_funcCallExpr callSLE (*(sle_funcs[0]), args);
        vector<BPatch_point*> *exitPoints = sle_funcs[0]->findPoint(BPatch_exit);
        beginInsertionSet();
        for (unsigned i=0; i < exitPoints->size(); i++) {
            insertSnippet( callSLE, *((*exitPoints)[i]) );
        }
    }

    if (NULL != hybridAnalysis_) {
        hybridAnalysis_->addReplacedFuncs(disabledFuncs);
    }
    finalizeInsertionSet(false);

    if (!user || !kern) {
        retval = false;
    }
    return retval;
}

bool BPatch_process::setMemoryAccessRights
(Address start, Address size, int rights)
{
    return llproc->setMemoryAccessRights(start,size,rights);
}

unsigned char * BPatch_process::makeShadowPage(Dyninst::Address pageAddr)
{
    unsigned pagesize = llproc->getMemoryPageSize();
    pageAddr = (pageAddr / pagesize) * pagesize;

    Address shadowAddr = pageAddr;
    if (llproc->isMemoryEmulated()) {
        bool valid = false;
        boost::tie(valid, shadowAddr) = llproc->getMemEm()->translate(pageAddr);
        assert(valid);
    }

    unsigned char* buf = (unsigned char*) ::malloc(pagesize);
    llproc->readDataSpace((void*)shadowAddr, pagesize, buf, true);
    return buf;
}

// is the first instruction: [00 00] add byte ptr ds:[eax],al ?
static bool hasWeirdEntryBytes(func_instance *func)
{
    using namespace SymtabAPI;
    Symtab *sym = func->obj()->parse_img()->getObject();
    if (sym->findEnclosingRegion(func->addr())
        !=
        sym->findEnclosingRegion(func->addr()+1))
    {
        return false;
    }
    unsigned short ebytes;
    memcpy(&ebytes,func->obj()->getPtrToInstruction(func->addr()),2);
    //proc()->readDataSpace((void*)func->addr(), sizeof(short), &ebytes, false);

    if (0 == ebytes) {
        return true;
    }
    return false;
}

// return true if the analysis changed
//
void BPatch_process::overwriteAnalysisUpdate
    ( std::map<Dyninst::Address,unsigned char*>& owPages, //input
      std::vector<Dyninst::Address>& deadBlockAddrs, //output
      std::vector<BPatch_function*>& owFuncs, //output: overwritten & modified
      std::set<BPatch_function *> &monitorFuncs, // output: those that call overwritten or modified funcs
      bool &changedPages, bool &changedCode) //output
{
   assert(0 && "FIXME!");

#if 0

    //1.  get the overwritten blocks and regions
    std::list<std::pair<Address,Address> > owRegions;
    std::list<block_instance *> owBBIs;
    llproc->getOverwrittenBlocks(owPages, owRegions, owBBIs);
    changedPages = ! owRegions.empty();
    changedCode = ! owBBIs.empty();

    /*2. remove dead code from the analysis */

    if ( !changedCode ) {
        // update the mapped data for the overwritten ranges
        llproc->updateCodeBytes(owRegions);
        return;
    }

    // identify the dead code
    std::set<block_instance*> delBBIs;
    std::map<func_instance*,set<block_instance*> > elimMap;
    std::list<func_instance*> deadFuncs;
    std::map<func_instance*,block_instance*> newFuncEntries;
    llproc->getDeadCode(owBBIs,delBBIs,elimMap,deadFuncs,newFuncEntries);

    // remove instrumentation from affected funcs
    beginInsertionSet();
    for(std::map<func_instance*,set<block_instance*> >::iterator fIter = elimMap.begin();
        fIter != elimMap.end();
        fIter++)
    {
        BPatch_function *bpfunc = findOrCreateBPFunc(fIter->first,NULL);
        bpfunc->removeInstrumentation(false);
    }

    //remove instrumentation from dead functions
    for(std::list<func_instance*>::iterator fit = deadFuncs.begin();
        fit != deadFuncs.end();
        fit++)
    {
        // remove instrumentation
        findOrCreateBPFunc(*fit,NULL)->removeInstrumentation(true);
    }

    // update the mapped data for the overwritten ranges
    llproc->updateCodeBytes(owRegions);

    finalizeInsertionSet(false);

    // create stub edge set which is: all edges such that:
    //     e->trg() in owBBIs and
    //     while e->src() in delBlocks try e->src()->sources()
    std::map<func_instance*,vector<edgeStub> > stubs =
       llproc->getStubs(owBBIs,delBBIs,deadFuncs);

    // remove dead springboards
    for(set<block_instance*>::iterator bit = delBBIs.begin();
        bit != delBBIs.end();
        bit++)
    {
        llproc->getMemEm()->removeSpringboards(*bit);
    }
    for(list<func_instance*>::iterator fit = deadFuncs.begin();
        fit != deadFuncs.end();
        fit++)
    {
        malware_cerr << "Removing instrumentation from dead func at "
            << (*fit)->addr() << endl;
        llproc->getMemEm()->removeSpringboards(*fit);
    }

    // delete delBlocks
    std::set<func_instance*> modFuncs;
    for(set<block_instance*>::iterator bit = delBBIs.begin();
        bit != delBBIs.end();
        bit++)
    {
        mal_printf("Deleting block [%lx %lx) from func at %lx\n",
                   (*bit)->start(),(*bit)->end(),(*bit)->func()->addr());
        func_instance *bFunc = (*bit)->func();
        modFuncs.insert(bFunc);
        if ((*bit)->getHighLevelBlock()) {
            ((BPatch_basicBlock*)(*bit)->getHighLevelBlock())
                ->setlowlevel_block(NULL);
        }
        deadBlockAddrs.push_back((*bit)->start());
        vector<ParseAPI::Block*> bSet;
        bSet.push_back((*bit)->llb());
        bFunc->deleteBlock((*bit));
        ParseAPI::Block *newEntry = NULL;
        if (newFuncEntries.end() != newFuncEntries.find(bFunc)) {
            newEntry = newFuncEntries[bFunc]->llb();
            bFunc->ifunc()->setEntryBlock(newEntry);
        }
        bFunc->ifunc()->destroyBlocks(bSet); //KEVINTODO: doing this one by one is highly inefficient
    }
    mal_printf("Done deleting blocks\n");
    // delete completely dead functions
    map<block_instance*,Address> deadFuncCallers; // build up list of live callers
    for(std::list<func_instance*>::iterator fit = deadFuncs.begin();
        fit != deadFuncs.end();
        fit++)
    {
        using namespace ParseAPI;
        Address funcAddr = (*fit)->addr();

        // grab callers that aren't also dead
        Block::edgelist &callEdges = (*fit)->ifunc()->entryBlock()->sources();
        Block::edgelist::iterator eit = callEdges.begin();
        for( ; eit != callEdges.end(); ++eit) {
            if (CALL == (*eit)->type()) {// includes tail calls
                parse_block *cBlk = (parse_block*)((*eit)->src());
                vector<ParseAPI::Function*> cFuncs;
                cBlk->getFuncs(cFuncs);
                for (unsigned fix=0; fix < cFuncs.size(); fix++) {
                    func_instance *cfunc = llproc->findFunction(
                        (parse_func*)(cFuncs[fix]));
                    block_instance *cbbi = cfunc->findBlock(cBlk);
                    if (delBBIs.end() != delBBIs.find(cbbi)) {
                        continue;
                    }
                    bool isFuncDead = false;
                    for (std::list<func_instance*>::iterator dfit = deadFuncs.begin();
                         dfit != deadFuncs.end();
                         dfit++)
                    {
                        if (cfunc == *dfit) {
                            isFuncDead = true;
                            break;
                        }
                    }
                    if (isFuncDead) {
                        continue;
                    }
                    if ( (*fit)->ifunc()->hasWeirdInsns() ||
                         hasWeirdEntryBytes(*fit) )
                    {
                       instPoint *cPoint = cbbi->preCallPoint();

                       monitorFuncs.insert(findOrCreateBPFunc(cfunc, NULL));
                    } else {
                        // parse right away
                        deadFuncCallers[cbbi] = funcAddr;
                    }
                }
            }
        }

        // add blocks to deadBlockAddrs

        const func_instance::BlockSet&
            deadBlocks = (*fit)->blocks();
        set<block_instance* ,block_instance::compare>::const_iterator
                bIter= deadBlocks.begin();
        for (; bIter != deadBlocks.end(); bIter++) {
            deadBlockAddrs.push_back((*bIter)->start());
        }
    }
    mal_printf("Done deleting func-blocks\n");
    //remove dead functions
    for(std::list<func_instance*>::iterator fit = deadFuncs.begin();
        fit != deadFuncs.end();
        fit++)
    {
        BPatch_function *bpfunc = findOrCreateBPFunc(*fit,NULL);
        bpfunc->getModule()->removeFunction(bpfunc,false);
        (*fit)->removeFromAll();
    }
    mal_printf("Done deleting functions\n");

    // set up datastructures for re-parsing dead function entries with valid call edges
    map<mapped_object*,vector<edgeStub> > dfstubs;
    set<Address> reParsedFuncs;
    vector<BPatch_module*> dontcare;
    vector<Address> targVec;
    for (map<block_instance*,Address>::iterator bit = deadFuncCallers.begin();
         bit != deadFuncCallers.end();
         bit++)
    {
        // the function is still reachable, reparse it
        if (reParsedFuncs.end() == reParsedFuncs.find(bit->second)) {
            reParsedFuncs.insert(bit->second);
            targVec.push_back(bit->second);
        }
        // re-instate call edges to the function
        dfstubs[bit->first->func()->obj()].push_back(edgeStub(bit->first,bit->second,ParseAPI::CALL));
    }

    // re-parse the functions
    for (map<mapped_object*,vector<edgeStub> >::iterator mit= dfstubs.begin();
         mit != dfstubs.end(); mit++)
    {
        if (getImage()->parseNewFunctions(dontcare, targVec)) {
            // add function to output vector
            for (unsigned tidx=0; tidx < targVec.size(); tidx++) {
                BPatch_function *bpfunc = findFunctionByEntry(targVec[tidx]);
                if (!bpfunc) {
                    owFuncs.push_back(bpfunc);
                } else {
                    // couldn't reparse
                    mal_printf("WARNING: Couldn't re-parse some of the overwritten "
                               "functions\n", targVec[tidx], FILE__,__LINE__);
                }
            }
        }
        mit->first->parseNewEdges(mit->second);
    }

    // set new entry points for functions with NewF blocks
    for (std::map<func_instance*,block_instance*>::iterator nit = newFuncEntries.begin();
         nit != newFuncEntries.end();
         nit++)
    {
        block_instance *entry = nit->first->setNewEntryPoint();
        if (entry != nit->second) {
            mal_printf("For overwritten executing func chose entry "
                       "block %lx rather than active block %lx %s %d\n",
                       entry->start(),
                       nit->second->start(),FILE__,__LINE__);
        }
    }

    //3. parse new code, one overwritten function at a time
    for(std::map<func_instance*,set<block_instance*> >::iterator
        fit = elimMap.begin();
        fit != elimMap.end();
        fit++)
    {
        // parse new edges in the function
        if (stubs[fit->first].size())
        {
            fit->first->obj()->parseNewEdges(stubs[fit->first]);
            modFuncs.insert(fit->first);
        }
        else if (newFuncEntries.end() == newFuncEntries.find(fit->first))
        {
            mal_printf("WARNING: didn't have any stub edges for overwritten "
                       "func %lx\n", fit->first->addr());
            vector<edgeStub> svec;
            svec.push_back(edgeStub(
                NULL, fit->first->addr(), ParseAPI::NOEDGE));
		    fit->first->obj()->parseNewEdges(svec);
            assert(0);
        }
        // else, this is the entry point of the function, do nothing,
        // we'll parse this anyway through recursive traversal if there's
        // code at this address

        // add curFunc to owFuncs, and clear the function's BPatch_flowGraph
        BPatch_function *bpfunc = findOrCreateBPFunc(fit->first,NULL);
        bpfunc->removeCFG();
        owFuncs.push_back(bpfunc);
    }

    // do a consistency check
    for (set<func_instance*>::iterator fit = modFuncs.begin();
        fit != modFuncs.end();
        fit++)
    {
        (*fit)->ifunc()->blocks(); // force ParseAPI func finalization
        (*fit)->addMissingBlocks();
        assert((*fit)->consistency());
    }
#endif
}


/* Protect analyzed code without protecting relocated code in the
 * runtime library and for now only protect code in the aOut,
 * also don't protect code that hasn't been analyzed
 */
bool BPatch_process::protectAnalyzedCode()
{
    bool ret = true;
    BPatch_Vector<BPatch_module *> *bpMods = image->getModules();
    for (unsigned midx=0; midx < bpMods->size(); midx++) {
       if (!(*bpMods)[midx]->setAnalyzedCodeWriteable(false)) {
           ret = false;
       }
    }
    return false;
}

void BPatch_process::set_llproc(process *proc)
{
    assert(NULL == llproc);
    llproc = proc;
}
