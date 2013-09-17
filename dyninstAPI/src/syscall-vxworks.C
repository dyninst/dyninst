/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

// $Id: syscall-linux.C,v 1.20 2008/05/28 17:14:19 legendre Exp $

#define FORK_FUNC "__libc_fork"
#define EXEC_FUNC "execve"
#define EXIT_FUNC "_exit"

#include "common/src/headers.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/syscallNotification.h"
#include "dyninstAPI/src/dynProcess.h"
#include "dyninstAPI/src/ast.h"

extern bool getInheritedMiniTramp(const miniTramp *parentMT,
                                  miniTramp *&childMT,
                                  PCProcess *childProc);

syscallNotification::syscallNotification(syscallNotification *parentSN,
                                         PCProcess *child) : preForkInst(NULL),
                                                           postForkInst(NULL),
                                                           preExecInst(NULL),
                                                           postExecInst(NULL),
                                                           preExitInst(NULL),
                                                           preLwpExitInst(NULL),
                                                           proc(child) {
    // We need to copy over the instMappings and get the new miniTramps from
    // the parent process
    // We don't copy the instMappings, but make new copies.
    if (parentSN->preForkInst) {
        preForkInst = new instMapping(parentSN->preForkInst, child);
    }
    if (parentSN->postForkInst) {
        postForkInst = new instMapping(parentSN->postForkInst, child);
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

    if (parentSN->preLwpExitInst) {
        preLwpExitInst = new instMapping(parentSN->preLwpExitInst, child);
    }
}

/////////// Prefork instrumentation 

bool syscallNotification::installPreFork() {
    // No fork on VxWorks.
    preForkInst = NULL;
    return true;
}

/////////// Postfork instrumentation

bool syscallNotification::installPostFork() {
    // No fork on VxWorks.
    postForkInst = NULL;
    return true;
}    

/////////// Pre-exec instrumentation

bool syscallNotification::installPreExec() {
    // No exec on VxWorks.
    preExecInst = NULL;
    return true;
}    

//////////// Post-exec instrumentation

bool syscallNotification::installPostExec() {
    // No exec on VxWorks.
    postExecInst = NULL;
    return true;
}    

/////////// Pre-exit instrumentation

bool syscallNotification::installPreExit() {
    // Exit handled by event system.
    preExitInst = NULL;
#if 0
    AstNodePtr arg0 = AstNode::operandNode(AstNode::Param, (void *)0);
    preExitInst = new instMapping(EXIT_FUNC, "DYNINST_instExitEntry",
                                  FUNC_ENTRY|FUNC_ARG,
                                  arg0);
    preExitInst->dontUseTrampGuard();

    preExitInst->allow_trap = true;

    pdvector<instMapping *> instReqs;
    instReqs.push_back(preExitInst);
    
    proc->installInstrRequests(instReqs);
    // Check to see if we put anything in the proggie
    if (preExitInst->miniTramps.size() == 0)
        return false;
#endif
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

