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

// $Id: syscall-nt.C,v 1.4 2006/03/14 22:57:31 legendre Exp $

#include "common/src/headers.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/syscallNotification.h"
#include "dyninstAPI/src/dynProcess.h"


syscallNotification::syscallNotification(syscallNotification *parentSN,
                                         PCProcess *p) : preForkInst(NULL),
                                                       postForkInst(NULL),
                                                       preExecInst(NULL),
                                                       postExecInst(NULL),
                                                       preExitInst(NULL),
                                                       proc(p) {
}

/////////// Prefork instrumentation 

bool syscallNotification::installPreFork() {
    preForkInst = SYSCALL_INSTALLED;
    return true;
}

/////////// Postfork instrumentation

bool syscallNotification::installPostFork() {
    return true;
}    

/////////// Pre-exec instrumentation

bool syscallNotification::installPreExec() {
    return true;
}    

//////////// Post-exec instrumentation

bool syscallNotification::installPostExec() {
    return true;
}    

/////////// Pre-exit instrumentation

bool syscallNotification::installPreExit() {
    return true;
}    

bool syscallNotification::installPreLwpExit() {
    return true;
}    

//////////////////////////////////////////////////////

/////// Remove pre-fork instrumentation

bool syscallNotification::removePreFork() {
    return true;
}

/////// Remove post-fork instrumentation

bool syscallNotification::removePostFork() {
    return true;
}

/////// Remove pre-exec instrumentation

bool syscallNotification::removePreExec() {
    return true;
}    

/////// Remove post-exec instrumentation

bool syscallNotification::removePostExec() {
    return true;
}

/////// Remove pre-exit instrumentation

bool syscallNotification::removePreExit() {
    return true;
}


bool syscallNotification::removePreLwpExit() {
    return true;
}    
