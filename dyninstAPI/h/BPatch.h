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

#ifndef _BPatch_h_
#define _BPatch_h_

#include <stdio.h>
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_type.h"

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

typedef void (*BPatchExitCallback)(BPatch_thread *proc, int code);

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
    bool	typeCheckOn;
    int		lastError;
    bool	debugParseOn;

    bool	getThreadEvent(bool block);
    bool	havePendingEvent();

    /* If true, trampolines can recurse to their heart's content.
       Defaults to false */
    bool        trampRecursiveOn;

public:
    static BPatch		 *bpatch;

    BPatch_builtInTypeCollection *builtInTypes;
    BPatch_typeCollection	 *stdTypes;
    BPatch_typeCollection        *APITypes; //API/User defined types
    BPatch_type			 *type_Error;
    BPatch_type			 *type_Untyped;
    
    bool isTypeChecked() { return typeCheckOn; }
    bool parseDebugInfo() { return debugParseOn; }
    bool isTrampRecursive() { return trampRecursiveOn; }

    // The following are only to be called by the library:
    void registerProvisionalThread(int pid);
    void registerForkedThread(int parentPid, int childPid, process *proc);
    void registerExec(BPatch_thread *thread);
    void registerExit(BPatch_thread *thread, int code);
    void registerThread(BPatch_thread *thread);
    void unRegisterThread(int pid);

    BPatch_thread *getThreadByPid(int pid, bool *exists = NULL);

    static void reportError(BPatchErrorLevel severity, int number, const char *str);

    void clearError() { lastError = 0; }
    int	getLastError() { return lastError; }
    // End of functions that are for internal use only

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

    BPatch_Vector<BPatch_thread*> *getThreads();

    void setDebugParsing(bool x) { debugParseOn = x; }
    void setTypeChecking(bool x) { typeCheckOn = x; }
    void setTrampRecursive(bool x) { trampRecursiveOn = x; }

    BPatch_thread *createProcess(char *path, char *argv[], 
	char *envp[] = NULL, int stdin_fd=0, int stdout_fd=1, int stderr_fd=2);
    BPatch_thread *attachProcess(char *path, int pid);

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
};

#endif /* _BPatch_h_ */
