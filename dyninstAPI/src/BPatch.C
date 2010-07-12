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

#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#if !defined(os_windows)
#include <unistd.h>
#endif

#define BPATCH_FILE
#include "common/h/Pair.h"
#include "common/h/Vector.h"
#include "signalhandler.h"
#include "common/h/stats.h"
#include "BPatch.h"
//#include "BPatch_typePrivate.h"
#include "process.h"
#include "BPatch_libInfo.h"
#include "BPatch_collections.h"
#include "BPatch_thread.h"
#include "BPatch_asyncEventHandler.h"
#include "callbacks.h"
#include "common/h/timing.h"
#include "debug.h"
#include "signalgenerator.h"
#include "mapped_module.h"
#include "instPoint.h"
#include "hybridAnalysis.h"

#if defined(i386_unknown_nt4_0) || defined(mips_unknown_ce2_11) //ccw 20 july 2000 : 28 mar 2001
#include "nt_signal_emul.h"
#endif

using namespace SymtabAPI;

extern void loadNativeDemangler();

BPatch *BPatch::bpatch = NULL;

extern BPatch_asyncEventHandler *global_async_event_handler;
void defaultErrorFunc(BPatchErrorLevel level, int num, const char * const *params);

extern void dyninst_yield();

/*
 * BPatch::BPatch
 *
 * Constructor for BPatch.  Performs one-time initialization needed by the
 * library.
 */
BPatch::BPatch()
  : info(NULL),
    typeCheckOn(true),
    lastError(0),
    debugParseOn(true),
    baseTrampDeletionOn(false),
    trampRecursiveOn(false),
    forceRelocation_NP(false),
    autoRelocation_NP(true),
    saveFloatingPointsOn(true),
    livenessAnalysisOn_(true),
    livenessAnalysisDepth_(3),
    asyncActive(false),
    delayedParsing_(false),
    instrFrames(false),
    systemPrelinkCommand(NULL),
    mutateeStatusChange(false),
    waitingForStatusChange(false),
    notificationFDOutput_(-1),
    notificationFDInput_(-1),
    FDneedsPolling_(false),
    builtInTypes(NULL),
    stdTypes(NULL),
    type_Error(NULL),
    type_Untyped(NULL)
{
    if (!global_mutex) {
      global_mutex = new eventLock();
      extern bool mutex_created;
      mutex_created = true;
    }

    global_mutex->_Lock(FILE__, __LINE__);
    init_debug();
    init_stats();

    memset(&stats, 0, sizeof(BPatch_stats));
    extern bool init();

    // Save a pointer to the one-and-only bpatch object.
    if (bpatch == NULL){
       bpatch = this;
    }
    
    BPatch::bpatch->registerErrorCallback(defaultErrorFunc);
    bpinfo("installed default error reporting function");
    
    /*
     * Create the list of processes.
     */
    info = new BPatch_libInfo();

    /*
     * Create the "error" and "untyped" types.
     */
    type_Error   = BPatch_type::createFake("<error>");
    type_Untyped = BPatch_type::createFake("<no type>");
    
    /*
     * Initialize hash table of API types.
     */
    APITypes = BPatch_typeCollection::getGlobalTypeCollection();

    stdTypes = BPatch_typeCollection::getGlobalTypeCollection();
    vector<Type *> *sTypes = Symtab::getAllstdTypes();
    for(unsigned i=0; i< sTypes->size(); i++)
        stdTypes->addType(new BPatch_type((*sTypes)[i]));

    builtInTypes = new BPatch_builtInTypeCollection;
    sTypes = Symtab::getAllbuiltInTypes();
    for(unsigned i=0; i< sTypes->size(); i++)
        builtInTypes->addBuiltInType(new BPatch_type((*sTypes)[i]));

    //loadNativeDemangler();

    global_async_event_handler = new BPatch_asyncEventHandler();
#if defined(cap_async_events)
    if (!global_async_event_handler->initialize()) {
      //  not much else we can do in the ctor, except complain (should we abort?)
      bperr("%s[%d]:  failed to initialize asyncEventHandler, possibly fatal\n",
            __FILE__, __LINE__);
    }
#endif
    global_mutex->_Unlock(FILE__, __LINE__);
}


/*
 * BPatch::~BPatch
 *
 * Destructor for BPatch.  Free allocated memory.
 */
void BPatch::BPatch_dtor()
{
    delete info;

    type_Error->decrRefCount();
    type_Untyped->decrRefCount();

    if (stdTypes)
        BPatch_typeCollection::freeTypeCollection(stdTypes);
    if (APITypes)
        BPatch_typeCollection::freeTypeCollection(APITypes);


    if(systemPrelinkCommand){
        delete [] systemPrelinkCommand;
    }
    bpatch = NULL;
}

BPatch *BPatch::getBPatch() {
	return bpatch;
}

char * BPatch::getPrelinkCommandInt(){
	return systemPrelinkCommand;
}

void BPatch::setPrelinkCommandInt(char *command){

	if(systemPrelinkCommand){
		delete [] systemPrelinkCommand;
	}
	systemPrelinkCommand = new char[strlen(command)+1];
	memcpy(systemPrelinkCommand, command, strlen(command)+1);
}

bool BPatch::isTypeCheckedInt()
{
  return typeCheckOn;
}
void BPatch::setTypeCheckingInt(bool x)
{
  typeCheckOn = x;
}
bool BPatch::parseDebugInfoInt()
{
  return debugParseOn;
}
bool BPatch::delayedParsingOnInt()
{
  return delayedParsing_;
}
void BPatch::setDebugParsingInt(bool x)
{
  debugParseOn = x;
}
bool BPatch::baseTrampDeletionInt()
{
  return baseTrampDeletionOn;
}
void BPatch::setBaseTrampDeletionInt(bool x)
{
  baseTrampDeletionOn = x;
}
bool BPatch::isTrampRecursiveInt()
{
  return trampRecursiveOn;
}
void BPatch::setTrampRecursiveInt(bool x)
{
  trampRecursiveOn = x;
}
void BPatch::setLivenessAnalysisInt(bool x)
{
    livenessAnalysisOn_ = x;
}
bool BPatch::livenessAnalysisOnInt() {
    return livenessAnalysisOn_;
}

void BPatch::setLivenessAnalysisDepthInt(int x)
{
    livenessAnalysisDepth_ = x;
}
int BPatch::livenessAnalysisDepthInt() {
    return livenessAnalysisDepth_;
}

bool BPatch::hasForcedRelocation_NPInt()
{
  return forceRelocation_NP;
}
void BPatch::setForcedRelocation_NPInt(bool x)
{
  forceRelocation_NP = x;
}
bool BPatch::autoRelocationOnInt()
{
  return autoRelocation_NP;
}
void BPatch::setAutoRelocation_NPInt(bool x)
{
  autoRelocation_NP = x;
}
void BPatch::setDelayedParsingInt(bool x)
{
  delayedParsing_ = x;
}
bool BPatch::isMergeTrampInt()
{
  return true;
}
void BPatch::setMergeTrampInt(bool)
{
}

bool BPatch::isSaveFPROnInt()
{
  return saveFloatingPointsOn;
}
void BPatch::setSaveFPRInt(bool x)
{
  saveFloatingPointsOn = x;
}




/*
 * BPatch::registerErrorCallbackInt
 *
 * Registers a function that is to be called by the library when an error
 * occurs or when there is status to report.  Returns the address of the
 * previously registered error callback function.
 *
 * function	The function to be called.
 */


BPatchErrorCallback BPatch::registerErrorCallbackInt(BPatchErrorCallback function)
{
    BPatchErrorCallback ret = NULL;

    pdvector<CallbackBase *> cbs;
    getCBManager()->removeCallbacks(evtError, cbs);

    if (cbs.size()) {
      mailbox_printf("%s[%d]:  removed %d error cbs\n", FILE__, __LINE__, cbs.size());
      ErrorCallback *ercb = (ErrorCallback *) cbs[0];
      ret = ercb->getFunc();
    }

    if (function != 0) {
	ErrorCallback *cb = new ErrorCallback(function);
	getCBManager()->registerCallback(evtError, cb);
    }
    // If function is zero, we treat it as a remove-callback request

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
BPatchForkCallback BPatch::registerPostForkCallbackInt(BPatchForkCallback func)
{
#if defined(i386_unknown_nt4_0) 
  reportError(BPatchWarning, 0,
	      "postfork callbacks not implemented on this platform\n");
  return NULL;
#else
    BPatchForkCallback ret = NULL;

    pdvector<CallbackBase *> cbs;
    getCBManager()->removeCallbacks(evtPostFork, cbs);

    if (cbs.size()) {
      ForkCallback *fcb = (ForkCallback *) cbs[0];
      ret =  fcb->getFunc();
    }

    if (func != 0) {
	ForkCallback *cb = new ForkCallback(func);
	getCBManager()->registerCallback(evtPostFork, cb);
    }
    // If func is zero, we assume it is a remove-callback request

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
BPatchForkCallback BPatch::registerPreForkCallbackInt(BPatchForkCallback func)
{
#if defined(i386_unknown_nt4_0)
    reportError(BPatchWarning, 0,
	"prefork callbacks not implemented on this platform\n");
    return NULL;
#else
    BPatchForkCallback ret = NULL;

    pdvector<CallbackBase *> cbs;
    getCBManager()->removeCallbacks(evtPreFork, cbs);

    if (cbs.size()) {
      ForkCallback *fcb = (ForkCallback *) cbs[0];
      ret =  fcb->getFunc();
    }

    if (func != 0) {
	ForkCallback *cb = new ForkCallback(func);
	getCBManager()->registerCallback(evtPreFork, cb);
    }
    // If func is zero, we assume it is a remove-callback request
	
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
BPatchExecCallback BPatch::registerExecCallbackInt(BPatchExecCallback func)
{

#if defined(i386_unknown_nt4_0) 
    reportError(BPatchWarning, 0,
	"exec callbacks not implemented on this platform\n");
    return NULL;
#else
    BPatchExecCallback ret = NULL;

    pdvector<CallbackBase *> cbs;
    getCBManager()->removeCallbacks(evtExec, cbs);
    if (cbs.size()) {
      ExecCallback *fcb = (ExecCallback *) cbs[0];
      ret =  fcb->getFunc();
    }

    if (func != 0) {
	ExecCallback *cb = new ExecCallback(func);
	getCBManager()->registerCallback(evtExec, cb);
    }
    // If func is zero, we assume it is a remove-callback request

    return ret;

#endif
}

/*
 * BPatch::registerExitCallback
 *
 * Registers a function that is to be called by the library when a 
 * process has just called the exit system call
 *
 * func	The function to be called.
 */
BPatchExitCallback BPatch::registerExitCallbackInt(BPatchExitCallback func)
{
    pdvector<CallbackBase *> cbs;
    getCBManager()->removeCallbacks(evtProcessExit, cbs);

    BPatchExitCallback ret = NULL;
    if (cbs.size()) {
	ExitCallback *fcb = (ExitCallback *) cbs[0];
	ret =  fcb->getFunc();
    }

    if (func != 0) {
	ExitCallback *cb = new ExitCallback(func);
	getCBManager()->registerCallback(evtProcessExit, cb);
    }
    // If func is zero, we assume it is a remove-callback request

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
BPatchOneTimeCodeCallback BPatch::registerOneTimeCodeCallbackInt(BPatchOneTimeCodeCallback func)
{
    BPatchOneTimeCodeCallback ret = NULL;

    pdvector<CallbackBase *> cbs;
    getCBManager()->removeCallbacks(evtOneTimeCode, cbs);
    if (cbs.size()) {
      OneTimeCodeCallback *fcb = (OneTimeCodeCallback *) cbs[0];
      ret =  fcb->getFunc();
    }

    if (func != 0) {
	OneTimeCodeCallback *cb = new OneTimeCodeCallback(func);
	getCBManager()->registerCallback(evtOneTimeCode, cb);
    }
    // If func is zero, we assume it is a remove-callback request

    return ret;
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
BPatch::registerDynLibraryCallbackInt(BPatchDynLibraryCallback function)
{

    BPatchDynLibraryCallback ret = NULL;

    pdvector<CallbackBase *> cbs;
    getCBManager()->removeCallbacks(evtLoadLibrary, cbs);
    if (cbs.size()) {
      DynLibraryCallback *fcb = (DynLibraryCallback *) cbs[0];
      ret =  fcb->getFunc();
    }

    if (function != 0) {
	DynLibraryCallback *cb = new DynLibraryCallback(function);
	getCBManager()->registerCallback(evtLoadLibrary, cb);
    }
    // If function is zero, we assume it is a remove-callback request

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
   if (bpatch == NULL) {
      return; //Probably decontructing objects.
   }
    assert(global_mutex);
    //assert(global_mutex->depth());

    bool do_unlock = false;
    if (!global_mutex->depth()) {
      fprintf(stderr, "%s[%d]:  WARN:  reportError called w/0 lock\n", FILE__, __LINE__);
      global_mutex->_Lock(FILE__, __LINE__);
      do_unlock = true;
    }

    // don't log BPatchWarning or BPatchInfo messages as "errors"
    if ((severity == BPatchFatal) || (severity == BPatchSerious))
        bpatch->lastError = number;

    pdvector<CallbackBase *> cbs;
    if (! getCBManager()->dispenseCallbacksMatching(evtError, cbs)) {
        fprintf(stdout, "%s[%d]:  DYNINST ERROR:\n %s\n", FILE__, __LINE__, str);
        fflush(stdout);
        if (do_unlock) 
          global_mutex->_Unlock(FILE__, __LINE__);
        return; 
    }
    
    for (unsigned int i = 0; i < cbs.size(); ++i) {
        ErrorCallback *cb = dynamic_cast<ErrorCallback *>(cbs[i]);
        if (cb)
            (*cb)(severity, number, str); 
    }

    if (do_unlock) 
       global_mutex->_Unlock(FILE__, __LINE__);
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
			       const char *fmt, const char * const *params)
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

static const char *lvl_str(BPatchErrorLevel lvl)
{
  switch(lvl) {
    case BPatchFatal: return "--FATAL--";
    case BPatchSerious: return "--SERIOUS--";
    case BPatchWarning: return "--WARN--";
    case BPatchInfo: return "--INFO--";
  };
  return "BAD ERR CODE";
}

void defaultErrorFunc(BPatchErrorLevel level, int num, const char * const *params)
{
    char line[256];

    if ((level == BPatchWarning) || (level == BPatchInfo)) {
         // ignore low level errors/warnings in the default reporter
         return;
    }

    const char *msg = BPatch::bpatch->getEnglishErrorString(num);
    BPatch::bpatch->formatErrorString(line, sizeof(line), msg, params);

    if (num != -1) {
       fprintf(stderr,"%s #%d: %s\n", lvl_str(level),num, line);
    }
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
BPatch_process *BPatch::getProcessByPid(int pid, bool *exists)
{
    if (info->procsByPid.defines(pid)) {
        if (exists) *exists = true;
        BPatch_process *proc = info->procsByPid[pid];
        return proc;
    } else {
        if (exists) *exists = false;
        return NULL;
    }
}

BPatch_thread *BPatch::getThreadByPid(int pid, bool *exists)
{
   BPatch_process *p = getProcessByPid(pid, exists);
   if (!exists)
      return NULL;
   assert(p->threads.size() > 0);
   return p->threads[0];
}


/*
 * BPatch::getThreads
 *
 * Returns a vector of all threads that are currently defined.  Includes
 * threads created directly using the library and those created with UNIX fork
 * or Windows NT spawn system calls.  The caller is responsible for deleting
 * the vector when it is no longer needed.
 */
BPatch_Vector<BPatch_thread *> *BPatch::getThreadsInt()
{
    BPatch_Vector<BPatch_thread *> *result = new BPatch_Vector<BPatch_thread *>;

    dictionary_hash_iter<int, BPatch_process *> ti(info->procsByPid);

    int pid;
    BPatch_process *proc;

    while (ti.next(pid, proc))
    {
       assert(proc);
       assert(proc->threads.size() > 0);
       result->push_back(proc->threads[0]);
    }

    return result;
}

/*
 * BPatch::getProcs
 *
 * Returns a vector of all threads that are currently defined.  Includes
 * threads created directly using the library and those created with UNIX fork
 * or Windows NT spawn system calls.  The caller is responsible for deleting
 * the vector when it is no longer needed.
 */
BPatch_Vector<BPatch_process *> *BPatch::getProcessesInt()
{
   BPatch_Vector<BPatch_process *> *result = new BPatch_Vector<BPatch_process *>;
   dictionary_hash_iter<int, BPatch_process *> ti(info->procsByPid);

   int pid;
   BPatch_process *proc;
   
   while (ti.next(pid, proc))
   {
      assert(proc);
      result->push_back(proc);
   }
   
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
    assert(!info->procsByPid.defines(pid));
    info->procsByPid[pid] = NULL;
}


/*
 * BPatch::registerForkedProcess
 *
 * Register a new process that is not yet associated with a thread.
 * (this function is an upcall when a new process is created).
 *
 * parentPid		the pid of the parent process.
 * childPid		The pid of the process to register.
 * proc			lower lever handle to process specific stuff
 *
 */
void BPatch::registerForkedProcess(process *parentProc, process *childProc)
{
    int parentPid = parentProc->getPid();
    int childPid = childProc->getPid();

    forkexec_printf("BPatch: registering fork, parent %d, child %d\n",
                    parentPid, childPid);
    assert(getProcessByPid(childPid) == NULL);
    
    BPatch_process *parent = getProcessByPid(parentPid);
    assert(parent);

    BPatch_process *child = new BPatch_process(childProc);

#if defined(cap_async_events)
    // We're already attached to the parent... let's see if the
    // simple way works.
    if (!getAsync()->mutateeDetach(child->lowlevel_process())) {
        bperr("%s[%d]:  asyncEventHandler->mutateeDetach failed\n", __FILE__, __LINE__);
    }
   // if (!getAsync()->detachFromProcess(child)) {
    //    bperr("%s[%d]:  asyncEventHandler->mutateeDetach failed\n", __FILE__, __LINE__);
    //}
    if (!getAsync()->connectToProcess(child->lowlevel_process())) {
        bperr("%s[%d]:  asyncEventHandler->connectToProcess failed\n", __FILE__, __LINE__);
    }
    else 
        asyncActive = true;
#endif
    forkexec_printf("Successfully connected socket to child\n");
    
    pdvector<CallbackBase *> cbs;
    getCBManager()->dispenseCallbacksMatching(evtPostFork,cbs);
    
    signalNotificationFD();
    
    for (unsigned int i = 0; i < cbs.size(); ++i) {

        ForkCallback *cb = dynamic_cast<ForkCallback *>(cbs[i]);
        if (cb) {
            (*cb)(parent->threads[0], child->threads[0]);
        }
    }

    child->isVisiblyStopped = false;
    forkexec_printf("BPatch: finished registering fork, parent %d, child %d\n",
                    parentPid, childPid);
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
void BPatch::registerForkingProcess(int forkingPid, process * /*proc*/)
{
    BPatch_process *forking = getProcessByPid(forkingPid);
    assert(forking);

    signalNotificationFD();

    pdvector<CallbackBase *> cbs;
    getCBManager()->dispenseCallbacksMatching(evtPreFork,cbs);

    for (unsigned int i = 0; i < cbs.size(); ++i) {

        assert(cbs[i]);
        ForkCallback *cb = dynamic_cast<ForkCallback *>(cbs[i]);
        if (cb)
            (*cb)(forking->threads[0], NULL);
    }
}


/*
 * BPatch::registerExecCleanup
 *
 * Register a process that has just entered exec
 *
 * Gives us some cleanup time
 */

void BPatch::registerExecCleanup(process *p, char *) 
{
    BPatch_process *execing = getProcessByPid(p->getPid());
    assert(execing);

    for (unsigned i=0; i<execing->threads.size(); i++)
       registerThreadExit(p, execing->threads[i]->getTid(), false);

    // tell the async that the process went away
    getAsync()->cleanupProc(p);
}    

/*
 * BPatch::registerExecExit
 *
 * Register a process that has just done an exec call.
 *
 * thread	thread that has just performed the exec
 *
 */

void BPatch::registerExecExit(process *proc)
{
    int execPid = proc->getPid();
    BPatch_process *process = getProcessByPid(execPid);
    assert(process);

   // build a new BPatch_image for this one
   if (process->image)
      process->image->removeAllModules();

   process->image = new BPatch_image(process);

   // The async pipe should be gone... handled in registerExecCleanup

   signalNotificationFD();

   //   for (unsigned i=0; i<process->threads.size(); i++)
   //   process->deleteBPThread(process->threads[i]);

#if defined(cap_async_events)
   //  I think in the case of exec that we do not need to re-initiate a async connection
   //  to the process that exec'd 
#if 1 
    if (!getAsync()->mutateeDetach(proc)) {
        bperr("%s[%d]:  asyncEventHandler->mutateeDetach failed\n", __FILE__, __LINE__);
    }

   async_printf("%s[%d]:  about to connect to exec process\n", FILE__, __LINE__);

   if (!getAsync()->connectToProcess(proc)) 
   {
      bperr("%s[%d]:  asyncEventHandler->connectToProcess failed\n", __FILE__, __LINE__);
	  async_printf("%s[%d]:  connect to exec process failed\n", FILE__, __LINE__);
   } 
   else
   {
      asyncActive = true;
	  async_printf("%s[%d]:  connect to exec process success\n", FILE__, __LINE__);
   }

#else

      asyncActive = true;
	  async_printf("%s[%d]:  connect to exec process skipped\n", FILE__, __LINE__);

#endif
#endif

   if (!process->updateThreadInfo()) 
   {
	   fprintf(stderr, "%s[%d]:  failed to updateThreadInfo after exec\n", FILE__, __LINE__);
	   return;
   }

    pdvector<CallbackBase *> cbs;
    getCBManager()->dispenseCallbacksMatching(evtExec,cbs);
    for (unsigned int i = 0; i < cbs.size(); ++i) {
        ExecCallback *cb = dynamic_cast<ExecCallback *>(cbs[i]);
        if (cb)
            (*cb)(process->threads[0]);
    }
}

void BPatch::registerNormalExit(process *proc, int exitcode)
{
   if (!proc)
      return;


   int pid = proc->getPid();

   BPatch_process *process = getProcessByPid(pid);

   if (!process) return;

   process->terminated = true;


   BPatch_thread *thrd = process->getThreadByIndex(0);

   process->setExitCode(exitcode);
   process->setExitedNormally();
   process->setUnreportedTermination(true);

   signalNotificationFD();

   pdvector<CallbackBase *> cbs;

   if (thrd) {
      getCBManager()->dispenseCallbacksMatching(evtThreadExit,cbs);
      for (unsigned int i = 0; i < cbs.size(); ++i) {
         AsyncThreadEventCallback *cb = dynamic_cast<AsyncThreadEventCallback *>(cbs[i]);
         if (cb)
            (*cb)(process, thrd);
      }
   }
   cbs.clear();
   getCBManager()->dispenseCallbacksMatching(evtProcessExit,cbs);
   for (unsigned int i = 0; i < cbs.size(); ++i) {
       ExitCallback *cb = dynamic_cast<ExitCallback *>(cbs[i]);
       if (cb) {
           signal_printf("%s[%d]:  about to register/wait for exit callback\n", FILE__, __LINE__);
           (*cb)(process->threads[0], ExitedNormally);
           signal_printf("%s[%d]:  exit callback done\n", FILE__, __LINE__);
       }
   }


   // We now run the process out; set its state to terminated. Really, the user shouldn't
   // try to do anything else with this, but we can get that happening.
   BPatch_process *stillAround = getProcessByPid(pid);
   if (stillAround) {
      stillAround->reportedExit = true;
      stillAround->terminated = true;
   }
}

void BPatch::registerSignalExit(process *proc, int signalnum)
{
   if (!proc)
      return;

   int pid = proc->getPid();

   BPatch_process *bpprocess = getProcessByPid(pid);
   if (!bpprocess) {
       // Error during startup can cause this -- we have a partially
       // constructed process object, but it was never registered with
       // bpatch
       return;
   }
   BPatch_thread *thrd = bpprocess->getThreadByIndex(0);

   bpprocess->setExitedViaSignal(signalnum);
   bpprocess->setUnreportedTermination(true);
   bpprocess->terminated = true;

   signalNotificationFD();

   pdvector<CallbackBase *> cbs;
   if (thrd) {
      getCBManager()->dispenseCallbacksMatching(evtThreadExit,cbs);
      for (unsigned int i = 0; i < cbs.size(); ++i) {
         
         AsyncThreadEventCallback *cb = dynamic_cast<AsyncThreadEventCallback *>(cbs[i]);
         if (cb) 
            (*cb)(bpprocess, thrd);
      }
      cbs.clear();
      getCBManager()->dispenseCallbacksMatching(evtProcessExit,cbs);
      for (unsigned int i = 0; i < cbs.size(); ++i) {
         
         ExitCallback *cb = dynamic_cast<ExitCallback *>(cbs[i]);
         if (cb) {
            (*cb)(bpprocess->threads[0], ExitedViaSignal);
         }
      }
   }
   
   // We now run the process out; set its state to terminated. Really, the user shouldn't
   // try to do anything else with this, but we can get that happening.
   BPatch_process *stillAround = getProcessByPid(pid);
   if (stillAround) {
      stillAround->reportedExit = true;
      stillAround->terminated = true;
   }

   // We need to clean this up... but the user still has pointers
   // into this code. Ugh.
   // Do not continue at this point; process is already gone.

}

bool BPatch::registerThreadCreate(BPatch_process *proc, BPatch_thread *newthr)
{
   if (newthr->reported_to_user) {
      async_printf("%s[%d]:  NOT ISSUING CALLBACK:  thread %lu exists\n", 
                   FILE__, __LINE__, (long) newthr->getTid());
      return false;
   }

   signalNotificationFD();

   pdvector<CallbackBase *> cbs;
   getCBManager()->dispenseCallbacksMatching(evtThreadCreate, cbs);
  
   for (unsigned int i = 0; i < cbs.size(); ++i) {
      
      AsyncThreadEventCallback &cb = * ((AsyncThreadEventCallback *) cbs[i]);
      async_printf("%s[%d]:  before issuing thread create callback: tid %lu\n", 
                   FILE__, __LINE__, newthr->getTid());
      cb(proc, newthr);
   }

   newthr->reported_to_user = true;
   BPatch::bpatch->mutateeStatusChange = true;
   proc->llproc->sh->signalEvent(evtThreadCreate);

   return true;
}


void BPatch::registerThreadExit(process *proc, long tid, bool exiting)
{
    if (!proc)
        return;
    
    int pid = proc->getPid();
    
    BPatch_process *bpprocess = getProcessByPid(pid);
    
    if (!bpprocess) {
        // Error during startup can cause this -- we have a partially
        // constructed process object, but it was never registered with
        // bpatch
        return;
    }
    BPatch_thread *thrd = bpprocess->getThread(tid);
    if (!thrd) {
        //If we don't have an BPatch thread, then it might have been an internal
        // thread that we decided not to report to the user (happens during 
        //  windows attach).  Just trigger the lower level clean up in this case.
        if (tid == 0) {
          fprintf(stderr, "%s[%d]:  about to deleteThread(0)\n", FILE__, __LINE__);
        }
        if (!exiting) proc->deleteThread(tid);        
        return;
    }

    if (thrd->deleted_callback_made) { 
        // Thread exits; we make the callback, then the process exits and
        // tries to nuke it as well. This guards against that.
        return;
    }

    signalNotificationFD();

    thrd->deleted_callback_made = true;
    pdvector<CallbackBase *> cbs;
    getCBManager()->dispenseCallbacksMatching(evtThreadExit, cbs);

    for (unsigned int i = 0; i < cbs.size(); ++i) {

        AsyncThreadEventCallback *cb = dynamic_cast<AsyncThreadEventCallback *>(cbs[i]);
        mailbox_printf("%s[%d]:  executing thread exit callback\n", FILE__, __LINE__);
        if (cb) {
            cb->set_synchronous(true);
            (*cb)(bpprocess, thrd);
            cb->set_synchronous(false);
        }
    }
    if (exiting) 
       return;
    if (proc->execing())
       thrd->deleteThread(false);
    else
       thrd->deleteThread();
}



/*
 * BPatch::registerLoadedModule
 *
 * Register a new module loaded by a process (e.g., dlopen)
 */

void BPatch::registerLoadedModule(process *process, mapped_module *mod) {

    BPatch_process *bProc = BPatch::bpatch->getProcessByPid(process->getPid());
    if (!bProc) return; // Done
    BPatch_image *bImage = bProc->getImage();
    assert(bImage); // This we can assert to be true
    
    BPatch_module *bpmod = bImage->findOrCreateModule(mod);

    signalNotificationFD();
    
    pdvector<CallbackBase *> cbs;
    
    if (! getCBManager()->dispenseCallbacksMatching(evtLoadLibrary, cbs)) {
        return;
    }
    for (unsigned int i = 0; i < cbs.size(); ++i) {
        DynLibraryCallback *cb = dynamic_cast<DynLibraryCallback *>(cbs[i]);
        if (cb)
            (*cb)(bProc->threads[0], bpmod, true);
    }
}

/*
 * BPatch::registerUnloadedModule
 *
 * Register a new module loaded by a process (e.g., dlopen)
 */

void BPatch::registerUnloadedModule(process *process, mapped_module *mod) {

    BPatch_process *bProc = BPatch::bpatch->getProcessByPid(process->getPid());
    if (!bProc) return; // Done
    BPatch_image *bImage = bProc->getImage();
    assert(bImage); // This we can assert to be true
    
    BPatch_module *bpmod = bImage->findModule(mod);
    if (bpmod == NULL) return;

    signalNotificationFD();
    
    pdvector<CallbackBase *> cbs;
    
    // For now we use the same callback for load and unload of library....
    if (! getCBManager()->dispenseCallbacksMatching(evtLoadLibrary, cbs)) {
        return;
    }
    for (unsigned int i = 0; i < cbs.size(); ++i) {
        DynLibraryCallback *cb = dynamic_cast<DynLibraryCallback *>(cbs[i]);
        if (cb)
            (*cb)(bProc->threads[0], bpmod, false);
    }

    bImage->removeModule(bpmod);
}


/*
 * BPatch::registerProcess
 *
 * Register a new BPatch_process object with the BPatch library (this function
 * is called only by the constructor for BPatch_process).
 *
 * process	A pointer to the process to register.
 */ 
void BPatch::registerProcess(BPatch_process *process, int pid)
{
   if (!pid)
      pid = process->getPid();

   assert(!info->procsByPid.defines(pid) || !info->procsByPid[pid]);
   info->procsByPid[pid] = process;
}


/*
 * BPatch::unRegisterProcess
 *
 * Remove the BPatch_thread associated with a given pid from the list of
 * threads being managed by the library.
 *
 * pid		The pid of the thread to be removed.
 */
void BPatch::unRegisterProcess(int pid, BPatch_process *proc)
{
   if (pid == -1 || !info->procsByPid.defines(pid)) {
      // Deleting an exited process; search and nuke
      dictionary_hash_iter<int, BPatch_process *> procsIter(info->procsByPid);
      BPatch_process *p;
      int pid2;
      while (procsIter.next(pid2, p)) {
         if (p == proc) {
            info->procsByPid.undef(pid2);
            return;
         }
      }
   }

   if (!info->procsByPid.defines(pid)) {
      char ebuf[256];
      sprintf(ebuf, "%s[%d]: no process %d defined in procsByPid\n", FILE__, __LINE__, pid);
      reportError(BPatchFatal, 68, ebuf);
      //fprintf(stderr, "%s[%d]:  ERROR, no process %d defined in procsByPid\n", FILE__,  __LINE__, pid);
      //dictionary_hash_iter<int, BPatch_process *> iter(info->procsByPid);
      //BPatch_process *p;
      //int pid;
      //while (iter.next(pid, p)) {
      //   fprintf(stderr, "%s[%d]:  have process %d\n", FILE__, __LINE__, pid);
      //}
      return;
   }
   assert(info->procsByPid.defines(pid));
   info->procsByPid.undef(pid);	
   assert(!info->procsByPid.defines(pid));
}


/*
 * BPatch::processCreateInt
 *
 * Create a process and return a BPatch_process representing it.
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
BPatch_process *BPatch::processCreateInt(const char *path, const char *argv[], 
                                         const char **envp, int stdin_fd, 
                                         int stdout_fd, int stderr_fd,
                                         BPatch_hybridMode mode)
{
   clearError();

   if ( path == NULL ) { return NULL; }

#if !defined (os_windows)
   //  This might be ok on windows...  not 100%sure and it takes to long to build for
   //  the moment.

   //  just a sanity check for the exitence of <path>
   struct stat statbuf;
   if (-1 == stat(path, &statbuf)) {
      char ebuf[2048];
      sprintf(ebuf, "createProcess(%s,...):  file does not exist\n", path);
      reportError(BPatchFatal, 68, ebuf);
      return NULL;
   }

   //  and ensure its a regular file:
   if (!S_ISREG(statbuf.st_mode)) {
      char ebuf[2048];
      sprintf(ebuf, "createProcess(%s,...):  not a regular file \n", path);
      reportError(BPatchFatal, 68, ebuf);
      return NULL;
   }

   //  and ensure its executable (does not check permissions):
#if !defined(os_vxworks) // Not necessary for VxWorks modules
   if (! ( (statbuf.st_mode & S_IXUSR)
            || (statbuf.st_mode & S_IXGRP)
            || (statbuf.st_mode & S_IXOTH) )) {
      char ebuf[2048];
      sprintf(ebuf, "createProcess(%s,...):  not an executable  \n", path);
      reportError(BPatchFatal, 68, ebuf);
      return NULL;
   }
#endif // VxWorks
#endif

   BPatch_process *ret = 
      new BPatch_process(path, argv, mode, envp, stdin_fd,stdout_fd,stderr_fd);

   if (!ret->llproc 
         ||  ret->llproc->status() != stopped 
         ||  !ret->llproc->isBootstrappedYet()) {
      ret->BPatch_process_dtor();  
      delete ret;
      reportError(BPatchFatal, 68, "create process failed bootstrap");
      return NULL;
   }

#if defined(cap_async_events)
   async_printf("%s[%d]:  about to connect to process\n", FILE__, __LINE__);
   if (!getAsync()->connectToProcess(ret->llproc)) {
      bpfatal("%s[%d]: asyncEventHandler->connectToProcess failed\n", __FILE__, __LINE__);
      fprintf(stderr,"%s[%d]: asyncEventHandler->connectToProcess failed\n", __FILE__, __LINE__);
      return NULL;
   }
   asyncActive = true;
#endif

   if (!ret->updateThreadInfo()) return NULL;

   return ret;
}

/*
 * BPatch::createProcess
 * This function is deprecated, see processCreate
 */
BPatch_thread *BPatch::createProcessInt(const char *path, const char *argv[], 
                                         const char **envp, int stdin_fd, 
                                         int stdout_fd, int stderr_fd)
{
   BPatch_process *ret = processCreateInt(path, argv, envp, stdin_fd, 
                                          stdout_fd, stderr_fd);
   if (!ret)
      return NULL;

   assert(ret->threads.size() > 0);
   return ret->threads[0];
}

/*
 * BPatch::processAttach
 *
 * Attach to a running process and return a BPatch_thread representing it.
 * Returns NULL upon failure.
 *
 * path		The pathname of the executable for the process.
 * pid		The id of the process to attach to.
 */
BPatch_process *BPatch::processAttachInt
(const char *path, int pid, BPatch_hybridMode mode)
{
   clearError();
   
   if (info->procsByPid.defines(pid)) {
      char msg[256];
      sprintf(msg, "attachProcess failed.  Dyninst is already attached to %d.",
              pid);
      reportError(BPatchWarning, 26, msg);      
      return NULL;
   }

   BPatch_process *ret = new BPatch_process(path, pid, mode);

   if (!ret->llproc ||
       ret->llproc->status() != stopped ||
       !ret->llproc->isBootstrappedYet()) {
       ret->BPatch_process_dtor();  
       char msg[256];
       sprintf(msg,"attachProcess failed: process %d may now be killed!",pid);
       reportError(BPatchWarning, 26, msg);
       delete ret;
       return NULL;
   }

#if defined(cap_async_events)
   if (!getAsync()->connectToProcess(ret->llproc)) {
      bperr("%s[%d]:  asyncEventHandler->connectToProcess failed\n", __FILE__, __LINE__);
      return NULL;
   } 
   asyncActive = true;
#endif
   if (!ret->updateThreadInfo()) return false;

   return ret;
}

/*
 * BPatch::attachProcess
 * This function is deprecated, see processAttach
 */
BPatch_thread *BPatch::attachProcessInt(const char *path, int pid)
{
   BPatch_process *proc = processAttachInt(path, pid);
   if (!proc)
      return NULL;
   
   assert(proc->threads.size() > 0);
   return proc->threads[0];
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
bool BPatch::pollForStatusChangeInt()
{
    getMailbox()->executeCallbacks(FILE__, __LINE__);

    clearNotificationFD();
    
    if (mutateeStatusChange) {
        mutateeStatusChange = false;
        return true;
    }
#if defined(os_linux)
   //  This might only be needed on linux 2.4, but we need to manually check
   //  to see if any threads exited here, and, if they have...  kick the 
   //  appropriate signal generator to wake up
   dictionary_hash_iter<int, BPatch_process *> ti(info->procsByPid);

   int pid;
   BPatch_process *proc;
   
   while (ti.next(pid, proc))
   {
      assert(proc);
      process *p = proc->llproc;
      assert(p);
      //  if the process has a rep lwp, it is not mt
      dyn_lwp *replwp = p->getRepresentativeLWP();
      if (replwp) continue;
      SignalGenerator *sg = p->getSG();
      // This guy exited, but we haven't deleted the procsByPid map
      if (!sg) continue;

      if (sg->exists_dead_lwp()) {
        if (sg->isWaitingForOS()) {
          //  the signal generator is inside a waitpid, kick the mutatee to wake
          //  it up.
          sg->forceWaitpidReturn();
        }
      }
   }
#endif
    return false;
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
bool BPatch::waitForStatusChangeInt()
{

  getMailbox()->executeCallbacks(FILE__, __LINE__);

  if (mutateeStatusChange) {
    mutateeStatusChange = false;
    clearNotificationFD();
    signal_printf("[%s:%u] - Returning due to immediate mutateeStatusChange\n", FILE__, __LINE__);
    return true;
  }

  SignalGenerator *sh = NULL;

  //  find a signal handler (in an active process)
  extern pdvector<process *> processVec;
  if (!processVec.size()) {
      clearNotificationFD();
      return false;
  }

  for (unsigned int i = 0; i < processVec.size(); ++i) {
    if (processVec[i] && processVec[i]->status() != deleted) {
      sh = processVec[i]->sh;
      break;
    }
  } 
  if (!sh) {
    clearNotificationFD();
    return false;
  }
  eventType evt = evtUndefined;
  do {
   pdvector<eventType> evts;
   //evts.push_back(evtProcessStop);
   //evts.push_back(evtProcessExit);
   //evts.push_back(evtThreadCreate);
   //evts.push_back(evtThreadExit);

   // I'm kinda confused... what about fork or exec? The above wouldn't wake us
   // up...
   //evts.push_back(evtSyscallExit);
   // Or a library load for that matter.

   // We need to wait for anything; non-exits may cause a callback to be made
   // (without hitting an evtProcessStop)
   evts.push_back(evtAnyEvent);

   waitingForStatusChange = true;
   getMailbox()->executeCallbacks(FILE__, __LINE__);
   if (mutateeStatusChange) break;
   signal_printf("Blocking in waitForStatusChange\n");
   evt = SignalGeneratorCommon::globalWaitForOneOf(evts);
  } while ((    evt != evtProcessStop ) 
            && (evt != evtProcessExit)
            && (evt != evtThreadExit)
           && (evt != evtThreadCreate));

  signal_printf("Returning from waitForStatusChange, evt = %s, mutateeStatusChange = %d\n", eventType2str(evt), (int) mutateeStatusChange);
  waitingForStatusChange = false;

  clearNotificationFD();

  if (mutateeStatusChange) {
    mutateeStatusChange = false;
    return true;
  }
  //  we waited for a change, but didn't get it
  signal_printf("%s[%d]:  Error in status change reporting\n", FILE__, __LINE__);
  return false;
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
BPatch_type * BPatch::createEnumInt( const char * name, 
				     BPatch_Vector<char *> &elementNames,
				     BPatch_Vector<int> &elementIds)
{
    if (elementNames.size() != elementIds.size()) {
      return NULL;
    }
    string typeName = name;
    vector<pair<string, int> *>elements;
    for (unsigned int i=0; i < elementNames.size(); i++) 
        elements.push_back(new pair<string, int>(elementNames[i], elementIds[i]));
    
    Type *typ = typeEnum::create( typeName, elements);
    if (!typ) return NULL;
    
    BPatch_type *newType = new BPatch_type(typ);
    if (!newType) return NULL;
    
    APITypes->addType(newType);

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
BPatch_type * BPatch::createEnumAutoId( const char * name, 
				        BPatch_Vector<char *> &elementNames)
{
    string typeName = name;
    vector<pair<string, int> *>elements;
    for (unsigned int i=0; i < elementNames.size(); i++) 
        elements.push_back(new pair<string, int>(elementNames[i], i));
    
    Type *typ = typeEnum::create( typeName, elements);
    if (!typ) return NULL;
    
    BPatch_type *newType = new BPatch_type(typ);
    if (!newType) return NULL;
    
    APITypes->addType(newType);

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

BPatch_type * BPatch::createStructInt( const char * name,
				       BPatch_Vector<char *> &fieldNames,
				       BPatch_Vector<BPatch_type *> &fieldTypes)
{
   unsigned int i;
   
   if (fieldNames.size() != fieldTypes.size()) {
      return NULL;
   }
   
   string typeName = name;
   vector<pair<string, Type *> *> fields;
   for(i=0; i<fieldNames.size(); i++)
   {
      if(!fieldTypes[i])
         return NULL;
      fields.push_back(new pair<string, Type *> (fieldNames[i], fieldTypes[i]->getSymtabType()));
   }	
   
   Type *typ = typeStruct::create(typeName, fields);
   if (!typ) return NULL;
   
   BPatch_type *newType = new BPatch_type(typ);
   if (!newType) return NULL;
   
   APITypes->addType(newType);
   
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

BPatch_type * BPatch::createUnionInt( const char * name, 
				      BPatch_Vector<char *> &fieldNames,
				      BPatch_Vector<BPatch_type *> &fieldTypes)
{
    unsigned int i;
    
    if (fieldNames.size() != fieldTypes.size()) {
      return NULL;
    }

    string typeName = name;
    vector<pair<string, Type *> *> fields;
    for(i=0; i<fieldNames.size(); i++)
    {
        if(!fieldTypes[i])
	    return NULL;
        fields.push_back(new pair<string, Type *> (fieldNames[i], fieldTypes[i]->getSymtabType()));
    }	
    
    Type *typ = typeUnion::create(typeName, fields);
    if (!typ) return NULL;
    
    BPatch_type *newType = new BPatch_type(typ);
    if (!newType) return NULL;

    APITypes->addType(newType);

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
BPatch_type * BPatch::createArrayInt( const char * name, BPatch_type * ptr,
				      unsigned int low, unsigned int hi)
{

    BPatch_type * newType;
    if (!ptr) 
        return NULL;
        
    string typeName = name;
    Type *typ = typeArray::create(typeName, ptr->getSymtabType(), low, hi);
    if (!typ) return NULL;
    
    newType = new BPatch_type(typ);
    if (!newType) return NULL;

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
BPatch_type * BPatch::createPointerInt(const char * name, BPatch_type * ptr,
                                       int /*size*/)
{
    BPatch_type * newType;
    if(!ptr)
        return NULL;
    
    string typeName = name;
    Type *typ = typePointer::create(typeName, ptr->getSymtabType());
    if (!typ) return NULL;
    
    newType = new BPatch_type(typ);
    if (!newType) return NULL;

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

BPatch_type * BPatch::createScalarInt( const char * name, int size)
{
    BPatch_type * newType;
    
    string typeName = name;
    Type *typ = typeScalar::create(typeName, size);
    if (!typ) return NULL;
    
    newType = new BPatch_type(typ);
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
BPatch_type * BPatch::createTypedefInt( const char * name, BPatch_type * ptr)
{
    BPatch_type * newType;
    if(!ptr)
        return NULL;
    
    string typeName = name;
    Type *typ = typeTypedef::create(typeName, ptr->getSymtabType());
    if (!typ) return NULL;
    
    newType = new BPatch_type(typ);
    if (!newType) return NULL;

    APITypes->addType(newType);
    return newType;
}

bool BPatch::waitUntilStoppedInt(BPatch_thread *appThread){

   bool ret = false;

   while (1) {
     __LOCK;
     if (!appThread->isStopped() && !appThread->isTerminated()) {
       __UNLOCK;
       this->waitForStatusChange();
     }
     else {
       __UNLOCK;
       break;
     }
   }

   __LOCK;

	if (!appThread->isStopped())
	{
		cerr << "ERROR : process did not signal mutator via stop"
		     << endl;
		ret = false;
 		goto done;
	}
#if defined(i386_unknown_nt4_0) || \
    defined(mips_unknown_ce2_11)
	else if((appThread->stopSignal() != EXCEPTION_BREAKPOINT) && 
		(appThread->stopSignal() != -1))
	{
		cerr << "ERROR : process stopped on signal different"
		     << " than SIGTRAP" << endl;
		ret =  false;
 		goto done;
	}
#else
	else if ((appThread->stopSignal() != SIGSTOP) &&
#if defined(bug_irix_broken_sigstop)
		 (appThread->stopSignal() != SIGEMT) &&
#endif 
		 (appThread->stopSignal() != SIGHUP)) {
		cerr << "ERROR :  process stopped on signal "
		     << "different than SIGSTOP" << endl;
		ret =  false;
 		goto done;
	}
#endif

  done:
   __UNLOCK;

  return ret;
}

BPatch_stats &BPatch::getBPatchStatisticsInt()
{
  updateStats();
  return stats;
}
//  updateStats() -- an internal function called before returning
//  statistics buffer to caller of BPatch_getStatistics(),
//  -- just copies global variable statistics counters into 
//  the buffer which is returned to the user.
void BPatch::updateStats() 
{
  stats.pointsUsed = pointsUsed.value();
  stats.totalMiniTramps = totalMiniTramps.value();
  stats.trampBytes = trampBytes.value();
  stats.ptraceOtherOps = ptraceOtherOps.value();
  stats.ptraceOps = ptraceOps.value();
  stats.ptraceBytes = ptraceBytes.value();
  stats.insnGenerated = insnGenerated.value();
}

bool BPatch::registerThreadEventCallbackInt(BPatch_asyncEventType type,
                                            BPatchAsyncThreadEventCallback func)
{
    
    eventType evt;
    switch (type) {
      case BPatch_threadCreateEvent: evt = evtThreadCreate; break;
      case BPatch_threadDestroyEvent: evt = evtThreadExit; break;
      default:
        fprintf(stderr, "%s[%d]:  Cannot register callback for type %s\n",
               FILE__, __LINE__, asyncEventType2Str(type));
        return false; 
    };

    pdvector<CallbackBase *> cbs;
    getCBManager()->removeCallbacks(evt, cbs);
    
    bool ret;
    if (func != 0) {
	AsyncThreadEventCallback *cb = new AsyncThreadEventCallback(func);
	ret = getCBManager()->registerCallback(evt, cb);
    }
    else {
	// For consistency with fork, exit, ... callbacks, treat
	// func == 0 as a remove-all-callbacks-of-type-x request
	ret = true;
    }
    return ret;
}

bool BPatch::removeThreadEventCallbackInt(BPatch_asyncEventType type,
                                          BPatchAsyncThreadEventCallback cb)
{
    eventType evt;
    switch (type) {
      case BPatch_threadCreateEvent: evt = evtThreadCreate; break;
      case BPatch_threadDestroyEvent: evt = evtThreadExit; break;
      default:
        fprintf(stderr, "%s[%d]:  Cannot remove callback for type %s\n",
               FILE__, __LINE__, asyncEventType2Str(type));
        return false; 
    };

    pdvector<CallbackBase *> cbs;
    if (!getCBManager()->removeCallbacks(evt, cbs)) {
        fprintf(stderr, "%s[%d]:  Cannot remove callback for type %s, not found\n",
               FILE__, __LINE__, asyncEventType2Str(type));
        return false;
    }

    //  See if supplied function was in the set of removed functions
    bool ret = false;
    for (int i = cbs.size() -1; i >= 0; i--) {
      AsyncThreadEventCallback *test = (AsyncThreadEventCallback *)cbs[i];
      if (test->getFunc() == cb) {
        //  found it, destroy it
        VECTOR_ERASE(cbs,i,i);
        ret = true;
        delete test;
      } 
    }

    //  we deleted any found target functions, put the others back.
    for (unsigned int i = 0; i < cbs.size(); ++i) 
       if (!getCBManager()->registerCallback(evt, cbs[i]))
          ret = false;

    return ret;
}

bool BPatch::registerDynamicCallCallbackInt(BPatchDynamicCallSiteCallback func)
{
    pdvector<CallbackBase *> cbs;
    DynamicCallsiteCallback *cb = new DynamicCallsiteCallback(func);
    getCBManager()->removeCallbacks(evtDynamicCall, cbs);
    bool ret = getCBManager()->registerCallback(evtDynamicCall, cb);

    return ret;
}

bool BPatch::removeDynamicCallCallbackInt(BPatchDynamicCallSiteCallback func)
{

    pdvector<CallbackBase *> cbs;
    if (!getCBManager()->removeCallbacks(evtDynamicCall, cbs)) {
        fprintf(stderr, "%s[%d]:  Cannot remove callback for type evtDynamicCall, not found\n",
               FILE__, __LINE__);
        return false;
    }

    //  See if supplied function was in the set of removed functions
    bool ret = false;
    for (int i = cbs.size() -1; i >= 0; i--) {
      DynamicCallsiteCallback *test = (DynamicCallsiteCallback *)cbs[i];
      if (test->getFunc() == func) {
        //  found it, destroy it
        VECTOR_ERASE(cbs,i,i);
        ret = true;
        delete test;
      } 
    }

    //  we deleted any found target functions, put the others back.
    for (unsigned int i = 0; i < cbs.size(); ++i) 
       if (!getCBManager()->registerCallback(evtDynamicCall, cbs[i]))
          ret = false;

    return ret;
}

bool BPatch::registerUserEventCallbackInt(BPatchUserEventCallback func)
{
  pdvector<CallbackBase *> cbs;
  UserEventCallback *cb = new UserEventCallback(func);
  bool ret = getCBManager()->registerCallback(evtUserEvent, cb);

  return ret;
}

bool BPatch::removeUserEventCallbackInt(BPatchUserEventCallback cb)
{
    bool ret = false;
    pdvector<CallbackBase *> cbs;
    if (!getCBManager()->removeCallbacks(evtUserEvent, cbs)) {
        fprintf(stderr, "%s[%d]:  Cannot remove callback evtUserEvent, not found\n",
               FILE__, __LINE__);
        return false;
    }

    //  See if supplied function was in the set of removed functions
    for (int i = cbs.size() -1; i >= 0; i--) {
      UserEventCallback *test = (UserEventCallback *)cbs[i];
      if (test->getFunc() == cb) {
        //  found it, destroy it
        VECTOR_ERASE(cbs,i,i);
        ret = true;
        delete test;
      } 
    }

    //  we deleted any found target functions, put the others back.
    for (unsigned int i = 0; i < cbs.size(); ++i) 
       if (!getCBManager()->registerCallback(evtUserEvent, cbs[i]))
          ret = false;

    return ret;
}

bool BPatch::registerCodeDiscoveryCallbackInt(BPatchCodeDiscoveryCallback cb)
{
    std::vector<BPatch_process*> *procs = getProcesses();
    for(unsigned i =0; i < procs->size(); i++) {
        HybridAnalysis *hybrid = (*procs)[i]->getHybridAnalysis();
        hybrid->registerCodeDiscoveryCallback(cb);
    }
    return true;
}

bool BPatch::removeCodeDiscoveryCallbackInt(BPatchCodeDiscoveryCallback)
{
    std::vector<BPatch_process*> *procs = getProcesses();
    for(unsigned i =0; i < procs->size(); i++) {
        HybridAnalysis *hybrid = (*procs)[i]->getHybridAnalysis();
        hybrid->removeCodeDiscoveryCallback();
    }
    return true;
}

bool BPatch::registerSignalHandlerCallbackInt
    (BPatchSignalHandlerCallback bpatchCB, BPatch_Set<long> *signums)
{
    pdvector<CallbackBase *> cbs;
    getCBManager()->removeCallbacks(evtSignalHandlerCB, cbs);
    SignalHandlerCallback *cb = new SignalHandlerCallback
        (HybridAnalysis::getSignalHandlerCB(), signums);
    bool ret = getCBManager()->registerCallback(evtSignalHandlerCB, cb);
    std::vector<BPatch_process*> *procs = getProcesses();
    for(unsigned i=0; i < procs->size(); i++) {
        HybridAnalysis *hybrid = (*procs)[i]->getHybridAnalysis();
        hybrid->registerSignalHandlerCallback(bpatchCB);
    }
    return ret;
}

bool BPatch::removeSignalHandlerCallbackInt(BPatchSignalHandlerCallback)
{
    bool ret = false;
    pdvector<CallbackBase *> cbs;
    if (!getCBManager()->removeCallbacks(evtSignalHandlerCB, cbs)) {
        fprintf(stderr, "%s[%d]:  Cannot remove callback for "
                "evtSignalHandlerCB, not found\n", FILE__, __LINE__);
        return false;
    }

    //  See if supplied function was in the set of removed functions
    for (int i = cbs.size() -1; i >= 0; i--) {
        SignalHandlerCallback *test = (SignalHandlerCallback *)cbs[i];
        //  found it, destroy it
        VECTOR_ERASE(cbs,i,i);
        ret = true;
        delete test;
    }

    //  we deleted any found target functions, put the others back.
    for (unsigned int i = 0; i < cbs.size(); ++i) 
        if (!getCBManager()->registerCallback(evtSignalHandlerCB, cbs[i]))
            ret = false;

    std::vector<BPatch_process*> *procs = getProcesses();
    for(unsigned i=0; i < procs->size(); i++) {
        HybridAnalysis *hybrid = (*procs)[i]->getHybridAnalysis();
        hybrid->removeSignalHandlerCallback();
    }
    return ret;
}

bool BPatch::registerCodeOverwriteCallbacksInt
    (BPatchCodeOverwriteBeginCallback cbBegin,
     BPatchCodeOverwriteEndCallback cbEnd)
{
    pdvector<CallbackBase *> cbs;
    getCBManager()->removeCallbacks(evtCodeOverwrite, cbs);
    CodeOverwriteCallback *cbInt = new CodeOverwriteCallback
        (HybridAnalysisOW::getCodeOverwriteCB());
    bool ret = getCBManager()->registerCallback(evtCodeOverwrite, cbInt);
    std::vector<BPatch_process*> *procs = getProcesses();
    for(unsigned i=0; i < procs->size(); i++) {
        HybridAnalysis *hybrid = (*procs)[i]->getHybridAnalysis();
        hybrid->hybridOW()->registerCodeOverwriteCallbacks(cbBegin,cbEnd);
    }
    return ret;
}

void BPatch::continueIfExists(int pid) 
{
    BPatch_process *proc = getProcessByPid(pid);
    if (!proc) return;

    // Set everything back to the way it was...
    // We use async continue here to _be sure_ that the SH
    // that we're in runs to completion, instead of blocking
    // on someone else.

    if (proc->isAttemptingAStop) {
        return;
    }

    proc->isVisiblyStopped = false;

    proc->llproc->sh->overrideSyncContinueState(runRequest);

    proc->llproc->sh->continueProcessAsync();
}

////////////// Signal FD functions

void BPatch::signalNotificationFD() {
#if !defined(os_windows)
    // If the FDs are set up, write a byte to the input side.
    createNotificationFD();

    if (notificationFDInput_ == -1) return;
    if (FDneedsPolling_) return;

    char f = (char) 42;

    int ret = write(notificationFDInput_, &f, sizeof(char));

    if (ret == -1)
        perror("Notification write");
    else 
        FDneedsPolling_ = true;
#endif
    return;
}

void BPatch::clearNotificationFD() {
#if !defined(os_windows)
    if (notificationFDOutput_ == -1) return;
    if (!FDneedsPolling_) return;
    char buf;

    read(notificationFDOutput_, &buf, sizeof(char));
    FDneedsPolling_ = false;
#endif
    return;
}

void BPatch::createNotificationFD() {
#if !defined(os_windows)
    if (notificationFDOutput_ == -1) {
        assert(notificationFDInput_ == -1);
        int pipeFDs[2];
        pipeFDs[0] = pipeFDs[1] = -1;
        int ret = pipe(pipeFDs);
        if (ret == 0) {
            notificationFDOutput_ = pipeFDs[0];
            notificationFDInput_ = pipeFDs[1];
        }
    }
#endif
}

int BPatch::getNotificationFDInt() {
#if !defined(os_windows)
    createNotificationFD();
    return notificationFDOutput_;
#else
    return -1;
#endif
}

/* If true, we return just filenames when the user asks for line info
   otherwise, we return filename plus path information. */
void BPatch::truncateLineInfoFilenamesInt(bool newval) {
   mapped_module::truncateLineFilenames = newval;
}

void BPatch::getBPatchVersionInt(int &major, int &minor, int &subminor) 
{
   major = DYNINST_MAJOR;
   minor = DYNINST_MINOR;
   subminor = DYNINST_SUBMINOR;
}

BPatch_binaryEdit *BPatch::openBinaryInt(const char *path, bool openDependencies /* = false */) {
   BPatch_binaryEdit *editor = new BPatch_binaryEdit(path, openDependencies);
   if (!editor)
      return NULL;
   if (editor->creation_error) {
      delete editor;
      return NULL;
   }
   return editor;
}

void BPatch::setInstrStackFramesInt(bool r)
{
   instrFrames = r;
}

bool BPatch::getInstrStackFramesInt()
{
   return instrFrames;
}
