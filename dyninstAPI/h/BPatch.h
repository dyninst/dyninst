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

#ifndef _BPatch_h_
#define _BPatch_h_

#include <stdio.h>
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_type.h"

#ifdef mips_unknown_ce2_11 //ccw 28 july 2000
#include "remoteDevice.h"
#endif

class BPatch_typeCollection;
class BPatch_libInfo;
class BPatch_module;

typedef enum {
    BPatchFatal, BPatchSerious, BPatchWarning, BPatchInfo
} BPatchErrorLevel;

typedef void (*BPatchErrorCallback)(BPatchErrorLevel severity,
				    int number,
				    const char **params);

typedef void (*BPatchDynLibraryCallback)(BPatch_thread *thr,
					 BPatch_module *mod,
					 bool load);

typedef void (*BPatchForkCallback)(BPatch_thread *parent, BPatch_thread *child);

typedef void (*BPatchExecCallback)(BPatch_thread *proc);

typedef void (*BPatchExitCallback)(BPatch_thread *proc,
                                   BPatch_exitType exit_type);

typedef void (*BPatchSignalCallback)(BPatch_thread *proc, int sigNum);

typedef void (*BPatchOneTimeCodeCallback)(BPatch_thread *proc, void *userData, void *returnValue);

#ifdef IBM_BPATCH_COMPAT
typedef void *BPatch_Address;
typedef void (*BPatchLoggingCallback)(char *msg, int);
extern void setLogging_NP(BPatchLoggingCallback func, int);

typedef void (*BPatchThreadEventCallback)(BPatch_thread *thr, void *arg1, void *arg2);

#define BP_OK			0
#define BP_Pending		1
#define BP_Delayed		2
#define BP_child		3
#define BP_state		4
#define BP_pthdbBadContext	5
#define BP_lastCode		6

extern int eCodes[BP_lastCode];

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

class BPATCH_DLL_EXPORT BPatch {
    friend class BPatch_thread;
    friend class process;

    BPatch_libInfo	*info;

    BPatchErrorCallback      	errorHandler;
    BPatchDynLibraryCallback 	dynLibraryCallback;
    BPatchForkCallback   	postForkCallback;
    BPatchForkCallback    	preForkCallback;
    BPatchExecCallback		execCallback;
    BPatchExitCallback		exitCallback;
    BPatchOneTimeCodeCallback   oneTimeCodeCallback;
    bool	typeCheckOn;
    int		lastError;
    bool	debugParseOn;
    bool	baseTrampDeletionOn;

    bool	getThreadEvent(bool block);
    bool	getThreadEventOnly(bool block);
    bool	havePendingEvent();

    /* If true, trampolines can recurse to their heart's content.
       Defaults to false */
    bool        trampRecursiveOn;
    bool        forceRelocation_NP;
    /* If true, allows automatic relocation of functions if dyninst
       deems it necessary.  Defaults to true */
    bool        autoRelocation_NP;

    BPatch_stats stats;
    void updateStats();
public:
    static BPatch		 *bpatch;

    BPatch_builtInTypeCollection *builtInTypes;
    BPatch_typeCollection	 *stdTypes;
    BPatch_typeCollection        *APITypes; //API/User defined types
    BPatch_type			 *type_Error;
    BPatch_type			 *type_Untyped;
#ifdef mips_unknown_ce2_11 //ccw 28 july 2000
	remoteDevice *rDevice;	//the ctor sets up the connection here and
				//gets the tramptemplate from the CE device.
#endif

 
    bool isTypeChecked() { return typeCheckOn; }
    bool parseDebugInfo() { return debugParseOn; }
    bool baseTrampDeletion() { return baseTrampDeletionOn; }
    bool isTrampRecursive() { return trampRecursiveOn; }
    bool hasForcedRelocation_NP() { return forceRelocation_NP; }
    bool autoRelocationOn() { return autoRelocation_NP; }

    // The following are only to be called by the library:
    void registerProvisionalThread(int pid);
    void registerForkedThread(int parentPid, int childPid, process *proc);
    void registerForkingThread(int forkingPid, process *proc);
    void registerExec(BPatch_thread *thread);
    void registerNormalExit(BPatch_thread *thread, int exitcode);
    void registerSignalExit(BPatch_thread *thread, int signalnum);
    void registerThread(BPatch_thread *thread);
    void unRegisterThread(int pid);
    void launchDeferredOneTimeCode();

    BPatch_thread *getThreadByPid(int pid, bool *exists = NULL);

    static void reportError(BPatchErrorLevel severity, int number, const char *str);

    void clearError() { lastError = 0; }
    int	getLastError() { return lastError; }
    // End of functions that are for internal use only

#ifdef IBM_BPATCH_COMPAT
    int	getLastErrorCode() { return lastError; }

    BPatchThreadEventCallback registerDetachDoneCallback(BPatchThreadEventCallback) { return NULL; }
    BPatchThreadEventCallback registerSnippetRemovedCallback(BPatchThreadEventCallback) { return NULL; }
    
    BPatchExitCallback registerSignalCallback(BPatchThreadEventCallback func, int sigNum) {return NULL;}
    BPatchExitCallback registerExitCallback(BPatchThreadEventCallback func);

    BPatchThreadEventCallback registerRPCTerminationCallback(BPatchThreadEventCallback);
    BPatchThreadEventCallback		RPCdoneCallback;
#endif

    BPatch();
    ~BPatch();

    static const char *getEnglishErrorString(int number);
    static void formatErrorString(char *dst, int size,
				  const char *fmt, const char **params);
    BPatchErrorCallback registerErrorCallback(BPatchErrorCallback function);
    BPatchDynLibraryCallback registerDynLibraryCallback(BPatchDynLibraryCallback function);
    BPatchForkCallback registerPostForkCallback(BPatchForkCallback func);
    BPatchForkCallback registerPreForkCallback(BPatchForkCallback func);
    BPatchExecCallback registerExecCallback(BPatchExecCallback func);
    BPatchExitCallback registerExitCallback(BPatchExitCallback func);
    BPatchOneTimeCodeCallback registerOneTimeCodeCallback(BPatchOneTimeCodeCallback func);

    BPatch_Vector<BPatch_thread*> *getThreads();

    void setDebugParsing(bool x) { debugParseOn = x; }
    void setBaseTrampDeletion(bool x) { baseTrampDeletionOn = x; }
    void setTypeChecking(bool x) { typeCheckOn = x; }
    void setTrampRecursive(bool x) { trampRecursiveOn = x; }
    void setForcedRelocation_NP(bool x) { forceRelocation_NP = x; }
    void setAutoRelocation_NP(bool x) { autoRelocation_NP = x; }

    BPatch_thread *createProcess(const char *path, const char *argv[], 
	const char *envp[] = NULL, int stdin_fd=0, int stdout_fd=1, int stderr_fd=2);
    BPatch_thread *attachProcess(const char *path, int pid);

    // Create Enum types. 
    BPatch_type * createEnum(const char * name, 
	BPatch_Vector<char *> elementNames,
	BPatch_Vector<int> elementIds);
    
    // API selects elemetIds
    BPatch_type * createEnum(const char * name, 
	BPatch_Vector<char *> elementNames);

    // Create Struct types. 
    BPatch_type * createStruct( const char * name,
				BPatch_Vector<char *> fieldNames,
				BPatch_Vector<BPatch_type *> fieldTypes);

    // Create Union types. 
    BPatch_type * createUnion( const char * name, 
				BPatch_Vector<char *> fieldNames,
				BPatch_Vector<BPatch_type *> fieldTypes);
 
    // Creates BPatch_array type or symtyperanges ( scalars with upper and
   //lower bound).
    BPatch_type * createArray( const char * name, BPatch_type * ptr,
			       unsigned int low, unsigned int hi );

    // Creates BPatch_pointer types	 
    BPatch_type * createPointer( const char * name, BPatch_type * ptr,
				 int size = sizeof(void *));

    // Creates BPatch_scalar types
    BPatch_type * createScalar( const char * name, int size );
    
    // Creates typedefs.
    BPatch_type * createTypedef( const char * name, BPatch_type * ptr );
	 
    bool 	pollForStatusChange();
    bool 	waitForStatusChange();

    bool waitUntilStopped(BPatch_thread *appThread);

    BPatch_stats &getBPatchStatistics() {
      updateStats();
      return stats;
    } 
};

#if defined(IBM_BPATCH_COMPAT) && (defined(rs6000_ibm_aix4_1) || defined(rs6000_ibm_aix5_1)) 
#include <sys/ldr.h>

typedef struct LD_INFO {
  union {
    struct ld_info32 {
      __I_FIELDS(__I_INT32, __I_PTR32, __I_FP32, __I_EMPTY)
    }ld_info32;
    struct ld_info64 {
      __I_FIELDS(__I_INT64, __I_PTR64, __I_FP64, uint ldinfo_flags;)
    }ld_info64;
  } ld_info;
} LD_INFO;

#define LD_32(a,b) (a->ld_info.ld_info32.b)
#define LD_64(a,b) (a->ld_info.ld_info64.b)

#endif // AIX IBM BPATCH

#endif /* _BPatch_h_ */
