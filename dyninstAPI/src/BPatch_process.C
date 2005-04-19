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
#include "signalhandler.h"
#include "inst.h"
#include "instP.h"
#include "instPoint.h"
#include "function.h" // int_function
#include "codeRange.h"
#include "func-reloc.h"

#include "BPatch_libInfo.h"
#include "BPatch_asyncEventHandler.h"
#include "BPatch.h"
#include "BPatch_thread.h"
#include "LineInformation.h"

extern BPatch_eventMailbox *event_mailbox;


/*
 * class OneTimeCodeInfo
 *
 * This is used by the oneTimeCode (inferiorRPC) mechanism to keep per-RPC
 * information.
 */
class OneTimeCodeInfo {
   bool synchronous;
   bool completed;
   void *userData;
   void *returnValue;
public:
   OneTimeCodeInfo(bool _synchronous, void *_userData) :
      synchronous(_synchronous), completed(false), userData(_userData) { };

   bool isSynchronous() { return synchronous; }

   bool isCompleted() const { return completed; }
   void setCompleted(bool _completed) { completed = _completed; }

   void *getUserData() { return userData; }

   void setReturnValue(void *_returnValue) { returnValue = _returnValue; }
   void *getReturnValue() { return returnValue; }
};


/*
 * BPatch_process::getImage
 *
 * Return the BPatch_image this object.
 */
BPatch_image *BPatch_process::getImageInt()
{
   return image;
}

/*
 * BPatch_process::getPid
 *
 * Return the process ID of the thread associated with this object.
 */
int BPatch_process::getPidInt()
{
   return llproc->getPid();
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
BPatch_process::BPatch_process(const char *path, char *argv[], char *envp[],
                               int stdin_fd, int stdout_fd, int stderr_fd)
   : llproc(NULL), image(NULL), lastSignal(-1), exitCode(-1), 
     exitedNormally(false), exitedViaSignal(false), mutationsActive(true), 
     createdViaAttach(false), detached(false), unreportedStop(false), 
     unreportedTermination(false)
{
   func_map = new BPatch_funcMap();
   instp_map = new BPatch_instpMap();

   pdvector<pdstring> argv_vec;
   pdvector<pdstring> envp_vec;
   // Contruct a vector out of the contents of argv
   for(int i = 0; argv[i] != NULL; i++)
      argv_vec.push_back(argv[i]);
    
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
   const char *dotslash = "./";
   if (NULL == strchr(path, '/'))
      directoryName = dotslash;
#endif

   llproc = ll_createProcess(path, &argv_vec, (envp ? &envp_vec : NULL), 
                           directoryName, stdin_fd, stdout_fd, stderr_fd);
   if (llproc == NULL) { 
      BPatch::bpatch->reportError(BPatchFatal, 68, 
           "Dyninst was unable to create the specified process");
      return;
   }
   
   llproc->newFunctionCallback(createBPFuncCB);
   llproc->newInstPointCallback(createBPPointCB);

   // Add this object to the list of processes
   assert(BPatch::bpatch != NULL);
   BPatch::bpatch->registerProcess(this);

   // Create an initial thread
   threads.push_back(new BPatch_thread(this));

   image = new BPatch_image(this);

   while (!llproc->isBootstrappedYet() && !statusIsTerminated())
      BPatch::bpatch->getThreadEvent(false);
}

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
     unreportedTermination(false)
{
   func_map = new BPatch_funcMap();
   instp_map = new BPatch_instpMap();

   // Create an initial thread
   threads.push_back(new BPatch_thread(this));

   // Add this object to the list of threads
   assert(BPatch::bpatch != NULL);
   BPatch::bpatch->registerProcess(this, pid);

   image = new BPatch_image(this);

   llproc = ll_attachProcess(path, pid);
   if (!llproc) {
      BPatch::bpatch->unRegisterProcess(pid);
      BPatch::bpatch->reportError(BPatchFatal, 68, 
             "Dyninst was unable to attach to the specified process");
      return;
   }

   llproc->newFunctionCallback(createBPFuncCB);
   llproc->newInstPointCallback(createBPPointCB);

   // Just to be sure, pause the process....
   llproc->pause();

   while (!llproc->isBootstrappedYet() && !statusIsTerminated()) {
      BPatch::bpatch->getThreadEventOnly(false);
      llproc->getRpcMgr()->launchRPCs(false);
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
BPatch_process::BPatch_process(int /*pid*/, process *nProc)
   : llproc(nProc), image(NULL), lastSignal(-1), exitCode(-1),
     exitedNormally(false), exitedViaSignal(false), mutationsActive(true), 
     createdViaAttach(true), detached(false),
     unreportedStop(false), unreportedTermination(false)
{
   // Add this object to the list of threads
   assert(BPatch::bpatch != NULL);
   BPatch::bpatch->registerProcess(this);

   func_map = new BPatch_funcMap();
   instp_map = new BPatch_instpMap();

   // Create an initial thread
   threads.push_back(new BPatch_thread(this));

   llproc->newFunctionCallback(createBPFuncCB);
   llproc->newInstPointCallback(createBPPointCB);

   image = new BPatch_image(this);
}

/*
 * BPatch_process::~BPatch_process
 *
 * Destructor for BPatch_process.  Detaches from the running thread.
 */
void BPatch_process::BPatch_process_dtor()
{
   if (!detached &&
       !BPatch::bpatch->eventHandler->detachFromProcess(this)) {
      bperr("%s[%d]:  trouble decoupling async event handler for process %d\n",
            __FILE__, __LINE__, getPid());
   }

   if (image)
      delete image;
   image = NULL;

   if (func_map)
      delete func_map;
   if (instp_map)
      delete instp_map;

   if (!llproc) { return; }

   /**
    * If we attached to the process, then we detach and leave it be,
    * otherwise we'll terminate it
    **/
   if (createdViaAttach)
      llproc->detachProcess(true);
   else 
      terminateExecution();

   BPatch::bpatch->unRegisterProcess(getPid());   
   delete llproc;
   assert(BPatch::bpatch != NULL);
}


/*
 * BPatch_process::stopExecution
 *
 * Puts the thread into the stopped state.
 */
bool BPatch_process::stopExecutionInt()
{
   return llproc->pause();
}

/*
 * BPatch_process::continueExecution
 *
 * Puts the thread into the running state.
 */
bool BPatch_process::continueExecutionInt()
{
   if (llproc->continueProc()) {
      setUnreportedStop(false);
      return true;
   }
   return false;
}


/*
 * BPatch_process::terminateExecution
 *
 * Kill the thread.
 */
bool BPatch_process::terminateExecutionInt()
{
   if (!llproc || !llproc->terminateProc())
      return false;
   while (!isTerminated());
   
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
   assert(BPatch::bpatch);
   if (statusIsStopped()) {
      setUnreportedStop(false);
      return true;
   }
   
   BPatch::bpatch->getThreadEvent(false);
   if (statusIsStopped()) {
      setUnreportedStop(false);
      return true;
   } else
      return false;
}

/*
 * BPatch_process::stopSignal
 *
 * Returns the number of the signal which caused the thread to stop.
 */
int BPatch_process::stopSignalInt()
{
   if (llproc->status() != neonatal && llproc->status() != stopped)
      return -1;
   else
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
   return llproc->status() == exited;
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
   // First see if we've already terminated to avoid 
   // checking process status too often.
   if (statusIsTerminated()) {
      llproc->terminateProc();
      setUnreportedTermination(false);
      return true;
   }

   // Check for status changes.
   assert(BPatch::bpatch);
   BPatch::bpatch->getThreadEvent(false);
   
   // Check again
   if (statusIsTerminated()) {
      llproc->terminateProc();
      setUnreportedTermination(false);
      return true;
   } else {
      return false;
   }
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
   if (!BPatch::bpatch->eventHandler->detachFromProcess(this)) {
      bperr("%s[%d]:  trouble decoupling async event handler for process %d\n",
            __FILE__, __LINE__, getPid());
   }
   
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
      terminateExecution();
   } else if (was_stopped) {
    	unreportedStop = had_unreportedStop;
   } else {
      continueExecution();
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
      continueExecution();

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

   stopExecution();

   bool ret = llproc->dumpImage(file);
   if (was_stopped) 
      unreportedStop = had_unreportedStop;
   else 
      continueExecution();

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
   void *mem = (void *)llproc->inferiorMalloc(type.getSize(), dataHeap);
   if (!mem) return NULL;
   return new BPatch_variableExpr(this, mem, Null_Register, &type);
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
                                  parentVar.getType());
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
BPatchSnippetHandle *BPatch_process::getInheritedSnippetInt(
                                                            BPatchSnippetHandle &parentSnippet)
{
   // a BPatchSnippetHandle has an miniTrampHandle for each point that
   // the instrumentation is inserted at
   BPatch_Vector<miniTrampHandle *> parent_mtHandles;
   parentSnippet.getMiniTrampHandles(&parent_mtHandles);

   BPatchSnippetHandle *childSnippet = new BPatchSnippetHandle(llproc);
   for(unsigned i=0; i<parent_mtHandles.size(); i++) {
      miniTrampHandle *childMT = NULL;
      if(!getInheritedMiniTramp(parent_mtHandles[i], childMT, llproc))
         return NULL;
      childSnippet->add(childMT);
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
BPatchSnippetHandle *BPatch_process::insertSnippetInt(
              const BPatch_snippet &expr, BPatch_point &point, 
              BPatch_snippetOrder order)
{
   BPatch_callWhen when;
   if (point.getPointType() == BPatch_exit)
      when = BPatch_callAfter;
   else
      when = BPatch_callBefore;

   return insertSnippet(expr, point, when, order);
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
BPatchSnippetHandle *BPatch_process::insertSnippetWhen(
               const BPatch_snippet &expr,
               BPatch_point &point,
               BPatch_callWhen when,
               BPatch_snippetOrder order)
{
   assert(BPatch::bpatch != NULL);

   // Can't insert code when mutations are not active.
   if (!mutationsActive)
      return NULL;
   
   // code is null (possibly an empy sequence or earlier error)
   if (!expr.ast) return NULL;
   
   callWhen _when;
   callOrder _order;
   
   if (when == BPatch_callBefore)
      _when = callPreInsn;
   else if (when == BPatch_callAfter)
      _when = callPostInsn;
   else
      return NULL;
   
   if (order == BPatch_firstSnippet)
      _order = orderFirstAtPoint;
   else if (order == BPatch_lastSnippet)
      _order = orderLastAtPoint;
   else
      return NULL;

   //
   // Check for valid combinations of BPatch_procedureLocation & call*
   // 	Right now we don't allow
   //		BPatch_callBefore + BPatch_exit
   //		BPatch_callAfter + BPatch_entry
   //
   //	These combinations are intended to be used to mark the point that
   //      is the last, first valid point where the local variables are
   //      valid.  This is different than the first/last instruction of
   //      a subroutine which is what the other combinations of BPatch_entry
   //	    and BPatch_exit refer to.
   //
   if (when == BPatch_callBefore && point.getPointType() == BPatch_exit) {
      BPatch_reportError(BPatchSerious, 113,
                         "BPatch_callBefore at BPatch_exit not supported yet");
      return NULL;
   }
   if (when == BPatch_callAfter && point.getPointType() == BPatch_entry) {
      BPatch_reportError(BPatchSerious, 113,
                         "BPatch_callAfter at BPatch_entry not supported yet");
      return NULL;
   }

    if ((point.getPointType() == BPatch_exit)) {
       //  XXX - Hack! 
       //  The semantics of pre/post insn at exit are setup for the new
       //  defintion of using this to control before/after stack creation,
       //  but the lower levels of dyninst don't know about this yet.
       _when = callPreInsn;
    }

   if (BPatch::bpatch->isTypeChecked()) {
      assert(expr.ast);
      if (expr.ast->checkType() == BPatch::bpatch->type_Error) {
         return NULL;
      }
   }
   
   BPatchSnippetHandle *handle = new BPatchSnippetHandle(llproc);
   AstNode *ast = (AstNode *)expr.ast;
   instPoint *&ip = (instPoint*&) point.point;
   
   if (point.proc != this) {
      BPatch_reportError(BPatchSerious, 113, "The given instPoint isn't from the same process as the invoked BPatch_process");
      return NULL;
   }

   bool use_recursive_tramps = BPatch::bpatch->isTrampRecursive();
#if defined(os_aix)
   BPatch_function *tmpFunc = (BPatch_function *) point.getFunction();
   char tmpFuncName[1024];
   
	if(llproc->collectSaveWorldData){
		tmpFunc->getName(tmpFuncName, 1024);
      
		if( !strncmp(tmpFuncName,"main",4)){
         use_recursive_tramps = true;
      }
	}
#endif
#if defined(os_irix)
   if (point.getPointType() == BPatch_arbitrary) 
      use_recursive_tramps = true;
#endif


   miniTrampHandle *mtHandle;
   loadMiniTramp_result result;
   result = addInstFunc(llproc, mtHandle, ip, ast, _when, _order, false, 
                        use_recursive_tramps, true);
   if(result != success_res) 
   {
      delete handle;
      return NULL;
   }
   
   handle->add(mtHandle);
   return handle;
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
BPatchSnippetHandle *BPatch_process::insertSnippetAtPointsWhen(
                const BPatch_snippet &expr,
                const BPatch_Vector<BPatch_point *> &points,
                BPatch_callWhen when,
                BPatch_snippetOrder order)
{
   BPatchSnippetHandle *handle = new BPatchSnippetHandle(llproc);
   
   for (unsigned int i = 0; i < points.size(); i++) {
      BPatch_point *point = points[i];
      
      BPatchSnippetHandle *ret = insertSnippet(expr, *point, when, order);
      if (ret) {
         for (unsigned int j=0; j < ret->mtHandles.size(); j++) {
            handle->add(ret->mtHandles[j]);
         }
         delete ret;
      } else {
         delete handle;
         return NULL;
      }
   }
   
   return handle;
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
   BPatchSnippetHandle *handle = new BPatchSnippetHandle(llproc);
   
   for (unsigned int i = 0; i < points.size(); i++) {
      BPatch_point *point = points[i];
      BPatch_callWhen when;

      if (point->getPointType() == BPatch_exit) {
         when = BPatch_callAfter;
      } else {
         when = BPatch_callBefore;
      }
      
      BPatchSnippetHandle *ret = insertSnippet(expr, *point, when, order);
      if (ret) {
         for (unsigned int j=0; j < ret->mtHandles.size(); j++) {
            handle->add(ret->mtHandles[j]);
         }
         delete ret;
      } else {
         delete handle;
         return NULL;
      }
   }
   
   return handle;
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
   if (handle->proc == llproc) {
      for (unsigned int i=0; i < handle->mtHandles.size(); i++)
         deleteInst(llproc, handle->mtHandles[i]);
      delete handle;
      return true;
   } 
   // Handle isn't to a snippet instance in this process
   cerr << "Error: wrong process in deleteSnippet" << endl;     
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
#ifdef BPATCH_SET_MUTATIONS_ACTIVE
   // If not activating or deactivating, just return.
   if ((activate && mutationsActive) || (!activate && !mutationsActive))
      return true;
   
   if (activate)
      llproc->reinstallMutations();
   else
      llproc->uninstallMutations();
   
   mutationsActive = activate;
#endif
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
   assert(point.point && newFunc.func);
   return llproc->replaceFunctionCall(point.point, newFunc.func);
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

   assert(oldFunc.func && newFunc.func);
   if (!mutationsActive)
      return false;
   
   // Self replacement is a nop
   if (oldFunc.func == newFunc.func)
      return true;

   bool old_recursion_flag = BPatch::bpatch->isTrampRecursive();
   BPatch::bpatch->setTrampRecursive( true );
   
   // We replace functions by instrumenting the entry of OLDFUNC with
   // a non-linking jump to NEWFUNC.  Calls to OLDFUNC do actually
   // transfer to OLDFUNC, but then our jump shunts them to NEWFUNC.
   // The non-linking jump ensures that when NEWFUNC returns, it
   // returns directly to the caller of OLDFUNC.
   BPatch_Vector<BPatch_point *> *pts = oldFunc.findPoint(BPatch_entry);
   if (! pts || ! pts->size())
      return false;
   BPatch_funcJumpExpr fje(newFunc);
   BPatchSnippetHandle * result = insertSnippet(fje, *pts, BPatch_callBefore);
   
   BPatch::bpatch->setTrampRecursive( old_recursion_flag );
   
   return (NULL != result);
#else
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
void *BPatch_process::oneTimeCodeInt(const BPatch_snippet &expr)
{
   return oneTimeCodeInternal(expr, NULL, true);
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
void BPatch_process::oneTimeCodeCallbackDispatch(process *theProc,
                                                 unsigned /* rpcid */, 
                                                 void *userData,
                                                 void *returnValue)
{
   assert(BPatch::bpatch != NULL);
   
   OneTimeCodeInfo *info = (OneTimeCodeInfo *)userData;
   
   BPatch_process *bproc =
      BPatch::bpatch->getProcessByPid(theProc->getPid());

   assert(bproc != NULL);
   assert(info && !info->isCompleted());

   info->setReturnValue(returnValue);
   info->setCompleted(true);
   
   if (!info->isSynchronous()) {
      if (BPatch::bpatch->oneTimeCodeCallback)
         event_mailbox->executeOrRegisterCallback(
             BPatch::bpatch->oneTimeCodeCallback, 
             bproc, info->getUserData(), returnValue);     
      delete info;
   }

#ifdef IBM_BPATCH_COMPAT
   if (BPatch::bpatch->RPCdoneCallback) {
      BPatch::bpatch->RPCdoneCallback(bproc, userData, returnValue);
   }
#endif
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
                                          void *userData,
                                          bool synchronous)
{
   bool needToResume = false;
   if (synchronous && !statusIsStopped()) {
      stopExecution();
      
      if (!statusIsStopped()) {
         cerr << "Failed to run oneTimeCodeInternal" << endl;
         return NULL;
      }
      needToResume = true;
   }

   OneTimeCodeInfo *info = new OneTimeCodeInfo(synchronous, userData);

   llproc->getRpcMgr()->postRPCtoDo(expr.ast,
                                  false, 
                                  BPatch_process::oneTimeCodeCallbackDispatch,
                                  (void *)info,
                                  false,
                                  NULL, NULL); 
    
   if (synchronous) {
      do {
         llproc->getRpcMgr()->launchRPCs(false);
         BPatch::bpatch->getThreadEvent(false);
      } while (!info->isCompleted() && !statusIsTerminated());
      
      void *ret = info->getReturnValue();
      delete info;

      if (needToResume) {
         continueExecution();
      }
        
      return ret;
   } else {
      llproc->getRpcMgr()->launchRPCs(llproc->status() == running);
      return NULL;
   }
}

//  BPatch_process::oneTimeCodeAsync
//
//  Have the specified code be executed by the mutatee once.  Don't wait 
//  until done.
bool BPatch_process::oneTimeCodeAsyncInt(const BPatch_snippet &expr, 
                                         void *userData)
{
   oneTimeCodeInternal(expr, userData, false);
   return true;
}

/*
 * BPatch_process::loadLibrary
 *
 * Load a dynamically linked library into the address space of the mutatee.
 *
 * libname	The name of the library to load.
 */
bool BPatch_process::loadLibraryInt(const char *libname, bool reload)
{
   stopExecution();
   if (!statusIsStopped()) {
      cerr << "Process not stopped in loadLibrary" << endl;
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
    
   if (!oneTimeCodeInternal(call_dlopen, NULL, true)) {
      BPatch_variableExpr *dlerror_str_var = 
         image->findVariable("gLoadLibraryErrorString");
      assert(NULL != dlerror_str_var);      
      char dlerror_str[256];
      dlerror_str_var->readValue((void *)dlerror_str, 256);
      BPatch_reportError(BPatchSerious, 124, dlerror_str);
      return false;
   }

#ifdef BPATCH_LIBRARY
#if defined(os_solaris) || (defined(os_linux) && defined(arch_x86)) || defined(os_aix)
	if(llproc->collectSaveWorldData && reload){
		llproc->saveWorldloadLibrary(libname);	
	}
#endif
#endif
   return true;
}

/** 
 * method that retrieves the line number and file name for a given
 * address. In failure it returns false. user is responsible to
 * to supply the buffer to write the file name and has to give the
 * size of the buffer available. The name of the file will be null
 * terminated by the program.
 */
bool BPatch_process::getLineAndFileInt(unsigned long addr,
                                       unsigned short& lineNo,
                                       char* fileName, int size)
{
	// /* DEBUG */ bperr( "Looking for addr 0x%lx.\n", addr );

	if(!fileName || (size <= 0)){
		return false;
	}
	size--;
	LineInformation* lineInformation = NULL;
	BPatch_Vector<BPatch_module*>* appModules = image->getModules();
	for(unsigned int i=0;i<appModules->size();i++){
		lineInformation = (*appModules)[i]->getLineInformation();

		if(!lineInformation) {
		  // /* DEBUG */ bperr( "No line information for module %d\n", i );
		  cerr << __FILE__ << __LINE__ << ": found NULL lineInformation "<< endl;
		  continue;
		}
#ifdef OLD_LINE_INFO
		
		Address inexactitude;
		Address leastInexactitude = 0xFFFFFFFF;
		unsigned short tempLineNo;
		
		for(int j=0;j<lineInformation->getSourceFileCount();j++){
			pdstring* fileN = lineInformation->sourceFileList[j];
			// /* DEBUG */ bperr( "Looking for information on source file '%s'.\n", fileN->c_str() );
			FileLineInformation* fInfo = 
				lineInformation->lineInformationList[j];
			if (!fInfo) {
			  // /* DEBUG */ bperr( "No information available on source file '%s'.\n", fileN->c_str() );
			  cerr << "found NULL FileLineInformation! "<< endl;
			  continue;
			}
			
			if( ! fInfo->getLineFromAddr( * fileN, tempLineNo, addr, true, false, & inexactitude ) ) { continue; }
			// /* DEBUG */ bperr( "%s: %d (0x%lx)\n", fileN->c_str(), tempLineNo, inexactitude );

			/* If the match got better, record its inexactitude, lineNo, and name as
			   our best match. */
			if( inexactitude < leastInexactitude ) {
				// /* DEBUG */ bperr( "Inexactitude decreased: 0x%lx\n", inexactitude );
				leastInexactitude = inexactitude;
				lineNo = tempLineNo;
				
				if( fileN->length() < (unsigned)size ) {
					size = fileN->length();
					}
				strncpy( fileName, fileN->c_str(), size );
				fileName[size] = '\0';
				}
			
			/* If the match is perfect, we're done; don't bother to iterate over the
			   rest of the files. */
			if( inexactitude == 0 ) {
				// /* DEBUG */ bperr( "Found perfect match.\n" );
				return true;
				}
			} /* end by-file iteraition */

		if( ! (leastInexactitude < 0xFFFFFFFF) ) {
			return false;
			}
		else {
			return true;
			}
#else
		if (lineInformation->getLineAndFile(addr, lineNo, fileName, size))
		    return true;
#endif
	}

	return false;
}


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

   if (range->is_relocated_func())
      func = range->is_relocated_func()->func();
   else
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

bool BPatch_process::registerAsyncThreadEventCallbackInt(
           BPatch_asyncEventType type,
           BPatchAsyncThreadEventCallback cb)
{
   bool ret = false;
   BPatch_asyncEventHandler *handler = BPatch::bpatch->eventHandler;
   ret = handler->registerThreadEventCallback(this, type, cb);
   if (ret) BPatch::bpatch->asyncActive = true;
   return ret;
}

bool BPatch_process::removeAsyncThreadEventCallbackInt(
           BPatch_asyncEventType type,
           BPatchAsyncThreadEventCallback cb)
{
   bool ret = false;
   BPatch_asyncEventHandler *handler = BPatch::bpatch->eventHandler;
   ret =  handler->removeThreadEventCallback(this, type, cb);
   if (ret) BPatch::bpatch->asyncActive = true;
   return ret;
}

bool BPatch_process::registerAsyncThreadEventCallbackMutateeSide(
           BPatch_asyncEventType type,
           BPatch_function *cb)
{
   BPatch_asyncEventHandler *handler = BPatch::bpatch->eventHandler;
   return handler->registerThreadEventCallback(this, type, cb);
}

bool BPatch_process::removeAsyncThreadEventCallbackMutateeSide(
           BPatch_asyncEventType type,
           BPatch_function *cb)
{
   BPatch_asyncEventHandler *handler = BPatch::bpatch->eventHandler;
   return handler->removeThreadEventCallback(this, type, cb);
}

void BPatch_process::setExitedViaSignal(int signalnumber) {
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

BPatch_thread *BPatch_process::getThreadInt(unsigned tid)
{
   for (unsigned i=0; i<threads.size(); i++)
      if (threads[i]->getTid() == tid)
         return threads[i];
   return NULL;
}

BPatch_function *BPatch_process::findOrCreateBPFunc(int_function* ifunc,
                                                    BPatch_module *bpmod)
{
  if (func_map->defines(ifunc)) {
    return func_map->get(ifunc);
  }
  
  // Find the module that contains the function
  if (bpmod == NULL && ifunc->pdmod() != NULL) {
    bpmod = getImage()->findModule(ifunc->pdmod()->fileName().c_str());
  }

  // findModule has a tendency to make new function objects... so
  // check the map again
  if (func_map->defines(ifunc)) {
    return func_map->get(ifunc);
  }

  BPatch_function *ret = new BPatch_function(this, ifunc, bpmod);
  return ret;
}

BPatch_point *BPatch_process::findOrCreateBPPoint(BPatch_function *bpfunc, 
              instPoint *ip, BPatch_procedureLocation pointType)
{
   Address addr = ip->pointAddr();
   if (ip->getOwner() != NULL) {
      Address baseAddr;
      if (llproc->getBaseAddress(ip->getOwner(), baseAddr)) {
         addr += baseAddr;
      }
   }

   if (instp_map->defines(addr)) 
      return instp_map->get(addr);
   
   if (bpfunc == NULL) 
      bpfunc = findOrCreateBPFunc(ip->pointFunc(), NULL);

   BPatch_point *pt = new BPatch_point(this, bpfunc, ip, pointType);
   instp_map->add(addr, pt);
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

#ifdef IBM_BPATCH_COMPAT
/**
 * In IBM's code, this is a wrapper for _BPatch_thread->addSharedObject (linux)
 * which is in turn a wrapper for creating a new 
 * ibmBpatchElf32Reader(name, addr)
 **/
bool BPatch_process::addSharedObjectInt(const char *name, 
                                        const unsigned long loadaddr)
{
   return loadLibrary(name);
}
#endif


/***************************************************************************
 * BPatch_snippetHandle
 ***************************************************************************/

/*
 * BPatchSnippetHandle::add
 *
 * Add an instance of a snippet to the list of instances held by the
 * BPatchSnippetHandle.
 *
 * instance	The instance to add.
 */
void BPatchSnippetHandle::add(miniTrampHandle *pointInstance)
{
   mtHandles.push_back(pointInstance);
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
