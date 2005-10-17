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

// $Id: arch-x86.h,v 1.36 2005/10/17 18:19:43 rutar Exp $
// x86 instruction declarations

#include <stdio.h>

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
class process; // relocation may need to look up a target

#if defined(i386_unknown_nt4_0)
// disable VC++ warning C4800: (performance warning)
// forcing 'unsigned int' value to bool 'true' or 'false'
#pragma warning (disable : 4800)
#endif

/* operand types */
typedef char byte_t;   /* a byte operand */
typedef short word_t;  /* a word (16-bit) operand */
typedef int dword_t;   /* a double word (32-bit) operand */

#if defined(arch_x86_64)
#define maxGPR 16
#else
#define maxGPR 8
#endif 

#define READ_OP 0
#define WRITE_OP 1

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

 
/* opcodes of some one byte opcode instructions */
/* ADD */
#define ADD_EB_GB (0x00)
#define ADD_EV_GV (0x01)
#define ADD_GB_EB (0x02)
#define ADD_GV_EV (0x03)
#define ADD_AL_LB (0x04)
#define ADD_RAX_LZ (0x05)

#define PUSHES    (0x06) /* Invalid in 64 bit mode */
#define POPES     (0x07) /* Invalid in 64 bit mode */

/* OR */
#define OR_EB_GB (0x08)
#define OR_EV_GV (0x09)
#define OR_GB_EB (0x0A)
#define OR_GV_EV (0x0B)
#define OR_AL_LB (0x0C)
#define OR_RAX_LZ (0x0D)

#define PUSHCS    (0x0E) /* Invalid in 64 bit mode */
#define TWO_BYTE_OPCODE (0x0F)

/* ADC */
#define ADC_EB_GB (0x10)
#define ADC_EV_GV (0x11)
#define ADC_GB_EB (0x12)
#define ADC_GV_EV (0x13)
#define ADC_AL_LB (0x14)
#define ADC_RAX_LZ (0x15)

#define PUSHSS    (0x16) /* Invalid in 64 bit mode */
#define POPSS     (0x17) /* Invalid in 64 bit mode */

/* SBB */
#define SBB_EB_GB (0x18)
#define SBB_EV_GV (0x19)
#define SBB_GB_EB (0x1A)
#define SBB_GV_EV (0x1B)
#define SBB_AL_LB (0x1C)
#define SBB_RAX_LZ (0x1D)

#define PUSH_DS  (0x1E) /* Invalid in 64 bit mode */
#define POP_DS   (0X1F) /* Invalid in 64 bit mode */

/* AND */
#define AND_EB_GB (0x20)
#define AND_EV_GV (0x21)
#define AND_GB_EB (0x22)
#define AND_GV_EV (0x23)
#define AND_AL_LB (0x24)
#define AND_RAX_LZ (0x25)

#define SEG_ES (0x26) /* Null prefix in 64-bit mode */
#define DAA    (0x27) /* Invalid in 64-bit mode */

/* SUB */
#define SUB_EB_GB (0x28)
#define SUB_EV_GV (0x29)
#define SUB_GB_EB (0x2A)
#define SUB_GV_EV (0x2B)
#define SUB_AL_LB (0x2C)
#define SUB_RAX_LZ (0x2D)

//(0x2E)
//   (0x2F)

/* XOR */
#define XOR_EB_GB (0x30)
#define XOR_EV_GV (0x31)
#define XOR_GB_EB (0x32)
#define XOR_GV_EV (0x33)
#define XOR_AL_LB (0x34)
#define XOR_RAX_LZ (0x35)

#define XOR_RM16_R16 (0x31)
#define XOR_RM32_R32 (0x31)
#define XOR_R8_RM8 (0x32)
#define XOR_R16_RM16 (0x33)
#define XOR_R32_RM32 (0x33)

#define SEG_SS (0x36) /* Null prefix in 64 bit mode */
#define AAA (0x37)    /* Invalid in 64-bit mode */


/* CMP */
#define CMP_EB_GB (0x38)
#define CMP_EV_GV (0x39)
#define CMP_GB_EB (0x3A)
#define CMP_GV_EV (0x3B)
#define CMP_AL_LB (0x3C)
#define CMP_RAX_LZ (0x3D)

//   (0x3E)
//   (0x3F)

/* INC - REX Prefixes in 64 bit mode*/
#define INC_EAX  (0x40)
#define INC_ECX  (0x41)
#define INC_EDX  (0x42)
#define INC_EBX  (0x43)
#define INC_ESP  (0x44)
#define INC_EBP  (0x45)
#define INC_ESI  (0x46)
#define INC_EDI  (0x47)

/* DEC - REX Prefixes in 64 bit mode */
#define DEC_EAX  (0x48)
#define DEC_ECX  (0x49)
#define DEC_EDX  (0x50)
#define DEC_EBX  (0x51)
#define DEC_ESP  (0x52)
#define DEC_EBP  (0x53)
#define DEC_ESI  (0x54)
#define DEC_EDI  (0x55)

/* PUSH */
#define PUSHEAX  (0x50)
#define PUSHECX  (0x51)
#define PUSHEDX  (0x52)
#define PUSHEBX  (0x53)
#define PUSHESP  (0x54)
#define PUSHEBP  (0x55)
#define PUSHESI  (0x56)
#define PUSHEDI  (0x57)

/* POP */
#define POP_EAX  (0x58)
#define POP_ECX  (0x59)
#define POP_EDX  (0x5A)
#define POP_EBX  (0x5b)
#define POP_ESP  (0x5c)
#define POP_EBP  (0x5d)
#define POP_EBI  (0x5e)
#define POP_EDI  (0x5f)


#define PUSHAD   (0x60)
#define POPAD    (0x61)




#define JE_R8    (0x74)
#define JNE_R8   (0x75)
#define JL_R8    (0x7C)
#define JLE_R8   (0x7E)
#define JG_R8    (0x7F)
#define JGE_R8   (0x7D)


#define MOVREGMEM_REG (0x8b) 
#define MOV_R8_TO_RM8 (0x88)     //move r8 to r/m8
#define MOV_R16_TO_RM16 (0x89)   //move r16 to r/m16
#define MOV_R32_TO_RM32 (0x89)   //move r32 to r/m32
#define MOV_RM8_TO_R8 (0x8A)
#define MOV_RM16_TO_R16 (0x8b)
#define MOV_RM32_TO_R32 (0x8b)



#define NOP      (0x90)
#define PUSHFD   (0x9C)
#define POPFD    (0x9D)


#define JCXZ     (0xE3)






#define FSAVE    (0x9BDD)
#define FSAVE_OP (6)

#define FRSTOR   (0xDD)
#define FRSTOR_OP (4)

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

// addressing methods (see appendix A-2)
// I've added am_reg (for registers implicitely encoded in instruciton), 
// and am_stackX for stack operands [this kinda' messy since there are actually two operands:
// the stack byte/word/dword and the (E)SP register itself - but is better than naught]
// added: am_reg, am_stack, am_allgprs
enum { am_A=1, am_C, am_D, am_E, am_F, am_G, am_I, am_J, am_M, am_O, // 10
       am_P, am_Q, am_R, am_S, am_T, am_V, am_W, am_X, am_Y, am_reg, // 20
       am_stackH, am_stackP, am_allgprs }; // pusH and poP produce different addresses

// operand types - idem, but I invented quite a few to make implicit operands explicit.
enum { op_a=1, op_b, op_c, op_d, op_dq, op_p, op_pd, op_pi, op_ps, // 9 
       op_q, op_s, op_sd, op_ss, op_si, op_v, op_w, op_z, op_lea, op_allgprs, op_512 };

// registers [only fancy names, not used right now]
enum { r_AH=100, r_BH, r_CH, r_DH, r_AL, r_BL, r_CL, r_DL, //107
       r_AX, r_DX, //109
       r_eAX, r_eBX, r_eCX, r_eDX, //113
       r_EAX, r_EBX, r_ECX, r_EDX, //117
       r_CS, r_DS, r_ES, r_FS, r_GS, r_SS, //123
       r_eSP, r_eBP, r_eSI, r_eDI, //127
       r_ESP, r_EBP, r_ESI, r_EDI, //131
       r_EDXEAX, r_ECXEBX }; //133
// last two are hacks for cmpxch8b which would have 5 operands otherwise!!!



// registers used for memory access
enum { mRAX=0, mRCX, mRDX, mRBX,
       mRSP, mRBP, mRSI, mRDI,
       mR8, mR9, mR10, mR11, 
       mR12,mR13, MR14, mR15 };

enum { mEAX=0, mECX, mEDX, mEBX,
       mESP, mEBP, mESI, mEDI };

enum { mAX=0, mCX, mDX, mBX,
       mSP, mBP, mSI, mDI };


// operand semantic - these make explicit all the implicit stuff in the Intel tables
// they are needed for memory access, but may be useful for other things: dataflow etc.
// Instructions that do not deal with memory are not tested, so caveat emptor...
// Also note that the stack is never specified as an operand in Intel tables, so it
// has to be dealt with here.

enum { sNONE=0, // the instruction does something that cannot be classified as read/write (by me)
       s1R,     // reads one operand, e.g. jumps
       s1W,     // e.g. lea
       s1RW,    // one operand read and written, e.g. inc
       s1R2R,   // reads two operands, e.g. cmp
       s1W2R,   // second operand read, first operand written (e.g. mov)
       s1RW2R,  // two operands read, first written (e.g. add)
       s1RW2RW, // e.g. xchg
       s1W2R3R, // e.g. imul
       s1W2W3R, // e.g. les
       s1W2RW3R, // some mul
       s1W2R3RW, // (stack) push & pop
       s1RW2R3R, // shld/shrd
       s1RW2RW3R, // [i]div, cmpxch8b
}; // should be no more than 2^16 otherwise adjust FPOS below


struct modRMByte {
  unsigned mod : 2;
  unsigned reg : 3;
  unsigned rm  : 3;
};

struct sIBByte {
  unsigned scale : 2;
  unsigned index : 3;
  unsigned base  : 3;
};


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
    printf("Initialize Mac\n");
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


struct ia32_operand {  // operand as given in Intel book tables
  unsigned int admet;  // addressing method
  unsigned int optype; // operand type;
};


// An instruction table entry
struct ia32_entry {
  char *name;                // name of the instruction (for debbuging only)
  unsigned int otable;       // which opcode table is next; if t_done it is the current one
  unsigned char tabidx;      // at what index to look, 0 if it easy to deduce from opcode
  bool hasModRM;             // true if the instruction has a MOD/RM byte
  ia32_operand operands[3];  // operand descriptors
  unsigned int legacyType;   // legacy type of the instruction (e.g. (IS_CALL | REL_W))
  // code to decode memory access - this field should be seen as two 16 bit fields
  // the lower half gives operand semantics, e.g. s1RW2R, the upper half is a fXXX hack if needed
  // before hating me for this: it takes a LOT less time to add ONE field to ~2000 table lines!
  unsigned int opsema;  
};



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
  ia32_entry     *entry;
  unsigned int   legacy_type;
  bool           rip_relative_data;


 public:
  ia32_instruction(ia32_memacc* _mac = NULL, ia32_condition* _cnd = NULL)
    : mac(_mac), cond(_cnd), rip_relative_data(false) {}

  ia32_entry * getEntry() { return entry; }
  unsigned int getSize() const { return size; }
  unsigned int getPrefixCount() const { return prf.getCount(); }
  ia32_prefixes * getPrefix() { return &prf; }
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

#if defined(arch_x86_64)
// size of instruction seqeunce to get anywhere in address space
// without touching any registers
#define JUMP_ABS64_SZ (17)
#endif

#define PUSH_RM_OPC1 (0xFF)
#define PUSH_RM_OPC2 (6)
#define CALL_RM_OPC1 (0xFF)
#define CALL_RM_OPC2 (2)
#define PUSH_EBP (0x50+REGNUM_EBP)
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

  // And the size necessary to reproduce this instruction
  // at some random point.
  unsigned spaceToRelocate() const;

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
  
  // Function relocation...
  static void generateInterFunctionBranch(codeGen &gen, Address from, Address to) { generateBranch(gen, from, to); }
  static unsigned maxInterFunctionJumpSize() { return maxJumpSize(); }

  // And tell us how much space we'll need...
  static int jumpSize(Address from, Address to);
  static int jumpSize(int disp);
  static unsigned maxJumpSize();


  // We may want to generate an efficient set 'o nops
  static void generateNOOP(codeGen &gen, unsigned size = 1);
  
  static void generateIllegal(codeGen &gen);
  static void generateTrap(codeGen &gen);

  void generate(codeGen &gen);

  // And generate an equivalent stream somewhere else...
  // fallthroughOverride and targetOverride are used for
  // making the behavior of jumps change. It won't work for 
  // jumptables; that should be cleared up sometime.
  bool generate(codeGen &gen,
                process *proc,
                Address origAddr,
                Address newAddr,
                Address fallthroughOverride = 0,
                Address targetOverride = 0);

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
#if defined(arch_x86_64)
inline bool is_disp32(long disp) {
  return (disp <= I32_MAX && disp >= I32_MIN);
}
inline bool is_disp32(Address a1, Address a2) {
  return is_disp32(a2 - (a1 + JUMP_REL32_SZ));
}
#endif

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
