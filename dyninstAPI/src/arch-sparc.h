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

// $Id: arch-sparc.h,v 1.27 2001/09/07 21:15:07 tikir Exp $

#if !defined(sparc_sun_sunos4_1_3) && !defined(sparc_sun_solaris2_4)
#error "invalid architecture-os inclusion"
#endif

#ifndef _ARCH_SPARC_H
#define _ARCH_SPARC_H

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

struct fmt3ix {
    unsigned op:2;
    unsigned rd:5;
    unsigned op3:6;
    unsigned rs1:5;
    unsigned i:1;
    unsigned x:1;
    unsigned unused:7;
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
    struct fmt3ix restix;
    unsigned int raw;
};

typedef union instructUnion instruction;


/*
 * Define the operation codes
 *
 */
#define SetCC		16
#define ADDop3		0
#define ADDop3cc	16
#define ANDop3		1
#define ANDop3cc        17
#define ANDNop3cc       21
#define ORop3		2
#define SUBop3		4
#define SUBop3cc	SetCC|SUBop3
#define SMULop3		11
#define SDIVop3		15
#define WRYop3          48      //SDIV needs to set Y register to 0
#define RDYop3          40
#define XNORop3		SetCC|7
#define SAVEop3		60
#define RESTOREop3	61
#define JMPLop3		56

#define SLLop3          37
#define SLLop           2
#define SRLop3          38
#define SRLop           2

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

#define FLUSHWop3 43

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
#define FBRNCHmatch     0x3<<23

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

#define RESTOREmask     (OPmask|OP3mask)
#define RESTOREmatch    0x81e80000

/* noop insn */
#define NOOPop2		4
#define SETHIop2	4

/* If these bits are non-zero an op2 instruction is a non-annuled branch */
#define ANNUL_BIT	0x40000000

#define BREAK_POINT_INSN 0x91d02001   /* ta 1 */
#define SPILL_REGISTERS_INSN 0x91d02003 /*ta 3*/


inline bool isInsnType(const instruction i,
		       const unsigned mask,
		       const unsigned match) {
  return ((i.raw & mask) == match);
}

/*
   Return boolean value specifying whether instruction is NOP....
 */
inline bool isNopInsn(const instruction i) {
    if (i.sethi.op == 0 && i.sethi.op2 == 0x4) {
        return true;
    }
    return false;
}

inline bool isCallInsn(const instruction i) {
  return (isInsnType(i, CALLmask, CALLmatch) ||
	  isInsnType(i, CALLImask, CALLImatch));
}

//  is instruction i a "true" call?
//  2 types of call insn in sparc:
//    true call - call to fixed address (really pc rel addr, but disass will show
//     fixed addr - for your convenience)....
//    jmpl (jump & link) - call to register | register + const |
//       register + register, and stick return address in some 
//       register.  If dest register is 07, a disassembler should
//       show a "call" insn.
inline bool isTrueCallInsn(const instruction i) {
    return (i.call.op == 0x1);
}

inline bool isJmplCallInsn(const instruction i) {
    // condition for jmpl insn
    if (i.resti.op == 0x2 && i.resti.op3 == 0x38) {
        // condition for dest register == 07
	if (i.resti.rd == 15) {
	    return true;
	}
    }
    return false;
}

inline bool isJmplInsn(const instruction i){
  if (i.resti.op == 0x2 && i.resti.op3 == 0x38)
    return true;
  return false;
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
	  isInsnType(insn, FBRNCHmask, FBRNCHmatch) ||
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

/* addresses on sparc are aligned to word boundaries */
inline bool isAligned(const Address addr) { return !(addr & 0x3); }

int get_disp(instruction *insn);
int set_disp(bool setDisp, instruction *insn, int newOffset, bool outOfFunc);
int sizeOfMachineInsn(instruction *insn);
int addressOfMachineInsn(instruction *insn);

#define region_lo(x) ( (x > (0x1 << 23))? (x-(0x1 << 23)):0x0 )
#define region_hi(x) ( (x > (-1UL - (1<<23))) ? -1UL : (x + (0x1 << 23)))

class InsnRegister {
public:
	enum RegisterType {GlobalIntReg=0,FloatReg,CoProcReg,SpecialReg,None};

	InsnRegister();
	InsnRegister(char isD,RegisterType rt,unsigned short rn);
	void setWordCount(char isD);
	void setType(RegisterType rt);
	void setNumber(short rn);
	bool is_o7();
	void print();

private:
	char wordCount;
	RegisterType regType;
	short regNumber;
};

void get_register_operands(const instruction&,
			   InsnRegister*,InsnRegister*,InsnRegister*);

#endif
