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

// $Id: syscall-linux.C,v 1.6 2004/04/01 23:06:29 tlmiller Exp $

#if defined( arch_x86 )
#define FORK_FUNC "__libc_fork"
#define EXEC_FUNC "__execve"
#define EXIT_FUNC "_exit"
#elif defined( arch_ia64 )
#define FORK_FUNC "__libc_fork"
#define EXEC_FUNC "execve"
#define EXIT_FUNC "_exit"
#else
#error Unsupported linux platform.
#endif

#include "common/h/headers.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/syscallNotification.h"
#include "dyninstAPI/src/process.h"

extern bool getInheritedMiniTramp(const miniTrampHandle *parentMT,
                                  miniTrampHandle *&childMT,
                                  process *childProc);

syscallNotification::syscallNotification(syscallNotification *parentSN,
                                         process *p) : preForkInst(NULL),
                                                       postForkInst(NULL),
                                                       preExecInst(NULL),
                                                       postExecInst(NULL),
                                                       preExitInst(NULL),
                                                       proc(p) {
    unsigned iter;
    // We need to copy over the instMappings and get the new mtHandles from
    // the parent process
    // We don't copy the instMappings, but make new copies.
    if (parentSN->preForkInst) {
        preForkInst = new instMapping(FORK_FUNC, 
                                      "DYNINST_instForkEntry",
                                      FUNC_ENTRY);
        for (iter = 0; iter < parentSN->preForkInst->mtHandles.size(); iter++) {
            miniTrampHandle *child = NULL;
            getInheritedMiniTramp(parentSN->preForkInst->mtHandles[iter],
                                  child,
                                  proc);
            preForkInst->mtHandles.push_back(child);
        }
    }
    if (parentSN->postForkInst) {
        AstNode *returnVal = new AstNode(AstNode::ReturnVal, (void *)0);
        postForkInst = new instMapping(FORK_FUNC, "DYNINST_instForkExit",
                                       FUNC_EXIT|FUNC_ARG,
                                       returnVal);
        postForkInst->dontUseTrampGuard();
        removeAst(returnVal);
        for (iter = 0; iter < parentSN->postForkInst->mtHandles.size(); iter++) {
            miniTrampHandle *child = NULL;
            getInheritedMiniTramp(parentSN->postForkInst->mtHandles[iter],
                                  child,
                                  proc);
            postForkInst->mtHandles.push_back(child);
        }
    }
    if (parentSN->preExecInst) {
        AstNode *arg0 = new AstNode(AstNode::Param, (void *)0);
        preExecInst = new instMapping(EXEC_FUNC, "DYNINST_instExecEntry",
                                      FUNC_ENTRY|FUNC_ARG,
                                      arg0);
        removeAst(arg0);
        for (iter = 0; iter < parentSN->preExecInst->mtHandles.size(); iter++) {
            miniTrampHandle *child = NULL;
            getInheritedMiniTramp(parentSN->preExecInst->mtHandles[iter],
                                  child,
                                  proc);
            preExecInst->mtHandles.push_back(child);
        }
    }
    if (parentSN->postExecInst) {
        // Nothing
    }
    if (parentSN->preExitInst) {
        AstNode *arg0 = new AstNode(AstNode::Param, (void *)0);
        preExitInst = new instMapping(EXIT_FUNC, "DYNINST_instExitEntry",
                                      FUNC_ENTRY|FUNC_ARG,
                                      arg0);
        removeAst(arg0);
        for (iter = 0; iter < parentSN->preExitInst->mtHandles.size(); iter++) {
            miniTrampHandle *child = NULL;
            getInheritedMiniTramp(parentSN->preExitInst->mtHandles[iter],
                                  child,
                                  proc);
            preExitInst->mtHandles.push_back(child);
        }
    }
}

/////////// Prefork instrumentation 

bool syscallNotification::installPreFork() {
    preForkInst = new instMapping(FORK_FUNC,
                                  "DYNINST_instForkEntry",
                                  FUNC_ENTRY);
    pdvector<instMapping *> instReqs;
    instReqs.push_back(preForkInst);
    
    proc->installInstrRequests(instReqs);

    // Check to see if we put anything in the proggie
    if (preForkInst->mtHandles.size() == 0)
        return false;
    return true;
}

/////////// Postfork instrumentation

bool syscallNotification::installPostFork() {
    AstNode *returnVal = new AstNode(AstNode::ReturnVal, (void *)0);
    postForkInst = new instMapping(FORK_FUNC, "DYNINST_instForkExit",
                                   FUNC_EXIT|FUNC_ARG,
                                   returnVal);
    postForkInst->dontUseTrampGuard();
    removeAst(returnVal);

    pdvector<instMapping *> instReqs;
    instReqs.push_back(postForkInst);
    
    proc->installInstrRequests(instReqs);

    // Check to see if we put anything in the proggie
    if (postForkInst->mtHandles.size() == 0)
        return false;
    return true;
}    

/////////// Pre-exec instrumentation

bool syscallNotification::installPreExec() {
    AstNode *arg0 = new AstNode(AstNode::Param, (void *)0);
    preExecInst = new instMapping(EXEC_FUNC, "DYNINST_instExecEntry",
                                   FUNC_ENTRY|FUNC_ARG,
                                   arg0);
    removeAst(arg0);

    pdvector<instMapping *> instReqs;
    instReqs.push_back(preExecInst);
    
    proc->installInstrRequests(instReqs);

    // Check to see if we put anything in the proggie
    if (preExecInst->mtHandles.size() == 0)
        return false;
    return true;
}    

//////////// Post-exec instrumentation

bool syscallNotification::installPostExec() {
    // OS-handled
    postExecInst = NULL;
    return true;
}    

/////////// Pre-exit instrumentation

bool syscallNotification::installPreExit() {
    AstNode *arg0 = new AstNode(AstNode::Param, (void *)0);
    preExitInst = new instMapping(EXIT_FUNC, "DYNINST_instExitEntry",
                                  FUNC_ENTRY|FUNC_ARG,
                                  arg0);
    removeAst(arg0);

    pdvector<instMapping *> instReqs;
    instReqs.push_back(preExitInst);
    
    proc->installInstrRequests(instReqs);
    // Check to see if we put anything in the proggie
    if (preExitInst->mtHandles.size() == 0)
        return false;
    return true;
}    

//////////////////////////////////////////////////////

/////// Remove pre-fork instrumentation

bool syscallNotification::removePreFork() {
    if (!proc->isAttached()) {
        delete preForkInst;
        preForkInst = NULL;
        return true;
    }
    
    if (!preForkInst) return false;
    
    miniTrampHandle *handle;
    for (unsigned i = 0; i < preForkInst->mtHandles.size(); i++) {
        handle = preForkInst->mtHandles[i];
        
        bool removed = deleteInst(proc, handle);
        // At some point we should handle a negative return... but I
        // have no idea how.
        assert(removed);
        // The miniTrampHandle is deleted when the miniTramp is freed, so
        // we don't have to.
    }
    delete preForkInst;
    preForkInst = NULL;
    return true;
}

    

/////// Remove post-fork instrumentation

bool syscallNotification::removePostFork() {

    if (!postForkInst) return false;

    if (!proc->isAttached()) {
        delete postForkInst;
        postForkInst = NULL;
        return true;
    }
    
    miniTrampHandle *handle;
    for (unsigned i = 0; i < postForkInst->mtHandles.size(); i++) {
        handle = postForkInst->mtHandles[i];
        
        bool removed = deleteInst(proc, handle);
        // At some point we should handle a negative return... but I
        // have no idea how.
        assert(removed);
        // The miniTrampHandle is deleted when the miniTramp is freed, so
        // we don't have to.
    }
    delete postForkInst;
    postForkInst = NULL;
    return true;
}

    

/////// Remove pre-exec instrumentation

bool syscallNotification::removePreExec() {
    if (!preExecInst) return false;

    if (!proc->isAttached()) {
        delete preExecInst;
        preExecInst = NULL;
        return true;
    }
    
    miniTrampHandle *handle;
    for (unsigned i = 0; i < preExecInst->mtHandles.size(); i++) {
        handle = preExecInst->mtHandles[i];
        
        bool removed = deleteInst(proc, handle);
        // At some point we should handle a negative return... but I
        // have no idea how.
        assert(removed);
        // The miniTrampHandle is deleted when the miniTramp is freed, so
        // we don't have to.
    }
    delete preExecInst;
    preExecInst = NULL;
    return true;
}

/////// Remove post-exec instrumentation

bool syscallNotification::removePostExec() {
    // OS traps this, we don't have a choice.
    return true;
}

/////// Remove pre-exit instrumentation

bool syscallNotification::removePreExit() {
    if (!preExitInst) return false;

    if (!proc->isAttached()) {
        delete preExitInst;
        preExitInst = NULL;
        return true;
    }
    
    miniTrampHandle *handle;
    for (unsigned i = 0; i < preExitInst->mtHandles.size(); i++) {
        handle = preExitInst->mtHandles[i];
        
        bool removed = deleteInst(proc, handle);
        // At some point we should handle a negative return... but I
        // have no idea how.
        assert(removed);
        // The miniTrampHandle is deleted when the miniTramp is freed, so
        // we don't have to.
    }
    delete preExitInst;
    preExitInst = NULL;
    return true;
}

//////////////////////////////////////////////

