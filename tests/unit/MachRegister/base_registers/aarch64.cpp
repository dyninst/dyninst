#include "basereg_check.h"
#include "registers/aarch64_regs.h"

int main() {

  // GPR 32 -> 64
  BASEREG_CHECK(aarch64::w0, aarch64::x0);
  BASEREG_CHECK(aarch64::w8, aarch64::x8);
  BASEREG_CHECK(aarch64::w30, aarch64::x30);

  // GPR 64 -> 64
  BASEREG_CHECK(aarch64::x0, aarch64::x0);
  BASEREG_CHECK(aarch64::x8, aarch64::x8);
  BASEREG_CHECK(aarch64::x30, aarch64::x30);

  // FPR 8 -> 64
  BASEREG_CHECK(aarch64::b0, aarch64::q0);
  BASEREG_CHECK(aarch64::b8, aarch64::q8);
  BASEREG_CHECK(aarch64::b31, aarch64::q31);

  // FPR 16 -> 64
  BASEREG_CHECK(aarch64::h0, aarch64::q0);
  BASEREG_CHECK(aarch64::h8, aarch64::q8);
  BASEREG_CHECK(aarch64::h31, aarch64::q31);

  // FPR 32 -> 64
  BASEREG_CHECK(aarch64::s0, aarch64::q0);
  BASEREG_CHECK(aarch64::s8, aarch64::q8);
  BASEREG_CHECK(aarch64::s31, aarch64::q31);

  // FPR 64 -> 64
  BASEREG_CHECK(aarch64::q0, aarch64::q0);
  BASEREG_CHECK(aarch64::q8, aarch64::q8);
  BASEREG_CHECK(aarch64::q31, aarch64::q31);

  // SPR 32 -> 64
  BASEREG_CHECK(aarch64::wsp, aarch64::sp);
  BASEREG_CHECK(aarch64::wzr, aarch64::xzr);

  // SPR 64 -> 64
  BASEREG_CHECK(aarch64::sp, aarch64::sp);
  BASEREG_CHECK(aarch64::xzr, aarch64::xzr);

  // Condition flags
  BASEREG_CHECK(aarch64::nzcv, aarch64::nzcv);
  BASEREG_CHECK(aarch64::n, aarch64::nzcv);
  BASEREG_CHECK(aarch64::z, aarch64::nzcv);
  BASEREG_CHECK(aarch64::c, aarch64::nzcv);
  BASEREG_CHECK(aarch64::v, aarch64::nzcv);

  // Interrupt Mask Bits
  BASEREG_CHECK(aarch64::daif, aarch64::daif);
  BASEREG_CHECK(aarch64::d, aarch64::daif);
  BASEREG_CHECK(aarch64::a, aarch64::daif);
  BASEREG_CHECK(aarch64::i, aarch64::daif);
  BASEREG_CHECK(aarch64::f_, aarch64::daif);

  // Process state flags
  BASEREG_CHECK(aarch64::pstate, aarch64::pstate);
  BASEREG_CHECK(aarch64::allint, aarch64::pstate);
  BASEREG_CHECK(aarch64::currentel, aarch64::pstate);
  BASEREG_CHECK(aarch64::dit, aarch64::pstate);
  BASEREG_CHECK(aarch64::pan, aarch64::pstate);
  BASEREG_CHECK(aarch64::pm, aarch64::pstate);
  BASEREG_CHECK(aarch64::spsel, aarch64::pstate);
  BASEREG_CHECK(aarch64::ssbs, aarch64::pstate);
  BASEREG_CHECK(aarch64::svcr, aarch64::pstate);
  BASEREG_CHECK(aarch64::tco, aarch64::pstate);
  BASEREG_CHECK(aarch64::uao, aarch64::pstate);

  // Floating-point control flags
  BASEREG_CHECK(aarch64::fpcr, aarch64::fpcr);

  // Aliases
  BASEREG_CHECK(aarch64::fp, aarch64::fp);
  BASEREG_CHECK(aarch64::fp, aarch64::x29);
  BASEREG_CHECK(aarch64::x29, aarch64::fp);
  BASEREG_CHECK(aarch64::x29, aarch64::x29);
  BASEREG_CHECK(aarch64::lr, aarch64::lr);
  BASEREG_CHECK(aarch64::lr, aarch64::x30);
  BASEREG_CHECK(aarch64::x30, aarch64::lr);
  BASEREG_CHECK(aarch64::x30, aarch64::x30);
  BASEREG_CHECK(aarch64::Ip0, aarch64::Ip0);
  BASEREG_CHECK(aarch64::Ip0, aarch64::x16);
  BASEREG_CHECK(aarch64::x16, aarch64::Ip0);
  BASEREG_CHECK(aarch64::x16, aarch64::x16);
  BASEREG_CHECK(aarch64::Ip1, aarch64::Ip1);
  BASEREG_CHECK(aarch64::Ip1, aarch64::x17);
  BASEREG_CHECK(aarch64::x17, aarch64::Ip1);
  BASEREG_CHECK(aarch64::x17, aarch64::x17);

  return EXIT_SUCCESS;
}
