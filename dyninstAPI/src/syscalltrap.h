/*
 * Copyright (c) 1996-2002 Barton P. Miller
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

/* $Id: syscalltrap.h,v 1.2 2003/04/16 21:07:10 bernat Exp $
 */

#ifndef _SYSCALL_TRAP_H_
#define _SYSCALL_TRAP_H_

#include "common/h/Types.h"

class dyn_thread;
class dyn_lwp;

/*
 * This file provides prototypes for the data structures which track
 * traps inserted at the exit of system calls. These are primarily
 * used to signal when it is possible to modify the state of the program.
 *
 */

/*
 * This is the process-wide version: per system call how many are waiting,
 * etc.
 */
struct syscallTrap {
    // Reference count (for MT)
    unsigned refcount;
    // Syscall ID
    Address syscall_id;
    // /proc setting
    int orig_setting;
    // Address/trap tracking
    char saved_insn[32];
    // Handle for further info
    void *saved_data;
};

/*
 * Per thread or LWP: a callback to be made when the
 * system call exits
 */

typedef void (*syscallTrapCallbackLWP_t)(dyn_lwp *lwp, void *data);

#endif /*_SYSCALL_TRAP_H_*/
