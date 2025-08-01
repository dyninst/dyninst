#include "external/rose/riscv64InstructionEnum.h"
#include "registers/riscv64_regs.h"
#include "rose_reg_check.h"

int main() {
  ROSEREG_CHECK(Dyninst::riscv64::pc, riscv64_regclass_pc, 0, 0);
  ROSEREG_CHECK(Dyninst::riscv64::pc.getBaseRegister(), riscv64_regclass_pc, 0, 0);

  ROSEREG_CHECK(Dyninst::riscv64::x0, riscv64_regclass_gpr, riscv64_gpr_x0, 0);
  ROSEREG_CHECK(Dyninst::riscv64::x0.getBaseRegister(), riscv64_regclass_gpr, riscv64_gpr_x0, 0);
  ROSEREG_CHECK(Dyninst::riscv64::x31, riscv64_regclass_gpr, riscv64_gpr_x31, 0);
  ROSEREG_CHECK(Dyninst::riscv64::x31.getBaseRegister(), riscv64_regclass_gpr, riscv64_gpr_x31, 0);

  ROSEREG_CHECK(Dyninst::riscv64::f0, riscv64_regclass_fpr, riscv64_fpr_f0, 0);
  ROSEREG_CHECK(Dyninst::riscv64::f0.getBaseRegister(), riscv64_regclass_fpr, riscv64_fpr_f0, 0);
  ROSEREG_CHECK(Dyninst::riscv64::f31, riscv64_regclass_fpr, riscv64_fpr_f31, 0);
  ROSEREG_CHECK(Dyninst::riscv64::f31.getBaseRegister(), riscv64_regclass_fpr, riscv64_fpr_f31, 0);

  return EXIT_SUCCESS;
}
