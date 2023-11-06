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

#ifndef _BPatch_h_
#define _BPatch_h_

#include <stdio.h>
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_Set.h"
#include "BPatch_thread.h"
#include "BPatch_type.h"
#include "BPatch_process.h"
#include "BPatch_enums.h"
#include "BPatch_callbacks.h"
#include <set>
#include <string>
#include "dyntypes.h"
#include "dyninstversion.h"
#include "compiler_diagnostics.h"

class BPatch_typeCollection;
class BPatch_libInfo;
class BPatch_module;
class PCProcess;
class PCThread;
class PCEventHandler;
class func_instance;

//Keep old versions defined, that way someone can test if we're more
// at or more recent than version 5.1 with '#if defined(DYNINST_5_1)'
//If they want to get the current version, they should use DYNINST_MAJOR,
// DYNINST_MINOR, and DYNINST_SUBMINOR
#define DYNINST_5_1
#define DYNINST_5_2
#define DYNINST_6_0
#define DYNINST_6_1
#define DYNINST_7_0
#define DYNINST_8_0
#define DYNINST_8_1
#define DYNINST_8_1_1
#define DYNINST_8_1_2
#define DYNINST_8_2
#define DYNINST_9_0
#define DYNINST_9_1

#define DYNINST_MAJOR DYNINST_MAJOR_VERSION
#define DYNINST_MINOR DYNINST_MINOR_VERSION
#define DYNINST_SUBMINOR DYNINST_PATCH_VERSION


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

class BPATCH_DLL_EXPORT BPatch {
    friend class BPatch_thread;
    friend class BPatch_process;
    friend class BPatch_point;
    friend class PCProcess;
    friend class func_instance;

    BPatch_libInfo *info; 

    bool	typeCheckOn;
    int		lastError;
    bool	debugParseOn;
    bool	baseTrampDeletionOn;

    /* If true, trampolines can recurse to their heart's content.
       Defaults to false */
    bool        trampRecursiveOn;

    bool        forceRelocation_NP;
    /* If true,allows automatic relocation of functions if dyninst
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

   /* Internal notification file descriptor - a pipe */
   int notificationFDOutput_;
   int notificationFDInput_;
   // Easier than non-blocking reads... there is either 1 byte in the pipe or 0.
   bool FDneedsPolling_;

   // Callbacks //
   BPatchErrorCallback errorCallback;
   BPatchForkCallback preForkCallback;
   BPatchForkCallback postForkCallback;
   BPatchExecCallback execCallback;
   BPatchExitCallback exitCallback;
   BPatchOneTimeCodeCallback oneTimeCodeCallback;
   BPatchDynLibraryCallback dynLibraryCallback;
   BPatchAsyncThreadEventCallback threadCreateCallback;
   BPatchAsyncThreadEventCallback threadDestroyCallback;
   BPatchDynamicCallSiteCallback dynamicCallSiteCallback;
   InternalSignalHandlerCallback signalHandlerCallback;
   std::set<long> callbackSignals;
   InternalCodeOverwriteCallback codeOverwriteCallback;
   
   BPatch_Vector<BPatchUserEventCallback> userEventCallbacks;
   BPatch_Vector<BPatchStopThreadCallback> stopThreadCallbacks;

   // If we're destroying everything, skip cleaning up some intermediate
   // data structures
   bool inDestructor;

   public:  
     
   
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
    void registerForkedProcess(PCProcess *parentProc, PCProcess *childProc);
    void registerForkingProcess(int forkingPid, PCProcess *proc);

    void registerExecExit(PCProcess *proc);
    void registerExecCleanup(PCProcess *proc, char *arg0);

    void registerNormalExit(PCProcess *proc, int exitcode);
    void registerSignalExit(PCProcess *proc, int signalnum);

    void registerThreadExit(PCProcess *llproc, PCThread *llthread);
    bool registerThreadCreate(BPatch_process *proc, BPatch_thread *newthr);

    void registerProcess(BPatch_process *process, int pid=0);
    void unRegisterProcess(int pid, BPatch_process *proc);

    void registerUserEvent(BPatch_process *process, void *buffer,
           unsigned int bufsize);

    void registerDynamicCallsiteEvent(BPatch_process *process, Dyninst::Address callTarget,
           Dyninst::Address callAddr);

    void registerStopThreadCallback(BPatchStopThreadCallback stopCB);
    int getStopThreadCallbackID(BPatchStopThreadCallback stopCB);

    void registerLoadedModule(PCProcess *process, mapped_object *obj);
    void registerUnloadedModule(PCProcess *process, mapped_object *obj);

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
    ~BPatch();

    static const char *getEnglishErrorString(int number);
    static void formatErrorString(char *dst, int size,
				  const char *fmt, const char * const *params);

    // BPatch::isTypeChecked:
    // returns whether type checking is on.
    bool isTypeChecked();

    // BPatch::parseDebugInfo:
    // returns whether debugging information is set to be parsed
    bool parseDebugInfo();

    // BPatch::baseTrampDeletion:
    // returns whether base trampolines are set to be deleted
    bool baseTrampDeletion();

    // BPatch::setPrelinkCommand
    // sets the fully qualified path name of the prelink command
    void setPrelinkCommand(char *command);

    // BPatch::getPrelinkCommand
    // gets the fully qualified path name of the prelink command
    char* getPrelinkCommand();

    // BPatch::isTrampRecursive:
    // returns whether trampolines are set to handle recursive instrumentation
    bool isTrampRecursive();

    // BPatch::isMergeTramp:
    // returns whether base tramp and mini-tramp is merged
    bool isMergeTramp();        

    // BPatch::saveFPROn:
    // returns whether base tramp and mini-tramp is merged
    bool isSaveFPROn();        

    // BPatch::forceSaveFPROn:
    // returns whether base tramp and mini-tramp is merged
    bool isForceSaveFPROn();        


    // BPatch::hasForcedRelocation_NP:
    // returns whether all instrumented functions will be relocated
    

    bool hasForcedRelocation_NP();

    // BPatch::autoRelocationsOn:
    // returns whether functions will be relocated when appropriate
    

    bool autoRelocationOn();


    // BPatch::delayedParsingOn:
    // returns whether inst info is parsed a priori, or on demand
    

    bool delayedParsingOn();

    // Liveness...
    
    bool  livenessAnalysisOn();

    
               int livenessAnalysisDepth();


    //  User-specified callback functions...

    //  BPatch::registerErrorCallback:
    //  Register error handling/reporting callback
    

    BPatchErrorCallback registerErrorCallback(BPatchErrorCallback function);

    //  BPatch::registerDynLibraryCallback:
    //  Register callback for new library events (eg. load)
    

    BPatchDynLibraryCallback registerDynLibraryCallback(BPatchDynLibraryCallback func);

    //  BPatch::registerPostForkCallback:
    //  Register callback to handle mutatee fork events (before fork)
    

    BPatchForkCallback registerPostForkCallback(BPatchForkCallback func);

    //  BPatch::registerPreForkCallback:
    //  Register callback to handle mutatee fork events (before fork)
    

    BPatchForkCallback registerPreForkCallback(BPatchForkCallback func);

    //  BPatch::registerExecCallback:
    //  Register callback to handle mutatee exec events 
    
    BPatchExecCallback registerExecCallback(BPatchExecCallback func);

    //  BPatch::registerExitCallback:
    //  Register callback to handle mutatee exit events 
    

    BPatchExitCallback registerExitCallback(BPatchExitCallback func);

    //  BPatch::registerOneTimeCodeCallback:
    //  Register callback to run at completion of oneTimeCode 
    
    BPatchOneTimeCodeCallback registerOneTimeCodeCallback(BPatchOneTimeCodeCallback func);

    //  BPatch::registerThreadEventCallback
    //  Registers a callback to run when a thread is created
    
    bool registerThreadEventCallback(BPatch_asyncEventType type, 
                                      BPatchAsyncThreadEventCallback cb);

    //  BPatch::removeThreadEventCallback
    //  Registers a callback to run when a thread is destroyed
    
    bool removeThreadEventCallback(BPatch_asyncEventType type,
                                    BPatchAsyncThreadEventCallback cb);

    //  BPatch::registerDynamicCallCallback
    //  Specifies a user-supplied function to be called when a dynamic call is
    //  executed.

    
    bool registerDynamicCallCallback(BPatchDynamicCallSiteCallback cb);

    
    bool removeDynamicCallCallback(BPatchDynamicCallSiteCallback cb);


    //  BPatch::registerUserEventCallback
    //  
    //  Specifies a user defined function to call when a "user event" 
    //  occurs, user events are trigger by calls to the function 
    //  DYNINSTuserMessage(void *, int) in the runtime library.
    //  
    //  BPatchUserEventCallback is:
    //  void (*BPatchUserEventCallback)(void *msg, unsigned int msg_size);

    
    bool registerUserEventCallback(BPatchUserEventCallback cb); 

    
    bool removeUserEventCallback(BPatchUserEventCallback cb);

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
     
    bool registerSignalHandlerCallback(BPatchSignalHandlerCallback cb, 
                                       std::set<long> &signal_numbers); 
    bool registerSignalHandlerCallback(BPatchSignalHandlerCallback cb, 
                                       BPatch_Set<long> *signal_numbers); 
     
     bool removeSignalHandlerCallback(BPatchSignalHandlerCallback cb); 

    
    bool registerCodeDiscoveryCallback(BPatchCodeDiscoveryCallback cb);
    
    bool removeCodeDiscoveryCallback(BPatchCodeDiscoveryCallback cb);

    // BPatch::registerCodeOverwriteCallbacks
    // 
    // Registers a callback at the beginning and end of overwrite events
    
    bool registerCodeOverwriteCallbacks
        (BPatchCodeOverwriteBeginCallback cbBegin,
         BPatchCodeOverwriteEndCallback cbEnd);


    //  BPatch::getProcesses:
    //  Get a vector of all processes 
    
    BPatch_Vector<BPatch_process*> * getProcesses();

    //
    //  General BPatch parameter settings:
    //
    
    //  BPatch::setDebugParsing:
    //  Turn on/off parsing of debug section(s)
    

    void setDebugParsing(bool x);

    //  BPatch::setBaseTrampDeletion:
    //  Turn on/off deletion of base tramp
    

    void setBaseTrampDeletion(bool x);

    //  BPatch::setTypeChecking:
    //  Turn on/off type checking
    

    void setTypeChecking(bool x);

    
    void setInstrStackFrames(bool b);

    
    bool getInstrStackFrames();

    //  BPatch::setTypeChecking:
    //  Turn on/off line info truncating
    

    DYNINST_DEPRECATED("Does nothing")
    void truncateLineInfoFilenames(bool);

    //  BPatch::setTrampRecursive:
    //  Turn on/off recursive trampolines
    

    void setTrampRecursive(bool x);

    //  BPatch::setMergeTramp:
    //  Turn on/off merged base & mini-tramps
    

    void setMergeTramp(bool x);

    //  BPatch::setSaveFPR:
    //  Turn on/off merged base & mini-tramps
    

    void setSaveFPR(bool x);

    //  BPatch::forceSaveFPR:
    //  Force Turn on/off merged base & mini-tramps - ignores isConservative
    

    void forceSaveFPR(bool x);


    //  BPatch::setForcedRelocation_NP:
    //  Turn on/off forced relocation of instrumted functions
    

    void setForcedRelocation_NP(bool x);

    //  BPatch::setAutoRelocation_NP:
    //  Turn on/off function relocations, performed when necessary
    

    void setAutoRelocation_NP(bool x);

    //  BPatch::setDelayedParsing:
    //  Turn on/off delayed parsing
    

    void setDelayedParsing(bool x);

    // Liveness...
    
    void  setLivenessAnalysis(bool x);

    
                 void  setLivenessAnalysisDepth(int x);

    // BPatch::processCreate:
    // Create a new mutatee process
    
    BPatch_process * processCreate(const char *path,
				   const char *argv[],
				   const char **envp = NULL,
				   int stdin_fd=0,
				   int stdout_fd=1,
				   int stderr_fd=2,
				   BPatch_hybridMode mode=BPatch_normalMode);


    // BPatch::processAttach
    // Attach to mutatee process
    
    BPatch_process *processAttach(const char *path, int pid, 
                                    BPatch_hybridMode mode=BPatch_normalMode);


    // BPatch::openBinary
    // Open a binary for static instrumentation
    //
    // The second parameter really should be a boolean, but the value
    // gets reset between the openBinary and openBinaryInt calls--is
    // this a gcc bug???
    // 
    
               BPatch_binaryEdit * openBinary(const char *path, bool openDependencies = false);

    // BPatch::createEnum:
    // Create Enum types. 
    

    BPatch_type *createEnum(const char * name, BPatch_Vector<char *> &elementNames,
                              BPatch_Vector<int> &elementIds);

    // BPatch::createEnum:
    // API selects elementIds
    

    BPatch_type *createEnum(const char * name, BPatch_Vector<char *> &elementNames);

    // BPatch::createStruct:
    // Create Struct types. 
    

    BPatch_type *createStruct(const char * name, BPatch_Vector<char *> &fieldNames,
                                BPatch_Vector<BPatch_type *> &fieldTypes);

    // BPatch::createUnion:
    // Create Union types. 
    

    BPatch_type *createUnion(const char * name, BPatch_Vector<char *> &fieldNames,
                               BPatch_Vector<BPatch_type *> &fieldTypes);

    // BPatch::createArray:
    // Creates BPatch_array type or symtyperanges ( scalars with upper and
    //lower bound).
    

    BPatch_type *createArray(const char * name, BPatch_type * ptr,
                               unsigned int low, unsigned int hi);

    // BPatch::createPointer:
    // Creates BPatch_pointer types	 
    

    BPatch_type *createPointer(const char * name, BPatch_type * ptr,
                                 int size = sizeof(void *));

    // BPatch::createScalar:
    // Creates BPatch_scalar types
    

    BPatch_type *createScalar(const char * name, int size);
    
    // BPatch::createTypedef:
    // Creates typedefs.
    

    BPatch_type *createTypedef(const char * name, BPatch_type * ptr);
	 
    // User programs are required to call pollForStatusChange or
    // waitForStatusChange before user-level callback functions
    // are executed (for example, fork, exit, or a library load). 

    // Non-blocking form; returns immediately if no callback is
    // ready, or executes callback(s) then returns.
    
    bool pollForStatusChange();

    // Blocks until a callback is ready.
    
    bool waitForStatusChange();

    // For user programs that block on other things as well,
    // we provide a (simulated) file descriptor that can be added
    // to a poll or select fdset. When a callback is prepared the BPatch
    // layer writes to this fd, thus making poll/select return. The user
    // program should then call pollForStatusChange. The BPatch layer
    // will handle clearing the file descriptor; all the program must do 
    // is call pollForStatusChange or waitForStatusChange.
    
    int getNotificationFD();

    //  BPatch:: waitUntilStopped:
    //  Block until specified process has stopped.
    

    bool waitUntilStopped(BPatch_thread *appThread);

    //  BPatch::getBPatchStatistics:
    //  Get Instrumentation statistics
    

    BPatch_stats & getBPatchStatistics();


    
    void getBPatchVersion(int &major, int &minor, int &subminor);

    // These three should probably be moved into their own BPatch_* class.
    // Perhaps BPatch_remoteDebug?
    
    bool  isConnected();

    
    bool  remoteConnect(BPatch_remoteHost &remote);

    
    bool getPidList(BPatch_remoteHost &remote, BPatch_Vector<unsigned int> &pidlist);

    
    bool getPidInfo(BPatch_remoteHost &remote, unsigned int pid, std::string &pidStr);

    
    bool  remoteDisconnect(BPatch_remoteHost &remote);

    //  BPatch::addNonReturningFunc:
    //  Globally specify that any function with a given name will not return
    
    void  addNonReturningFunc(std::string name);
};


#endif /* _BPatch_h_ */
