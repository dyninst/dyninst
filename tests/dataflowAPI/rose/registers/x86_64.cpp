#include "external/rose/rose-compat.h"
#include "registers/x86_64_regs.h"
#include "rose_reg_check.h"

int main() {
  ROSEREG_CHECK(Dyninst::x86_64::eip, x86_regclass_ip, 0, x86_regpos_dword);
  ROSEREG_CHECK(Dyninst::x86_64::eip.getBaseRegister(), x86_regclass_ip, 0, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rip, x86_regclass_ip, 0, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rip.getBaseRegister(), x86_regclass_ip, 0, x86_regpos_qword);

  ROSEREG_CHECK(Dyninst::x86_64::al, x86_regclass_gpr, x86_gpr_ax, x86_regpos_low_byte);
  ROSEREG_CHECK(Dyninst::x86_64::al.getBaseRegister(), x86_regclass_gpr, x86_gpr_ax, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::ah, x86_regclass_gpr, x86_gpr_ax, x86_regpos_high_byte);
  ROSEREG_CHECK(Dyninst::x86_64::ah.getBaseRegister(), x86_regclass_gpr, x86_gpr_ax, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::ax, x86_regclass_gpr, x86_gpr_ax, x86_regpos_word);
  ROSEREG_CHECK(Dyninst::x86_64::ax.getBaseRegister(), x86_regclass_gpr, x86_gpr_ax, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::eax, x86_regclass_gpr, x86_gpr_ax, x86_regpos_dword);
  ROSEREG_CHECK(Dyninst::x86_64::eax.getBaseRegister(), x86_regclass_gpr, x86_gpr_ax, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rax, x86_regclass_gpr, x86_gpr_ax, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rax.getBaseRegister(), x86_regclass_gpr, x86_gpr_ax, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::bl, x86_regclass_gpr, x86_gpr_bx, x86_regpos_low_byte);
  ROSEREG_CHECK(Dyninst::x86_64::bl.getBaseRegister(), x86_regclass_gpr, x86_gpr_bx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::bh, x86_regclass_gpr, x86_gpr_bx, x86_regpos_high_byte);
  ROSEREG_CHECK(Dyninst::x86_64::bh.getBaseRegister(), x86_regclass_gpr, x86_gpr_bx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::bx, x86_regclass_gpr, x86_gpr_bx, x86_regpos_word);
  ROSEREG_CHECK(Dyninst::x86_64::bx.getBaseRegister(), x86_regclass_gpr, x86_gpr_bx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::ebx, x86_regclass_gpr, x86_gpr_bx, x86_regpos_dword);
  ROSEREG_CHECK(Dyninst::x86_64::ebx.getBaseRegister(), x86_regclass_gpr, x86_gpr_bx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rbx, x86_regclass_gpr, x86_gpr_bx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rbx.getBaseRegister(), x86_regclass_gpr, x86_gpr_bx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::cl, x86_regclass_gpr, x86_gpr_cx, x86_regpos_low_byte);
  ROSEREG_CHECK(Dyninst::x86_64::cl.getBaseRegister(), x86_regclass_gpr, x86_gpr_cx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::ch, x86_regclass_gpr, x86_gpr_cx, x86_regpos_high_byte);
  ROSEREG_CHECK(Dyninst::x86_64::ch.getBaseRegister(), x86_regclass_gpr, x86_gpr_cx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::cx, x86_regclass_gpr, x86_gpr_cx, x86_regpos_word);
  ROSEREG_CHECK(Dyninst::x86_64::cx.getBaseRegister(), x86_regclass_gpr, x86_gpr_cx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::ecx, x86_regclass_gpr, x86_gpr_cx, x86_regpos_dword);
  ROSEREG_CHECK(Dyninst::x86_64::ecx.getBaseRegister(), x86_regclass_gpr, x86_gpr_cx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rcx, x86_regclass_gpr, x86_gpr_cx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rcx.getBaseRegister(), x86_regclass_gpr, x86_gpr_cx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::dl, x86_regclass_gpr, x86_gpr_dx, x86_regpos_low_byte);
  ROSEREG_CHECK(Dyninst::x86_64::dl.getBaseRegister(), x86_regclass_gpr, x86_gpr_dx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::dh, x86_regclass_gpr, x86_gpr_dx, x86_regpos_high_byte);
  ROSEREG_CHECK(Dyninst::x86_64::dh.getBaseRegister(), x86_regclass_gpr, x86_gpr_dx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::dx, x86_regclass_gpr, x86_gpr_dx, x86_regpos_word);
  ROSEREG_CHECK(Dyninst::x86_64::dx.getBaseRegister(), x86_regclass_gpr, x86_gpr_dx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::edx, x86_regclass_gpr, x86_gpr_dx, x86_regpos_dword);
  ROSEREG_CHECK(Dyninst::x86_64::edx.getBaseRegister(), x86_regclass_gpr, x86_gpr_dx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rdx, x86_regclass_gpr, x86_gpr_dx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rdx.getBaseRegister(), x86_regclass_gpr, x86_gpr_dx, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::r8b, x86_regclass_gpr, x86_gpr_r8, x86_regpos_low_byte);
  ROSEREG_CHECK(Dyninst::x86_64::r8b.getBaseRegister(), x86_regclass_gpr, x86_gpr_r8, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::r8w, x86_regclass_gpr, x86_gpr_r8, x86_regpos_word);
  ROSEREG_CHECK(Dyninst::x86_64::r8w.getBaseRegister(), x86_regclass_gpr, x86_gpr_r8, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::r8d, x86_regclass_gpr, x86_gpr_r8, x86_regpos_dword);
  ROSEREG_CHECK(Dyninst::x86_64::r8d.getBaseRegister(), x86_regclass_gpr, x86_gpr_r8, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::r8, x86_regclass_gpr, x86_gpr_r8, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::r8.getBaseRegister(), x86_regclass_gpr, x86_gpr_r8, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::r15b, x86_regclass_gpr, x86_gpr_r15, x86_regpos_low_byte);
  ROSEREG_CHECK(Dyninst::x86_64::r15b.getBaseRegister(), x86_regclass_gpr, x86_gpr_r15, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::r15w, x86_regclass_gpr, x86_gpr_r15, x86_regpos_word);
  ROSEREG_CHECK(Dyninst::x86_64::r15w.getBaseRegister(), x86_regclass_gpr, x86_gpr_r15, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::r15d, x86_regclass_gpr, x86_gpr_r15, x86_regpos_dword);
  ROSEREG_CHECK(Dyninst::x86_64::r15d.getBaseRegister(), x86_regclass_gpr, x86_gpr_r15, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::r15, x86_regclass_gpr, x86_gpr_r15, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::r15.getBaseRegister(), x86_regclass_gpr, x86_gpr_r15, x86_regpos_qword);

  ROSEREG_CHECK(Dyninst::x86_64::sp, x86_regclass_gpr, x86_gpr_sp, x86_regpos_word);
  ROSEREG_CHECK(Dyninst::x86_64::sp.getBaseRegister(), x86_regclass_gpr, x86_gpr_sp, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::esp, x86_regclass_gpr, x86_gpr_sp, x86_regpos_dword);
  ROSEREG_CHECK(Dyninst::x86_64::esp.getBaseRegister(), x86_regclass_gpr, x86_gpr_sp, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rsp, x86_regclass_gpr, x86_gpr_sp, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rsp.getBaseRegister(), x86_regclass_gpr, x86_gpr_sp, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::bp, x86_regclass_gpr, x86_gpr_bp, x86_regpos_word);
  ROSEREG_CHECK(Dyninst::x86_64::bp.getBaseRegister(), x86_regclass_gpr, x86_gpr_bp, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::ebp, x86_regclass_gpr, x86_gpr_bp, x86_regpos_dword);
  ROSEREG_CHECK(Dyninst::x86_64::ebp.getBaseRegister(), x86_regclass_gpr, x86_gpr_bp, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rbp, x86_regclass_gpr, x86_gpr_bp, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rbp.getBaseRegister(), x86_regclass_gpr, x86_gpr_bp, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::si, x86_regclass_gpr, x86_gpr_si, x86_regpos_word);
  ROSEREG_CHECK(Dyninst::x86_64::si.getBaseRegister(), x86_regclass_gpr, x86_gpr_si, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::esi, x86_regclass_gpr, x86_gpr_si, x86_regpos_dword);
  ROSEREG_CHECK(Dyninst::x86_64::esi.getBaseRegister(), x86_regclass_gpr, x86_gpr_si, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rsi, x86_regclass_gpr, x86_gpr_si, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rsi.getBaseRegister(), x86_regclass_gpr, x86_gpr_si, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::di, x86_regclass_gpr, x86_gpr_di, x86_regpos_word);
  ROSEREG_CHECK(Dyninst::x86_64::di.getBaseRegister(), x86_regclass_gpr, x86_gpr_di, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::edi, x86_regclass_gpr, x86_gpr_di, x86_regpos_dword);
  ROSEREG_CHECK(Dyninst::x86_64::edi.getBaseRegister(), x86_regclass_gpr, x86_gpr_di, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rdi, x86_regclass_gpr, x86_gpr_di, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::rdi.getBaseRegister(), x86_regclass_gpr, x86_gpr_di, x86_regpos_qword);

  ROSEREG_CHECK(Dyninst::x86_64::cf, x86_regclass_flags, x86_flag_cf, 0);
  ROSEREG_CHECK(Dyninst::x86_64::cf.getBaseRegister(), x86_regclass_flags, x86_flag_cf, 0);

  ROSEREG_CHECK(Dyninst::x86_64::st0, x86_regclass_st, 0, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::st0.getBaseRegister(), x86_regclass_st, 0, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::st7, x86_regclass_st, 7, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::st7.getBaseRegister(), x86_regclass_st, 7, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::mm0, x86_regclass_mm, 0, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::mm0.getBaseRegister(), x86_regclass_st, 0, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::mm7, x86_regclass_mm, 7, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::mm7.getBaseRegister(), x86_regclass_st, 7, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::xmm0, x86_regclass_xmm, 0, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::xmm0.getBaseRegister(), x86_regclass_zmm, 0, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::xmm31, x86_regclass_xmm, 31, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::xmm31.getBaseRegister(), x86_regclass_zmm, 31, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::ymm0, x86_regclass_ymm, 0, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::ymm0.getBaseRegister(), x86_regclass_zmm, 0, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::ymm31, x86_regclass_ymm, 31, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::ymm31.getBaseRegister(), x86_regclass_zmm, 31, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::zmm0, x86_regclass_zmm, 0, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::zmm0.getBaseRegister(), x86_regclass_zmm, 0, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::zmm31, x86_regclass_zmm, 31, x86_regpos_all);
  ROSEREG_CHECK(Dyninst::x86_64::zmm31.getBaseRegister(), x86_regclass_zmm, 31, x86_regpos_all);

  ROSEREG_CHECK(Dyninst::x86_64::cr0, x86_regclass_cr, 0, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::cr0.getBaseRegister(), x86_regclass_cr, 0, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::cr7, x86_regclass_cr, 7, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::cr7.getBaseRegister(), x86_regclass_cr, 7, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::dr0, x86_regclass_dr, 0, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::dr0.getBaseRegister(), x86_regclass_dr, 0, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::dr7, x86_regclass_dr, 7, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::dr7.getBaseRegister(), x86_regclass_dr, 7, x86_regpos_qword);

  ROSEREG_CHECK(Dyninst::x86_64::k0, x86_regclass_kmask, 0, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::k0.getBaseRegister(), x86_regclass_kmask, 0, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::k7, x86_regclass_kmask, 7, x86_regpos_qword);
  ROSEREG_CHECK(Dyninst::x86_64::k7.getBaseRegister(), x86_regclass_kmask, 7, x86_regpos_qword);

  return EXIT_SUCCESS;
}
