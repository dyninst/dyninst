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
/*
 * $Log: BPatch_thread.h,v $
 * Revision 1.1  1997/03/18 19:43:37  buck
 * first commit of dyninst library.  Also includes:
 * 	moving templates from paradynd to dyninstAPI
 * 	converting showError into a function (in showerror.C)
 * 	many ifdefs for BPATCH_LIBRARY in dyinstAPI/src.
 *
 *
 */

#ifndef _BPatch_thread_h_
#define _BPatch_thread_h_

#include "BPatch_Vector.h"
#include "BPatch_image.h"
#include "BPatch_snippet.h"

class process;

/*
 * Used to specify whether a snippet is to be called before the instructions
 * at the point where it is inserted, or after.
 */
typedef enum {
    BPatch_callBefore,
    BPatch_callAfter
} BPatch_callWhen;


/*
 * Used to specify whether a snippet should be installed before other snippets
 * that have previously been inserted at the same point, or after.
 */
typedef enum {
    BPatch_firstSnippet,
    BPatch_lastSnippet
} BPatch_snippetOrder;


class BPatch_thread {
    friend bool	pollForStatusChange();

    static BPatch_Vector<BPatch_thread*> threadVec;

    static BPatch_thread *pidToThread(int pid);
    static bool		lib_inited;

    process		*proc;
    BPatch_image	*image;
    int			lastSignal;

    int			getPid();
public:

    BPatch_thread(char *path, char *argv[], char *envp[] = NULL);
    BPatch_thread(char *path, int pid);
    ~BPatch_thread();

    BPatch_image *getImage() { return image; }

    bool	stopExecution();
    bool	continueExecution();
    bool	terminateExecution();

    bool	isStopped();
    int		stopSignal();
    bool	isTerminated();

    bool	dumpCore(const char *file, bool terminate);

    BPatch_variableExpr	*malloc(int n);
    BPatch_variableExpr	*malloc(const BPatch_type &type);
    void	free(const BPatch_variableExpr &ptr);

    bool	insertSnippet(const BPatch_snippet &expr,
			      const BPatch_point &point,
			      BPatch_callWhen when = BPatch_callBefore,
			      BPatch_snippetOrder order = BPatch_firstSnippet);
    bool	insertSnippet(const BPatch_snippet &expr,
			      const BPatch_Vector<BPatch_point *> &points,
			      BPatch_callWhen when = BPatch_callBefore,
			      BPatch_snippetOrder order = BPatch_firstSnippet);
};

extern bool pollForStatusChange();

#endif /* BPatch_thread_h_ */
