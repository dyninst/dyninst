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

#include "process.h"
#include "inst.h"
#include "instP.h"

#include "BPatch.h"
#include "BPatch_thread.h"


/*
 * BPatch_thread::getPid
 *
 * Return the process ID of the thread associated with this object.
 */
int BPatch_thread::getPid()
{
    return proc->getPid();
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
BPatch_thread::BPatch_thread(char *path, char *argv[], char *envp[])
    : lastSignal(-1), mutationsActive(true), createdViaAttach(false),
      detached(false), proc(NULL), image(NULL)
{
    vector<string> argv_vec;
    vector<string> envp_vec;

    // Contruct a vector out of the contents of argv
    for(int i = 0; argv[i] != NULL; i++)
	argv_vec += argv[i];

    // Construct a vector out of the contents of envp
    if (envp != NULL) {
    	for(int i = 0; envp[i] != NULL; i++)
    	    envp_vec += envp[i];
    }

    proc = createProcess(path, argv_vec, envp_vec, "");

    // XXX Should do something more sensible.
    if (proc == NULL) return;

    // Add this object to the list of threads
    // XXX Should be conditional on success of creating process
    assert(BPatch::bpatch != NULL);
    BPatch::bpatch->registerThread(this);

    image = new BPatch_image(proc);

    while (!proc->isBootstrappedYet() && !isTerminated())
	pollForStatusChange();
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
BPatch_thread::BPatch_thread(char *path, int pid)
    : lastSignal(-1), mutationsActive(true), createdViaAttach(true),
      detached(false), proc(NULL), image(NULL)
{
    if (!attachProcess(path, pid, 1, proc)) {
    	// XXX Should do something more sensible
	proc = NULL;
	return;
    }

    // Add this object to the list of threads
    assert(BPatch::bpatch != NULL);
    BPatch::bpatch->registerThread(this);

    image = new BPatch_image(proc);

    while (!proc->isBootstrappedYet() && !isTerminated()) {
	pollForStatusChange();
	proc->launchRPCifAppropriate(false, false);
    }
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
    	if (createdViaAttach)
    	    proc->API_detach(true);
	else
	    terminateExecution();
    }

    assert(BPatch::bpatch != NULL);
    BPatch::bpatch->unRegisterThread(getPid());

    // XXX I think there are some other things we need to deallocate -- check
    // on that.

    delete proc;
}


/*
 * BPatch_thread::stopExecution
 *
 * Puts the thread into the stopped state.
 */
bool BPatch_thread::stopExecution()
{
    pollForStatusChange();

    return proc->pause();
}


/*
 * BPatch_thread::continueExecution
 *
 * Puts the thread into the running state.
 */
bool BPatch_thread::continueExecution()
{
    pollForStatusChange();

    return proc->continueProc();
}


/*
 * BPatch_thread::continueExecution
 *
 * Puts the thread into the running state.
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
 * Returns true if the thread is stopped, and false if it is not.
 */
bool BPatch_thread::isStopped()
{
    pollForStatusChange();

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
 * BPatch_thread::isTerminated
 *
 * Returns true if the process has terminated, false if it has not.
 */
bool BPatch_thread::isTerminated()
{
    if (proc == NULL) return true;
    pollForStatusChange();

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
    if (isStopped()) was_stopped = true;
    else was_stopped = false;

    stopExecution();

    bool ret = proc->dumpCore(file);
    if (ret && terminate) {
	terminateExecution();
    } else if (!was_stopped) {
	continueExecution();
    }

    return ret;
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
 * XXX Should return NULL on failure, but the function which it calls,
 *     inferiorMalloc, calls exit rather than returning an error, so this
 *     is not currently possible.
 */
BPatch_variableExpr *BPatch_thread::malloc(int n)
{
    // XXX What to do about the type?
    assert(BPatch::bpatch != NULL);
    return new BPatch_variableExpr(proc,
	    (void *)inferiorMalloc(proc, n, dataHeap),
	    BPatch::bpatch->type_Untyped);
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
    /*
     * XXX For now, the only type that will work is "int."
     */
    void *mem = (void *)inferiorMalloc(proc, sizeof(int), dataHeap);

    /* XXX At least for now, the memory is initially filled with zeroes. */
    int zero = 0;
    proc->writeDataSpace((char *)mem, sizeof(int), (char *)&zero);

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
    vector<unsigVecType> pointsToCheck;	// We'll leave this empty

    inferiorFree(proc, (unsigned)ptr.getAddress(), dataHeap, pointsToCheck);
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
						  const BPatch_point &point,
						  BPatch_callWhen when,
						  BPatch_snippetOrder order)
{
    BPatch_Vector<BPatch_point *> point_vec;

    point_vec.push_back((BPatch_point *)&point);

    return insertSnippet(expr, point_vec, when, order);
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
    // Can't insert code when mutations are not active.
    if (!mutationsActive)
	return NULL;

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

    BPatchSnippetHandle *handle = new BPatchSnippetHandle(proc);

    for (int i = 0; i < points.size(); i++) {
	instPoint *point = (instPoint *)points[i]->point; // Cast away const

	// XXX Really only need to type check once per function the snippet is
	// being inserted into, not necessarily once per point.
	if (expr.ast->checkType() == BPatch::bpatch->type_Error) {
	    // XXX Type check error - should call callback
	    delete handle;
	    return NULL;
	}

	AstNode *ast = (AstNode *)expr.ast;  /* XXX no const */

	// XXX We just pass false for the "noCost" parameter here - do we want
	// to make that an option?
	instInstance *instance; 
	if ((instance =
		addInstFunc(proc,
			    point,
			    ast,
			    _when,
			    _order,
			    false)) != NULL) {
	    handle->add(instance);
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
void BPatchSnippetHandle::add(instInstance *pointInstance)
{
    assert(pointInstance->proc == proc);
    instance.push_back(pointInstance);
}


/*
 * BPatchSnippetHandle::~BPatchSnippetHandle
 *
 * Destructor for BPatchSnippetHandle.  Delete the snippet instance(s)
 * associated with the BPatchSnippetHandle.
 */
BPatchSnippetHandle::~BPatchSnippetHandle()
{
    for (int i = 0; i < instance.size(); i++)
	deleteInst(instance[i], getAllTrampsAtPoint(instance[i]));
}
