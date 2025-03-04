#include "registers/MachRegister.h"
#include "registers/aarch64_regs.h"
#include "type_query_check.h"
#include <iostream>

static bool is_arithmetic_vector(Dyninst::MachRegister);

constexpr auto arch = Dyninst::Arch_aarch64;

int main() {
  TYPE_QUERIES_CHECK(Dyninst::aarch64::pc, isPC);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getPC(arch), isPC);

  TYPE_QUERIES_CHECK(Dyninst::aarch64::x29, isFramePointer);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::fp, isFramePointer);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getFramePointer(arch), isFramePointer);

  TYPE_QUERIES_CHECK(Dyninst::aarch64::sp, isStackPointer);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::wsp, isStackPointer);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::wsp.getBaseRegister(), isStackPointer);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getStackPointer(arch), isStackPointer);

  TYPE_QUERIES_CHECK(Dyninst::aarch64::w8, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::x8, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getSyscallNumberReg(arch), isSyscallNumberReg);

  TYPE_QUERIES_CHECK(Dyninst::aarch64::w0, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::x0, isSyscallReturnValueReg);

  /*********************************************************************
   *      General Purpose
   *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::aarch64::w0, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::w30, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::x0, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::x30, isGeneralPurpose);

  /*********************************************************************
   *      Flags
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::aarch64::nzcv, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::n, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::z, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::c, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::v, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::nzcv.getBaseRegister(), isFlag);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::n.getBaseRegister(), isFlag);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::z.getBaseRegister(), isFlag);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::c.getBaseRegister(), isFlag);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::v.getBaseRegister(), isFlag);

  /*********************************************************************
   *      Zero Flag
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::aarch64::z, isZeroFlag);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::aarch64::nzcv, isZeroFlag);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::aarch64::n, isZeroFlag);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::aarch64::c, isZeroFlag);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::aarch64::v, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getZeroFlag(arch), isZeroFlag);

  /*********************************************************************
   *      Control/Status
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::aarch64::p0, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::p15, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::fpcr, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::fpsr, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::ffr, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::vg, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zt0, isControlStatus);

  /*********************************************************************
   *      Floating-Point
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::aarch64::b0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::b31, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::d0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::d31, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::h0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::h31, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::q0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::q31, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::s0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::s31, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::hq0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::hq31, isFloatingPoint);

  TYPE_QUERIES_CHECK(Dyninst::aarch64::fpcr, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::fpsr, isFloatingPoint);

  TYPE_QUERIES_CHECK(Dyninst::aarch64::ffr, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::vg, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zt0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::za, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zab0, isFloatingPoint);

  TYPE_QUERIES_CHECK(Dyninst::aarch64::p0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::p15, isFloatingPoint);

  TYPE_QUERIES_CHECK(Dyninst::aarch64::z0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::z31, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zad0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zad7, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zah0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zah1, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zaq0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zaq15, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zas0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zas3, isFloatingPoint);

  /*********************************************************************
   *      Vector
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::aarch64::ffr, isVector);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::vg, isVector);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zt0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::za, isVector);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zab0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::p0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::p15, isVector);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::z0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::z31, isVector);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zad0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zad7, isVector);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zah0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zah1, isVector);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zaq0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zaq15, isVector);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zas0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::aarch64::zas3, isVector);


  /*********************************************************************
   *      Filtering
  *********************************************************************/
  {
    using mr = Dyninst::MachRegister;
    for(auto reg : mr::getAllRegistersForArch(arch)) {
      if(reg.isVector() && !reg.isControlStatus()) {
        TYPE_QUERIES_ASSERT_TRUE(reg, is_arithmetic_vector(reg));
      }
      if(is_arithmetic_vector(reg)) {
        TYPE_QUERIES_ASSERT_TRUE(reg, reg.isVector() && !reg.isControlStatus());
      }
    }
  }

  return EXIT_SUCCESS;
}

static bool is_arithmetic_vector(Dyninst::MachRegister reg) {
  switch(reg) {
    case Dyninst::aarch64::iz0:
    case Dyninst::aarch64::iz1:
    case Dyninst::aarch64::iz2:
    case Dyninst::aarch64::iz3:
    case Dyninst::aarch64::iz4:
    case Dyninst::aarch64::iz5:
    case Dyninst::aarch64::iz6:
    case Dyninst::aarch64::iz7:
    case Dyninst::aarch64::iz8:
    case Dyninst::aarch64::iz9:
    case Dyninst::aarch64::iz10:
    case Dyninst::aarch64::iz11:
    case Dyninst::aarch64::iz12:
    case Dyninst::aarch64::iz13:
    case Dyninst::aarch64::iz14:
    case Dyninst::aarch64::iz15:
    case Dyninst::aarch64::iz16:
    case Dyninst::aarch64::iz17:
    case Dyninst::aarch64::iz18:
    case Dyninst::aarch64::iz19:
    case Dyninst::aarch64::iz20:
    case Dyninst::aarch64::iz21:
    case Dyninst::aarch64::iz22:
    case Dyninst::aarch64::iz23:
    case Dyninst::aarch64::iz24:
    case Dyninst::aarch64::iz25:
    case Dyninst::aarch64::iz26:
    case Dyninst::aarch64::iz27:
    case Dyninst::aarch64::iz28:
    case Dyninst::aarch64::iz29:
    case Dyninst::aarch64::iz30:
    case Dyninst::aarch64::iz31:
    case Dyninst::aarch64::iza:
    case Dyninst::aarch64::izab0:
    case Dyninst::aarch64::izad0:
    case Dyninst::aarch64::izad1:
    case Dyninst::aarch64::izad2:
    case Dyninst::aarch64::izad3:
    case Dyninst::aarch64::izad4:
    case Dyninst::aarch64::izad5:
    case Dyninst::aarch64::izad6:
    case Dyninst::aarch64::izad7:
    case Dyninst::aarch64::izah0:
    case Dyninst::aarch64::izah1:
    case Dyninst::aarch64::izaq0:
    case Dyninst::aarch64::izaq1:
    case Dyninst::aarch64::izaq2:
    case Dyninst::aarch64::izaq3:
    case Dyninst::aarch64::izaq4:
    case Dyninst::aarch64::izaq5:
    case Dyninst::aarch64::izaq6:
    case Dyninst::aarch64::izaq7:
    case Dyninst::aarch64::izaq8:
    case Dyninst::aarch64::izaq9:
    case Dyninst::aarch64::izaq10:
    case Dyninst::aarch64::izaq11:
    case Dyninst::aarch64::izaq12:
    case Dyninst::aarch64::izaq13:
    case Dyninst::aarch64::izaq14:
    case Dyninst::aarch64::izaq15:
    case Dyninst::aarch64::izas0:
    case Dyninst::aarch64::izas1:
    case Dyninst::aarch64::izas2:
    case Dyninst::aarch64::izas3:
      return true;
  }
  return false;
}
