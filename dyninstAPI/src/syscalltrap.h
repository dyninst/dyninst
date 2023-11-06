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

/* $Id: syscalltrap.h,v 1.5 2006/03/29 21:35:04 bernat Exp $
 */

#ifndef _SYSCALL_TRAP_H_
#define _SYSCALL_TRAP_H_


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

typedef bool (*syscallTrapCallbackLWP_t)(PCThread *thread, void *data);

#endif /*_SYSCALL_TRAP_H_*/
