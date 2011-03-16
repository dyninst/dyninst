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

#if !defined(sparc_sun_solaris2_4) && !defined(i386_unknown_solaris2_5)
#error "invalid architecture-os inclusion"
#endif
#ifndef SOLARIS_PD_HDR
#define SOLARIS_PD_HDR

#include <procfs.h>
#include <sys/param.h>

#include "dyninstAPI/src/sol_proc.h"
#include "dyninstAPI/src/unix.h"

#define EXIT_NAME "_exithandle"

#define BYTES_TO_SAVE 256 // it should be a multiple of instruction::size()

#define SIGNAL_HANDLER	 "sigacthandler"

typedef int handleT; // a /proc file descriptor

struct dyn_saved_regs
{
    prgregset_t theIntRegs;
    prfpregset_t theFpRegs;
};

typedef lwpstatus_t procProcStatus_t;

#if defined (cap_save_the_world)
#include "writeBackElf.h" //ccw 28 oct 2001
#include "addLibrary.h" //ccw 3 dec 2001
#endif
#endif
