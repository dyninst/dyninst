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

#if !defined(mips_sgi_irix6_4) && !defined(mips_unknown_ce2_11) //ccw 20 july 2000
#error "invalid architecture-os inclusion"
#endif

#ifndef _ARCH_MIPS_H_
#define _ARCH_MIPS_H_

/*
 * $Id: arch-mips.h,v 1.9 2004/03/23 01:12:01 eli Exp $
 */

#include "common/h/Types.h" // for "Address"

#define BRANCH_MAX (131068)
#define BRANCH_MIN (-131072)

/*
 * Define MIPS instruction information.
 *
 */

// ccw 9 aug 2000 : 28 mar 2001
// WinCE puts the MIPS chip into little-endian mode! how nice.
// the following instruction masks are for little-endian mode.
#ifdef mips_unknown_ce2_11
struct fmt_decode {
  // unsigned op:6; //ccw 9 aug 2000
  unsigned rest:26;
  unsigned op:6; //ccw 9 aug 2000 : see comment for fmt_jtype
};

struct fmt_itype {
  signed   simm16:16;
  unsigned rt:5;
  unsigned rs:5;
  unsigned op:6;
};

/* all regimm insns have primary opcode "REGIMM" */
struct fmt_regimm {
  signed   simm16:16;
  unsigned opr:5;
  unsigned rs:5;
  unsigned op:6; // op == REGIMMop
};

struct fmt_jtype {
  //unsigned op:6; //ccw 9 aug 2000
  unsigned imm26:26;
  unsigned op:6; //ccw 9 aug 2000 : this was moved down b/c WINCE is little endian
};

/* all R-Type insns have primary opcode "SPECIAL" */
struct fmt_rtype {
  unsigned ops:6;
  unsigned sa:5;
  unsigned rd:5;
  unsigned rt:5;
  unsigned rs:5;
  unsigned op:6; // op == SPECIALop
};

struct fmt_except {
  unsigned ops:6;
  unsigned code:20;
  unsigned op:6; // op == SPECIALop
};

#else

	//ccw 9 aug 2000 : big-endian mode!

struct fmt_decode {
  unsigned op:6;
  unsigned rest:26;
};

struct fmt_itype {
  unsigned op:6;
  unsigned rs:5;
  unsigned rt:5;
  signed   simm16:16;
};

/* all regimm insns have primary opcode "REGIMM" */
struct fmt_regimm {
  unsigned op:6; // op == REGIMMop
  unsigned rs:5;
  unsigned opr:5;
  signed   simm16:16;
};

struct fmt_jtype {
  unsigned op:6;
  unsigned imm26:26;
};

/* all R-Type insns have primary opcode "SPECIAL" */
struct fmt_rtype {
  unsigned op:6; // op == SPECIALop
  unsigned rs:5;
  unsigned rt:5;
  unsigned rd:5;
  unsigned sa:5;
  unsigned ops:6;
};

struct fmt_except {
  unsigned op:6; // op == SPECIALop
  unsigned code:20;
  unsigned ops:6;
};
#endif

union instructUnion {
  unsigned int        raw;    // make this field first to allow initialization
  struct fmt_decode   decode;
  struct fmt_itype    itype;
  struct fmt_regimm   regimm;
  struct fmt_jtype    jtype;
  struct fmt_rtype    rtype;
  struct fmt_except   except;
};

typedef union instructUnion instruction;
#define INSN_SIZE (sizeof(instruction))


/*
 * define instruction opcodes
 */
#define SPECIALop 0
#define REGIMMop 1
#define ADDops 32
#define ADDIop 8
#define ADDIUop 9
#define ADDUops 33
#define ANDops 36
#define ANDIop 12
#define BEQop 4
#define BEQLop 20
#define BGEZopr 1
#define BGEZALopr 17
#define BGEZALLopr 19
#define BGEZLopr 3
#define BGTZop 7
#define BGTZLop 23
#define BLEZop 6
#define BLEZLop 22
#define BLTZopr 0
#define BLTZALopr 16
#define BLTZALLopr 18
#define BLTZLopr 2
#define BNEop 5
#define BNELop 21
#define BREAKops 13
#define COP0op 16
#define COP1op 17
#define COP1Xop 19
#define COP2op 18
#define DADDops 44
#define DADDIop 24
#define DADDIUop 25
#define DADDUops 45
#define DDIVops 30
#define DDIVUops 31
#define DIVops 26
#define DIVUops 27
#define DMULTops 28
#define DMULTUops 29
#define DSLLops 56
#define DSLL32ops 60
#define DSLLVops 20
#define DSRAops 59
#define DSRA32ops 63
#define DSRAVops 23
#define DSRLops 58
#define DSRL32ops 62
#define DSRLVops 22
#define DSUBops 46
#define DSUBUops 47
#define Jop 2
#define JALop 3
#define JALRops 9
#define JRops 8
#define LBop 32
#define LBUop 36
#define LDop 55
#define LDC1op 53
#define LDC2op 45
#define LDLop 26
#define LDRop 27
#define LHop 33
#define LHUop 37
#define LLop 48
#define LLDop 52
#define LUIop 15
#define LWop 35
#define LWC1op 49
#define LWC2op 50
#define LWLop 34
#define LWRop 38
#define LWUop 39
#define MFHIops 16
#define MFLOops 18
#define MOVNops 11
#define MOVZops 10
#define MTHIops 17
#define MTLOops 19
#define MULTops 24
#define MULTUops 25
#define NOPop 28
#define NORops 39
#define ORops 37
#define ORIop 13
#define PREFop 51
#define SBop 40
#define SCop 56
#define SCDop 60
#define SDop 63
#define SDC1op 61
#define SDC2op 62
#define SDLop 44
#define SDRop 45
#define SHop 41
#define SLLops 0
#define SLLVops 4
#define SLTops 42
#define SLTIop 10
#define SLTIUop 11
#define SLTUops 43
#define SRAops 3
#define SRAVops 7
#define SRLops 2
#define SRLVops 6
#define SUBops 34
#define SUBUops 35
#define SWop 43
#define SWC1op 57
#define SWC2op 58
#define SWC3op 59
#define SWLop 42
#define SWRop 46
#define SYNCops 15
#define SYSCALLops 12
#define TEQops 52
#define TEQIopr 12
#define TGEops 48
#define TGEIopr 8
#define TGEIUopr 9
#define TGEUops 49
#define TLTops 50
#define TLTIopr 10
#define TLTIUopr 11
#define TLTUops 51
#define TNEops 54
#define TNEIopr 14
#define XORops 38
#define XORIop 14

#define NOP_INSN     (0x00000000)

#ifdef mips_unknown_ce2_11 //ccw 13 feb 2001 : 28 mar 2001
//on wince 0x00000034 causes a print string debug exeception to be thrown,
//without telling what address or instruction threw it, so it is useless
#define TRAP_INSN    (0x0000000D)
#else
#define TRAP_INSN    (0x00000034)
#endif

#define ILLEGAL_INSN (0x70000000)


/* register info */
#define NUM_REGS (32)
/* named registers */
#define REG_ZERO (0)
#define REG_AT   (1)
#define REG_V0   (2)
#define REG_A0   (4)
#define REG_T9  (25)
#define REG_GP  (28)
#define REG_SP  (29)
#define REG_S8  (30)
#define REG_RA  (31)
/* procfs (gregset_t) register indices */
#define PROC_REG_RV (CTX_V0)
#define PROC_REG_SP (CTX_SP)
#define PROC_REG_FP (CTX_S8)
#define PROC_REG_PC (CTX_EPC)
#define PROC_REG_A0 (CTX_A0)
#define PROC_REG_A1 (CTX_A1)
#define PROC_REG_T3 (CTX_T3)
#define PROC_REG_T9 (CTX_T9)
#define PROC_REG_RA (CTX_RA)

/* calling conventions */
#ifdef mips_unknown_ce2_11 //ccw 10 jan 2001 : 28 mar 2001
  // ccw  10 jan 2001 mips on the CE devices
  // uses the o32 calling convention, only allowing 
  // 4 arguments to be passed in registers.
#define NUM_ARG_REGS (4)
//ccw 10 jan 2001 : ce is 32 bit!
#define BYTES_PER_ARG (4) 

#else
#define NUM_ARG_REGS (8)
#define BYTES_PER_ARG (8)
#endif

extern char *reg_names[];

/**********/
/* memory */
/**********/

/* IRIX memory "regions" */
#define REGION_OFF         (28)
#define REGION_OFF_MASK    ((Address)((1 << REGION_OFF) - 1))
#define REGION_NUM_MASK    (~(Address)REGION_OFF_MASK)
#define region_num(addr)   (((Address)addr) >> REGION_OFF)
#define region_lo(addr)    (((Address)addr) & REGION_NUM_MASK)
#define region_hi(addr)    (region_lo(addr) | REGION_OFF_MASK)
#define region_index(addr) ((((Address)addr) & REGION_OFF_MASK) >> 2)

/* instruction addresses are aligned on 4-byte boundaries */
inline bool isAligned(const Address addr) { return !(addr & 0x3); }


/************************/
/* instruction matching */
/************************/

inline bool isInsnType(const instruction i,
		       const unsigned mask,
		       const unsigned match) {
  return ((i.raw & mask) == match);
}

/* masks for various instruction components */
#define OPmask 0xfc000000
#define OPSmask 0x0000003f
#define SHAMTmask 0x000007c0
#define OPRmask 0x001f0000
#define RSmask 0x03e00000
#define RTmask 0x001f0000
#define RDmask 0x0000f800

/* instruction field offsets */
#define OPoff 26
#define OPSoff 0
#define OPRoff 16
#define RSoff 21
#define RToff 16
#define RDoff 11

/* instruction masks and matches */
#define ADDmask      (OPmask | OPSmask)
#define ADDmatch     (0x00000020)
#define ADDImask     (OPmask | OPSmask)
#define ADDImatch    (0x20000000)
#define ADDIUmask    (OPmask)
#define ADDIUmatch   (0x24000000)
#define ADDUmask     (OPmask | OPSmask)
#define ADDUmatch    (0x00000021)
#define BEQmask      (OPmask)
#define BEQmatch     (0x10000000)
#define BEQLmask     (OPmask)
#define BEQLmatch    (0x50000000)
#define BGEZALLmask  (OPmask | OPRmask)
#define BGEZALLmatch (0x04130000)
#define BNEmask      (OPmask)
#define BNEmatch     (0x14000000)
#define BNELmask     (OPmask)
#define BNELmatch    (0x54000000)
#define BRANCHmask   (0xb0000000)
#define BRANCHmatch  (0x10000000)
#define BRANCHmaskR  (0xfc0c0000)
#define BRANCHmatchR (0x04000000)
#define DADDmask     (OPmask | OPSmask)
#define DADDmatch    (0x0000002c)
#define DADDIUmask   (OPmask)
#define DADDIUmatch  (0x64000000)
#define DADDUmask    (OPmask | OPSmask)
#define DADDUmatch   (0x0000002d)
#define Jmask        (OPmask)
#define Jmatch       (0x08000000)
#define JALRmask     (OPmask | OPSmask)
#define JALRmatch    (0x00000009)
#define JALmask      (OPmask)
#define JALmatch     (0x0c000000)
#define JRmask       (OPmask | OPSmask)
#define JRmatch      (0x00000008)
#define LBmask       (OPmask)
#define LBmatch      (0x80000000)
#define LBUmask      (OPmask)
#define LBUmatch     (0x90000000)
#define LDmask       (OPmask)
#define LDmatch      (0xdc000000)
#define LDC1mask     (OPmask)
#define LDC1match    (0xd4000000)
#define LDC2mask     (OPmask)
#define LDC2match    (0xd8000000)
#define LDLmask      (OPmask)
#define LDLmatch     (0x68000000)
#define LDRmask      (OPmask)
#define LDRmatch     (0x6c000000)
#define LHmask       (OPmask)
#define LHmatch      (0x84000000)
#define LHUmask      (OPmask)
#define LHUmatch     (0x94000000)
#define LLmask       (OPmask)
#define LLmatch      (0xc0000000)
#define LLDmask      (OPmask)
#define LLDmatch     (0xd0000000)
#define LUImask      (OPmask)
#define LUImatch     (0x3c000000)
#define LWmask       (OPmask)
#define LWmatch      (0x8c000000)
#define LWC1mask     (OPmask)
#define LWC1match    (0xc4000000)
#define LWC2mask     (OPmask)
#define LWC2match    (0xc8000000)
#define LWLmask      (OPmask)
#define LWLmatch     (0x88000000)
#define LWRmask      (OPmask)
#define LWRmatch     (0x98000000)
#define LWUmask      (OPmask)
#define LWUmatch     (0x9c000000)
#define ORmask       (OPmask | OPSmask)
#define ORmatch      (0x00000025)
#define SBmask       (OPmask)
#define SBmatch      (0xa0000000)
#define SCmask       (OPmask)
#define SCmatch      (0xe0000000)
#define SCDmask      (OPmask)
#define SCDmatch     (0xf0000000)
#define SDmask       (OPmask)
#define SDmatch      (0xfc000000)
#define SDC1mask     (OPmask)
#define SDC1match    (0xf4000000)
#define SDC2mask     (OPmask)
#define SDC2match    (0xf8000000)
#define SDLmask      (OPmask)
#define SDLmatch     (0xb0000000)
#define SDRmask      (OPmask)
#define SDRmatch     (0xb4000000)
#define SHmask       (OPmask)
#define SHmatch      (0xa4000000)
#define SUBUmask     (OPmask | OPSmask)
#define SUBUmatch    (0x00000023)
#define SWmask       (OPmask)
#define SWmatch      (0xab000000)
#define SWC1mask     (OPmask)
#define SWC1match    (0xe4000000)
#define SWC2mask     (OPmask)
#define SWC2match    (0xe8000000)
#define SWLmask      (OPmask)
#define SWLmatch     (0xa8000000)
#define SWRmask      (OPmask)
#define SWRmatch     (0xb8000000)
#define SYSCALLmask  (OPmask | OPSmask)
#define SYSCALLmatch (0x0000000c)
#define TRAPmaskR1   (0xfc1c0000)
#define TRAPmatchR1  (0x04080000)
#define TRAPmaskR2   (0xfc1d0000)
#define TRAPmatchR2  (0x040c0000)
#define TRAPmaskS1   (0xfc00003c)
#define TRAPmatchS1  (0x00000030)
#define TRAPmaskS2   (0xfc00003d)
#define TRAPmatchS2  (0x00000034)

/* jumps */
inline bool isJumpInsn(const instruction i) {
  return (isInsnType(i, Jmask, Jmatch) ||
	  isInsnType(i, JALmask, JALmatch) ||
	  isInsnType(i, JALRmask, JALRmatch) ||
	  isInsnType(i, JRmask, JRmatch));
}

/* traps */
inline bool isTrapR(const instruction i) {
  return (isInsnType(i, TRAPmaskR1, TRAPmatchR1) ||
	  isInsnType(i, TRAPmaskR2, TRAPmatchR2));
}
inline bool isTrapS(const instruction i) {
  return (isInsnType(i, TRAPmaskS1, TRAPmatchS1) ||
	  isInsnType(i, TRAPmaskS2, TRAPmatchS2));
}
inline bool isTrapInsn(const instruction i) {
  return (isTrapR(i) || isTrapS(i));
}

/* save (case 1): "addiu sp,sp,-XX" */
static inline bool isSave1(const instruction i) {
  return (isInsnType(i, ADDIUmask, ADDIUmatch) &&
	  (i.itype.rs == REG_SP) &&
	  (i.itype.rt == REG_SP) &&
	  (i.itype.simm16 < 0));
}
inline bool isSaveInsn(const instruction i) {
  return (isSave1(i));
}

/* branches */
inline bool isBranchInsn(const instruction i) {
  return (isInsnType(i, BRANCHmask, BRANCHmatch) ||
	  isInsnType(i, BRANCHmaskR, BRANCHmatchR));
}

/* call (case 1): "jalr ra,RR" */
/* call (case 2): "jal  AAAA"  */
static inline bool isCall1(const instruction i) {
  return (isInsnType(i, JALRmask, JALRmatch) &&
	  (i.rtype.rd == REG_RA));
}
static inline bool isCall2(const instruction i) {
  return (isInsnType(i, JALmask, JALmatch));
}
inline bool isCallInsn(const instruction i) {
  return (isCall1(i) ||
	  isCall2(i));
}

/* return (case 1): "jr ra" */
inline bool isReturnInsn(const instruction i) {
  return (isInsnType(i, JRmask, JRmatch) &&
	  (i.rtype.rs == REG_RA));
}

#endif /* _ARCH_MIPS_H_ */
