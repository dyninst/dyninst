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

union instructUnion {
  struct if_PAL          pal;
  struct if_Branch       branch;
  struct if_Mem          mem;
  struct if_Mem_jmp      mem_jmp;
  struct if_Operate      oper;
  struct if_Operate_lit  oper_lit;
  unsigned               raw;
};

typedef union instructUnion instruction;
#define INSN_SIZE	sizeof(instruction)

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

inline bool isInsnType(const instruction i,
                       const unsigned mask,
                       const unsigned match) {
  return ((i.raw & mask) == match);
}

inline bool isJmpType(const instruction& insn) {
  return ((insn.mem_jmp.opcode == OP_MEM_BRANCH) &&
	  ((insn.mem_jmp.ext == MD_JMP) ||
	   (insn.mem_jmp.ext == MD_JSR) ||
	   (insn.mem_jmp.ext == MD_RET) ||
	   (insn.mem_jmp.ext == MD_JSR_CO)));
}

inline bool isBranchType(const instruction& insn) {
  return ((insn.branch.opcode == OP_BSR) ||
          (insn.branch.opcode == OP_BR) ||
          (insn.branch.opcode == OP_BLBC) ||
          (insn.branch.opcode == OP_BLBS) ||
          (insn.branch.opcode == OP_FBEQ) ||
          (insn.branch.opcode == OP_FBNE) ||
          (insn.branch.opcode == OP_BEQ) ||
          (insn.branch.opcode == OP_BNE) ||
          (insn.branch.opcode == OP_FBLT) ||
          (insn.branch.opcode == OP_FBGE) ||
          (insn.branch.opcode == OP_BLT) ||
          (insn.branch.opcode == OP_BGE) ||
          (insn.branch.opcode == OP_FBLE) ||
          (insn.branch.opcode == OP_FBGT) ||
          (insn.branch.opcode == OP_BLE) ||
          (insn.branch.opcode == OP_BGT));
}

inline bool isBsr(const instruction& insn) {
  return (insn.branch.opcode == OP_BSR);
}

inline bool isJsr(const instruction& insn) {
  return ((insn.mem_jmp.opcode == OP_MEM_BRANCH) &&
	  (insn.mem_jmp.ext == MD_JSR));
}
/*
inline bool isReturnInsn(const image *owner, Address adr, bool &lastOne)
{
  instruction insn;

  insn.raw = owner->get_instruction(adr);
  // need to know if this is really the last return.
  //    for now assume one return per function - jkh 4/12/96
  lastOne = true;
  return ((insn.mem_jmp.opcode == OP_MEM_BRANCH) &&
	  (insn.mem_jmp.ext == MD_RET));
}

inline bool isCallInsn(const instruction i) {
  return (isBsr(i) || isJsr(i));
}
*/

// align to word boundaries
inline bool isAligned(const Address addr)
{
  return(!(addr & 0x3));
}


inline bool IS_VALID_INSN(const instruction& insn) {
  return (((insn.mem.opcode >= 0x20) &&
	   (insn.mem.opcode <= 0x2f)) ||
	  (insn.mem.opcode == 0x0b) ||
	  (insn.mem.opcode == 0x0f) ||
	  (insn.mem.opcode == 0x08) ||
	  (insn.mem.opcode == 0x09) ||
	  ((insn.mem.opcode == 0x18) && 
	   (insn.mem.disp == 0x0000) ||
	   (insn.mem.disp == 0x4000)) ||
	   // these are are larger than the field - jkh 12/1/98
	   // (insn.mem.disp == 0x8000) ||
	   // (insn.mem.disp == 0xe000) ||
	   // (insn.mem.disp == 0xf000) ||
	  (insn.mem.opcode == 0x1a) ||
	  ((insn.branch.opcode >= 0x30) &&
	   (insn.branch.opcode <= 0x3f)) ||
	  (insn.oper.opcode == 0x10) ||        // integer operations
	  (insn.oper.opcode == 0x11) ||
	  (insn.oper.opcode == 0x12) ||
	  (insn.oper.opcode == 0x13) ||
	  (insn.oper.opcode == 0x15) ||	       // floating point
	  (insn.oper.opcode == 0x16) ||	       // floating point
	  (insn.oper.opcode == 0x17) ||        // floating point
	  (insn.oper.opcode == 0x00));         // PAL code
}


#endif


