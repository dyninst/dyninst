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

#ifndef _ARCH_POWER_H
#define _ARCH_POWER_H

/*
 * $Log: arch-power.h,v $
 * Revision 1.5  1996/10/09 20:43:36  naim
 * Implementation of emitImm procedure - naim
 *
 * Revision 1.4  1996/09/05 16:14:44  lzheng
 * Move the defination of BREAK_POINT_INSN to the machine dependent file
 *
 * Revision 1.3  1996/08/16 21:18:05  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.2  1996/03/25 22:57:58  hollings
 * Support functions that have multiple exit points.
 *
 * Revision 1.1  1995/08/24  15:03:38  hollings
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
 * Define power instruction information.
 *
 */

/*
 * unconditional branch instruction.
 *
 */
struct iform {
    unsigned op:6;
    signed li:24;
    unsigned aa:1;
    unsigned lk:1;
};

/* conditional branch */
struct bform {
    unsigned op:6;
    unsigned bo:5;
    unsigned bi:5;
    signed bd:14;
    unsigned aa:1;
    unsigned lk:1;
};

struct dform {
    unsigned op:6;
    unsigned rt:5;
    unsigned ra:5;
    unsigned d_or_si:16;
};

struct xform {
    unsigned op:6;
    unsigned rt:5;
    unsigned ra:5;
    unsigned rb:5;
    unsigned xo:10;
    unsigned rc:1;
};

struct mform {
    unsigned op:6;
    unsigned rs:5;
    unsigned ra:5;
    unsigned sh:5;
    unsigned mb:5;
    unsigned me:5;
    unsigned rc:1;
};

struct xoform {
    unsigned op:6;
    unsigned rt:5;
    unsigned ra:5;
    unsigned rb:5;
    unsigned oe:1;
    unsigned xo:9;
    unsigned rc:1;
};

union instructUnion {
    struct iform branch;
    struct bform cbranch;
    struct dform dform;
    struct xform xform;
    struct xoform xoform;
    struct mform mform;
    unsigned int raw;
};

typedef union instructUnion instruction;


/*
 * Define the operation codes
 *
 */
#define CMPIop		11	/* compare imm op */
#define ADDIop		14	/* add immediate op */
#define ADDISop		15	/* add immediate shifted op */
#define BCop		16	/* branch conditional */
#define SCop		17	/* system call */
#define Bop		18	/* unconditional branch */
#define BCLRop		19	/* branch conditional to the link reg */
#define ORIop		24	/* logical or operation */
#define XFPop		31	/* extendened fixed point ops (XO form) */
#define Lop		32	/* load word op (aka lwz op in PowerPC) */
#define STWop		36	/* store word op */
#define STWUop		37	/* store word and update op */

/* a few full instructions that are in forms we don't break down by field */
#define MTLR0		0x7c0803a6	/* move from link reg -- mtlw r0 */
#define MFLR0		0x7c0802a6	/* move from link reg -- mflr r0 */
#define BRL		0x4e800021	/* branch and link to link reg */

/* XO fields of XFPop */
#define CMPxop		0
#define SUBFxop		8
#define MULLWxop	235
#define ADDxop		266
#define DIVWxop		491

/* Shift operations */
#define RLWINMxop       21

/* Immediate operation codes */
#define SIop            12
#define ANDIop          28

/* Logical operation codes */
#define ANDop           31
#define ANDxop          28
#define ORop            31
#define ORxop           444

/* BO field of branch conditional */
/* This assumes we want to use the default branch prediction.  To override
 * the default branch prediction add a lower order 1 bit to this field. 
 */
#define BFALSEcond		12
#define BTRUEcond		4

/* BI field for branch conditional (really bits of the CR to use) */
/* we assume that we will always use the low bits of the CR (as set by the
 *    compare instruction.
 */
#define LTcond			0		/* negative */
#define GTcond			1		/* positive */
#define EQcond			2		/* zero */
#define SOcond			4		/* summary overflow */

/* mask bits for various parts of the instruction format */
#define OPmask		0xfc000000
#define AAmask		0x00000002		/* absolutate address */
#define LLmask		0x00000001		/* set linkage register */
#define AALKmask	AAmask|LLmask		/* AA | LK bits */

/* op = 01 -- mask for and match for call (branch and link) instruction */
#define	CALLop		Bop
#define RETop		BCLRop

/* instruction plus AA and LK fields */
#define CALLmask	OPmask|AALKmask
/* need the 3/4 of third nibble to distinguish from bctr instruction. */
#define RETmask		0xffffffff
#define Bmask		OPmask|AAmask

#define CALLmatch	0x48000001 /* pc rel unconditional branch and link */
#define Bmatch		0x48000000 /* pc relative unconditional branch */
#define BCmatch		0x46000000 /* pc relative conditional branch */
#define RETmatch	0x4e800020 /* br instruction */
#define BCTRmatch	0x4e800420 /* bctr instrunction */

#define BREAK_POINT_INSN 0x7d821008  /* brpt */

/* high and low half words.  Useful to load addresses as two parts */
#define LOW(x)  ((x)%65536)
#define HIGH(x) ((x)/65536)

inline bool isInsnType(const instruction i,
		       const unsigned mask,
		       const unsigned match) {
  return ((i.raw & mask) == match);
}

inline bool isCallInsn(const instruction i) {
  return (isInsnType(i, CALLmask, CALLmatch));
}

/* catch small ints that are invalid instructions */
/*
 * opcode 0 is really reserved, not illegal (with the exception of all 0's).
 *
 * opcodes 1, 4-6, 56-57, 60-61 are illegal.
 *
 * opcodes 2, 30, 58, 62 are illegal in 32 bit implementations, but are
 *    defined in 64 bit implementations.
 *
 * opcodes 19, 30, 31, 59, 62, 63 contain extended op codes that are unused.
 */
inline bool IS_VALID_INSN(const instruction insn) {
  return ((insn.branch.op) || ((insn.branch.op == 1) ||
			     (insn.branch.op == 4) ||
			     (insn.branch.op == 5) ||
			     (insn.branch.op == 6) ||
			     (insn.branch.op == 56) ||
			     (insn.branch.op == 57) ||
			     (insn.branch.op == 60) ||
			     (insn.branch.op == 61)));
}

#endif
