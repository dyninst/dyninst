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

// $Id: BPatch.C,v 1.85 2005/02/02 17:27:12 bernat Exp $

#include <stdio.h>
#include <assert.h>
#include <signal.h>

#define BPATCH_FILE
#include "signalhandler.h"
#include "stats.h"
#include "BPatch.h"
#include "BPatch_typePrivate.h"
#include "BPatch_libInfo.h"
#include "process.h"
#include "BPatch_collections.h"
#include "common/h/timing.h"

#if defined(i386_unknown_nt4_0) || defined(mips_unknown_ce2_11) //ccw 20 july 2000 : 28 mar 2001
#include "nt_signal_emul.h"
#endif

extern bool dyninstAPI_init();
extern void loadNativeDemangler();

BPatch *BPatch::bpatch = NULL;

//  global stat vars defined in stats.C
extern unsigned int trampBytes;
extern unsigned int pointsUsed;
extern unsigned int insnGenerated;
extern unsigned int totalMiniTramps;
extern unsigned int ptraceOtherOps;
extern unsigned int ptraceOps;
extern unsigned int ptraceBytes;


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
    baseTrampDeletionOn(false),
    trampRecursiveOn(false),
    forceRelocation_NP(false),
    autoRelocation_NP(true),
    delayedParsing_(false),
    builtInTypes(NULL),
    stdTypes(NULL),
    type_Error(NULL),
    type_Untyped(NULL)
{
    memset(&stats, 0, sizeof(BPatch_stats));
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
    type_Error   = BPatch_type::createFake("<error>");
    type_Untyped = BPatch_type::createFake("<no type>");

    /*
     * Initialize hash table of standard types.
     */
    BPatch_type *newType;
    stdTypes = new BPatch_typeCollection;
    stdTypes->addType(newType = new BPatch_typeScalar(-1, sizeof(int), "int"));
    newType->decrRefCount();
    BPatch_type *charType = new BPatch_typeScalar(-2, sizeof(char), "char");
    stdTypes->addType(charType);
    stdTypes->addType(newType = new BPatch_typePointer(-3, charType, "char *"));
    charType->decrRefCount();
    newType->decrRefCount();
    BPatch_type *voidType = new BPatch_typeScalar(-11, 0, "void");
    stdTypes->addType(voidType);
    stdTypes->addType(newType = new BPatch_typePointer(-4, voidType, "void *"));
    voidType->decrRefCount();
    newType->decrRefCount();
    stdTypes->addType(newType = new BPatch_typeScalar(-12, sizeof(float), "float"));
    newType->decrRefCount();
#if defined(i386_unknown_nt4_0)
    stdTypes->addType(newType = new BPatch_typeScalar(-31, sizeof(LONGLONG), "long long"));    
#else
    stdTypes->addType(newType = new BPatch_typeScalar(-31, sizeof(long long), "long long"));
#endif
    newType->decrRefCount();

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
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-1, 4, "int"));
    newType->decrRefCount();
    // -2  char, 8 bit type holding a character. GDB & dbx(AIX) treat as signed
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-2, 1, "char"));
    newType->decrRefCount();
    // -3  short, 16 bit signed integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-3, 2, "short"));
    newType->decrRefCount();
    // -4  long, 32/64 bit signed integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-4, sizeof(long), "long"));
    newType->decrRefCount();
    // -5  unsigned char, 8 bit unsigned integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-5, 1, "unsigned char"));
    newType->decrRefCount();
    // -6  signed char, 8 bit signed integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-6, 1, "signed char"));
    newType->decrRefCount();
    // -7  unsigned short, 16 bit unsigned integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-7, 2, "unsigned short"));
    newType->decrRefCount();
    // -8  unsigned int, 32 bit unsigned integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-8, 4, "unsigned int"));
    newType->decrRefCount();
    // -9  unsigned, 32 bit unsigned integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-9, 4, "unsigned"));
    newType->decrRefCount();
    // -10 unsigned long, 32 bit unsigned integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-10, sizeof(unsigned long), "unsigned long"));
    newType->decrRefCount();
    // -11 void, type indicating the lack of a value
    //  XXX-size may not be correct jdd 4/22/99
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-11, 0, "void"));
    newType->decrRefCount();
    // -12 float, IEEE single precision
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-12, sizeof(float), "float"));
    newType->decrRefCount();
    // -13 double, IEEE double precision
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-13, sizeof(double), "double"));
    newType->decrRefCount();
    // -14 long double, IEEE double precision, size may increase in future
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-14, sizeof(long double), "long double"));
    newType->decrRefCount();
    // -15 integer, 32 bit signed integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-15, 4, "integer"));
    newType->decrRefCount();
    // -16 boolean, 32 bit type. GDB/GCC 0=False, 1=True, all other values
    //     have unspecified meaning
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-16, sizeof(bool), "boolean"));
    newType->decrRefCount();
    // -17 short real, IEEE single precision
    //  XXX-size may not be correct jdd 4/22/99
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-17, sizeof(float), "short real"));
    newType->decrRefCount();
    // -18 real, IEEE double precision XXX-size may not be correct jdd 4/22/99 
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-18, sizeof(double), "real"));
    newType->decrRefCount();
    // -19 stringptr XXX- size of void * -- jdd 4/22/99
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-19, sizeof(void *), "stringptr"));
    newType->decrRefCount();
    // -20 character, 8 bit unsigned character type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-20, 1, "character"));
    newType->decrRefCount();
    // -21 logical*1, 8 bit type (Fortran, used for boolean or unsigned int)
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-21, 1, "logical*1"));
    newType->decrRefCount();
    // -22 logical*2, 16 bit type (Fortran, some for boolean or unsigned int)
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-22, 2, "logical*2"));
    newType->decrRefCount();
    // -23 logical*4, 32 bit type (Fortran, some for boolean or unsigned int)
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-23, 4, "logical*4"));
    newType->decrRefCount();
    // -24 logical, 32 bit type (Fortran, some for boolean or unsigned int)
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-24, 4, "logical"));
    newType->decrRefCount();
    // -25 complex, consists of 2 IEEE single-precision floating point values
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-25, sizeof(float)*2, "complex"));
    newType->decrRefCount();
    // -26 complex, consists of 2 IEEE double-precision floating point values
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-26, sizeof(double)*2, "complex*16"));
    newType->decrRefCount();
    // -27 integer*1, 8 bit signed integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-27, 1, "integer*1"));
    newType->decrRefCount();
    // -28 integer*2, 16 bit signed integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-28, 2, "integer*2"));
    newType->decrRefCount();

/* Quick hack to make integer*4 compatible with int for Fortran
   jnb 6/20/01 */

    // This seems questionable - let's try removing that hack - jmo 05/21/04
    /*
    builtInTypes->addBuiltInType(newType = new BPatch_type("int",-29,
                                                 BPatch_built_inType, 4));
    newType->decrRefCount();
    */
    // -29 integer*4, 32 bit signed integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-29, 4, "integer*4"));
    newType->decrRefCount();
    // -30 wchar, Wide character, 16 bits wide, unsigned (unknown format)
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-30, 2, "wchar"));
    newType->decrRefCount();
#ifdef i386_unknown_nt4_0
    // -31 long long, 64 bit signed integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-31, sizeof(LONGLONG), "long long"));
    newType->decrRefCount();
    // -32 unsigned long long, 64 bit unsigned integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-32, sizeof(ULONGLONG), "unsigned long long"));
    newType->decrRefCount();
#else
    // -31 long long, 64 bit signed integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-31, sizeof(long long), "long long"));
    newType->decrRefCount();
    // -32 unsigned long long, 64 bit unsigned integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-32, sizeof(unsigned long long), "unsigned long long"));
    newType->decrRefCount();
#endif
    // -33 logical*8, 64 bit unsigned integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-33, 8, "logical*8"));
    newType->decrRefCount();
    // -34 integer*8, 64 bit signed integral type
    builtInTypes->addBuiltInType(newType = new BPatch_typeScalar(-34, 8, "integer*8"));
    newType->decrRefCount();

    // default callbacks are null
    postForkCallback = NULL;
    preForkCallback = NULL;
    errorHandler = NULL;
    dynLibraryCallback = NULL;
    execCallback = NULL;
    exitCallback = NULL;
    oneTimeCodeCallback = NULL;

#ifdef IBM_BPATCH_COMPAT
    RPCdoneCallback = NULL;
#endif

    loadNativeDemangler();

}


/*
 * BPatch::~BPatch
 *
 * Destructor for BPatch.  Free allocated memory.
 */
BPatch::~BPatch()
{
    delete info;

    type_Error->decrRefCount();
    type_Untyped->decrRefCount();

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
#if defined(i386_unknown_nt4_0) 
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
#if defined(i386_unknown_nt4_0)
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

#if defined(i386_unknown_nt4_0) 
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
    BPatchExitCallback ret;

    ret = exitCallback;
    exitCallback = func;
    
    return ret;
}

/*
 * BPatch::registerOneTimeCodeCallback
 *
 * Registers a function that is to be called by the library when a 
 * oneTimeCode (inferior RPC) is completed.
 *
 * func	The function to be called.
 */
BPatchOneTimeCodeCallback BPatch::registerOneTimeCodeCallback(BPatchOneTimeCodeCallback func)
{
    BPatchOneTimeCodeCallback ret;

    ret = oneTimeCodeCallback;
    oneTimeCodeCallback = func;

    return ret;
}

#ifdef IBM_BPATCH_COMPAT
BPatchExitCallback BPatch::registerExitCallback(BPatchThreadEventCallback func)
{

    BPatchExitCallback ret;

    ret = exitCallback;
    exitCallback = (BPatchExitCallback) func;

    return ret;

}
#endif

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
    } else if ((severity == BPatchFatal) || (severity == BPatchSerious)){
	fprintf(stdout, "DYNINST ERROR: %s\n", str);
	fflush(stdout);
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
    // We don't want to touch the bpatch threads here, as they may have been
    // deleted in the callback
    // TODO: figure out if they have and remove them from the info list
}

/*
 * BPatch::registerForkingThread
 *
 * Perform whatever processing is necessary when a thread enters
 * a fork system call. Previously the preForkCallback was made directly.
 *
 * forkingPid   pid of the forking process
 * proc			lower lever handle to process specific stuff
 *
 */
void BPatch::registerForkingThread(int forkingPid, process * /*proc*/)
{
    BPatch_thread *forking = info->threadsByPid[forkingPid];
    // Wouldn't this be the same as proc->thread?
    assert(forking);

    if (preForkCallback) {
        preForkCallback(forking, NULL);
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
    thread->image = new BPatch_image(thread);

    if (execCallback) {
       execCallback(thread);
    }
}

void BPatch::registerNormalExit(BPatch_thread *thread, int exitcode)
{
    thread->setExitCode(exitcode);
    thread->setExitedNormally();
    if (exitCallback) {
        exitCallback(thread, ExitedNormally);
    }
}

void BPatch::registerSignalExit(BPatch_thread *thread, int signalnum)
{
    thread->setExitedViaSignal(signalnum);
    if (exitCallback) {
        exitCallback(thread, ExitedViaSignal);
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
BPatch_thread *BPatch::createProcess(const char *path, const char *argv[], const char *envp[],
                                     int stdin_fd, int stdout_fd, int stderr_fd)
{
    clearError();

    BPatch_thread *ret = 
	new BPatch_thread(path, const_cast<char **>(argv), const_cast<char **>(envp), 
                          stdin_fd, stdout_fd, stderr_fd);

    if (!ret->proc ||
       (ret->proc->status() != stopped) ||
       !ret->proc->isBootstrappedYet()) {
	delete ret;
        reportError(BPatchFatal, 68, "create process failed bootstrap");
	return NULL;
    }
    ret->proc->collectSaveWorldData = false;
    //ccw 23 jan 2002 : this forces the user to call
    //BPatch_thread::enableDumpPatchedImage() if they want to use the save the world
    //functionality.
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
BPatch_thread *BPatch::attachProcess(const char *path, int pid)
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
    ret->proc->collectSaveWorldData = false;
    //ccw 31 jan 2003 : this forces the user to call
    //BPatch_thread::enableDumpPatchedImage() if they want to use the save the world
    //functionality.

    return ret;
}

/*
 * BPatch::getThreadEvent
 *
 * Checks for changes in any child process, and optionally blocks until such a
 * change has occurred.  Also performs housekeeping on behalf of Dyninst:
 * - Launches pending oneTimeCode calls (aka inferior RPCs)
 * - Updates the process object representing each process for which a
 *   change is detected.
 *
 * The return value is true if a change was detected, otherwise it is false.
 *
 * block	Set this parameter to true to block waiting for a change,
 * 		set to false to poll and return immediately, whether or not a
 * 		change occurred.
 */
bool BPatch::getThreadEvent(bool block)
{
    launchDeferredOneTimeCode();

    return getThreadEventOnly(block);
}

/*
 * BPatch::getThreadEventOnly
 *
 * Like getThreadEvent (which actually calls this function), this function
 * checks for changes in any child process, optionally blocking until such a
 * change occurs.  It also updates the process objects with information about
 * such changes.  Unlike getThreadEvent, it does not perform any additional
 * housekeeping like launching deferred RPCs.
 *
 * Returns true if a change was detected, otherwise returns false.
 *
 * block	Set this parameter to true to block waiting for a change,
 * 		set to false to poll and return immediately, whether or not a
 * 		change occurred.
 */
bool BPatch::getThreadEventOnly(bool block)
{
   bool	result = false;

   // Handles all process (object)-level events.
   
   pdvector<procevent *> events;
   result = getSH()->checkForProcessEvents(&events, -1, block);

   pdvector<procevent *> unhandledEvents = getSH()->handleProcessEvents(events);

   for(unsigned i=0; i < unhandledEvents.size(); i++)
   {
       // The only thing we expect to see in here is a SIGSTOP from DYNINSTbreakPoint...
       procevent *cur_event = unhandledEvents[i];
       process *proc = cur_event->proc;
       procSignalWhy_t why   = cur_event->why;
       procSignalWhat_t what = cur_event->what;
       
       bool exists;
       BPatch_thread *thread = getThreadByPid(proc->getPid(), &exists);
       if (thread == NULL) {
           if (exists) {
               bperr("Warning: event on an existing thread, but can't find thread handle\n");
           } else {
               bperr( "Warning - wait returned status of an unknown process (%d)\n",
                       proc->getPid());
           }
       }
       else { // found a thread
#if !defined(os_windows)
           if (didProcReceiveSignal(why)) {
               thread->lastSignal = what;
#if defined(os_irix)
               unsigned int stop_sig = SIGEMT;
#else
               unsigned int stop_sig = SIGSTOP;
#endif
               if (what != stop_sig) {
                   forwardSigToProcess(*cur_event);
               }
               else {
                   thread->setUnreportedStop(true);
               }
           }
           else {
              // bperr( "Unhandled event (why %d, what %d) on process %d\n",
              // why, what, proc->getPid());
               thread->setUnreportedStop(true);
           }
           
#else // os_windows
           thread->setUnreportedStop(true);
#endif
       }
       delete unhandledEvents[i];
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
    if (havePendingEvent()) {
        return true;
    }
    
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

    BPatch_fieldListType * newType = new BPatch_typeEnum(name);
    if (!newType) return NULL;
    
    APITypes->addType(newType);

    // ADD components to type
    for (unsigned int i=0; i < elementNames.size(); i++) {
        newType->addField(elementNames[i], elementIds[i]);
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
    BPatch_fieldListType * newType = new BPatch_typeEnum(name);

    if (!newType) return NULL;
    
    APITypes->addType(newType);

    // ADD components to type
    for (unsigned int i=0; i < elementNames.size(); i++) {
        newType->addField(elementNames[i], i);
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
    unsigned int i;
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
  
    BPatch_fieldListType *newType = new BPatch_typeStruct(name);
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
    unsigned int i;
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
  
    BPatch_fieldListType * newType = new BPatch_typeUnion(name);
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
        newType = new BPatch_typeArray(ptr, low, hi, name);
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

    BPatch_type * newType = new BPatch_typePointer(ptr, name);
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
    BPatch_type * newType = new BPatch_typeScalar(size, name);
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
    BPatch_type * newType = new BPatch_typeTypedef(ptr, name);

    if (!newType) return NULL;

    APITypes->addType(newType);
  
    return newType;
}

bool BPatch::waitUntilStopped(BPatch_thread *appThread){
	while (!appThread->isStopped() && !appThread->isTerminated())
		this->waitForStatusChange();

	if (!appThread->isStopped())
	{
		cerr << "ERROR : process did not signal mutator via stop"
		     << endl;
		return false;
	}
#if defined(i386_unknown_nt4_0) || \
    defined(mips_unknown_ce2_11)
	else if((appThread->stopSignal() != EXCEPTION_BREAKPOINT) && 
		(appThread->stopSignal() != -1))
	{
		cerr << "ERROR : process stopped on signal different"
		     << " than SIGTRAP" << endl;
		return false;
	}
#else
	else if ((appThread->stopSignal() != SIGSTOP) &&
#if defined(bug_irix_broken_sigstop)
		 (appThread->stopSignal() != SIGEMT) &&
#endif 
		 (appThread->stopSignal() != SIGHUP)) {
		cerr << "ERROR :  process stopped on signal "
		     << "different than SIGSTOP" << endl;
		return false;
	}
#endif
	return true;
}

#ifdef IBM_BPATCH_COMPAT

/*
 * Register a function to call when an RPC (i.e. oneshot) is done.
 *
 * dyninst version is a callback that is defined for BPatch_thread
 *
 */
BPatchThreadEventCallback BPatch::registerRPCTerminationCallback(BPatchThreadEventCallback func)
{
    BPatchThreadEventCallback ret;

    ret = RPCdoneCallback;
    RPCdoneCallback = func;

    return ret;
}

void setLogging_NP(BPatchLoggingCallback, int)
{
    return;
}

#endif

/*
 * BPatch::launchDeferredOneTimeCode
 *
 * Launch any deferred oneTimeCode calls (aka inferior RPCs) that might exist,
 * if it is now okay to do so.
 */
void BPatch::launchDeferredOneTimeCode()
{
    for (unsigned int p = 0; p < processVec.size(); p++) {
        process *proc = processVec[p];

        if (proc == NULL)
            continue;
        
        if (!proc->isAttached() ||
            proc->status() == neonatal)
            continue;

        proc->getRpcMgr()->launchRPCs(proc->status() == running);        
    }
}

//  updateStats() -- an internal function called before returning
//  statistics buffer to caller of BPatch_getStatistics(),
//  -- just copies global variable statistics counters into 
//  the buffer which is returned to the user.
void BPatch::updateStats() 
{
  stats.pointsUsed = pointsUsed;
  stats.totalMiniTramps = totalMiniTramps;
  stats.trampBytes = trampBytes;
  stats.ptraceOtherOps = ptraceOtherOps;
  stats.ptraceOps = ptraceOps;
  stats.ptraceBytes = ptraceBytes;
  stats.insnGenerated = insnGenerated;
}
