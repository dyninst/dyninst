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

// $Id: BPatch_thread.C,v 1.69 2003/01/02 19:51:47 schendel Exp $

#ifdef sparc_sun_solaris2_4
#include <dlfcn.h>
#endif

#define BPATCH_FILE


#include "process.h"
#include "inst.h"
#include "instP.h"

#include "BPatch.h"
#include "BPatch_thread.h"
#include "LineInformation.h"

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
 * BPatch_thread::getPid
 *
 * Return the process ID of the thread associated with this object.
 */
int BPatch_thread::getPid()
{
    return proc->getPid();
}


static void insertVForkInst(BPatch_thread *thread)
{

    BPatch_image *appImage = thread->getImage();
    if (!appImage) return;

#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11) //ccw 20 july 2000 : 28 mar 2001
    BPatch_function *vforkFunc = appImage->findFunction("DYNINSTvfork");

    BPatch_Vector<BPatch_function *>  vforks;
    if (NULL == appImage->findFunction("vfork", vforks, 1) || ( 0 == vforks.size())) {
      fprintf(stderr, "%s[%d]:  FATAL  : findFunction(`vfork`, ...), no vfork found!\n",
	      __FILE__, __LINE__);
      return;
    }
    
    if (vforks.size() > 1)
      // we should really go through the modules and make sure we have the right one here -- JAW
      fprintf(stderr, "%s[%d]:  SERIOUS  : found %d functions called 'vfork' in image, "
	      "might be picking the wrong one\n", __FILE__, __LINE__, vforks.size());
    
    BPatch_function *one_vfork = vforks[0];
    assert(one_vfork);

    BPatch_Vector<BPatch_point *> *points = one_vfork->findPoint(BPatch_exit);

    if (vforkFunc && points) {
	BPatch_Vector<BPatch_snippet *> args;
	BPatch_constExpr pidExpr(thread->getPid());
	args.push_back(&pidExpr);

	BPatch_snippet *ret = new BPatch_funcCallExpr(*vforkFunc, args);
	if (!ret) {
	    printf("error creating function\n");
	    return;
	}

	thread->insertSnippet(*ret, *points);
    }
#endif
}

/*
 * BPatch_thread::BPatch_thread
 *
 * Starts a new process and associates it with the BPatch_thread being
 * constructed.  The new process is placed into a stopped state before
 * executing any code.
 *
 * path		Pathname of the executable to start.
 * argv		A list of pointers to character strings which are the
 *              arguments for the new process, terminated by a NULL pointer.
 * envp		A list of pointers to character strings which are to be
 *              passed to the new process as its environment, terminated by a
 *              NULL pointer.  If envp is NULL, the parent's environment is
 *              copied and passed to the child.
 */
BPatch_thread::BPatch_thread(const char *path, char *argv[], char *envp[],
			     int stdin_fd, int stdout_fd, int stderr_fd)
  : proc(NULL), image(NULL), lastSignal(-1), exitCode(-1), mutationsActive(true), 
    createdViaAttach(false), detached(false),
    unreportedStop(false), unreportedTermination(false)
{
    pdvector<string> argv_vec;
    pdvector<string> envp_vec;

    // Contruct a vector out of the contents of argv
    for(int i = 0; argv[i] != NULL; i++)
	argv_vec.push_back(argv[i]);

    // Construct a vector out of the contents of envp
    if (envp != NULL) {
    	for(int i = 0; envp[i] != NULL; i++)
    	    envp_vec.push_back(envp[i]);
    }

    string directoryName = "";

#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11) // i.e. for all unixes

    // this fixes a problem on linux and alpha platforms where pathless filenames
    // are searched for either in a standard, predefined path, or in $PATH by execvp.
    // thus paths that should resolve to "./" are missed.
    // Note that the previous use of getcwd() here for the alpha platform was dangerous
    // since this is an API and we don't know where the user's code will leave the
    // cwd pointing.
    const char *dotslash = "./";
    if (NULL == strchr(path, '/'))
      directoryName = dotslash;
#endif

    //#if defined(alpha_dec_osf4_0)
    //char buf[1024];
    //(void*) getcwd(buf, sizeof(buf));
    //directoryName = buf;
    //#endif

    proc = createProcess(path, argv_vec, envp_vec, directoryName, stdin_fd,
      stdout_fd, stderr_fd);

    // XXX Should do something more sensible.
    if (proc == NULL) return;

    proc->thread = this;

    // Add this object to the list of threads
    // XXX Should be conditional on success of creating process
    assert(BPatch::bpatch != NULL);
    BPatch::bpatch->registerThread(this);

    image = new BPatch_image(proc);

    while (!proc->isBootstrappedYet() && !statusIsTerminated())
	BPatch::bpatch->getThreadEvent(false);

    if (BPatch::bpatch->postForkCallback) {
      insertVForkInst(this);
    }
}


/*
 * BPatch_thread::BPatch_thread
 *
 * Constructs a new BPatch_thread and associates it with a running process.
 * Stops execution of the process.
 *
 * path		Pathname of the executable file for the process.
 * pid		Process ID of the target process.
 */
BPatch_thread::BPatch_thread(const char *path, int pid)
  : proc(NULL), image(NULL), lastSignal(-1), exitCode(-1), mutationsActive(true), 
    createdViaAttach(true), detached(false),
    unreportedStop(false), unreportedTermination(false)
{

    /* For some reason, on Irix, evaluating the return value of
       attachProcess directly (i.e. no "ret" variable) causes the
       expression to be incorrectly evaluated as false.  This appears
       to be a compiler bug ("g++ -mabi=64 -O3"). */
    bool ret = attachProcess(path, pid, 1, &proc);
    if (!(ret)) {
      // XXX Should do something more sensible
      proc = NULL;
      return;
    }

    proc->thread = this;

    // Add this object to the list of threads
    assert(BPatch::bpatch != NULL);
    BPatch::bpatch->registerThread(this);

    image = new BPatch_image(proc);

    while (!proc->isBootstrappedYet() && !statusIsTerminated()) {
	BPatch::bpatch->getThreadEventOnly(false);
	proc->launchRPCifAppropriate(false);
    }

#if defined(alpha_dec_osf4_0) 
    // need to perform this after dyninst Heap is present and happy 
    proc->getDyn()->setMappingHooks(proc); 
#endif 

    insertVForkInst(this);
}

/*
 * BPatch_thread::BPatch_thread
 *
 * Constructs a new BPatch_thread and associates it with a forked process.
 *
 * parentPid          Pathname of the executable file for the process.
 * childPid           Process ID of the target process.
 */
BPatch_thread::BPatch_thread(int /*pid*/, process *nProc)
  : proc(nProc), image(NULL), lastSignal(-1), exitCode(-1), mutationsActive(true), 
    createdViaAttach(true), detached(false),
    unreportedStop(false), unreportedTermination(false)
{
    proc->thread = this;

    // Add this object to the list of threads
    assert(BPatch::bpatch != NULL);
    BPatch::bpatch->registerThread(this);

    image = new BPatch_image(proc);
}


/*
 * BPatch_thread::~BPatch_thread
 *
 * Destructor for BPatch_thread.  Detaches from the running thread.
 */
BPatch_thread::~BPatch_thread()
{
    if (image) delete image;

    // XXX Make sure that anything else that needs to be deallocated
    //     gets taken care of.
    if (!proc) return;

    if (!detached) {
#if !defined(i386_unknown_nt4_0) && !(defined mips_unknown_ce2_11) //ccw 20 july 2000 : 28 mar 2001
    	if (createdViaAttach && !statusIsTerminated())
    	    proc->API_detach(true);
	else
#endif
	    terminateExecution();
    }

    assert(BPatch::bpatch != NULL);
    BPatch::bpatch->unRegisterThread(getPid());

    // Remove the process from our list of processes
    unsigned i;
    for (i = 0; i < processVec.size(); i++) {
	if (processVec[i] == proc) {
	    break;
	}
    }
    assert(i < processVec.size());
    while (i < processVec.size()-1) {
	processVec[i] = processVec[i+1];
	i++;
    }
    processVec.resize(processVec.size()-1);

    // XXX I think there are some other things we need to deallocate -- check
    // on that.

    // XXX We'd like to delete the process object here, but if we do that then
    // we get problems (specifically, the library gets confused in some way so
    // that it won't be able to run another mutatee process).  Anyway, the
    // process class has no destructor right now, so whether we delete proc or
    // not we're leaking megabytes of memory.  So, for now, we just leave it
    // around.
    // delete proc;
}


/*
 * BPatch_thread::stopExecution
 *
 * Puts the thread into the stopped state.
 */
bool BPatch_thread::stopExecution()
{
    assert(BPatch::bpatch);
    BPatch::bpatch->getThreadEvent(false);

#ifdef DETACH_ON_THE_FLY
    return proc->reattachAndPause();
#else
    return proc->pause();
#endif


    assert(BPatch::bpatch);

    while (!statusIsStopped())
	BPatch::bpatch->getThreadEvent(false);

    setUnreportedStop(false);

    return true;
}


/*
 * BPatch_thread::continueExecution
 *
 * Puts the thread into the running state.
 */
bool BPatch_thread::continueExecution()
{
    assert(BPatch::bpatch);
    BPatch::bpatch->getThreadEvent(false);

#ifdef DETACH_ON_THE_FLY
    if (proc->detachAndContinue()) {
#else
    if (proc->continueProc()) {
#endif
	setUnreportedStop(false);
	return true;
    }

    return false;
}


/*
 * BPatch_thread::terminateExecution
 *
 * Kill the thread.
 */
bool BPatch_thread::terminateExecution()
{
    if (!proc || !proc->terminateProc())
	return false;

    // Wait for the process to die
    while (!isTerminated()) ;

    return true;
}


/*
 * BPatch_thread::isStopped
 *
 * Returns true if the thread has stopped, and false if it has not.  This may
 * involve checking for thread events that may have recently changed this
 * thread's status.  This function also updates the unreportedStop flag if a
 * stop is detected, in order to indicate that the stop has been reported to
 * the user.
 */
bool BPatch_thread::isStopped()
{
    // Check for status changes.
    assert(BPatch::bpatch);
    BPatch::bpatch->getThreadEvent(false);
    if (statusIsStopped()) {
	setUnreportedStop(false);
	return true;
    } else
	return false;
}


/*
 * BPatch_thread::statusIsStopped
 *
 * Returns true if the thread is stopped, and false if it is not.
 */
bool BPatch_thread::statusIsStopped()
{
    return proc->status() == stopped;
}


/*
 * BPatch_thread::stopSignal
 *
 * Returns the number of the signal which caused the thread to stop.
 */
int BPatch_thread::stopSignal()
{
    if (proc->status() != neonatal && proc->status() != stopped)
	return -1;
    else
	return lastSignal;
}


/*
 * BPatch_thread::terminationStatus
 *
 * Returns the exit status code of the terminated thread.
 */
int BPatch_thread::terminationStatus()
{
    if (proc->status() != exited)
	return -1;
    else
	return exitCode;
}


/*
 * BPatch_thread::isTerminated
 *
 * Returns true if the thread has terminated, and false if it has not.  This
 * may involve checking for thread events that may have recently changed this
 * thread's status.  This function also updates the unreportedTermination flag
 * if the program terminated, in order to indicate that the termination has
 * been reported to the user.
 */
bool BPatch_thread::isTerminated()
{
    // Check for status changes.
    assert(BPatch::bpatch);
    BPatch::bpatch->getThreadEvent(false);

    if (statusIsTerminated()) {
	proc->terminateProc();
	setUnreportedTermination(false);
	return true;
    } else
	return false;
}


/*
 * BPatch_thread::statusIsTerminated
 *
 * Returns true if the process has terminated, false if it has not.
 */
bool BPatch_thread::statusIsTerminated()
{
    if (proc == NULL) return true;
    return proc->status() == exited;
}


/*
 * BPatch_thread::detach
 *
 * Detach from the thread represented by this object.
 *
 * cont		True if the thread should be continued as the result of the
 * 		detach, false if it should not.
 */
void BPatch_thread::detach(bool cont)
{
#ifdef DETACH_ON_THE_FLY
    proc->reattachAndPause();
#endif
    proc->API_detach(cont);

    detached = true;
}


/*
 * BPatch_thread::dumpCore
 *
 * Causes the thread to dump its state to a file, and optionally to terminate.
 * Returns true upon success, and false upon failure.
 *
 * file		The name of the file to which the state should be written.
 * terminate	Indicates whether or not the thread should be terminated after
 *		dumping core.  True indicates that it should, false that is
 *		should not.
 */
bool BPatch_thread::dumpCore(const char *file, bool terminate)
{
    bool was_stopped;
    bool had_unreportedStop = unreportedStop;
    if (isStopped()) was_stopped = true;
    else was_stopped = false;

    stopExecution();

    bool ret = proc->dumpCore(file);
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
 * BPatch_thread::dumpPatchedImage
 *
 * Writes the mutated file back to disk,
 * in ELF format.  (Solaris only)
 *
 *
 */
char* BPatch_thread::dumpPatchedImage(const char* file){ //ccw 28 oct 2001

#if !defined(sparc_sun_solaris2_4) && !defined(i386_unknown_linux2_0)
	return NULL;
#else
    bool was_stopped;
    bool had_unreportedStop = unreportedStop;
    if (isStopped()) was_stopped = true;
    else was_stopped = false;

    stopExecution();

    char* ret = proc->dumpPatchedImage(file);
    if (was_stopped) {
        unreportedStop = had_unreportedStop;
    } else {
        continueExecution();
    }

    return ret;
#endif
}


/*
 * BPatch_thread::dumpImage
 *
 * Writes the contents of memory into a file.
 * Returns true upon success, and false upon failure.
 *
 * file		The name of the file to which the image should be written.
 */
bool BPatch_thread::dumpImage(const char *file)
{
#if defined(i386_unknown_nt4_0) || defined(mips_unknown_ce2_11) //ccw 20 july 2000 : 28 mar 2001
    return false;
#else
    bool was_stopped;
    bool had_unreportedStop = unreportedStop;
    if (isStopped()) was_stopped = true;
    else was_stopped = false;

    stopExecution();

    bool ret = proc->dumpImage(file);
    if (was_stopped) {
    	unreportedStop = had_unreportedStop;
    } else {
	continueExecution();
    }

    return ret;
#endif
}


/*
 * BPatch_thread::malloc
 *
 * Allocate memory in the thread's address space.
 *
 * n	The number of bytes to allocate.
 *
 * Returns:
 * 	A pointer to a BPatch_variableExpr representing the memory.
 *
 */
BPatch_variableExpr *BPatch_thread::malloc(int n)
{
    assert(BPatch::bpatch != NULL);

    void *ptr = (void *) proc->inferiorMalloc(n, dataHeap);
    if (!ptr) {
	return NULL;
    }

    BPatch_variableExpr *ret;
    ret =  new BPatch_variableExpr(proc, ptr, BPatch::bpatch->type_Untyped);
    return ret;
}


/*
 * BPatch_thread::malloc
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
BPatch_variableExpr *BPatch_thread::malloc(const BPatch_type &type)
{
    void *mem = (void *)proc->inferiorMalloc(type.getSize(), dataHeap);

    if (!mem) return NULL;

    return new BPatch_variableExpr(proc, mem, &type);
}


/*
 * BPatch_thread::free
 *
 * Free memory that was allocated with BPatch_thread::malloc.
 *
 * ptr		A BPatch_variableExpr representing the memory to free.
 */
void BPatch_thread::free(const BPatch_variableExpr &ptr)
{
    proc->inferiorFree((Address)ptr.getBaseAddr());
}


/*
 * BPatch_thread::getInheritedVariable
 *
 * Allows one to retrieve a variable which exists in a child process which 
 * was inherited from and originally created in the parent process.
 * Function is invoked on the child BPatch_thread (created from a fork in 
 * the application).
 *
 * parentVar   A BPatch_variableExpr created in the parent thread
 *
 * Returns:    The corresponding BPatch_variableExpr from the child thread
 *             or NULL if the variable argument hasn't been malloced
 *             in a parent process.
 */
BPatch_variableExpr *BPatch_thread::getInheritedVariable(
                                          const BPatch_variableExpr &parentVar)
{
  if(! isInferiorAllocated(proc, (Address)parentVar.getBaseAddr())) {
    // isn't defined in this process so must not have been defined in a
    // parent process
    return NULL;
  }
  return new BPatch_variableExpr(proc, parentVar.getBaseAddr(),
				 parentVar.getType());
}


/*
 * BPatch_thread::getInheritedSnippet
 *
 * Allows one to retrieve a snippet which exists in a child process which 
 * was inherited from and originally created in the parent process.
 * Function is invoked on the child BPatch_thread (created from a fork in 
 * the application).
 *
 * Allows one to retrieve a snippet which exists in a child process which
 * was inherited from and originally created in the parent process.
 * Function is invoked on the child BPatch_thread (created from a fork in
 * the application).
 *
 * parentSnippet: A BPatchSnippetHandle created in the parent thread
 *
 * Returns:       The corresponding BPatchSnippetHandle from the child thread.
 *
 */
BPatchSnippetHandle *BPatch_thread::getInheritedSnippet(
			 	          BPatchSnippetHandle &parentSnippet)
{
  // a BPatchSnippetHandle has an miniTrampHandle for each point that
  // the instrumentation is inserted at
  BPatch_Vector<miniTrampHandle *> parent_mtHandles;
  parentSnippet.getMiniTrampHandles(&parent_mtHandles);

  BPatchSnippetHandle *childSnippet = new BPatchSnippetHandle(proc);
  for(unsigned i=0; i<parent_mtHandles.size(); i++) {
    miniTrampHandle *childMT = new miniTrampHandle;
    if(! getInheritedMiniTramp(parent_mtHandles[i], childMT, proc)) {
      // error, couldn't find a snippet
      return NULL;
    }
    childSnippet->add(childMT);
  }
  return childSnippet;
}


/*
 * BPatch_thread::insertSnippet
 *
 * Insert a code snippet at a given instrumentation point.  Upon succes,
 * returns a handle to the created instance of the snippet, which can be used
 * to delete it.  Otherwise returns NULL.
 *
 * expr		The snippet to insert.
 * point	The point at which to insert it.
 */
BPatchSnippetHandle *BPatch_thread::insertSnippet(const BPatch_snippet &expr,
						  BPatch_point &point,
						  BPatch_snippetOrder order)
{
      BPatch_callWhen when;

      // which one depends on the type of the point
      // need to cast away const since getPointType isn't const
      if (((BPatch_point)point).getPointType() == BPatch_exit) {
	  when = BPatch_callAfter;
      } else {
	  when = BPatch_callBefore;
      }

      return insertSnippet(expr, point, when, order);
}

/*
 * BPatch_thread::insertSnippet
 *
 * Insert a code snippet at a given instrumentation point.  Upon succes,
 * returns a handle to the created instance of the snippet, which can be used
 * to delete it.  Otherwise returns NULL.
 *
 * expr		The snippet to insert.
 * point	The point at which to insert it.
 */
BPatchSnippetHandle *BPatch_thread::insertSnippet(const BPatch_snippet &expr,
						  BPatch_point &point,
						  BPatch_callWhen when,
						  BPatch_snippetOrder order)
{
    // Can't insert code when mutations are not active.
    if (!mutationsActive)
	return NULL;

    // code is null (possibly an empy sequence or earlier error)
    if (!expr.ast) return NULL;

    callWhen 	_when;
    callOrder	_order;

    switch (when) {
      case BPatch_callBefore:
	_when = callPreInsn;
	break;
      case BPatch_callAfter:
	_when = callPostInsn;
	break;
      default:
	return NULL;
    };

    switch (order) {
      case BPatch_firstSnippet:
	_order = orderFirstAtPoint;
	break;
      case BPatch_lastSnippet:
	_order = orderLastAtPoint;
	break;
      default:
	return NULL;
    }

    assert(BPatch::bpatch != NULL);	// We'll use this later

    //
    // Check for valid combinations of BPatch_procedureLocation & call*
    // 	Right now we don't allow
    //		BPatch_callBefore + BPatch_exit
    //		BPatch_callAfter + BPatch_entry
    //
    //	These combinations are intended to be used to mark the point that
    //      is the last, first valid point where the local variables are
    //      valid.  This is deifferent than the first/last instruction of
    //      a subroutine which is what the other combinations of BPatch_entry
    //	    and BPatch_exit refer to.
    //
    //	jkh 3/1/00 (based on agreement reached at dyninst Jan'00 meeting)
    //
    if ((when == BPatch_callBefore) && 
	(point.getPointType() == BPatch_exit)) {
	BPatch_reportError(BPatchSerious, 113,
	       "BPatch_callBefore at BPatch_exit not supported yet");
	return NULL;
    } else if ((when == BPatch_callAfter) && 
	       (point.getPointType() == BPatch_entry)) {
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

    // XXX Really only need to type check once per function the snippet is
    // being inserted into, not necessarily once per point.
    if (BPatch::bpatch->isTypeChecked()) {
	assert(expr.ast);
	if (expr.ast->checkType() == BPatch::bpatch->type_Error) {
	    // XXX Type check error - should call callback
	    return NULL;
	}
    }

    BPatchSnippetHandle *handle = new BPatchSnippetHandle(proc);

    AstNode *ast = (AstNode *)expr.ast;  /* XXX no const */

    // XXX We just pass false for the "noCost" parameter here - do we want
    // to make that an option?
    miniTrampHandle *mtHandle = new miniTrampHandle;
    instPoint *&ip = (instPoint*&) point.point;

    if(point.proc != proc) {
      cerr << "insertSnippet: the given instPoint isn't from the same process "
	   << "as the invoked BPatch_thread\n";
      return NULL;
    }

    loadMiniTramp_result res = addInstFunc(mtHandle, proc, ip, ast, _when, 
					   _order, false,
			// Do we want the base tramp (if any) created allowing
			// recursion? 
#if defined(mips_sgi_irix6_4)
			// On MIPS, we can't have recursive guards on arbitrary
			// inst point.
			point.getPointType() == BPatch_instruction ?  true : 
#endif
			BPatch::bpatch->isTrampRecursive()
					   );

    if(res == success_res) {
      handle->add(mtHandle);
    } else {
      delete handle;
      return NULL;
    }
    return handle;
}


/*
 * BPatch_thread::insertSnippet
 *
 * Insert a code snippet at each of a list of instrumentation points.  Upon
 * success, Returns a handle to the created instances of the snippet, which
 * can be used to delete them (as a unit).  Otherwise returns NULL.
 *
 * expr		The snippet to insert.
 * points	The list of points at which to insert it.
 */
BPatchSnippetHandle *BPatch_thread::insertSnippet(
				    const BPatch_snippet &expr,
				    const BPatch_Vector<BPatch_point *> &points,
				    BPatch_callWhen when,
				    BPatch_snippetOrder order)
{
    BPatchSnippetHandle *handle = new BPatchSnippetHandle(proc);

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
 * BPatch_thread::insertSnippet
 *
 * Insert a code snippet at each of a list of instrumentation points.  Upon
 * success, Returns a handle to the created instances of the snippet, which
 * can be used to delete them (as a unit).  Otherwise returns NULL.
 *
 * expr		The snippet to insert.
 * points	The list of points at which to insert it.
 */
BPatchSnippetHandle *BPatch_thread::insertSnippet(
				    const BPatch_snippet &expr,
				    const BPatch_Vector<BPatch_point *> &points,
				    BPatch_snippetOrder order)
{
    BPatchSnippetHandle *handle = new BPatchSnippetHandle(proc);

    for (unsigned int i = 0; i < points.size(); i++) {
	BPatch_point *point = points[i]; // Cast away const

        BPatch_callWhen when;

        // which one depends on the type of the point
        // need to cast away const since getPointType isn't const
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
 * BPatch_thread::deleteSnippet
 * 
 * Deletes an instance of a snippet.
 *
 * handle	The handle returned by insertSnippet when the instance to
 *		deleted was created.
 */
bool BPatch_thread::deleteSnippet(BPatchSnippetHandle *handle)
{
    if (handle->proc == proc) {
	for (unsigned int i=0; i < handle->mtHandles.size(); i++) {
	  deleteInst(proc, *(handle->mtHandles[i]));
	}
	delete handle;
	return true;
    } else { // Handle isn't to a snippet instance in this process
	return false;
    }
}


/*
 * BPatch_thread::setMutationsActive
 *
 * Enable or disable the execution of all snippets for the thread.
 * 
 * activate	If set to true, execution of snippets is enabled.  If false,
 *		execution is disabled.
 */
void BPatch_thread::setMutationsActive(bool activate)
{
    // If not activating or deactivating, just return.
    if ((activate && mutationsActive) || (!activate && !mutationsActive))
	return;

#if 0
    // The old implementation
    dictionary_hash_iter<const instPoint*, trampTemplate *> bmi(proc->baseMap);

    const instPoint *point;
    trampTemplate   *tramp;

    while (bmi.next(point, tramp)) {

	/*
	if (tramp->retInstance != NULL) {
	    if (activate)
		tramp->retInstance->installReturnInstance(proc);
	    else
		tramp->retInstance->unInstallReturnInstance(proc);
	}
	*/
    }
#endif
    if (activate)
	proc->reinstallMutations();
    else
	proc->uninstallMutations();

    mutationsActive = activate;
}


/*
 * BPatch_thread::replaceFunctionCall
 *
 * Replace a function call with a call to a different function.  Returns true
 * upon success, false upon failure.
 * 
 * point	The call site that is to be changed.
 * newFunc	The function that the call site will now call.
 */
bool BPatch_thread::replaceFunctionCall(BPatch_point &point,
					BPatch_function &newFunc)
{
    // Can't make changes to code when mutations are not active.
    if (!mutationsActive)
	return false;

    assert(point.point && newFunc.func);

    return proc->replaceFunctionCall(point.point, newFunc.func);
}


/*
 * BPatch_thread::removeFunctionCall
 *
 * Replace a function call with a NOOP.  Returns true upon success, false upon
 * failure.
 * 
 * point	The call site that is to be NOOPed out.
 */
bool BPatch_thread::removeFunctionCall(BPatch_point &point)
{
    // Can't make changes to code when mutations are not active.
    if (!mutationsActive)
	return false;

    assert(point.point);

    return proc->replaceFunctionCall(point.point, NULL);
}


/*
 * BPatch_thread::replaceFunction
 *
 * Replace all calls to function OLDFUNC with calls to NEWFUNC.
 * Returns true upon success, false upon failure.
 * 
 * oldFunc	The function to replace
 * newFunc      The replacement function
 */
bool BPatch_thread::replaceFunction(BPatch_function &oldFunc,
				    BPatch_function &newFunc)
{
#if defined(sparc_sun_solaris2_4) || defined(alpha_dec_osf4_0) || defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
    // Can't make changes to code when mutations are not active.
    if (!mutationsActive)
	return false;

    assert(oldFunc.func && newFunc.func);

    // Self replacement is a nop
    if (oldFunc.func == newFunc.func)
	 return true;

    /* mihai
     *
     */
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

    /* mihai
     *
     */
    BPatch::bpatch->setTrampRecursive( old_recursion_flag );

    return (NULL != result);
#else
    BPatch_reportError(BPatchSerious, 109,
		       "replaceFunction is not implemented on this platform");
    return false;
#endif
}


/*
 * BPatch_thread::oneTimeCodeCallbackDispatch
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
void BPatch_thread::oneTimeCodeCallbackDispatch(process *theProc,
						void *userData,
						void *returnValue)
{
    assert(BPatch::bpatch != NULL);

    OneTimeCodeInfo *info = (OneTimeCodeInfo *)userData;

    BPatch_thread *theThread =
	BPatch::bpatch->getThreadByPid(theProc->getPid());

    assert(theThread != NULL);
    assert(info && !info->isCompleted());

    info->setReturnValue(returnValue);
    info->setCompleted(true);

    if (!info->isSynchronous()) {
	if (BPatch::bpatch->oneTimeCodeCallback)
	    BPatch::bpatch->oneTimeCodeCallback(theThread, info->getUserData(), returnValue);

	delete info;
    }

#ifdef IBM_BPATCH_COMPAT
    if (BPatch::bpatch->RPCdoneCallback) {
	printf("invoking IBM thread callback function\n");
	BPatch::bpatch->RPCdoneCallback(theThread, userData, returnValue);
    }
#endif
}


/*
 * BPatch_thread::oneTimeCodeInternal
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
void *BPatch_thread::oneTimeCodeInternal(const BPatch_snippet &expr,
					 void *userData,
					 bool synchronous)
{
    bool needToResume = false;

    if (synchronous && !statusIsStopped()) {
        stopExecution();
        if (!statusIsStopped()) {
            return NULL;
        }
        needToResume = true;
    }

    OneTimeCodeInfo *info = new OneTimeCodeInfo(synchronous, userData);

    proc->postRPCtoDo(expr.ast,
		      false, // XXX = calculate cost - is this what we want?
		      BPatch_thread::oneTimeCodeCallbackDispatch, // Callback
		      (void *)info, // User data
		      -1,   // This isn't a metric definition - we shouldn't
		      NULL, // No particular thread (yet),
		      0,    // Same -- no kernel thread
                      false);  

    if (synchronous) {
	do {
	    proc->launchRPCifAppropriate(false);
	    BPatch::bpatch->getThreadEvent(false);
	} while (!info->isCompleted() && !statusIsTerminated());

	void *ret = info->getReturnValue();
	delete info;

	if (needToResume)
	    continueExecution();

	return ret;
    } else {
	proc->launchRPCifAppropriate(proc->status() == running);

	return NULL;
    }
}


/*
 * BPatch_thread::loadLibrary
 *
 * Load a dynamically linked library into the address space of the mutatee.
 *
 * libname	The name of the library to load.
 */
bool BPatch_thread::loadLibrary(const char *libname, bool reload)
{
#if defined(sparc_sun_solaris2_4)  || defined(i386_unknown_solaris2_5) || \
    defined(i386_unknown_linux2_0) || defined(mips_sgi_irix6_4) || \
    defined(alpha_dec_osf4_0) || defined(rs6000_ibm_aix4_1) ||\
    defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
  if (!statusIsStopped()) {
    return false;
  }
    BPatch_Vector<BPatch_snippet *> args;

    BPatch_constExpr nameArg(libname);
    // BPatch_constExpr modeArg(RTLD_NOW | RTLD_GLOBAL);

    args.push_back(&nameArg);
    // args.push_back(&modeArg);

    BPatch_function *dlopen_func = image->findFunction("DYNINSTloadLibrary");
    if (dlopen_func == NULL) return false;

    BPatch_funcCallExpr call_dlopen(*dlopen_func, args);

    if (!oneTimeCodeInternal(call_dlopen, NULL, true)) {
      // dlopen FAILED
      // find the (global var) error string in the RT Lib and send it to the
      // error reporting mechanism
      BPatch_variableExpr *dlerror_str_var = image->findVariable("gLoadLibraryErrorString");
      assert(NULL != dlerror_str_var);
      
      char dlerror_str[256];
      dlerror_str_var->readValue((void *)dlerror_str, 256);
      BPatch_reportError(BPatchSerious, 124, dlerror_str);
      return false;
    }

#ifdef BPATCH_LIBRARY //ccw 14 may 2002
#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)
	if(proc->collectSaveWorldData && reload){
		proc->saveWorldloadLibrary(libname);	
	}
#endif
#endif
    return true;
#else
    return false;
#endif
}

/** method that retrieves the line number and file name for a given
  * address. In failure it returns false. user is responsible to
  * to supply the buffer to write the file name and has to give the
  * size of the buffer available. The name of the file will be null
  * terminated by the program.
  */
bool BPatch_thread::getLineAndFile(unsigned long addr,unsigned short& lineNo,
		    		   char* fileName,int size)
{
	if(!fileName || (size <= 0)){
		return false;
	}
	size--;
	LineInformation* lineInformation = NULL;
	BPatch_Vector<BPatch_module*>* appModules = image->getModules();
	for(unsigned int i=0;i<appModules->size();i++){
		lineInformation = (*appModules)[i]->lineInformation;
		if(!lineInformation)
			continue;
		for(int j=0;j<lineInformation->sourceFileCount;j++){
			string* fileN = lineInformation->sourceFileList[j];
			FileLineInformation* fInfo = 
				lineInformation->lineInformationList[j];
			if(fInfo->getLineFromAddr(*fileN,lineNo,addr,true,false)){
				if(fileN->length() < (unsigned)size)
					size = fileN->length();
				strncpy(fileName,fileN->c_str(),size);
				fileName[size] = '\0';
				return true;
			}	
		}
	}
	return false;
}


/*
 * BPatch_thread::findFunctionByAddr
 *
 * Returns the function that contains the specified address, or NULL if the
 * address is not within a function.
 *
 * addr		The address to use for the lookup.
 */
BPatch_function *BPatch_thread::findFunctionByAddr(void *addr)
{
    pd_Function *func;
    
    if ((func = proc->findFuncByAddr((Address)addr)) == NULL)
	return NULL;

    return proc->findOrCreateBPFunc(func);
}


/* 
	this function sets a flag in process that 
	forces the collection of data for saveworld. //ccw 23 jan 2002
*/
void BPatch_thread::startSaveWorld(){
	proc->collectSaveWorldData=true;
}


/*
 * BPatch_thread::getCallStack
 *
 * Returns information about the frames currently on the thread's stack.
 *
 * stack	The vector to fill with the stack trace information.
 */
void BPatch_thread::getCallStack(BPatch_Vector<BPatch_frame>& stack)
{
    pdvector<pdvector<Frame> > stackWalks;

    proc->walkStacks(stackWalks);

    // We can only handle one thread right now; change when we begin to handle
    // multiple threads.
    assert(stackWalks.size() == 1);

    for (int i = 0; i < stackWalks[0].size(); i++) {
	stack.push_back(BPatch_frame(this,
				     (void*)stackWalks[0][i].getPC(),
				     (void*)stackWalks[0][i].getFP()));
    }
}

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
BPatchSnippetHandle::~BPatchSnippetHandle()
{
    // don't delete inst instances since they are might have been copied
}

#ifdef IBM_BPATCH_COMPAT
bool BPatch_thread::addSharedObject(const char *name, const unsigned long loadaddr)
{
  //  in IBM's code, this is a wrapper for _BPatch_thread->addSharedObject (linux)
  // which is in turn a wrapper for creating a new ibmBpatchElf32Reader(name, addr)
  //
  fprintf(stderr, "%s[%d]: inside addSharedObject(%s, %lu), which is not properly implemented\n"
	  "using loadLibrary(char* = %s)\n",
	  __FILE__, __LINE__, name, loadaddr, name);

  return loadLibrary(name);
}

#endif

/***************************************************************************
 * BPatch_frame
 ***************************************************************************/

/*
 * BPatch_frame::getFrameType()
 *
 * Returns the type of frame: BPatch_frameNormal for the stack frame for a
 * function, BPatch_frameSignal for the stack frame created when a signal is
 * delivered, or BPatch_frameTrampoline for a stack frame for a trampoline.
 */
BPatch_frameType BPatch_frame::getFrameType()
{
    if (thread->proc->isInSignalHandler((Address)getPC()))
	return BPatch_frameSignal;
    else
	return BPatch_frameNormal;
}

/*
 * BPatch_frame::findFunction()
 *
 * Returns the function associated with the stack frame, or NULL if there is
 * none.
 */
BPatch_function *BPatch_frame::findFunction()
{
    return thread->findFunctionByAddr(getPC());
}
