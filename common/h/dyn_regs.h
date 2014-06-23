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


#if !defined(DYN_REGS_H_)
#define DYN_REGS_H_

#include "util.h"
#include "boost/shared_ptr.hpp"

#include <assert.h>
#include <map>
#include <string>

namespace Dyninst
{
   struct x86OperandParser;
   struct ppcOperandParser;
   typedef unsigned long MachRegisterVal;

   typedef enum
   {
      Arch_none   = 0x00000000,
      Arch_x86    = 0x14000000,
      Arch_x86_64 = 0x18000000,
      Arch_ppc32  = 0x24000000,
      Arch_ppc64  = 0x28000000
   } Architecture;


   COMMON_EXPORT bool isSegmentRegister(int regClass);
   COMMON_EXPORT unsigned getArchAddressWidth(Dyninst::Architecture arch);
   class COMMON_EXPORT MachRegister {
      friend struct ::Dyninst::x86OperandParser;
      friend struct ::Dyninst::ppcOperandParser;
     private:
      signed int reg;

      typedef std::map<signed int, std::string> NameMap;
      static boost::shared_ptr<MachRegister::NameMap> names();
      void init_names();
   public:

	  MachRegister();
     explicit MachRegister(signed int r);
     explicit MachRegister(signed int r, const char *n);
     explicit MachRegister(signed int r, std::string n);

      MachRegister getBaseRegister() const;
      Architecture getArchitecture() const;
      bool isValid() const;
      MachRegisterVal getSubRegValue(const MachRegister& subreg, MachRegisterVal &orig) const;

      std::string name() const;
      unsigned int size() const;
      bool operator<(const MachRegister &a) const;
      bool operator==(const MachRegister &a) const;
      operator signed int() const;
      signed int val() const;
      unsigned int regClass() const;

      static MachRegister getPC(Dyninst::Architecture arch);
      static MachRegister getFramePointer(Dyninst::Architecture arch);
      static MachRegister getStackPointer(Dyninst::Architecture arch);
      static MachRegister getSyscallNumberReg(Dyninst::Architecture arch);
      static MachRegister getSyscallReturnValueReg(Dyninst::Architecture arch);
      bool isPC() const;
      bool isFramePointer() const;
      bool isStackPointer() const;
      bool isSyscallNumberReg() const;
      bool isSyscallReturnValueReg() const;

      void getROSERegister(int &c, int &n, int &p);

      static MachRegister DwarfEncToReg(int encoding, Dyninst::Architecture arch);
      int getDwarfEnc() const;
   };

   /**
    * DEF_REGISTER will define its first parameter as the name of the object
    * it's declaring, and 'i<name>' as the integer value representing that object.
    * As an example, the name of a register may be 
    *  x86::EAX
    * with that register having a value of
    *  x86::iEAX
    * 
    * The value is mostly useful in the 'case' part switch statements.
    **/
#if defined(DYN_DEFINE_REGS)
   //DYN_DEFINE_REGS Should only be defined in libcommon.
   //We want one definition, which will be in libcommon, and declarations
   //for everyone else.
   //
   //I wanted these to be const MachRegister objects, but that changes the
   //linker scope.  Instead they're non-const.  Every accessor function is
   //const anyways, so we'll just close our eyes and pretend they're declared
   //const.
#define DEF_REGISTER(name, value, Arch) \
  const signed int i##name = (value); \
  COMMON_EXPORT MachRegister name(i##name, Arch "::" #name)
#else
#define DEF_REGISTER(name, value, Arch) \
  const signed int i##name = (value); \
  COMMON_EXPORT extern MachRegister name

#endif

   /**
    * For interpreting constants:
    *  Lowest 16 bits (0x000000ff) is base register ID
    *  Next 16 bits (0x0000ff00) is the aliasing and subrange ID-
    *    used on x86/x86_64 to distinguish between things like EAX and AH
    *  Next 16 bits (0x00ff0000) are the register category, GPR/FPR/MMX/...
    *  Top 16 bits (0xff000000) are the architecture.
    *
    *  These values/layout are not guaranteed to remain the same as part of the 
    *  public interface, and may change.
    **/

   //Abstract registers used for stackwalking
   DEF_REGISTER(InvalidReg, 0 | Arch_none, "abstract");
   DEF_REGISTER(FrameBase,  1 | Arch_none, "abstract");
   DEF_REGISTER(ReturnAddr, 2 | Arch_none, "abstract");
   DEF_REGISTER(StackTop,   3 | Arch_none, "abstract");
   // DWARF-ism; the CFA is the value of the stack pointer in the previous frame
   DEF_REGISTER(CFA,        4 | Arch_none, "abstract");

   namespace x86
   {
      const signed int L_REG = 0x00000100; //8-bit, first byte
      const signed int H_REG = 0x00000200; //8-bit, second byte
      const signed int W_REG = 0x00000300; //16-bit, first word
      const signed int FULL  = 0x00000000; //32 bits
      const signed int QUAD  = 0x00004000; //64 bits
      const signed int OCT   = 0x00002000; //128 bits
      const signed int FPDBL = 0x00001000; // 80 bits
      const signed int BIT   = 0x00008000; // 1 bit
      const signed int GPR   = 0x00010000;
      const signed int SEG   = 0x00020000;
      const signed int FLAG  = 0x00030000;
      const signed int MISC  = 0x00040000;
      const signed int XMM   = 0x00050000;
      const signed int MMX   = 0x00060000;
      const signed int CTL   = 0x00070000;
      const signed int DBG   = 0x00080000;
      const signed int TST   = 0x00090000;
      const signed int BASEA  = 0x0;
      const signed int BASEC  = 0x1;
      const signed int BASED  = 0x2;
      const signed int BASEB  = 0x3;
      const signed int BASESP = 0x4;
      const signed int BASEBP = 0x5;
      const signed int BASESI = 0x6;
      const signed int BASEDI = 0x7;
      const signed int FLAGS = 0x0;
      
      const signed int CF = 0x0;
      const signed int FLAG1 = 0x1;
      const signed int PF = 0x2;
      const signed int FLAG3 = 0x3;
      const signed int AF = 0x4;
      const signed int FLAG5 = 0x5;
      const signed int ZF = 0x6;
      const signed int SF = 0x7;
      const signed int TF = 0x8;
      const signed int IF = 0x9;
      const signed int DF = 0xa;
      const signed int OF = 0xb;
      const signed int FLAGC = 0xc;
      const signed int FLAGD = 0xd;
      const signed int NT = 0xe;
      const signed int FLAGF = 0xf;
      const signed int RF = 0x10;

      DEF_REGISTER(eax,   BASEA   | FULL  | GPR  | Arch_x86, "x86");
      DEF_REGISTER(ecx,   BASEC   | FULL  | GPR  | Arch_x86, "x86");
      DEF_REGISTER(edx,   BASED   | FULL  | GPR  | Arch_x86, "x86");
      DEF_REGISTER(ebx,   BASEB   | FULL  | GPR  | Arch_x86, "x86");
      DEF_REGISTER(esp,   BASESP  | FULL  | GPR  | Arch_x86, "x86");
      DEF_REGISTER(ebp,   BASEBP  | FULL  | GPR  | Arch_x86, "x86");
      DEF_REGISTER(esi,   BASESI  | FULL  | GPR  | Arch_x86, "x86");
      DEF_REGISTER(edi,   BASEDI  | FULL  | GPR  | Arch_x86, "x86");
      DEF_REGISTER(ah,    BASEA   | H_REG | GPR  | Arch_x86, "x86");
      DEF_REGISTER(al,    BASEA   | L_REG | GPR  | Arch_x86, "x86");
      DEF_REGISTER(ax,    BASEA   | W_REG | GPR  | Arch_x86, "x86");
      DEF_REGISTER(ch,    BASEC   | H_REG | GPR  | Arch_x86, "x86");
      DEF_REGISTER(cl,    BASEC   | L_REG | GPR  | Arch_x86, "x86");
      DEF_REGISTER(cx,    BASEC   | W_REG | GPR  | Arch_x86, "x86");
      DEF_REGISTER(dh,    BASED   | H_REG | GPR  | Arch_x86, "x86");
      DEF_REGISTER(dl,    BASED   | L_REG | GPR  | Arch_x86, "x86");
      DEF_REGISTER(dx,    BASED   | W_REG | GPR  | Arch_x86, "x86");
      DEF_REGISTER(bh,    BASEB   | H_REG | GPR  | Arch_x86, "x86");
      DEF_REGISTER(bl,    BASEB   | L_REG | GPR  | Arch_x86, "x86");
      DEF_REGISTER(bx,    BASEB   | W_REG | GPR  | Arch_x86, "x86");
      DEF_REGISTER(sp,    BASESP  | W_REG | GPR  | Arch_x86, "x86");
      DEF_REGISTER(bp,    BASEBP  | W_REG | GPR  | Arch_x86, "x86");
      DEF_REGISTER(si,    BASESI  | W_REG | GPR  | Arch_x86, "x86");
      DEF_REGISTER(di,    BASEDI  | W_REG | GPR  | Arch_x86, "x86");
      DEF_REGISTER(eip,   0x10    | FULL         | Arch_x86, "x86");
      DEF_REGISTER(flags, FLAGS   | FULL  | FLAG | Arch_x86, "x86");
      DEF_REGISTER(cf,    CF      | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(flag1, FLAG1   | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(pf,    PF      | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(flag3, FLAG3   | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(af,    AF      | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(flag5, FLAG5   | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(zf,    ZF      | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(sf,    SF      | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(tf,    TF      | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(if_,   IF      | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(df,    DF      | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(of,    OF      | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(flagc, FLAGC   | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(flagd, FLAGD   | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(nt_,   NT      | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(flagf, FLAGF   | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(rf,    RF      | BIT   | FLAG | Arch_x86, "x86");
      DEF_REGISTER(ds,    0x0     | W_REG | SEG  | Arch_x86, "x86");
      DEF_REGISTER(es,    0x1     | W_REG | SEG  | Arch_x86, "x86");
      DEF_REGISTER(fs,    0x2     | W_REG | SEG  | Arch_x86, "x86");
      DEF_REGISTER(gs,    0x3     | W_REG | SEG  | Arch_x86, "x86");
      DEF_REGISTER(cs,    0x4     | W_REG | SEG  | Arch_x86, "x86");
      DEF_REGISTER(ss,    0x5     | W_REG | SEG  | Arch_x86, "x86");
      DEF_REGISTER(oeax,  0x0     | FULL  | MISC | Arch_x86, "x86");
      DEF_REGISTER(fsbase, 0x1    | FULL  | MISC | Arch_x86, "x86");
      DEF_REGISTER(gsbase, 0x2    | FULL  | MISC | Arch_x86, "x86");
      DEF_REGISTER(xmm0,  0x0     | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm1,  0x1     | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm2,  0x2     | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm3,  0x3     | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm4,  0x4     | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm5,  0x5     | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm6,  0x6     | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm7,  0x7     | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(mm0,   0x0     | FPDBL | MMX  | Arch_x86, "x86");
      DEF_REGISTER(mm1,   0x1     | FPDBL | MMX  | Arch_x86, "x86");
      DEF_REGISTER(mm2,   0x2     | FPDBL | MMX  | Arch_x86, "x86");
      DEF_REGISTER(mm3,   0x3     | FPDBL | MMX  | Arch_x86, "x86");
      DEF_REGISTER(mm4,   0x4     | FPDBL | MMX  | Arch_x86, "x86");
      DEF_REGISTER(mm5,   0x5     | FPDBL | MMX  | Arch_x86, "x86");
      DEF_REGISTER(mm6,   0x6     | FPDBL | MMX  | Arch_x86, "x86");
      DEF_REGISTER(mm7,   0x7     | FPDBL | MMX  | Arch_x86, "x86");
      DEF_REGISTER(cr0,   0x0     | FULL  | CTL  | Arch_x86, "x86");
      DEF_REGISTER(cr1,   0x1     | FULL  | CTL  | Arch_x86, "x86");
      DEF_REGISTER(cr2,   0x2     | FULL  | CTL  | Arch_x86, "x86");
      DEF_REGISTER(cr3,   0x3     | FULL  | CTL  | Arch_x86, "x86");
      DEF_REGISTER(cr4,   0x4     | FULL  | CTL  | Arch_x86, "x86");
      DEF_REGISTER(cr5,   0x5     | FULL  | CTL  | Arch_x86, "x86");
      DEF_REGISTER(cr6,   0x6     | FULL  | CTL  | Arch_x86, "x86");
      DEF_REGISTER(cr7,   0x7     | FULL  | CTL  | Arch_x86, "x86");
      DEF_REGISTER(dr0,   0x0     | FULL  | DBG  | Arch_x86, "x86");
      DEF_REGISTER(dr1,   0x1     | FULL  | DBG  | Arch_x86, "x86");
      DEF_REGISTER(dr2,   0x2     | FULL  | DBG  | Arch_x86, "x86");
      DEF_REGISTER(dr3,   0x3     | FULL  | DBG  | Arch_x86, "x86");
      DEF_REGISTER(dr4,   0x4     | FULL  | DBG  | Arch_x86, "x86");
      DEF_REGISTER(dr5,   0x5     | FULL  | DBG  | Arch_x86, "x86");
      DEF_REGISTER(dr6,   0x6     | FULL  | DBG  | Arch_x86, "x86");
      DEF_REGISTER(dr7,   0x7     | FULL  | DBG  | Arch_x86, "x86");
      DEF_REGISTER(tr0,   0x0     | FULL  | TST  | Arch_x86, "x86");
      DEF_REGISTER(tr1,   0x1     | FULL  | TST  | Arch_x86, "x86");
      DEF_REGISTER(tr2,   0x2     | FULL  | TST  | Arch_x86, "x86");
      DEF_REGISTER(tr3,   0x3     | FULL  | TST  | Arch_x86, "x86");
      DEF_REGISTER(tr4,   0x4     | FULL  | TST  | Arch_x86, "x86");
      DEF_REGISTER(tr5,   0x5     | FULL  | TST  | Arch_x86, "x86");
      DEF_REGISTER(tr6,   0x6     | FULL  | TST  | Arch_x86, "x86");
      DEF_REGISTER(tr7,   0x7     | FULL  | TST  | Arch_x86, "x86");
      DEF_REGISTER(st0,   0x0     | FPDBL | MMX  | Arch_x86, "x86");
      DEF_REGISTER(st1,   0x1     | FPDBL | MMX  | Arch_x86, "x86");
      DEF_REGISTER(st2,   0x2     | FPDBL | MMX  | Arch_x86, "x86");
      DEF_REGISTER(st3,   0x3     | FPDBL | MMX  | Arch_x86, "x86");
      DEF_REGISTER(st4,   0x4     | FPDBL | MMX  | Arch_x86, "x86");
      DEF_REGISTER(st5,   0x5     | FPDBL | MMX  | Arch_x86, "x86");
      DEF_REGISTER(st6,   0x6     | FPDBL | MMX  | Arch_x86, "x86");
      DEF_REGISTER(st7,   0x7     | FPDBL | MMX  | Arch_x86, "x86");
   }
   namespace x86_64
   {
      const signed int L_REG = 0x00000100;  //8-bit, first byte
      const signed int H_REG = 0x00000200;  //8-bit, second byte
      const signed int W_REG = 0x00000300; //16 bit, first work
      const signed int D_REG = 0x00000f00; //32 bit, first double word
      const signed int FULL  = 0x00000000; //64 bit
      const signed int FPDBL = 0x00001000; // 80 bits
      const signed int OCT   = 0x00002000; //128 bits
      const signed int BIT   = 0x00008000; // 1 bit
      const signed int GPR   = 0x00010000;
      const signed int SEG   = 0x00020000;
      const signed int FLAG  = 0x00030000;
      const signed int MISC  = 0x00040000;
      const signed int XMM   = 0x00050000;
      const signed int MMX   = 0x00060000;
      const signed int CTL   = 0x00070000;
      const signed int DBG   = 0x00080000;
      const signed int TST   = 0x00090000;
      const signed int FLAGS = 0x00000000;
      const signed int BASEA  = 0x0;
      const signed int BASEC  = 0x1;
      const signed int BASED  = 0x2;
      const signed int BASEB  = 0x3;
      const signed int BASESP = 0x4;
      const signed int BASEBP = 0x5;
      const signed int BASESI = 0x6;
      const signed int BASEDI = 0x7;
      const signed int BASE8  = 0x8;
      const signed int BASE9  = 0x9;
      const signed int BASE10 = 0xa;
      const signed int BASE11 = 0xb;
      const signed int BASE12 = 0xc;
      const signed int BASE13 = 0xd;
      const signed int BASE14 = 0xe;
      const signed int BASE15 = 0xf;

      const signed int CF = x86::CF;
      const signed int PF = x86::PF;
      const signed int AF = x86::AF;
      const signed int ZF = x86::ZF;
      const signed int SF = x86::SF;
      const signed int TF = x86::TF;
      const signed int IF = x86::IF;
      const signed int DF = x86::DF;
      const signed int OF = x86::OF;
      const signed int NT = x86::NT;
      const signed int RF = x86::RF;

      DEF_REGISTER(rax,    BASEA  | FULL  | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(rcx,    BASEC  | FULL  | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(rdx,    BASED  | FULL  | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(rbx,    BASEB  | FULL  | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(rsp,    BASESP | FULL  | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(rbp,    BASEBP | FULL  | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(rsi,    BASESI | FULL  | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(rdi,    BASEDI | FULL  | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r8,     BASE8  | FULL  | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r9,     BASE9  | FULL  | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r10,    BASE10 | FULL  | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r11,    BASE11 | FULL  | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r12,    BASE12 | FULL  | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r13,    BASE13 | FULL  | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r14,    BASE14 | FULL  | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r15,    BASE15 | FULL  | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ah,     BASEA  | H_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(al,     BASEA  | L_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ax,     BASEA  | W_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(eax,    BASEA  | D_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ch,     BASEC  | H_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(cl,     BASEC  | L_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(cx,     BASEC  | W_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ecx,    BASEC  | D_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(dh,     BASED  | H_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(dl,     BASED  | L_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(dx,     BASED  | W_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(edx,    BASED  | D_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(bh,     BASEB  | H_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(bl,     BASEB  | L_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(bx,     BASEB  | W_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ebx,    BASEB  | D_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(spl,    BASESP | L_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(sp,     BASESP | W_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(esp,    BASESP | D_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(bpl,    BASEBP | L_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(bp,     BASEBP | W_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ebp,    BASEBP | D_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(dil,    BASEDI | L_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(di,     BASEDI | W_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(edi,    BASEDI | D_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(sil,    BASESI | L_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(si,     BASESI | W_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(esi,    BASESI | D_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r8b,    BASE8  | L_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r8w,    BASE8  | W_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r8d,    BASE8  | D_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r9b,    BASE9  | L_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r9w,    BASE9  | W_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r9d,    BASE9  | D_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r10b,   BASE10 | L_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r10w,   BASE10 | W_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r10d,   BASE10 | D_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r11b,   BASE11 | L_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r11w,   BASE11 | W_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r11d,   BASE11 | D_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r12b,   BASE12 | L_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r12w,   BASE12 | W_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r12d,   BASE12 | D_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r13b,   BASE13 | L_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r13w,   BASE13 | W_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r13d,   BASE13 | D_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r14b,   BASE14 | L_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r14w,   BASE14 | W_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r14d,   BASE14 | D_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r15b,   BASE15 | L_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r15w,   BASE15 | W_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(r15d,   BASE15 | D_REG | GPR  | Arch_x86_64, "x86_64");
      DEF_REGISTER(rip,    0x10   | FULL         | Arch_x86_64, "x86_64");
      DEF_REGISTER(eip,    0x10   | D_REG        | Arch_x86_64, "x86_64");
      DEF_REGISTER(flags,  FLAGS  | FULL  | FLAG | Arch_x86_64, "x86_64");
      DEF_REGISTER(cf,     CF     | BIT   | FLAG | Arch_x86_64, "x86_64");
      DEF_REGISTER(pf,     PF     | BIT   | FLAG | Arch_x86_64, "x86_64");
      DEF_REGISTER(af,     AF     | BIT   | FLAG | Arch_x86_64, "x86_64");
      DEF_REGISTER(zf,     ZF     | BIT   | FLAG | Arch_x86_64, "x86_64");
      DEF_REGISTER(sf,     SF     | BIT   | FLAG | Arch_x86_64, "x86_64");
      DEF_REGISTER(tf,     TF     | BIT   | FLAG | Arch_x86_64, "x86_64");
      DEF_REGISTER(if_,    IF     | BIT   | FLAG | Arch_x86_64, "x86_64");
      DEF_REGISTER(df,     DF     | BIT   | FLAG | Arch_x86_64, "x86_64");
      DEF_REGISTER(of,     OF     | BIT   | FLAG | Arch_x86_64, "x86_64");
      DEF_REGISTER(nt_,    NT     | BIT   | FLAG | Arch_x86_64, "x86_64");
      DEF_REGISTER(rf,     RF     | BIT   | FLAG | Arch_x86_64, "x86_64");
      DEF_REGISTER(ds,     0x0    | FULL  | SEG  | Arch_x86_64, "x86_64");
      DEF_REGISTER(es,     0x1    | FULL  | SEG  | Arch_x86_64, "x86_64");
      DEF_REGISTER(fs,     0x2    | FULL  | SEG  | Arch_x86_64, "x86_64");
      DEF_REGISTER(gs,     0x3    | FULL  | SEG  | Arch_x86_64, "x86_64");
      DEF_REGISTER(cs,     0x4    | FULL  | SEG  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ss,     0x5    | FULL  | SEG  | Arch_x86_64, "x86_64");
      DEF_REGISTER(orax,   0x0    | FULL  | MISC | Arch_x86_64, "x86_64");
      DEF_REGISTER(fsbase, 0x1    | FULL  | MISC | Arch_x86_64, "x86_64");
      DEF_REGISTER(gsbase, 0x2    | FULL  | MISC | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm0,  0x0     | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm1,  0x1     | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm2,  0x2     | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm3,  0x3     | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm4,  0x4     | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm5,  0x5     | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm6,  0x6     | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm7,  0x7     | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm8,  0x8     | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm9,  0x9     | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm10, 0xA     | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm11, 0xB     | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm12, 0xC     | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm13, 0xD     | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm14, 0xE     | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm15, 0xF     | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(mm0,   0x0     | FPDBL | MMX  | Arch_x86_64, "x86_64");
      DEF_REGISTER(mm1,   0x1     | FPDBL | MMX  | Arch_x86_64, "x86_64");
      DEF_REGISTER(mm2,   0x2     | FPDBL | MMX  | Arch_x86_64, "x86_64");
      DEF_REGISTER(mm3,   0x3     | FPDBL | MMX  | Arch_x86_64, "x86_64");
      DEF_REGISTER(mm4,   0x4     | FPDBL | MMX  | Arch_x86_64, "x86_64");
      DEF_REGISTER(mm5,   0x5     | FPDBL | MMX  | Arch_x86_64, "x86_64");
      DEF_REGISTER(mm6,   0x6     | FPDBL | MMX  | Arch_x86_64, "x86_64");
      DEF_REGISTER(mm7,   0x7     | FPDBL | MMX  | Arch_x86_64, "x86_64");
      DEF_REGISTER(cr0,   0x0     | FULL  | CTL  | Arch_x86_64, "x86_64");
      DEF_REGISTER(cr1,   0x1     | FULL  | CTL  | Arch_x86_64, "x86_64");
      DEF_REGISTER(cr2,   0x2     | FULL  | CTL  | Arch_x86_64, "x86_64");
      DEF_REGISTER(cr3,   0x3     | FULL  | CTL  | Arch_x86_64, "x86_64");
      DEF_REGISTER(cr4,   0x4     | FULL  | CTL  | Arch_x86_64, "x86_64");
      DEF_REGISTER(cr5,   0x5     | FULL  | CTL  | Arch_x86_64, "x86_64");
      DEF_REGISTER(cr6,   0x6     | FULL  | CTL  | Arch_x86_64, "x86_64");
      DEF_REGISTER(cr7,   0x7     | FULL  | CTL  | Arch_x86_64, "x86_64");
      DEF_REGISTER(dr0,   0x0     | FULL  | DBG  | Arch_x86_64, "x86_64");
      DEF_REGISTER(dr1,   0x1     | FULL  | DBG  | Arch_x86_64, "x86_64");
      DEF_REGISTER(dr2,   0x2     | FULL  | DBG  | Arch_x86_64, "x86_64");
      DEF_REGISTER(dr3,   0x3     | FULL  | DBG  | Arch_x86_64, "x86_64");
      DEF_REGISTER(dr4,   0x4     | FULL  | DBG  | Arch_x86_64, "x86_64");
      DEF_REGISTER(dr5,   0x5     | FULL  | DBG  | Arch_x86_64, "x86_64");
      DEF_REGISTER(dr6,   0x6     | FULL  | DBG  | Arch_x86_64, "x86_64");
      DEF_REGISTER(dr7,   0x7     | FULL  | DBG  | Arch_x86_64, "x86_64");
      DEF_REGISTER(tr0,   0x0     | FULL  | TST  | Arch_x86_64, "x86_64");
      DEF_REGISTER(tr1,   0x1     | FULL  | TST  | Arch_x86_64, "x86_64");
      DEF_REGISTER(tr2,   0x2     | FULL  | TST  | Arch_x86_64, "x86_64");
      DEF_REGISTER(tr3,   0x3     | FULL  | TST  | Arch_x86_64, "x86_64");
      DEF_REGISTER(tr4,   0x4     | FULL  | TST  | Arch_x86_64, "x86_64");
      DEF_REGISTER(tr5,   0x5     | FULL  | TST  | Arch_x86_64, "x86_64");
      DEF_REGISTER(tr6,   0x6     | FULL  | TST  | Arch_x86_64, "x86_64");
      DEF_REGISTER(tr7,   0x7     | FULL  | TST  | Arch_x86_64, "x86_64");
      DEF_REGISTER(st0,   0x0     | FPDBL | MMX  | Arch_x86_64, "x86_64");
      DEF_REGISTER(st1,   0x1     | FPDBL | MMX  | Arch_x86_64, "x86_64");
      DEF_REGISTER(st2,   0x2     | FPDBL | MMX  | Arch_x86_64, "x86_64");
      DEF_REGISTER(st3,   0x3     | FPDBL | MMX  | Arch_x86_64, "x86_64");
      DEF_REGISTER(st4,   0x4     | FPDBL | MMX  | Arch_x86_64, "x86_64");
      DEF_REGISTER(st5,   0x5     | FPDBL | MMX  | Arch_x86_64, "x86_64");
      DEF_REGISTER(st6,   0x6     | FPDBL | MMX  | Arch_x86_64, "x86_64");
      DEF_REGISTER(st7,   0x7     | FPDBL | MMX  | Arch_x86_64, "x86_64");
   }
   namespace ppc32 {
      const signed int GPR   = 0x00010000;
      const signed int FPR   = 0x00020000;
      const signed int FSR   = 0x00040000;
      const signed int SPR   = 0x00080000;
      
      DEF_REGISTER(r0,       0 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r1,       1 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r2,       2 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r3,       3 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r4,       4 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r5,       5 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r6,       6 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r7,       7 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r8,       8 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r9,       9 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r10,     10 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r11,     11 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r12,     12 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r13,     13 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r14,     14 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r15,     15 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r16,     16 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r17,     17 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r18,     18 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r19,     19 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r20,     20 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r21,     21 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r22,     22 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r23,     23 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r24,     24 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r25,     25 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r26,     26 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r27,     27 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r28,     28 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r29,     29 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r30,     30 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(r31,     31 | GPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr0,     0 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr1,     1 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr2,     2 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr3,     3 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr4,     4 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr5,     5 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr6,     6 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr7,     7 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr8,     8 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr9,     9 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr10,   10 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr11,   11 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr12,   12 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr13,   13 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr14,   14 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr15,   15 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr16,   16 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr17,   17 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr18,   18 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr19,   19 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr20,   20 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr21,   21 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr22,   22 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr23,   23 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr24,   24 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr25,   25 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr26,   26 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr27,   27 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr28,   28 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr29,   29 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr30,   30 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpr31,   31 | FPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr0,     0 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr1,     1 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr2,     2 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr3,     3 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr4,     4 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr5,     5 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr6,     6 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr7,     7 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr8,     8 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr9,     9 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr10,   10 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr11,   11 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr12,   12 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr13,   13 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr14,   14 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr15,   15 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr16,   16 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr17,   17 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr18,   18 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr19,   19 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr20,   20 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr21,   21 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr22,   22 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr23,   23 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr24,   24 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr25,   25 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr26,   26 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr27,   27 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr28,   28 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr29,   29 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr30,   30 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fsr31,   31 | FSR | Arch_ppc32, "ppc32");
      DEF_REGISTER(mq,       0 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(xer,      1 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(lr,       8 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(ctr,      9 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(dsisr,   18 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(dar,     19 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(dec,     22 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(sdr1,    25 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(srr0,    26 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(srr1,    27 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(sprg0,  272 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(sprg1,  273 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(sprg2,  274 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(sprg3,  275 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(sprg4,  276 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(sprg5,  277 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(sprg6,  278 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(sprg7,  279 | SPR | Arch_ppc32, "ppc32");

      DEF_REGISTER(sprg3_ro,  259 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(sprg4_ro,  260 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(sprg5_ro,  261 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(sprg6_ro,  262 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(sprg7_ro,  263 | SPR | Arch_ppc32, "ppc32");


      DEF_REGISTER(ear,    282 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(tbl_wo,    284 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(tbl_ro,    268 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(tbu_wo,    285 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(tbu_ro,    269 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(pvr,    287 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(ibat0u, 528 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(ibat0l, 529 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(ibat1u, 530 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(ibat1l, 531 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(ibat2u, 532 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(ibat2l, 533 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(ibat3u, 534 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(ibat3l, 535 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(dbat0u, 536 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(dbat0l, 537 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(dbat1u, 538 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(dbat1l, 539 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(dbat2u, 540 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(dbat2l, 541 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(dbat3u, 542 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(dbat3l, 543 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(pc,     600 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpscw,  601 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpscw0, 602 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpscw1, 603 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpscw2, 604 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpscw3, 605 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpscw4, 606 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpscw5, 607 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpscw6, 608 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(fpscw7, 609 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(msr,    610 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(ivpr,   611 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(ivor8,  612 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(seg0,   613 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(seg1,   614 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(seg2,   615 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(seg3,   616 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(seg4,   617 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(seg5,   618 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(seg6,   619 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(seg7,   620 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr0,    621 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr1,    622 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr2,    623 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr3,    624 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr4,    625 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr5,    626 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr6,    627 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr7,    628 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr,     629 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(or3,    630 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(trap,   631 | SPR | Arch_ppc32, "ppc32");      
   }
   namespace ppc64 {
      const signed int GPR   = 0x00010000;
      const signed int FPR   = 0x00020000;
      const signed int FSR   = 0x00040000;
      const signed int SPR   = 0x00080000;
      
      DEF_REGISTER(r0,       0 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r1,       1 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r2,       2 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r3,       3 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r4,       4 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r5,       5 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r6,       6 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r7,       7 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r8,       8 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r9,       9 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r10,     10 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r11,     11 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r12,     12 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r13,     13 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r14,     14 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r15,     15 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r16,     16 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r17,     17 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r18,     18 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r19,     19 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r20,     20 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r21,     21 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r22,     22 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r23,     23 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r24,     24 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r25,     25 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r26,     26 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r27,     27 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r28,     28 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r29,     29 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r30,     30 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(r31,     31 | GPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr0,     0 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr1,     1 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr2,     2 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr3,     3 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr4,     4 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr5,     5 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr6,     6 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr7,     7 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr8,     8 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr9,     9 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr10,   10 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr11,   11 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr12,   12 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr13,   13 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr14,   14 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr15,   15 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr16,   16 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr17,   17 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr18,   18 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr19,   19 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr20,   20 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr21,   21 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr22,   22 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr23,   23 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr24,   24 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr25,   25 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr26,   26 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr27,   27 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr28,   28 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr29,   29 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr30,   30 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpr31,   31 | FPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr0,     0 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr1,     1 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr2,     2 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr3,     3 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr4,     4 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr5,     5 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr6,     6 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr7,     7 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr8,     8 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr9,     9 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr10,   10 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr11,   11 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr12,   12 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr13,   13 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr14,   14 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr15,   15 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr16,   16 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr17,   17 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr18,   18 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr19,   19 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr20,   20 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr21,   21 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr22,   22 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr23,   23 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr24,   24 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr25,   25 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr26,   26 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr27,   27 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr28,   28 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr29,   29 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr30,   30 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fsr31,   31 | FSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(mq,       0 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(xer,      1 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(lr,       8 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(ctr,      9 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(dsisr,   18 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(dar,     19 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(dec,     22 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(sdr1,    25 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(srr0,    26 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(srr1,    27 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(sprg0,  272 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(sprg1,  273 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(sprg2,  274 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(sprg3,  275 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(sprg4,  276 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(sprg5,  277 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(sprg6,  278 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(sprg7,  279 | SPR | Arch_ppc64, "ppc64");

      DEF_REGISTER(sprg3_ro,  259 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(sprg4_ro,  260 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(sprg5_ro,  261 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(sprg6_ro,  262 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(sprg7_ro,  263 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(ear,    282 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(tbl_wo,    284 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(tbl_ro,    268 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(tbu_wo,    285 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(tbu_ro,    269 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(pvr,    287 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(ibat0u, 528 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(ibat0l, 529 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(ibat1u, 530 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(ibat1l, 531 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(ibat2u, 532 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(ibat2l, 533 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(ibat3u, 534 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(ibat3l, 535 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(dbat0u, 536 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(dbat0l, 537 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(dbat1u, 538 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(dbat1l, 539 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(dbat2u, 540 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(dbat2l, 541 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(dbat3u, 542 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(dbat3l, 543 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(pc,     600 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpscw,  601 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpscw0, 602 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpscw1, 603 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpscw2, 604 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpscw3, 605 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpscw4, 606 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpscw5, 607 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpscw6, 608 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(fpscw7, 609 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(msr,    610 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(ivpr,   611 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(ivor8,  612 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(seg0,   613 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(seg1,   614 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(seg2,   615 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(seg3,   616 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(seg4,   617 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(seg5,   618 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(seg6,   619 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(seg7,   620 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr0,    621 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr1,    622 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr2,    623 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr3,    624 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr4,    625 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr5,    626 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr6,    627 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr7,    628 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr,     629 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(or3,    630 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(trap,   631 | SPR | Arch_ppc64, "ppc64");      
   }
}

#endif
