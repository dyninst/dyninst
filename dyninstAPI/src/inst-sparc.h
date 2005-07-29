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

// $Id: inst-sparc.h,v 1.64 2005/07/29 19:18:39 bernat Exp $

#if !defined(sparc_sun_sunos4_1_3) && !defined(sparc_sun_solaris2_4)
#error "invalid architecture-os inclusion"
#endif

#ifndef INST_SPARC_H
#define INST_SPARC_H

#include "common/h/headers.h"
#ifndef BPATCH_LIBRARY
#include "rtinst/h/rtinst.h"
#endif
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/ast.h"
//#include "dyninstAPI/src/inst-sparc.h"
#include "dyninstAPI/src/arch-sparc.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/as-sparc.h"
#include "dyninstAPI/src/instP.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#define INSN_SIZE ( sizeof( instruction ) )

// some macros for helping code which contains register symbolic names
#define REG_I(x) (x + 24)
#define REG_L(x) (x + 16) 
#define REG_O(x) (x + 8)
#define REG_G(x) (x)


#define REG_MT_POS           REG_G(6)   /* Register which contains the current POS
					   value (for caching) */
#define NUM_INSN_MT_PREAMBLE 27   /* number of instructions required for   */
                                  /* the MT preamble.                      */ 

#define RECURSIVE_GUARD_ON_CODE_SIZE   7
#define RECURSIVE_GUARD_OFF_CODE_SIZE  3

// NOTE: LOW() and HIGH() can return ugly values if x is negative, because in
// that case, 2's complement has really changed the bitwise representation!
// Perhaps an assert should be done!
#define LOW10(x) ((x) & 0x3ff)
#define LOW13(x) ((x) & 0x1fff)
#define HIGH22(x) ((x) >> 10)

inline Address ABS(int x) {
   if (x < 0) return -x;
   return x;
}
//#define ABS(x)		((x) > 0 ? x : -x)

//#define MAX_BRANCH	(0x1<<23)

#define MAX_IMM13       (4095)
#define MIN_IMM13       (-4096)

#define REG_SPTR          14
#define REG_FPTR          30

extern registerSpace *regSpace;
extern Register deadList[];

bool processOptimaRet(instPoint *location, AstNode *&ast);

extern bool isPowerOf2(int value, int &result);
extern void generateNoOp(process *proc, Address addr);
extern void changeBranch(process *proc, Address fromAddr, Address newAddr,
		  instruction originalBranch);

extern void generateBranch(process *proc, Address fromAddr, Address newAddr);
extern void generateCall(process *proc, Address fromAddr, Address newAddr);
extern void generateBranchOrCallNoSaveRestore(process *proc,Address fromAddr, Address toAddr);
extern void genImm(process *proc, Address fromAddr, int op, Register rs1, 
		   int immd, Register rd);

extern int getInsnCost(opCode op);
extern bool isReturnInsn(const image *owner, Address adr, bool &lastOne, 
			 pdstring name); 
extern bool isReturnInsn(instruction i, Address adr, pdstring name);
extern bool isBranchInsn(instruction i);
extern bool branchInsideRange(instruction i, Address branchAddress, 
      Address firstAddress, Address lastAddress); 
extern bool trueCallInsideRange(instruction instr, Address callAddress, 
      Address firstAddress, Address lastAddress);

#endif
