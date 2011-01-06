/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

#ifdef sparc_sun_solaris2_4
#include <dlfcn.h>
#endif

#define BPATCH_FILE

#include <string>

#include "inst.h"
#include "instP.h"
#include "instPoint.h"
#include "function.h" // int_function
#include "codeRange.h"
#include "miniTramp.h"
#include "pcProcess.h"
#include "pcThread.h"
#include "pcEventHandler.h"
#include "os.h"

#include "mapped_module.h"

#include "BPatch_libInfo.h"
#include "BPatch.h"
#include "BPatch_thread.h"
#include "BPatch_function.h"
#include "BPatch_module.h"
#include "hybridAnalysis.h"
#include "BPatch_private.h"
#include "parseAPI/h/CFG.h"
#include "ast.h"
#include "debug.h"
#include "eventLock.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

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
   return llproc ? llproc->getPid() : -1;
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
     createdViaAttach(false), detached(false), 
     terminated(false), reportedExit(false),
     activeOneTimeCodes_(0),
     resumeAfterCompleted_(false), hybridAnalysis_(NULL)
{
   image = NULL;
   pendingInsertions = NULL;

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
   llproc = PCProcess::createProcess(spath, &argv_vec, mode, (envp ? &envp_vec : NULL),
                             directoryName, stdin_fd, stdout_fd, stderr_fd,
                             BPatch::bpatch->eventHandler_);
   if (llproc == NULL) {
      BPatch_reportError(BPatchFatal, 68,
           "Dyninst was unable to create the specified process");
      return;
   }
   startup_cerr << "Registering function callback..." << endl;
   llproc->registerFunctionCallback(createBPFuncCB);
   
   startup_cerr << "Registering instPoint callback..." << endl;
   llproc->registerInstPointCallback(createBPPointCB);
   llproc->set_up_ptr(this);


   // Add this object to the list of processes
   assert(BPatch::bpatch != NULL);
   startup_cerr << "Registering process..." << endl;
   BPatch::bpatch->registerProcess(this);

   // Create an initial thread
   startup_cerr << "Getting initial thread..." << endl;
   PCThread *thr = llproc->getInitialThread();
   BPatch_thread *initial_thread = new BPatch_thread(this, thr);
   threads.push_back(initial_thread);

   startup_cerr << "Creating new BPatch_image..." << endl;
   image = new BPatch_image(this);

   assert(llproc->isBootstrapped());

   assert(BPatch_heuristicMode != llproc->getHybridMode());
   if ( BPatch_normalMode != mode ) {
       hybridAnalysis_ = new HybridAnalysis(llproc->getHybridMode(),this);
   }

   // Let's try to profile memory usage
#if defined(PROFILE_MEM_USAGE)
   void *mem_usage = sbrk(0);
   fprintf(stderr, "Post BPatch_process: sbrk %p\n", mem_usage);
#endif

   startup_cerr << "BPatch_process::BPatch_process, completed." << endl;
}

#if defined(os_linux)
/* Particular linux kernels running dyninst in particular patterns
   (namely, with a single process having spawned the mutator and the
   mutatee) are susceptible to a kernel bug that will cause a panic
   if the mutator exits before the mutatee. See the comment above
   class ForkNewProcessCallback : public DBICallbackBase in 
   debuggerinterface.h for details.
*/
bool LinuxConsideredHarmful(pid_t pid) // PUSH
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
     createdViaAttach(true), detached(false), 
     terminated(false), reportedExit(false),
     activeOneTimeCodes_(0), resumeAfterCompleted_(false),
     hybridAnalysis_(NULL)
{
   image = NULL;
   pendingInsertions = NULL;

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

   assert(BPatch::bpatch != NULL);

    startup_printf("%s[%d]:  creating new BPatch_image...\n", FILE__, __LINE__);
   image = new BPatch_image(this);
    startup_printf("%s[%d]:  created new BPatch_image...\n", FILE__, __LINE__);
   std::string spath = path ? std::string(path) : std::string();
    startup_printf("%s[%d]:  attaching to process %s/%d\n", FILE__, __LINE__, 
          path ? path : "no_path", pid);

   llproc = PCProcess::attachProcess(spath, pid, mode, BPatch::bpatch->eventHandler_);
   if (!llproc) {
      BPatch_reportError(BPatchFatal, 68, 
             "Dyninst was unable to attach to the specified process");
      return;
   }

   BPatch::bpatch->registerProcess(this, pid);
   startup_printf("%s[%d]:  attached to process %s/%d\n", FILE__, __LINE__, path ? path : 
            "no_path", pid);

   // Create an initial thread
   PCThread *thr = llproc->getInitialThread();
   BPatch_thread *initial_thread = new BPatch_thread(this, thr);
   threads.push_back(initial_thread);

   llproc->registerFunctionCallback(createBPFuncCB);
   llproc->registerInstPointCallback(createBPPointCB);
   llproc->set_up_ptr(this);

   assert(llproc->isBootstrapped());
   assert(llproc->isStopped());

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
BPatch_process::BPatch_process(PCProcess *nProc)
   : llproc(nProc), lastSignal(-1), exitCode(-1),
     exitedNormally(false), exitedViaSignal(false), mutationsActive(true), 
     createdViaAttach(true), detached(false),
     terminated(false),
     reportedExit(false), activeOneTimeCodes_(0),
     resumeAfterCompleted_(false), hybridAnalysis_(NULL)
{
   // Add this object to the list of threads
   assert(BPatch::bpatch != NULL);
   image = NULL;
   pendingInsertions = NULL;

   BPatch::bpatch->registerProcess(this);

   // Create the initial threads
   pdvector<PCThread *> llthreads;
   llproc->getThreads(llthreads);
   for (pdvector<PCThread *>::iterator i = llthreads.begin();
           i != llthreads.end(); ++i)
   {
      BPatch_thread *thrd = new BPatch_thread(this, *i);
      threads.push_back(thrd);
   }

   llproc->registerFunctionCallback(createBPFuncCB);
   llproc->registerInstPointCallback(createBPPointCB);
   llproc->set_up_ptr(this);

   image = new BPatch_image(this);
}

/*
 * BPatch_process::~BPatch_process
 *
 * Destructor for BPatch_process.
 */
void BPatch_process::BPatch_process_dtor()
{
   if( llproc ) {
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
       delete llproc;
       llproc = NULL;
   }

   for (int i=threads.size()-1; i>=0; i--) {
       delete threads[i];
   }

   if (image) delete image;
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
   
   if (NULL != hybridAnalysis_) {
       delete hybridAnalysis_;
   }

   assert(BPatch::bpatch != NULL);
}

/*
 * BPatch_process::triggerInitialThreadEvents
 *
 * Events and callbacks shouldn't be delivered from a constructor so after a
 * BPatch_process is constructed, this should be called.
 */
void BPatch_process::triggerInitialThreadEvents() {
    // For compatibility, only do this for multithread capable processes
    if( llproc->multithread_capable() ) {
        for (BPatch_Vector<BPatch_thread *>::iterator i = threads.begin();
                i != threads.end(); ++i) 
        {
            BPatch::bpatch->registerThreadCreate(this, *i);
        }
    }
}

/*
 * BPatch_process::stopExecution
 *
 * Puts the thread into the stopped state.
 */
bool BPatch_process::stopExecutionInt() 
{
    if( NULL == llproc ) return false;

    // The user has already indicated they would like the process stopped
    if( llproc->getDesiredProcessState() == PCProcess::ps_stopped ) return true;

    llproc->setDesiredProcessState(PCProcess::ps_stopped);
    return llproc->stopProcess();
}

/*
 * BPatch_process::continueExecution
 *
 * Puts the thread into the running state.
 */
bool BPatch_process::continueExecutionInt() 
{
    if( NULL == llproc ) return false;

    if( !llproc->isBootstrapped() ) return false;

    // The user has already indicated they would like the process running
    if( llproc->getDesiredProcessState() == PCProcess::ps_running ) return true;

    llproc->setDesiredProcessState(PCProcess::ps_running);

    return llproc->continueProcess();
}

/*
 * BPatch_process::terminateExecution
 *
 * Kill the thread.
 */
bool BPatch_process::terminateExecutionInt() 
{
    if( NULL == llproc ) return false;

    if( isTerminated() ) return true;

    proccontrol_printf("%s[%d]:  about to terminate proc\n", FILE__, __LINE__);
    return llproc->terminateProcess();
}

/*
 * BPatch_process::isStopped
 *
 * Returns true if the thread has stopped, and false if it has not.  
 */
bool BPatch_process::isStoppedInt()
{
    if( llproc == NULL ) return true;

    // The state visible to the user is different than the state
    // maintained by ProcControlAPI because processes remain in
    // a stopped state while doing event handling -- the user 
    // shouldn't see the process in a stopped state in this
    // case
    //
    // The following list is all cases where the user should see
    // the process stopped:
    // 1) BPatch_process::stopExecution is invoked
    // 2) A snippet breakpoint occurs
    // 3) The mutatee is delivered a stop signal

    return llproc->getDesiredProcessState() == PCProcess::ps_stopped;
}

/*
 * BPatch_process::stopSignal
 *
 * Returns the number of the signal which caused the thread to stop.
 */
int BPatch_process::stopSignalInt()
{
    if (!isStoppedInt()) {
        BPatch::reportError(BPatchWarning, 0, 
                "Request for stopSignal when process is not stopped");
        return -1;
    }
    return lastSignal;
}

/*
 * BPatch_process::statusIsTerminated
 *
 * Returns true if the process has terminated, false if it has not.
 */
bool BPatch_process::statusIsTerminated()
{
   if (llproc == NULL) return true;
   return llproc->isTerminated();
}

/*
 * BPatch_process::isTerminated
 *
 * Returns true if the thread has terminated, and false if it has not.  This
 * may involve checking for thread events that may have recently changed this
 * thread's status.  
 */
bool BPatch_process::isTerminatedInt()
{
    if( NULL == llproc ) return true;

    if( exitedNormally || exitedViaSignal ) return true;

    return llproc->isTerminated();
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
   if (image)
      image->removeAllModules();
   detached = llproc->detachProcess(cont);
   BPatch::bpatch->unRegisterProcess(getPid(), this);
   return detached;
}

/*
 * BPatch_process::isDetached
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
   bool was_stopped = isStoppedInt();

   stopExecution();

   bool ret = llproc->dumpCore(file);
   if (ret && terminate) {
      terminateExecutionInt();
   } else if (!was_stopped) {
      continueExecutionInt();
   }
    
   return ret;
}

/* 
 * BPatch_process::dumpPatchedImage
 *
 * No longer supported
 */
char* BPatch_process::dumpPatchedImageInt(const char*)
{
   return NULL;
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
   bool was_stopped = isStoppedInt();

   stopExecutionInt();

   bool ret = llproc->dumpImage(file);
   if (!was_stopped) continueExecutionInt();

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
bool doingCatchup = false;
/*static*/ pdvector<pdvector<Frame> >  stacks;
/*static*/ pdvector<Address> pcs;

bool BPatch_process::finalizeInsertionSetInt(bool atomic, bool *modified) 
{
    // Can't insert code when mutations are not active.
    bool shouldContinue = false;
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

    // Define up here so we don't have gotos causing issues
    std::set<int_function *> instrumentedFunctions;

    if (!isStoppedInt()) {
       stopExecutionInt();
       shouldContinue = true;
    }

    if (!llproc->walkStacks(stacks)) {
       inst_printf("%s[%d]:  walkStacks failed\n", FILE__, __LINE__);
       if (shouldContinue) 
          continueExecutionInt();
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
           if (shouldContinue) 
              continueExecutionInt();
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
            
            miniTramp *mini = point->addInst(bir->snip.ast_wrapper,
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

   // Having inserted the requests, we hand things off to functions to 
   // actually do work. First, develop a list of unique functions. 

   for (unsigned i = 0; i < pendingInsertions->size(); i++) {
       batchInsertionRecord *&bir = (*pendingInsertions)[i];
       for (unsigned j = 0; j < bir->points_.size(); j++) {
           BPatch_point *bppoint = bir->points_[j];
		   if (!bppoint)
		   {
			   fprintf(stderr, "%s[%d]:  FIXME!\n", FILE__, __LINE__);
			   continue;
		   }
           instPoint *point = bppoint->point;
		   if (!point)
		   {
			   fprintf(stderr, "%s[%d]:  FIXME!\n", FILE__, __LINE__);
			   continue;
		   }

           point->optimizeBaseTramps(bir->when_[j]);
           instrumentedFunctions.insert(point->func());
       }
   }

   for (std::set<int_function *>::iterator funcIter = instrumentedFunctions.begin();
        funcIter != instrumentedFunctions.end();
        funcIter++) {
       pdvector<instPoint *> failedInstPoints;
       (*funcIter)->performInstrumentation(atomic,
                                           failedInstPoints);
       if (failedInstPoints.size() && atomic) {
           err = true;
           goto cleanup;
       }
   }

#if 0

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
               bperr("ERROR: failed to insert instrumentation: generate\n");
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
              bperr("ERROR: failed to insert instrumentation: install\n");
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
             
          if (!point->linkInst(false)) {
             bperr("ERROR: failed to insert instrumentation: link\n");
               err = true;
           }

           if (atomic && err) break;
       }
       if (atomic && err) break;
   }

   if (atomic && err) 
      goto cleanup;
#endif

   // Move this to function instrumentation? It's more whole-process,
   // as it modifies stacks.

    if (modified) *modified = false; 
    for (unsigned thrIter = 0; thrIter < stacks.size(); thrIter++) {
        for (unsigned sIter = 0; sIter < stacks[thrIter].size(); sIter++) {
            bool fixedFrame = false;
            for (unsigned ptIter = 0; 
                 !fixedFrame && ptIter < pts.size(); 
                 ptIter++) 
            {
                instPoint *pt = pts[ptIter];
                if (pt->instrSideEffect(stacks[thrIter][sIter])) {
                    if (modified) *modified = true;
                    fixedFrame = true;
                }
            }
        }
    }


   llproc->trapMapping.flush();

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
    if (shouldContinue) 
       continueExecutionInt();

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
      fprintf(stderr, "%s[%d]:  BEGIN CATCHUP ANALYSIS:  num inst req: %ld\n", 
              FILE__, __LINE__, (long) pendingInsertions->size());
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
           // Are we in the "Active" (executing) frame?
          bool active = (j == (((signed) one_stack.size()) -1)) ? true : false;

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
                   const char *point_type = "no type";
                   switch(pt->getPointType()) {
                   case noneType: point_type = "noneType"; break;
                   case functionEntry: point_type = "funcEntry"; break;
                   case functionExit: point_type = "funcExit"; break;
                   case callSite: point_type = "callSite"; break;
                   case otherPoint: point_type = "otherPoint"; break;
                   case abruptEnd: point_type = "abruptEnd"; break;
                   default: assert(0);
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
               PCThread *thr = frame.getThread();
               assert(thr);
               dynthread_t tid = thr->getTid();
               BPatch_process *bpproc = dynamic_cast<BPatch_process *>(sh->getAddressSpace());
               // Catchup with a rewrite? Yeah, sure.
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
                   
                   if (bir->snip.ast_wrapper->accessesParam())
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
                                                                              mtHandle, 
                                                                              active);
                   
                   
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
                       const char *str_iPresult = "error";
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

int BPatch_process::oneTimeCodeCallbackDispatch(PCProcess *theProc,
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

   if (returnValue == (void *) -1L) {
       BPatch::reportError(BPatchWarning, 0,
               "No return value for rpc");
   }

   info->setReturnValue(returnValue);
   info->setCompleted(true);

   bool synchronous = info->isSynchronous();
   
   if (!synchronous) {
       // Asynchronous RPCs: if we're running, then hint to run the process
       if (bproc->isStopped())
           retval = RPC_STOP_WHEN_DONE;
       else
           retval = RPC_RUN_WHEN_DONE;

      // Do the callback specific to this OneTimeCode, if set
      BPatchOneTimeCodeCallback specificCB = info->getCallback();
      if( specificCB ) {
          (*specificCB)(bproc->threads[0], info->getUserData(), returnValue);
      }

      // Do the registered callback
      BPatchOneTimeCodeCallback cb = BPatch::bpatch->oneTimeCodeCallback;
      if( cb ) {
          (*cb)(bproc->threads[0], info->getUserData(), returnValue);
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
    if( statusIsTerminated() ) { 
        BPatch_reportError(BPatchWarning, 0,
                "oneTimeCode failing because process has already exited");
        if( err ) *err = true;
        return NULL;
    }

    if( !isStoppedInt() && synchronous ) resumeAfterCompleted_ = true;

    proccontrol_printf("%s[%d]: UI top of oneTimeCode...\n", FILE__, __LINE__);

    OneTimeCodeInfo *info = new OneTimeCodeInfo(synchronous, userData, cb,
            (thread) ? thread->getBPatchIDInt() : 0);

    activeOneTimeCodes_++;

    if( !llproc->postIRPC(expr.ast_wrapper, 
            (void *)info,
            false, // TODO this should be set according to state instead of using resumeAfterCompleted_
            (thread ? thread->llthread : NULL),
            synchronous,
            NULL, // the result will be passed to the callback 
            true) ) // deliver callbacks
    {
        BPatch_reportError(BPatchWarning, 0,
                    "failed to continue process to run oneTimeCode");
        if( err ) *err = true;
        delete info;
        return NULL;
    }


    if( !synchronous ) return NULL;

    assert( info->isCompleted() );

    void *ret = info->getReturnValue();

    proccontrol_printf("%s[%d]: RPC completed, process status %s\n",
                       FILE__, __LINE__, isStoppedInt() ? "stopped" : "running");

    if (err) *err = false;
    delete info;
    return ret;
}

void BPatch_process::oneTimeCodeCompleted(bool isSynchronous) {
    assert(activeOneTimeCodes_ > 0);
    activeOneTimeCodes_--;
    
    if (activeOneTimeCodes_ == 0 && isSynchronous) {
        proccontrol_printf("%s[%d]: oneTimeCodes outstanding reached 0, isStopped %d, completing: %s\n",
                           FILE__, __LINE__, 
                           isStoppedInt(),
                           resumeAfterCompleted_ ? "setting running" : "leaving stopped");
        if( resumeAfterCompleted_ ) {
            continueExecution();
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
   if (!isStoppedInt()) {
      BPatch_reportError(BPatchWarning, 0, 
              "Process not stopped in loadLibrary");
      return false;
   }
   
   if (!libname) {
      BPatch_reportError(BPatchWarning, 0, 
              "loadLibrary called with NULL library name");
      return false;
   }

   /**
    * Find the DYNINSTloadLibrary function
    **/
   BPatch_Vector<BPatch_function *> bpfv;
   BPatch_module* dyn_rt_lib = image->findModule("dyninstAPI_RT", true);
   if(dyn_rt_lib == NULL)
   {
      BPatch_reportError(BPatchFatal, 0, 
               "FATAL: Cannot find module for DyninstAPI Runtime Library");
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
   return true;
}

/* 
 *	this function sets a flag in process that 
 *	forces the collection of data for saveworld.
 */
void BPatch_process::enableDumpPatchedImageInt(){
    // llproc->collectSaveWorldData=true;
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

/**
 * Removes the BPatch_thread from this process' collection of
 * threads
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

#if !defined(USE_DEPRECATED_BPATCH_VECTOR)
   // STL vectors don't have item erase. We use iterators instead...
   threads.erase(std::find(threads.begin(),
                                 threads.end(),
                                 thrd));
#else
   for (unsigned i=0; i< threads.size(); i++) {
      if (threads[i] == thrd) {
         threads.erase(i);
         break;
      }
   }
#endif

   llproc->removeThread(thrd->getTid());

   // We allow users to maintain pointers to exited threads
   // If this changes, the memory can be free'd here
   // delete thrd;
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

/**
 * This function continues a stopped process, letting it execute in single step mode,
 * and printing the current instruction as it executes.
 **/

void BPatch_process::debugSuicideInt() 
{
    llproc->debugSuicide();
}

void BPatch_process::triggerThreadCreate(PCThread *thread) {
  BPatch_thread *newthr = BPatch_thread::createNewThread(this, thread);
  threads.push_back(newthr);

  BPatch::bpatch->registerThreadCreate(this, newthr);
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
         int_function *intFunc, int cb_ID, void *retVal)
{
    // find the BPatch_point corresponding to the instrumentation point
    BPatch_function *bpFunc = findOrCreateBPFunc(intFunc, NULL);
    BPatch_procedureLocation bpPointType = 
        BPatch_point::convertInstPointType_t(intPoint->getPointType());
    BPatch_point *bpPoint = findOrCreateBPPoint(bpFunc, intPoint, bpPointType);
    if (!bpPoint) { 
        return false; 
    }

    // Trigger all the callbacks matching this snippet
    for(unsigned int i = 0; i < BPatch::bpatch->stopThreadCallbacks.size(); ++i) {
        BPatchStopThreadCallback curCallback = BPatch::bpatch->stopThreadCallbacks[i];
        if( cb_ID == BPatch::bpatch->info->getStopThreadCallbackID((Address)curCallback) ) {
            (*curCallback)(bpPoint, retVal);
        }
    }

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
        int_function *intFunc, long signum, BPatch_Vector<Address> *handlers)
{
    // find the BPatch_point corresponding to the exception-raising instruction
    BPatch_function *bpFunc = findOrCreateBPFunc(intFunc, NULL);
    BPatch_procedureLocation bpPointType = 
        BPatch_point::convertInstPointType_t(intPoint->getPointType());
    BPatch_point *bpPoint = findOrCreateBPPoint(bpFunc, intPoint, bpPointType);
    if (!bpPoint) { return false; }

    // Do the callback
    InternalSignalHandlerCallback cb = BPatch::bpatch->signalHandlerCallback;
    if( cb ) {
        (*cb)(bpPoint, signum, *handlers);
        return true;
    }

    return false;
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
bool BPatch_process::triggerCodeOverwriteCB(Address fault_instr, Address viol_target)
{
    BPatch_function *func = findOrCreateBPFunc
        (llproc->findActiveFuncByAddr(fault_instr),NULL);
    assert(func);
    BPatch_point *fault_point = image->createInstPointAtAddr
        ((void*)fault_instr, NULL, func);

    // Do the callback
    InternalCodeOverwriteCallback cb = BPatch::bpatch->codeOverwriteCallback;
    if( cb ) {
        (*cb)(fault_point, viol_target);
        return true;
    }

    return false;
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
    bool retval = llproc->hideDebugger();
    // disable API calls
    BPatch_module *kern = image->findModule("kernel32.dll");
    if (kern) { // should only succeed on windows
        // CheckRemoteDebuggerPresent
        vector<BPatch_function*> funcs;
        kern->findFunction(
            "CheckRemoteDebuggerPresent",
            funcs, false, false, true);
        assert (funcs.size());
        Address entry = (Address)funcs[0]->getBaseAddr();
        unsigned char patch[3];
        patch[0] = 0x33; //xor eax,eax
        patch[1] = 0xc0;
        patch[2] = 0xc3; // retn
        llproc->writeDataSpace((void*)entry,3,&patch);
        funcs.clear();

        // OutputDebugStringA
        kern->findFunction("OutputDebugStringA",
            funcs, false, false, true);
        assert(funcs.size());
        vector<BPatch_function*> sle_funcs;
        kern->findFunction(
            "SetLastError",
            sle_funcs, false, false, true);
        if ( sle_funcs.size() ) {
            // KEVINTODO: I'm not finding this function, so I'm not safe from 
            // the anti-debug tactic
            vector<BPatch_snippet*> args;
            BPatch_constExpr lasterr(1);
            args.push_back(&lasterr);
            BPatch_funcCallExpr callSLE (*(sle_funcs[0]), args);
            vector<BPatch_point*> *exitPoints = sle_funcs[0]->findPoint(BPatch_exit);
            beginInsertionSet();
            for (unsigned i=0; i < exitPoints->size(); i++) {
                insertSnippet( callSLE, *((*exitPoints)[i]) );
            }
            finalizeInsertionSet(false);
        }
    } 
    return retval;
}

bool BPatch_process::setMemoryAccessRights
(Address start, Address size, int rights)
{
    return llproc->setMemoryAccessRights(start, size, rights);
}

unsigned char * BPatch_process::makeShadowPage(Dyninst::Address pageAddress)
{
    unsigned pagesize = llproc->getMemoryPageSize();
    unsigned char* buf = (unsigned char*) ::malloc(pagesize);
    pageAddress = (pageAddress / pagesize) * pagesize;
    llproc->readDataSpace((void*)pageAddress, pagesize, buf,true);
    return buf;
}


// return true if the analysis changed
// 
void BPatch_process::overwriteAnalysisUpdate
    ( std::map<Dyninst::Address,unsigned char*>& owPages, //input
      std::vector<Dyninst::Address>& deadBlockAddrs, //output
      std::vector<BPatch_function*>& owFuncs, //output: overwritten & modified
      bool &changedPages, bool &changedCode) //output
{
    std::map<Address,Address> owRegions;
    std::set<bblInstance *>   owBBIs;
    std::set<int_function*>   owIntFuncs;
    std::set<int_function*>   deadFuncs;

    //1.  get the overwritten blocks and regions
    llproc->getOverwrittenBlocks(owPages, owRegions, owBBIs);
    changedPages = (bool) owRegions.size();
    changedCode = (bool) owBBIs.size();

    if ( !changedCode ) {
        return;
    }

    /*2. update the analysis */

    llproc->updateActiveMultis();

    //2. update the mapped data for the overwritten ranges
    llproc->updateMappedFile(owPages,owRegions);

    //2. create stub list
    std::vector<image_basicBlock*> stubSourceBlocks;
    std::vector<Address> stubTargets;
    std::vector<EdgeTypeEnum> stubEdgeTypes;
    std::set<bblInstance*>::iterator deadIter = owBBIs.begin();
    for (;deadIter != owBBIs.end(); deadIter++) 
    {
        using namespace ParseAPI;
        
        SingleContext epred_((*deadIter)->func()->ifunc(),true,true);
        Intraproc epred(&epred_);
        image_basicBlock *curImgBlock = (*deadIter)->block()->llb();
        Block::edgelist & sourceEdges = curImgBlock->sources();
        Block::edgelist::iterator eit = sourceEdges.begin(&epred);

        Address baseAddr = (*deadIter)->firstInsnAddr() 
            - curImgBlock->firstInsnOffset();

        // being careful not to add the source block if it is also dead
        for( ; eit != sourceEdges.end(); ++eit) {
            image_basicBlock *sourceBlock = 
                dynamic_cast<image_basicBlock*>((*eit)->src());
            int_basicBlock *sourceIntB = llproc->findBasicBlockByAddr
                ( baseAddr + sourceBlock->firstInsnOffset() );
            if (owBBIs.end() == owBBIs.find(sourceIntB->origInstance()) ) {
                stubSourceBlocks.push_back(sourceBlock);
                stubTargets.push_back(curImgBlock->firstInsnOffset()+baseAddr);
                stubEdgeTypes.push_back((*eit)->type());
            }
        }
        if ((*deadIter)->block()->getHighLevelBlock()) {
            ((BPatch_basicBlock*)(*deadIter)->block()->getHighLevelBlock())
                ->setlowlevel_block(NULL);
        }
    }

    //2. identify the dead code 
    llproc->getDeadCodeFuncs(owBBIs,owIntFuncs,deadFuncs); // initializes owIntFuncs

    // remove instrumentation from affected funcs
    std::set<int_function*>::iterator fIter = owIntFuncs.begin();
    for(; fIter != owIntFuncs.end(); fIter++) {
        BPatch_function *bpfunc = findOrCreateBPFunc(*fIter,NULL);
        bpfunc->removeInstrumentation();
    }

    // if stubSrcBlocks and owBBIs overlap, replace overwritten stub  
    // blocks with ones that haven't been overwritten 
    unsigned sidx=0; 
    vector<ParseAPI::Function *> stubfuncs;
    while(sidx < stubSourceBlocks.size()) {

        // find the stub's bblInstance and function
        using namespace ParseAPI;
        stubSourceBlocks[sidx]->getFuncs(stubfuncs);
        image_func *stubfunc = (image_func*)stubfuncs[0];
        stubfuncs.clear();
        Address baseAddr = stubfunc->img()->desc().loadAddr();
        bblInstance *curBBI = llproc->findOrigByAddr
            (stubSourceBlocks[sidx]->firstInsnOffset() + baseAddr)->
             is_basicBlockInstance();
        assert(curBBI);

        // continue if the block hasn't been overwritten
        if (owBBIs.end() == owBBIs.find(curBBI)) {
            sidx++;
            continue;
        }

        // the stub's been overwritten, choose a non-overwritten source
        // block on a non-call edge as the new stub with which to replace it
        SingleContext epred_(stubfunc,true,true);
        Intraproc epred(&epred_);
        Block::edgelist inEdges = stubSourceBlocks[sidx]->sources();
        Block::edgelist::iterator eit = inEdges.begin(&epred);
        bool foundNewStub = false;

        for( ; !foundNewStub && eit != inEdges.end(); ++eit) 
        {
            if (CALL == (*eit)->type()) {
                continue;
            }
            bblInstance *srcBBI = llproc->findOrigByAddr
                (stubSourceBlocks[sidx]->firstInsnOffset() + baseAddr)->
                is_basicBlockInstance();
            if (owBBIs.end() == owBBIs.find(srcBBI)) {
                stubSourceBlocks[sidx] = 
                    dynamic_cast<image_basicBlock*>((*eit)->src());
                stubTargets[sidx] = stubSourceBlocks[sidx]->firstInsnOffset()
                    + baseAddr;
                stubEdgeTypes[sidx] = (*eit)->type();
                foundNewStub = true;
            }
        }
        if (!foundNewStub) {
            // all input blocks are dead. Search backwards through whole 
            // function for a non-call edge we haven't tried
            set<image_basicBlock*> visitedBlocks;
            std::queue<Edge*> worklist;
            visitedBlocks.insert(stubSourceBlocks[sidx]);
            Block::edgelist & inEdges = stubSourceBlocks[sidx]->sources();
            for(eit=inEdges.begin(&epred); eit != inEdges.end(); ++eit) {
                if (CALL != (*eit)->type() && 
                    visitedBlocks.end() == 
                    visitedBlocks.find
                      (static_cast<image_basicBlock*>((*eit)->src())))
                {
                    worklist.push(*eit);
                }
            }
            while (worklist.size()) {
                Edge *curEdge = worklist.front();
                worklist.pop();
                image_basicBlock *srcBlock = 
                    static_cast<image_basicBlock*>(curEdge->src());
                bblInstance *srcBBI = llproc->findOrigByAddr
                    ( srcBlock->firstInsnOffset() + baseAddr )->
                    is_basicBlockInstance();
                assert(srcBBI);
                if (owBBIs.end() == owBBIs.find(srcBBI)) {
                    // found a valid stub block
                    stubSourceBlocks[sidx] = srcBlock;
                    stubTargets[sidx] = srcBBI->firstInsnAddr();
                    stubEdgeTypes[sidx] = curEdge->type();
                    foundNewStub = true;
                    break;
                }
                else if (visitedBlocks.end() == visitedBlocks.find(srcBlock)) {
                    Block::edgelist & edges = srcBlock->sources();
                    for(eit=edges.begin(&epred); eit != edges.end(); ++eit) {
                        if (CALL != (*eit)->type() && 
                            visitedBlocks.end() == 
                            visitedBlocks.find
                              (static_cast<image_basicBlock*>((*eit)->src())))
                        {
                            worklist.push(*eit);
                        }
                    }
                }
                visitedBlocks.insert(srcBlock);
            }
            if (!foundNewStub) {
                fprintf(stderr,"WARNING: failed to find stub to replace "
                        "the overwritten stub [%lx %lx] for overwritten "
                        "block at %lx %s[%d]\n",
                        stubSourceBlocks[sidx]->firstInsnOffset(), 
                        stubSourceBlocks[sidx]->endOffset(), 
                        stubTargets[sidx],FILE__,__LINE__);
                assert(0); // the whole function should be gone in this case
            }
        }
        sidx++;
    }

    // now traverse stub lists to remove any duplicates that were introduced by 
    // overwritten stub replacement
    for(unsigned i=0; i < stubSourceBlocks.size()-1; i++) {
        for(unsigned j=i+1; j < stubSourceBlocks.size(); j++) {
            if (stubSourceBlocks[i] == stubSourceBlocks[j]) {
                if (j < stubSourceBlocks.size()-1) {
                    stubSourceBlocks[j] = 
                        stubSourceBlocks[stubSourceBlocks.size()-1];
                }
                stubSourceBlocks.pop_back();
            }
        }
    }

    // remove dead blocks belonging to affected funcs, but not dead funcs
    std::set<bblInstance*>::iterator bIter = owBBIs.begin();
    vector<ParseAPI::Block*> owImgBs;
    for (; bIter != owBBIs.end(); bIter++) {
        if (deadFuncs.end() == deadFuncs.find((*bIter)->func())) {
            (*bIter)->func()->deleteBlock((*bIter)->block());
            owImgBs.push_back((*bIter)->block()->llb());
            (*bIter)->func()->ifunc()->deleteBlocks(owImgBs,NULL);
            owImgBs.clear();
        }
    }


    for(fIter = deadFuncs.begin(); fIter != deadFuncs.end(); fIter++) {

        using namespace ParseAPI;
        Address funcAddr = (*fIter)->getAddress();

        // grab callers
        vector<image_basicBlock*> callBlocks; 
        Block::edgelist & callEdges = (*fIter)->ifunc()->entryBlock()->sources();
        Block::edgelist::iterator eit = callEdges.begin();
        for( ; eit != callEdges.end(); ++eit) {
            if (CALL == (*eit)->type()) {
                callBlocks.push_back
                    (dynamic_cast<image_basicBlock*>((*eit)->src()));
            }
        }

        //remove instrumentation and the function itself
        BPatch_function *bpfunc = findOrCreateBPFunc(*fIter,NULL);
        bpfunc->removeInstrumentation();
        (*fIter)->removeFromAll();

        // only add to deadBlockAddrs if the dead function is not reachable, 
        // i.e., it's the entry point of the a.out, and we've already
        // executed it
        if ((*fIter)->obj()->parse_img()->isAOut() && 
            0 == strncmp("main",(*fIter)->symTabName().c_str(),5)) 
        {
            const std::set< int_basicBlock* , int_basicBlock::compare >& 
                deadBlocks = (*fIter)->blocks();
            std::set<int_basicBlock* ,int_basicBlock::compare>::const_iterator
                    bIter= deadBlocks.begin();
            for (; bIter != deadBlocks.end(); bIter++) {
                deadBlockAddrs.push_back
                    ((*bIter)->origInstance()->firstInsnAddr());
            }
        }
        else {

            // the function is still reachable, reparse it
            vector<BPatch_module*> dontcare; 
            vector<Address> targVec; 
            targVec.push_back(funcAddr);
            if (getImage()->parseNewFunctions(dontcare, targVec)) {
                // add function to output vector
                bpfunc = findFunctionByAddr((void*)funcAddr);
                assert(bpfunc);
                owFuncs.push_back(bpfunc);
                // re-instate call edges to the function
                for (unsigned bidx=0; bidx < callBlocks.size(); bidx++) {
                    ParseAPI::Block *src = callBlocks[bidx];
                    ParseAPI::Block *trg = 
                        bpfunc->lowlevel_func()->ifunc()->entryBlock();
                    ParseAPI::Edge *callEdge = new Edge
                        (src, trg, ParseAPI::CALL);
                    callEdge->install();
                }
            } else {
                // the function really is dead
                const std::set< int_basicBlock* , int_basicBlock::compare >& 
                    deadBlocks = (*fIter)->blocks();
                std::set<int_basicBlock* ,int_basicBlock::compare>::const_iterator
                        bIter= deadBlocks.begin();
                for (; bIter != deadBlocks.end(); bIter++) {
                    deadBlockAddrs.push_back
                        ((*bIter)->origInstance()->firstInsnAddr());
                }
            }
        }
    }

    //2. parse new code, one overwritten function at a time
    for(fIter = owIntFuncs.begin(); fIter != owIntFuncs.end(); fIter++) {
        // find subset of stubs that correspond to this function
        int_function *curFunc = *fIter;
        std::vector<ParseAPI::Block*> cur_ssb;
        std::vector<Address> cur_st;
        std::vector<EdgeTypeEnum> cur_set;
        for(unsigned idx=0; idx < stubSourceBlocks.size(); idx++) {
            image_basicBlock *curblock = stubSourceBlocks[idx];
            vector<ParseAPI::Function *> sharefuncs;
            curblock->getFuncs(sharefuncs);
            for (vector<ParseAPI::Function *>::iterator fit= sharefuncs.begin();
                 sharefuncs.end() != fit;
                 fit++) 
            {
                if ((*fit) == curFunc->ifunc()) {
                    cur_ssb.push_back(curblock);
                    cur_st.push_back(stubTargets[idx]);
                    cur_set.push_back(stubEdgeTypes[idx]);
                }
            }
        }

        // parse new edges in the function
        if (cur_ssb.size()) {
            curFunc->parseNewEdges(cur_ssb,cur_st,cur_set);
        } else {
            cur_ssb.push_back(NULL);
            cur_st.push_back(curFunc->ifunc()->getOffset());
            cur_set.push_back(ParseAPI::NOEDGE);
		    curFunc->parseNewEdges(cur_ssb,cur_st,cur_set);
        }
        // else, this is the entry point of the function, do nothing, 
        // we'll parse this anyway through recursive traversal if there's
        // code at this address

        // add curFunc to owFuncs, and clear the function's BPatch_flowGraph
        BPatch_function *bpfunc = findOrCreateBPFunc(curFunc,NULL);
        bpfunc->removeCFG();
        owFuncs.push_back(bpfunc);
    }
}


bool BPatch_process::removeFunctionSubRange
                 ( BPatch_function &func, 
                   Dyninst::Address startAddr, 
                   Dyninst::Address endAddr, 
                   std::vector<Dyninst::Address> &deadBlockAddrs )
{
    int_basicBlock *newEntryBlock = NULL;

    bool success = func.lowlevel_func()->removeFunctionSubRange(
                                                  startAddr, 
                                                  endAddr, 
                                                  deadBlockAddrs,
                                                  newEntryBlock);
    if (success) {
        func.cfg->invalidate();
    }

    return success;
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
       mapped_module *curMod = (*bpMods)[midx]->lowlevel_mod();
       if ( ! curMod->getFuncVectorSize() 
           || curMod->obj()->isSharedLib()) {
          continue; // don't trigger analysis and don't protect shared libraries
       }
       ret = (*bpMods)[midx]->protectAnalyzedCode() && ret;
    }
    return false;
}
