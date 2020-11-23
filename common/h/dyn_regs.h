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
    struct aarch64OperandParser;

    typedef unsigned long MachRegisterVal;

    //0xff000000 is used to encode architecture
    typedef enum
    {
        Arch_none        =  0x00000000,
        Arch_x86         =  0x14000000,
        Arch_x86_64      =  0x18000000,
        Arch_ppc32       =  0x24000000,
        Arch_ppc64       =  0x28000000,
        Arch_aarch32     =  0x44000000, //for later use
        Arch_aarch64     =  0x48000000,
        Arch_amdgpu_vega      =  0x84000000,
        Arch_cuda        =  0x88000000,
        Arch_amdgpu_rdna =  0x8c000000, //future support for rdna
        Arch_intelGen9 = 0xb6000000	//same as machine no. retrevied from eu-readelf
    } Architecture;


    COMMON_EXPORT bool isSegmentRegister(int regClass);
    COMMON_EXPORT unsigned getArchAddressWidth(Dyninst::Architecture arch);
    class COMMON_EXPORT MachRegister {
        friend struct ::Dyninst::x86OperandParser;
        friend struct ::Dyninst::ppcOperandParser;
        friend struct ::Dyninst::aarch64OperandParser;
    private:
        signed int reg;

        typedef std::map<signed int, std::string> NameMap;
        static boost::shared_ptr<MachRegister::NameMap> names();
        void init_names();
        // reg_class is set to a corresponding enum value that can be found in "external/rose/amdgpuInstructionEnum.h"
        // reg_idx is set to the index/id of the register for future lookup
        // offset is set to the byte offset from where the register value starts, that is, sub register access ( or register access in a register vector)
        void getAMDGPUROSERegister(int &reg_class, int &reg_idx, int &offset);
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
        static MachRegister getReturnAddress(Dyninst::Architecture arch);
        static MachRegister getFramePointer(Dyninst::Architecture arch);
        static MachRegister getStackPointer(Dyninst::Architecture arch);
        static MachRegister getSyscallNumberReg(Dyninst::Architecture arch);
        static MachRegister getSyscallNumberOReg(Dyninst::Architecture arch);
        static MachRegister getSyscallReturnValueReg(Dyninst::Architecture arch);
	static MachRegister getZeroFlag(Dyninst::Architecture arch);

        bool isPC() const;
        bool isFramePointer() const;
        bool isStackPointer() const;
        bool isSyscallNumberReg() const;
        bool isSyscallReturnValueReg() const;
	bool isFlag() const;
	bool isZeroFlag() const;

        void getROSERegister(int &c, int &n, int &p);

        static MachRegister DwarfEncToReg(int encoding, Dyninst::Architecture arch);
        static MachRegister getArchRegFromAbstractReg(MachRegister abstract, Dyninst::Architecture arch);
        int getDwarfEnc() const;

        static MachRegister getArchReg(unsigned int regNum, Dyninst::Architecture arch);
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
      // MachRegister::getBaseRegister clears the bit field for size,
      // so the full register size has to be 0
      const signed int FULL  = 0x00000000; //32 bits
      const signed int OCT   = 0x00000600; //128 bits
      const signed int FPDBL = 0x00000700; // 80 bits
      const signed int BIT   = 0x00000800; // 1 bit
      const signed int YMMS  = 0x00000900; // YMM are 256 bits
      const signed int ZMMS  = 0x00000A00; // ZMM are 512 bits
      const signed int GPR   = 0x00010000;
      const signed int SEG   = 0x00020000;
      const signed int FLAG  = 0x00030000;
      const signed int MISC  = 0x00040000;
      const signed int KMASK = 0x00050000;
      const signed int XMM   = 0x00060000;
      const signed int YMM   = 0x00070000;
      const signed int ZMM   = 0x00080000;
      const signed int MMX   = 0x00090000;
      const signed int CTL   = 0x000A0000;
      const signed int DBG   = 0x000B0000;
      const signed int TST   = 0x000C0000;
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

      DEF_REGISTER(k0,    0x00    | OCT   | KMASK| Arch_x86, "x86");
      DEF_REGISTER(k1,    0x01    | OCT   | KMASK| Arch_x86, "x86");
      DEF_REGISTER(k2,    0x02    | OCT   | KMASK| Arch_x86, "x86");
      DEF_REGISTER(k3,    0x03    | OCT   | KMASK| Arch_x86, "x86");
      DEF_REGISTER(k4,    0x04    | OCT   | KMASK| Arch_x86, "x86");
      DEF_REGISTER(k5,    0x05    | OCT   | KMASK| Arch_x86, "x86");
      DEF_REGISTER(k6,    0x06    | OCT   | KMASK| Arch_x86, "x86");
      DEF_REGISTER(k7,    0x07    | OCT   | KMASK| Arch_x86, "x86");
      
      DEF_REGISTER(xmm0,  0x00    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm1,  0x01    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm2,  0x02    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm3,  0x03    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm4,  0x04    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm5,  0x05    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm6,  0x06    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm7,  0x07    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm8,  0x08    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm9,  0x09    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm10, 0x0A    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm11, 0x0B    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm12, 0x0C    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm13, 0x0D    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm14, 0x0E    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm15, 0x0F    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm16, 0x10    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm17, 0x11    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm18, 0x12    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm19, 0x13    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm20, 0x14    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm21, 0x15    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm22, 0x16    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm23, 0x17    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm24, 0x18    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm25, 0x19    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm26, 0x1A    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm27, 0x1B    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm28, 0x1C    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm29, 0x1D    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm30, 0x1E    | OCT   | XMM  | Arch_x86, "x86");
      DEF_REGISTER(xmm31, 0x1F    | OCT   | XMM  | Arch_x86, "x86");


      DEF_REGISTER(ymm0,  0x00    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm1,  0x01    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm2,  0x02    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm3,  0x03    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm4,  0x04    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm5,  0x05    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm6,  0x06    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm7,  0x07    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm8,  0x08    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm9,  0x09    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm10, 0x0A    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm11, 0x0B    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm12, 0x0C    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm13, 0x0D    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm14, 0x0E    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm15, 0x0F    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm16, 0x10    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm17, 0x11    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm18, 0x12    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm19, 0x13    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm20, 0x14    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm21, 0x15    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm22, 0x16    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm23, 0x17    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm24, 0x18    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm25, 0x19    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm26, 0x1A    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm27, 0x1B    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm28, 0x1C    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm29, 0x1D    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm30, 0x1E    | YMMS  | YMM  | Arch_x86, "x86");
      DEF_REGISTER(ymm31, 0x1F    | YMMS  | YMM  | Arch_x86, "x86");

      DEF_REGISTER(zmm0,  0x00    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm1,  0x01    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm2,  0x02    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm3,  0x03    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm4,  0x04    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm5,  0x05    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm6,  0x06    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm7,  0x07    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm8,  0x08    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm9,  0x09    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm10, 0x0A    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm11, 0x0B    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm12, 0x0C    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm13, 0x0D    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm14, 0x0E    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm15, 0x0F    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm16, 0x10    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm17, 0x11    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm18, 0x12    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm19, 0x13    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm20, 0x14    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm21, 0x15    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm22, 0x16    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm23, 0x17    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm24, 0x18    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm25, 0x19    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm26, 0x1A    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm27, 0x1B    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm28, 0x1C    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm29, 0x1D    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm30, 0x1E    | ZMMS  | ZMM  | Arch_x86, "x86");
      DEF_REGISTER(zmm31, 0x1F    | ZMMS  | ZMM  | Arch_x86, "x86");

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
      const signed int D_REG = 0x00000F00; //32 bit, first double word
      // MachRegister::getBaseRegister clears the bit field for size,
      // so the full register size has to be 0
      const signed int FULL  = 0x00000000; //64 bits
      const signed int OCT   = 0x00000600; //128 bits
      const signed int FPDBL = 0x00000700; // 80 bits
      const signed int BIT   = 0x00000800; // 1 bit
      const signed int YMMS  = 0x00000900; // YMM are 256 bits
      const signed int ZMMS  = 0x00000A00; // ZMM are 512 bits
      const signed int GPR   = 0x00010000;
      const signed int SEG   = 0x00020000;
      const signed int FLAG  = 0x00030000;
      const signed int MISC  = 0x00040000;
      const signed int KMASK = 0x00050000;
      const signed int XMM   = 0x00060000;
      const signed int YMM   = 0x00070000;
      const signed int ZMM   = 0x00080000;
      const signed int MMX   = 0x00090000;
      const signed int CTL   = 0x000A0000;
      const signed int DBG   = 0x000B0000;
      const signed int TST   = 0x000C0000;
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
      DEF_REGISTER(k0,    0x00    | OCT   | KMASK| Arch_x86_64, "x86_64");
      DEF_REGISTER(k1,    0x01    | OCT   | KMASK| Arch_x86_64, "x86_64");
      DEF_REGISTER(k2,    0x02    | OCT   | KMASK| Arch_x86_64, "x86_64");
      DEF_REGISTER(k3,    0x03    | OCT   | KMASK| Arch_x86_64, "x86_64");
      DEF_REGISTER(k4,    0x04    | OCT   | KMASK| Arch_x86_64, "x86_64");
      DEF_REGISTER(k5,    0x05    | OCT   | KMASK| Arch_x86_64, "x86_64");
      DEF_REGISTER(k6,    0x06    | OCT   | KMASK| Arch_x86_64, "x86_64");
      DEF_REGISTER(k7,    0x07    | OCT   | KMASK| Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm0,  0x00    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm1,  0x01    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm2,  0x02    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm3,  0x03    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm4,  0x04    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm5,  0x05    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm6,  0x06    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm7,  0x07    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm8,  0x08    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm9,  0x09    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm10, 0x0A    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm11, 0x0B    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm12, 0x0C    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm13, 0x0D    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm14, 0x0E    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm15, 0x0F    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm16, 0x10    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm17, 0x11    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm18, 0x12    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm19, 0x13    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm20, 0x14    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm21, 0x15    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm22, 0x16    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm23, 0x17    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm24, 0x18    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm25, 0x19    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm26, 0x1A    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm27, 0x1B    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm28, 0x1C    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm29, 0x1D    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm30, 0x1E    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(xmm31, 0x1F    | OCT   | XMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm0,  0x00    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm1,  0x01    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm2,  0x02    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm3,  0x03    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm4,  0x04    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm5,  0x05    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm6,  0x06    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm7,  0x07    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm8,  0x08    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm9,  0x09    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm10, 0x0A    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm11, 0x0B    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm12, 0x0C    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm13, 0x0D    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm14, 0x0E    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm15, 0x0F    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm16, 0x10    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm17, 0x11    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm18, 0x12    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm19, 0x13    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm20, 0x14    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm21, 0x15    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm22, 0x16    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm23, 0x17    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm24, 0x18    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm25, 0x19    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm26, 0x1A    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm27, 0x1B    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm28, 0x1C    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm29, 0x1D    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm30, 0x1E    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(ymm31, 0x1F    | YMMS  | YMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm0,  0x00    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm1,  0x01    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm2,  0x02    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm3,  0x03    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm4,  0x04    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm5,  0x05    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm6,  0x06    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm7,  0x07    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm8,  0x08    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm9,  0x09    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm10, 0x0A    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm11, 0x0B    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm12, 0x0C    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm13, 0x0D    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm14, 0x0E    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm15, 0x0F    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm16, 0x10    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm17, 0x11    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm18, 0x12    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm19, 0x13    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm20, 0x14    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm21, 0x15    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm22, 0x16    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm23, 0x17    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm24, 0x18    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm25, 0x19    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm26, 0x1A    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm27, 0x1B    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm28, 0x1C    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm29, 0x1D    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm30, 0x1E    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
      DEF_REGISTER(zmm31, 0x1F    | ZMMS  | ZMM  | Arch_x86_64, "x86_64");
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
      DEF_REGISTER(amr,     13 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(dscr,    17 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(dsisr,   18 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(dar,     19 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(dec,     22 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(sdr1,    25 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(srr0,    26 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(srr1,    27 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cfar,    28 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(amr_pri, 29 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(pid,     48 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(gdecar,  53 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(decar,   54 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(mcivpr,  55 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(lper,    56 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(lperu,   57 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(csrr0,   58 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(csrr1,   59 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(gtsrwr,  60 | SPR | Arch_ppc32, "ppc32");
//      DEF_REGISTER(iamr,    61 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(esr,     62 | SPR | Arch_ppc32, "ppc32");
//      DEF_REGISTER(ivpr,    66 | SPR | Arch_ppc32, "ppc32");

      DEF_REGISTER(vrsave, 256 | SPR | Arch_ppc32, "ppc32");


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
      DEF_REGISTER(cr0l,   700 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr0g,   701 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr0e,   702 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr0s,   703 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr1l,   704 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr1g,   705 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr1e,   706 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr1s,   707 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr2l,   708 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr2g,   709 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr2e,   710 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr2s,   711 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr3l,   712 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr3g,   713 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr3e,   714 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr3s,   715 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr4l,   716 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr4g,   717 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr4e,   718 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr4s,   719 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr5l,   720 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr5g,   721 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr5e,   722 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr5s,   723 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr6l,   724 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr6g,   725 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr6e,   726 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr6s,   727 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr7l,   728 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr7g,   729 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr7e,   730 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(cr7s,   731 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(ppr,    896 | SPR | Arch_ppc32, "ppc32");
      DEF_REGISTER(ppr32,  898 | SPR | Arch_ppc32, "ppc32");


   }
   namespace ppc64 {
      const signed int GPR   = 0x00010000;
      const signed int FPR   = 0x00020000;
      const signed int FSR   = 0x00040000;
      const signed int SPR   = 0x00080000;
      const signed int VSR   = 0x00000000;

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
      DEF_REGISTER(vrsave, 256 | SPR | Arch_ppc64, "ppc64");
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
      DEF_REGISTER(cr0l,   700 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr0g,   701 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr0e,   702 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr0s,   703 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr1l,   704 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr1g,   705 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr1e,   706 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr1s,   707 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr2l,   708 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr2g,   709 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr2e,   710 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr2s,   711 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr3l,   712 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr3g,   713 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr3e,   714 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr3s,   715 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr4l,   716 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr4g,   717 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr4e,   718 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr4s,   719 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr5l,   720 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr5g,   721 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr5e,   722 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr5s,   723 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr6l,   724 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr6g,   725 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr6e,   726 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr6s,   727 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr7l,   728 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr7g,   729 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr7e,   730 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(cr7s,   731 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(ppr,    896 | SPR | Arch_ppc64, "ppc64");
      DEF_REGISTER(ppr32,  898 | SPR | Arch_ppc64, "ppc64");

      DEF_REGISTER(vsr0,   0   | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr1,   1   | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr2,   2   | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr3,   3   | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr4,   4   | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr5,   5   | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr6,   6   | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr7,   7   | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr8,   8   | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr9,   9   | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr10,  10  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr11,  11  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr12,  12  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr13,  13  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr14,  14  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr15,  15  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr16,  16  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr17,  17  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr18,  18  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr19,  19  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr20,  20  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr21,  21  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr22,  22  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr23,  23  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr24,  24  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr25,  25  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr26,  26  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr27,  27  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr28,  28  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr29,  29  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr30,  30  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr31,  31  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr32,  32  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr33,  33  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr34,  34  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr35,  35  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr36,  36  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr37,  37  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr38,  38  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr39,  39  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr40,  40  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr41,  41  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr42,  42  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr43,  43  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr44,  44  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr45,  45  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr46,  46  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr47,  47  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr48,  48  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr49,  49  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr50,  50  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr51,  51  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr52,  52  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr53,  53  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr54,  54  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr55,  55  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr56,  56  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr57,  57  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr58,  58  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr59,  59  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr60,  60  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr61,  61  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr62,  62  | VSR | Arch_ppc64, "ppc64");
      DEF_REGISTER(vsr63,  63  | VSR | Arch_ppc64, "ppc64");
  
  
   

   }

	namespace aarch64{
      //0xff000000  0x00ff0000      0x0000ff00      0x000000ff
      //arch        reg cat:GPR     alias&subrange  reg ID
      const signed int GPR   = 0x00010000;
      const signed int FPR   = 0x00020000;
      const signed int FLAG  = 0x00030000;
      const signed int FSR   = 0x00040000;
      const signed int SPR   = 0x00080000;
      const signed int SYSREG = 0x00100000;

      const signed int BIT   = 0x00008000;
      const signed int B_REG = 0x00000100;      //8bit  byte reg
      const signed int W_REG = 0x00000300;      //16bit half-wor reg
      const signed int D_REG = 0x00000f00;      //32bit single-word reg
      const signed int FULL  = 0x00000000;      //64bit double-word reg
      const signed int Q_REG = 0x00000400;      //128bit reg
      const signed int HQ_REG = 0x00000500;      //second 64bit in 128bit reg

      //31 GPRs, double word long registers
      //          (name   regID| alias | cat | arch           arch    )
      DEF_REGISTER(x0,       0 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w0,       0 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x1,       1 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w1,       1 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x2,       2 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w2,       2 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x3,       3 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w3,       3 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x4,       4 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w4,       4 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x5,       5 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w5,       5 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x6,       6 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w6,       6 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x7,       7 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w7,       7 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x8,       8 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w8,       8 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x9,       9 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w9,       9 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x10,     10 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w10,     10 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x11,     11 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w11,     11 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x12,     12 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w12,     12 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x13,     13 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w13,     13 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x14,     14 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w14,     14 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x15,     15 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w15,     15 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x16,     16 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w16,     16 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x17,     17 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w17,     17 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x18,     18 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w18,     18 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x19,     19 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w19,     19 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x20,     20 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w20,     20 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x21,     21 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w21,     21 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x22,     22 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w22,     22 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x23,     23 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w23,     23 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x24,     24 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w24,     24 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x25,     25 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w25,     25 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x26,     26 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w26,     26 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x27,     27 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w27,     27 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x28,     28 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w28,     28 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x29,     29 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w29,     29 | D_REG  |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(x30,     30 | FULL   |GPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(w30,     30 | D_REG  |GPR | Arch_aarch64, "aarch64");

      //32 FPRs-----------q-d-s-h-b
      //128 bit
      DEF_REGISTER(q0,       0 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q1,       1 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q2,       2 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q3,       3 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q4,       4 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q5,       5 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q6,       6 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q7,       7 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q8,       8 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q9,       9 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q10,     10 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q11,     11 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q12,     12 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q13,     13 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q14,     14 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q15,     15 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q16,     16 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q17,     17 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q18,     18 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q19,     19 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q20,     20 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q21,     21 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q22,     22 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q23,     23 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q24,     24 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q25,     25 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q26,     26 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q27,     27 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q28,     28 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q29,     29 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q30,     30 | Q_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(q31,     31 | Q_REG |FPR | Arch_aarch64, "aarch64");

      // second 64bit
      DEF_REGISTER(hq0,       0 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq1,       1 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq2,       2 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq3,       3 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq4,       4 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq5,       5 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq6,       6 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq7,       7 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq8,       8 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq9,       9 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq10,     10 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq11,     11 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq12,     12 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq13,     13 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq14,     14 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq15,     15 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq16,     16 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq17,     17 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq18,     18 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq19,     19 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq20,     20 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq21,     21 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq22,     22 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq23,     23 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq24,     24 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq25,     25 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq26,     26 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq27,     27 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq28,     28 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq29,     29 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq30,     30 | HQ_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(hq31,     31 | HQ_REG |FPR | Arch_aarch64, "aarch64");

      //64bit FP regs
      DEF_REGISTER(d0,       0 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d1,       1 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d2,       2 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d3,       3 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d4,       4 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d5,       5 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d6,       6 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d7,       7 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d8,       8 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d9,       9 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d10,     10 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d11,     11 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d12,     12 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d13,     13 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d14,     14 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d15,     15 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d16,     16 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d17,     17 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d18,     18 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d19,     19 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d20,     20 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d21,     21 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d22,     22 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d23,     23 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d24,     24 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d25,     25 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d26,     26 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d27,     27 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d28,     28 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d29,     29 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d30,     30 | FULL |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(d31,     31 | FULL |FPR | Arch_aarch64, "aarch64");

      //32 bit FP regs
      DEF_REGISTER(s0,       0 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s1,       1 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s2,       2 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s3,       3 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s4,       4 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s5,       5 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s6,       6 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s7,       7 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s8,       8 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s9,       9 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s10,     10 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s11,     11 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s12,     12 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s13,     13 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s14,     14 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s15,     15 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s16,     16 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s17,     17 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s18,     18 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s19,     19 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s20,     20 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s21,     21 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s22,     22 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s23,     23 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s24,     24 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s25,     25 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s26,     26 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s27,     27 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s28,     28 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s29,     29 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s30,     30 | D_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(s31,     31 | D_REG |FPR | Arch_aarch64, "aarch64");


      //16 bit FP regs
      DEF_REGISTER(h0,       0 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h1,       1 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h2,       2 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h3,       3 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h4,       4 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h5,       5 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h6,       6 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h7,       7 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h8,       8 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h9,       9 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h10,     10 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h11,     11 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h12,     12 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h13,     13 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h14,     14 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h15,     15 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h16,     16 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h17,     17 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h18,     18 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h19,     19 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h20,     20 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h21,     21 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h22,     22 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h23,     23 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h24,     24 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h25,     25 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h26,     26 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h27,     27 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h28,     28 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h29,     29 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h30,     30 | W_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(h31,     31 | W_REG |FPR | Arch_aarch64, "aarch64");

      //8 bit FP regs
      DEF_REGISTER(b0,       0 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b1,       1 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b2,       2 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b3,       3 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b4,       4 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b5,       5 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b6,       6 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b7,       7 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b8,       8 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b9,       9 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b10,     10 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b11,     11 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b12,     12 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b13,     13 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b14,     14 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b15,     15 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b16,     16 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b17,     17 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b18,     18 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b19,     19 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b20,     20 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b21,     21 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b22,     22 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b23,     23 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b24,     24 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b25,     25 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b26,     26 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b27,     27 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b28,     28 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b29,     29 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b30,     30 | B_REG |FPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(b31,     31 | B_REG |FPR | Arch_aarch64, "aarch64");

#include "aarch64_sys_regs.h"

      //GPRs aliases:
      //by convention
      //x29 is used as frame pointer
	  //x30 is the linking register
      //x31 can be sp or zero register depending on the context

      //special registers
	  //PC is not writable in aarch64
      const signed int N_FLAG   =   31;
      const signed int Z_FLAG   =   30;
      const signed int C_FLAG   =   29;
      const signed int V_FLAG   =   28;

      DEF_REGISTER(sp,       31| FULL   |SPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(wsp,      0 | D_REG  |SPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(pc,       32| FULL   |SPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(pstate,   2 | D_REG  |SPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(xzr,		 3 | FULL   |SPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(n,   N_FLAG | BIT    |FLAG| Arch_aarch64, "aarch64");
      DEF_REGISTER(z,   Z_FLAG | BIT    |FLAG| Arch_aarch64, "aarch64");
      DEF_REGISTER(c,   C_FLAG | BIT    |FLAG| Arch_aarch64, "aarch64");
      DEF_REGISTER(v,   V_FLAG | BIT    |FLAG| Arch_aarch64, "aarch64");
      DEF_REGISTER(wzr,		 3 | D_REG  |SPR | Arch_aarch64, "aarch64");
      DEF_REGISTER(fpcr,     4 | D_REG  |SPR | Arch_aarch64, "aarch64");
     DEF_REGISTER(fpsr,     5 | D_REG  |SPR | Arch_aarch64, "aarch64");

	}	//end of aarch64 namespace
	namespace amdgpu_vega{
      //0xff000000  0x00ff0000      0x0000ff00      0x000000ff
      //arch        reg cat:GPR     alias&subrange  reg ID
      const signed int SGPR           = 0x00010000;
      const signed int SGPR_VEC2      = 0x00020000;
      const signed int SGPR_VEC4      = 0x00030000;
      const signed int SGPR_VEC8      = 0x00040000;
      const signed int SGPR_VEC16     = 0x00050000;

      const signed int VGPR           = 0x00060000;
      const signed int VGPR_VEC2      = 0x00070000;
      const signed int VGPR_VEC4      = 0x00080000;
      const signed int VGPR_VEC8      = 0x00090000;
      const signed int VGPR_VEC16     = 0x000A0000;
      
      const signed int HWR            = 0x000B0000;
      const signed int TTMP_SGPR      = 0x000C0000;
      const signed int FLAGS          = 0x000D0000;
      const signed int PC             = 0x000E0000;
      const signed int SYSREG         = 0x00100000;

      // aliasing for flags
      // if we found out that it is a flag, we no longer need to use the cat  0x00ff0000
      // so we use thhat part to encode the low offset in the base register
      //



      const signed int BITS_1       = 0x00000100;
      const signed int BITS_2       = 0x00000200;
      const signed int BITS_3       = 0x00000300;
      const signed int BITS_4       = 0x00000400;
      const signed int BITS_6       = 0x00000500;
      const signed int BITS_7       = 0x00000600;
      const signed int BITS_8       = 0x00000700;
      const signed int BITS_9       = 0x00000800;
      const signed int BITS_15      = 0x00000900;
      const signed int BITS_16      = 0x00000A00;
      const signed int BITS_32      = 0x00000B00;
      const signed int BITS_48      = 0x00000C00;
      const signed int BITS_64      = 0x00000D00;
      const signed int BITS_128     = 0x00000E00;
      const signed int BITS_256     = 0x00000F00;
      const signed int BITS_512     = 0x00001000;




      /*const signed int BIT     = 0x00001000;
      const signed int D_BIT   = 0x00002000;
      const signed int T_BIT   = 0x00003000;
      const signed int Q_BIT   = 0x00004000;
      const signed int H_BIT   = 0x00006000;
      const signed int S_BIT   = 0x00007000;
      const signed int O_BIT   = 0x00008000;
      const signed int N_BIT   = 0x00009000;
      const signed int D_REG_BIT   = 0x0000A000;

      const signed int B_REG   = 0x00000100;      //8bit  byte reg
      const signed int W_REG   = 0x00000200;      //16bit half-wor reg
      const signed int D_REG   = 0x00000300;      //32bit single-word reg
      const signed int FE_REG  = 0x00000400;     //48bit reg
      const signed int FULL    = 0x00000500;      //64bit double-word reg
      const signed int Q_REG   = 0x00000600;      //128bit reg
      const signed int YMMS    = 0x00000700;       //256bit reg
      const signed int ZMMS    = 0x00000800;       //512bit reg
      const signed int HQ_REG  = 0x00000900;      //second 64bit in 128bit reg*/

      #include "amdgpu_vega_sys_regs.h"
    }


  namespace cuda {
    const signed int GPR   = 0x00000000;
    const signed int PR    = 0x00010000;
    const signed int BR    = 0x00020000;
    const signed int UR    = 0x00040000;
    const signed int UPR   = 0x00080000;

    // General purpose registers
    DEF_REGISTER(r0,       0 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r1,       1 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r2,       2 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r3,       3 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r4,       4 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r5,       5 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r6,       6 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r7,       7 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r8,       8 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r9,       9 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r10,     10 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r11,     11 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r12,     12 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r13,     13 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r14,     14 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r15,     15 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r16,     16 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r17,     17 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r18,     18 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r19,     19 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r20,     20 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r21,     21 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r22,     22 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r23,     23 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r24,     24 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r25,     25 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r26,     26 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r27,     27 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r28,     28 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r29,     29 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r30,     30 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r31,     31 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r32,     32 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r33,     33 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r34,     34 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r35,     35 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r36,     36 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r37,     37 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r38,     38 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r39,     39 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r40,     40 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r41,     41 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r42,     42 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r43,     43 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r44,     44 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r45,     45 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r46,     46 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r47,     47 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r48,     48 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r49,     49 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r50,     50 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r51,     51 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r52,     52 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r53,     53 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r54,     54 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r55,     55 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r56,     56 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r57,     57 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r58,     58 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r59,     59 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r60,     60 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r61,     61 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r62,     62 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r63,     63 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r64,     64 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r65,     65 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r66,     66 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r67,     67 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r68,     68 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r69,     69 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r70,     70 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r71,     71 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r72,     72 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r73,     73 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r74,     74 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r75,     75 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r76,     76 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r77,     77 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r78,     78 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r79,     79 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r80,     80 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r81,     81 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r82,     82 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r83,     83 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r84,     84 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r85,     85 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r86,     86 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r87,     87 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r88,     88 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r89,     89 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r90,     90 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r91,     91 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r92,     92 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r93,     93 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r94,     94 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r95,     95 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r96,     96 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r97,     97 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r98,     98 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r99,     99 | GPR | Arch_cuda, "cuda");
    DEF_REGISTER(r100,    100 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r101,    101 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r102,    102 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r103,    103 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r104,    104 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r105,    105 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r106,    106 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r107,    107 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r108,    108 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r109,    109 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r110,    110 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r111,    111 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r112,    112 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r113,    113 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r114,    114 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r115,    115 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r116,    116 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r117,    117 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r118,    118 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r119,    119 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r120,    120 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r121,    121 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r122,    122 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r123,    123 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r124,    124 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r125,    125 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r126,    126 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r127,    127 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r128,    128 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r129,    129 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r130,    130 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r131,    131 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r132,    132 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r133,    133 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r134,    134 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r135,    135 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r136,    136 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r137,    137 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r138,    138 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r139,    139 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r140,    140 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r141,    141 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r142,    142 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r143,    143 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r144,    144 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r145,    145 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r146,    146 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r147,    147 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r148,    148 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r149,    149 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r150,    150 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r151,    151 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r152,    152 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r153,    153 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r154,    154 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r155,    155 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r156,    156 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r157,    157 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r158,    158 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r159,    159 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r160,    160 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r161,    161 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r162,    162 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r163,    163 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r164,    164 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r165,    165 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r166,    166 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r167,    167 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r168,    168 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r169,    169 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r170,    170 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r171,    171 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r172,    172 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r173,    173 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r174,    174 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r175,    175 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r176,    176 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r177,    177 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r178,    178 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r179,    179 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r180,    180 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r181,    181 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r182,    182 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r183,    183 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r184,    184 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r185,    185 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r186,    186 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r187,    187 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r188,    188 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r189,    189 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r190,    190 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r191,    191 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r192,    192 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r193,    193 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r194,    194 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r195,    195 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r196,    196 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r197,    197 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r198,    198 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r199,    199 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r200,    200 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r201,    201 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r202,    202 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r203,    203 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r204,    204 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r205,    205 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r206,    206 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r207,    207 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r208,    208 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r209,    209 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r210,    210 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r211,    211 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r212,    212 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r213,    213 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r214,    214 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r215,    215 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r216,    216 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r217,    217 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r218,    218 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r219,    219 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r220,    220 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r221,    221 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r222,    222 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r223,    223 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r224,    224 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r225,    225 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r226,    226 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r227,    227 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r228,    228 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r229,    229 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r230,    230 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r231,    231 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r232,    232 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r233,    233 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r234,    234 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r235,    235 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r236,    236 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r237,    237 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r238,    238 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r239,    239 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r240,    240 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r241,    241 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r242,    242 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r243,    243 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r244,    244 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r245,    245 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r246,    246 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r247,    247 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r248,    248 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r249,    249 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r250,    250 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r251,    251 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r252,    252 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r253,    253 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r254,    254 | GPR| Arch_cuda, "cuda");
    DEF_REGISTER(r255,    255 | GPR| Arch_cuda, "cuda");

    // uniform registers
    DEF_REGISTER(ur0,       0 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur1,       1 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur2,       2 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur3,       3 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur4,       4 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur5,       5 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur6,       6 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur7,       7 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur8,       8 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur9,       9 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur10,     10 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur11,     11 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur12,     12 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur13,     13 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur14,     14 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur15,     15 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur16,     16 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur17,     17 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur18,     18 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur19,     19 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur20,     20 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur21,     21 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur22,     22 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur23,     23 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur24,     24 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur25,     25 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur26,     26 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur27,     27 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur28,     28 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur29,     29 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur30,     30 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur31,     31 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur32,     32 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur33,     33 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur34,     34 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur35,     35 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur36,     36 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur37,     37 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur38,     38 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur39,     39 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur40,     40 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur41,     41 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur42,     42 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur43,     43 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur44,     44 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur45,     45 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur46,     46 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur47,     47 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur48,     48 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur49,     49 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur50,     50 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur51,     51 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur52,     52 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur53,     53 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur54,     54 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur55,     55 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur56,     56 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur57,     57 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur58,     58 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur59,     59 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur60,     60 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur61,     61 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur62,     62 | UR | Arch_cuda, "cuda");
    DEF_REGISTER(ur63,     63 | UR | Arch_cuda, "cuda");

    // Placeholder for a pc register, so that we don't assert
    DEF_REGISTER(pc,    256 | GPR| Arch_cuda, "cuda");

    // Predicate registers used as source or dest operands
    // Different from a predicate register used as instruction predicate,
    // which is handle by operand::isTruePredicate and operand::isFalsePredicate
    DEF_REGISTER(p0,    0 | PR | Arch_cuda, "cuda");
    DEF_REGISTER(p1,    1 | PR | Arch_cuda, "cuda");
    DEF_REGISTER(p2,    2 | PR | Arch_cuda, "cuda");
    DEF_REGISTER(p3,    3 | PR | Arch_cuda, "cuda");
    DEF_REGISTER(p4,    4 | PR | Arch_cuda, "cuda");
    DEF_REGISTER(p5,    5 | PR | Arch_cuda, "cuda");
    DEF_REGISTER(p6,    6 | PR | Arch_cuda, "cuda");

    DEF_REGISTER(b1,    1 | BR | Arch_cuda, "cuda");
    DEF_REGISTER(b2,    2 | BR | Arch_cuda, "cuda");
    DEF_REGISTER(b3,    3 | BR | Arch_cuda, "cuda");
    DEF_REGISTER(b4,    4 | BR | Arch_cuda, "cuda");
    DEF_REGISTER(b5,    5 | BR | Arch_cuda, "cuda");
    DEF_REGISTER(b6,    6 | BR | Arch_cuda, "cuda");

    // XXX(Keren): not sure how many uprs, use 16 for safety
    DEF_REGISTER(up0,    0 | UPR | Arch_cuda, "cuda");
    DEF_REGISTER(up1,    1 | UPR | Arch_cuda, "cuda");
    DEF_REGISTER(up2,    2 | UPR | Arch_cuda, "cuda");
    DEF_REGISTER(up3,    3 | UPR | Arch_cuda, "cuda");
    DEF_REGISTER(up4,    4 | UPR | Arch_cuda, "cuda");
    DEF_REGISTER(up5,    5 | UPR | Arch_cuda, "cuda");
    DEF_REGISTER(up6,    6 | UPR | Arch_cuda, "cuda");
    DEF_REGISTER(up7,    7 | UPR | Arch_cuda, "cuda");
    DEF_REGISTER(up8,    8 | UPR | Arch_cuda, "cuda");
    DEF_REGISTER(up9,    9 | UPR | Arch_cuda, "cuda");
    DEF_REGISTER(up10,  10 | UPR | Arch_cuda, "cuda");
    DEF_REGISTER(up11,  11 | UPR | Arch_cuda, "cuda");
    DEF_REGISTER(up12,  12 | UPR | Arch_cuda, "cuda");
    DEF_REGISTER(up13,  13 | UPR | Arch_cuda, "cuda");
    DEF_REGISTER(up14,  14 | UPR | Arch_cuda, "cuda");
    DEF_REGISTER(up15,  15 | UPR | Arch_cuda, "cuda");
  } //end of cuda namespace
}

#endif
