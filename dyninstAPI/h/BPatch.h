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
#include "BPatch_Vector.h"
#include "BPatch_thread.h"

class BPatch_typeCollection;
class BPatch_type;
class BPatch_libInfo;
class BPatch_module;

typedef enum BPatchErrorLevel {
    BPatchFatal, BPatchSerious, BPatchWarning, BPatchInfo
};

typedef void (*BPatchErrorCallback)(BPatchErrorLevel severity,
				    int number,
				    const char **params);

class BPatch {
    friend class BPatch_thread;
    friend class process;

    BPatch_libInfo	*info;

    BPatchErrorCallback      errorHandler;

    bool	typeCheckOn;
    int		lastError;

    bool	getThreadEvent(bool block);
    bool	havePendingEvent();

public:
    static BPatch		*bpatch;

    BPatch_typeCollection	*stdTypes;
    BPatch_type			*type_Error;
    BPatch_type			*type_Untyped;

    // The following are only to be called by the library:
    bool isTypeChecked() { return typeCheckOn; }

    void registerProvisionalThread(int pid);
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
    BPatch_Vector<BPatch_thread*> *getThreads();

    void setTypeChecking(bool x) { typeCheckOn = x; }

    BPatch_thread *createProcess(char *path, char *argv[], char *envp[] = NULL);
    BPatch_thread *attachProcess(char *path, int pid);

    bool 	pollForStatusChange();
    bool 	waitForStatusChange();
};

#endif /* _BPatch_h_ */
