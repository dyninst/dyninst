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

// $Id: BPatch.C,v 1.37 2001/08/01 15:39:54 chadd Exp $

#include <stdio.h>
#include <assert.h>
#include <signal.h>

#define BPATCH_FILE
#include "BPatch.h"
#include "BPatch_libInfo.h"
#include "process.h"
#include "BPatch_collections.h"
#include "common/h/timing.h"

#if defined(i386_unknown_nt4_0) || defined(mips_unknown_ce2_11) //ccw 20 july 2000 : 28 mar 2001
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
BPatch::BPatch()
  : info(NULL),
    errorHandler(NULL),
    dynLibraryCallback(NULL),
    typeCheckOn(true),
    lastError(0),
    debugParseOn(true),
    trampRecursiveOn(false),
    forceRelocation_NP(false),
    builtInTypes(NULL),
    stdTypes(NULL),
    type_Error(NULL),
    type_Untyped(NULL)
{
    extern bool init();

    // Save a pointer to the one-and-only bpatch object.
    if (bpatch == NULL){
	bpatch = this;
#ifdef mips_unknown_ce2_11 //ccw 10 aug 2000 : 28 mar 2001
	rDevice = new remoteDevice(); //ccw 8 aug 2000
#endif
	}

    /* XXX else
     * 	(indicate an error somehow)
     */

    // XXX dyninstAPI_init returns success/failure -- should pass on somehow
    dyninstAPI_init();
    initCyclesPerSecond();

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
    stdTypes->addType(new BPatch_type("int",-1, BPatch_scalar, sizeof(int)));
    stdTypes->addType(new BPatch_type("char *",-3, BPatch_scalar, sizeof(char*)));
    BPatch_type *voidType = new BPatch_type("void",-11, BPatch_scalar, 0);
    stdTypes->addType(voidType);
    stdTypes->addType(new BPatch_type("void *",-4, BPatch_pointer, voidType));
    stdTypes->addType(new BPatch_type("float",-12, BPatch_scalar, sizeof(float)));

    /*
     * Initialize hash table of API types.
     */
    APITypes = new BPatch_typeCollection;

    /*
     *  Initialize hash table of Built-in types.
     *  Negative type numbers defined in the gdb stab-docs
     */
     
    builtInTypes = new BPatch_builtInTypeCollection;
    
    // NOTE: integral type  mean twos-complement
    // -1  int, 32 bit signed integral type
    // in stab document, size specified in bits, system size is in bytes
    builtInTypes->addBuiltInType(new BPatch_type("int",-1, BPatch_built_inType,
						 4));
    // -2  char, 8 bit type holding a character. GDB & dbx(AIX) treat as signed
    builtInTypes->addBuiltInType(new BPatch_type("char",-2,
						 BPatch_built_inType, 1));
    // -3  short, 16 bit signed integral type
    builtInTypes->addBuiltInType(new BPatch_type("short",-3,
						 BPatch_built_inType, 2));
    // -4  long, 32/64 bit signed integral type
    builtInTypes->addBuiltInType(new BPatch_type("long",-4,
						 BPatch_built_inType, 
						 sizeof(long)));
    // -5  unsigned char, 8 bit unsigned integral type
    builtInTypes->addBuiltInType(new BPatch_type("unsigned char",-5,
						 BPatch_built_inType, 1));
    // -6  signed char, 8 bit signed integral type
    builtInTypes->addBuiltInType(new BPatch_type("signed char",-6,
						 BPatch_built_inType, 1));
    // -7  unsigned short, 16 bit unsigned integral type
    builtInTypes->addBuiltInType(new BPatch_type("unsigned short",-7,
						 BPatch_built_inType, 2));
    // -8  unsigned int, 32 bit unsigned integral type
    builtInTypes->addBuiltInType(new BPatch_type("unsigned int",-8,
						 BPatch_built_inType, 4));
    // -9  unsigned, 32 bit unsigned integral type
    builtInTypes->addBuiltInType(new BPatch_type("unsigned",-9,
						 BPatch_built_inType,4));
    // -10 unsigned long, 32 bit unsigned integral type
    builtInTypes->addBuiltInType(new BPatch_type("unsigned long",-10,
						 BPatch_built_inType, 
						 sizeof(unsigned long)));
    // -11 void, type indicating the lack of a value
    //  XXX-size may not be correct jdd 4/22/99
    builtInTypes->addBuiltInType(new BPatch_type("void",-11,
						 BPatch_built_inType,
						 0));
    // -12 float, IEEE single precision
    builtInTypes->addBuiltInType(new BPatch_type("float",-12,
						 BPatch_built_inType,
						 sizeof(float)));
    // -13 double, IEEE double precision
    builtInTypes->addBuiltInType(new BPatch_type("double",-13,
						 BPatch_built_inType,
						 sizeof(double)));
    // -14 long double, IEEE double precision, size may increase in future
    builtInTypes->addBuiltInType(new BPatch_type("long double",-14,
						 BPatch_built_inType,
						 sizeof(long double)));
    // -15 integer, 32 bit signed integral type
    builtInTypes->addBuiltInType(new BPatch_type("integer",-15,
						 BPatch_built_inType, 4));
    // -16 boolean, 32 bit type. GDB/GCC 0=False, 1=True, all other values
    //     have unspecified meaning
    builtInTypes->addBuiltInType(new BPatch_type("boolean",-16,
						 BPatch_built_inType, 4));
    // -17 short real, IEEE single precision
    //  XXX-size may not be correct jdd 4/22/99
    builtInTypes->addBuiltInType(new BPatch_type("short real",-17,
						 BPatch_built_inType,
						 sizeof(float)));
    // -18 real, IEEE double precision XXX-size may not be correct jdd 4/22/99 
    builtInTypes->addBuiltInType(new BPatch_type("real",-18,
						 BPatch_built_inType,
						 sizeof(double)));
    // -19 stringptr XXX- size of void * -- jdd 4/22/99
    builtInTypes->addBuiltInType(new BPatch_type("stringptr",-19,
						 BPatch_built_inType,
						 sizeof(void *)));
    // -20 character, 8 bit unsigned character type
    builtInTypes->addBuiltInType(new BPatch_type("character",-20,
						 BPatch_built_inType, 1));
    // -21 logical*1, 8 bit type (Fortran, used for boolean or unsigned int)
    builtInTypes->addBuiltInType(new BPatch_type("logical*1",-21,
						 BPatch_built_inType, 1));
    // -22 logical*2, 16 bit type (Fortran, some for boolean or unsigned int)
    builtInTypes->addBuiltInType(new BPatch_type("logical*2",-22,
						 BPatch_built_inType, 2));
    // -23 logical*4, 32 bit type (Fortran, some for boolean or unsigned int)
    builtInTypes->addBuiltInType(new BPatch_type("logical*4",-23,
						 BPatch_built_inType, 4));
    // -24 logical, 32 bit type (Fortran, some for boolean or unsigned int)
    builtInTypes->addBuiltInType(new BPatch_type("logical",-24,
						 BPatch_built_inType, 4));
    // -25 complex, consists of 2 IEEE single-precision floating point values
    builtInTypes->addBuiltInType(new BPatch_type("complex",-25,
						 BPatch_built_inType,
						 (sizeof(float)*2)));
    // -26 complex, consists of 2 IEEE double-precision floating point values
    builtInTypes->addBuiltInType(new BPatch_type("complex",-26,
						 BPatch_built_inType,
						 (sizeof(double)*2)));
    // -27 integer*1, 8 bit signed integral type
    builtInTypes->addBuiltInType(new BPatch_type("integer*1",-27,
						 BPatch_built_inType, 1));
    // -28 integer*2, 16 bit signed integral type
    builtInTypes->addBuiltInType(new BPatch_type("integer*2",-28,
						 BPatch_built_inType, 2));
    // -29 integer*4, 32 bit signed integral type
    builtInTypes->addBuiltInType(new BPatch_type("integer*4",-29,
						 BPatch_built_inType, 4));
    // -30 wchar, Wide character, 16 bits wide, unsigned (unknown format)
    builtInTypes->addBuiltInType(new BPatch_type("wchar",-30,
						 BPatch_built_inType, 2));
    // -31 long long, 64 bit signed integral type
    builtInTypes->addBuiltInType(new BPatch_type("long long",-31,
						 BPatch_built_inType, 8));
    // -32 unsigned long long, 64 bit unsigned integral type
    builtInTypes->addBuiltInType(new BPatch_type("unsigned long long", -32,
						 BPatch_built_inType, 8));
    // -33 logical*8, 64 bit unsigned integral type
    builtInTypes->addBuiltInType(new BPatch_type("logical*8",-33,
						 BPatch_built_inType, 8));
    // -34 integer*8, 64 bit signed integral type
    builtInTypes->addBuiltInType(new BPatch_type("integer*8",-34,
						 BPatch_built_inType, 8));

    // default callbacks are null
    postForkCallback = NULL;
    preForkCallback = NULL;
    errorHandler = NULL;
    dynLibraryCallback = NULL;
    execCallback = NULL;
    exitCallback = NULL;

#ifdef DETACH_ON_THE_FLY
    // Register handler for notification from detached inferiors
    extern void initDetachOnTheFly();
    initDetachOnTheFly();
#endif
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


/*
 * BPatch::registerPostForkCallback
 *
 * Registers a function that is to be called by the library when a new
 * process has been forked off by an mutatee process.
 *
 * function	The function to be called.
 */
BPatchForkCallback BPatch::registerPostForkCallback(BPatchForkCallback func)
{

#if !defined(sparc_sun_solaris2_4) && \
    !defined(i386_unknown_solaris2_5) && \
    !defined(alpha_dec_osf4_0) && \
    !defined(mips_sgi_irix6_4)
    reportError(BPatchWarning, 0,
	"postfork callbacks not implemented on this platform\n");
    return NULL;
#else
    BPatchForkCallback ret;

    ret = postForkCallback;
    postForkCallback = func;

    return ret;
#endif
}

/*
 * BPatch::registerPreForkCallback
 *
 * Registers a function that is to be called by the library when a process
 * is about to fork a new process
 *
 * function	The function to be called.
 */
BPatchForkCallback BPatch::registerPreForkCallback(BPatchForkCallback func)
{
#if !defined(sparc_sun_solaris2_4) && \
    !defined(i386_unknown_solaris2_5) &&\
    !defined(alpha_dec_osf4_0) && \
    !defined(mips_sgi_irix6_4)
    reportError(BPatchWarning, 0,
	"prefork callbacks not implemented on this platform\n");
    return NULL;
#else
    BPatchForkCallback ret;

    ret = preForkCallback;
    preForkCallback = func;

    return ret;
#endif
}

/*
 * BPatch::registerExecCallback
 *
 * Registers a function that is to be called by the library when a 
 * process has just completed an exec* call
 *
 * func	The function to be called.
 */
BPatchExecCallback BPatch::registerExecCallback(BPatchExecCallback func)
{

#if !defined(sparc_sun_solaris2_4) && \
    !defined(i386_unknown_solaris2_5) &&\
    !defined(alpha_dec_osf4_0) && \
    !defined(mips_sgi_irix6_4)
    reportError(BPatchWarning, 0,
	"exec callbacks not implemented on this platform\n");
    return NULL;
#else
    BPatchExecCallback ret;

    ret = execCallback;
    execCallback = func;

    return ret;
#endif
}

/*
 * BPatch::registerExecCallback
 *
 * Registers a function that is to be called by the library when a 
 * process has just called the exit system call
 *
 * func	The function to be called.
 */
BPatchExitCallback BPatch::registerExitCallback(BPatchExitCallback func)
{

#if !defined(sparc_sun_solaris2_4) && \
    !defined(i386_unknown_solaris2_5) &&\
    !defined(alpha_dec_osf4_0) && \
    !defined(mips_sgi_irix6_4)
    reportError(BPatchWarning, 0,
	"exit callbacks not implemented on this platform\n");
    return NULL;
#else
    BPatchExitCallback ret;

    ret = exitCallback;
    exitCallback = func;

    return ret;
#endif
}

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
  
        // don't log BPatchWarning or BPatchInfo messages as "errors"
    if ((severity == BPatchFatal) || (severity == BPatchSerious))
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
 * BPatch::registerForkedThread
 *
 * Register a new process that is not yet associated with a thread.
 * (this function is an upcall when a new process is created).
 *
 * parentPid		the pid of the parent process.
 * childPid		The pid of the process to register.
 * proc			lower lever handle to process specific stuff
 *
 */
void BPatch::registerForkedThread(int parentPid, int childPid, process *proc)
{
    assert(!info->threadsByPid.defines(childPid));

    BPatch_thread *parent = info->threadsByPid[parentPid];

    assert(parent);
    info->threadsByPid[childPid] = new BPatch_thread(childPid, proc);

    if (postForkCallback) {
	postForkCallback(parent, info->threadsByPid[childPid]);
    }
}


/*
 * BPatch::registerExec
 *
 * Register a process that has just done an exec call.
 *
 * thread	thread that has just performed the exec
 *
 */
void BPatch::registerExec(BPatch_thread *thread)
{
    // build a new BPatch_image for this one
    thread->image = new BPatch_image(thread->proc);

    if (execCallback) {
	execCallback(thread);
    }
}


/*
 * BPatch::registerExit
 *
 * Register a process that has just done an exit call.
 *
 * thread	thread that has just performed the exec
 * code		the exit status code
 *
 */
void BPatch::registerExit(BPatch_thread *thread, int code)
{
    if (exitCallback) {
	exitCallback(thread, code);
    }
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
 * stdin_fd	file descriptor to use for stdin for the application
 * stdout_fd	file descriptor to use for stdout for the application
 * stderr_fd	file descriptor to use for stderr for the application

 */
BPatch_thread *BPatch::createProcess(char *path, char *argv[], 
	char *envp[], int stdin_fd, int stdout_fd, int stderr_fd)
{
    clearError();

    BPatch_thread *ret = 
	new BPatch_thread(path, argv, envp, stdin_fd, stdout_fd, stderr_fd);

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
 * Attach to a running process and return a BPatch_thread representing it.
 * Returns NULL upon failure.
 *
 * path		The pathname of the executable for the process.
 * pid		The id of the process to attach to.
 */
BPatch_thread *BPatch::attachProcess(char *path, int pid)
{
    clearError();

    BPatch_thread *ret = new BPatch_thread(path, pid);

    if (!ret->proc ||
       (ret->proc->status() != stopped) ||
       !ret->proc->isBootstrappedYet()) {
        // It would be considerate to (attempt to) leave the process running
        // at this point (*before* deleting the BPatch_thread handle for it!),
        // even though it might be in bad shape after the attempted attach.
        char msg[256];
        sprintf(msg,"attachProcess failed: process %d may now be killed!",pid);
        reportError(BPatchWarning, 26, msg);
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
#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11) //ccw 20 july 2000 : 28 mar 2001
                thread->proc->exitCode_ = WEXITSTATUS(status);
#endif
                thread->exitCode = thread->proc->exitCode();
		thread->lastSignal = 0; /* XXX Make into some constant */
		thread->setUnreportedTermination(true);
	    }
	}
#if !(defined i386_unknown_nt4_0) && !(defined mips_unknown_ce2_11) //ccw 20 july 2000 : 28 mar 2001
	handleSigChild(pid, status);
#ifdef notdef
	if (thread->lastSignal == SIGSTOP) {
	    // need to continue process after initial sigstop
	    // thread->continueExecution();
	    printf("BPatch past handleSigChild for SIGSTOP\n");
	    if (thread->proc->wasCreatedViaFork()) {
		printf("marking forked process stopped\n");
		thread->proc->status_ = stopped;
    		// thread->lastSignal = SIGSTOP;
		// thread->setUnreportedStop(true);
		// thread->proc->continueProc();
	    }
	}
#endif
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
#if defined(i386_unknown_nt4_0) || defined(mips_unknown_ce2_11) //ccw 20 july 2001 : 28 mar 2001
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

/*
 * createEnum
 *
 * This function is a wrapper for the BPatch_type constructors for API/User
 * created types.
 *
 * It returns a pointer to a BPatch_type that was added to the APITypes
 * collection.
 */
BPatch_type * BPatch::createEnum( const char * name, 
				  BPatch_Vector<char *> elementNames,
				  BPatch_Vector<int> elementIds)
{

    if (elementNames.size() != elementIds.size()) {
      return NULL;
    }

    BPatch_type * newType = new BPatch_type( name, BPatch_enumerated );
    if (!newType) return NULL;
    
    APITypes->addType(newType);

    // ADD components to type
    for (int i=0; i < elementNames.size(); i++) {
        newType->addField(elementNames[i], BPatch_scalar, elementIds[i]);
    }

    return(newType);
}


/*
 * createEnum
 *
 * This function is a wrapper for the BPatch_type constructors for API/User
 * created types.  The user has left element id specification to us
 *
 * It returns a pointer to a BPatch_type that was added to the APITypes
 * collection.
 */
BPatch_type * BPatch::createEnum( const char * name, 
				  BPatch_Vector<char *> elementNames)
{
    BPatch_type * newType = new BPatch_type( name, BPatch_enumerated );

    if (!newType) return NULL;
    
    APITypes->addType(newType);

    // ADD components to type
    for (int i=0; i < elementNames.size(); i++) {
        newType->addField(elementNames[i], BPatch_scalar, i);
    }

    return(newType);
}

/*
 * createStructs
 *
 * This function is a wrapper for the BPatch_type constructors for API/User
 * created types.
 *
 * It returns a pointer to a BPatch_type that was added to the APITypes
 * collection.
 */

BPatch_type * BPatch::createStruct( const char * name,
				    BPatch_Vector<char *> fieldNames,
				    BPatch_Vector<BPatch_type *> fieldTypes)
{
    int i;
    int offset, size;

    offset = size = 0;
    if (fieldNames.size() != fieldTypes.size()) {
      return NULL;
    }

    //Compute the size of the struct
    for (i=0; i < fieldNames.size(); i++) {
        BPatch_type *type = fieldTypes[i];
        size = type->getSize();
        size += size;
    }
  
    BPatch_type *newType = new BPatch_type(name, BPatch_structure, size);
    if (!newType) return NULL;
    
    APITypes->addType(newType);

    // ADD components to type
    size = 0;
    for (i=0; i < fieldNames.size(); i++) {
        BPatch_type *type = fieldTypes[i];
        size = type->getSize();
        newType->addField(fieldNames[i], type->getDataClass(), type, offset, size);
  
        // Calculate next offset (in bits) into the struct
        offset += (size * 8);
    }

    return(newType);
}

/*
 * createUnions
 *
 * This function is a wrapper for the BPatch_type constructors for API/User
 * created types.
 *
 * It returns a pointer to a BPatch_type that was added to the APITypes
 * collection.
 */

BPatch_type * BPatch::createUnion( const char * name, 
				   BPatch_Vector<char *> fieldNames,
				   BPatch_Vector<BPatch_type *> fieldTypes)
{
    int i;
    int offset, size, newsize;
    offset = size = newsize = 0;

    if (fieldNames.size() != fieldTypes.size()) {
        return NULL;
    }

    // Compute the size of the union
    for (i=0; i < fieldTypes.size(); i++) {
	BPatch_type *type = fieldTypes[i];
	newsize = type->getSize();
	if(size < newsize) size = newsize;
    }
  
    BPatch_type * newType = new BPatch_type(name, BPatch_union, size);
    if (!newType) return NULL;

    APITypes->addType(newType);

    // ADD components to type
    for (i=0; i < fieldNames.size(); i++) {
        BPatch_type *type = fieldTypes[i];
	size = type->getSize();
	newType->addField(fieldNames[i], type->getDataClass(), type, offset, size);
    }  
    return(newType);
}


/*
 * createArray for Arrays and SymTypeRanges
 *
 * This function is a wrapper for the BPatch_type constructors for API/User
 * created types.
 *
 * It returns a pointer to a BPatch_type that was added to the APITypes
 * collection.
 */
BPatch_type * BPatch::createArray( const char * name, BPatch_type * ptr,
				   unsigned int low, unsigned int hi)
{

    BPatch_type * newType;

    if (!ptr) {
        return NULL;
    } else {
        newType = new BPatch_type(name, BPatch_array , ptr, low, hi);
        if (!newType) return NULL;
    }

    APITypes->addType(newType);

    return newType;
}

/*
 * createPointer for BPatch_pointers
 *
 * This function is a wrapper for the BPatch_type constructors for API/User
 * created types.
 *
 * It returns a pointer to a BPatch_type that was added to the APITypes
 * collection.
 */
BPatch_type * BPatch::createPointer(const char * name, BPatch_type * ptr,
				    int size)
{

    BPatch_type * newType = new BPatch_type(name, ptr, size);
    if(!newType) return NULL;

    APITypes->addType(newType);
  
    return newType;
}

/*
 * createScalar for scalars with a size and no range
 *
 * This function is a wrapper for the BPatch_type constructors for API/User
 * created types.
 *
 * It returns a pointer to a BPatch_type that was added to the APITypes
 * collection.
 */

BPatch_type * BPatch::createScalar( const char * name, int size)
{
    BPatch_type * newType = new BPatch_type(name, BPatch_scalar, size);
    if (!newType) return NULL;

    APITypes->addType(newType);
 
    return newType;
}

/*
 * createType for typedefs
 *
 * This function is a wrapper for the BPatch_type constructors for API/User
 * created types.
 *
 * It returns a pointer to a BPatch_type that was added to the APITypes
 * collection.
 */
BPatch_type * BPatch::createTypedef( const char * name, BPatch_type * ptr)
{
    BPatch_type * newType = new BPatch_type(name, ptr);

    if (!newType) return NULL;

    APITypes->addType(newType);
  
    return newType;
}
