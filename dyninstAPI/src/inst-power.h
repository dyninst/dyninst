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

/*
 * inst-power.h - Common definitions to the POWER specific instrumentation code.
 * $Id: inst-power.h,v 1.21 2003/09/29 20:47:59 bernat Exp $
 */

#ifndef INST_POWER_H
#define INST_POWER_H


#include "dyninstAPI/src/ast.h"

#ifdef BPATCH_LIBRARY
#include "BPatch_function.h"
#endif

/* "pseudo" instructions that are placed in the tramp code for the inst funcs
 *   to patch up.   This must be invalid instructions (any instruction with
 *   its top 10 bits as 0 is invalid (technically UNIMP).
 *
 */

extern registerSpace *regSpace;

extern trampTemplate baseTemplate;
#ifdef BPATCH_LIBRARY
extern trampTemplate conservativeTemplate;
#endif
extern trampTemplate noArgsTemplate;
extern trampTemplate withArgsTemplate;

#define GPRSIZE               4
#define FPRSIZE               8

#define REG_SP		      1		
#define REG_TOC               2   /* TOC anchor                            */
#define REG_GUARD_ADDR        5   /* Arbitrary                             */
#define REG_GUARD_VALUE       6
#define REG_GUARD_OFFSET      7
#define REG_MT_POS           12   /* Register to reserve for MT implementation */
#define NUM_INSN_MT_PREAMBLE 26   /* number of instructions required for   */
                                  /* the MT preamble.                      */ 

#define STACKSKIP 220
#define GPRSAVE   (14*4)
#define FPRSAVE   (14*8)
#define SPRSAVE   (6*4+8)
#define PDYNSAVE  (8)
#define FUNCSAVE  (14*4)
#define FUNCARGS  32
#define LINKAREA  24

// Okay, now that we have those defined, let's define the offsets upwards
#define TRAMP_FRAME_SIZE (STACKSKIP + GPRSAVE + FPRSAVE + SPRSAVE + PDYNSAVE + \
                          FUNCSAVE + FUNCARGS + LINKAREA)
#define PDYN_RESERVED (LINKAREA + FUNCARGS + FUNCSAVE)
#define TRAMP_SPR_OFFSET (PDYN_RESERVED + PDYNSAVE) /* 4 for LR */
#define STK_GUARD (PDYN_RESERVED)
#define STK_LR    (           0)
#define STK_CR    (STK_LR   + 4)
#define STK_CTR   (STK_CR   + 4)
#define STK_XER   (STK_CTR  + 4)
#define STK_SPR0  (STK_XER  + 4)
#define STK_FP_CR (STK_SPR0 + 4)

#define TRAMP_FPR_OFFSET (TRAMP_SPR_OFFSET + SPRSAVE)
#define TRAMP_GPR_OFFSET (TRAMP_FPR_OFFSET + FPRSAVE)

#define FUNC_CALL_SAVE (LINKAREA + FUNCARGS)

/* Cookie values for marking if we're in a tramp */
#define MODIFIED_LR 0x54a7
#define MODIFIED_LR_MASK 0xffff


#define IN_TRAMP 0xda73
#define IN_TRAMP_MASK 0xffff


/* ipOther is never used in Paradyn, but simplifies code to unify */
enum ipFuncLoc { ipFuncEntry, ipFuncReturn, ipFuncCallPoint, ipOther };


bool isCallInsn(const instruction);
bool isReturnInsn(const image *, Address);

#endif
