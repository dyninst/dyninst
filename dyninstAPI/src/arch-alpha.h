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

#if !defined(arch_alpha_hdr)
#define arch_alpha_hdr

#include <assert.h>
#include "ast.h" // opCode


// This is actually a "Software Trap" instruction.
// Changed to get GCC 3.x working.
#define BREAK_POINT_INSN 0x000000AA


// TODO -- which way do the bits go?
struct if_PAL {
  unsigned number:26;
  unsigned opcode:6;
};

struct if_Branch {
  signed disp:21;
  unsigned ra:5;
  unsigned opcode:6;
};

struct if_Mem {
  signed disp:16;
  unsigned rb:5;
  unsigned ra:5;
  unsigned opcode:6;
};

struct if_Mem_jmp {
  signed disp:14;
  unsigned ext:2;
  unsigned rb:5;
  unsigned ra:5;
  unsigned opcode:6;
};

struct if_Operate {
  unsigned rc:5;
  unsigned function:7;
  unsigned zero:1;            // always 0
  unsigned sbz:3;
  unsigned rb:5;
  unsigned ra:5;
  unsigned opcode:6;
};

struct if_Operate_lit {
  unsigned rc:5;
  unsigned function:7;
  unsigned one:1;            // always 1
  unsigned lit:8;
  unsigned ra:5;
  unsigned opcode:6;
};

typedef union {
  struct if_PAL          pal;
  struct if_Branch       branch;
  struct if_Mem          mem;
  struct if_Mem_jmp      mem_jmp;
  struct if_Operate      oper;
  struct if_Operate_lit  oper_lit;
  u_int32_t               raw;
} instructUnion;

// Code generation

typedef instructUnion codeBuf_t;
typedef unsigned codeBufIndex_t;

// ************************************************************************
// Memory format opcodes
// ************************************************************************

#define OP_LDL     0x28
#define OP_LDQ     0x29
#define OP_LDL_L   0x2a
#define OP_LDQ_L   0x2b
#define OP_LDQ_U   0x0b
#define OP_LDA     0x08
#define OP_LDBU    0x0a
#define OP_LDWU    0x0c

#define OP_STL     0x2c
#define OP_STQ     0x2d
#define OP_STL_C   0x2e
#define OP_STQ_C   0x2f
#define OP_STQ_U   0x0f
#define OP_LDAH    0x09

#define OP_LDF     0x20
#define OP_LDG     0x21
#define OP_LDS     0x22
#define OP_LDT     0x23

#define OP_STF     0x24
#define OP_STG     0x25
#define OP_STS     0x26
#define OP_STT     0x27

// Byte/word extension (BWX)

#define OP_SEXTX   0x1c
#define FC_SEXTB   0x00
#define FC_SEXTW   0x01

// This opcode requires a function code
// Function code goes in if_Mem.disp
#define OP_MEM_FC  0x18

// Function codes for previous opcode
#define FC_FETCH   0x8000
#define FC_RC      0xe000
#define FC_TRAPB   0x0000
#define FC_FETCH_M 0xa000
#define FC_RPCC    0xc000
#define FC_MB      0x4000
#define FC_RS      0xf000

// Use struct if_Mem_jmp for these instructions
// Opcode for memory branch
#define OP_MEM_BRANCH  0x1a

// High bit of displacement field for memory format branch
// Use field if_Mem_jmp.ext
#define MD_JMP     0x0
#define MD_JSR     0x1
#define MD_RET     0x2
#define MD_JSR_CO  0x3

// ************************************************************************
// Branch format opcodes
// ************************************************************************

#define OP_BR        0x30
#define OP_BSR       0x34
#define OP_BLBC      0x38
#define OP_BLBS      0x3c
#define OP_FBEQ      0x31
#define OP_FBNE      0x35
#define OP_BEQ       0x39
#define OP_BNE       0x3d
#define OP_FBLT      0x32
#define OP_FBGE      0x36
#define OP_BLT       0x3a
#define OP_BGE       0x3e
#define OP_FBLE      0x33
#define OP_FBGT      0x37
#define OP_BLE       0x3b
#define OP_BGT       0x3f

// ************************************************************************
// Operate format opcodes
// ************************************************************************

// Opcodes and function codes

#define OP_ADDL      0x10
#define FC_ADDL      0x00

#define OP_ADDL_V    0x10
#define FC_ADDL_V    0x40

#define OP_ADDQ      0x10
#define FC_ADDQ      0x20

#define OP_ADDQ_V    0x10
#define FC_ADDQ_V    0x60

#define OP_CMPULE    0x10
#define FC_CMPULE    0x3d

#define OP_CMPBGE    0x10
#define FC_CMPBGE    0x0f

#define OP_SUBL      0x10
#define FC_SUBL      0x09

#define OP_SUBL_V    0x10
#define FC_SUBL_V    0x49

#define OP_SUBQ      0x10
#define FC_SUBQ      0x29

#define OP_SUBQ_V    0x10
#define FC_SUBQ_V    0x69

#define OP_CMPEQ     0x10
#define FC_CMPEQ     0x2d

#define OP_CMPLT     0x10
#define FC_CMPLT     0x4d

#define OP_CMPLE     0x10
#define FC_CMPLE     0x6d

#define OP_CMPULT    0x10
#define FC_CMPULT    0x1d



#define OP_S4ADDL    0x10
#define FC_S4ADDL    0x02

#define OP_S4ADDQ    0x10
#define FC_S4ADDQ    0x22

#define OP_S4SUBL    0x10
#define FC_S4SUBL    0x0b

#define OP_S4SUBQ    0x10
#define FC_S4SUBQ    0x2b

#define OP_S8ADDL    0x10
#define FC_S8ADDL    0x12

#define OP_S8ADDQ    0x10
#define FC_S8ADDQ    0x32

#define OP_S8SUBL    0x10
#define FC_S8SUBL    0x1b

#define OP_S8SUBQ    0x10
#define FC_S8SUBQ    0x3b


#define OP_AND       0x11
#define FC_AND       0x00

#define OP_BIC       0x11
#define FC_BIC       0x08

#define OP_CMOVEQ    0x11
#define FC_CMOVEQ    0x24

#define OP_CMOVNE    0x11
#define FC_CMOVNE    0x26

#define OP_CMOVLBS   0x11
#define FC_CMOVLBS   0x14

#define OP_BIS       0x11
#define FC_BIS       0x20

#define OP_ORNOT     0x11
#define FC_ORNOT     0x28

#define OP_CMOVLT    0x11
#define FC_CMOVLT    0x44

#define OP_CMOVGE    0x11
#define FC_CMOVGE    0x46

#define OP_CMOVLBC   0x11
#define FC_CMOVLBC   0x16

#define OP_XOR       0x11
#define FC_XOR       0x40

#define OP_EQV       0x11
#define FC_EQV       0x48

#define OP_CMOVLE    0x11
#define FC_CMOVLE    0x64

#define OP_CMOVEGT   0x11
#define FC_CMOVEGT   0x66

#define OP_SLL       0x12
#define FC_SLL       0x39

#define OP_EXTBL     0x12
#define FC_EXTBL     0x06

#define OP_EXTWL     0x12
#define FC_EXTWL     0x16

#define OP_EXTLL     0x12
#define FC_EXTLL     0x26

#define OP_EXTQL     0x12
#define FC_EXTQL     0x36

#define OP_EXTWH     0x12
#define FC_EXTWH     0x5a

#define OP_EXTLH     0x12
#define FC_EXTLH     0x6a

#define OP_EXTQH     0x12
#define FC_EXTQH     0x7a

#define OP_SRA       0x12
#define FC_SRA       0x3c

#define OP_INSBL     0x12
#define FC_INSBL     0x0b

#define OP_INSWL     0x12
#define FC_INSWL     0x1b

#define OP_INSLL     0x12
#define FC_INSLL     0x2b

#define OP_INSQL     0x12
#define FC_INSQL     0x3b

#define OP_INSWH     0x12
#define FC_INSWH     0x57

#define OP_INSLH     0x12
#define FC_INSLH     0x67

#define OP_INSQH     0x12
#define FC_INSQH     0x77

#define OP_SRL       0x12
#define FC_SRL       0x34

#define OP_MSKBL     0x12
#define FC_MSKBL     0x02

#define OP_MSKWL     0x12
#define FC_MSKWL     0x12

#define OP_MSKLL     0x12
#define FC_MSKLL     0x22

#define OP_MSKQL     0x12
#define FC_MSKQL     0x32

#define OP_MSKWH     0x12
#define FC_MSKWH     0x52

#define OP_MSKLH     0x12
#define FC_MSKLH     0x62

#define OP_MSKQH     0x12
#define FC_MSKQH     0x72

#define OP_ZAP       0x12
#define FC_ZAP       0x30

#define OP_ZAPNOT    0x12
#define FC_ZAPNOT    0x31

#define OP_MULL      0x13
#define FC_MULL      0x00

#define OP_MULQ_V    0x13
#define FC_MULQ_V    0x60

#define OP_MULL_V    0x13
#define FC_MULL_V    0x40

#define OP_UMULH     0x13
#define FC_UMULH     0x30

#define OP_MULQ      0x13
#define FC_MULQ      0x20

// ************************************************************************
// PAL format opcodes
// ************************************************************************

#define OP_IMB       0x00
#define FC_IMB       0x0086

typedef enum { dw_long, dw_quad, dw_byte, dw_word } data_width;

#define MASK_4(x)     (x & 0xffffffff)
#define MASK_2(x)     (x & 0xffff)

const Address shifted_16 = (Address(1)) << 16;
const Address shifted_32 = (Address(1)) << 32;
const Address shifted_48 = (Address(1)) << 48;
const Address bit_15 =     (Address(1)) << 15;
const Address bit_31 =     (Address(1)) << 31;
const Address bit_47 =     (Address(1)) << 47;

#define FIRST_16(x)       ((x) & 0xffff)
#define SECOND_16(x)      ((x) >> 16)
#define THIRD_16(x)       ((x) >> 32)

#define ABS(x)		((x) > 0 ? (x) : -(x))

// TODO -- the max branch is (0x1 << 22)
// if we limited branches to this distance, dyn. inst. would not work
// on a 64 bit architecture
#define MAX_BRANCH	(0x1<<22)

#define MAX_IMM	0x1<<8  // 8 bits

#define SEXT_16(x) (((x) & bit_15) ? ((x) | 0xffffffffffff0000) : (x))
typedef unsigned long int Offset;


class instruction {
 private:
    static codeBuf_t *insnPtr(codeGen &gen);
    static codeBuf_t *ptrAndInc(codeGen &gen);
 public:
    instruction() {insn_.raw = 0;};
    instruction(unsigned int raw) { insn_.raw = raw; }

    instruction(const instruction &insn) :
        insn_(insn.insn_) {};
    instruction(instructUnion &insn) :
        insn_(insn) {};
    
    void setInstruction(codeBuf_t *ptr, Address = 0);

    static void generateTrap(codeGen &gen);    
    static void generateIllegal(codeGen &gen);


    // All of these write into a buffer
    static void generateLoad(codeGen &gen, Register rdest, Register rbase,
			     int disp, data_width width, bool aligned);
    static void generateStore(codeGen &gen, Register rsrc, Register rbase,
			     int disp, data_width width);
    static void generateNOOP(codeGen &gen, unsigned size = 4);
    static void generateOperate(codeGen &gen, Register reg_a, Register reg_b,
				Register reg_c, unsigned int opcode, 
				unsigned int func_code);
    static void generateLitOperate(codeGen &gen, Register reg_a, int literal,
				   Register reg_c, unsigned opcode, unsigned func_code);

    static void generateJump(codeGen &gen, Register dest_reg, unsigned long ext_code,
			     Register ret_reg, int hint);
    static void generateBranch(codeGen &gen, Register src_reg, int disp,
			       unsigned int opcode);
    // Known displacement
    static void generateBranch(codeGen &gen, int disp);
    // From-to combo
    static void generateBranch(codeGen &gen, Address from, Address to);

    static void generateBSR(codeGen &gen, int disp);
    static void generateRel(codeGen &gen, unsigned opcode, unsigned fcode,
			    Register src1, Register src2, Register dest,
			    bool Imm, bool do_not);
    static void generateLDA(codeGen &gen, Register rdest, Register rstart,
			    long disp, bool do_low);
    static void generateSXL(codeGen &gen, Register rdest, unsigned long amount,
			    bool left, Register rsrc);
    static void generateAddress(codeGen &gen, Register rdest, const Address &addr,
				int &remainder);
    static void generateIntegerOp(codeGen &gen, Register src1, Register src2,
				  Register dest, opCode op, bool Imm);

    // We need instruction::size() all _over_ the place.
    static unsigned size() { return sizeof(instructUnion); } 

    Address getBranchOffset() const;
    void setBranchOffset(Address newOffset);
    Address getTarget(Address origAddr) const;

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
  
  // Local version
  bool isInsnType(const unsigned mask, const unsigned match) const { 
      return ((insn_.raw & mask) == match);
  }

  bool isJmp() const {
      return ((insn_.mem_jmp.opcode == OP_MEM_BRANCH) &&
              ((insn_.mem_jmp.ext == MD_JMP) ||
               (insn_.mem_jmp.ext == MD_JSR) ||
               (insn_.mem_jmp.ext == MD_RET) ||
               (insn_.mem_jmp.ext == MD_JSR_CO)));
  }

  bool isBranch() const {
      return ((insn_.branch.opcode == OP_BSR) ||
              (insn_.branch.opcode == OP_BR) ||
              (insn_.branch.opcode == OP_BLBC) ||
              (insn_.branch.opcode == OP_BLBS) ||
              (insn_.branch.opcode == OP_FBEQ) ||
              (insn_.branch.opcode == OP_FBNE) ||
              (insn_.branch.opcode == OP_BEQ) ||
              (insn_.branch.opcode == OP_BNE) ||
              (insn_.branch.opcode == OP_FBLT) ||
              (insn_.branch.opcode == OP_FBGE) ||
              (insn_.branch.opcode == OP_BLT) ||
              (insn_.branch.opcode == OP_BGE) ||
              (insn_.branch.opcode == OP_FBLE) ||
              (insn_.branch.opcode == OP_FBGT) ||
              (insn_.branch.opcode == OP_BLE) ||
              (insn_.branch.opcode == OP_BGT));
  }

  bool isBsr() const {
      return (insn_.branch.opcode == OP_BSR);
  }
  
  bool isJsr() const {
      return ((insn_.mem_jmp.opcode == OP_MEM_BRANCH) &&
              (insn_.mem_jmp.ext == MD_JSR));
  }
  // align to word boundaries
  static bool isAligned(const Address addr) {
      return(!(addr & 0x3));
  }
  
  
  bool valid() const;


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

  bool isCall() const;

  bool isCondBranch() const;
  bool isUncondBranch() const;
  
  bool isReturn() const;

#if 0
  // Copied from arch-x86... implement as needed
  bool isCallIndir() const { return (type_ & IS_CALL) && (type_ & INDIR); }
  bool isReturn() const { return (type_ & IS_RET) || (type_ & IS_RETF); }
  bool isRetFar() const { return type_ & IS_RETF; }
  bool isJumpIndir() const { return (type_ & IS_JUMP) && (type_ & INDIR); }
  bool isJumpDir() const
    { return ~(type_ & INDIR) && ((type_ & IS_JUMP) || (type_ & IS_JCC)); }
  bool isUncondJump() const
    { return ((type_ & IS_JUMP) && !(type_ & IS_JCC)); }
  bool isIndir() const { return type_ & INDIR; }
  bool isIllegal() const { return type_ & ILLEGAL; }
  bool isLeave() const { return *ptr_ == 0xC9; }  
  bool isMoveRegMemToRegMem() const 
    { const unsigned char* p = op_ptr_ ? op_ptr_ : ptr_;
      return *p == MOV_R8_TO_RM8   || *p == MOV_R16_TO_RM16 ||
             *p == MOV_R32_TO_RM32 || *p ==  MOV_RM8_TO_R8  ||
             *p == MOV_RM16_TO_R16 || *p == MOV_RM32_TO_R32;   }
  bool isXORRegMemRegMem() const
      { const unsigned char* p = op_ptr_ ? op_ptr_ : ptr_;
        return *p == XOR_RM16_R16 || *p ==  XOR_RM32_R32 ||
               *p ==  XOR_R8_RM8  || *p ==  XOR_R16_RM16 ||
               *p == XOR_R32_RM32; }
  bool isANearBranch() const { return isJumpDir(); }

  bool isTrueCallInsn() const { return (isCall() && !isCallIndir()); }
#endif

 private:
  instructUnion insn_;
};


#endif


