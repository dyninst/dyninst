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

// $Id: sol_proc.h,v 1.3 2005/11/03 05:21:07 jaw Exp $


/*
 * COMPATIBILITY SECTION
 * 
 * Even though aix and solaris are almost entirely the same, there are differences.
 * This section defines macros to deal with this
 */

#if defined(sparc_sun_solaris2_4)
#define GETREG_nPC(regs)      (regs[R_nPC])
#define GETREG_PC(regs)       (regs[R_PC])
#define GETREG_FP(regs)       (regs[R_O6])
#define GETREG_INFO(regs)     (regs[R_O0])
#define GETREG_GPR(regs, reg) (regs[reg])
// Solaris uses the same operators on all set datatypes
#define prfillsysset(x)       prfillset(x)
#define premptysysset(x)      premptyset(x)
#define praddsysset(x,y)      praddset(x,y)
#define prdelsysset(x,y)      prdelset(x,y)
#define prissyssetmember(x,y) prismember(x,y)
#define proc_sigset_t         sigset_t
#define SYSSET_ALLOC(x)     ((sysset_t *)malloc(sizeof(sysset_t)))
#define SYSSET_FREE(x)      (free(x))
#define SYSSET_SIZE(x)      (sizeof(sysset_t))
#endif
#if defined(i386_unknown_solaris2_5)
#define REG_PC(regs) (regs->theIntRegs[EIP])
#define REG_FP(regs) (regs->theIntRegs[EBP])
#define REG_SP(regs) (regs->theIntRegs[UESP])
#endif
#if defined(AIX_PROC)
#define GETREG_nPC(regs)       (regs.__iar)
#define GETREG_PC(regs)        (regs.__iar)
#define GETREG_FP(regs)        (regs.__gpr[1])
#define GETREG_INFO(regs)      (regs.__gpr[3])
#define GETREG_GPR(regs,reg)   (regs.__gpr[reg])
#define PR_BPTADJ           0 // Not defined on AIX
#define PR_MSACCT           0 // Again, not defined
#define proc_sigset_t          pr_sigset_t
extern int SYSSET_SIZE(sysset_t *);
extern sysset_t *SYSSET_ALLOC(int);
#define SYSSET_FREE(x)      (free(x))
#endif


