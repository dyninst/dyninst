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

// $Id: syscall-solproc.C,v 1.22 2008/09/19 00:56:09 jaw Exp $

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
#include "dyninstAPI/src/EventHandler.h"
#include "dyninstAPI/src/symtab.h"

#include "dyninstAPI/src/ast.h"

#define FORK_FUNC "fork"
#define FORK_LIB  "libc.a"

#define LWP_EXIT_FUNC "_thr_exit_common"

syscallNotification::syscallNotification(syscallNotification *parentSN,
                                         process *child) :
    preForkInst(parentSN->preForkInst),
    postForkInst(parentSN->postForkInst),
    preExecInst(parentSN->preExecInst),
    postExecInst(parentSN->postExecInst),
    preExitInst(parentSN->preExitInst),
    preLwpExitInst(parentSN->preLwpExitInst),
    proc(child) {

    // We set PR_FORK in the parent, so everything was copied.
    
    if (parentSN->postForkInst &&
        (parentSN->postForkInst != SYSCALL_INSTALLED))
        postForkInst = new instMapping(parentSN->postForkInst, child);

    // I thought we needed this for a while, but we don't. Leave it here 
    // anyway
    if (parentSN->preLwpExitInst &&
        (parentSN->preLwpExitInst != SYSCALL_INSTALLED))
        preLwpExitInst = new instMapping(parentSN->preLwpExitInst, child);

}

/////////// Prefork instrumentation 

bool syscallNotification::installPreFork() {
    // Get existing flags, add pre-fork, and set
    SYSSET_DECLAREPID(proc_pid, proc->getPid());
    
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

bool syscallNotification::installPostFork() 
{
#if defined(os_aix)
    /* Block-comment
       
    On AIX, for reasons which are unclear, we cannot count on the OS
    to copy the address space of the parent to the child on fork. In
    particular, library text and the a.out text are _not_
    copied. Areas allocated with mmap (data areas, effectively) are.

    Also, if we tell the process to stop (via /proc) on the exit of
    fork, we see one of two cases: either the child does not stop (AIX
    5.1), or the child stops in an unmodifiable state (AIX
    5.2). Neither is acceptable.

    Our solution is to use instrumentation at the exit of
    fork(). However, this is complicated by the non-copy behavior of
    AIX. Our solution is relocation; we ensure that the fork() that is
    executed is located in a data area. As a result, it is copied to
    the child, and the child picks up executing there.
    */


    AstNodePtr returnVal = AstNode::operandNode(AstNode::ReturnVal, (void *)0);
    std::string ffunc(FORK_FUNC);
    std::string ftarget("DYNINST_instForkExit");
    std::string flib(FORK_LIB);
    postForkInst = new instMapping(ffunc, ftarget,
                                   FUNC_EXIT|FUNC_ARG,
                                   returnVal,
                                   flib);
#if 0
    postForkInst = new instMapping(FORK_FUNC, "DYNINST_instForkExit",
                                   FUNC_EXIT|FUNC_ARG,
                                   returnVal,
                                   FORK_LIB);
#endif
    postForkInst->dontUseTrampGuard();
    
    pdvector<instMapping *> instReqs;
    instReqs.push_back(postForkInst);
    
    proc->installInstrRequests(instReqs);
    
    // Check to see if we put anything in the proggie
    if (postForkInst->miniTramps.size() == 0) {
       fprintf(stderr, "%s[%d]:  WARNING:  failed to generate inst for post-fork\n", 
             FILE__, __LINE__);
        return false;
    }
    return true;
#else
    // Get existing flags, add post-fork, and set
    SYSSET_DECLAREPID(proc_pid,proc->getPid()); 
    
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
    SYSSET_DECLAREPID(proc_pid,proc->getPid());
    
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
    SYSSET_DECLAREPID(proc_pid,proc->getPid());
    
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
    SYSSET_DECLAREPID(proc_pid, proc->getPid());
    
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
    SYSSET_DECLAREPID(proc_pid, proc->getPid());
    
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

#if 0
    preLwpExitInst = new instMapping(LWP_EXIT_FUNC, "DYNINST_instLwpExit",
                                   FUNC_ENTRY);
    preLwpExitInst->dontUseTrampGuard();
    
    pdvector<instMapping *> instReqs;
    instReqs.push_back(preLwpExitInst);
    
    proc->installInstrRequests(instReqs);
    
    // Check to see if we put anything in the proggie
    if (preLwpExitInst->miniTramps.size() == 0)
        return false;
    return true;
#endif
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
    SYSSET_DECLAREPID(proc_pid,proc->getPid());
    
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

    if (postForkInst != SYSCALL_INSTALLED) {
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
    }
    
    // else...

    if (!proc->isAttached() || proc->execing()) {
        postForkInst = NULL;
        return true;
    }

    // Get existing flags, add post-fork, and set
    SYSSET_DECLAREPID(proc_pid,proc->getPid());
    
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
    SYSSET_DECLAREPID(proc_pid,proc->getPid());
    
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
    SYSSET_DECLAREPID(proc_pid, proc->getPid());
    
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
    SYSSET_DECLAREPID(proc_pid, proc->getPid());
    
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

    if (preLwpExitInst != SYSCALL_INSTALLED) {
        if (!proc->isAttached() || proc->execing()) {
            delete preLwpExitInst;
            preLwpExitInst = NULL;
            return true;
        }
        
        miniTramp *handle;
        for (unsigned i = 0; i < preLwpExitInst->miniTramps.size(); i++) {
            handle = preLwpExitInst->miniTramps[i];
            
            bool removed = handle->uninstrument();
            // At some point we should handle a negative return... but I
            // have no idea how.
            
            assert(removed);
            // The miniTramp is deleted when the miniTramp is freed, so
            // we don't have to.
        } 
        delete preLwpExitInst;
        preLwpExitInst = NULL;
        return true;
    }

    if (!proc->isAttached()) {
        preLwpExitInst = NULL;
        return true;
    }

    // Get existing flags, add pre-exit, and set
    SYSSET_DECLAREPID(proc_pid, proc->getPid());
    
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


