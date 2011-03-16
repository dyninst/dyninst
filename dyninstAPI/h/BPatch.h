/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

#ifndef _BPatch_h_
#define _BPatch_h_

#include <stdio.h>
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_type.h"
#include "BPatch_eventLock.h"
#include "BPatch_process.h"
#include "BPatch_hybridAnalysis.h"

class BPatch_typeCollection;
class BPatch_libInfo;
class BPatch_module;
class int_function;
class process;

//Keep old versions defined, that way someone can test if we're more
// at or more recent than version 5.1 with '#if defined(DYNINST_5_1)'
//If they want to get the current version, they should use DYNINST_MAJOR,
// DYNINST_MINOR, and DYNINST_SUBMINOR
#define DYNINST_5_1
#define DYNINST_5_2
#define DYNINST_6_0
#define DYNINST_6_1
#define DYNINST_7_0

#define DYNINST_MAJOR 7
#define DYNINST_MINOR 0
#define DYNINST_SUBMINOR 0

#ifdef IBM_BPATCH_COMPAT
typedef void *BPatch_Address;
#endif

//  BPatch_stats is a collection of instrumentation statistics.
//  Introduced to export this information to paradyn, which 
//  produces a summary of these numbers upon application exit.
//  It probably makes more sense to maintain such numbers on a
//  per-process basis.  But is set up globally due to historical
//  precendent.   

typedef struct {
  unsigned int pointsUsed;
  unsigned int totalMiniTramps;
  unsigned int trampBytes;
  unsigned int ptraceOtherOps;
  unsigned int ptraceOps;
  unsigned int ptraceBytes;
  unsigned int insnGenerated;
} BPatch_stats;

// --------------------------------------------------------------------
// This is a purposefully undocumented prototype of a "remote debugging"
// interface.  Meant to generalize debuggers like remote gdb and wtx.

typedef enum {
    BPATCH_REMOTE_DEBUG_WTX,

    BPATCH_REMOTE_DEBUG_END
} BPatch_remote_t;

typedef struct {
    char *target;
    char *tool;
    char *host;
} BPatch_remoteWtxInfo;

typedef struct {
    BPatch_remote_t type;
    void *info;
} BPatch_remoteHost;
// --------------------------------------------------------------------

class EventRecord;

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch
class BPATCH_DLL_EXPORT BPatch : public BPatch_eventLock {
    friend class BPatch_thread;
    friend class BPatch_process;
    friend class BPatch_point;
    friend class process;
    friend class int_function;
    friend class SignalHandler;
    friend class BPatch_asyncEventHandler;
    friend bool handleSigStopNInt(EventRecord &ev);

    BPatch_libInfo *info; 

    bool	typeCheckOn;
    int		lastError;
    bool	debugParseOn;
    bool	baseTrampDeletionOn;

    /* If true, trampolines can recurse to their heart's content.
       Defaults to false */
    bool        trampRecursiveOn;

    bool        forceRelocation_NP;
    /* If true, allows automatic relocation of functions if dyninst
       deems it necessary.  Defaults to true */
    bool        autoRelocation_NP;

    /* If true, we save FPRs in situations we normally would 
       Defaults to true */
    bool saveFloatingPointsOn;
    bool forceSaveFloatingPointsOn;

    /* If true, we will use liveness calculations to avoid saving
       registers on platforms that support it. 
       Defaults to true. */
    bool livenessAnalysisOn_;
    /* How far through the CFG do we follow calls? */
    int livenessAnalysisDepth_;

    /* If true, override requests to block while waiting for events,
       polling instead */
    bool asyncActive;

    /* If true, deep parsing (anything beyond symtab info) is delayed until
       accessed */
    /* Note: several bpatch constructs have "access everything" behavior, 
       which will trigger full parsing. This should be looked into. */
    bool delayedParsing_;

    bool instrFrames;

    BPatch_stats stats;
    void updateStats();

	/* this is used to denote the fully qualified name of the prelink command on linux */
	char *systemPrelinkCommand;

        // Wrapper - start process running if it was not deleted. 
        // We use this at the end of callbacks to user code, since those
        // callbacks may delete BPatch objects. 
        void continueIfExists(int pid);

    /* flag that is set when a mutatee's runnning status changes,
       for use with pollForStatusChange */
   bool mutateeStatusChange;
   bool waitingForStatusChange;

   /* Internal notification file descriptor - a pipe */
   int notificationFDOutput_;
   int notificationFDInput_;
   // Easier than non-blocking reads... there is either 1 byte in the pipe or 0.
   bool FDneedsPolling_;
   public:  
   /* And auxiliary functions for the above */
   /* These are NOT part of the API, do not use externally */
   void signalNotificationFD(); // Called when an event happens
   void clearNotificationFD(); // Called during poll/waitForStatusChange
   void createNotificationFD(); // Creates the FD
   
public:
    static BPatch		 *bpatch;

	static BPatch *getBPatch();
    BPatch_builtInTypeCollection *builtInTypes;
    BPatch_typeCollection	 *stdTypes;
    BPatch_typeCollection        *APITypes; //API/User defined types
    BPatch_type			 *type_Error;
    BPatch_type			 *type_Untyped;

    // The following are only to be called by the library:
    //  These functions are not locked.
    void registerProvisionalThread(int pid);
    void registerForkedProcess(process *parentProc, process *childProc);
    void registerForkingProcess(int forkingPid, process *proc);

    void registerExecExit(process *proc);
    void registerExecCleanup(process *proc, char *arg0);

    void registerNormalExit(process *proc, int exitcode);
    void registerSignalExit(process *proc, int signalnum);

    void registerThreadExit(process *proc, long tid, bool exiting);
    bool registerThreadCreate(BPatch_process *proc, BPatch_thread *newthr);

    void registerProcess(BPatch_process *process, int pid=0);
    void unRegisterProcess(int pid, BPatch_process *proc);

    void launchDeferredOneTimeCode();

    void registerLoadedModule(process *process, mapped_module *mod);
    void registerUnloadedModule(process *process, mapped_module *mod);

    BPatch_thread *getThreadByPid(int pid, bool *exists = NULL);
    BPatch_process *getProcessByPid(int pid, bool *exists = NULL);

    static void reportError(BPatchErrorLevel severity, int number, const char *str);

    void clearError() { lastError = 0; }
    int getLastError() { return lastError; }
    // End of functions that are for internal use only

    public:

    BPatch();

    //  BPatch::~BPatch
    //  destructor
    API_EXPORT_DTOR(_dtor, (),

    ~,BPatch,());

    static const char *getEnglishErrorString(int number);
    static void formatErrorString(char *dst, int size,
				  const char *fmt, const char * const *params);

    // BPatch::isTypeChecked:
    // returns whether type checking is on.
    API_EXPORT(Int, (),

    bool,isTypeChecked,());

    // BPatch::parseDebugInfo:
    // returns whether debugging information is set to be parsed
    API_EXPORT(Int, (),

    bool,parseDebugInfo,());

    // BPatch::baseTrampDeletion:
    // returns whether base trampolines are set to be deleted
    API_EXPORT(Int, (),

    bool,baseTrampDeletion,());

    // BPatch::setPrelinkCommand
    // sets the fully qualified path name of the prelink command
    API_EXPORT_V(Int, (command),

 	void,setPrelinkCommand,(char *command));

    // BPatch::getPrelinkCommand
    // gets the fully qualified path name of the prelink command
    API_EXPORT(Int, (),

 	char*,getPrelinkCommand,());

    // BPatch::isTrampRecursive:
    // returns whether trampolines are set to handle recursive instrumentation
    API_EXPORT(Int, (),

    bool,isTrampRecursive,());

    // BPatch::isMergeTramp:
    // returns whether base tramp and mini-tramp is merged
    API_EXPORT(Int, (),

    bool,isMergeTramp,());        

    // BPatch::saveFPROn:
    // returns whether base tramp and mini-tramp is merged
    API_EXPORT(Int, (),

    bool,isSaveFPROn,());        

    // BPatch::forceSaveFPROn:
    // returns whether base tramp and mini-tramp is merged
    API_EXPORT(Int, (),

    bool,isForceSaveFPROn,());        


    // BPatch::hasForcedRelocation_NP:
    // returns whether all instrumented functions will be relocated
    API_EXPORT(Int, (),

    bool,hasForcedRelocation_NP,());

    // BPatch::autoRelocationsOn:
    // returns whether functions will be relocated when appropriate
    API_EXPORT(Int, (),

    bool,autoRelocationOn,());


    // BPatch::delayedParsingOn:
    // returns whether inst info is parsed a priori, or on demand
    API_EXPORT(Int, (),

    bool,delayedParsingOn,());

    // Liveness...
    API_EXPORT(Int, (),
    bool, livenessAnalysisOn, ());

    API_EXPORT(Int, (),
               int, livenessAnalysisDepth, ());

    //  User-specified callback functions...

    //  BPatch::registerErrorCallback:
    //  Register error handling/reporting callback
    API_EXPORT(Int, (function),

    BPatchErrorCallback, registerErrorCallback,(BPatchErrorCallback function));

    //  BPatch::registerDynLibraryCallback:
    //  Register callback for new library events (eg. load)
    API_EXPORT(Int, (func),

    BPatchDynLibraryCallback, registerDynLibraryCallback,(BPatchDynLibraryCallback func));

    //  BPatch::registerPostForkCallback:
    //  Register callback to handle mutatee fork events (before fork)
    API_EXPORT(Int, (func),

    BPatchForkCallback, registerPostForkCallback,(BPatchForkCallback func));

    //  BPatch::registerPreForkCallback:
    //  Register callback to handle mutatee fork events (before fork)
    API_EXPORT(Int, (func),

    BPatchForkCallback, registerPreForkCallback,(BPatchForkCallback func));

    //  BPatch::registerExecCallback:
    //  Register callback to handle mutatee exec events 
    API_EXPORT(Int, (func),
    BPatchExecCallback, registerExecCallback,(BPatchExecCallback func));

    //  BPatch::registerExitCallback:
    //  Register callback to handle mutatee exit events 
    API_EXPORT(Int, (func),

    BPatchExitCallback, registerExitCallback,(BPatchExitCallback func));

    //  BPatch::registerOneTimeCodeCallback:
    //  Register callback to run at completion of oneTimeCode 
    API_EXPORT(Int, (func),
    BPatchOneTimeCodeCallback, registerOneTimeCodeCallback,(BPatchOneTimeCodeCallback func));

    //  BPatch::registerThreadEventCallback
    //  Registers a callback to run when a thread is created
    API_EXPORT(Int, (type,cb),
    bool,registerThreadEventCallback,(BPatch_asyncEventType type, 
                                      BPatchAsyncThreadEventCallback cb));

    //  BPatch::removeThreadEventCallback
    //  Registers a callback to run when a thread is destroyed
    API_EXPORT(Int, (type,cb),
    bool,removeThreadEventCallback,(BPatch_asyncEventType type,
                                    BPatchAsyncThreadEventCallback cb));

    //  BPatch::registerDynamicCallCallback
    //  Specifies a user-supplied function to be called when a dynamic call is
    //  executed.

    API_EXPORT(Int, (cb),
    bool,registerDynamicCallCallback,(BPatchDynamicCallSiteCallback cb));

    API_EXPORT(Int, (cb),
    bool,removeDynamicCallCallback,(BPatchDynamicCallSiteCallback cb));


    //  BPatch::registerUserEventCallback
    //  
    //  Specifies a user defined function to call when a "user event" 
    //  occurs, user events are trigger by calls to the function 
    //  DYNINSTuserMessage(void *, int) in the runtime library.
    //  
    //  BPatchUserEventCallback is:
    //  void (*BPatchUserEventCallback)(void *msg, unsigned int msg_size);

    API_EXPORT(Int, (cb),
    bool,registerUserEventCallback,(BPatchUserEventCallback cb)); 

    API_EXPORT(Int, (cb),
    bool,removeUserEventCallback,(BPatchUserEventCallback cb));

    // BPatch::registerSignalHandlerCallback 
    // 
    // If the mutator produces a signal matching an element of
    // signal_numbers, the callback is invoked, returning the point
    // that caused the exception, the signal number, and a Vector
    // representing the address of signal handler(s) in the mutatee
    // for the exception.  In Windows this is the handler stack, each
    // function of which is invoked until one is found that agrees to
    // handle the exception.  In Unix there will be at most one
    // handler for the signal number, the handler registered with
    // syscalls signal() or sigaction(), or the default system
    // handler, in which case we return an empty vector.
     API_EXPORT(Int, (cb,signal_numbers), 
                bool,registerSignalHandlerCallback,
                (BPatchSignalHandlerCallback cb, 
                 BPatch_Set<long> *signal_numbers)); 

     API_EXPORT(Int, (cb), 
     bool,removeSignalHandlerCallback,(BPatchSignalHandlerCallback cb)); 

    API_EXPORT(Int, (cb), 
    bool,registerCodeDiscoveryCallback,(BPatchCodeDiscoveryCallback cb));
    API_EXPORT(Int, (cb), 
    bool,removeCodeDiscoveryCallback,(BPatchCodeDiscoveryCallback cb));

    // BPatch::registerCodeOverwriteCallbacks
    // 
    // Registers a callback at the beginning and end of overwrite events
    API_EXPORT(Int, (cbBegin, cbEnd), 
    bool,registerCodeOverwriteCallbacks,
        (BPatchCodeOverwriteBeginCallback cbBegin,
         BPatchCodeOverwriteEndCallback cbEnd));


    //  BPatch::getProcesses:
    //  Get a vector of all processes 
    API_EXPORT(Int, (),
    BPatch_Vector<BPatch_process*> *,getProcesses,());

    //
    //  General BPatch parameter settings:
    //
    
    //  BPatch::setDebugParsing:
    //  Turn on/off parsing of debug section(s)
    API_EXPORT_V(Int, (x),

    void,setDebugParsing,(bool x));

    //  BPatch::setBaseTrampDeletion:
    //  Turn on/off deletion of base tramp
    API_EXPORT_V(Int, (x),

    void,setBaseTrampDeletion,(bool x));

    //  BPatch::setTypeChecking:
    //  Turn on/off type checking
    API_EXPORT_V(Int, (x),

    void,setTypeChecking,(bool x));

    API_EXPORT_V(Int, (b),
    void,setInstrStackFrames,(bool b));

    API_EXPORT(Int, (),
    bool,getInstrStackFrames,());

    //  BPatch::setTypeChecking:
    //  Turn on/off line info truncating
    API_EXPORT_V(Int, (x),

    void,truncateLineInfoFilenames,(bool x));

    //  BPatch::setTrampRecursive:
    //  Turn on/off recursive trampolines
    API_EXPORT_V(Int, (x),

    void,setTrampRecursive,(bool x));

    //  BPatch::setMergeTramp:
    //  Turn on/off merged base & mini-tramps
    API_EXPORT_V(Int, (x),

    void,setMergeTramp,(bool x));

    //  BPatch::setSaveFPR:
    //  Turn on/off merged base & mini-tramps
    API_EXPORT_V(Int, (x),

    void,setSaveFPR,(bool x));

    //  BPatch::forceSaveFPR:
    //  Force Turn on/off merged base & mini-tramps - ignores isConservative
    API_EXPORT_V(Int, (x),

    void,forceSaveFPR,(bool x));


    //  BPatch::setForcedRelocation_NP:
    //  Turn on/off forced relocation of instrumted functions
    API_EXPORT_V(Int, (x),

    void,setForcedRelocation_NP,(bool x));

    //  BPatch::setAutoRelocation_NP:
    //  Turn on/off function relocations, performed when necessary
    API_EXPORT_V(Int, (x),

    void,setAutoRelocation_NP,(bool x));

    //  BPatch::setDelayedParsing:
    //  Turn on/off delayed parsing
    API_EXPORT_V(Int, (x),

    void,setDelayedParsing,(bool x));

    // Liveness...
    API_EXPORT_V(Int, (x),
    void, setLivenessAnalysis, (bool x));

    API_EXPORT_V(Int, (x),
                 void, setLivenessAnalysisDepth, (int x));

    // BPatch::processCreate:
    // Create a new mutatee process
    API_EXPORT(Int, (path, argv, envp, stdin_fd, stdout_fd, stderr_fd, mode),
    BPatch_process *,processCreate,(const char *path,
                                    const char *argv[],
                                    const char **envp = NULL,
                                    int stdin_fd=0,
                                    int stdout_fd=1,
                                    int stderr_fd=2,
                                    BPatch_hybridMode mode=BPatch_normalMode));


    // BPatch::processAttach
    // Attach to mutatee process
    API_EXPORT(Int, (path, pid, mode),
    BPatch_process *,processAttach,(const char *path, int pid, 
                                    BPatch_hybridMode mode=BPatch_normalMode));


    // BPatch::openBinary
    // Open a binary for static instrumentation
    //
    // The second parameter really should be a boolean, but the value
    // gets reset between the openBinary and openBinaryInt calls--is
    // this a gcc bug???
    // 
    API_EXPORT(Int, (path, openDependencies), 
               BPatch_binaryEdit *, openBinary, (const char *path, bool openDependencies = false));

    // BPatch::createEnum:
    // Create Enum types. 
    API_EXPORT(Int, (name, elementNames, elementIds),

    BPatch_type *,createEnum,(const char * name, BPatch_Vector<char *> &elementNames,
                              BPatch_Vector<int> &elementIds));

    // BPatch::createEnum:
    // API selects elementIds
    API_EXPORT(AutoId, (name, elementNames),

    BPatch_type *,createEnum,(const char * name, BPatch_Vector<char *> &elementNames));

    // BPatch::createStruct:
    // Create Struct types. 
    API_EXPORT(Int, (name, fieldNames, fieldTypes),

    BPatch_type *,createStruct,(const char * name, BPatch_Vector<char *> &fieldNames,
                                BPatch_Vector<BPatch_type *> &fieldTypes));

    // BPatch::createUnion:
    // Create Union types. 
    API_EXPORT(Int, (name, fieldNames, fieldTypes),

    BPatch_type *,createUnion,(const char * name, BPatch_Vector<char *> &fieldNames,
                               BPatch_Vector<BPatch_type *> &fieldTypes));

    // BPatch::createArray:
    // Creates BPatch_array type or symtyperanges ( scalars with upper and
    //lower bound).
    API_EXPORT(Int, (name, ptr, low, hi),

    BPatch_type *,createArray,(const char * name, BPatch_type * ptr,
                               unsigned int low, unsigned int hi));

    // BPatch::createPointer:
    // Creates BPatch_pointer types	 
    API_EXPORT(Int, (name, ptr, size),

    BPatch_type *,createPointer,(const char * name, BPatch_type * ptr,
                                 int size = sizeof(void *)));

    // BPatch::createScalar:
    // Creates BPatch_scalar types
    API_EXPORT(Int, (name, size),

    BPatch_type *,createScalar,(const char * name, int size));
    
    // BPatch::createTypedef:
    // Creates typedefs.
    API_EXPORT(Int, (name, ptr),

    BPatch_type *,createTypedef,(const char * name, BPatch_type * ptr));
	 
    // User programs are required to call pollForStatusChange or
    // waitForStatusChange before user-level callback functions
    // are executed (for example, fork, exit, or a library load). 

    // Non-blocking form; returns immediately if no callback is
    // ready, or executes callback(s) then returns.
    API_EXPORT(Int, (),
    bool,pollForStatusChange,());

    // Blocks until a callback is ready.
    API_EXPORT(Int, (),
    bool,waitForStatusChange,());

    // For user programs that block on other things as well,
    // we provide a (simulated) file descriptor that can be added
    // to a poll or select fdset. When a callback is prepared the BPatch
    // layer writes to this fd, thus making poll/select return. The user
    // program should then call pollForStatusChange. The BPatch layer
    // will handle clearing the file descriptor; all the program must do 
    // is call pollForStatusChange or waitForStatusChange.
    API_EXPORT(Int, (),
    int, getNotificationFD, ());

    //  BPatch:: waitUntilStopped:
    //  Block until specified process has stopped.
    API_EXPORT(Int, (appThread),

    bool,waitUntilStopped,(BPatch_thread *appThread));

    //  BPatch::getBPatchStatistics:
    //  Get Instrumentation statistics
    API_EXPORT(Int, (),

    BPatch_stats &,getBPatchStatistics,());


    API_EXPORT_V(Int, (major, minor, subminor),
    void ,getBPatchVersion,(int &major, int &minor, int &subminor));

    // These three should probably be moved into their own BPatch_* class.
    // Perhaps BPatch_remoteDebug?
    API_EXPORT(Int, (),
    bool, isConnected, ());

    API_EXPORT(Int, (remote),
    bool, remoteConnect, (BPatch_remoteHost &remote));

    API_EXPORT(Int, (remote, pidlist),
    bool,getPidList,(BPatch_remoteHost &remote, BPatch_Vector<unsigned int> &pidlist));

    API_EXPORT(Int, (remote, pid, pidStr),
    bool,getPidInfo,(BPatch_remoteHost &remote, unsigned int pid, std::string &pidStr));

    API_EXPORT(Int, (remote),
    bool, remoteDisconnect, (BPatch_remoteHost &remote));

    //  BPatch::addNonReturningFunc:
    //  Globally specify that any function with a given name will not return
    API_EXPORT_V(Int, (name),
    void, addNonReturningFunc, (std::string name));
};

#endif /* _BPatch_h_ */
