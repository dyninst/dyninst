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

// $Id: arch-sparc.h,v 1.36 2005/08/15 22:20:05 bernat Exp $

#if !defined(arch_sparc)
#error "invalid architecture-os inclusion"
#endif

#ifndef _ARCH_SPARC_H
#define _ARCH_SPARC_H

// Code generation

typedef unsigned int codeBuf_t;
typedef unsigned codeBufIndex_t;


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
    unsigned anneal:1; // VG: must read anull I guess...
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

typedef union {
    struct fmt1	call;
    struct fmt2	branch;
    struct fmt2a sethi;
    struct fmt3 rest;
    struct fmt3i resti;
    struct fmt3ix restix;
    unsigned int raw;
} instructUnion;

/* Address space identifiers */
#define ASI_PRIMARY 0x80
#define ASI_PRIMARY_NOFAULT 0x82

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
#define LDUBop3 1
#define LDSBop3 9
#define STBop3 5
#define LDUHop3 2
#define LDSHop3 10
#define STHop3 6
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

// VG(4/24/2002): This was wrong; fortunately, it was not used anywhere...
#define ANNUL_BIT	0x20000000

#define BREAK_POINT_INSN 0x91d02001   /* ta 1 */
#define SPILL_REGISTERS_INSN 0x91d02003 /*ta 3*/

#define region_lo(x) ( (x > (0x1 << 23))? (x-(0x1 << 23)):0x0 )
#define region_hi(x) ( (x > (-1UL - (1<<23))) ? -1UL : (x + (0x1 << 23)))

class InsnRegister {
public:
	enum RegisterType { GlobalIntReg=0, FloatReg, CoProcReg, SpecialReg, NoneReg};

	InsnRegister();
	InsnRegister(char isD, RegisterType rt, short rn);
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

class codeGen;

class instruction {
 private:
    static instructUnion *insnPtr(codeGen &gen);
    static instructUnion *ptrAndInc(codeGen &gen);
 public:
    instruction() {insn_.raw = 0;};
    instruction(unsigned int raw) { insn_.raw = raw; }

    instruction(const instruction &insn) :
        insn_(insn.insn_) {};
    instruction(instructUnion &insn) :
        insn_(insn) {};
    
    instruction *copy() const;

    void setInstruction(codeBuf_t *ptr, Address = 0);

    static bool offsetWithinRangeOfBranchInsn(int off);
    
    static void generateTrap(codeGen &gen);
    static void generateIllegal(codeGen &gen);

    static void generateNOOP(codeGen &gen,
                             unsigned size = 4);
    static void generateBranch(codeGen &gen,
                               int jump_off);
    static void generateBranch(codeGen &gen,
                               Address from,
                               Address to);
    static void generateCall(codeGen &gen,
                             Address fromAddr,
                             Address toAddr);
    static void generateJmpl(codeGen &gen,
                             int rs1,
                             int jump_off,
                             int rd);
    static void generateCondBranch(codeGen &gen,
                                   int jump_off,
                                   unsigned condition,
                                   bool annul);
    static void generateAnnulledBranch(codeGen &gen,
                                       int jump_off);
    static void generateSimple(codeGen &gen,
                               int op,
                               Register rs1,
                               Register rs2,
                               Register rd);
    static void generateImm(codeGen &gen,
                            int op,
                            Register rs1,
                            int immd,
                            Register rd);
    static void generateImmRelOp(codeGen &gen,
                               int cond,
                               Register rs1,
                               int immd,
                                 Register rd);
    static void generateRelOp(codeGen &gen,
                              int cond,
                              Register rs1,
                              Register rs2,
                              Register rd);
    static void generateSetHi(codeGen &gen,
                              int src1,
                              int dest);
    static void generateStore(codeGen &gen,
                              int rd,
                              int rs1,
                              int store_off);
    static void generateLShift(codeGen &gen,
                               int rs1,
                               int shift,
                               int rd);
    static void generateRShift(codeGen &gen,
                               int rs1,
                               int shift,
                               int rd);
    static void generateLoad(codeGen &gen,
                             int rs1,
                             int load_off,
                             int rd);
    static void generateStoreD(codeGen &gen,
                               int rd,
                               int rs1,
                               int shift);
    static void generateLoadD(codeGen &gen,
                               int rs1,
                               int load_off,
                               int rd);
    static void generateLoadB(codeGen &gen,
                              int rs1,
                              int load_off,
                              int rd);    
    static void generateLoadH(codeGen &gen,
                              int rs1,
                              int load_off,
                              int rd);
    static void generateStoreFD(codeGen &gen,
                                int rd,
                                int rs1,
                                int shift);
    static void generateLoadFD(codeGen &gen,
                               int rs1,
                               int load_off,
                               int rd);
    static void generateFlushw(codeGen &gen);
    static void generateTrapRegisterSpill(codeGen &gen);
  
    void write(codeGen &gen);
    void generate(codeGen &gen);

  // return the type of the instruction
  unsigned type() const;

  // return a pointer to the instruction
  const unsigned char *ptr() const { return (const unsigned char *)&insn_; }

  const unsigned int &raw() const { return insn_.raw; }
  
  const unsigned opcode() const;

  // For external modification
  instructUnion &operator* () { return insn_; }
  const instructUnion &operator* () const { return insn_; }
  
  Address getTarget(Address insnAddr) const;
  Address getOffset() const;

  bool isInsnType(const unsigned mask,
                  const unsigned match) const {
      return ((insn_.raw & mask) == match);
  }

  bool isCall() const;

  void get_register_operands(InsnRegister *,
                             InsnRegister *,
                             InsnRegister *);

  //  is instruction i a "true" call?
  //  2 types of call insn in sparc:
  //    true call - call to fixed address (really pc rel addr, but disass will show
  //     fixed addr - for your convenience)....
  //    jmpl (jump & link) - call to register | register + const |
  //       register + register, and stick return address in some 
  //       register.  If dest register is 07, a disassembler should
  //       show a "call" insn.
  bool isTrueCallInsn() const {
      return (insn_.call.op == 0x1);
  }
  
  bool isJmplCallInsn() const {
      // condition for jmpl insn
      if (insn_.resti.op == 0x2 && insn_.resti.op3 == 0x38) {
          // condition for dest register == 07
          if (insn_.resti.rd == 15) {
              return true;
          }
      }
      return false;
  }
  
  inline bool isJmplInsn() const{
      if (insn_.resti.op == 0x2 && insn_.resti.op3 == 0x38)
          return true;
      return false;
  }
    
  // VG(4/24/200): DCTI = delayed control-transfer instruction
  // It was incomplete on v9, so here's a quick rewrite:
  bool isDCTI() const {
      return (insn_.call.op == 1 || // call
              (insn_.branch.op == 0 && // branches (BPcc, Bicc, BPr, FBPfcc, FBfcc)
               insn_.branch.op2 != 0 &&
               insn_.branch.op2 != 4 &&
               insn_.branch.op2 != 7) ||
              (insn_.rest.op == 2 &&  // JMPL, RETURN
               ((insn_.rest.op3 | 1) == 0x39))
              );
      /*   return (insn_.call.op == CALLop || */
      /* 	  isInsnType(insn, JMPLmask, JMPLmatch) || */
      /* 	  isInsnType(insn, FBRNCHmask, FBRNCHmatch) || */
      /* 	  isInsnType(insn, BRNCHmask, BRNCHmatch)); */
  }
  
  bool isAnnulled() const {
      return insn_.branch.anneal == 1;
  }
  
  // VG(4/24/200): UB = unconditional branch
  inline bool isUB() const {
      // Only Bicc(010), BPcc(001), FBfcc (110), FBPfcc(101) can be unconditional
      if(insn_.branch.op==00 && ((insn_.branch.op2 & 3) == 1 ||
                                 (insn_.branch.op2 & 3) == 2) &&
         // 0x0=never; 0x8=always
         (insn_.branch.cond == 0x0 || insn_.branch.cond == 0x8))
          return true;
      return false;
  }
  
  // VG(4/24/200): UBA = unconditional branch anulled
  bool isUBA() const {
      return isUB() && isAnnulled();
  }

  bool isRestore() const {
      return (insn_.rest.op == 2 && insn_.rest.op3 == RESTOREop3);
  }

  bool isMovToO7() const {
      return (insn_.rest.op == 2 && insn_.rest.op3 == ORop3 &&
              insn_.rest.rd == 15 && insn_.rest.rs1 == 0);
  }
  
  /* catch small ints that are invalid instructions */
  /*
   * insn.call.op checks for CALL or Format 3 insns
   * op2 == {2,4,6,7} checks for valid format 2 instructions.
   *    See SPARC Arch manual v8 p. 44.
   *
   */
  inline bool valid() const {
      return ((insn_.call.op) || ((insn_.branch.op2 == 2) ||
                                  (insn_.branch.op2 == 4) ||
                                  (insn_.branch.op2 == 6) ||
                                  (insn_.branch.op2 == 7)));
  }
  
  /* addresses on sparc are aligned to word boundaries */
  static bool isAligned(const Address addr) { return !(addr & 0x3); }
  
  int get_disp();
  void set_disp(bool setDisp, int newOffset, bool outOfFunc);
  static unsigned size();
  
  bool isUncondBranch() const;
  bool isCondBranch() const;
  
  bool isNop() const {
      if (insn_.sethi.op == 0 && insn_.sethi.op2 == 0x4) {
          return true;
      }
      return false;
  }
  
 private:
  instructUnion insn_;

  
};

#endif
