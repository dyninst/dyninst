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
#include "ia32_memacc.h"
#include "ia32_prefixes.h"
#include "ia32_entry.h"
#include "ia32_instruction.h"

namespace NS_x86 {

typedef unsigned char codeBuf_t;
typedef unsigned codeBufIndex_t;


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

