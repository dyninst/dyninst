// $Id: arch-ia32.h,v 1.2 2002/06/07 20:17:54 gaburici Exp $
// VG(02/06/2002): configurable IA-32 decoder

#if !(defined(i386_unknown_linux2_0))
#error "invalid architecture-os inclusion"
#endif

#ifndef _ARCH_IA32_H
#define _ARCH_IA32_H

#include <stdlib.h>
#include "arch-x86.h"


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


class ia32_prefixes
{
  friend ia32_prefixes& ia32_decode_prefixes(const unsigned char* addr, ia32_prefixes&);
 private:
  unsigned int count;
  // At most 4 prefixes are allowed for Intel 32-bit CPUs
  // There also 4 groups, so this array is 0 if no prefix
  // from that group is present, otherwise it contains the
  // prefix opcode
  unsigned char prfx[4];
 public:
  unsigned int const getCount() const { return count; }
  unsigned char getPrefix(unsigned char group) const { return prfx[group]; }
};


ia32_prefixes& ia32_decode_prefixes(const unsigned char* addr, ia32_prefixes&);


class ia32_entry;

class ia32_instruction {
  friend unsigned int ia32_decode_operands (const ia32_prefixes& pref, const ia32_entry& gotit, 
                                            const char* addr, ia32_instruction& instruct);
  template <unsigned int capa>
    friend ia32_instruction& ia32_decode(const unsigned char* addr, ia32_instruction& instruct);
  friend unsigned int ia32_decode_operands (const ia32_prefixes& pref, const ia32_entry& gotit, 
                                     const unsigned char* addr, ia32_instruction& instruct);
  friend ia32_instruction& ia32_decode_FP(const ia32_prefixes& pref, const unsigned char* addr,
                                          ia32_instruction& instruct);
  friend unsigned int ia32_emulate_old_type(ia32_instruction& instruct);
  unsigned int size;
  ia32_prefixes prf;
  unsigned int legacy_type;

 public:
  unsigned int getSize() const { return size; }
  unsigned int getLegacyType() const { return legacy_type; }

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
#define IA32_DECODE_MOVS	(1<<4)
#define IA32_DECODE_CONDITIONS	(1<<5)

#define IA32_FULL_DECODER 0xffffffffffffffffu
#define IA32_SIZE_DECODER 0

template <unsigned int capabilities>
ia32_instruction& ia32_decode(const unsigned char* addr, ia32_instruction&);

// If typing the template every time is a pain, the following should help:
#define ia32_decode_all  ia32_decode<IA32_FULL_DECODER>
#define ia32_decode_size ia32_decode<IA32_SIZE_DECODER>
#define ia32_size(a,i)   ia32_decode_size((a),(i)).size

#endif
