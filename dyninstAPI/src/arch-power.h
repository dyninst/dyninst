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

// $Id: arch-power.h,v 1.21 2003/04/09 23:22:29 buck Exp $

#ifndef _ARCH_POWER_H
#define _ARCH_POWER_H

/*
 * Define power instruction information.
 *
 */

struct genericform {
  unsigned op : 6;
  unsigned XX : 26;
};

struct iform {            // unconditional branch + 
  unsigned op : 6;
  signed   li : 24;
  unsigned aa : 1;
  unsigned lk : 1;

};

struct bform {            // conditional branch +
  unsigned op : 6;
  unsigned bo : 5;
  unsigned bi : 5;
  signed   bd : 14;
  unsigned aa : 1;
  unsigned lk : 1;
};

struct dform {
    unsigned op : 6;
    unsigned rt : 5;        // rt, rs, frt, frs, to, bf_l
    unsigned ra : 5;
    signed   d_or_si : 16;  // d, si, ui
};

struct dsform {
    unsigned op : 6;
    unsigned rt : 5;        // rt, rs
    unsigned ra : 5;
    signed   d  : 14;
    unsigned xo : 2;
};

struct xform {
    unsigned op : 6;
    unsigned rt : 5;   // rt, frt, bf_l, rs, frs, to, bt
    unsigned ra : 5;   // ra, fra, bfa_, sr, spr
    unsigned rb : 5;   // rb, frb, sh, nb, u_
    unsigned xo : 10;  // xo, eo
    unsigned rc : 1;
};

struct xlform {
  unsigned op : 6;
  unsigned bt : 5;   // rt, bo, bf_
  unsigned ba : 5;   // ba, bi, bfa_
  unsigned bb : 5; 
  unsigned xo : 10;  // xo, eo
  unsigned lk : 1;
};

struct xfxform {
  unsigned op : 6;
  unsigned rt : 5;   // rs
  unsigned spr: 10;  // spr, tbr, fxm
  unsigned xo : 10;
  unsigned rc : 1;
};

struct xflform {
  unsigned op : 6;
  unsigned u1 : 1;
  unsigned flm: 8;
  unsigned u2 : 1;
  unsigned frb: 5;
  unsigned xo : 10;
  unsigned rc : 1;
};

struct xoform {
    unsigned op : 6;
    unsigned rt : 5;
    unsigned ra : 5;
    unsigned rb : 5;
    unsigned oe : 1;
    unsigned xo : 9; // xo, eo'
    unsigned rc : 1;
};

struct mform {
    unsigned op : 6;
    unsigned rs : 5;
    unsigned ra : 5;
    unsigned sh : 5;
    unsigned mb : 5; // mb, sh
    unsigned me : 5;
    unsigned rc : 1;
};

struct mdform {
    unsigned op : 6;
    unsigned rs : 5;
    unsigned ra : 5;
    unsigned sh : 5;
    unsigned mb_or_me : 5;
    unsigned mb_or_me2 : 1;
    unsigned xo : 3;
    unsigned sh2 : 1;
    unsigned rc : 1;
};

union instructUnion {
    struct iform  iform;  // branch;
    struct bform  bform;  // cbranch;
    struct dform  dform;
    struct dsform dsform;
    struct xform  xform;
    struct xoform xoform;
    struct xlform xlform;
    struct xfxform xfxform;
    struct xflform xflform;
    struct mform  mform;
    struct mdform  mdform;
    struct genericform generic;
    unsigned int  raw;
};

typedef union instructUnion instruction;

/*
 * Register saving constants
 */
#define maxFPR 13           /* Save FPRs 0-13 */
#define maxGPR 19           /* More space than is needed */
#define FPRspaceUsed (8*16) /* Aligned space for FPRs */
#define GPRoffset(reg) (-1* (FPRspaceUsed + 4*(maxGPR-reg)))
#define GPRspaceUsed (20*4) /* Aligned space for GPRs */

#define stackFrameSize (FPRspaceUsed+GPRspaceUsed+128)

/*
 * Define the operation codes
 */

// ------------- Op Codes, instruction form I  ------------------
#define Bop		18	/* (unconditional) branch */

// ------------- Op Codes, instruction form D  ------------------
#define CMPIop		11	/* compare immediate */
#define SIop            12      /* subtract immediate */
#define CALop		14	/* compute address lower -- ADDIop */
#define CAUop		15	/* compute address upper -- ADDISop*/
#define ORILop		24	/* (logical) or immediate lower -- ORIop*/
#define ANDILop         28      /* and immediate lower -- ANDIop*/
#define RLDop		30	/* RLD* family -- rotate left doubleword */
#define Lop		32	/* load (word) (aka lwz op in PowerPC) */
#define LUop		33
#define LBZop		34
#define LBZUop		35
#define STop		36	/* store (word) -- STWop */
#define STUop		37	/* store (word) with update -- STWUop */
#define STBop		38
#define STBUop		39
#define LHZop		40
#define LHZUop		41
#define LHAop		42
#define LHAUop		43
#define STHop		44
#define STHUop		45
#define LMop		46
#define STMop		47
#define LFSop		48
#define LFSUop		49
#define LFDop           50      /* load floating-point double */
#define LFDUop		51
#define STFSop		52
#define STFSUop		53
#define STFDop          54      /* store floating-point double */
#define STFDUop		55

// ------------- Op Codes, instruction form DS  ------------------
#define LDop		58	// LWA and LDU have the same op, xop differs
#define LDxop		0
#define LDUxop		1
#define LWAxop		2
#define STDop		62	// ditto
#define STDxop		0
#define STDUxop		1

// ------------- Op Codes, instruction form B  ------------------
#define BCop		16	/* branch conditional */

// ------------- Op Codes, instruction form X  ------------------
/* #define XFPop        31      -- extendened fixed point ops */
// -- X-from Loads
#define LXop		31	// VG: all X-from loads have op 31, only xop differs
#define LWARXxop	20
#define LDXxop		21
#define LXxop		23
#define LDUXxop		53
#define LUXxop		55
#define LDARXxop	84
#define LBZXxop		87
#define LBZUXxop	119
#define LHZXxop		279
#define LHZUXxop	311
#define MFSPRxop	339
#define LHAXxop		343
#define LWAXxop		341
#define LWAUXxop	373
#define LHAUXxop	375
#define LSXxop		533
#define LWBRXxop	534
#define LFSXxop		535
#define LFSUXxop	567	// I guess it must be so ;)
#define LSIxop		597
#define LFDXxop		599
#define LFDUXxop	631
#define LHBRXxop	790
// -- X-from stores
#define STXop		31	// VG: X-form stores, same story
#define STDXxop		149
#define STWCXxop	150
#define STXxop		151
#define STDUXxop	181
#define STUXxop		183
#define STDCXxop	214
#define STBXxop		215
#define STBUXxop	247
#define STHXxop		407
#define STHUXxop	439
#define MTSPRxop	467
#define STSXxop		661
#define STBRXxop	662
#define STFSXxop	663
#define STFSUXxop	695
#define STSIxop		725
#define STFDXxop	727
#define STFDUXxop	759
#define STHBRXxop	918
#define STFIWXxop	983
// -- other X-forms
#define CMPop           31      /* compare -- XFPop*/
#define CMPxop		0       /* compare */
#define ANDop           31      /* and */
#define ANDxop          28      /* and */
#define ORop            31      /* or */
#define ORxop           444     /* or */

// ------------- Op Codes, instruction form XL  -----------------
#define BCLRop		19	/* branch conditional link register */
#define BCLRxop		16	/* branch conditional link register */
#define BCCTRxop        528     /* branch conditional count register */

// ------------- Op Codes, instruction form XO  -----------------
/* #define XFPop        31      -- extendened fixed point ops */
#define SFop		31      /* subtract from -- XFPop */
#define SFxop		8       /* subtract from -- SUBFxop */
#define MULSop          31      /* multiply short -- XFPop */
#define MULSxop         235     /* multiply short -- MULLWxop */
#define CAXop		31      /* compute address -- XFPop */
#define CAXxop		266     /* compute address -- ADDxop */
#define DIVSop          31      /* divide short -- XFPop */
#define DIVSxop         363     /* divide short -- replacing DIVWxop */

// ------------- Op Codes, instruction form SC  -----------------
#define SVCop		17	/* supervisor call -- used to be SCop */

// ------------- Op Codes, instruction form M  -----------------
#define RLINMxop        21      /* rotate left immediate then AND with mask
                                 * -- RLWINMxop */

// -------------------------- Raw instructions ------------------
/* a few full instructions that are in forms we don't break down by field */
// xlform
#define MTLR0raw       0x7c0803a6	/* move from link reg -- mtlw r0 */
#define MFLR0raw       0x7c0802a6	/* move from link reg -- mflr r0 */
#define BCTRraw        0x4e800420      /* bctr instrunction */
#define BRraw          0x4e800020      /* br instruction */
#define BRLraw         0x4e800021	/* branch and link to link reg */
#define NOOPraw        0x60000000       /* noop, d form ORIL 0, 0, 0 */

// -------------------------- Branch fields ------------------------------
// BO field of branch conditional
#define Bcondmask		0x1e
#define BALWAYSmask		0x14

#define BPREDICTbit		1	// Set means reverse branch prediction
#define BifCTRzerobit		2	// Set means branch if CTR = 0
#define BnoDecCTRbit		4	// Set means decrement CTR and branch
#define BifTRUEbit		8	// Set means branch if condition true
#define BnoCondbit		16	// Set means ignore condition (use CTR)

#define BFALSEcond              4
#define BTRUEcond               12
#define BALWAYScond		20

/* BI field for branch conditional (really bits of the CR Field to use) */
#define LTcond			0		/* negative */
#define GTcond			1		/* positive */
#define EQcond			2		/* zero */
#define SOcond			3		/* summary overflow */

// -------------------------- Load/Store fields ---------------------------
#define RTmask		0x03e00000		// bits  6-10
#define RAmask		0x001f0000		// bits 11-15
#define RBmask		0x0000f800		// bits 16-20
#define DinDmask	0x0000ffff		// bits 16-31
#define DinDSmask	0x0000fffc		// bits 16-29

#define getRT(x) (((x) & RTmask) >> 21)
#define getRA(x) (((x) & RAmask) >> 16)
#define getRB(x) (((x) & RBmask) >> 11)
#define getDinD(x) ((x) & DinDmask)
#define getDinDS(x) ((x) & DinDSmask)

// -------------------------------------------------------------------

/* mask bits for various parts of the more common instruction format */
#define OPmask		0xfc000000              /* most sig. 6 bits */
#define AAmask		0x00000002		/* absolutate address */
#define LLmask		0x00000001		/* set linkage register */
#define AALKmask	AAmask|LLmask		/* AA | LK bits */
#define FULLmask	0xffffffff

#define Bmask		OPmask | AAmask
#define Bmatch		0x48000000 /* pc relative unconditional branch */
#define BCmatch		0x40000000 /* pc relative conditional branch */


#define BREAK_POINT_INSN 0x7d821008  /* trap */
// #define BREAK_POINT_INSN 0x7fe00008  -- this form should also work and
// follows the recommended form outlined in the AIX manual

/* high and low half words.  Useful to load addresses as two parts */
#define LOW(x)  ((x)%65536)
#define HIGH(x) ((x)/65536)


inline bool isInsnType(const instruction i,
		       const unsigned mask,
		       const unsigned match)
{
  return ((i.raw & mask) == match);
}

/* -- CHECK !!!!!
 * catch small ints that are invalid instructions
 * opcode 0 is really reserved, not illegal (with the exception of all 0's).
 *
 * opcodes 1, 4-6, 56-57, 60-61 are illegal.
 *
 * opcodes 2, 30, 58, 62 are illegal in 32 bit implementations, but are
 *    defined in 64 bit implementations.
 *
 * opcodes 19, 30, 31, 59, 62, 63 contain extended op codes that are unused.
 */
inline bool IS_VALID_INSN(instruction insn)
{
  return(insn.iform.op > 0);
}

// addresses on power are aligned to word boundaries
inline bool isAligned(const Address addr)
{
  return(!(addr & 0x3));
}

// Delcared some other functions in inst-power.C
// bool isCallInsn(const instruction);
// bool isReturnInsn(const image *, Address, bool&);

// Define bounds for immediate offsets
#define MIN_IMM16	-32768
#define MAX_IMM16	32767

#endif
