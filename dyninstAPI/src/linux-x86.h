/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

// $Id: linux-x86.h,v 1.7 2007/12/14 04:16:48 jaw Exp $

#if !defined(i386_unknown_linux2_0) \
 && !defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ 
#error "invalid architecture-os inclusion"
#endif

#ifndef LINUX_X86_HDR
#define LINUX_X86_HDR

#include "inst-x86.h"
#include "codegen.h"
#include "sys/user.h"

/* For linuxDL.C */
instruction generateTrapInstruction();


#ifndef _SYS_USER_H
struct user_regs_struct
{
  long ebx;
  long ecx;
  long edx;
  long esi;
  long edi;
  long ebp;
  long eax;
  long xds;
  long xes;
  long xfs;
  long xgs;
  long orig_eax;
  long eip;
  long xcs;
  long eflags;
  long esp;
  long xss;
};
#endif

struct dyn_saved_regs
{
    user_regs_struct gprs;
#ifdef _SYS_USER_H
    user_fpregs_struct fprs;
#else
    user_i387_struct fprs;
#endif
};



#endif
