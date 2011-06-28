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

// $Id: sol_proc.h,v 1.6 2006/05/10 02:31:02 jaw Exp $


#ifndef _SOL_PROC_H_
#define _SOL_PROC_H_

#define INDEPENDENT_LWP_CONTROL false

/*
 * COMPATIBILITY SECTION
 * 
 * Even though aix and solaris are almost entirely the same, there are differences.
 * This section defines macros to deal with this
 */

#if defined(os_aix)
#define GETREG_nPC(regs)       (regs.__iar)
#define GETREG_PC(regs)        (regs.__iar)
#define GETREG_FP(regs)        (regs.__gpr[1])
#define GETREG_INFO(regs)      (regs.__gpr[3])
#define GETREG_GPR(regs,reg)   (regs.__gpr[reg])
#define PR_BPTADJ           0 // Not defined on AIX
#define PR_MSACCT           0 // Again, not defined
#define proc_sigset_t          pr_sigset_t
#define SYSSET_DECLAREPID(x,y) int x=y
extern unsigned int SYSSET_SIZE(sysset_t *);
extern sysset_t *SYSSET_ALLOC(int);
#define SYSSET_FREE(x)      (free(x))
#endif

#endif


