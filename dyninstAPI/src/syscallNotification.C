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

#include "common/src/headers.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/syscallNotification.h"
#include "dyninstAPI/src/dynProcess.h"
#include "dyninstAPI/src/ast.h"

#include "EventType.h"
#include "dyninstAPI/src/pcEventMuxer.h"
#include "patchAPI/h/PatchMgr.h"
#include "patchAPI/h/Point.h"

using namespace ProcControlAPI;
using namespace PatchAPI;


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
  if (!PCEventMuxer::useBreakpoint(EventType(EventType::Pre, EventType::Fork))) {
    return true;
  }

   preForkInst = new instMapping(getForkFuncName(),
                                 "DYNINST_instForkEntry",
                                 FUNC_ENTRY);
   preForkInst->dontUseTrampGuard();
   std::vector<instMapping *> instReqs;
   instReqs.push_back(preForkInst);
   
   proc->installInstrRequests(instReqs);
   proc->trapMapping.flush();

   return true;
}

/////////// Postfork instrumentation

bool syscallNotification::installPostFork() {
   if (!PCEventMuxer::useBreakpoint(EventType(EventType::Post, EventType::Fork))) return true;

   AstNodePtr returnVal = AstNode::operandNode(AstNode::operandType::ReturnVal, (void *)0);
   postForkInst = new instMapping(getForkFuncName(), "DYNINST_instForkExit",
                                  FUNC_EXIT|FUNC_ARG,
                                  returnVal);
   postForkInst->dontUseTrampGuard();
   postForkInst->canUseTrap(false);
   
   std::vector<instMapping *> instReqs;
   instReqs.push_back(postForkInst);
   
   proc->installInstrRequests(instReqs);
   proc->trapMapping.flush();

   return true;
}    

/////////// Pre-exec instrumentation

bool syscallNotification::installPreExec() {
   if (!PCEventMuxer::useBreakpoint(EventType(EventType::Pre, EventType::Exec))) return true;
   AstNodePtr arg0 = AstNode::operandNode(AstNode::operandType::Param, (void *)0);
   preExecInst = new instMapping(getExecFuncName(), "DYNINST_instExecEntry",
                                 FUNC_ENTRY|FUNC_ARG,
                                 arg0);
   preExecInst->dontUseTrampGuard();
   
   std::vector<instMapping *> instReqs;
   instReqs.push_back(preExecInst);
   
   proc->installInstrRequests(instReqs);
   proc->trapMapping.flush();

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
   if (!PCEventMuxer::useBreakpoint(EventType(EventType::Pre, EventType::Exit))) return true;
   AstNodePtr arg0 = AstNode::operandNode(AstNode::operandType::Param, (void *)0);
   preExitInst = new instMapping(getExitFuncName(), "DYNINST_instExitEntry",
                                 FUNC_ENTRY|FUNC_ARG,
                                 arg0);
   preExitInst->dontUseTrampGuard();
   
   preExitInst->allow_trap = true;
   
   std::vector<instMapping *> instReqs;
   instReqs.push_back(preExitInst);
   
   proc->installInstrRequests(instReqs);
   proc->trapMapping.flush();

   return true;
}    

bool syscallNotification::installPreLwpExit() {
   preLwpExitInst = NULL;
   return true;
}

//////////////////////////////////////////////////////

/////// Remove pre-fork instrumentation

bool syscallNotification::removePreFork() {
   if (!PCEventMuxer::useBreakpoint(EventType(EventType::Pre, EventType::Fork))) return true;

    if (!preForkInst) return false;

    if (!proc->isAttached() || proc->isExecing()) {
        delete preForkInst;
        preForkInst = NULL;
        return true;
    }
    
    
    InstancePtr handle;
    for (unsigned i = 0; i < preForkInst->instances.size(); i++) {
       handle = preForkInst->instances[i];
        
       bool removed = uninstrument(handle);
       // At some point we should handle a negative return... but I
       // have no idea how.
       assert(removed);
       // The instance is deleted when the instance is freed, so
       // we don't have to.
    }
    //proc->relocate();
    /* PatchAPI stuffs */
    AddressSpace::patch(proc);
    /* End of PatchAPI stuffs */


    delete preForkInst;
    preForkInst = NULL;
    return true;
}

    

/////// Remove post-fork instrumentation

bool syscallNotification::removePostFork() {
   if (!PCEventMuxer::useBreakpoint(EventType(EventType::Post, EventType::Fork))) return true;

    if (!postForkInst) return false;

    if (!proc->isAttached() || proc->isExecing()) {
        delete postForkInst;
        postForkInst = NULL;
        return true;
    }

    InstancePtr handle;
    for (unsigned i = 0; i < postForkInst->instances.size(); i++) {
       handle = postForkInst->instances[i];
        
       bool removed = uninstrument(handle);
       // At some point we should handle a negative return... but I
       // have no idea how.
       assert(removed);
       // The instance is deleted when the instance is freed, so
       // we don't have to.
    }
    //proc->relocate();
    /* PatchAPI stuffs */
    AddressSpace::patch(proc);
    /* End of PatchAPI stuffs */

    delete postForkInst;
    postForkInst = NULL;
    return true;
}

/////// Remove pre-exec instrumentation

bool syscallNotification::removePreExec() {
   if (!PCEventMuxer::useBreakpoint(EventType(EventType::Pre, EventType::Exec))) return true;

    if (!preExecInst) return false;

    if (!proc->isAttached() || proc->isExecing()) {
        delete preExecInst;
        preExecInst = NULL;
        return true;
    }

    InstancePtr handle;
    for (unsigned i = 0; i < preExecInst->instances.size(); i++) {
       handle = preExecInst->instances[i];
        
       bool removed = uninstrument(handle);
       // At some point we should handle a negative return... but I
       // have no idea how.
       assert(removed);
       // The instance is deleted when the instance is freed, so
       // we don't have to.
    }
    //proc->relocate();
    /* PatchAPI stuffs */
    AddressSpace::patch(proc);
    /* End of PatchAPI stuffs */

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
   if (!PCEventMuxer::useBreakpoint(EventType(EventType::Pre, EventType::Exit))) return true;

    if( !preExitInst ) return false;

    if (!proc->isAttached() || proc->isExecing()) {
        delete preExitInst;
        preExitInst = NULL;
        return true;
    }
    
    InstancePtr handle;
    for (unsigned i = 0; i < preExitInst->instances.size(); i++) {
       handle = preExitInst->instances[i];
        
       bool removed = uninstrument(handle);
       // At some point we should handle a negative return... but I
       // have no idea how.
       assert(removed);
       // The instance is deleted when the instance is freed, so
       // we don't have to.
    }
    //proc->relocate();
    /* PatchAPI stuffs */
    AddressSpace::patch(proc);
    /* End of PatchAPI stuffs */

    delete preExitInst;
    preExitInst = NULL;
    return true;
}

bool syscallNotification::removePreLwpExit() {
   return true;
}

//////////////////////////////////////////////

