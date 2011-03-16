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

// $Id: aix.h,v 1.24 2008/02/23 02:09:04 jaw Exp $

#if !defined(os_aix)
#error "invalid architecture-os inclusion"
#endif

#ifndef AIX_PD_HDR
#define AIX_PD_HDR

#if defined(cap_proc)
#include <sys/procfs.h>
#endif

#include <sys/param.h>
#include "unix.h"
#include "sol_proc.h"
#define EXIT_NAME "exit"

#define SIGNAL_HANDLER "no_signal_handler" 

/* How many bytes our insertion of "dlopen <libname>" needs */
/* Should be set dynamically */
/* Nice thing about AIX is that we don't have to overwrite main with
   the call, we just use the empty space after the program */
#define BYTES_TO_SAVE 256 // should be a multiple of instruction::size()

typedef int handleT; // defined for compatibility with other platforms
                     // not currently used on the AIX platform

#define num_special_registers 9
#if defined(cap_proc)
// Register storage structure
struct dyn_saved_regs
{
    prgregset_t theIntRegs;
    prfpregset_t theFpRegs;
};

// AIX /proc doesn't define some things that Solaris' does
#define PR_PCINVAL PR_STOPPED

#else
struct dyn_saved_regs
{
    int gprs[32];
    double fprs[32];
    int sprs[num_special_registers];
};
#endif

#define NUM_SYSCALLS 1024

typedef lwpstatus_t procProcStatus_t;
#endif
