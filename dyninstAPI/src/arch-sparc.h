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

#if !defined(sparc_sun_sunos4_1_3) && !defined(sparc_sun_solaris2_4) && !defined(sparc_tmc_cmost7_3)
#error "invalid architecture-os inclusion"
#endif

#ifndef _ARCH_SPARC_H
#define _ARCH_SPARC_H

/*
 * $Log: arch-sparc.h,v $
 * Revision 1.12  1996/09/26 18:58:23  newhall
 * added support for instrumenting dynamic executables on sparc-solaris
 * platform
 *
 * Revision 1.11  1996/09/05 16:17:00  lzheng
 * Move the defination of BREAK_POINT_INSN to the machine dependent file
 *
 * Revision 1.10  1996/08/20 19:10:02  lzheng
 * Implementation of moving multiple instructions sequence
 * Correcting a few opmask here and add some new.
 *
 * Revision 1.9  1996/08/16 21:18:07  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.8  1996/04/29 22:08:44  mjrg
 * added some extra defines
 *
 * Revision 1.7  1995/12/11 15:06:41  naim
 * Implementing >, >=, < and <= operators - naim
 *
 * Revision 1.6  1995/07/11  20:57:23  jcargill
 * Changed sparc-specific ifdefs to include sparc_tmc_cmost7_3
 *
 * Revision 1.5  1995/05/30  05:04:49  krisna
 * upgrade from solaris-2.3 to solaris-2.4.
 * architecture-os based include protection of header files.
 * removed architecture-os dependencies in generic sources.
 * changed ST_* symbol names to PDST_* (to avoid conflict on HPUX)
 *
 * Revision 1.4  1994/11/02  10:59:12  markc
 * Replaced some of the hash-defs with inlines
 *
 * Revision 1.3  1994/09/22  01:31:33  markc
 * Added log message, duplicate include guards
 *
 */


typedef enum { 
    noneType,
    functionEntry,
    functionExit,
    callSite
} instPointType;


/*
 * Define sparc instruction information.
 *
 */
struct fmt1 {
    unsigned op:2;
    signed disp30:30;
};

struct fmt2 {
    unsigned op:2;
    unsigned anneal:1;
    unsigned cond:4;
    unsigned op2:3;
    signed disp22:22;
};

struct fmt2a {
    unsigned op:2;
    unsigned rd:5;
    unsigned op2:3;
    signed imm22:22;
};

struct fmt3 {
    unsigned op:2;
    unsigned rd:5;
    unsigned op3:6;
    unsigned rs1:5;
    unsigned i:1;
    unsigned unused:8;
    unsigned rs2:5;
};

struct fmt3i {
    unsigned op:2;
    unsigned rd:5;
    unsigned op3:6;
    unsigned rs1:5;
    unsigned i:1;
    signed simm13:13;
};

union instructUnion {
    struct fmt1	call;
    struct fmt2	branch;
    struct fmt2a sethi;
    struct fmt3 rest;
    struct fmt3i resti;
    unsigned int raw;
};

typedef union instructUnion instruction;


/*
 * Define the operation codes
 *
 */
#define SetCC		16
#define ADDop3		0
#define ANDop3		1
#define ORop3		2
#define SUBop3		4
#define SUBop3cc	SetCC|SUBop3
#define SMULop3		11
#define SDIVop3		15
#define XNORop3		SetCC|7
#define SAVEop3		60
#define RESTOREop3	61
#define JMPLop3		56

#define FP_OP3_LOW	0x20
#define LDFop3		0x20
#define LDFSRop3	0x21
#define LDDFop3		0x23
#define STFop3		0x24
#define STFSRop3	0x25
#define STDFQop3	0x26
#define STDFop3		0x27
#define FP_OP3_HIGH	0x27

#define FP_OP2_FPop1	0x34
#define FP_OP2_FPop2	0x35

/* op = 11 */
#define LDSTop  3
#define STop	3
#define LDop3	0
#define STop3	4
#define LDDop3  3
#define STDop3  7
#define SWAPop  3
#define SWAPop3 15 

/* mask bits for various parts of the instruction format */
#define OPmask		0xc0000000
#define OP2mask		0x01c00000
#define OP3mask		0x01f80000
#define RDmask		0x3e000000

#define DISP30mask	0x3fffffff

/* op = 01 -- mask for and match for call instruction */
#define	CALLop		1
#define CALLmask	OPmask
#define CALLmatch	0x40000000

/* (op==10) && (op3 == 111000) 
 */
#define RESTop		2
#define JMPLmask	(OPmask|OP3mask)
#define JMPLmatch	0x81c00000

#define FMT2op		0
#define LOADop		3

/*
 * added this on 8/18 (jkh) to tell a jmpl from a call indirect.
 *
 */
#define CALLImask	(OPmask|RDmask|OP3mask)
#define CALLImatch	0x9fc00000

/* (op=10) && (op3==111001) trap instructions */
#define TRAPmask	(OPmask|OP3mask)
#define TRAPmatch	0x81d00000

/* (op == 00) && (op2 ^ 2) mask for conditional branching instructions */
#define BICCop2		2

#define BEcond		1
#define BLEcond		2
#define BLTcond         3
#define BGTcond        10
#define BGEcond        11
#define BAcond		8
#define BNEcond		9

#define BRNCHmask	(OPmask|OP2mask)
#define BRNCHmatch	0x1<<23

#define FBRNCHmask      (OPmask|OP2mask)
#define FBRNCHmatch     0x11<<23

/* really jmpl %i7+8,%g0 */
/* changed this to jmpl %i7+??,%g0 since g++ sometimes returns +0xc not +0x8 
 * jkh - 7/12/93
 */
#define RETmask         0xfffff000
#define RETmatch	0x81c7e000

/* retl - leaf return really jmpl %o7+8,%g0 */
/* changed this to jmpl %i7+??,%g0 since g++ sometimes returns +0xc not +0x8
 * jkh - 7/12/93
 */
#define RETLmask        0xfffff000
#define RETLmatch	0x81c3e000


#define SAVEmask        (OPmask|OP3mask)
#define SAVEmatch       0x81e00000


/* noop insn */
#define NOOPop2		4
#define SETHIop2	4

/* If these bits are non-zero an op2 instruction is a non-annuled branch */
#define ANNUL_BIT	0x40000000

#define LOW(x)	((x)%1024)
#define HIGH(x)	((x)/1024)

#define BREAK_POINT_INSN 0x91d02001   /* ta 1 */

inline bool isInsnType(const instruction i,
		       const unsigned mask,
		       const unsigned match) {
  return ((i.raw & mask) == match);
}

inline bool isCallInsn(const instruction i) {
  return (isInsnType(i, CALLmask, CALLmatch) ||
	  isInsnType(i, CALLImask, CALLImatch));
}

inline bool isCondBranch(const instruction i){
     if (i.branch.op == 0 && (i.branch.op2 == 2 || i.branch.op2 == 6)) {
         if ((i.branch.cond != 0) && (i.branch.cond != 8))  
	     return true;
     }
     return false;
}

inline bool IS_DELAYED_INST(const instruction insn) {
  return (insn.call.op == CALLop ||
	  isInsnType(insn, JMPLmask, JMPLmatch) ||
	  isInsnType(insn, BRNCHmask, BRNCHmatch));
}

/* catch small ints that are invalid instructions */
/*
 * insn.call.op checks for CALL or Format 3 insns
 * op2 == {2,4,6,7} checks for valid format 2 instructions.
 *    See SPARC Arch manual v8 p. 44.
 *
 */
inline bool IS_VALID_INSN(const instruction insn) {
  return ((insn.call.op) || ((insn.branch.op2 == 2) ||
			     (insn.branch.op2 == 4) ||
			     (insn.branch.op2 == 6) ||
			     (insn.branch.op2 == 7)));
}

#endif
