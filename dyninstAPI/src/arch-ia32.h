// $Id: arch-ia32.h,v 1.5 2002/08/04 17:29:52 gaburici Exp $
// VG(02/06/2002): configurable IA-32 decoder

#if !(defined(i386_unknown_linux2_0) || defined(i386_unknown_nt4_0))
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

//VG(6/20/02): To support Paradyn without forcing it to include BPatch_memoryAccess, we
//             define this IA-32 "specific" class to encapsulate the same info - yuck

struct ia32_memacc
{
  bool is;
  bool read;
  bool write;
  bool nt;     // non-temporal, e.g. movnt...
  bool prefetch;

  bool addr32; // true if 32-bit addressing, false otherwise
  int imm;
  int scale;
  int regs[2]; // register encodings (in ISA order): 0-7
               // (E)AX, (E)CX, (E)DX, (E)BX
               // (E)SP, (E)BP, (E)SI, (E)DI

  int size;
  int sizehack;  // register (E)CX or string based
  int prftchlvl; // prefetch level
  int prftchstt; // prefetch state (AMD)

  ia32_memacc() : is(false), read(false), write(false), nt(false), 
       prefetch(false), addr32(true), imm(0), scale(0), size(0), sizehack(0),
       prftchlvl(0), prftchstt(0)
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


ia32_prefixes& ia32_decode_prefixes(const unsigned char* addr, ia32_prefixes&);


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

  unsigned int size;
  ia32_prefixes prf;
  ia32_memacc  *mac;
  unsigned int legacy_type;

 public:

  ia32_instruction(ia32_memacc* _mac = NULL) : mac(_mac) {}

  unsigned int getSize() const { return size; }
  unsigned int getLegacyType() const { return legacy_type; }
  const ia32_memacc& getMac(int which) const { return mac[which]; }

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
#define IA32_DECODE_CONDITIONS	(1<<5)

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

#endif
