/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: linux-power.h,v 1.2 2007/07/02 16:45:50 ssuen Exp $

#if !defined(os_linux) || !defined(arch_power)
#error "invalid architecture-os inclusion"
#endif

#ifndef LINUX_POWER_HDR
#define LINUX_POWER_HDR

#include <asm/ptrace.h>

// fields within ptrace() pt_regs structure
#define PTRACE_REG_FP gpr[1] // frame pointer
#define PTRACE_REG_IP nip    // next instruction pointer

struct dyn_saved_regs
{
   struct pt_regs gprs;      // 32 general purpose registers plus most SPRs
   struct fp_regs {
     double        fpr[32];  // 32 floating point registers
     unsigned long fpscr;    // floating point status and control register
   }              fprs;
};

inline Address region_lo(const Address /* x */) {
   return 0x08000000;  // start of text
}

inline Address region_hi(const Address /* x */) {
   return (Address)0xc << (sizeof(Address) * 8 - 4);  // start of kernel
}

#endif
