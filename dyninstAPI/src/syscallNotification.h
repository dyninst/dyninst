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

// $Id: syscallNotification.h,v 1.1 2004/03/02 22:46:18 bernat Exp $

#if !defined(SYSCALL_NOTIFICATION_H)
#define SYSCALL_NOTIFICATION_H

// Non-NULL for platforms where we don't need the instMapping
#define SYSCALL_INSTALLED ((instMapping *)1)

class process;
class instMapping;

class syscallNotification {
  private:
    // If we use instrumentation to get notification of a syscall
    instMapping *preForkInst;
    instMapping *postForkInst;
    instMapping *preExecInst;
    instMapping *postExecInst;
    instMapping *preExitInst;
    process *proc;
    
  public:
    syscallNotification() :
    preForkInst(NULL), postForkInst(NULL),
    preExecInst(NULL), postExecInst(NULL),
    preExitInst(NULL) { assert(0 && "ILLEGAL USE OF DEFAULT CONSTRUCTOR"); }

    syscallNotification(process *p) :
    preForkInst(NULL), postForkInst(NULL),
    preExecInst(NULL), postExecInst(NULL),
    preExitInst(NULL), proc(p) {};

    // fork constructor
    syscallNotification(syscallNotification *parentSN,
                        process *p);
    
    ~syscallNotification() {
        // These must check if the process is exited before doing anything
        // dangerous
        if (preForkInst) removePreFork();
        if (postForkInst) removePostFork();
        if (preExecInst) removePreExec();
        if (postExecInst) removePostExec();
        if (preExitInst) removePreExit();
    }
    
    bool installPreFork();
    bool installPostFork();
    bool installPreExec();
    bool installPostExec();
    bool installPreExit();

    bool removePreFork();
    bool removePostFork();
    bool removePreExec();
    bool removePostExec();
    bool removePreExit();
};

#endif

