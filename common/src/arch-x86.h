/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

// $Id: arch-x86.h,v 1.67 2008/10/28 18:42:39 bernat Exp $
// x86 instruction declarations

#ifndef _ARCH_X86_H
#define _ARCH_X86_H

#include "dyntypes.h"
#include <assert.h>
#include <stdio.h>
#include <set>
#include <map>
#include <vector>
#include "entryIDs.h"
#include "registers/MachRegister.h"
#include "common/src/ia32_locations.h"
#include "dyn_register.h"
#include "encoding-x86.h"

namespace NS_x86 {

typedef unsigned char codeBuf_t;
typedef unsigned codeBufIndex_t;

class ia32_instruction;

class ia32_prefixes
{
  friend DYNINST_EXPORT bool ia32_decode_prefixes(const unsigned char *addr, ia32_instruction &insn, bool mode_64);
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

  /* Because VEX fields are based on the VEX type, they are decoded immediately. */
  bool vex_present; /* Does this instruction have a vex prefix?  */
  bool XOP;  /* whether this instrucxtion is an XOP instrucxtion */
  VEX_TYPE vex_type; /* If there is a vex prefix present, what type is it? */
  unsigned char vex_prefix[5]; /* Support up to EVEX (VEX-512) */
  int vex_sse_mult; /* index for sse multiplexer table */
  int vex_vvvv_reg; /* The register specified by this prefix. */
  int vex_ll; /* l bit for VEX2, VEX3 or ll for EVEX */
  int vex_pp; /* pp bits for VEX2, VEX3 or EVEX */
  int vex_m_mmmm; /* m-mmmm bits for VEX2, VEX3 or EVEX */
  int vex_w; /* w bit for VEX2, VEX3 or EVEX */
  int vex_V; /* V' modifier for EVEX */
  int vex_r; /* The VEX REXR bit for VEX2, VEX3 or EVEX*/
  int vex_R; /* The VEX REXR' bit for EVEX */
  int vex_x; /* The VEX REXX bit for VEX2, VEX3 or EVEX */
  int vex_b; /* The VEX REXB bit for VEX2, VEX3 or EVEX */
  int vex_aaa; /* Selects the vector mask register for EVEX */
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

  ia32_condition() : is(false), tttn(-1) {}
  void set(int _tttn) { is = true; tttn = _tttn; }
};

struct ia32_operand {  // operand as given in Intel book tables
  unsigned int admet;  // addressing method
  unsigned int optype; // operand type;
};

// An instruction table entry
struct ia32_entry {
  DYNINST_EXPORT const char* name(ia32_locations* locs = NULL);
  DYNINST_EXPORT entryID getID(ia32_locations* locs = NULL) const;
  entryID id;
  unsigned int otable;       // which opcode table is next; if t_done it is the current one
  unsigned char tabidx;      // at what index to look, 0 if it easy to deduce from opcode
  bool hasModRM;             // true if the instruction has a MOD/RM byte
  ia32_operand operands[4];  // operand descriptors
  unsigned int legacyType;   // legacy type of the instruction (e.g. (IS_CALL | REL_W))
  // code to decode memory access - this field should be seen as two 16 bit fields
  // the lower half gives operand semantics, e.g. s1RW2R, the upper half is a fXXX hack if needed
  // before hating me for this: it takes a LOT less time to add ONE field to ~2000 table lines!
  // The upper 3 bits of this field (bits 29, 30, 31) are specifiers for implicit operands.
  unsigned int opsema;  

  unsigned int impl_dec; /* Implicit operands and decoration descriptions */
};

class ia32_instruction
{
  friend unsigned int ia32_decode_operands (const ia32_prefixes& pref, 
                                            const ia32_entry& gotit, 
                                            const char* addr, 
                                            ia32_instruction& instruct);
  friend DYNINST_EXPORT bool ia32_decode_prefixes(const unsigned char *addr, ia32_instruction &insn, bool mode_64);
  friend DYNINST_EXPORT ia32_instruction &
  ia32_decode(unsigned int capa, const unsigned char *addr, ia32_instruction &instruct, bool mode_64);
  friend DYNINST_EXPORT int
  ia32_decode_opcode(unsigned int capa, const unsigned char *addr, ia32_instruction &instruct, ia32_entry **gotit_ret,
                       bool mode_64);
  friend unsigned int ia32_decode_operands(const ia32_prefixes &pref, const ia32_entry &gotit, const unsigned char *addr,
                                             ia32_instruction &instruct, ia32_memacc *mac, bool mode_64);
  friend ia32_instruction& ia32_decode_FP(const ia32_prefixes& pref, const unsigned char* addr,
                                          ia32_instruction& instruct);
  friend unsigned int ia32_emulate_old_type(ia32_instruction &instruct, bool mode_64);
  friend ia32_instruction &
  ia32_decode_FP(unsigned int opcode, const ia32_prefixes &pref, const unsigned char *addr, ia32_instruction &instruct,
                   ia32_entry *entry, ia32_memacc *mac, bool mode_64);

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
    : size(0), prf(), mac(_mac), cond(_cnd), entry(NULL), loc(loc_),
      legacy_type(0), rip_relative_data(false)
  {}

  ia32_entry * getEntry() { return entry; }
  unsigned int getSize() const { return size; }
  unsigned int getPrefixCount() const { return prf.getCount(); }
  ia32_prefixes * getPrefix() { return &prf; }
  unsigned int getLegacyType() const { return legacy_type; }
  bool hasRipRelativeData() const { return rip_relative_data; }
  const ia32_memacc& getMac(int which) const { return mac[which]; }
  const ia32_condition& getCond() const { return *cond; }
  const ia32_locations& getLocationInfo() const { return *loc; }
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
#define IA32_DECODE_JMPS	    (1<<3)
#define IA32_DECODE_MEMACCESS	(1<<4)
#define IA32_DECODE_CONDITION 	(1<<5)

#define IA32_FULL_DECODER (IA32_DECODE_PREFIXES \
        | IA32_DECODE_MNEMONICS \
        | IA32_DECODE_OPERANDS \
        | IA32_DECODE_JMPS \
        | IA32_DECODE_MEMACCESS \
        | IA32_DECODE_CONDITION)
#define IA32_SIZE_DECODER 0

/* TODO: documentation*/
DYNINST_EXPORT bool ia32_decode_prefixes(const unsigned char *addr, ia32_instruction &insn, bool mode_64);

/**
 * Decode just the opcode of the given instruction. This implies that
 * ia32_decode_prefixes has already been called on the given instruction
 * and addr has been moved past the prefix bytes. Returns zero on success,
 * non zero otherwise.
 */
DYNINST_EXPORT int
ia32_decode_opcode(unsigned int capa, const unsigned char *addr, ia32_instruction &instruct, ia32_entry **gotit_ret,
                   bool mode_64);

/**
 * Do a complete decoding of the instruction at the given address. This
 * function calls ia32_decode_prefixes, ia32_decode_opcode and
 * ia32_decode_operands. Returns zero on success, non zero otherwise.
 * When there is a decoding failure, the state of the given instruction
 * is not defined. capabilities is a mask of the above flags (IA32_DECODE_*).
 * The mask determines what part of the instruction should be decoded.
 */
DYNINST_EXPORT ia32_instruction &
ia32_decode(unsigned int capabilities, const unsigned char *addr, ia32_instruction &, bool mode_64);


/*
   get_instruction: get the instruction that starts at instr.
   return the size of the instruction and set instType to a type descriptor
*/
DYNINST_EXPORT unsigned
get_instruction(const unsigned char *instr, unsigned &instType, const unsigned char **op_ptr, bool mode_64);

/* get the target of a jump or call */
DYNINST_EXPORT Dyninst::Address get_target(const unsigned char *instr, unsigned type, unsigned size,
		   Dyninst::Address addr);

// Size of a jump rel32 instruction
#define JUMP_REL32_SZ (6)
// Maxium size of an emitted jump
#define JUMP_SZ (5)
// Size of a call rel32 instruction
#define CALL_REL32_SZ (5)
// >2gb displacement in 32 bit mode
#define CALL_ABS32_SZ (11)
#define JUMP_ABS32_SZ (6)
// Max size of a relocated thunk call
#define CALL_RELOC_THUNK (13)

#if defined(DYNINST_CODEGEN_ARCH_X86_64)
// size of instruction seqeunce to get anywhere in address space
// without touching any registers
#define JUMP_ABS64_SZ (14)
// Jump is push/return; call is push/push/return, so subtract a return
#define CALL_ABS64_SZ (JUMP_ABS64_SZ+JUMP_ABS64_SZ-1)
#endif

#define PUSH_RM_OPC1 (0xFF)
#define PUSH_RM_OPC2 (6)
#define CALL_RM_OPC1 (0xFF)
#define CALL_RM_OPC2 (2)
#define JUMP_RM_OPC1 (0xFF)
#define JUMP_RM_OPC2 (4)
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
#define EXTENDED_0x83_AND 4

class instruction {
 public:
    instruction(): type_(0), size_(0), ptr_(0), op_ptr_(0) {}

  instruction(const unsigned char *p, unsigned type, unsigned sz, const unsigned char* op = 0):
      type_(type), size_(sz), ptr_(p), op_ptr_(op ? op : p) {}

  instruction(const instruction &insn)
  {
    type_ = insn.type_;
    size_ = insn.size_;
    ptr_ = insn.ptr_;
    op_ptr_ = insn.op_ptr_;
  }
  
  DYNINST_EXPORT instruction *copy() const;

  instruction(const void *ptr, bool mode_64) :
      type_(0), size_(0), ptr_(NULL), op_ptr_(0) {
      setInstruction((const unsigned char *) ptr, 0, mode_64);
  }

  unsigned setInstruction(const unsigned char *p, Dyninst::Address, bool mode_64) {
      ptr_ = p;
      size_ = get_instruction(ptr_, type_, &op_ptr_, mode_64);
      return size_;
  }

  // if the instruction is a jump or call, return the target, else return zero
  Dyninst::Address getTarget(Dyninst::Address addr) const {
    return (Dyninst::Address)get_target(ptr_, type_, size_, addr);
  }

  // return the size of the instruction in bytes
  unsigned size() const { return size_; }

  // And the size necessary to reproduce this instruction
  // at some random point.
  DYNINST_EXPORT unsigned spaceToRelocate() const;

  // return the type of the instruction
  unsigned type() const { return type_; }

  // return a pointer to the instruction
  const unsigned char *ptr() const { 
      return ptr_; 
  }

  // return a pointer to the instruction's opcode
  const unsigned char* op_ptr() const { return op_ptr_; }

  // Function relocation...
  static unsigned maxInterFunctionJumpSize(unsigned addr_width) { return maxJumpSize(addr_width); }

  // And tell us how much space we'll need...
  DYNINST_EXPORT static unsigned jumpSize(Dyninst::Address from, Dyninst::Address to, unsigned addr_width);
  DYNINST_EXPORT static unsigned jumpSize(long disp, unsigned addr_width);
  DYNINST_EXPORT static unsigned maxJumpSize(unsigned addr_width);

    bool isCall() const { return type_ & IS_CALL; }
  bool isCallIndir() const { return (type_ & IS_CALL) && (type_ & INDIR); }
  bool isReturn() const { return (type_ & IS_RET) || (type_ & IS_RETF); }
  bool isRetFar() const { return type_ & IS_RETF; }
  bool isCleaningRet() const {return type_ & IS_RETC; }
  bool isJumpIndir() const { return (type_ & IS_JUMP) && (type_ & INDIR); }
  bool isJumpDir() const
    { return !(type_ & INDIR) && ((type_ & IS_JUMP) || (type_ & IS_JCC)); }
  bool isUncondJump() const
    { return ((type_ & IS_JUMP) && !(type_ & IS_JCC)); }

    bool isIndir() const { return type_ & INDIR; }
  bool isIllegal() const { return type_ & ILLEGAL; }
  bool isLeave() const { return *ptr_ == 0xC9; }  
  bool isPrivileged() const { return (type_ & PRVLGD); }
  bool isMoveRegMemToRegMem() const 
    { const unsigned char* p = op_ptr_ ? op_ptr_ : ptr_;
      return *p == MOV_R8_TO_RM8   || *p == MOV_R16_TO_RM16 ||
             *p == MOV_R32_TO_RM32 || *p ==  MOV_RM8_TO_R8  ||
             *p == MOV_RM16_TO_R16 || *p == MOV_RM32_TO_R32;   }
  bool isXORRegMemRegMem() const
      { const unsigned char* p = op_ptr_ ? op_ptr_ : ptr_;
        return
               *p == XOR_RM16_R16 ||
#if !defined(XOR_RM16_R16) || !defined(XOR_RM32_R32) || (XOR_RM16_R16) != (XOR_RM32_R32)
// XOR_RM16_R16 and XOR_RM32_R32 are macros with the same value so disable this
// clause as it is unnecessary and produces a compiler warning
               *p ==  XOR_RM32_R32 ||
#endif
               *p ==  XOR_R8_RM8  || *p ==  XOR_R16_RM16 ||
               *p == XOR_R32_RM32; }
  bool isANearBranch() const { return isJumpDir(); }

  bool isSysCallInsn() const { return op_ptr_[0] == SYSCALL[0] &&
                                   op_ptr_[1] == SYSCALL[1]; }

    void print()
  {
      for (unsigned i = 0; i < size_; i++)
	  fprintf(stderr, " %02x", *(ptr_ + i));
      fprintf(stderr, "\n");
  }

private:
  unsigned type_;   // type of the instruction (e.g. IS_CALL | INDIR)
  unsigned size_;   // size in bytes
  const unsigned char *ptr_;       // pointer to the instruction
  const unsigned char *op_ptr_;    // pointer to the opcode
};

/** Returns the immediate operand of an instruction **/

    DYNINST_EXPORT int count_prefixes(unsigned insnType);

inline bool is_disp8(long disp) {
   return (disp >= -128 && disp < 127);
}

inline bool is_disp32(long disp) {
  return (disp <= INT32_MAX && disp >= INT32_MIN);
}
inline bool is_disp32(Dyninst::Address a1, Dyninst::Address a2) {
  return is_disp32(a2 - (a1 + JUMP_REL32_SZ));
}
inline bool is_addr32(Dyninst::Address addr) {
    return (addr < UINT32_MAX);
}

} // namespace arch_x86

#endif

