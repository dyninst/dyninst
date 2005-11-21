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

// $Id: syscall-linux.C,v 1.13 2005/11/21 17:16:14 jaw Exp $

#if defined( arch_x86 ) || defined( arch_x86_64 )
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

#include "dyninstAPI/src/miniTramp.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/syscallNotification.h"
#include "dyninstAPI/src/process.h"

extern bool getInheritedMiniTramp(const miniTramp *parentMT,
                                  miniTramp *&childMT,
                                  process *childProc);

syscallNotification::syscallNotification(syscallNotification *parentSN,
                                         process *child) : preForkInst(NULL),
                                                           postForkInst(NULL),
                                                           preExecInst(NULL),
                                                           postExecInst(NULL),
                                                           preExitInst(NULL),
                                                           proc(child) {
    // We need to copy over the instMappings and get the new miniTramps from
    // the parent process
    // We don't copy the instMappings, but make new copies.
    if (parentSN->preForkInst) {
        preForkInst = new instMapping(parentSN->preForkInst, child);
    }
    if (parentSN->preExecInst) {
        preExecInst = new instMapping(parentSN->preExecInst, child);
    }
    if (parentSN->postExecInst) {
        postExecInst = new instMapping(parentSN->postExecInst, child);
    }
    if (parentSN->preExitInst) {
        preExitInst = new instMapping(parentSN->preExitInst, child);
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
    if (preForkInst->miniTramps.size() == 0)
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
    postForkInst->canUseTrap(false);
    removeAst(returnVal);

    pdvector<instMapping *> instReqs;
    instReqs.push_back(postForkInst);
    
    proc->installInstrRequests(instReqs);

    // Check to see if we put anything in the proggie
    if (postForkInst->miniTramps.size() == 0)
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
    if (preExecInst->miniTramps.size() == 0)
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

    preExitInst->allow_trap = true;

    pdvector<instMapping *> instReqs;
    instReqs.push_back(preExitInst);
    
    proc->installInstrRequests(instReqs);
    // Check to see if we put anything in the proggie
    if (preExitInst->miniTramps.size() == 0)
        return false;
    return true;
}    

bool syscallNotification::installPreLwpExit() {
   preLwpExitInst = NULL;
   return true;
}

//////////////////////////////////////////////////////

/////// Remove pre-fork instrumentation

bool syscallNotification::removePreFork() {
    if (!proc->isAttached() || proc->execing()) {
        delete preForkInst;
        preForkInst = NULL;
        return true;
    }
    
    if (!preForkInst) return false;
    
    miniTramp *handle;
    for (unsigned i = 0; i < preForkInst->miniTramps.size(); i++) {
        handle = preForkInst->miniTramps[i];
        
        bool removed = handle->uninstrument();
        // At some point we should handle a negative return... but I
        // have no idea how.
        assert(removed);
        // The miniTramp is deleted when the miniTramp is freed, so
        // we don't have to.
    }
    delete preForkInst;
    preForkInst = NULL;
    return true;
}

    

/////// Remove post-fork instrumentation

bool syscallNotification::removePostFork() {

    if (!postForkInst) return false;

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

    

/////// Remove pre-exec instrumentation

bool syscallNotification::removePreExec() {
    if (!preExecInst) return false;

    if (!proc->isAttached() || proc->execing()) {
        delete preExecInst;
        preExecInst = NULL;
        return true;
    }
    
    miniTramp *handle;
    for (unsigned i = 0; i < preExecInst->miniTramps.size(); i++) {
        handle = preExecInst->miniTramps[i];
        
        bool removed = handle->uninstrument();
        // At some point we should handle a negative return... but I
        // have no idea how.
        assert(removed);
        // The miniTramp is deleted when the miniTramp is freed, so
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
    if (!proc->isAttached() || proc->execing()) {
        delete preExitInst;
        preExitInst = NULL;
        return true;
    }
    
    miniTramp *handle;
    for (unsigned i = 0; i < preExitInst->miniTramps.size(); i++) {
        handle = preExitInst->miniTramps[i];
        
        bool removed = handle->uninstrument();
        // At some point we should handle a negative return... but I
        // have no idea how.
        assert(removed);
        // The miniTramp is deleted when the miniTramp is freed, so
        // we don't have to.
    }
    delete preExitInst;
    preExitInst = NULL;
    return true;
}

bool syscallNotification::removePreLwpExit() {
   return true;
}

//////////////////////////////////////////////

