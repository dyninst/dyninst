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
 * $Id: inst-power.h,v 1.13 2000/11/15 22:56:07 bernat Exp $
 */

#ifndef INST_POWER_H
#define INST_POWER_H


#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/as-power.h"

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

#define REG_SP		      1		
#define REG_TOC               2   /* TOC anchor                            */
#define REG_MT               12   /* register saved to keep the address of */
                                  /* the current vector of counter/timers  */
                                  /* for each thread.                      */
#define NUM_INSN_MT_PREAMBLE 21   /* number of instructions required for   */
                                  /* the MT preamble.                      */ 

#ifdef BPATCH_LIBRARY
enum ipFuncLoc { ipFuncEntry, ipFuncReturn, ipFuncCallPoint, ipOther };
#else
enum ipFuncLoc { ipFuncEntry, ipFuncReturn, ipFuncCallPoint };
#endif


bool isCallInsn(const instruction);
bool isReturnInsn(const image *, Address);

#endif
