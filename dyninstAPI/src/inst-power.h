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


#ifndef INST_POWER_H
#define INST_POWER_H

/*
 * $Log: inst-power.h,v $
 * Revision 1.6  1997/04/14 00:21:53  newhall
 * removed class pdFunction and replaced it with base class function_base and
 * derived class pd_Function
 *
 * Revision 1.5  1997/02/21 20:13:27  naim
 * Moving files from paradynd to dyninstAPI + moving references to dataReqNode
 * out of the ast class. The is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.4  1997/02/18 21:24:09  sec
 * Added a field to instPoint, which identifies what the instrumentation
 * point is for (an enum saying ipFuncEntry, ipFuncReturn, ipFuncCallPoint).
 *
 * Revision 1.3  1997/01/27 19:40:48  naim
 * Part of the base instrumentation for supporting multithreaded applications
 * (vectors of counter/timers) implemented for all current platforms +
 * different bug fixes - naim
 *
 * Revision 1.2  1996/08/16 21:18:56  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.1  1995/08/24 15:03:58  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 *
 */

/*
 * inst-power.h - Common definitions to the POWER specific instrumentation code.
 *
 * inst-power.h,v
 */


#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/as-power.h"

/* "pseudo" instructions that are placed in the tramp code for the inst funcs
 *   to patch up.   This must be invalid instructions (any instruction with
 *   its top 10 bits as 0 is invalid (technically UNIMP).
 *
 */

extern registerSpace *regSpace;

extern trampTemplate baseTemplate;
extern trampTemplate noArgsTemplate;
extern trampTemplate withArgsTemplate;

#define REG_MT               12   /* register saved to keep the address of */
                                  /* the current vector of counter/timers  */
                                  /* for each thread.                      */
#define NUM_INSN_MT_PREAMBLE 21   /* number of instructions required for   */
                                  /* the MT preamble.                      */ 

enum ipFuncLoc { ipFuncEntry, ipFuncReturn, ipFuncCallPoint };

class instPoint {
public:
  instPoint(pd_Function *f, const instruction &instr, const image *owner,
	    const Address adr, const bool delayOK, const ipFuncLoc);

  ~instPoint() {  /* TODO */ }

  // can't set this in the constructor because call points can't be classified until
  // all functions have been seen -- this might be cleaned up
  void set_callee(pd_Function *to) { callee = to; }


  Address addr;                   /* address of inst point */
  instruction originalInstruction;    /* original instruction */

//  instruction delaySlotInsn;  /* original instruction */
//  instruction aggregateInsn;  /* aggregate insn */
//  bool inDelaySlot;            /* Is the instruction in a delay slot */
//  bool isDelayed;		/* is the instruction a delayed instruction */

  bool callIndirect;		/* is it a call whose target is rt computed ? */

//  bool callAggregate;		/* calling a func that returns an aggregate
//				   we need to reolcate three insns in this case
//				   */

  pd_Function *callee;		/* what function is called */
  pd_Function *func;		/* what function we are inst */
  
  ipFuncLoc ipLoc;
};


bool isCallInsn(const instruction);
bool isReturnInsn(const image *, Address);

#endif
