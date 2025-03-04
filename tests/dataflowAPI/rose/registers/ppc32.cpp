#include "external/rose/powerpcInstructionEnum.h"
#include "registers/ppc32_regs.h"
#include "rose_reg_check.h"

int main() {
  ROSEREG_CHECK(Dyninst::ppc32::r0, powerpc_regclass_gpr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::r0.getBaseRegister(), powerpc_regclass_gpr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::r31, powerpc_regclass_gpr, 31, 0);
  ROSEREG_CHECK(Dyninst::ppc32::r31.getBaseRegister(), powerpc_regclass_gpr, 31, 0);

  ROSEREG_CHECK(Dyninst::ppc32::fpr0, powerpc_regclass_fpr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::fpr0.getBaseRegister(), powerpc_regclass_fpr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::fpr31, powerpc_regclass_fpr, 31, 0);
  ROSEREG_CHECK(Dyninst::ppc32::fpr31.getBaseRegister(), powerpc_regclass_fpr, 31, 0);
  ROSEREG_CHECK(Dyninst::ppc32::fsr0, powerpc_regclass_fpr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::fsr0.getBaseRegister(), powerpc_regclass_fpr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::fsr31, powerpc_regclass_fpr, 31, 0);
  ROSEREG_CHECK(Dyninst::ppc32::fsr31.getBaseRegister(), powerpc_regclass_fpr, 31, 0);

  ROSEREG_CHECK(Dyninst::ppc32::pc, powerpc_regclass_spr, 600, 0);
  ROSEREG_CHECK(Dyninst::ppc32::pc.getBaseRegister(), powerpc_regclass_spr, 600, 0);
  ROSEREG_CHECK(Dyninst::ppc32::sprg0, powerpc_regclass_spr, 272, 0);
  ROSEREG_CHECK(Dyninst::ppc32::sprg0.getBaseRegister(), powerpc_regclass_spr, 272, 0);
  ROSEREG_CHECK(Dyninst::ppc32::sprg7, powerpc_regclass_spr, 279, 0);
  ROSEREG_CHECK(Dyninst::ppc32::sprg7.getBaseRegister(), powerpc_regclass_spr, 279, 0);
  ROSEREG_CHECK(Dyninst::ppc32::sprg3_ro, powerpc_regclass_spr, 259, 0);
  ROSEREG_CHECK(Dyninst::ppc32::sprg3_ro.getBaseRegister(), powerpc_regclass_spr, 259, 0);
  ROSEREG_CHECK(Dyninst::ppc32::sprg7_ro, powerpc_regclass_spr, 263, 0);
  ROSEREG_CHECK(Dyninst::ppc32::sprg7_ro.getBaseRegister(), powerpc_regclass_spr, 263, 0);

  ROSEREG_CHECK(Dyninst::ppc32::seg0, powerpc_regclass_sr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::seg0.getBaseRegister(), powerpc_regclass_sr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::seg7, powerpc_regclass_sr, 0, 7);
  ROSEREG_CHECK(Dyninst::ppc32::seg7.getBaseRegister(), powerpc_regclass_sr, 0, 7);

  ROSEREG_CHECK(Dyninst::ppc32::cr, powerpc_regclass_cr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::cr.getBaseRegister(), powerpc_regclass_cr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::cr0, powerpc_regclass_cr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::cr0.getBaseRegister(), powerpc_regclass_cr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::cr0l, powerpc_regclass_cr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::cr0l.getBaseRegister(), powerpc_regclass_cr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::cr0g, powerpc_regclass_cr, 0, 1);
  ROSEREG_CHECK(Dyninst::ppc32::cr0g.getBaseRegister(), powerpc_regclass_cr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::cr0e, powerpc_regclass_cr, 0, 2);
  ROSEREG_CHECK(Dyninst::ppc32::cr0e.getBaseRegister(), powerpc_regclass_cr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::cr0s, powerpc_regclass_cr, 0, 3);
  ROSEREG_CHECK(Dyninst::ppc32::cr0s.getBaseRegister(), powerpc_regclass_cr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::cr7, powerpc_regclass_cr, 0, 7);
  ROSEREG_CHECK(Dyninst::ppc32::cr7.getBaseRegister(), powerpc_regclass_cr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::cr7l, powerpc_regclass_cr, 0, 28);
  ROSEREG_CHECK(Dyninst::ppc32::cr7l.getBaseRegister(), powerpc_regclass_cr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::cr7g, powerpc_regclass_cr, 0, 29);
  ROSEREG_CHECK(Dyninst::ppc32::cr7g.getBaseRegister(), powerpc_regclass_cr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::cr7e, powerpc_regclass_cr, 0, 30);
  ROSEREG_CHECK(Dyninst::ppc32::cr7e.getBaseRegister(), powerpc_regclass_cr, 0, 0);
  ROSEREG_CHECK(Dyninst::ppc32::cr7s, powerpc_regclass_cr, 0, 31);
  ROSEREG_CHECK(Dyninst::ppc32::cr7s.getBaseRegister(), powerpc_regclass_cr, 0, 0);

  return EXIT_SUCCESS;
}
