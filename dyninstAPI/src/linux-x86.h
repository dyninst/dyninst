/*
 * Copyright (c) 1996 Barton P. Miller
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

// $Id: linux-x86.h,v 1.4 2004/03/16 18:20:47 schendel Exp $

#if !(defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4))
#error "invalid architecture-os inclusion"
#endif

#ifndef LINUX_X86_HDR
#define LINUX_X86_HDR

#include "inst-x86.h"
#include "arch-x86.h"
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
