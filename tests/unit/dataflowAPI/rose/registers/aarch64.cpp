#include "external/rose/armv8InstructionEnum.h"
#include "registers/aarch64_regs.h"
#include "rose_reg_check.h"

int main() {
  ROSEREG_CHECK(Dyninst::aarch64::pc, armv8_regclass_pc, 0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::pc.getBaseRegister(), armv8_regclass_pc, 0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::xzr, armv8_regclass_gpr, armv8_gpr_zr, 0);
  ROSEREG_CHECK(Dyninst::aarch64::xzr.getBaseRegister(), armv8_regclass_gpr, armv8_gpr_zr, 0);
  ROSEREG_CHECK(Dyninst::aarch64::wzr, armv8_regclass_gpr, armv8_gpr_zr, 0);
  ROSEREG_CHECK(Dyninst::aarch64::wzr.getBaseRegister(), armv8_regclass_gpr, armv8_gpr_zr, 0);

  ROSEREG_CHECK(Dyninst::aarch64::sp, armv8_regclass_sp, 0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::sp.getBaseRegister(), armv8_regclass_sp, 0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::nzcv, armv8_regclass_pstate, 0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::nzcv.getBaseRegister(), armv8_regclass_pstate, 0, 0);

  ROSEREG_CHECK(Dyninst::aarch64::w0, armv8_regclass_gpr, armv8_gpr_r0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::w0.getBaseRegister(), armv8_regclass_gpr, armv8_gpr_r0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::w30, armv8_regclass_gpr, armv8_gpr_r30, 0);
  ROSEREG_CHECK(Dyninst::aarch64::w30.getBaseRegister(), armv8_regclass_gpr, armv8_gpr_r30, 0);
  ROSEREG_CHECK(Dyninst::aarch64::x0, armv8_regclass_gpr, armv8_gpr_r0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::x0.getBaseRegister(), armv8_regclass_gpr, armv8_gpr_r0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::x30, armv8_regclass_gpr, armv8_gpr_r30, 0);
  ROSEREG_CHECK(Dyninst::aarch64::x30.getBaseRegister(), armv8_regclass_gpr, armv8_gpr_r30, 0);

  ROSEREG_CHECK(Dyninst::aarch64::b0, armv8_regclass_simd_fpr, armv8_simdfpr_v0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::b0.getBaseRegister(), armv8_regclass_simd_fpr, armv8_simdfpr_v0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::b30, armv8_regclass_simd_fpr, armv8_simdfpr_v30, 0);
  ROSEREG_CHECK(Dyninst::aarch64::b30.getBaseRegister(), armv8_regclass_simd_fpr, armv8_simdfpr_v30, 0);
  ROSEREG_CHECK(Dyninst::aarch64::d0, armv8_regclass_simd_fpr, armv8_simdfpr_v0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::d0.getBaseRegister(), armv8_regclass_simd_fpr, armv8_simdfpr_v0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::d30, armv8_regclass_simd_fpr, armv8_simdfpr_v30, 0);
  ROSEREG_CHECK(Dyninst::aarch64::d30.getBaseRegister(), armv8_regclass_simd_fpr, armv8_simdfpr_v30, 0);
  ROSEREG_CHECK(Dyninst::aarch64::h0, armv8_regclass_simd_fpr, armv8_simdfpr_v0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::h0.getBaseRegister(), armv8_regclass_simd_fpr, armv8_simdfpr_v0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::h30, armv8_regclass_simd_fpr, armv8_simdfpr_v30, 0);
  ROSEREG_CHECK(Dyninst::aarch64::h30.getBaseRegister(), armv8_regclass_simd_fpr, armv8_simdfpr_v30, 0);
  ROSEREG_CHECK(Dyninst::aarch64::q0, armv8_regclass_simd_fpr, armv8_simdfpr_v0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::q0.getBaseRegister(), armv8_regclass_simd_fpr, armv8_simdfpr_v0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::q30, armv8_regclass_simd_fpr, armv8_simdfpr_v30, 0);
  ROSEREG_CHECK(Dyninst::aarch64::q30.getBaseRegister(), armv8_regclass_simd_fpr, armv8_simdfpr_v30, 0);
  ROSEREG_CHECK(Dyninst::aarch64::s0, armv8_regclass_simd_fpr, armv8_simdfpr_v0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::s0.getBaseRegister(), armv8_regclass_simd_fpr, armv8_simdfpr_v0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::s30, armv8_regclass_simd_fpr, armv8_simdfpr_v30, 0);
  ROSEREG_CHECK(Dyninst::aarch64::s30.getBaseRegister(), armv8_regclass_simd_fpr, armv8_simdfpr_v30, 0);
  ROSEREG_CHECK(Dyninst::aarch64::hq0, armv8_regclass_simd_fpr, armv8_simdfpr_v0, 64);
  ROSEREG_CHECK(Dyninst::aarch64::hq0.getBaseRegister(), armv8_regclass_simd_fpr, armv8_simdfpr_v0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::hq30, armv8_regclass_simd_fpr, armv8_simdfpr_v30, 64);
  ROSEREG_CHECK(Dyninst::aarch64::hq30.getBaseRegister(), armv8_regclass_simd_fpr, armv8_simdfpr_v30, 0);

  ROSEREG_CHECK(Dyninst::aarch64::n, armv8_regclass_pstate, 0, armv8_pstatefield_n);
  ROSEREG_CHECK(Dyninst::aarch64::n.getBaseRegister(), armv8_regclass_pstate, 0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::z, armv8_regclass_pstate, 0, armv8_pstatefield_z);
  ROSEREG_CHECK(Dyninst::aarch64::z.getBaseRegister(), armv8_regclass_pstate, 0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::c, armv8_regclass_pstate, 0, armv8_pstatefield_c);
  ROSEREG_CHECK(Dyninst::aarch64::c.getBaseRegister(), armv8_regclass_pstate, 0, 0);
  ROSEREG_CHECK(Dyninst::aarch64::v, armv8_regclass_pstate, 0, armv8_pstatefield_v);
  ROSEREG_CHECK(Dyninst::aarch64::v.getBaseRegister(), armv8_regclass_pstate, 0, 0);

  return EXIT_SUCCESS;
}
