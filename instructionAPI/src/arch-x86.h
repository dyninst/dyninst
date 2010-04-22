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

// $Id: arch-x86.h,v 1.6 2008/10/28 18:42:41 bernat Exp $
// x86 instruction declarations

#ifndef _ARCH_X86_IAPI_H
#define _ARCH_X86_IAPI_H

#include <stdio.h>
#include <set>
#include <map>
#include <vector>
#include "../../common/h/Types.h"
#include "../h/RegisterIDs-x86.h"
#include "../h/entryIDs-IA32.h"
#include "ia32_locations.h"


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
#define PRVLGD  (1<<7)   /* privileged */
#define IS_RETC (1<<8)   /* return and pop bytes off of stack*/
#define IS_NOP  (1<<9)   /* Nop, Lea--lea is only sometime a return, be sure to double check */

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

 

/* limits */
#define MIN_IMM8 (-128)
#define MAX_IMM8 (127)
#define MIN_IMM16 (-32768)
#define MAX_IMM16 (32767)

// Size of floating point information saved by FSAVE
#define FSAVE_STATE_SIZE 108

// Prefix groups
enum {
  RepGroup = 0
};



#ifndef PREFIX_LOCK

#define PREFIX_LOCK   (unsigned char)(0xF0)
#define PREFIX_REPNZ  (unsigned char)(0xF2)
#define PREFIX_REP    (unsigned char)(0xF3)

#define PREFIX_SEGCS  (unsigned char)(0x2E)
#define PREFIX_SEGSS  (unsigned char)(0x36)
#define PREFIX_SEGDS  (unsigned char)(0x3E)
#define PREFIX_SEGES  (unsigned char)(0x26)
#define PREFIX_SEGFS  (unsigned char)(0x64)
#define PREFIX_SEGGS  (unsigned char)(0x65)

#define PREFIX_BRANCH0 (unsigned char)(0x2E)
#define PREFIX_BRANCH1 (unsigned char)(0x3E)

#define PREFIX_SZOPER  (unsigned char)(0x66)
#define PREFIX_SZADDR  (unsigned char)(0x67)
#endif


INSTRUCTION_EXPORT void ia32_set_mode_64(bool mode);
INSTRUCTION_EXPORT bool ia32_is_mode_64();

// addressing methods (see appendix A-2)
// I've added am_reg (for registers implicitely encoded in instruciton), 
// and am_stackX for stack operands [this kinda' messy since there are actually two operands:
// the stack byte/word/dword and the (E)SP register itself - but is better than naught]
// added: am_reg, am_stack, am_allgprs
// ADDED: am_ImplImm for implicit immediates

enum { am_A=1, am_C, am_D, am_E, am_F, am_G, am_I, am_J, am_M, am_O, // 10
       am_P, am_Q, am_R, am_S, am_T, am_V, am_W, am_X, am_Y, am_reg, // 20
       am_stackH, am_stackP, am_allgprs, am_VR, am_tworeghack, am_ImplImm }; // pusH and poP produce different addresses

// operand types - idem, but I invented quite a few to make implicit operands explicit.
enum { op_a=1, op_b, op_c, op_d, op_dq, op_p, op_pd, op_pi, op_ps, // 9 
       op_q, op_s, op_sd, op_ss, op_si, op_v, op_w, op_z, op_lea, op_allgprs, op_512,
       op_f, op_dbl, op_14, op_28};


// tables and pseudotables
enum {
  t_ill=0, t_oneB, t_twoB, t_prefixedSSE, t_coprocEsc, t_grp, t_sse, t_grpsse, t_3dnow, t_done=99
};


// registers [only fancy names, not used right now]
/* enum RegisterID { r_AH=100, r_BH, r_CH, r_DH, r_AL, r_BL, r_CL, r_DL, //107 */
/* 		  r_AX, r_DX, //109 */
/* 		  r_eAX, r_eBX, r_eCX, r_eDX, //113 */
/* 		  r_EAX, r_EBX, r_ECX, r_EDX, //117 */
/* 		  r_CS, r_DS, r_ES, r_FS, r_GS, r_SS, //123 */
/* 		  r_eSP, r_eBP, r_eSI, r_eDI, //127 */
/* 		  r_ESP, r_EBP, r_ESI, r_EDI, //131 */
/* 		  r_EDXEAX, r_ECXEBX, //133 */
/* 		  // above two are hacks for cmpxch8b which would have 5 operands otherwise!!! */
/* 		  r_OF, r_SF, r_ZF, r_AF, r_PF, r_CF, r_TF, r_IF, r_DF, r_NT, r_RF, */
/* 		  // flags need to be separate registers for proper liveness analysis */
/* 		  r_DummyFPR, r_Reserved, */
/* 		  // and we have a dummy register to make liveness consistent since floating point saves are all/none at present */
/* 		  r_R8, r_R9, r_R10, r_R11, r_R12, r_R13, r_R14, r_R15 */
/* 		  // AMD64 GPRs */
/* };  */



// registers used for memory access
enum { mRAX=0, mRCX, mRDX, mRBX,
       mRSP, mRBP, mRSI, mRDI,
       mR8, mR9, mR10, mR11, 
       mR12,mR13, MR14, mR15, mRIP };

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
       s1R2RW, // (stack) push
       s1W2RW, // pop
       s1W2R3RW, // additional push/pop
       s1RW2R3R, // shld/shrd
       s1RW2RW3R, // [i]div, cmpxch8b
       s1R2R3R
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


/**
 * This structure can be passed to ia32_decode to have it fill in the 
 * locations of where it found the individual parts of an instruction.
 **/

class ia32_prefixes
{
  friend bool ia32_decode_prefixes(const unsigned char* addr, ia32_prefixes&,
                                   ia32_locations *loc);
  friend bool ia32_decode_rex(const unsigned char* addr, ia32_prefixes&,
                              ia32_locations *loc);
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
  unsigned int getCount() const { return count; }
  unsigned char getPrefix(unsigned char group) const { assert(group <= 4); return prfx[group]; }
  bool rexW() const { return prfx[4] & 0x8; }
  bool rexR() const { return prfx[4] & 0x4; }
  bool rexX() const { return prfx[4] & 0x2; }
  bool rexB() const { return prfx[4] & 0x1; }
  unsigned char getOpcodePrefix() const { return opcode_prefix; }
  unsigned char getAddrSzPrefix() const { return prfx[3]; }
  unsigned char getOperSzPrefix() const { return prfx[2]; }
};

// helper routine to tack-on rex bit when needed
inline int apply_rex_bit(int reg, bool rex_bit)
{
    if (rex_bit)
	return reg + 8;
    else
	return reg;
}

//VG(6/20/02): To support Paradyn without forcing it to include BPatch_memoryAccess, we
//             define this IA-32 "specific" class to encapsulate the same info - yuck

struct ia32_memacc
{
  bool is;
  bool read;
  bool write;
  bool nt;     // non-temporal, e.g. movntq...
  bool prefetch;

  int addr_size; // size of address in 16-bit words
  long imm;
  int scale;
  int regs[2]; // register encodings (in ISA order): 0-7
               // (E)AX, (E)CX, (E)DX, (E)BX
               // (E)SP, (E)BP, (E)SI, (E)DI

  int size;
  int sizehack;  // register (E)CX or string based
  int prefetchlvl; // prefetch level
  int prefetchstt; // prefetch state (AMD)

  ia32_memacc() : is(false), read(false), write(false), nt(false), 
       prefetch(false), addr_size(2), imm(0), scale(0), size(0), sizehack(0),
       prefetchlvl(-1), prefetchstt(-1)
  {
    regs[0] = -1;
    regs[1] = -1;
  }

  void set16(int reg0, int reg1, long disp)
  { 
    is = true;
    addr_size  = 1; 
    regs[0] = reg0; 
    regs[1] = reg1; 
    imm     = disp;
  }

  void set(int reg, long disp, int addr_sz)
  { 
    is = true;
    addr_size = addr_sz;
    regs[0] = reg; 
    imm     = disp;
  }

  void set_sib(int base, int scal, int indx, long disp, int addr_sz)
  {
    is = true;
    addr_size = addr_sz;
    regs[0] = base;
    regs[1] = indx;
    scale   = scal;
    imm     = disp;
  }

  void setXY(int reg, int _size, int _addr_size)
  {
    is = true;
    regs[0] = reg;
    size = _size;
    addr_size = _addr_size;
  }

  void print();
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

bool ia32_decode_prefixes(const unsigned char* addr, ia32_prefixes&,
                          ia32_locations *loc = NULL);


struct ia32_operand {  // operand as given in Intel book tables
  unsigned int admet;  // addressing method
  unsigned int optype; // operand type;
};

// An instruction table entry
struct ia32_entry {
  const char* name(ia32_locations* locs = NULL);
  INSTRUCTION_EXPORT entryID getID(ia32_locations* locs = NULL) const;
  // returns true if any flags are read/written, false otherwise
  INSTRUCTION_EXPORT bool flagsUsed(std::set<Dyninst::InstructionAPI::IA32Regs>& flagsRead, std::set<Dyninst::InstructionAPI::IA32Regs>& flagsWritten,
		 ia32_locations* locs = NULL);
  entryID id;
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

using std::vector;
struct flagInfo
{
  flagInfo(const vector<Dyninst::InstructionAPI::IA32Regs>& rf, const vector<Dyninst::InstructionAPI::IA32Regs>& wf) : readFlags(rf), writtenFlags(wf) 
  {
  }
  flagInfo() 
  {
  }
  
  vector<Dyninst::InstructionAPI::IA32Regs> readFlags;
  vector<Dyninst::InstructionAPI::IA32Regs> writtenFlags;
};

class ia32_instruction
{
  friend unsigned int ia32_decode_operands (const ia32_prefixes& pref, 
                                            const ia32_entry& gotit, 
                                            const char* addr, 
                                            ia32_instruction& instruct);
  friend INSTRUCTION_EXPORT ia32_instruction& ia32_decode(unsigned int capa, const unsigned char* addr,
		  		       ia32_instruction& instruct);
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
					  ia32_entry * entry,
                                          ia32_memacc *mac = NULL);

  unsigned int   size;
  ia32_prefixes  prf;
  ia32_memacc    *mac;
  ia32_condition *cond;
  ia32_entry     *entry;
  ia32_locations *loc;
  unsigned int   legacy_type;
  bool           rip_relative_data;


 public:
  ia32_instruction(ia32_memacc* _mac = NULL, ia32_condition* _cnd = NULL,
                   ia32_locations *loc_ = NULL)
    : mac(_mac), cond(_cnd), entry(NULL), loc(loc_), rip_relative_data(false) {}

  ia32_entry * getEntry() { return entry; }
  unsigned int getSize() const { return size; }
  unsigned int getPrefixCount() const { return prf.getCount(); }
  ia32_prefixes * getPrefix() { return &prf; }
  unsigned int getLegacyType() const { return legacy_type; }
  bool hasRipRelativeData() const { return rip_relative_data; }
  const ia32_memacc& getMac(int which) const { return mac[which]; }
  const ia32_condition& getCond() const { return *cond; }
  const ia32_locations& getLocationInfo() const { return *loc; }

  static const dyn_hash_map<entryID, flagInfo>& getFlagTable();
  static void initFlagTable(dyn_hash_map<entryID, flagInfo>&);
  
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

#define IA32_FULL_DECODER (IA32_DECODE_PREFIXES | IA32_DECODE_MNEMONICS | IA32_DECODE_OPERANDS | IA32_DECODE_JMPS | IA32_DECODE_MEMACCESS | IA32_DECODE_CONDITION)
#define IA32_SIZE_DECODER 0

INSTRUCTION_EXPORT ia32_instruction& ia32_decode(unsigned int capabilities, const unsigned char* addr, ia32_instruction&);


enum dynamic_call_address_mode {
  REGISTER_DIRECT, REGISTER_INDIRECT,
  REGISTER_INDIRECT_DISPLACED, SIB, DISPLACED,
  IP_INDIRECT_DISPLACED
};

/*
   get_instruction: get the instruction that starts at instr.
   return the size of the instruction and set instType to a type descriptor
*/
INSTRUCTION_EXPORT unsigned get_instruction(const unsigned char *instr, unsigned &instType,
			 const unsigned char** op_ptr = NULL);

/* get the target of a jump or call */
INSTRUCTION_EXPORT Address get_target(const unsigned char *instr, unsigned type, unsigned size,
		   Address addr);

// Size of a jump rel32 instruction
#define JUMP_REL32_SZ (6)
// Maxium size of an emitted jump
#define JUMP_SZ (5)
// Size of a call rel32 instruction
#define CALL_REL32_SZ (5)

#if defined(arch_x86_64)
// size of instruction seqeunce to get anywhere in address space
// without touching any registers
#define JUMP_ABS64_SZ (17)
// Jump is push/return; call is push/push/return, so subtract a return
#define CALL_ABS64_SZ (JUMP_ABS64_SZ+JUMP_ABS64_SZ-1)
#endif

#define PUSH_RM_OPC1 (0xFF)
#define PUSH_RM_OPC2 (6)
#define CALL_RM_OPC1 (0xFF)
#define CALL_RM_OPC2 (2)
#define PUSH_EBP (0x50+REGNUM_EBP)
#define SUB_REG_IMM32 (5)
#define LEAVE (0xC9)

#define EXTENDED_0x81_ADD 0
#define EXTENDED_0x81_OR 1
#define EXTENDED_0x81_ADDC 2
#define EXTENDED_0x81_SHIFT 3
#define EXTENDED_0x81_AND 4
#define EXTENDED_0x81_SUB 5
#define EXTENDED_0x81_XOR 6
#define EXTENDED_0x81_CMP 7

INSTRUCTION_EXPORT int displacement(const unsigned char *instr, unsigned type);

inline bool is_disp8(long disp) {
   return (disp >= -128 && disp < 127);
}

inline bool is_disp16(long disp) {
   return (disp >= -32768 && disp < 32767);
}


INSTRUCTION_EXPORT int get_instruction_operand(const unsigned char *i_ptr, Register& base_reg,
			    Register& index_reg, int& displacement, 
			    unsigned& scale, unsigned &mod);
INSTRUCTION_EXPORT void decode_SIB(unsigned sib, unsigned& scale, Register& index_reg, Register& base_reg);
INSTRUCTION_EXPORT const unsigned char* skip_headers(const unsigned char*, ia32_prefixes* = NULL);

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

INSTRUCTION_EXPORT bool insn_hasSIB(unsigned,unsigned&,unsigned&,unsigned&);
INSTRUCTION_EXPORT bool insn_hasDisp8(unsigned ModRM);
INSTRUCTION_EXPORT bool insn_hasDisp32(unsigned ModRM);


#endif
