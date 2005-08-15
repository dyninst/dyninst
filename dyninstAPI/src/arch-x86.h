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

// $Id: arch-x86.h,v 1.31 2005/08/15 22:20:07 bernat Exp $
// x86 instruction declarations

#if !defined(i386_unknown_solaris2_5) \
 && !defined(i386_unknown_nt4_0) \
 && !defined(i386_unknown_linux2_0) \
 && !defined(x86_64_unknown_linux2_4)
#error "invalid architecture-os inclusion"
#endif

#ifndef _ARCH_X86_H
#define _ARCH_X86_H

// Code generation

typedef unsigned char codeBuf_t;
typedef unsigned codeBufIndex_t;

class codeGen;

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
#define REL_D_DATA (1<<18) /* AMD64 RIP-relative data addressing */

/* prefixes */
#define PREFIX_INST   (1<<20) /* instruction prefix */
#define PREFIX_SEG    (1<<21) /* segment override prefix */
#define PREFIX_OPR    (1<<22) /* operand size override */
#define PREFIX_ADDR   (1<<23) /* address size override */
#define PREFIX_REX    (1<<24) /* AMD64 REX prefix */
#define PREFIX_OPCODE (1<<25) /* prefix is part of opcode (SSE) */

/* end of instruction type descriptor values */

 
/* opcodes of some instructions */
#define PUSHAD   (0x60)
#define PUSHFD   (0x9C)
#define PUSHEBP  (0x55)
#define PUSHESI  (0x56)
#define PUSHESP  (0x54)
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
#define MOV_R8_TO_RM8 (0x88)     //move r8 to r/m8
#define MOV_R16_TO_RM16 (0x89)   //move r16 to r/m16
#define MOV_R32_TO_RM32 (0x89)   //move r32 to r/m32
#define MOV_RM8_TO_R8 (0x8A)
#define MOV_RM16_TO_R16 (0x8b)
#define MOV_RM32_TO_R32 (0x8b)

#define XOR_RM16_R16 (0x31)
#define XOR_RM32_R32 (0x31)
#define XOR_R8_RM8 (0x32)
#define XOR_R16_RM16 (0x33)
#define XOR_R32_RM32 (0x33)

/* limits */
#define MIN_IMM8 (-128)
#define MAX_IMM8 (127)
#define MIN_IMM16 (-32768)
#define MAX_IMM16 (32767)

// Size of floating point information saved by FSAVE
#define FSAVE_STATE_SIZE 108


#define PREFIX_LOCK   0xF0
#define PREFIX_REPNZ  0xF2
#define PREFIX_REP    0xF3

#define PREFIX_SEGCS  0x2E
#define PREFIX_SEGSS  0x36
#define PREFIX_SEGDS  0x3E
#define PREFIX_SEGES  0x26
#define PREFIX_SEGFS  0x64
#define PREFIX_SEGGS  0x65

#define PREFIX_BRANCH0 0x2E
#define PREFIX_BRANCH1 0x3E

#define PREFIX_SZOPER  0x66
#define PREFIX_SZADDR  0x67

void ia32_set_mode_64(bool mode);
bool ia32_is_mode_64();

class ia32_prefixes
{
  friend bool ia32_decode_prefixes(const unsigned char* addr, ia32_prefixes&);
  friend bool ia32_decode_rex(const unsigned char* addr, ia32_prefixes&);
 private:
  unsigned int count;
  // At most 4 prefixes are allowed for Intel 32-bit CPUs
  // There also 4 groups, so this array is 0 if no prefix
  // from that group is present, otherwise it contains the
  // prefix opcode
  // For 64-bit CPUs, an additional REX prefix is possible,
  // so this array is extended to 5 elements
  unsigned char prfx[5];
  unsigned char opcode_prefix;
 public:
  unsigned int const getCount() const { return count; }
  unsigned char getPrefix(unsigned char group) const { return prfx[group]; }
  bool rexW() const { return prfx[4] & 0x8; }
  bool rexR() const { return prfx[4] & 0x4; }
  bool rexX() const { return prfx[4] & 0x2; }
  bool rexB() const { return prfx[4] & 0x1; }
  unsigned char getOpcodePrefix() const { return opcode_prefix; }
  unsigned char getAddrSzPrefix() const { return prfx[3]; }
  unsigned char getOperSzPrefix() const { return prfx[2]; }
};

//VG(6/20/02): To support Paradyn without forcing it to include BPatch_memoryAccess, we
//             define this IA-32 "specific" class to encapsulate the same info - yuck

struct ia32_memacc
{
  bool is;
  bool read;
  bool write;
  bool nt;     // non-temporal, e.g. movntq...
  bool prefetch;

  bool addr32; // true if 32-bit addressing, false otherwise
  int imm;
  int scale;
  int regs[2]; // register encodings (in ISA order): 0-7
               // (E)AX, (E)CX, (E)DX, (E)BX
               // (E)SP, (E)BP, (E)SI, (E)DI

  int size;
  int sizehack;  // register (E)CX or string based
  int prefetchlvl; // prefetch level
  int prefetchstt; // prefetch state (AMD)

  ia32_memacc() : is(false), read(false), write(false), nt(false), 
       prefetch(false), addr32(true), imm(0), scale(0), size(0), sizehack(0),
       prefetchlvl(-1), prefetchstt(-1)
  {
    regs[0] = -1;
    regs[1] = -1;
  }

  void set16(int reg0, int reg1, int disp)
  { 
    is = true;
    addr32  = false; 
    regs[0] = reg0; 
    regs[1] = reg1; 
    imm     = disp;
  }

  void set32(int reg, int disp)
  { 
    is = true;
    regs[0] = reg; 
    imm     = disp;
  }

  void set32sib(int base, int scal, int indx, int disp)
  {
    is = true;
    regs[0] = base;
    regs[1] = indx;
    scale   = scal;
    imm     = disp;
  }

  void setXY(int reg, int _size, int _addr32)
  {
    is = true;
    regs[0] = reg;
    size = _size;
    addr32 = _addr32;
  }
};


enum sizehacks {
  shREP=1,
  shREPECMPS,
  shREPESCAS,
  shREPNECMPS,
  shREPNESCAS
};


struct ia32_condition
{
  bool is;
  // TODO: add a field/hack for ECX [not needed for CMOVcc, but for Jcc]
  int tttn;

  ia32_condition() : is(false) {}
  void set(int _tttn) { is = true; tttn = _tttn; }
};

bool ia32_decode_prefixes(const unsigned char* addr, ia32_prefixes&);


struct ia32_entry;

class ia32_instruction
{
  friend unsigned int ia32_decode_operands (const ia32_prefixes& pref, const ia32_entry& gotit, 
                                            const char* addr, ia32_instruction& instruct);
#if defined(i386_unknown_nt4_0) && _MSC_VER < 1300
  friend ia32_instruction& ia32_decode(unsigned int capa, const unsigned char* addr,
		  		       ia32_instruction& instruct);
#else
  template <unsigned int capa>
    friend ia32_instruction& ia32_decode(const unsigned char* addr, ia32_instruction& instruct);
#endif
  friend unsigned int ia32_decode_operands (const ia32_prefixes& pref, const ia32_entry& gotit, 
                                            const unsigned char* addr, ia32_instruction& instruct,
                                            ia32_memacc *mac = NULL);
  friend ia32_instruction& ia32_decode_FP(const ia32_prefixes& pref, const unsigned char* addr,
                                          ia32_instruction& instruct);
  friend unsigned int ia32_emulate_old_type(ia32_instruction& instruct);
  friend ia32_instruction& ia32_decode_FP(unsigned int opcode, 
                                          const ia32_prefixes& pref,
                                          const unsigned char* addr, 
                                          ia32_instruction& instruct,
                                          ia32_memacc *mac = NULL);

  unsigned int   size;
  ia32_prefixes  prf;
  ia32_memacc    *mac;
  ia32_condition *cond;
  unsigned int   legacy_type;
  bool           rip_relative_data;


 public:
  ia32_instruction(ia32_memacc* _mac = NULL, ia32_condition* _cnd = NULL)
    : mac(_mac), cond(_cnd), rip_relative_data(false) {}

  unsigned int getSize() const { return size; }
  unsigned int getPrefixCount() const { return prf.getCount(); }
  unsigned int getLegacyType() const { return legacy_type; }
  bool hasRipRelativeData() const { return rip_relative_data; }
  const ia32_memacc& getMac(int which) const { return mac[which]; }
  const ia32_condition& getCond() const { return *cond; }
};

// VG(02/07/2002): Information that the decoder can return is
//   #defined below. The decoder always returns the size of the 
//   instruction because that has to be determined anyway.
//   Please don't add things that should be external to the
//   decoder, e.g.: how may bytes a relocated instruction needs
//   IMHO that stuff should go into inst-x86...

#define IA32_DECODE_PREFIXES	(1<<0)
#define IA32_DECODE_MNEMONICS	(1<<1)
#define IA32_DECODE_OPERANDS	(1<<2)
#define IA32_DECODE_JMPS	(1<<3)
#define IA32_DECODE_MEMACCESS	(1<<4)
#define IA32_DECODE_CONDITION 	(1<<5)

#define IA32_FULL_DECODER 0xffffffffffffffffu
#define IA32_SIZE_DECODER 0

// old broken MS compiler cannot do this properly, so we revert to args
#if defined(i386_unknown_nt4_0) && _MSC_VER < 1300

ia32_instruction& ia32_decode(unsigned int capabilities, const unsigned char* addr, ia32_instruction&);

#else

template <unsigned int capabilities>
ia32_instruction& ia32_decode(const unsigned char* addr, ia32_instruction&);
// If typing the template every time is a pain, the following should help:
#define ia32_decode_all  ia32_decode<IA32_FULL_DECODER>
#define ia32_decode_size ia32_decode<IA32_SIZE_DECODER>
#define ia32_size(a,i)   ia32_decode_size((a),(i)).size

#endif

enum dynamic_call_address_mode {
  REGISTER_DIRECT, REGISTER_INDIRECT,
  REGISTER_INDIRECT_DISPLACED, SIB, DISPLACED
};

/*
   get_instruction: get the instruction that starts at instr.
   return the size of the instruction and set instType to a type descriptor
*/
unsigned get_instruction(const unsigned char *instr, unsigned &instType,
			 const unsigned char** op_ptr = NULL);

/* get the target of a jump or call */
Address get_target(const unsigned char *instr, unsigned type, unsigned size,
		   Address addr);


// Size of a jump rel32 instruction
#define JUMP_REL32_SZ (5)
#define JUMP_SZ (5)
// Size of a call rel32 instruction
#define CALL_REL32_SZ (5)

#define PUSH_RM_OPC1 (0xFF)
#define PUSH_RM_OPC2 (6)
#define CALL_RM_OPC1 (0xFF)
#define CALL_RM_OPC2 (2)
#define PUSH_EBP (0x50+EBP)
#define SUB_REG_IMM32 (5)
#define LEAVE (0xC9)

class instruction {
 public:
    instruction(): type_(0), size_(0), ptr_(0), op_ptr_(0) {}

  instruction(const unsigned char *p, unsigned type, unsigned sz, const unsigned char* op = 0):
      type_(type), size_(sz), ptr_(p), op_ptr_(op ? op : p) {}

  instruction(const instruction &insn) {
    type_ = insn.type_;
    size_ = insn.size_;
    ptr_ = insn.ptr_;
    op_ptr_ = insn.op_ptr_;
  }
  
  instruction *copy() const;

  instruction(const void *ptr) :
      type_(0), size_(0), ptr_(NULL), op_ptr_(0) {
      setInstruction((const unsigned char*)ptr);
  }

  unsigned setInstruction(const unsigned char *p, Address = 0) {
      ptr_ = p;
      size_ = get_instruction(ptr_, type_, &op_ptr_);
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
  const unsigned char *ptr() const { 
      return ptr_; 
  }

  // return a pointer to the instruction's opcode
  const unsigned char* op_ptr() const { return op_ptr_; }

  // Code generation
  static void generateBranch(codeGen &gen, Address from, Address to); 
  static void generateBranch(codeGen &gen, int disp); 
  static void generateCall(codeGen &gen, Address from, Address to);

  // We may want to generate an efficient set 'o nops
  static void generateNOOP(codeGen &gen, unsigned size);
  
  static void generateIllegal(codeGen &gen);
  static void generateTrap(codeGen &gen);

  void generate(codeGen &gen);

  bool isCall() const { return type_ & IS_CALL; }
  bool isCallIndir() const { return (type_ & IS_CALL) && (type_ & INDIR); }
  bool isReturn() const { return (type_ & IS_RET) || (type_ & IS_RETF); }
  bool isRetFar() const { return type_ & IS_RETF; }
  bool isJumpIndir() const { return (type_ & IS_JUMP) && (type_ & INDIR); }
  bool isJumpDir() const
    { return ~(type_ & INDIR) && ((type_ & IS_JUMP) || (type_ & IS_JCC)); }
  bool isUncondJump() const
    { return ((type_ & IS_JUMP) && !(type_ & IS_JCC)); }
  bool isNop() const { return *ptr_ == 0x90; }
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

  static bool isAligned(const Address ) { return true; }

 private:
  unsigned type_;   // type of the instruction (e.g. IS_CALL | INDIR)
  unsigned size_;   // size in bytes
  const unsigned char *ptr_;       // pointer to the instruction
  const unsigned char *op_ptr_;    // pointer to the opcode
};


int get_disp(instruction *insn);
int set_disp(bool setDisp, instruction *insn, int newOffset, bool outOfFunc);
int displacement(const unsigned char *instr, unsigned type);

int sizeOfMachineInsn(instruction *insn);
long addressOfMachineInsn(instruction *insn);

int get_instruction_operand(const unsigned char *i_ptr, Register& base_reg,
			    Register& index_reg, int& displacement, 
			    unsigned& scale, unsigned &mod);
void decode_SIB(unsigned sib, unsigned& scale, Register& index_reg, Register& base_reg);
const unsigned char* skip_headers(const unsigned char*, ia32_prefixes* = NULL);

/* addresses on x86 don't have to be aligned */
/* Address bounds of new dynamic heap segments.  On x86 we don't try
to allocate new segments near base tramps, so heap segments can be
allocated anywhere (the tramp address "x" is ignored). */
inline Address region_lo(const Address /*x*/) { return 0x00000000; }
inline Address region_hi(const Address /*x*/) { return 0xf0000000; }

#if defined(arch_x86_64)
// range functions for AMD64
inline Address region_lo_64(const Address x) { return x & 0xffffffff80000000; }
inline Address region_hi_64(const Address x) { return x | 0x000000007fffffff; }
#endif

bool insn_hasSIB(unsigned,unsigned&,unsigned&,unsigned&);
bool insn_hasDisp8(unsigned ModRM);
bool insn_hasDisp32(unsigned ModRM);
bool isFunctionPrologue( instruction& insn1 );
bool isStackFramePreamble( instruction& insn );

#endif
