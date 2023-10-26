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

#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#if !defined(os_windows)
#include <unistd.h>
#endif

#define BPATCH_FILE
#include "common/src/stats.h"
#include "BPatch.h"
#include "BPatch_libInfo.h"
#include "BPatch_collections.h"
#include "BPatch_thread.h"
#include "debug.h"
#include "mapped_module.h"
#include "instPoint.h"
#include "hybridAnalysis.h"
#include "BPatch_object.h"
#include "os.h"

// ProcControlAPI interface
#include "dynProcess.h"
#include "dynThread.h"
#include "pcEventMuxer.h"

#if defined(i386_unknown_nt4_0)
#include "nt_signal_emul.h"
#endif

#include <fstream>
#include <numeric>

using namespace std;
using namespace SymtabAPI;

extern void loadNativeDemangler();

BPatch *BPatch::bpatch = NULL;

void defaultErrorFunc(BPatchErrorLevel level, int num, const char * const *params);

#ifndef CASE_RETURN_STR
#define CASE_RETURN_STR(x) case x: return #x
#endif

const char *asyncEventType2Str(BPatch_asyncEventType ev) {
    switch(ev) {
        CASE_RETURN_STR(BPatch_nullEvent);
        CASE_RETURN_STR(BPatch_newConnectionEvent);
        CASE_RETURN_STR(BPatch_internalShutDownEvent);
        CASE_RETURN_STR(BPatch_threadCreateEvent);
        CASE_RETURN_STR(BPatch_threadDestroyEvent);
        CASE_RETURN_STR(BPatch_dynamicCallEvent);
    default:
        return "BadEventType";
    }
}

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
    forceSaveFloatingPointsOn(false),
    livenessAnalysisOn_(true),
    livenessAnalysisDepth_(3),
    asyncActive(false),
    delayedParsing_(false),
    instrFrames(false),
    systemPrelinkCommand(NULL),
    notificationFDOutput_(-1),
    notificationFDInput_(-1),
    FDneedsPolling_(false),
    errorCallback(NULL),
    preForkCallback(NULL),
    postForkCallback(NULL),
    execCallback(NULL),
    exitCallback(NULL),
    oneTimeCodeCallback(NULL),
    dynLibraryCallback(NULL),
    threadCreateCallback(NULL),
    threadDestroyCallback(NULL),
    dynamicCallSiteCallback(NULL),
    signalHandlerCallback(NULL),
    codeOverwriteCallback(NULL),
    inDestructor(false),
    builtInTypes(NULL),
    stdTypes(NULL),
    type_Error(NULL),
    type_Untyped(NULL)
{
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
    vector<boost::shared_ptr<Type>> sTypes;
    Symtab::getAllstdTypes(sTypes);
    BPatch_type* type = NULL;
    for(const auto& t: sTypes) {
        stdTypes->addType(type = new BPatch_type(t));
        type->decrRefCount();
    }
    sTypes.clear();

    builtInTypes = new BPatch_builtInTypeCollection;
    Symtab::getAllbuiltInTypes(sTypes);
    for(const auto& t: sTypes) {
        builtInTypes->addBuiltInType(type = new BPatch_type(t));
        type->decrRefCount();
    }

    //loadNativeDemangler();

	// Start up the event handler thread
	PCEventMuxer::start();
}


/*
 * BPatch::~BPatch
 *
 * Destructor for BPatch.  Free allocated memory.
 */
BPatch::~BPatch()
{
   inDestructor = true;
    for(auto i = info->procsByPid.begin(); 
        i != info->procsByPid.end();
        ++i)
    {
       delete i->second;
    }

    delete info;

    type_Error->decrRefCount();
    type_Untyped->decrRefCount();

    if (stdTypes)
        BPatch_typeCollection::freeTypeCollection(stdTypes);
    if (APITypes)
        BPatch_typeCollection::freeTypeCollection(APITypes);
    if(builtInTypes)
      delete builtInTypes;
    
    if(systemPrelinkCommand){
        delete [] systemPrelinkCommand;
    }
    bpatch = NULL;
}

BPatch *BPatch::getBPatch() {
	return bpatch;
}

char * BPatch::getPrelinkCommand(){
	return systemPrelinkCommand;
}

void BPatch::setPrelinkCommand(char *command){

	if(systemPrelinkCommand){
		delete [] systemPrelinkCommand;
	}
	systemPrelinkCommand = new char[strlen(command)+1];
	memcpy(systemPrelinkCommand, command, strlen(command)+1);
}

bool BPatch::isTypeChecked()
{
  return typeCheckOn;
}
void BPatch::setTypeChecking(bool x)
{
  typeCheckOn = x;
}
bool BPatch::parseDebugInfo()
{
  return debugParseOn;
}
bool BPatch::delayedParsingOn()
{
  return delayedParsing_;
}
void BPatch::setDebugParsing(bool x)
{
  debugParseOn = x;
}
bool BPatch::baseTrampDeletion()
{
  return baseTrampDeletionOn;
}
void BPatch::setBaseTrampDeletion(bool x)
{
  baseTrampDeletionOn = x;
}
bool BPatch::isTrampRecursive()
{
  return trampRecursiveOn;
}
void BPatch::setTrampRecursive(bool x)
{
  trampRecursiveOn = x;
}
void BPatch::setLivenessAnalysis(bool x)
{
    livenessAnalysisOn_ = x;
}
bool BPatch::livenessAnalysisOn() {
    return livenessAnalysisOn_;
}

void BPatch::setLivenessAnalysisDepth(int x)
{
    livenessAnalysisDepth_ = x;
}
int BPatch::livenessAnalysisDepth() {
    return livenessAnalysisDepth_;
}

bool BPatch::hasForcedRelocation_NP()
{
  return forceRelocation_NP;
}
void BPatch::setForcedRelocation_NP(bool x)
{
  forceRelocation_NP = x;
}
bool BPatch::autoRelocationOn()
{
  return autoRelocation_NP;
}
void BPatch::setAutoRelocation_NP(bool x)
{
  autoRelocation_NP = x;
}
void BPatch::setDelayedParsing(bool x)
{
  delayedParsing_ = x;
}
bool BPatch::isMergeTramp()
{
  return true;
}
void BPatch::setMergeTramp(bool)
{
}

bool BPatch::isSaveFPROn()
{
  return saveFloatingPointsOn;
}
void BPatch::setSaveFPR(bool x)
{
  saveFloatingPointsOn = x;
}

bool BPatch::isForceSaveFPROn()
{
  return forceSaveFloatingPointsOn;
}
void BPatch::forceSaveFPR(bool x)
{
  forceSaveFloatingPointsOn = x;
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
    BPatchErrorCallback previous = errorCallback;
    errorCallback = function;
    return previous;
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
    BPatchForkCallback previous = postForkCallback;
    postForkCallback = func;
    return previous;
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
    BPatchForkCallback previous = preForkCallback;
    preForkCallback = func;
    return previous;
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
    BPatchExecCallback previous = execCallback;
    execCallback = func;
    return previous;
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
BPatchExitCallback BPatch::registerExitCallback(BPatchExitCallback func)
{
    BPatchExitCallback previous = exitCallback;
    exitCallback = func;
    return previous;
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
    BPatchOneTimeCodeCallback previous = oneTimeCodeCallback;
    oneTimeCodeCallback = func;
    return previous;
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
    BPatchDynLibraryCallback previous = dynLibraryCallback;
    dynLibraryCallback = function;
    return previous;
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


    // don't log BPatchWarning or BPatchInfo messages as "errors"
    if ((severity == BPatchFatal) || (severity == BPatchSerious))
        bpatch->lastError = number;

    if( !BPatch::bpatch->errorCallback ) { 
        fprintf(stdout, "%s[%d]:  DYNINST ERROR:\n %s\n", FILE__, __LINE__, str);
        fflush(stdout);
        return; 
    }

    BPatch::bpatch->errorCallback(severity, number, &str);
    
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
   auto iter = info->procsByPid.find(pid);
   if (iter != info->procsByPid.end()) {
      if (exists) *exists = true;
      BPatch_process *proc = iter->second;
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
 * BPatch::getProcs
 *
 * Returns a vector of all threads that are currently defined.  Includes
 * threads created directly using the library and those created with UNIX fork
 * or Windows NT spawn system calls.  The caller is responsible for deleting
 * the vector when it is no longer needed.
 */
BPatch_Vector<BPatch_process *> *BPatch::getProcesses()
{
   BPatch_Vector<BPatch_process *> *result = new BPatch_Vector<BPatch_process *>;
   for (auto iter = info->procsByPid.begin(); iter != info->procsByPid.end(); ++iter) {
      result->push_back(iter->second);
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
   assert(info->procsByPid.find(pid) == info->procsByPid.end());
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
void BPatch::registerForkedProcess(PCProcess *parentProc, PCProcess *childProc)
{
    int parentPid = parentProc->getPid();
    int childPid = childProc->getPid();

    proccontrol_printf("BPatch: registering fork, parent %d, child %d\n",
                    parentPid, childPid);
    assert(getProcessByPid(childPid) == NULL);
    
    BPatch_process *parent = getProcessByPid(parentPid);
    assert(parent);

    BPatch_process *child = new BPatch_process(childProc);
    child->triggerInitialThreadEvents();

    if( postForkCallback ) {
        postForkCallback(parent->threads[0], child->threads[0]);
    }
    
    proccontrol_printf("BPatch: finished registering fork, parent %d, child %d\n",
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
void BPatch::registerForkingProcess(int forkingPid, PCProcess * /*proc*/)
{
    BPatch_process *forking = getProcessByPid(forkingPid);
    assert(forking);

    if( preForkCallback ) {
        preForkCallback(forking->threads[0], NULL);
    }
}


/*
 * BPatch::registerExecCleanup
 *
 * Register a process that has just entered exec
 *
 * Gives us some cleanup time
 */

void BPatch::registerExecCleanup(PCProcess *p, char *) 
{
    BPatch_process *execing = getProcessByPid(p->getPid());
    assert(execing);

    for (unsigned i=0; i<execing->threads.size(); i++)
       registerThreadExit(p, execing->threads[i]->llthread);

}    

/*
 * BPatch::registerExecExit
 *
 * Register a process that has just done an exec call.
 *
 * proc - the representation of the process after the exec
 */
void BPatch::registerExecExit(PCProcess *proc) {
    int execPid = proc->getPid();
    BPatch_process *process = getProcessByPid(execPid);
    assert(process);

    assert( process->threads.size() <= 1 );

    // There is a new underlying process representation
    process->llproc = proc;
    PCThread *thr = proc->getInitialThread();

    // Create a new initial thread or update it
    BPatch_thread *initialThread;
    if( process->threads.size() == 0 ) { 
        initialThread = new BPatch_thread(process, thr);
        process->threads.push_back(initialThread);
    }else{
        initialThread = process->getThreadByIndex(0);
        initialThread->updateThread(thr);
    }

    // build a new BPatch_image for this one
    if (process->image)
        process->image->removeAllModules();

    BPatch_image *oldImage = process->image;
    process->image = new BPatch_image(process);
    if( oldImage ) delete oldImage;

    assert( proc->isBootstrapped() );

    // ProcControlAPI doesn't deliver callbacks for the initial thread,
    // even if the mutatee is multithread capable
    if( proc->multithread_capable() ) {
        registerThreadCreate(process, initialThread);
    }

    if( execCallback ) {
        execCallback(process->threads[0]);
    }
}

void BPatch::registerNormalExit(PCProcess *proc, int exitcode)
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

   if (thrd) {
	   if( threadDestroyCallback && !thrd->madeExitCallback() ) {
          threadDestroyCallback(process, thrd);
      }
   }

   if( exitCallback ) {
       exitCallback(process->threads[0], ExitedNormally);
   }

   // We now run the process out; set its state to terminated. Really, the user shouldn't
   // try to do anything else with this, but we can get that happening.
   BPatch_process *stillAround = getProcessByPid(pid);
   if (stillAround) {
      stillAround->reportedExit = true;
      stillAround->terminated = true;
   }
}

void BPatch::registerSignalExit(PCProcess *proc, int signalnum)
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
   bpprocess->terminated = true;

   if (thrd) {
	   if( threadDestroyCallback && !thrd->madeExitCallback() ) {
          threadDestroyCallback(bpprocess, thrd);
      }
      if( exitCallback ) {
          exitCallback(bpprocess->threads[0], ExitedViaSignal);
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
   if( threadCreateCallback ) {
       threadCreateCallback(proc, newthr);
   }

   return true;
}

void BPatch::registerThreadExit(PCProcess *llproc, PCThread *llthread)
{
    assert( llproc && llthread );
    
    BPatch_process *bpprocess = getProcessByPid(llproc->getPid());
    
    if (!bpprocess) {
        // Error during startup can cause this -- we have a partially
        // constructed process object, but it was never registered with
        // bpatch
        return;
    }

    BPatch_thread *thrd = bpprocess->getThread(llthread->getTid());
    if (!thrd) {
        //If we don't have an BPatch thread, then it might have been an internal
        // thread that we decided not to report to the user (happens during 
        //  windows attach).  Just trigger the lower level clean up in this case.
        llproc->removeThread(llthread->getTid());
        return;
    }

	if (thrd->madeExitCallback()) return;

    if( threadDestroyCallback ) {
        threadDestroyCallback(bpprocess, thrd);
    }

	thrd->setMadeExitCallback();
    bpprocess->deleteBPThread(thrd);
}


void BPatch::registerUserEvent(BPatch_process *process, void *buffer,
                       unsigned int bufsize)
{
    for(unsigned i = 0; i < userEventCallbacks.size(); ++i) {
        (userEventCallbacks[i])(process, buffer, bufsize);
    }
}

void BPatch::registerDynamicCallsiteEvent(BPatch_process *process, Address callTarget,
                       Address callAddr)
{
    // find the point that triggered the event

    proccontrol_printf("%s[%d]: dynamic call event from 0x%lx to 0x%lx\n",
            FILE__, __LINE__, callAddr, callTarget);
    BPatch_point *point = info->getMonitoredPoint(callAddr);
    if ( point == NULL ) {
        proccontrol_printf("%s[%d]: failed to find point for dynamic callsite event\n",
                FILE__, __LINE__);
        return;
    }

    func_instance *targetFunc = process->llproc->findOneFuncByAddr(callTarget);
    if( targetFunc == NULL ) {
        proccontrol_printf("%s[%d]: failed to find dynamic call target function\n",
                FILE__, __LINE__);
        return;
    }

    BPatch_function *bpatchTargetFunc = process->findOrCreateBPFunc(targetFunc, NULL);
    if( bpatchTargetFunc == NULL ) {
        proccontrol_printf("%s[%d]: failed to find BPatch target function\n",
                FILE__, __LINE__);
        return;
    }

    if( dynamicCallSiteCallback ) {
        dynamicCallSiteCallback(point, bpatchTargetFunc);
    }
}

/*
 * BPatch::registerLoadedModule
 *
 * Register a new module loaded by a process (e.g., dlopen)
 */

void BPatch::registerLoadedModule(PCProcess *process, mapped_object *obj) {

    BPatch_process *bProc = BPatch::bpatch->getProcessByPid(process->getPid());
    if (!bProc) return; // Done

    // Squash this notification if the PCProcess has changed (e.g. during exec)
    if (bProc->llproc != process) return;

    BPatch_image *bImage = bProc->getImage();
    assert(bImage); // This we can assert to be true
    
    BPatch_object *bpobj = bImage->findOrCreateObject(obj);

    if( dynLibraryCallback ) {
        dynLibraryCallback(bProc->threads[0], bpobj, true);
    }
}

/*
 * BPatch::registerUnloadedModule
 *
 * Register a new module loaded by a process (e.g., dlopen)
 */

void BPatch::registerUnloadedModule(PCProcess *process, mapped_object *obj) {

    BPatch_process *bProc = BPatch::bpatch->getProcessByPid(process->getPid());
    if (!bProc) return; // Done

    // Squash this notification if the PCProcess has changed (e.g. during exec)
    if (bProc->llproc != process) return;

    BPatch_image *bImage = bProc->getImage();
    if (!bImage) { // we got an event during process startup
        return;
    }
    
    BPatch_object *bpobj = bImage->findObject(obj);
    if (bpobj == NULL) return;

    
    // For now we use the same callback for load and unload of library....
    if( dynLibraryCallback ) {
        dynLibraryCallback(bProc->threads[0], bpobj, false);
    }

    bImage->removeObject(bpobj);
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

   assert(info->procsByPid.find(pid) == info->procsByPid.end());
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
   // DO NOT CHANGE THE MAP!
   if (inDestructor) return;

   if (pid == -1 || (info->procsByPid.find(pid) == info->procsByPid.end())) {
      // Deleting an exited process; search and nuke
      for (auto iter = info->procsByPid.begin(); iter != info->procsByPid.end(); ++iter) {
         if (iter->second == proc) {
            info->procsByPid.erase(iter);
            return;
         }
      }
      if (pid != -1) {
         char ebuf[256];
         sprintf(ebuf, "%s[%d]: no process %d defined in procsByPid\n", FILE__, __LINE__, pid);
         reportError(BPatchFatal, 68, ebuf);
         return;
      }
   }
   info->procsByPid.erase(pid);
}

static void buildPath(const char *path, const char **argv,
                      char * &pathToUse,
                      char ** &argvToUse) {
   ifstream file;
   file.open(path);
   if (!file.is_open()) return;
   std::string line;
   getline(file, line);
   if (line.compare(0, 2, "#!") != 0) {
      file.close();
      return;
   }

   // A shell script, so reinterpret path/argv

   // Modeled after Linux's fs/binfmt_script.c
   // #! lines have the interpreter and optionally a single argument,
   // all separated by spaces and/or tabs.

   size_t pos_start = line.find_first_not_of(" \t", 2);
   if (pos_start == std::string::npos) {
      file.close();
      return;
   }
   size_t pos_end = line.find_first_of(" \t", pos_start);
   std::string interp = line.substr(pos_start, pos_end - pos_start);
   pathToUse = strdup(interp.c_str());

   std::string interp_arg;
   pos_start = line.find_first_not_of(" \t", pos_end);
   if (pos_start != std::string::npos) {
      // The argument goes all the way to the last non-space/tab,
      // even if there are spaces/tabs in the middle somewhere.
      pos_end = line.find_last_not_of(" \t") + 1;
      interp_arg = line.substr(pos_start, pos_end - pos_start);
   }

   // Count the old and new argc values
   int argc = 0;
   while(argv[argc] != NULL) {
      argc++;
   }
   int argcToUse = argc + 1;
   if (!interp_arg.empty()) {
      argcToUse++;
   }
   argvToUse = (char **) malloc((argcToUse+1) * sizeof(char *));

   // The interpreter takes the new argv[0]
   int argi = 0;
   argvToUse[argi++] = strdup(pathToUse);

   // If there's an interpreter argument, that's the new argv[1]
   if (!interp_arg.empty()) {
      argvToUse[argi++] = strdup(interp_arg.c_str());
   }

   // Then comes path, *replacing* the old argv[0],
   // and the old argv[1..] are filled in for the rest
   argvToUse[argi++] = strdup(path);
   for (int tmp = 1; tmp < argc; ++tmp) {
      argvToUse[argi++] = strdup(argv[tmp]);
   }
   argvToUse[argcToUse] = NULL;
   file.close();
}

/*
 * BPatch::processCreate
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
BPatch_process *BPatch::processCreate(const char *path, const char *argv[], 
                                         const char **envp, int stdin_fd, 
                                         int stdout_fd, int stderr_fd,
                                         BPatch_hybridMode mode)
{
   clearError();

    if (!OS_isConnected()) {
        reportError(BPatchFatal, 68, "Attempted to create process before connected to target server\n");
        return NULL;
    }

   if ( path == NULL ) { return NULL; }

#if !defined (os_windows)
   //  This might be ok on windows...  not 100% sure and it takes
   //  to long to build for the moment.

   //  just a sanity check for the exitence of <path>
   struct stat statbuf;
   if (-1 == stat(path, &statbuf)) {
      auto msg = std::string("createProcess(") + path + ",...):  file does not exist\n";
      reportError(BPatchFatal, 68, msg.c_str());
      return NULL;
   }

   //  and ensure its a regular file:
   if (!S_ISREG(statbuf.st_mode)) {
      auto msg = std::string("createProcess(") + path + ",...):  not a regular file\n";
      reportError(BPatchFatal, 68, msg.c_str());
      return NULL;
   }

   //  and ensure its executable (does not check permissions):
   if (! ( (statbuf.st_mode & S_IXUSR)
            || (statbuf.st_mode & S_IXGRP)
            || (statbuf.st_mode & S_IXOTH) )) {
      auto msg = std::string("createProcess(") + path + "%s,...):  not an executable\n";
      reportError(BPatchFatal, 68, msg.c_str());
      return NULL;
   }

#endif // !Windows

   // User request: work on scripts by creating the interpreter instead
   char *pathToUse = NULL;
   char **argvToUse = NULL;

   buildPath(path, argv, pathToUse, argvToUse);

   BPatch_process *ret = 
      new BPatch_process((pathToUse ? pathToUse : path), 
                         (argvToUse ? (const_cast<const char **>(argvToUse)) : argv), 
                         mode, envp, stdin_fd,stdout_fd,stderr_fd);
   
   if (pathToUse) free(pathToUse);
   if (argvToUse) {

      int tmp = 0;
      while(argvToUse[tmp] != NULL) {
         free(argvToUse[tmp]);
         tmp++;
      }
      free(argvToUse);
   }

   if (!ret->llproc 
         ||  !ret->llproc->isStopped()
         ||  !ret->llproc->isBootstrapped()) {
      delete ret;
      reportError(BPatchFatal, 68, "create process failed bootstrap");
      return NULL;
   }

   ret->triggerInitialThreadEvents();

   if (ret->lowlevel_process()->isExploratoryModeOn()) {
       if (!ret->getHybridAnalysis()->init()) {
           delete ret;
           reportError(BPatchFatal, 68, "create process failed defensive instrumentation");
           return NULL;
       }
   }

   return ret;
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
BPatch_process *BPatch::processAttach
(const char *path, int pid, BPatch_hybridMode mode)
{
   clearError();

    if (!OS_isConnected()) {
        reportError(BPatchFatal, 68, "Error: Attempted to attach to process before connected to target server.");
        return NULL;
    }

    if (info->procsByPid.find(pid) != info->procsByPid.end()) {
      char msg[256];
      sprintf(msg, "attachProcess failed.  Dyninst is already attached to %d.",
              pid);
      reportError(BPatchWarning, 26, msg);      
      return NULL;
   }

   BPatch_process *ret = new BPatch_process(path, pid, mode);

   if (!ret->llproc ||
       !ret->llproc->isStopped() ||
       !ret->llproc->isBootstrapped()) {
       char msg[256];
       sprintf(msg,"attachProcess failed: process %d may now be killed!",pid);
       reportError(BPatchWarning, 26, msg);
	   
	   delete ret;
       return NULL;
   }

   ret->triggerInitialThreadEvents();

   if (ret->lowlevel_process()->isExploratoryModeOn()) {
       ret->getHybridAnalysis()->init();
   }

   return ret;
}

static bool recursiveEventHandling = false;

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
    // Sanity check: don't allow waiting for events in the callbacks
    if( recursiveEventHandling ) {
        BPatch_reportError(BPatchWarning, 0,
                "Cannot wait for events in a callback");
        return false;
    }

    proccontrol_printf("[%s:%d] Polling for events\n", FILE__, __LINE__);

    recursiveEventHandling = true;
    PCEventMuxer::WaitResult result = PCEventMuxer::wait(false);
    recursiveEventHandling = false;

    if( result == PCEventMuxer::Error ) {
        proccontrol_printf("[%s:%d] Failed to poll for events\n",
                FILE__, __LINE__);
        BPatch_reportError(BPatchWarning, 0, 
                "Failed to handle events and deliver callbacks");
        return false;
    }


    if( result == PCEventMuxer::EventsReceived ) {
        proccontrol_printf("[%s:%d] Events received\n", FILE__, __LINE__);
        return true;
    }
  
    proccontrol_printf("[%s:%d] No events available\n", FILE__, __LINE__);
    return false;
}

/*
 * waitForStatusChange
 *
 * Blocks waiting for a change to occur in the running status of a child
 * process.  Returns true upon success, false upon failure.
 */
bool BPatch::waitForStatusChange() {
    // Sanity check: don't allow waiting for events in the callbacks
    if( recursiveEventHandling ) {
        BPatch_reportError(BPatchWarning, 0,
                "Cannot wait for events in a callback");
        return false;
    }

    // Sanity check: make sure there are processes running that could
    // cause events to occur, otherwise the user will be waiting indefinitely
    bool processRunning = false;
    for(auto i = info->procsByPid.begin(); i != info->procsByPid.end(); ++i) 
    {
       if( !i->second->isStopped() &&
	   !i->second->isTerminated()) {
          processRunning = true;
          break;
       }
    }

    if( !processRunning ) {
        BPatch_reportError(BPatchWarning, 0,
                "No processes running, not waiting for events");
		return false;
    }

    proccontrol_printf("%s:[%d] Waiting for events\n", FILE__, __LINE__);

    recursiveEventHandling = true;
    PCEventMuxer::WaitResult result = PCEventMuxer::wait(true);
    recursiveEventHandling = false;

    if( result == PCEventMuxer::Error ) {
        proccontrol_printf("%s:[%d] Failed to wait for events\n",
                      FILE__, __LINE__);
        BPatch_reportError(BPatchWarning, 0,
							"Failed to handle events and deliver callbacks");
		return false;
    }


    if( result == PCEventMuxer::EventsReceived ) {
        proccontrol_printf("%s:[%d] Events received in waitForStatusChange\n", FILE__, __LINE__);
        return true;
    }
    else {
        proccontrol_printf("%s:[%d] No events received in waitForStatusChange\n", FILE__, __LINE__);
        return true;
    }
    //  we waited for a change, but didn't get it
    proccontrol_printf("%s[%d]:  Error in status change reporting\n", FILE__, __LINE__);
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
BPatch_type * BPatch::createEnum( const char * name, 
				     BPatch_Vector<char *> &elementNames,
				     BPatch_Vector<int> &elementIds)
{
    if (elementNames.size() != elementIds.size()) {
      return NULL;
    }

    // Make the underlying type a 4-byte signed int
    boost::shared_ptr<Type> underlying_type = boost::make_shared<typeScalar>(4, "int", true);

    auto *tenum = new typeEnum(underlying_type, name);
    for(auto i=0UL; i<elementNames.size(); i++) {
    	tenum->addConstant(elementNames[i], elementIds[i]);
    }

    BPatch_type *newType = new BPatch_type(tenum);
    APITypes->addType(newType);
    return newType;
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
				        BPatch_Vector<char *> &elementNames)
{
	// We were only given names, so assume sequentially-ordered values
	BPatch_Vector<int> ids(elementNames.size());
	std::iota(ids.begin(), ids.end(), 0);
	return createEnum(name, elementNames, ids);
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
				       BPatch_Vector<char *> &fieldNames,
				       BPatch_Vector<BPatch_type *> &fieldTypes)
{
   unsigned int i;
   
   if (fieldNames.size() != fieldTypes.size()) {
      return NULL;
   }
   
   string typeName = name;
   dyn_c_vector<pair<string, boost::shared_ptr<Type> > *> fields;
   for(i=0; i<fieldNames.size(); i++)
   {
      if(!fieldTypes[i])
         return NULL;
      fields.push_back(new pair<string, boost::shared_ptr<Type>>(fieldNames[i], fieldTypes[i]->getSymtabType(Type::share)));
   }	
   
   boost::shared_ptr<Type> typ(typeStruct::create(typeName, fields));
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

BPatch_type * BPatch::createUnion( const char * name, 
				      BPatch_Vector<char *> &fieldNames,
				      BPatch_Vector<BPatch_type *> &fieldTypes)
{
    unsigned int i;
    
    if (fieldNames.size() != fieldTypes.size()) {
      return NULL;
    }

    string typeName = name;
    dyn_c_vector<pair<string, boost::shared_ptr<Type> > *> fields;
    for(i=0; i<fieldNames.size(); i++)
    {
        if(!fieldTypes[i])
	    return NULL;
        fields.push_back(new pair<string, boost::shared_ptr<Type> > (fieldNames[i], fieldTypes[i]->getSymtabType(Type::share)));
    }	
    
    boost::shared_ptr<Type> typ(typeUnion::create(typeName, fields));
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
BPatch_type * BPatch::createArray( const char * name, BPatch_type * ptr,
				      unsigned int low, unsigned int hi)
{

    BPatch_type * newType;
    if (!ptr) 
        return NULL;
        
    string typeName = name;
    boost::shared_ptr<Type> typ(typeArray::create(typeName, ptr->getSymtabType(Type::share), low, hi));
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
BPatch_type * BPatch::createPointer(const char * name, BPatch_type * ptr,
                                       int /*size*/)
{
    BPatch_type * newType;
    if(!ptr)
        return NULL;
    
    string typeName = name;
    boost::shared_ptr<Type> typ(typePointer::create(typeName, ptr->getSymtabType(Type::share)));
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

BPatch_type * BPatch::createScalar( const char * name, int size)
{
    BPatch_type * newType;
    
    string typeName = name;
    boost::shared_ptr<Type> typ(typeScalar::create(typeName, size));
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
BPatch_type * BPatch::createTypedef( const char * name, BPatch_type * ptr)
{
    BPatch_type * newType;
    if(!ptr)
        return NULL;
    
    string typeName = name;
    boost::shared_ptr<Type> typ(typeTypedef::create(typeName, ptr->getSymtabType(Type::share)));
    if (!typ) return NULL;
    
    newType = new BPatch_type(typ);
    if (!newType) return NULL;

    APITypes->addType(newType);
    return newType;
}

bool BPatch::waitUntilStopped(BPatch_thread *appThread){

   bool ret = false;

   while (1) {
     if (!appThread->getProcess()->isStopped() && !appThread->getProcess()->isTerminated()) {
       this->waitForStatusChange();
     }
     else {
       break;
     }
   }

   if (!appThread->getProcess()->isStopped())
	{
		cerr << "ERROR : process did not signal mutator via stop"
		     << endl;
		ret = false;
 		goto done;
	}
#if defined(os_windows)
	else if((appThread->getProcess()->stopSignal() != EXCEPTION_BREAKPOINT) && 
		(appThread->getProcess()->stopSignal() != -1))
	{
		cerr << "ERROR : process stopped on signal different"
		     << " than SIGTRAP" << endl;
		ret =  false;
 		goto done;
	}
#else
	else if ((appThread->getProcess()->stopSignal() != SIGSTOP) &&
		 (appThread->getProcess()->stopSignal() != SIGHUP)) {
		cerr << "ERROR :  process stopped on signal "
		     << "different than SIGSTOP" << endl;
		ret =  false;
 		goto done;
	}
#endif

  done:
  return ret;
}

BPatch_stats &BPatch::getBPatchStatistics()
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

bool BPatch::registerThreadEventCallback(BPatch_asyncEventType type,
                                            BPatchAsyncThreadEventCallback func)
{
    switch(type) {
        case BPatch_threadCreateEvent:
            threadCreateCallback = func;
            break;
        case BPatch_threadDestroyEvent:
            threadDestroyCallback = func;
            break;
        default:
            bpwarn("Cannot register callback for non-thread event type %s",
                    asyncEventType2Str(type));
            return false;
    }

    return true;
}

bool BPatch::removeThreadEventCallback(BPatch_asyncEventType type,
                                          BPatchAsyncThreadEventCallback cb)
{
    bool result = false;
    switch(type) {
        case BPatch_threadCreateEvent:
            if( cb == threadCreateCallback ) {
                threadCreateCallback = NULL;
                result = true;
            }
            break;
        case BPatch_threadDestroyEvent:
            if( cb == threadDestroyCallback ) {
                threadDestroyCallback = NULL;
                result = true;
            }
            break;
        default:
            bpwarn("Cannot remove callback for non-thread event type %s",
                    asyncEventType2Str(type));
            return false;
    }

    return result;
}

bool BPatch::registerDynamicCallCallback(BPatchDynamicCallSiteCallback func)
{
    dynamicCallSiteCallback = func;
    return true;
}

bool BPatch::removeDynamicCallCallback(BPatchDynamicCallSiteCallback func)
{
    if( dynamicCallSiteCallback == func ) {
        dynamicCallSiteCallback = func;
        return true;
    }

    return false;
}

bool BPatch::registerUserEventCallback(BPatchUserEventCallback func)
{
    userEventCallbacks.push_back(func);
    return true;
}

bool BPatch::removeUserEventCallback(BPatchUserEventCallback cb)
{
    bool result = false;
    BPatch_Vector<BPatchUserEventCallback> userCallbacks;
    for(unsigned int i = 0; i < userEventCallbacks.size(); ++i) {
        if( cb != userEventCallbacks[i] ) {
            userCallbacks.push_back(userEventCallbacks[i]);
        }else{
            result = true;
        }
    }

    userEventCallbacks = userCallbacks;

    return result;
}

bool BPatch::registerCodeDiscoveryCallback(BPatchCodeDiscoveryCallback cb)
{
    std::vector<BPatch_process*> *procs = getProcesses();
    for(unsigned i =0; i < procs->size(); i++) {
        HybridAnalysis *hybrid = (*procs)[i]->getHybridAnalysis();
        hybrid->registerCodeDiscoveryCallback(cb);
    }
    return true;
}

bool BPatch::removeCodeDiscoveryCallback(BPatchCodeDiscoveryCallback)
{
    std::vector<BPatch_process*> *procs = getProcesses();
    for(unsigned i =0; i < procs->size(); i++) {
        HybridAnalysis *hybrid = (*procs)[i]->getHybridAnalysis();
        hybrid->removeCodeDiscoveryCallback();
    }
    return true;
}

bool BPatch::registerSignalHandlerCallback(BPatchSignalHandlerCallback bpatchCB, 
                                           std::set<long> &signums)
{
    signalHandlerCallback = HybridAnalysis::getSignalHandlerCB();
    callbackSignals = signums;

    std::vector<BPatch_process*> *procs = getProcesses();
    for(unsigned i=0; i < procs->size(); i++) {
        HybridAnalysis *hybrid = (*procs)[i]->getHybridAnalysis();
        hybrid->registerSignalHandlerCallback(bpatchCB);
    }
    return true;
}

bool BPatch::registerSignalHandlerCallback(BPatchSignalHandlerCallback bpatchCB, 
                                           BPatch_Set<long> *signums) {
   // This is unfortunate, but our method above takes a std::set<long>,
   // not a std::set<long, comparison<long>>
   
   std::set<long> tmp;
   if (NULL == signums || signums->empty())
	   tmp = std::set<long>();
   else
       std::copy(signums->begin(), signums->end(), std::inserter(tmp, tmp.end()));
   
   return registerSignalHandlerCallback(bpatchCB, tmp);
}

bool BPatch::removeSignalHandlerCallback(BPatchSignalHandlerCallback)
{
    signalHandlerCallback = NULL;
    callbackSignals.clear();

    std::vector<BPatch_process*> *procs = getProcesses();
    for(unsigned i=0; i < procs->size(); i++) {
        HybridAnalysis *hybrid = (*procs)[i]->getHybridAnalysis();
        hybrid->removeSignalHandlerCallback();
    }
    return true;
}

bool BPatch::registerCodeOverwriteCallbacks
    (BPatchCodeOverwriteBeginCallback cbBegin,
     BPatchCodeOverwriteEndCallback cbEnd)
{
    codeOverwriteCallback = HybridAnalysisOW::getCodeOverwriteCB();

    std::vector<BPatch_process*> *procs = getProcesses();
    for(unsigned i=0; i < procs->size(); i++) {
        HybridAnalysis *hybrid = (*procs)[i]->getHybridAnalysis();
        hybrid->hybridOW()->registerCodeOverwriteCallbacks(cbBegin,cbEnd);
    }
    return true;
}

void BPatch::continueIfExists(int pid) 
{
    BPatch_process *proc = getProcessByPid(pid);
    if (!proc) return;

    proc->continueExecution();
}


int BPatch::getNotificationFD() {
#if !defined(os_windows)
   return Dyninst::ProcControlAPI::evNotify()->getFD(); 
#else
    return -1;
#endif
}

void BPatch::truncateLineInfoFilenames(bool) {}

void BPatch::getBPatchVersion(int &major, int &minor, int &subminor) 
{
   major = DYNINST_MAJOR;
   minor = DYNINST_MINOR;
   subminor = DYNINST_SUBMINOR;
}

BPatch_binaryEdit *BPatch::openBinary(const char *path, bool openDependencies /* = false */) {
   BPatch_binaryEdit *editor = new BPatch_binaryEdit(path, openDependencies);
   if (!editor)
      return NULL;
   if (editor->creation_error) {
      delete editor;
      return NULL;
   }
   return editor;
}

void BPatch::setInstrStackFrames(bool r)
{
   instrFrames = r;
}

bool BPatch::getInstrStackFrames()
{
   return instrFrames;
}

bool BPatch::isConnected()
{
    return OS_isConnected();
}

// -----------------------------------------------------------
// Undocumented public remote debugging interface.
// See comments in BPatch.h about the future of these methods.
bool BPatch::remoteConnect(BPatch_remoteHost &remote)
{
    if (remote.type >= BPATCH_REMOTE_DEBUG_END) {
        fprintf(stderr, "Unknown remote debugging protocol %d\n", remote.type);
        return false;
    }

    return OS_connect(remote);
}

bool BPatch::getPidList(BPatch_remoteHost &remote, BPatch_Vector<unsigned int> &pidlist)
{
    if (remote.type >= BPATCH_REMOTE_DEBUG_END) {
        fprintf(stderr, "Unknown remote debugging protocol %d\n", remote.type);
        return false;
    }

    return OS_getPidList(remote, pidlist);
}

bool BPatch::getPidInfo(BPatch_remoteHost &remote, unsigned int pid,
                           std::string &pidInfo)
{
    if (remote.type >= BPATCH_REMOTE_DEBUG_END) {
        fprintf(stderr, "Unknown remote debugging protocol %d\n", remote.type);
        return false;
    }

    return OS_getPidInfo(remote, pid, pidInfo);
}

bool BPatch::remoteDisconnect(BPatch_remoteHost &remote)
{
    if (remote.type >= BPATCH_REMOTE_DEBUG_END) {
        fprintf(stderr, "Unknown remote debugging protocol %d\n", remote.type);
        return false;
    }

    return OS_disconnect(remote);
}
// -----------------------------------------------------------

void BPatch::addNonReturningFunc(std::string name)
{
  Dyninst::ParseAPI::SymtabCodeSource::addNonReturning(name);
}

int BPatch_libInfo::getStopThreadCallbackID(Address cb) {
   auto iter = stopThreadCallbacks_.find(cb);
   if (iter != stopThreadCallbacks_.end()) {
      return iter->second;
   }

    int cb_id = ++stopThreadIDCounter_;
    stopThreadCallbacks_[cb] = cb_id;
    return cb_id;
}

bool BPatch_libInfo::registerMonitoredPoint(BPatch_point *point) {
   if (monitoredPoints_.find((Address) point->getAddress()) != monitoredPoints_.end())
      return false;

    monitoredPoints_[(Address)point->getAddress()] = point;

    proccontrol_printf("%s[%d]: monitoring address 0x%lx for dynamic calls\n",
            FILE__, __LINE__, (unsigned long)point->getAddress());

    return true;
}

BPatch_point *BPatch_libInfo::getMonitoredPoint(Address addr) {
   auto iter = monitoredPoints_.find(addr);
   if (iter == monitoredPoints_.end()) return NULL;
   return iter->second;
}

// Functions for accessing stop thread callback state
void BPatch::registerStopThreadCallback(BPatchStopThreadCallback stopCB) {
    stopThreadCallbacks.push_back(stopCB);
}

int BPatch::getStopThreadCallbackID(BPatchStopThreadCallback stopCB) {
    return info->getStopThreadCallbackID((Address)stopCB);
}
