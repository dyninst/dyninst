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

#include <stdio.h>
#include <assert.h>
#include <signal.h>

#include "BPatch.h"
#include "BPatch_type.h"
#include "BPatch_libInfo.h"
#include "process.h"

#ifdef i386_unknown_nt4_0
#include "nt_signal_emul.h"
#endif

extern bool dyninstAPI_init();
extern int handleSigChild(int pid, int status);


BPatch *BPatch::bpatch = NULL;


/*
 * BPatch::BPatch
 *
 * Constructor for BPatch.  Performs one-time initialization needed by the
 * library.
 */
BPatch::BPatch() : errorHandler(NULL), typeCheckOn(true)
{
    extern bool init();
    extern double cyclesPerSecond;
    extern double timing_loop(const unsigned, const unsigned);

    // Save a pointer to the one-and-only bpatch object.
    if (bpatch == NULL)
	bpatch = this;
    /* XXX else
     * 	(indicate an error somehow)
     */

    // XXX dyninstAPI_init returns success/failure -- should pass on somehow
    dyninstAPI_init();
    cyclesPerSecond = timing_loop(1, 100000) * 1000000;

    /*
     * Create the library private info object.
     */
    info = new BPatch_libInfo;

    /*
     * Create the "error" and "untyped" types.
     */
    type_Error   = new BPatch_type("<error>", true);
    type_Untyped = new BPatch_type("<no type>", true);

    /*
     * Initialize hash table of standard types.
     */
    stdTypes = new BPatch_typeCollection;
    stdTypes->addType(new BPatch_type("int"));
    stdTypes->addType(new BPatch_type("char *"));
}


/*
 * BPatch::~BPatch
 *
 * Destructor for BPatch.  Free allocated memory.
 */
BPatch::~BPatch()
{
    delete info;

    delete type_Error;
    delete type_Untyped;

    delete stdTypes;

    bpatch = NULL;
}


/*
 * BPatch::registerErrorCallback
 *
 * Registers a function that is to be called by the library when an error
 * occurs or when there is status to report.  Returns the address of the
 * previously registered error callback function.
 *
 * function	The function to be called.
 */
BPatchErrorCallback BPatch::registerErrorCallback(BPatchErrorCallback function)
{
    BPatchErrorCallback ret;

    ret = errorHandler;
    errorHandler = function;

    return ret;
}


#ifdef BPATCH_NOT_YET
/*
 * BPatch::registerDynLibraryCallback
 *
 * Registers a function that is to be called by the library when a dynamically
 * loaded library is loaded or unloaded by a process under the API's control.
 * Returns the address of the previously registered callback function.
 *
 * function	The function to be called.
 */
BPatchDynLibraryCallback
BPatch::registerDynLibraryCallback(BPatchDynLibraryCallback function)
{
    BPatchDynLibraryCallback ret;

    ret = dynLibraryCallback;
    dynLibraryCallback = function;

    return ret;
}
#endif


/*
 * BPatch::getEnglishErrorString
 *
 * Returns the descriptive error string for the passed error number.
 *
 * number	The number that identifies the error.
 */
const char *BPatch::getEnglishErrorString(int /* number */)
{
    return "%s";
}


/*
 * BPatch::reportError
 *
 * Report an error using the callback mechanism.
 *
 * severity	The severity level of the error.
 * number	Identifies the error.
 * str		A string to pass as the first element of the list of strings
 *		given to the callback function.
 */
void BPatch::reportError(BPatchErrorLevel severity, int number, const char *str)
{
    assert(bpatch != NULL);

    if (severity != BPatchInfo)
	bpatch->lastError = number;

    if (bpatch->errorHandler != NULL) {
	bpatch->errorHandler(severity, number, &str);
    }
}


/*
 * BPatch::formatErrorString
 *
 * Takes a format string with an error message (obtained from
 * getEnglishErrorString) and an array of parameters that were passed to an
 * error callback function, and creates a string with the parameters
 * substituted into it.
 *
 * dst		The address into which the formatted string should be copied.
 * size		If the formatted string is equal to or longer than this number
 * 		of characters, then it will be truncated to size-1 characters
 * 		and terminated with a nul ('\0').
 * fmt		The format string (returned by a function such as
 *		getEnglishErrorString).
 * params	The array of parameters that were passed to an error callback
 *		function.
 */
void BPatch::formatErrorString(char *dst, int size,
			       const char *fmt, const char **params)
{
    int cur_param = 0;

    while (size > 1 && *fmt) {
	if (*fmt == '%') {
	    if (fmt[1] == '\0') {
		break;
	    } else if (fmt[1] == '%') {
		*dst++ = '%';
		size--;
	    } else if (fmt[1] == 's') {
		char *p = const_cast<char *>(params[cur_param++]);
		while (size > 1 && *p) {
		    *dst++ = *p++;
		    size--;
		}
	    } else {
		// Illegal specifier
		*dst++ = fmt[0];
		*dst++ = fmt[1];
		size -= 2;
	    }
    	    fmt += 2;
	} else {
	    *dst++ = *fmt++;
	    size--;
	}
    }
    if (size > 0)
	*dst = '\0';
}


/*
 * BPatch::getThreadByPid
 *
 * Given a process ID, this function returns a pointer to the associated
 * BPatch_thread object (or NULL if there is none).  Since a process may be
 * registered provisionally with a thread object pointer of NULL, the boolean
 * pointed to by the parameter "exists" is set to true if the pid exists in
 * the table of processes, and false if it does not.
 *
 * pid		The pid to look up.
 * exists	A pointer to a boolean to fill in with true if the pid exists
 *		in the table and false if it does not.  NULL may be passed in
 *		if this information is not required.
 */
BPatch_thread *BPatch::getThreadByPid(int pid, bool *exists)
{
    if (info->threadsByPid.defines(pid)) {
	if (exists) *exists = true;
	return info->threadsByPid[pid];
    } else {
	if (exists) *exists = false;
    	return NULL;
    }
}


/*
 * BPatch::getThreads
 *
 * Returns a vector of all threads that are currently defined.  Includes
 * threads created directly using the library and those created with UNIX fork
 * or Windows NT spawn system calls.  The caller is responsible for deleting
 * the vector when it is no longer needed.
 */
BPatch_Vector<BPatch_thread *> *BPatch::getThreads()
{
    BPatch_Vector<BPatch_thread *> *result = new BPatch_Vector<BPatch_thread *>;

    dictionary_hash_iter<int, BPatch_thread *> ti(info->threadsByPid);

    int pid;
    BPatch_thread *thread;

    while (ti.next(pid, thread))
    	result->push_back(thread);

    return result;
}


/*
 * BPatch::registerProvisionalThread
 *
 * Register a new process that is not yet associated with a thread.
 * (this function is called only by createProcess).
 *
 * pid		The pid of the process to register.
 */
void BPatch::registerProvisionalThread(int pid)
{
    assert(!info->threadsByPid.defines(pid));
    info->threadsByPid[pid] = NULL;
}


/*
 * BPatch::registerThread
 *
 * Register a new BPatch_thread object with the BPatch library (this function
 * is called only by the constructor for BPatch_thread).
 *
 * thread	A pointer to the thread to register.
 */
void BPatch::registerThread(BPatch_thread *thread)
{
    assert(!info->threadsByPid.defines(thread->getPid()) ||
	    info->threadsByPid[thread->getPid()] == NULL);
    info->threadsByPid[thread->getPid()] = thread;
}


/*
 * BPatch::unRegisterThread
 *
 * Remove the BPatch_thread associated with a given pid from the list of
 * threads being managed by the library.
 *
 * pid		The pid of the thread to be removed.
 */
void BPatch::unRegisterThread(int pid)
{
    assert(info->threadsByPid.defines(pid));
    info->threadsByPid.undef(pid);	
}


/*
 * BPatch::createProcess
 *
 * Create a process and return a BPatch_thread representing it.
 * Returns NULL upon failure.
 *
 * path		The pathname of the executable for the new process.
 * argv		A list of the arguments for the new process, terminated by a
 *		NULL.
 * envp		A list of values that make up the environment for the new
 *		process, terminated by a NULL.  If envp is NULL, the new
 *		new process will inherit the environemnt of the parent.
 */
BPatch_thread *BPatch::createProcess(char *path, char *argv[], char *envp[])
{
    clearError();

    BPatch_thread *ret = new BPatch_thread(path, argv, envp);

    if (!ret->proc ||
       (ret->proc->status() != stopped) ||
       !ret->proc->isBootstrappedYet()) {
	delete ret;
	return NULL;
    }

    return ret;
}


/*
 * BPatch::attachProcess
 *
 * Attach to a running pprocess and return a BPatch_thread representing it.
 * Returns NULL upon failure.
 *
 * path		The pathname of the executable for the process.
 * pid		The id of the process to attach to.
 */
BPatch_thread *BPatch::attachProcess(char *path, int pid)
{
    clearError();

    BPatch_thread *ret = new BPatch_thread(path, pid);

    if (getLastError()) {
	delete ret;
	return NULL;
    }

    return ret;
}


/*
 * getThreadEvent
 *
 * Checks for changes in any child process, and optionally blocks until such a
 * change has occurred.  Also updates the process object representing each
 * process for which a change is detected.  The return value is true if a
 * change was detected, otherwise it is false.
 *
 * block	Set this parameter to true to block waiting for a change,
 * 		set to false to poll and return immediately, whether or not a
 * 		change occurred.
 */
bool BPatch::getThreadEvent(bool block)
{
    bool	result = false;
    int		pid, status;

    // while ((pid = process::waitProcs(&status, block)) > 0) {
    if ((pid = process::waitProcs(&status, block)) > 0) {
	// There's been a change in a child process
	result = true;
	// Since we found something, we don't want to block anymore
	block = false;

	bool exists;
	BPatch_thread *thread = getThreadByPid(pid, &exists);
	if (thread == NULL) {
	    if (exists) {
		if (WIFSIGNALED(status) || WIFEXITED(status))
		    unRegisterThread(pid);
	    } else {
    		fprintf(stderr, "Warning - wait returned status of an unknown process (%d)\n", pid);
	    }
	}
	if (thread != NULL) {
	    if (WIFSTOPPED(status)) {
    		thread->lastSignal = WSTOPSIG(status);
		thread->setUnreportedStop(true);
	    } else if (WIFSIGNALED(status)) {
		thread->lastSignal = WTERMSIG(status);
		thread->setUnreportedTermination(true);
	    } else if (WIFEXITED(status)) {
		thread->lastSignal = 0; /* XXX Make into some constant */
		thread->setUnreportedTermination(true);
	    }
	}
#ifndef i386_unknown_nt4_0
	handleSigChild(pid, status);
#endif
    }

    return result;
}


/*
 * havePendingEvent
 *
 * Returns true if any thread has stopped or terminated and that fact hasn't
 * been reported to the user of the library.  Otherwise, returns false.
 */
bool BPatch::havePendingEvent()
{
#ifdef i386_unknown_nt4_0
    // On NT, we need to poll for events as often as possible, so that we can
    // handle traps.
    if (getThreadEvent(false))
	return true;
#endif

    // For now, we'll do it by iterating over the threads and checking them,
    // and we'll change it to something more efficient later on.
    dictionary_hash_iter<int, BPatch_thread *> ti(info->threadsByPid);

    int pid;
    BPatch_thread *thread;

    while (ti.next(pid, thread)) {
	if (thread != NULL &&
	    (thread->pendingUnreportedStop() ||
	     thread->pendingUnreportedTermination())) {
	    return true;
	}
    }

    return false;
}


/*
 * pollForStatusChange
 *
 * Checks for unreported changes to the status of any child process, and
 * returns true if any are detected.  Returns false otherwise.
 *
 * This function is declared as a friend of BPatch_thread so that it can use
 * the BPatch_thread::getThreadEvent call to check for status changes.
 */
bool BPatch::pollForStatusChange()
{
    if (havePendingEvent())
	return true;
  
    // No changes were previously detected, so check for new changes
    return getThreadEvent(false);
}


/*
 * waitForStatusChange
 *
 * Blocks waiting for a change to occur in the running status of a child
 * process.  Returns true upon success, false upon failure.
 *
 * This function is declared as a friend of BPatch_thread so that it can use
 * the BPatch_thread::getThreadEvent call to check for status changes.
 */
bool BPatch::waitForStatusChange()
{
    if (havePendingEvent())
	return true;

    // No changes were previously detected, so wait for a new change
    return getThreadEvent(true);
}
