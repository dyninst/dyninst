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


typedef struct {
  unsigned int pointsUsed;
  unsigned int totalMiniTramps;
  unsigned int trampBytes;
  unsigned int ptraceOtherOps;
  unsigned int ptraceOps;
  unsigned int ptraceBytes;
  unsigned int insnGenerated;
} BPatch_stats;

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

    bool        trampRecursiveOn;

    bool        forceRelocation_NP;
    bool        autoRelocation_NP;
    bool saveFloatingPointsOn;
    bool forceSaveFloatingPointsOn;
    bool livenessAnalysisOn_;
    int livenessAnalysisDepth_;
    bool asyncActive;
    bool delayedParsing_;

    bool instrFrames;

    BPatch_stats stats;
    void updateStats();

	char *systemPrelinkCommand;

        void continueIfExists(int pid);

   int notificationFDOutput_;
   int notificationFDInput_;
   bool FDneedsPolling_;

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

   bool inDestructor;

   public:  
     
   
public:
    static BPatch		 *bpatch;

	static BPatch *getBPatch();
    BPatch_builtInTypeCollection *builtInTypes;
    BPatch_typeCollection	 *stdTypes;
    BPatch_typeCollection        *APITypes;
    BPatch_type			 *type_Error;
    BPatch_type			 *type_Untyped;

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

    public:

    BPatch();

    ~BPatch();

    static const char *getEnglishErrorString(int number);
    static void formatErrorString(char *dst, int size,
				  const char *fmt, const char * const *params);

    bool isTypeChecked();

    bool parseDebugInfo();

    bool baseTrampDeletion();

    void setPrelinkCommand(char *command);

    char* getPrelinkCommand();

    bool isTrampRecursive();

    bool isMergeTramp();        

    bool isSaveFPROn();        

    bool isForceSaveFPROn();        

    bool hasForcedRelocation_NP();

    bool autoRelocationOn();


    bool delayedParsingOn();

    bool  livenessAnalysisOn();

    
               int livenessAnalysisDepth();


    BPatchErrorCallback registerErrorCallback(BPatchErrorCallback function);

    BPatchDynLibraryCallback registerDynLibraryCallback(BPatchDynLibraryCallback func);

    BPatchForkCallback registerPostForkCallback(BPatchForkCallback func);

    BPatchForkCallback registerPreForkCallback(BPatchForkCallback func);

    BPatchExecCallback registerExecCallback(BPatchExecCallback func);

    BPatchExitCallback registerExitCallback(BPatchExitCallback func);

    BPatchOneTimeCodeCallback registerOneTimeCodeCallback(BPatchOneTimeCodeCallback func);

    bool registerThreadEventCallback(BPatch_asyncEventType type, 
                                      BPatchAsyncThreadEventCallback cb);

    bool removeThreadEventCallback(BPatch_asyncEventType type,
                                    BPatchAsyncThreadEventCallback cb);

    bool registerDynamicCallCallback(BPatchDynamicCallSiteCallback cb);

    
    bool removeDynamicCallCallback(BPatchDynamicCallSiteCallback cb);


    //  BPatchUserEventCallback is:
    //  void (*BPatchUserEventCallback)(void *msg, unsigned int msg_size);

    
    bool registerUserEventCallback(BPatchUserEventCallback cb); 

    
    bool removeUserEventCallback(BPatchUserEventCallback cb);

    bool registerSignalHandlerCallback(BPatchSignalHandlerCallback cb, 
                                       std::set<long> &signal_numbers); 
    bool registerSignalHandlerCallback(BPatchSignalHandlerCallback cb, 
                                       BPatch_Set<long> *signal_numbers); 
     
     bool removeSignalHandlerCallback(BPatchSignalHandlerCallback cb); 

    
    bool registerCodeDiscoveryCallback(BPatchCodeDiscoveryCallback cb);
    
    bool removeCodeDiscoveryCallback(BPatchCodeDiscoveryCallback cb);

    bool registerCodeOverwriteCallbacks
        (BPatchCodeOverwriteBeginCallback cbBegin,
         BPatchCodeOverwriteEndCallback cbEnd);


    BPatch_Vector<BPatch_process*> * getProcesses();

    void setDebugParsing(bool x);

    void setBaseTrampDeletion(bool x);

    void setTypeChecking(bool x);

    
    void setInstrStackFrames(bool b);

    
    bool getInstrStackFrames();

    DYNINST_DEPRECATED("Does nothing")
    void truncateLineInfoFilenames(bool);

    void setTrampRecursive(bool x);

    void setMergeTramp(bool x);

    void setSaveFPR(bool x);

    void forceSaveFPR(bool x);


    void setForcedRelocation_NP(bool x);

    void setAutoRelocation_NP(bool x);

    void setDelayedParsing(bool x);

    void  setLivenessAnalysis(bool x);

    
                 void  setLivenessAnalysisDepth(int x);

    BPatch_process * processCreate(const char *path,
				   const char *argv[],
				   const char **envp = NULL,
				   int stdin_fd=0,
				   int stdout_fd=1,
				   int stderr_fd=2,
				   BPatch_hybridMode mode=BPatch_normalMode);


    BPatch_process *processAttach(const char *path, int pid, 
                                    BPatch_hybridMode mode=BPatch_normalMode);


               BPatch_binaryEdit * openBinary(const char *path, bool openDependencies = false);

    BPatch_type *createEnum(const char * name, BPatch_Vector<char *> &elementNames,
                              BPatch_Vector<int> &elementIds);

    BPatch_type *createEnum(const char * name, BPatch_Vector<char *> &elementNames);

    BPatch_type *createStruct(const char * name, BPatch_Vector<char *> &fieldNames,
                                BPatch_Vector<BPatch_type *> &fieldTypes);

    BPatch_type *createUnion(const char * name, BPatch_Vector<char *> &fieldNames,
                               BPatch_Vector<BPatch_type *> &fieldTypes);

    BPatch_type *createArray(const char * name, BPatch_type * ptr,
                               unsigned int low, unsigned int hi);

    BPatch_type *createPointer(const char * name, BPatch_type * ptr,
                                 int size = sizeof(void *));

    BPatch_type *createScalar(const char * name, int size);
    
    BPatch_type *createTypedef(const char * name, BPatch_type * ptr);
	 
    bool pollForStatusChange();

    bool waitForStatusChange();

    int getNotificationFD();

    bool waitUntilStopped(BPatch_thread *appThread);

    BPatch_stats & getBPatchStatistics();


    
    void getBPatchVersion(int &major, int &minor, int &subminor);

    bool  isConnected();

    
    bool  remoteConnect(BPatch_remoteHost &remote);

    
    bool getPidList(BPatch_remoteHost &remote, BPatch_Vector<unsigned int> &pidlist);

    
    bool getPidInfo(BPatch_remoteHost &remote, unsigned int pid, std::string &pidStr);

    
    bool  remoteDisconnect(BPatch_remoteHost &remote);

    void  addNonReturningFunc(std::string name);
};


#endif /* _BPatch_h_ */
