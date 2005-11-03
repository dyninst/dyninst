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

// $Id: syscall-solproc.C,v 1.9 2005/11/03 05:21:08 jaw Exp $

#if defined(os_aix)
#include <sys/procfs.h>
#else
#include <procfs.h>
#endif
#include "common/h/headers.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/syscallNotification.h"
#include "dyninstAPI/src/sol_proc.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/miniTramp.h"


#if defined(bug_aix_proc_broken_fork)
#include "dyninstAPI/src/symtab.h"
#define FORK_FUNC "__fork"
#endif

syscallNotification::syscallNotification(syscallNotification *parentSN,
                                         process *child) :
    preForkInst(parentSN->preForkInst),
    postForkInst(parentSN->postForkInst),
    preExecInst(parentSN->preExecInst),
    postExecInst(parentSN->postExecInst),
    preExitInst(parentSN->preExitInst),
    proc(child) {

    // We set PR_FORK in the parent, so everything was copied.
    
#if defined(bug_aix_proc_broken_fork)
    if (parentSN->postForkInst) {
        postForkInst = new instMapping(parentSN->postForkInst, child);
    }
#endif

}

/////////// Prefork instrumentation 

bool syscallNotification::installPreFork() {
    // Get existing flags, add pre-fork, and set
    int proc_pid = proc->getPid();
    
    sysset_t *entryset = SYSSET_ALLOC(proc_pid);
    
    if (!proc->get_entry_syscalls(entryset)) return false;

    if (SYSSET_MAP(SYS_fork, proc_pid) != -1) {
        praddsysset(entryset, SYSSET_MAP(SYS_fork, proc_pid));
    }
    if (SYSSET_MAP(SYS_fork1, proc_pid) != -1) {
        praddsysset(entryset, SYSSET_MAP(SYS_fork1, proc_pid));
    }
    if (SYSSET_MAP(SYS_vfork, proc_pid) != -1) {
        praddsysset(entryset, SYSSET_MAP(SYS_vfork, proc_pid));
    }
    if (!proc->set_entry_syscalls(entryset)) return false;;
    SYSSET_FREE(entryset);
    // Make sure our removal code gets run
    preForkInst = SYSCALL_INSTALLED;
    return true;
}

/////////// Postfork instrumentation

bool syscallNotification::installPostFork() {
#if defined(bug_aix_proc_broken_fork)

    fprintf(stderr, "%s[%d][%s]:  welcome to installPostFork\n", __FILE__, __LINE__, getThreadStr(getExecThreadID()));
    AstNode *returnVal = new AstNode(AstNode::ReturnVal, (void *)0);
    postForkInst = new instMapping(FORK_FUNC, "DYNINST_instForkExit",
                                   FUNC_EXIT|FUNC_ARG,
                                   returnVal);
    postForkInst->dontUseTrampGuard();
    removeAst(returnVal);

    pdvector<instMapping *> instReqs;
    instReqs.push_back(postForkInst);
    
    fprintf(stderr, "%s[%d]:  before installInstrRequests\n", __FILE__, __LINE__);
    proc->installInstrRequests(instReqs);
    fprintf(stderr, "%s[%d]:  after installInstrRequests\n", __FILE__, __LINE__);

    // Check to see if we put anything in the proggie
    if (postForkInst->miniTramps.size() == 0)
        return false;
    return true;


#else
    // Get existing flags, add post-fork, and set
    int proc_pid = proc->getPid();
    
    sysset_t *exitset = SYSSET_ALLOC(proc_pid);
    
    if (!proc->get_exit_syscalls(exitset)) return false;;

    if (SYSSET_MAP(SYS_fork, proc_pid) != -1) {
        praddsysset(exitset, SYSSET_MAP(SYS_fork, proc_pid));
    }
    if (SYSSET_MAP(SYS_fork1, proc_pid) != -1) {
        praddsysset(exitset, SYSSET_MAP(SYS_fork1, proc_pid));
    }
    if (SYSSET_MAP(SYS_vfork, proc_pid) != -1) {
        praddsysset(exitset, SYSSET_MAP(SYS_vfork, proc_pid));
    }
    if (!proc->set_exit_syscalls(exitset)) return false;;
    SYSSET_FREE(exitset);
    // Make sure our removal code gets run
    postForkInst = SYSCALL_INSTALLED;
    return true;
#endif
}    

/////////// Pre-exec instrumentation

bool syscallNotification::installPreExec() {
    // Get existing flags, add pre-exec, and set
    int proc_pid = proc->getPid();
    
    sysset_t *entryset = SYSSET_ALLOC(proc_pid);
    
    if (!proc->get_entry_syscalls(entryset)) return false;;

    if (SYSSET_MAP(SYS_exec, proc_pid) != -1) {
        praddsysset(entryset, SYSSET_MAP(SYS_exec, proc_pid));
    }
    if (SYSSET_MAP(SYS_execve, proc_pid) != -1) {
        praddsysset(entryset, SYSSET_MAP(SYS_execve, proc_pid));
    }
    if (!proc->set_entry_syscalls(entryset)) return false;;
    SYSSET_FREE(entryset);
    // Make sure our removal code gets run
    preExecInst = SYSCALL_INSTALLED;
    return true;
}    

//////////// Post-exec instrumentation

bool syscallNotification::installPostExec() {
    // Get existing flags, add post-exec, and set
    int proc_pid = proc->getPid();
    
    sysset_t *exitset = SYSSET_ALLOC(proc_pid);
    
    if (!proc->get_exit_syscalls(exitset)) return false;;

    if (SYSSET_MAP(SYS_exec, proc_pid) != -1) {
        praddsysset(exitset, SYSSET_MAP(SYS_exec, proc_pid));
    }
    if (SYSSET_MAP(SYS_execve, proc_pid) != -1) {
        praddsysset(exitset, SYSSET_MAP(SYS_execve, proc_pid));
    }
    if (!proc->set_exit_syscalls(exitset)) return false;;
    SYSSET_FREE(exitset);
    // Make sure our removal code gets run
    postExecInst = SYSCALL_INSTALLED;
    return true;
}    

/////////// Pre-exit instrumentation

bool syscallNotification::installPreExit() {
    // Get existing flags, add pre-exit, and set
    int proc_pid = proc->getPid();
    
    sysset_t *entryset = SYSSET_ALLOC(proc_pid);
    
    if (!proc->get_entry_syscalls(entryset)) return false;;

    if (SYSSET_MAP(SYS_exit, proc_pid) != -1) {
        praddsysset(entryset, SYSSET_MAP(SYS_exit, proc_pid));
    }
    if (!proc->set_entry_syscalls(entryset)) return false;;
    // Make sure our removal code gets run
    preExitInst = SYSCALL_INSTALLED;
    SYSSET_FREE(entryset);
    return true;
}    


/////////// Pre-lwp-exit instrumentation

bool syscallNotification::installPreLwpExit() {
    // Get existing flags, add pre-lwp-exit, and set
    int proc_pid = proc->getPid();
    
    sysset_t *entryset = SYSSET_ALLOC(proc_pid);
    
    if (!proc->get_entry_syscalls(entryset)) return false;;

    if (SYSSET_MAP(SYS_lwp_exit, proc_pid) != -1) {
        praddsysset(entryset, SYSSET_MAP(SYS_lwp_exit, proc_pid));
    }
    if (!proc->set_entry_syscalls(entryset)) return false;;
    // Make sure our removal code gets run
    preLwpExitInst = SYSCALL_INSTALLED;
    SYSSET_FREE(entryset);
    return true;
}    


//////////////////////////////////////////////////////

/////// Remove pre-fork instrumentation

bool syscallNotification::removePreFork() {
    if (!preForkInst) return false;
    if (!proc->isAttached() || proc->execing()) {
        preForkInst = NULL;
        return true;
    }
    
    // Get existing flags, add pre-fork, and set
    int proc_pid = proc->getPid();
    
    sysset_t *entryset = SYSSET_ALLOC(proc_pid);
    
    if (!proc->get_entry_syscalls(entryset)) return false;;

    if (SYSSET_MAP(SYS_fork, proc_pid) != -1) {
        prdelsysset(entryset, SYSSET_MAP(SYS_fork, proc_pid));
    }
    if (SYSSET_MAP(SYS_fork1, proc_pid) != -1) {
        prdelsysset(entryset, SYSSET_MAP(SYS_fork1, proc_pid));
    }
    if (SYSSET_MAP(SYS_vfork, proc_pid) != -1) {
        prdelsysset(entryset, SYSSET_MAP(SYS_vfork, proc_pid));
    }
    if (!proc->set_entry_syscalls(entryset)) return false;;
    SYSSET_FREE(entryset);
    preForkInst = NULL;
    return true;
}

/////// Remove post-fork instrumentation

bool syscallNotification::removePostFork() {
    if (!postForkInst) return false;

#if defined(bug_aix_proc_broken_fork) 

    if (!proc->isAttached() || proc->execing()) {
        delete postForkInst;
        postForkInst = NULL;
        return true;
    }
    
    miniTramp *handle;
    for (unsigned i = 0; i < postForkInst->miniTramps.size(); i++) {
        handle = postForkInst->miniTramps[i];
        
        bool removed = handle->uninstrument();
        // At some point we should handle a negative return... but I
        // have no idea how.
        
        assert(removed);
        // The miniTramp is deleted when the miniTramp is freed, so
        // we don't have to.
    } 
    delete postForkInst;
    postForkInst = NULL;
    return true;
#else

    if (!proc->isAttached() || proc->execing()) {
        postForkInst = NULL;
        return true;
    }

    // Get existing flags, add post-fork, and set
    int proc_pid = proc->getPid();
    
    sysset_t *exitset = SYSSET_ALLOC(proc_pid);
    
    if (!proc->get_exit_syscalls(exitset)) return false;;

    if (SYSSET_MAP(SYS_fork, proc_pid) != -1) {
        prdelsysset(exitset, SYSSET_MAP(SYS_fork, proc_pid));
    }
    if (SYSSET_MAP(SYS_fork1, proc_pid) != -1) {
        prdelsysset(exitset, SYSSET_MAP(SYS_fork1, proc_pid));
    }
    if (SYSSET_MAP(SYS_vfork, proc_pid) != -1) {
        prdelsysset(exitset, SYSSET_MAP(SYS_vfork, proc_pid));
    }
    if (!proc->set_exit_syscalls(exitset)) return false;;
    SYSSET_FREE(exitset);
    postForkInst = NULL;
    return true;
#endif
}

/////// Remove pre-exec instrumentation

bool syscallNotification::removePreExec() {
    if (!preExecInst) {
        bperr("Tracing never installed\n");
        return false;
    }
    
    if (!proc->isAttached() || proc->execing()) {
        preExecInst = NULL;
        return true;
    }

    // Get existing flags, add pre-exec, and set
    int proc_pid = proc->getPid();
    
    sysset_t *entryset = SYSSET_ALLOC(proc_pid);
    
    if (!proc->get_entry_syscalls(entryset)) return false;;

    if (SYSSET_MAP(SYS_exec, proc_pid) != -1) {
        prdelsysset(entryset, SYSSET_MAP(SYS_exec, proc_pid));
    }
    if (SYSSET_MAP(SYS_execve, proc_pid) != -1) {
        prdelsysset(entryset, SYSSET_MAP(SYS_execve, proc_pid));
    }
    if (!proc->set_entry_syscalls(entryset)) return false;;
    SYSSET_FREE(entryset);
    preExecInst = NULL;
    return true;

}    

/////// Remove post-exec instrumentation

bool syscallNotification::removePostExec() {
    if (!postExecInst) return false;
    // <whistles>
    if (!proc->isAttached() || proc->execing()) {
        postExecInst = NULL;
        return true;
    }

    // Get existing flags, add post-exec, and set
    int proc_pid = proc->getPid();
    
    sysset_t *exitset = SYSSET_ALLOC(proc_pid);
    
    if (!proc->get_exit_syscalls(exitset)) return false;;

    if (SYSSET_MAP(SYS_exec, proc_pid) != -1) {
        prdelsysset(exitset, SYSSET_MAP(SYS_exec, proc_pid));
    }
    if (SYSSET_MAP(SYS_execve, proc_pid) != -1) {
        prdelsysset(exitset, SYSSET_MAP(SYS_execve, proc_pid));
    }
    if (!proc->set_exit_syscalls(exitset)) return false;;
    SYSSET_FREE(exitset);
    postExecInst = NULL;
    return true;
}

/////// Remove pre-exit instrumentation

bool syscallNotification::removePreExit() {
    if (!preExitInst) return false;
    if (!proc->isAttached() || proc->execing()) {
        preExitInst = NULL;
        return true;
    }

    // Get existing flags, add pre-exit, and set
    int proc_pid = proc->getPid();
    
    sysset_t *entryset = SYSSET_ALLOC(proc_pid);
    
    if (!proc->get_entry_syscalls(entryset)) return false;;

    if (SYSSET_MAP(SYS_exit, proc_pid) != -1) {
        prdelsysset(entryset, SYSSET_MAP(SYS_exit, proc_pid));
    }
    if (!proc->set_entry_syscalls(entryset)) return false;;
    SYSSET_FREE(entryset);
    preExitInst = NULL;
    
    return true;
}

/////// Remove pre-exit instrumentation

bool syscallNotification::removePreLwpExit() {
    if (!preLwpExitInst) return false;
    if (!proc->isAttached()) {
        preLwpExitInst = NULL;
        return true;
    }

    // Get existing flags, add pre-exit, and set
    int proc_pid = proc->getPid();
    
    sysset_t *entryset = SYSSET_ALLOC(proc_pid);
    
    if (!proc->get_entry_syscalls(entryset)) return false;;

    if (SYSSET_MAP(SYS_lwp_exit, proc_pid) != -1) {
        prdelsysset(entryset, SYSSET_MAP(SYS_lwp_exit, proc_pid));
    }
    if (!proc->set_entry_syscalls(entryset)) return false;;
    SYSSET_FREE(entryset);
    preLwpExitInst = NULL;
    
    return true;
}


