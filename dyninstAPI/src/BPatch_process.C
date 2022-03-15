/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include "inst.h"
#include "instP.h"
#include "instPoint.h"
#include "function.h" // func_instance
#include "codeRange.h"
#include "dynProcess.h"
#include "dynThread.h"
#include "pcEventHandler.h"
#include "os.h"

#include "mapped_module.h"
#include "mapped_object.h"

#include "BPatch_libInfo.h"
#include "BPatch.h"
#include "BPatch_point.h"
#include "BPatch_thread.h"
#include "BPatch_function.h"
#include "BPatch_basicBlock.h"
#include "BPatch_module.h"
#include "hybridAnalysis.h"
#include "BPatch_private.h"
#include "parseAPI/h/CFG.h"
#include "ast.h"
#include "debug.h"
#include <boost/tuple/tuple.hpp>

#include "PatchMgr.h"
#include "PatchModifier.h"
#include "Command.h"
#include "Relocation/DynAddrSpace.h"
#include "Relocation/DynPointMaker.h"
#include "Relocation/DynObject.h"

#include "Point.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using PatchAPI::DynObject;
using PatchAPI::DynAddrSpace;
using PatchAPI::PatchMgr;
using PatchAPI::Patcher;

int BPatch_process::getAddressWidth(){
        return llproc->getAddressWidth();
}

/*
 * BPatch_process::getPid
 *
 * Return the process ID of the thread associated with this object.
 */
int BPatch_process::getPid()
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
 * path         Pathname of the executable to start.
 * argv         A list of pointers to character strings which are the
 *              arguments for the new process, terminated by a NULL pointer.
 * envp         A list of pointers to character strings which are the
 *              environment variables for the new process, terminated by a
 *              NULL pointer.  If NULL, the default environment will be used.
 */
BPatch_process::BPatch_process(const char *path, const char *argv[],
                               BPatch_hybridMode mode, const char **envp,
                               int stdin_fd, int stdout_fd, int stderr_fd)
   : llproc(NULL), lastSignal(-1), exitCode(-1), exitSignal(-1),
     exitedNormally(false), exitedViaSignal(false), mutationsActive(true), 
     createdViaAttach(false), detached(false), 
     terminated(false), reportedExit(false),
     hybridAnalysis_(NULL)
{
   image = NULL;
   pendingInsertions = NULL;

   std::vector<std::string> argv_vec;
   std::vector<std::string> envp_vec;
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
   llproc = PCProcess::createProcess(spath, argv_vec, mode, envp_vec,
                                     directoryName, 
                                     stdin_fd, stdout_fd, stderr_fd);
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
       BPatch::bpatch->setInstrStackFrames(true);
       hybridAnalysis_ = new HybridAnalysis(llproc->getHybridMode(),this);
   }

   // Let's try to profile memory usage
#if defined(PROFILE_MEM_USAGE)
   void *mem_usage = sbrk(0);
   fprintf(stderr, "Post BPatch_process: sbrk %p\n", mem_usage);
#endif

   startup_cerr << "BPatch_process::BPatch_process, completed." << endl;
}

/*
 * BPatch_process::BPatch_process
 *
 * Constructs a new BPatch_process and associates it with a running process.
 * Stops execution of the process.
 *
 * path         Pathname of the executable file for the process.
 * pid          Process ID of the target process.
 */
BPatch_process::BPatch_process
(const char *path, int pid, BPatch_hybridMode mode)
   : llproc(NULL), lastSignal(-1), exitCode(-1), exitSignal(-1),
     exitedNormally(false), exitedViaSignal(false), mutationsActive(true), 
     createdViaAttach(true), detached(false), 
     terminated(false), reportedExit(false),
     hybridAnalysis_(NULL)
{
   image = NULL;
   pendingInsertions = NULL;

   assert(BPatch::bpatch != NULL);

    startup_printf("%s[%d]:  creating new BPatch_image...\n", FILE__, __LINE__);
   image = new BPatch_image(this);
    startup_printf("%s[%d]:  created new BPatch_image...\n", FILE__, __LINE__);
   std::string spath = path ? std::string(path) : std::string();
    startup_printf("%s[%d]:  attaching to process %s/%d\n", FILE__, __LINE__,
          path ? path : "no_path", pid);

   llproc = PCProcess::attachProcess(spath, pid, mode);
   if (!llproc) {
      BPatch_reportError(BPatchFatal, 68, "Dyninst was unable to attach to the specified process");
      BPatch::bpatch->unRegisterProcess(pid, this);

      return;
   }

   BPatch::bpatch->registerProcess(this, pid);
   startup_printf("%s[%d]:  attached to process %s/%d\n", FILE__, __LINE__, path ? path : 
            "no_path", pid);

   // Create the initial threads
   std::vector<PCThread *> llthreads;
   llproc->getThreads(llthreads);
   for (std::vector<PCThread *>::iterator i = llthreads.begin();
           i != llthreads.end(); ++i)
   {
      BPatch_thread *thrd = new BPatch_thread(this, *i);
      threads.push_back(thrd);
   }

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
   : llproc(nProc), lastSignal(-1), exitCode(-1), exitSignal(-1),
     exitedNormally(false), exitedViaSignal(false), mutationsActive(true),
     createdViaAttach(true), detached(false),
     terminated(false),
     reportedExit(false), hybridAnalysis_(NULL)
{
   // Add this object to the list of threads
   assert(BPatch::bpatch != NULL);
   image = NULL;
   pendingInsertions = NULL;

   BPatch::bpatch->registerProcess(this);

   // Create the initial threads
   std::vector<PCThread *> llthreads;
   llproc->getThreads(llthreads);
   for (std::vector<PCThread *>::iterator i = llthreads.begin();
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
BPatch_process::~BPatch_process()
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
           if (llproc->isAttached()) {
               terminateExecution();
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
bool BPatch_process::stopExecution() 
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
bool BPatch_process::continueExecution() 
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
bool BPatch_process::terminateExecution() 
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
bool BPatch_process::isStopped()
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
int BPatch_process::stopSignal()
{
    if (!isStopped()) {
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
bool BPatch_process::isTerminated()
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
BPatch_exitType BPatch_process::terminationStatus() {
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
int BPatch_process::getExitCode()
{
   return exitCode;
}

/*
 * BPatch_process::getExitSignal
 *
 * Returns signal number that caused application to exit.
 *
 */
int BPatch_process::getExitSignal()
{
   return lastSignal;
}

bool BPatch_process::wasRunningWhenAttached()
{
  if (!llproc) return false;
  return llproc->wasRunningWhenAttached();
}

/*
 * BPatch_process::detach
 *
 * Detach from the thread represented by this object.
 *
 * cont         True if the thread should be continued as the result of the
 *              detach, false if it should not.
 */
bool BPatch_process::detach(bool cont)
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
bool BPatch_process::isDetached()
{
   return detached;
}

/*
 * BPatch_process::dumpCore
 *
 * Causes the process to dump its state to a file, and optionally to terminate.
 * Returns true upon success, and false upon failure.
 *
 * file         The name of the file to which the state should be written.
 * terminate    Indicates whether or not the thread should be terminated after
 *              dumping core.  True indicates that it should, false that is
 *              should not.
 */
bool BPatch_process::dumpCore(const char *file, bool terminate)
{
   bool was_stopped = isStopped();

   stopExecution();

   bool ret = llproc->dumpCore(file);
   if (ret && terminate) {
      terminateExecution();
   } else if (!was_stopped) {
      continueExecution();
   }

   return ret;
}

/*
 * BPatch_process::dumpImage
 *
 * Writes the contents of memory into a file.
 * Returns true upon success, and false upon failure.
 *
 * file         The name of the file to which the image should be written.
 */
bool BPatch_process::dumpImage(const char *file)
{
#if defined(os_windows) 
   return false;
#else
   bool was_stopped = isStopped();

   stopExecution();

   bool ret = llproc->dumpImage(file);
   if (!was_stopped) continueExecution();

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
BPatch_variableExpr *BPatch_process::getInheritedVariable(
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

BPatchSnippetHandle *BPatch_process::getInheritedSnippet(BPatchSnippetHandle &parentSnippet)
{
    // a BPatchSnippetHandle has an miniTramp for each point that
    // the instrumentation is inserted at
   const BPatch_Vector<Dyninst::PatchAPI::Instance::Ptr> &instances = parentSnippet.instances_;

   BPatchSnippetHandle *childSnippet = new BPatchSnippetHandle(this);
   for(unsigned i=0; i<instances.size(); i++) {
      Dyninst::PatchAPI::Instance::Ptr child = getChildInstance(instances[0], llproc);
      if (child) childSnippet->addInstance(child);
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

void BPatch_process::beginInsertionSet()
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
bool BPatch_process::finalizeInsertionSet(bool, bool *)
{

   if (statusIsTerminated()) return false;


  // Can't insert code when mutations are not active.
  bool shouldContinue = false;
  if (!mutationsActive) {
    return false;
  }
  
  if ( ! isStopped() ) {
    shouldContinue = true;
    stopExecution();
  }

  /* PatchAPI stuffs */
  bool ret = AddressSpace::patch(llproc);
  /* End of PatchAPI stuffs */

  llproc->trapMapping.flush();

  if (shouldContinue)
    continueExecution();

  if (pendingInsertions) {
    delete pendingInsertions;
    pendingInsertions = NULL;
  }

  return ret;
}


bool BPatch_process::finalizeInsertionSetWithCatchup(bool, bool *,
                                                        BPatch_Vector<BPatch_catchupInfo> &)
{
   return false;
}

/*
 * BPatch_process::oneTimeCode
 *
 * execute argument <expr> once.
 *
 */
void *BPatch_process::oneTimeCode(const BPatch_snippet &expr, bool *err)
{
    if( !isStopped() ) {
        BPatch_reportError(BPatchWarning, 0,
                "oneTimeCode failing because process is not stopped");
        if( err ) *err = true;
        return NULL;
    }

    return oneTimeCodeInternal(expr, NULL, NULL, NULL, true, err, true);
}

/*
 * BPatch_process::oneTimeCodeCallbackDispatch
 *
 * theProc	The process in which the RPC completed.
 * userData	This is a value that can be set when we invoke an inferior RPC
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

    OneTimeCodeInfo *info = (OneTimeCodeInfo *)userData;

    BPatch_process *bproc =
    BPatch::bpatch->getProcessByPid(theProc->getPid());

    assert(bproc != NULL);

    assert(info && !info->isCompleted());

    info->setReturnValue(returnValue);
    info->setCompleted(true);

    if (!info->isSynchronous()) {
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

        // This is the case if the user requested a stop in a callback
        if (bproc->isStopped()) retval = RPC_STOP_WHEN_DONE;
        else retval = RPC_RUN_WHEN_DONE;

        delete info;
    }

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
 * expr         The snippet to evaluate.
 * userData     This value is given to the callback function along with the
 *              return value for the snippet.  Can be used by the caller to
 *              store per-oneTimeCode information.
 * synchronous  True means wait until the snippet has executed, false means
 *              return immediately.
 */
void *BPatch_process::oneTimeCodeInternal(const BPatch_snippet &expr,
                                          BPatch_thread *thread,
                                          void *userData,
                                          BPatchOneTimeCodeCallback cb,
                                          bool synchronous,
                                          bool *err,
                                          bool userRPC)
{
    if( statusIsTerminated() ) { 
        BPatch_reportError(BPatchWarning, 0,
                "oneTimeCode failing because process has already exited");
        if( err ) *err = true;
        return NULL;
    }

    proccontrol_printf("%s[%d]: UI top of oneTimeCode...\n", FILE__, __LINE__);

    OneTimeCodeInfo *info = new OneTimeCodeInfo(synchronous, userData, cb,
            (thread) ? thread->getBPatchID() : 0);

    if( !llproc->postIRPC(expr.ast_wrapper, 
            (void *)info,
            !isStopped(), 
            (thread ? thread->llthread : NULL),
            synchronous,
            NULL, // the result will be passed to the callback 
            userRPC) )
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
                       FILE__, __LINE__, isStopped() ? "stopped" : "running");

    if (err) *err = false;
    delete info;
    return ret;
}

//  BPatch_process::oneTimeCodeAsync
//
//  Have the specified code be executed by the mutatee once.  Don't wait
//  until done.
bool BPatch_process::oneTimeCodeAsync(const BPatch_snippet &expr,
                                         void *userData, BPatchOneTimeCodeCallback cb)
{
   bool err = false;
   oneTimeCodeInternal(expr, NULL, userData,  cb, false, &err, true);

   if( err ) return false;
   return true;
}

/*
 * BPatch_process::loadLibrary
 *
 * Load a dynamically linked library into the address space of the mutatee.
 *
 * libname      The name of the library to load.
 */
BPatch_object *BPatch_process::loadLibrary(const char *libname, bool)
{
   if (!libname) {
      fprintf(stderr, "[%s:%d] - loadLibrary called with NULL library name\n",
              __FILE__, __LINE__);
      return NULL;
   }

   bool wasStopped = isStopped();
   if( !wasStopped ) {
       if (!stopExecution()) {
          BPatch_reportError(BPatchWarning, 0, 
                  "Failed to stop process for loadLibrary");
          return NULL;
       }
   }

   BPatch_object *object = NULL;
   do {

      /**
       * Find the DYNINSTloadLibrary function
       **/
      BPatch_Vector<BPatch_function *> bpfv;
      image->findFunction("DYNINSTloadLibrary", bpfv);
      if (!bpfv.size()) {
         cerr << __FILE__ << ":" << __LINE__ << ": FATAL:  Cannot find Internal"
              << "Function DYNINSTloadLibrary" << endl;
         break;
      }
      if (bpfv.size() > 1) {
         std::string msg = std::string("Found ") + utos(bpfv.size()) +
            std::string("functions called DYNINSTloadLibrary -- not fatal but weird");
         BPatch_reportError(BPatchSerious, 100, msg.c_str());
      }
      BPatch_function *dlopen_func = bpfv[0];
      if (dlopen_func == NULL)
        break;

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
         break;
      }
      /* Find the new mapped_object, map it to a BPatch_module, and return it */

      mapped_object* plib = llproc->findObject(libname);
      if (!plib) {
        std::string wildcard(libname);
        wildcard += "*";
        plib = llproc->findObject(wildcard, true);
      }
      if (!plib) {
         // Best effort; take the latest added mapped_object
         plib = llproc->mappedObjects().back();
      }

      dynamic_cast<DynAddrSpace*>(llproc->mgr()->as())->loadLibrary(plib);
      object = getImage()->findOrCreateObject(plib);

   } while (0);

   if( !wasStopped ) {
       if( !continueExecution() ) {
           BPatch_reportError(BPatchWarning, 0,
                   "Failed to continue process for loadLibrary");
       }
   }

   return object;
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

void BPatch_process::getThreads(BPatch_Vector<BPatch_thread *> &thrds)
{
   for (unsigned i=0; i<threads.size(); i++)
      thrds.push_back(threads[i]);
}

bool BPatch_process::isMultithreaded()
{
   return (threads.size() > 1);
}

bool BPatch_process::isMultithreadCapable()
{
   if (!llproc) return false;
   return llproc->multithread_capable();
}

BPatch_thread *BPatch_process::getThread(dynthread_t tid)
{
   for (unsigned i=0; i<threads.size(); i++)
      if (threads[i]->getTid() == tid)
         return threads[i];
   return NULL;
}

BPatch_thread *BPatch_process::getThreadByIndex(unsigned index)
{
   for (unsigned i=0; i<threads.size(); i++)
      if (threads[i]->getBPatchID() == index)
         return threads[i];
   return NULL;
}

processType BPatch_process::getType()
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

   threads.erase(std::find(threads.begin(),
                                 threads.end(),
                                 thrd));

   llproc->removeThread(thrd->getTid());

   // We allow users to maintain pointers to exited threads
   // If this changes, the memory can be free'd here
   // delete thrd;
}

/**
 * This function continues a stopped process, letting it execute in single step mode,
 * and printing the current instruction as it executes.
 **/

void BPatch_process::debugSuicide()
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
 * snippets allow a different callback to be triggered for each
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
        func_instance *intFunc, long signum, BPatch_Vector<Address> *handlers)
{
    // find the BPatch_point corresponding to the exception-raising instruction
    BPatch_function *bpFunc = findOrCreateBPFunc(intFunc, NULL);
    BPatch_procedureLocation bpPointType =
        BPatch_point::convertInstPointType_t(intPoint->type());
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
bool BPatch_process::triggerCodeOverwriteCB(instPoint *faultPoint,
                                            Address faultTarget)
{
    BPatch_function *bpFunc = findOrCreateBPFunc
        (faultPoint->func(),NULL);
    assert(bpFunc);
    BPatch_point *bpPoint = findOrCreateBPPoint(
        bpFunc,
        faultPoint,
        BPatch_point::convertInstPointType_t(faultPoint->type()));

    // Do the callback
    InternalCodeOverwriteCallback cb = BPatch::bpatch->codeOverwriteCallback;
    if( cb ) {
        (*cb)(bpPoint, faultTarget);
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
bool BPatch_process::hideDebugger()
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
        if (!funcs.empty()) {
			BPatch_module *rtlib = this->image->findOrCreateModule(
				(*llproc->runtime_lib.begin())->getModules().front());
			vector<BPatch_function*> repfuncs;
			rtlib->findFunction("DYNINST_FakeTickCount", repfuncs, false);
			assert(!repfuncs.empty());
			replaceFunction(*funcs[0],*repfuncs[0]);
			disabledFuncs.push_back(pair<BPatch_function*,BPatch_function*>(
									funcs[0],repfuncs[0]));
		}
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

bool BPatch_process::setMemoryAccessRights(Address start, size_t size, Dyninst::ProcControlAPI::Process::mem_perm rights) {
    bool wasStopped = isStopped();
    if( !wasStopped ) {
        if (!stopExecution()) {
            BPatch_reportError(BPatchWarning, 0,
                               "Failed to stop process for setMemoryAccessRights");
            return false;
        }
    }

    int result = llproc->setMemoryAccessRights(start, size, rights);

    if( !wasStopped ) {
        if( !continueExecution() ) {
            BPatch_reportError(BPatchWarning, 0,
                    "Failed to continue process for setMemoryAccessRights");
            return false;
        }
    }

    return (result != -1);
}

unsigned char * BPatch_process::makeShadowPage(Dyninst::Address pageAddr)
{
    unsigned pagesize = llproc->getMemoryPageSize();
    pageAddr = (pageAddr / pagesize) * pagesize;

    Address shadowAddr = pageAddr;

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

    if (0 == ebytes) {
        mal_printf("funct at %lx hasWeirdEntryBytes, 0x0000\n", func->addr());
        return true;
    }
    return false;
}

void BPatch_process::overwriteAnalysisUpdate
    ( std::map<Dyninst::Address,unsigned char*>& owPages, //input
      std::vector<std::pair<Dyninst::Address,int> >& deadBlocks, //output
      std::vector<BPatch_function*>& owFuncs, //output: overwritten & modified
      std::set<BPatch_function *> &monitorFuncs, // output: those that call overwritten or modified funcs
      bool &changedPages, bool &changedCode) //output
{
    //1.  get the overwritten blocks and regions
    std::list<std::pair<Address,Address> > owRegions;
    std::list<block_instance *> owBBIs;
    llproc->getOverwrittenBlocks(owPages, owRegions, owBBIs);
    changedPages = ! owRegions.empty();
    changedCode = ! owBBIs.empty();

    if ( !changedCode ) {
        // update the mapped data for the overwritten ranges
        llproc->updateCodeBytes(owRegions);
        return;
    }

    /*2. remove dead code from the analysis */

    std::set<block_instance*> delBlocks; 
    std::map<func_instance*,set<block_instance*> > elimMap; 
    std::list<func_instance*> deadFuncs; 
    std::map<func_instance*,block_instance*> newFuncEntries; 

    // remove instrumentation from affected funcs
    beginInsertionSet();
    for(std::map<func_instance*,set<block_instance*> >::iterator fIter = elimMap.begin();
        fIter != elimMap.end();
        fIter++)
    {
        BPatch_function *bpfunc = findOrCreateBPFunc(fIter->first,NULL);
        //hybridAnalysis_->removeInstrumentation(bpfunc,false,false);
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

    finalizeInsertionSet(false);

    // update the mapped data for the overwritten ranges
    llproc->updateCodeBytes(owRegions);

    // create stub edge set which is: all edges such that:
    //     e->trg() in owBBIs and
    //     while e->src() in delBlocks choose stub from among e->src()->sources()
    std::map<func_instance*,vector<edgeStub> > stubs =
       llproc->getStubs(owBBIs,delBlocks,deadFuncs);

    // get stubs for dead funcs
    map<Address,vector<block_instance*> > deadFuncCallers;
    for(std::list<func_instance*>::iterator fit = deadFuncs.begin();
        fit != deadFuncs.end();
        fit++)
    {
       if ((*fit)->getLiveCallerBlocks(delBlocks, deadFuncs, deadFuncCallers) &&
           ((*fit)->ifunc()->hasWeirdInsns() || hasWeirdEntryBytes(*fit))) 
       {
          // don't reparse the function if it's likely a garbage function, 
          // but mark the caller point as unresolved so we'll re-parse
          // if we actually call into the garbage func
          Address funcAddr = (*fit)->addr();
          vector<block_instance*>::iterator sit = deadFuncCallers[funcAddr].begin();
          for ( ; sit != deadFuncCallers[funcAddr].end(); sit++) {
             (*sit)->llb()->setUnresolvedCF(true);
             vector<func_instance*> cfuncs;
             (*sit)->getFuncs(std::back_inserter(cfuncs));
             for (unsigned i=0; i < cfuncs.size(); i++) {
                cfuncs[i]->ifunc()->setPrevBlocksUnresolvedCF(0); // force rebuild of unresolved list
                cfuncs[i]->preCallPoint(*sit, true); // create point
                monitorFuncs.insert(findOrCreateBPFunc(cfuncs[i], NULL));
             }
          }
          deadFuncCallers.erase(deadFuncCallers.find(funcAddr));
       }
    }

    // set new entry points for functions with NewF blocks, the active blocks
    // in newFuncEntries serve as suggested entry points, but will not be 
    // chosen if there are other blocks in the function with no incoming edges
    for (map<func_instance*,block_instance*>::iterator nit = newFuncEntries.begin();
         nit != newFuncEntries.end();
         nit++)
    {
        nit->first->setNewEntry(nit->second,delBlocks);
    }
    
    // delete delBlocks and set new function entry points, if necessary
    vector<PatchBlock*> delVector;
    for(set<block_instance*>::reverse_iterator bit = delBlocks.rbegin(); 
        bit != delBlocks.rend();
        bit++)
    {
        mal_printf("Deleting block [%lx %lx)\n", (*bit)->start(),(*bit)->end());
        deadBlocks.push_back(pair<Address,int>((*bit)->start(),(*bit)->size()));
        delVector.push_back(*bit);
    }
    if (!delVector.empty() && ! PatchAPI::PatchModifier::remove(delVector,true)) {
        assert(0);
    }
    mal_printf("Done deleting blocks\n"); 
    // delete completely dead functions // 

    // save deadFunc block addresses in deadBlocks
    for(std::list<func_instance*>::iterator fit = deadFuncs.begin();
        fit != deadFuncs.end();
        fit++)
    {
        const PatchFunction::Blockset& deadBs = (*fit)->blocks();
        PatchFunction::Blockset::const_iterator bIter= deadBs.begin();
        for (; bIter != deadBs.end(); bIter++) {
            deadBlocks.push_back(pair<Address,int>((*bIter)->start(),
                                                   (*bIter)->size()));
        }
    }

    // now actually delete the dead functions and redirect call edges to sink 
    // block (if there already is an edge to the sink block, redirect 
    // doesn't duplicate the edge)
    for(std::list<func_instance*>::iterator fit = deadFuncs.begin();
        fit != deadFuncs.end();
        fit++)
    {
        const PatchBlock::edgelist & srcs = (*fit)->entry()->sources();
        vector<PatchEdge*> srcVec; // can't operate off edgelist, since we'll be deleting edges
        srcVec.insert(srcVec.end(), srcs.begin(), srcs.end());
        for (vector<PatchEdge*>::const_iterator sit = srcVec.begin();
             sit != srcVec.end();
             sit++)
        {
           if ((*sit)->type() == ParseAPI::CALL) {
              PatchAPI::PatchModifier::redirect(*sit, NULL);
           }
        }

        if (false == PatchAPI::PatchModifier::remove(*fit)) {
            assert(0);
        }
    }
    mal_printf("Done deleting functions\n");


    // set up data structures for re-parsing dead functions from stubs
    map<mapped_object*,vector<edgeStub> > dfstubs;
    for (map<Address, vector<block_instance*> >::iterator sit = deadFuncCallers.begin();
         sit != deadFuncCallers.end();
         sit++)
    {
       for (vector<block_instance*>::iterator bit = sit->second.begin();
            bit != sit->second.end();
            bit++) 
       {
          // re-instate call edges to the function
          dfstubs[(*bit)->obj()].push_back(edgeStub(*bit,
                                                    sit->first,
                                                    ParseAPI::CALL));
       }
   }

    // re-parse the functions
    for (map<mapped_object*,vector<edgeStub> >::iterator mit= dfstubs.begin();
         mit != dfstubs.end(); mit++)
    {
        mit->first->setCodeBytesUpdated(false);
        if (mit->first->parseNewEdges(mit->second)) {
            // add functions to output vector
            for (unsigned fidx=0; fidx < mit->second.size(); fidx++) {
               BPatch_function *bpfunc = findFunctionByEntry(mit->second[fidx].trg);
               if (bpfunc) {
                  owFuncs.push_back(bpfunc);
               } else {
                  // couldn't reparse
                  mal_printf("WARNING: Couldn't re-parse an overwritten "
                             "function at %lx %s[%d]\n", mit->second[fidx].trg, 
                             FILE__,__LINE__);
               }
            }
        } else {
            mal_printf("ERROR: Couldn't re-parse overwritten "
                       "functions %s[%d]\n", FILE__,__LINE__);
        }
    }

    //3. parse new code, one overwritten function at a time
    for(std::map<func_instance*,set<block_instance*> >::iterator
        fit = elimMap.begin();
        fit != elimMap.end();
        fit++)
    {
        // parse new edges in the function
       if (!stubs[fit->first].empty()) {
          fit->first->obj()->parseNewEdges(stubs[fit->first]);
       } else {
          // stubs may have been shared with another function and parsed in 
          // the other function's context.  
          mal_printf("WARNING: didn't have any stub edges for overwritten "
                     "func %lx\n", fit->first->addr());
          //KEVINTEST: we used to wind up here with deleted functions, hopefully we do not anymore
       }
        // add curFunc to owFuncs, and clear the function's BPatch_flowGraph
        BPatch_function *bpfunc = findOrCreateBPFunc(fit->first,NULL);
        bpfunc->removeCFG();
        owFuncs.push_back(bpfunc);
    }

    // do a consistency check
    for(std::map<func_instance*,set<block_instance*> >::iterator 
        fit = elimMap.begin();
        fit != elimMap.end();
        fit++) 
    {
        assert(fit->first->consistency());
    }
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
    return ret;
}


bool BPatch_process::supportsUserThreadEvents() {
    if (llproc == NULL) return false;
    return llproc->supportsUserThreadEvents(); 
}
