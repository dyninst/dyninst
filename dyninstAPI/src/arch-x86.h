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

// $Id: arch-x86.h,v 1.18 2004/04/01 00:48:19 lharris Exp $
// x86 instruction declarations

#if !(defined(i386_unknown_solaris2_5) || defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0))
#error "invalid architecture-os inclusion"
#endif

#ifndef _ARCH_X86_H
#define _ARCH_X86_H

#if defined(i386_unknown_nt4_0)
// disable VC++ warning C4800: (performance warning)
// forcing 'unsigned int' value to bool 'true' or 'false'
#pragma warning (disable : 4800)
#endif

/* operand types */
typedef char byte_t;   /* a byte operand */
typedef short word_t;  /* a word (16-bit) operand */
typedef int dword_t;   /* a double word (32-bit) operand */

/* operand sizes */
#define byteSzB (1)    /* size of a byte operand */
#define wordSzB (2)    /* size of a word operand */
#define dwordSzB (4)   /* size of a dword operand */
#define qwordSzB (8)   /* size of a qword operand */
#define dqwordSzB (16)   /* size of a double qword (oword) operand */

/* The following values are or'ed together to form an instruction type descriptor */
/* the instruction types of interest */
#define IS_CALL (1<<1)   /* call instruction */
#define IS_RET  (1<<2)   /* near return instruction */
#define IS_RETF (1<<3)   /* far return instruction */
#define IS_JUMP (1<<4)   /* jump instruction */
#define IS_JCC  (1<<5)   /* conditional jump instruction */
#define ILLEGAL (1<<6)   /* illegal instruction */

/* addressing modes for calls and jumps */
#define REL_B   (1<<10)  /* relative address, byte offset */
#define REL_W   (1<<11)  /* relative address, word offset */
#define REL_D   (1<<12)  /* relative address, dword offset */
#define REL_X   (1<<13)  /* relative address, word or dword offset */
#define INDIR   (1<<14)  /* indirect (register or memory) address */
#define PTR_WW  (1<<15)  /* 4-byte pointer */
#define PTR_WD  (1<<16)  /* 6-byte pointer */
#define PTR_WX  (1<<17)  /* 4 or 6-byte pointer */

/* prefixes */
#define PREFIX_INST (1<<20) /* instruction prefix */
#define PREFIX_SEG  (1<<21) /* segment override prefix */
#define PREFIX_OPR  (1<<22) /* operand size override */
#define PREFIX_ADDR (1<<23) /* address size override */

/* end of instruction type descriptor values */


/* opcodes of some instructions */
#define PUSHAD   (0x60)
#define PUSHFD   (0x9C)
#define PUSHEBP  (0x55)
#define POPAD    (0x61)
#define POPFD    (0x9D)
#define PUSH_DS  (0x1E)
#define POP_DS   (0X1F)
#define POP_EAX  (0x58)
#define POP_EBX  (0x5b)
#define POP_EBP  (0x5d)
#define NOP      (0x90)

#define JCXZ     (0xE3)

#define JE_R8    (0x74)
#define JNE_R8   (0x75)
#define JL_R8    (0x7C)
#define JLE_R8   (0x7E)
#define JG_R8    (0x7F)
#define JGE_R8   (0x7D)

#define FSAVE    (0x9BDD)
#define FSAVE_OP (6)

#define FRSTOR   (0xDD)
#define FRSTOR_OP (4)

#define MOVREGMEM_REG (0x8b)

/* limits */
#define MIN_IMM8 (-128)
#define MAX_IMM8 (127)
#define MIN_IMM16 (-32768)
#define MAX_IMM16 (32767)

// Size of floating point information saved by FSAVE
#define FSAVE_STATE_SIZE 108

enum dynamic_call_address_mode {
  REGISTER_DIRECT, REGISTER_INDIRECT,
  REGISTER_INDIRECT_DISPLACED, SIB, DISPLACED
};

/*
   get_instruction: get the instruction that starts at instr.
   return the size of the instruction and set instType to a type descriptor
*/
unsigned get_instruction(const unsigned char *instr, unsigned &instType);

/* get the target of a jump or call */
Address get_target(const unsigned char *instr, unsigned type, unsigned size,
                Address addr);


class instruction {
 public:
  instruction(): type_(0), size_(0), ptr_(0) {}

  instruction(const unsigned char *p, unsigned type, unsigned sz):
    type_(type), size_(sz), ptr_(p) {}

  instruction(const instruction &insn) {
    type_ = insn.type_;
    size_ = insn.size_;
    ptr_ = insn.ptr_;
  }

  unsigned getNextInstruction(const unsigned char *p) {
    ptr_ = p;
    size_ = get_instruction(ptr_, type_);
    return size_;
  }

  // if the instruction is a jump or call, return the target, else return zero
  Address getTarget(Address addr) const { 
    return (Address)get_target(ptr_, type_, size_, addr); 
  }

  // return the size of the instruction in bytes
  unsigned size() const { return size_; }

  // return the type of the instruction
  unsigned type() const { return type_; }

  // return a pointer to the instruction
  const unsigned char *ptr() const { return ptr_; }

  bool isCall() const { return type_ & IS_CALL; }
  bool isCallIndir() const { return (type_ & IS_CALL) && (type_ & INDIR); }
  bool isReturn() const { return (type_ & IS_RET) || (type_ & IS_RETF); }
  bool isRetFar() const { return type_ & IS_RETF; }
  bool isJumpIndir() const { return (type_ & IS_JUMP) && (type_ & INDIR); }
  bool isJumpDir() const
    { return ~(type_ & INDIR) && ((type_ & IS_JUMP) || (type_ & IS_JCC)); }
  bool isNop() const { return *ptr_ == 0x90; }
  bool isIndir() const { return type_ & INDIR; }
  bool isIllegal() const { return type_ & ILLEGAL; }
  bool isLeave() const { return *ptr_ == 0xC9; }
  bool isFrameAlloc() const { return (*ptr_ == 0x89 && *(ptr_+1) == 0xe5); }

 private:
  unsigned type_;   // type of the instruction (e.g. IS_CALL | INDIR)
  unsigned size_;   // size in bytes
  const unsigned char *ptr_;       // pointer to the instruction
};


int get_disp(instruction *insn);
int set_disp(bool setDisp, instruction *insn, int newOffset, bool outOfFunc);
int displacement(const unsigned char *instr, unsigned type);

int sizeOfMachineInsn(instruction *insn);
int addressOfMachineInsn(instruction *insn);

int get_instruction_operand(const unsigned char *i_ptr, Register& base_reg,
			    Register& index_reg, int& displacement, 
			    unsigned& scale, unsigned &mod);
void decode_SIB(unsigned sib, unsigned& scale, Register& index_reg, Register& base_reg);

/* addresses on x86 don't have to be aligned */
inline bool isAligned(const Address ) { return true; }

/* Address bounds of new dynamic heap segments.  On x86 we don't try
to allocate new segments near base tramps, so heap segments can be
allocated anywhere (the tramp address "x" is ignored). */
inline Address region_lo(const Address /*x*/) { return 0x00000000; }
inline Address region_hi(const Address /*x*/) { return 0xf0000000; }

bool insn_hasSIB(unsigned,unsigned&,unsigned&,unsigned&);
bool insn_hasDisp8(unsigned ModRM);
bool insn_hasDisp32(unsigned ModRM);

#endif
