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
/*
 * $Log: BPatch_thread.C,v $
 * Revision 1.1  1997/03/18 19:44:03  buck
 * first commit of dyninst library.  Also includes:
 * 	moving templates from paradynd to dyninstAPI
 * 	converting showError into a function (in showerror.C)
 * 	many ifdefs for BPATCH_LIBRARY in dyinstAPI/src.
 *
 *
 */

#include "inst.h"
#include "process.h"

#include "BPatch_thread.h"

extern int dyninstAPI_handleSigChild(int pid, int status);
extern process *dyninstAPI_createProcess(const string File,
	vector<string> argv, vector<string> envp, const string dir = "");
extern process *dyninstAPI_attachProcess(const string &progpath, int pid,
	int afterAttach);
extern void BPatch_init();

BPatch_Vector<BPatch_thread*> BPatch_thread::threadVec;
bool BPatch_thread::lib_inited = FALSE;


/*
 * BPatch_init
 *
 * Initializes the dyninstAPI lirbary.
 * XXX This function really doesn't belong here.  It will be moved out as soon
 *     as we create the BPatch class that represents the library (it will be
 *     moved to the constructor for that class).
 */
void BPatch_init()
{
    extern bool init();
    extern double cyclesPerSecond;
    extern double timing_loop(const unsigned, const unsigned);

    init();
    cyclesPerSecond = timing_loop(1, 100000) * 1000000;
}


/*
 * pollForStatusChange
 *
 * Checks for changes in the state of any child process, and returns true if
 * it discovers any such changes.  Also updates the process object
 * representing each process for which a change is detected.
 *
 * This function is declared as a friend of BPatch_thread so that it can use
 * the BPatch_thread::pidToThread call and so that it can set the lastSignal
 * member of a BPatch_thread object.
 */
bool pollForStatusChange()
{
    bool	result = false;
    int		pid, status;

    while ((pid = process::waitProcs(&status)) > 0) {
	// There's been a change in a child process
	result = true;
	BPatch_thread *thread = BPatch_thread::pidToThread(pid);
	assert(thread != NULL);
	if (thread != NULL) {
	    if (WIFSTOPPED(status))
    		thread->lastSignal = WSTOPSIG(status);
	    else if (WIFSIGNALED(status))
		thread->lastSignal = WTERMSIG(status);
	    else if (WIFEXITED(status))
		thread->lastSignal = 0; /* XXX Make into some constant */
	}
	dyninstAPI_handleSigChild(pid, status);
    }
    return result;
}


/*
 * static BPatch_thread::pidToThread
 *
 * Given a process ID, this function returns a pointer to the associated
 * BPatch_thread object (or NULL if there is none).
 */
BPatch_thread *BPatch_thread::pidToThread(int pid)
{
    for (int i = 0; i < threadVec.size(); i++)
	if (threadVec[i]->getPid() == pid) return threadVec[i];

    return NULL;
}


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
    : lastSignal(-1)
{
    if (!lib_inited) {
	BPatch_init();
	lib_inited = TRUE;
    }

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

    proc = dyninstAPI_createProcess(path, argv_vec, envp_vec, "");

    // XXX Should do something more sensible.
    if (proc == NULL) return;

    // Add this object to the list of threads
    // XXX Should be conditional on success of creating process
    threadVec.push_back(this);

    image = new BPatch_image(proc);
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
    : lastSignal(-1)
{
    if (!lib_inited) {
	BPatch_init();
	lib_inited = TRUE;
    }

    proc = dyninstAPI_attachProcess(path, pid, 1);

    // XXX Should do something more sensible
    if (proc == NULL) return;

    // Add this object to the list of threads
    threadVec.push_back(this);

    image = new BPatch_image(proc);
}


/*
 * BPatch_thread::~BPatch_thread
 *
 * Destructor for BPatch_thread.  Detaches from the running thread.
 */
BPatch_thread::~BPatch_thread()
{
    // Detach from the thread
    proc->detach(FALSE);

    // XXX Should also deallocate memory and remove process ID from
    //     the map of process IDs to thread objects.
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
    return P_kill(getPid(), SIGKILL);
}


/*
 * BPatch_thread::isStopped
 *
 * Returns true if the thread is stopped, and false if it is not.
 */
bool BPatch_thread::isStopped()
{
    pollForStatusChange();

    return proc->status() == neonatal || proc->status() == stopped;
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
    return new BPatch_variableExpr((void *)inferiorMalloc(proc, n, dataHeap));
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
     * XXX For now, the only type supported is "int."
     */
    void *mem = (void *)inferiorMalloc(proc, sizeof(int), dataHeap);

    /* XXX At least for now, the memory is initially filled with zeroes. */
    int zero = 0;
    proc->writeDataSpace((char *)mem, sizeof(int), (char *)&zero);

    return new BPatch_variableExpr(mem);
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
 * Insert a code snippet at a given instrumentation point.
 *
 * expr		The snippet to insert.
 * point	The point at which to insert it.
 */
bool BPatch_thread::insertSnippet(const BPatch_snippet &expr,
				  const BPatch_point &point,
				  BPatch_callWhen when,
				  BPatch_snippetOrder order)
{
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
	return false;
    };

    switch (order) {
      case BPatch_firstSnippet:
	_order = orderFirstAtPoint;
	break;
      case BPatch_lastSnippet:
	_order = orderLastAtPoint;
	break;
      default:
	return false;
    }

    // XXX We just pass false for the "noCost" parameter here - do we want to
    // make that an option?
    if (addInstFunc(proc,
		    ((BPatch_point)point).point, /* XXX Cast away const */
		    ((BPatch_snippet)expr).ast,  /* XXX Cast away const */
		    _when,
		    _order,
		    false) != NULL)
	return true;
    else
	return false;
}


/*
 * BPatch_thread::insertSnippet
 *
 * Insert a code snippet at each of a list of instrumentation points.
 *
 * expr		The snippet to insert.
 * points	The list of points at which to insert it.
 */
bool BPatch_thread::insertSnippet(const BPatch_snippet &expr,
				  const BPatch_Vector<BPatch_point *> &points,
				  BPatch_callWhen when,
				  BPatch_snippetOrder order)
{
    /*
     * XXX We should change this so either all instrumentation gets inserted
     * or none of it does.
     */
    for (int i = 0; i < points.size(); i++) {
	if (!insertSnippet(expr, *points[i], when, order))
	    return false;
    }

    return true;
}
