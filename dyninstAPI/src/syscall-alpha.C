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

// $Id: syscall-alpha.C,v 1.2 2004/03/23 01:12:10 eli Exp $

#include <sys/procfs.h>
#include <sys/syscall.h>
#include "common/h/headers.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/syscallNotification.h"
#include "dyninstAPI/src/process.h"


syscallNotification::syscallNotification(syscallNotification *parentSN,
                                         process *p) : preForkInst(parentSN->preForkInst),
                                                       postForkInst(parentSN->postForkInst),
                                                       preExecInst(parentSN->preExecInst),
                                                       postExecInst(parentSN->postExecInst),
                                                       preExitInst(parentSN->preExitInst),
                                                       proc(p) {

    // We set PR_FORK in the parent, so everything was copied.
    
}

/////////// Prefork instrumentation 

bool syscallNotification::installPreFork() {
    // Get existing flags, add pre-fork, and set    
    sysset_t entryset;
    if (!proc->get_entry_syscalls(&entryset)) return false;
    
    praddset(&entryset, SYS_fork);
    praddset(&entryset, SYS_vfork);
    
    if (!proc->set_entry_syscalls(&entryset)) return false;
    preForkInst = SYSCALL_INSTALLED;
    
    
    return true;
}

/////////// Postfork instrumentation

bool syscallNotification::installPostFork() {
    // Get existing flags, add post-fork, and set
    
    sysset_t exitset;
    if (!proc->get_exit_syscalls(&exitset)) return false;
    praddset(&exitset, SYS_fork);
    praddset(&exitset, SYS_vfork);
    
    if (!proc->set_exit_syscalls(&exitset)) return false;

    postForkInst = SYSCALL_INSTALLED;
    return true;
}    

/////////// Pre-exec instrumentation

bool syscallNotification::installPreExec() {
    // Get existing flags, add pre-exec, and set
    
    sysset_t entryset;
    if (!proc->get_entry_syscalls(&entryset)) return false;
    
    praddset(&entryset, SYS_exec);
    praddset(&entryset, SYS_execve);
    
    if (!proc->set_entry_syscalls(&entryset)) return false;
    preExecInst = SYSCALL_INSTALLED;
    
    return true;
}

//////////// Post-exec instrumentation

bool syscallNotification::installPostExec() {
    // Get existing flags, add post-exec, and set
    
    sysset_t exitset;
    if (!proc->get_exit_syscalls(&exitset)) return false;
    
    praddset(&exitset, SYS_exec);
    praddset(&exitset, SYS_execve);
    
    if (!proc->set_exit_syscalls(&exitset)) return false;
    postExecInst = SYSCALL_INSTALLED;
    return true;
}    

/////////// Pre-exit instrumentation

bool syscallNotification::installPreExit() {
    // Get existing flags, add pre-exit, and set
    
    sysset_t entryset;
    if (!proc->get_entry_syscalls(&entryset)) return false;
    
    praddset(&entryset, SYS_exit);
    if (!proc->set_entry_syscalls(&entryset)) return false;
    preExitInst = SYSCALL_INSTALLED;
    return true;
}    

//////////////////////////////////////////////////////

/////// Remove pre-fork instrumentation

bool syscallNotification::removePreFork() {
    // Get existing flags, add pre-fork, and set    

    if (!preForkInst) return false;
    if (proc->hasExited()) {
        preForkInst = NULL;
        return true;
    }

    sysset_t entryset;
    if (!proc->get_entry_syscalls(&entryset)) return false;
    
    prdelset(&entryset, SYS_fork);
    prdelset(&entryset, SYS_vfork);
    
    if (!proc->set_entry_syscalls(&entryset)) return false;
    preForkInst = NULL;
    return true;
}

/////// Remove post-fork instrumentation

bool syscallNotification::removePostFork() {
    if (!postForkInst) return false;
    if (proc->hasExited()) {
        postForkInst = NULL;
        return true;
    }
    // Get existing flags, add post-fork, and set
    
    sysset_t exitset;
    if (!proc->get_exit_syscalls(&exitset)) return false;

    prdelset(&exitset, SYS_fork);
    prdelset(&exitset, SYS_vfork);
    
    if (!proc->set_exit_syscalls(&exitset)) return false;
    postForkInst = NULL;
    return true;
}

/////// Remove pre-exec instrumentation

bool syscallNotification::removePreExec() {
    if (!preExecInst) return false;
    if (proc->hasExited()) {
        preExecInst = NULL;
        return true;
    }
    // Get existing flags, add pre-exec, and set
    
    sysset_t entryset;
    if (!proc->get_entry_syscalls(&entryset)) return false;
    
    praddset(&entryset, SYS_exec);
    praddset(&entryset, SYS_execve);
    
    if (!proc->set_entry_syscalls(&entryset)) return false;
    preExecInst = NULL;
    return true;
}    

/////// Remove post-exec instrumentation

bool syscallNotification::removePostExec() {
    if (!postExecInst) return false;
    if (proc->hasExited()) {
        postExecInst = NULL;
        return true;
    }
    // Get existing flags, add post-exec, and set
    
    sysset_t exitset;
    if (!proc->get_exit_syscalls(&exitset)) return false;
    
    prdelset(&exitset, SYS_exec);
    prdelset(&exitset, SYS_execve);
    
    if (!proc->set_exit_syscalls(&exitset)) return false;
    postExecInst = NULL;
    return true;
}

/////// Remove pre-exit instrumentation

bool syscallNotification::removePreExit() {
    if (!preExitInst) return false;
    if (proc->hasExited()) {
        preExitInst = NULL;
        return true;
    }

    // Get existing flags, add pre-exit, and set
    
    sysset_t entryset;
    if (!proc->get_entry_syscalls(&entryset)) return false;
    
    prdelset(&entryset, SYS_exit);
    if (!proc->set_entry_syscalls(&entryset)) return false;
    preExitInst = NULL;
    return true;
}


