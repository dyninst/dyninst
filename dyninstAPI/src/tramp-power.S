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

/* $Id: tramp-power.S,v 1.17 2001/11/06 19:20:21 bernat Exp $ */

/*
 * trampoline code to get from a code location to an inst. primitive.
 *
 *    This code starts life in the controller process and moves into the
 *    appropriate inferior process via ptrace calls.
 */
#include "as-power.h"

#if defined(rs6000_ibm_aix4_1)
#define nop oril 0,0,0
#endif


/*
 * Structure for recursion-protected base tramp.
 *
 * - do global before local because global call DYNINSTinit.
 *
 */
.csect .data[RW]
	.globl baseTramp
	.globl _baseTramp
baseTramp:		
_baseTramp:	
	.long   UPDATE_LR
	nop
	nop
	nop
	nop
	.long   SKIP_PRE_INSN		/* if no instrumentation, then skip */
	.long	SAVE_PRE_INSN		/* save registers */    /* GPR10 */
	nop							/* GPR11 */
	nop                                                     /* GPR9  */
	nop                                                     /* GPR8  */
	nop                                                     /* GPR7  */
	nop                                                     /* GPR6  */
	nop                                                     /* GPR5  */
	nop                                                     /* GPR4  */
	nop                                                     /* GPR3  */
	nop                             /* mfspr  1. Copy LR into a GPR  */
	nop                             /* st     2. Copy GPR onto stack */
	nop				/* mfspr  1. Copy CTR into a GPR */
	nop				/* st     2. Copy GPR onto stack */
	nop                                                     /* FPR0  */
	nop                                                     /* FPR1  */
	nop                                                     /* FPR2  */
	nop                                                     /* FPR3  */
	nop                                                     /* FPR4  */
	nop                                                     /* FPR5  */
	nop                                                     /* FPR6  */
	nop                                                     /* FPR7  */
	nop                                                     /* FPR8  */
	nop                                                     /* FPR9  */
	nop                                                     /* FPR10 */
	nop                                                     /* FPR11 */
	nop                                                     /* FPR12 */
	nop                                                     /* FPR13 */
/* Attempt 1 to avoid reentering dyninst instructions from
	within a dyninst instruction */
	.long REENTRANT_GUARD_ADDR	/* cau 6,0,HIGH(baseAddr) */
	nop				/* oril 6,6,LOW(baseAddr) */
	.long REENTRANT_GUARD_LOAD	/* l 5,0,(6)              */
	.long REENTRANT_PRE_INSN_JUMP	/* cmpi 0,0,5,0           */
	nop				/* bc 12,2,<offset>R_PRE_INSN */
	.long REENTRANT_GUARD_INC	/* cal 5,1(0)             */
	.long REENTRANT_GUARD_STORE	/* stw 5,0(6)             */
	.long	GLOBAL_PRE_BRANCH	/* cau   */
	nop				/* oril  */
	nop				/* mtspr */
	nop				/* bcrl  */
        nop				/* fill this in with instructions to */
	nop				/* compute the address of the vector */
	nop				/* of counter/timers for each thread */
	nop
	nop
	nop
	nop
	nop
        nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
/* Multi-thread nops (NUM_INSN_MT_PREAMBLE insns) */
#ifdef MT_THREAD
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop	
#endif
	.long	LOCAL_PRE_BRANCH	/* cau   */
	nop				/* oril  */
	nop				/* mtspr */
	nop				/* bcrl  */
	.long   UPDATE_COST_INSN
	nop
	nop
	nop
	nop
	nop
	/* Decrease the counter protector */
	.long REENTRANT_GUARD_ADDR      /* cau 6,0,HIGH(baseAddr) */
	nop				/* oril 6,6,LOW(baseAddr) */
	.long REENTRANT_GUARD_DEC	/* cal 5,0(0)             */
	.long REENTRANT_GUARD_STORE	/* stw 5,0(6)             */
	.long	RESTORE_PRE_INSN	/* restore registers *//* l     LR   */
	nop                                                    /* mtspr ""   */
	nop				                       /* l CTR      */
	nop				                       /* mfspr ""   */
	nop							     /* GRP11*/
	nop                                                          /* GPR10*/
	nop                                                          /* GPR9 */
	nop                                                          /* GPR8 */
	nop                                                          /* GPR7 */
	nop                                                          /* GPR6 */
	nop                                                          /* GPR5 */
	nop                                                          /* GPR4 */
	nop                                                          /* GPR3 */
	nop                                                          /* FPR0 */
	nop                                                          /* FPR1 */
	nop                                                          /* FPR2 */
	nop                                                          /* FPR3 */
	nop                                                          /* FPR4 */
	nop                                                          /* FPR5 */
	nop                                                          /* FPR6 */
	nop                                                          /* FPR7 */
	nop                                                          /* FPR8 */
	nop                                                          /* FPR9 */
	nop                                                          /* FPR10*/
	nop                                                          /* FPR11*/
	nop                                                          /* FPR12*/
	nop                                                          /* FPR13*/
	.long 	EMULATE_INSN
	nop
        .long	SKIP_POST_INSN		/* if no instrumentation, then skip */
	.long	SAVE_POST_INSN		/* save registers */    /* GPR10 */
	nop	
	nop                                                     /* GPR9  */
	nop                                                     /* GPR8  */
	nop                                                     /* GPR7  */
	nop                                                     /* GPR6  */
	nop                                                     /* GPR5  */
	nop                                                     /* GPR4  */
	nop                                                     /* GPR3  */
	nop                             /* mfspr  1. Copy LR into a GPR  */
	nop                             /* st     2. Copy GRP onto stack */
	nop				/* mfspr  1. Copy CTR into a GPR */
	nop				/* st     2. Copy GPR onto stack */
	nop                                                     /* FPR0  */
	nop                                                     /* FPR1  */
	nop                                                     /* FPR2  */
	nop                                                     /* FPR3  */
	nop                                                     /* FPR4  */
	nop                                                     /* FPR5  */
	nop                                                     /* FPR6  */
	nop                                                     /* FPR7  */
	nop                                                     /* FPR8  */
	nop                                                     /* FPR9  */
	nop                                                     /* FPR10 */
	nop                                                     /* FPR11 */
	nop                                                     /* FPR12 */
	nop                                                     /* FPR13 */
	.long REENTRANT_GUARD_ADDR	/* cau 6,0,HIGH(baseAddr) */
	nop				/* oril 6,6,LOW(baseAddr) */
        .long REENTRANT_GUARD_LOAD	/* l 5,0,(6)              */
	.long REENTRANT_POST_INSN_JUMP	/* cmpi 0,0,5,0           */
	nop				/* bc 12,2,<offset>R_PRE_INSN */
	.long REENTRANT_GUARD_INC	/* cal 5,1(0)             */
	.long REENTRANT_GUARD_STORE	/* stw 5,0(6)             */
	.long	GLOBAL_POST_BRANCH	/* cau   */
	nop				/* oril  */
	nop				/* mtspr */
	nop				/* bcrl  */
        nop				/* fill this in with instructions to */
	nop				/* compute the address of the vector */
	nop				/* of counter/timers for each thread */
	nop
	nop
	nop
	nop
	nop
        nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
/* Multi-thread nops (NUM_INSN_MT_PREAMBLE insns) */
#ifdef MT_THREAD
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop	
#endif
	.long	LOCAL_POST_BRANCH	/* cau   */
	nop				/* oril  */
	nop				/* mtspr */
	nop				/* bcrl  */
	/* Decrease the counter protector */
	.long REENTRANT_GUARD_ADDR      /* cau 6,0,HIGH(baseAddr) */
	nop				/* oril 6,6,LOW(baseAddr) */
	.long REENTRANT_GUARD_DEC	/* cal 5,0(0)             */
	.long REENTRANT_GUARD_STORE	/* stw 5,0(6)             */
        .long RESTORE_POST_INSN         /* restore registers *//* l     LR   */
	nop                                                    /* mtspr ""   */
	nop				                       /* l CTR      */
	nop				                       /* mfspr ""   */
	nop
	nop                                                          /* GPR10*/
	nop                                                          /* GPR9 */
	nop                                                          /* GPR8 */
	nop                                                          /* GPR7 */
	nop                                                          /* GPR6 */
	nop                                                          /* GPR5 */
	nop                                                          /* GPR4 */
	nop                                                          /* GPR3 */
	nop                                                          /* FPR0 */
	nop                                                          /* FPR1 */
	nop                                                          /* FPR2 */
	nop                                                          /* FPR3 */
	nop                                                          /* FPR4 */
	nop                                                          /* FPR5 */
	nop                                                          /* FPR6 */
	nop                                                          /* FPR7 */
	nop                                                          /* FPR8 */
	nop                                                          /* FPR9 */
	nop                                                          /* FPR10*/
        nop                                                          /* FPR11*/
	nop                                                          /* FPR12*/
	nop                                                          /* FPR13*/
	.long	RETURN_INSN
	nop
	.long	END_TRAMP

/* #ifdef BPATCH_LIBRARY */
/*
 * This is the conservative version of the base tramp.  It save more registers
 * that the version above (which can get away with saving fewer because it is
 * only used at function call boundaries, where many registers are considered
 * dead).
 *
 */
.csect .data[RW]
	.globl conservativeTramp
	.globl _conservativeTramp
conservativeTramp:
_conservativeTramp:
	/* should update cost of base tramp here, but we don't have a
	   register to use!
         */
	.long   SKIP_PRE_INSN		/* if no instrumentation, then skip */
	.long	SAVE_PRE_INSN		/* save registers */    /* GPR12 */
	nop                                                     /* GPR11 */
	nop                                                     /* GPR10 */
	nop                                                     /* GPR9  */
	nop                                                     /* GPR8  */
	nop                                                     /* GPR7  */
	nop                                                     /* GPR6  */
	nop                                                     /* GPR5  */
	nop                                                     /* GPR4  */
	nop                                                     /* GPR3  */
	nop                                                     /* GPR0  */
	nop                             /* mfspr  1. Copy LR into a GPR  */
	nop                             /* st     2. Copy GPR onto stack */
	nop                             /* mfcr   1. Copy CR into a GPR  */
	nop                             /* st     2. Copy GPR onto stack */
	nop                             /* mfcr   1. Copy XER into a GPR  */
	nop                             /* st     2. Copy GPR onto stack */
	nop                             /* mfcr   1. Copy CTR into a GPR  */
	nop                             /* st     2. Copy GPR onto stack */
	nop                             /* mfcr   1. Copy SPR0 into a GPR  */
	nop                             /* st     2. Copy GPR onto stack */
	nop                                                     /* FPR0  */
	nop                                                     /* FPR1  */
	nop                                                     /* FPR2  */
	nop                                                     /* FPR3  */
	nop                                                     /* FPR4  */
	nop                                                     /* FPR5  */
	nop                                                     /* FPR6  */
	nop                                                     /* FPR7  */
	nop                                                     /* FPR8  */
	nop                                                     /* FPR9  */
	nop                                                     /* FPR10 */
	nop                                                     /* FPR11 */
	nop                                                     /* FPR12 */
	nop                                                     /* FPR13 */
	nop                             /* mffs   1. Copy FPSCR into an FPR */
	nop                             /* st     2. Copy FPR onto stack */
	.long	GLOBAL_PRE_BRANCH	/* cau   */
	nop				/* oril  */
	nop				/* mtspr */
	nop				/* bcrl  */
        nop				/* fill this in with instructions to */
	nop				/* compute the address of the vector */
	nop				/* of counter/timers for each thread */
	nop
	nop
	nop
	nop
	nop
        nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
/* Multi-thread nops (NUM_INSN_MT_PREAMBLE insns) */
#ifdef MT_THREAD
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop	
#endif
	.long	LOCAL_PRE_BRANCH	/* cau   */
	nop				/* oril  */
	nop				/* mtspr */
	nop				/* bcrl  */
	.long   UPDATE_COST_INSN
	nop
	nop
	nop
	nop
	nop
	.long	RESTORE_PRE_INSN	/* restore registers *//* l     LR   */
	nop                                                    /* mtspr ""   */
	nop						       /* l     CR   */
	nop                                                    /* mtcrf ""   */
	nop						       /* l     XER  */
	nop                                                    /* mtspr ""   */
	nop						       /* l     CTR  */
	nop                                                    /* mtspr ""   */
	nop						       /* l     SPR0 */
	nop                                                    /* mtspr ""   */
	nop						       /* l     FPSCR*/
	nop                                                    /* mtspr ""   */
	nop                                                          /* GPR12*/
	nop                                                          /* GPR11*/
	nop                                                          /* GPR10*/
	nop                                                          /* GPR9 */
	nop                                                          /* GPR8 */
	nop                                                          /* GPR7 */
	nop                                                          /* GPR6 */
	nop                                                          /* GPR5 */
	nop                                                          /* GPR4 */
	nop                                                          /* GPR3 */
	nop                                                          /* GPR0 */
	nop                                                          /* FPR0 */
	nop                                                          /* FPR1 */
	nop                                                          /* FPR2 */
	nop                                                          /* FPR3 */
	nop                                                          /* FPR4 */
	nop                                                          /* FPR5 */
	nop                                                          /* FPR6 */
	nop                                                          /* FPR7 */
	nop                                                          /* FPR8 */
	nop                                                          /* FPR9 */
	nop                                                          /* FPR10*/
	nop                                                          /* FPR11*/
	nop                                                          /* FPR12*/
	nop                                                          /* FPR13*/
	.long 	EMULATE_INSN
	nop
	nop
	nop
	nop
        .long	SKIP_POST_INSN		/* if no instrumentation, then skip */
	.long	SAVE_POST_INSN		/* save registers */    /* GPR12 */
	nop                                                     /* GPR11 */
	nop                                                     /* GPR10 */
	nop                                                     /* GPR9  */
	nop                                                     /* GPR8  */
	nop                                                     /* GPR7  */
	nop                                                     /* GPR6  */
	nop                                                     /* GPR5  */
	nop                                                     /* GPR4  */
	nop                                                     /* GPR3  */
	nop                                                     /* GPR0  */
	nop                             /* mfspr  1. Copy LR into a GPR  */
	nop                             /* st     2. Copy GPR onto stack */
	nop                             /* mfcr   1. Copy CR into a GPR  */
	nop                             /* st     2. Copy GPR onto stack */
	nop                             /* mfcr   1. Copy XER into a GPR  */
	nop                             /* st     2. Copy GPR onto stack */
	nop                             /* mfcr   1. Copy CTR into a GPR  */
	nop                             /* st     2. Copy GPR onto stack */
	nop                             /* mfcr   1. Copy SPR0 into a GPR  */
	nop                             /* st     2. Copy GPR onto stack */
	nop                                                     /* FPR0  */
	nop                                                     /* FPR1  */
	nop                                                     /* FPR2  */
	nop                                                     /* FPR3  */
	nop                                                     /* FPR4  */
	nop                                                     /* FPR5  */
	nop                                                     /* FPR6  */
	nop                                                     /* FPR7  */
	nop                                                     /* FPR8  */
	nop                                                     /* FPR9  */
	nop                                                     /* FPR10 */
	nop                                                     /* FPR11 */
	nop                                                     /* FPR12 */
	nop                                                     /* FPR13 */
	nop                             /* mffs   1. Copy FPSCR into an FPR */
	nop                             /* st     2. Copy FPR onto stack */
	.long	GLOBAL_POST_BRANCH	/* cau   */
	nop				/* oril  */
	nop				/* mtspr */
	nop				/* bcrl  */
        nop				/* fill this in with instructions to */
	nop				/* compute the address of the vector */
	nop				/* of counter/timers for each thread */
	nop
	nop
	nop
	nop
	nop
        nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
/* Multi-thread nops (NUM_INSN_MT_PREAMBLE insns) */
#ifdef MT_THREAD
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop	
#endif
	.long	LOCAL_POST_BRANCH	/* cau   */
	nop				/* oril  */
	nop				/* mtspr */
	nop				/* bcrl  */
        .long   RESTORE_POST_INSN       /* restore registers *//* l     LR   */
	nop                                                    /* mtspr ""   */
	nop						       /* l     CR   */
	nop                                                    /* mtcrf ""   */
	nop						       /* l     XER  */
	nop                                                    /* mtspr ""   */
	nop						       /* l     CTR  */
	nop                                                    /* mtspr ""   */
	nop						       /* l     SPR0 */
	nop                                                    /* mtspr ""   */
	nop						       /* l     FPSCR*/
	nop                                                    /* mtfsf ""   */
	nop                                                          /* GPR12*/
	nop                                                          /* GPR11*/
	nop                                                          /* GPR10*/
	nop                                                          /* GPR9 */
	nop                                                          /* GPR8 */
	nop                                                          /* GPR7 */
	nop                                                          /* GPR6 */
	nop                                                          /* GPR5 */
	nop                                                          /* GPR4 */
	nop                                                          /* GPR3 */
	nop                                                          /* GPR0 */
	nop                                                          /* FPR0 */
	nop                                                          /* FPR1 */
	nop                                                          /* FPR2 */
	nop                                                          /* FPR3 */
	nop                                                          /* FPR4 */
	nop                                                          /* FPR5 */
	nop                                                          /* FPR6 */
	nop                                                          /* FPR7 */
	nop                                                          /* FPR8 */
	nop                                                          /* FPR9 */
	nop                                                          /* FPR10*/
        nop                                                          /* FPR11*/
	nop                                                          /* FPR12*/
	nop                                                          /* FPR13*/
	.long	RETURN_INSN
	nop
	.long	END_TRAMP
/* #endif BPATCH_LIBRARY */
