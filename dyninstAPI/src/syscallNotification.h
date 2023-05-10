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

// $Id: syscallNotification.h,v 1.3 2005/09/09 18:07:12 legendre Exp $

#if !defined(SYSCALL_NOTIFICATION_H)
#define SYSCALL_NOTIFICATION_H

// Non-NULL for platforms where we don't need the instMapping
#define SYSCALL_INSTALLED ((instMapping *)1)

#include <cassert>
#include <cstdlib>

class instMapping;
class PCProcess;

class syscallNotification {
  private:
    // If we use instrumentation to get notification of a syscall
    instMapping *preForkInst;
    instMapping *postForkInst;
    instMapping *preExecInst;
    instMapping *postExecInst;
    instMapping *preExitInst;
    instMapping *preLwpExitInst;
    PCProcess *proc;

    // platform dependent
    const char *getForkFuncName();
    const char *getExecFuncName();
    const char *getExitFuncName();
    
  public:
    syscallNotification() :
    preForkInst(NULL), postForkInst(NULL),
    preExecInst(NULL), postExecInst(NULL),
    preExitInst(NULL), preLwpExitInst(NULL),
    proc(NULL)
       { assert(0 && "ILLEGAL USE OF DEFAULT CONSTRUCTOR"); }

    syscallNotification(PCProcess *p) :
    preForkInst(NULL), postForkInst(NULL),
    preExecInst(NULL), postExecInst(NULL),
    preExitInst(NULL), preLwpExitInst(NULL), proc(p) {}

    // fork constructor
    syscallNotification(syscallNotification *parentSN,
                        PCProcess *p);
    
    ~syscallNotification() {
    }
    
    bool installPreFork();
    bool installPostFork();
    bool installPreExec();
    bool installPostExec();
    bool installPreExit();
    bool installPreLwpExit();

    bool removePreFork();
    bool removePostFork();
    bool removePreExec();
    bool removePostExec();
    bool removePreExit();
    bool removePreLwpExit();
};

#endif

