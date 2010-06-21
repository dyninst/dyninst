/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: arch-sparc.h,v 1.56 2008/11/03 15:19:24 jaw Exp $

#include "common/h/Vector.h"
// TUGRUL
#include "Annotatable.h"

#ifndef _ARCH_SPARC_H
#define _ARCH_SPARC_H

#include "common/h/Types.h"

class AddressSpace;
namespace NS_sparc {

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

/* illegal instructions */
#define ILLTRAPop 0
#define ILLTRAPop2 = 0

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

/*
 *    The following definitions are used for readWriteRegisters method
 */
#define WIN_SIZE 16
#define MAX_SETS 32
#define FLOAT_OFFSET 530 // must be greater than 528 which is the max. number of registers on sparc

#define SINGLE 1
#define DOUBLE 2
#define QUAD 4

/* although the real values of the following constants are not important, they have to be greater than
 * FLOAT_OFFSET + 62
 */
#define FCC0 600
#define FCC1 601
#define FCC2 602
#define FCC3 603
#define ICC 604
#define XCC 605
#define FSR 606
#define REG_Y_reg 610
#define REG_CCR 611
#define REG_ASI 612
#define REG_TICK 613
#define REG_PC_reg 614
#define REG_FPRS 615
#define REG_TPC 616
#define REG_TNPC 617
#define REG_TSTATE 618
#define REG_TT 619
//#define REG_TICK 620
#define REG_TBA 621
#define REG_PSTATE 622
#define REG_TL 623
#define REG_PIL 624
#define REG_CWP 625
#define REG_CANSAVE 626
#define REG_CANRESTORE 627
#define REG_CLEANWIN 628
#define REG_OTHERWIN 629
#define REG_WSTATE 630
#define REG_FQ 631
#define REG_VER 632

/* opcodes */
#define SETHIop2 4
#define BProp2 3
#define BPop2cc 1
#define Bop2icc 2
#define FBPop2fcc 5
#define FBop2fcc 6

#define ILLEGAL_0x19 0x19
#define ILLEGAL_0x1D 0x1D
#define ILLEGAL_0x29 0x29
#define ILLEGAL_0x33 0x33
#define ILLEGAL_0x3F 0x3F
#define ADDCop3 0x08
#define MULXop3 0x09
#define UMULop3 0x0A
#define SUBCop3 0x0C
#define UDIVXop3 0x0D
#define UDIVop3 0x0E
#define ORop3cc 0x12
#define XORop3cc 0x13
#define ORNop3cc 0x16
#define XNORop3cc 0x17
#define ADDCop3cc 0x18
#define UMULop3cc 0x1A
#define SMULop3cc 0x1B
#define SUBCop3cc 0x1C
#define UDIVop3cc 0x1E
#define SDIVop3cc 0x1F
#define TADDop3cc 0x20
#define TSUBop3cc 0x21
#define TADDop3ccTV 0x22
#define TSUBop3ccTV 0x23
#define MULSop3cc 0x24
#define RDPRop3 0x2A
#define MOVop3cc 0x2C
#define SDIVXop3 0x2D
#define POPCop3 0x2E
#define MOVrop3 0x2F
#define SAVED_RESTOREDop3 0x31
#define WRPRop3 0x32
#define IMPDEP1op3 0x36
#define IMPDEP2op3 0x37
#define RETURNop3 0x39
#define Tccop3 0X3A
#define FLUSHop3 0X3B
#define DONE_RETRYop3 0X3E

/* the following constants are used when opcode=2, op3=0x30 */
#define WRY 0
#define WRCCR 2
#define WRASI 3
#define WRFPRS 6
#define SIR 15

/* the following constants are used when opcode=2, op3=0x30 */
#define TPC 0
#define TNPC 1
#define TSTATE 2
#define TT 3
#define TICK_reg 4 // TICK is defined somewhere else
#define TBA 5
#define PSTATE 6
#define TL 7
#define PIL 8
#define CWP 9
#define CANSAVE 10
#define CANRESTORE 11
#define CLEANWIN 12
#define OTHERWIN 13
#define WSTATE 14

/* the following constants are used when opcode=2, op3=0x34 */
#define FMOVs 0x001
#define FMOVd 0x002
#define FMOVq 0x003
#define FNEGs 0x005
#define FNEGd 0x006
#define FNEGq 0x007
#define FABSs 0x009
#define FABSd 0x00A
#define FABSq 0x00B
#define FSQRTs 0x029
#define FSQRTd 0x02A
#define FSQRTq 0x02B
#define FsTOx 0x081
#define FdTOx 0x082
#define FqTOx 0x083
#define FxTOs 0x084
#define FxTOd 0x088
#define FxTOq 0x08C
#define FiTOs 0x0C4
#define FdTOs 0x0C6
#define FqTOs 0x0C7
#define FiTOd 0x0C8
#define FsTOd 0x0C9
#define FqTOd 0x0CB
#define FiTOq 0x0CC
#define FsTOq 0x0CD
#define FdTOq 0x0CE
#define FsTOi 0x0D1
#define FdTOi 0x0D2
#define FqTOi 0x0D3
#define FADDs 0x041
#define FADDd 0x042
#define FADDq 0x043
#define FSUBs 0x045
#define FSUBd 0x046
#define FSUBq 0x047
#define FMULs 0x049
#define FMULd 0x04A
#define FMULq 0x04B
#define FDIVs 0x04D
#define FDIVd 0x04E
#define FDIVq 0x04F
#define FsMULd 0x069
#define FdMULq 0x06E

/* the following constants are used when opcode=2, op3=0x35 */
#define FMOVsfcc0 0x001
#define FMOVdfcc0 0x002
#define FMOVqfcc0 0x003
#define FMOVRsZ 0x025
#define FMOVRdZ 0x026
#define FMOVRqZ 0x027
#define FMOVsfcc1 0x041
#define FMOVdfcc1 0x042
#define FMOVqfcc1 0x043
#define FMOVRsLEZ 0x045
#define FMOVRdLEZ 0x046
#define FMOVRqLEZ 0x047
#define FCMPs 0x051
#define FCMPd 0x052
#define FCMPq 0x053
#define FCMPEs 0x055
#define FCMPEd 0x056
#define FCMPEq 0x057
#define FMOVRsLZ 0x065
#define FMOVRdLZ 0x066
#define FMOVRqLZ 0x067
#define FMOVsfcc2 0x081
#define FMOVdfcc2 0x082
#define FMOVqfcc2 0x083
#define FMOVRsNZ 0x0A5
#define FMOVRdNZ 0x0A6
#define FMOVRqNZ 0x0A7
#define FMOVsfcc3 0x0C1
#define FMOVdfcc3 0x0C2
#define FMOVqfcc3 0x0C3
#define FMOVRsGZ 0x0C5
#define FMOVRdGZ 0x0C6
#define FMOVRqGZ 0x0C7
#define FMOVRsGEZ 0x0E5
#define FMOVRdGEZ 0x0E6
#define FMOVRqGEZ 0x0E7
#define FMOVsicc 0x101
#define FMOVdicc 0x102
#define FMOVqicc 0x103
#define FMOVsxcc 0x181
#define FMOVdxcc 0x182
#define FMOVqxcc 0x183

#define LDSWop3 0x08
#define LDXop3 0x0B
#define LDSTUBop3 0x0D
#define STXop3 0x0E
#define LDUWAop3 0x10
#define LDUBAop3 0x11
#define LDUHAop3 0x12
#define LDDAop3 0x13
#define STWAop3 0x14
#define STBAop3 0x15
#define STHAop3 0x16
#define STDAop3 0x17
#define LDSWAop3 0x18
#define LDSBAop3 0x19
#define LDSHAop3 0x1A
#define LDXAop3 0x1B
#define LDSTUBAop3 0x1D
#define STXAop3 0x1E
#define SWAPAop3 0x1F
#define LDQFop3 0x22
#define STQFop3 0x26
#define PREFETCHop3 0x2D
#define LDFAop3 0x30
#define LDQFAop3 0x32
#define LDDFAop3 0x33
#define STFAop3 0x34
#define STQFAop3 0x36
#define STDFAop3 0x37
#define CASAop3 0x3C
#define PREFETCHAop3 0x3D
#define CASXAop3 0x3E

#define SRAop3 0x27

// some macros for helping code which contains register symbolic names
#define REG_I(x) (x + 24)
#define REG_L(x) (x + 16) 
#define REG_O(x) (x + 8)
#define REG_G(x) (x)

/* End definitions for readWriteRegisters */

class InsnRegister {
public:
	enum RegisterType { GlobalIntReg=0, FloatReg, CoProcReg, SpecialReg, NoneReg};

	InsnRegister();
	InsnRegister(char isD, RegisterType rt, short rn);
	void setWordCount(char isD);
	void setType(RegisterType rt);
	void setNumber(short rn);
	int getWordCount();
	RegisterType getType();
	int getNumber();
	bool is_o7();
	void print();

private:
	char wordCount;
	RegisterType regType;
	short regNumber;
};

#if 0
typedef struct {} register_read_set_a;
typedef struct {} register_write_set_a;
static AnnotationClass<std::vector<InsnRegister *> > RegisterReadSetAnno("RegisterReadSetAnno");
static AnnotationClass<std::vector<InsnRegister *> > RegisterWriteSetAnno("RegisterWriteSetAnno");
#endif
class InsnRegister;
class instruction : public AnnotatableSparse
{
 public:
    instruction() {insn_.raw = 0;};
    instruction(unsigned int raw) { insn_.raw = raw; }

    instruction(const instruction &insn) :
        AnnotatableSparse(),
        insn_(insn.insn_) {};
    instruction(instructUnion &insn) :
        insn_(insn) {};
    
    instruction *copy() const;

    void setInstruction(codeBuf_t *ptr, Address = 0);

    static bool offsetWithinRangeOfBranchInsn(long off);
    
    unsigned spaceToRelocate() const;
  bool getUsedRegs(pdvector<int> &regs);


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

  // And tell us how much space we'll need...
  static unsigned jumpSize(Address from, Address to, unsigned addr_width);
  static unsigned jumpSize(int disp, unsigned addr_width);
  static unsigned maxJumpSize(unsigned addr_width);

  static unsigned maxInterFunctionJumpSize(unsigned addr_width);


  bool isInsnType(const unsigned mask,
                  const unsigned match) const {
      return ((insn_.raw & mask) == match);
  }

  bool isCall() const;

  void get_register_operands();
#if 0
  void get_register_operands(InsnRegister * reads,
			     InsnRegister * writes);
#endif

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

  bool isIllegal() const {
    if (insn_.sethi.op == 0 && insn_.sethi.op2 == 0) {
        return true;
    }
    return false;
  }
  bool isCleaningRet() const { return false; }
  
 private:
  instructUnion insn_;

  
};

inline Address ABS(int x) {
   if (x < 0) return -x;
   return x;
}

} // arch_sparc namespace

#endif
