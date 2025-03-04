#include "basereg_check.h"
#include "registers/x86_64_regs.h"

int main() {
  BASEREG_CHECK(x86_64::al, x86_64::rax);
  BASEREG_CHECK(x86_64::ah, x86_64::rax);
  BASEREG_CHECK(x86_64::ax, x86_64::rax);
  BASEREG_CHECK(x86_64::eax, x86_64::rax);
  BASEREG_CHECK(x86_64::rax, x86_64::rax);

  BASEREG_CHECK(x86_64::bl, x86_64::rbx);
  BASEREG_CHECK(x86_64::bh, x86_64::rbx);
  BASEREG_CHECK(x86_64::bx, x86_64::rbx);
  BASEREG_CHECK(x86_64::ebx, x86_64::rbx);
  BASEREG_CHECK(x86_64::rbx, x86_64::rbx);

  BASEREG_CHECK(x86_64::cl, x86_64::rcx);
  BASEREG_CHECK(x86_64::ch, x86_64::rcx);
  BASEREG_CHECK(x86_64::cx, x86_64::rcx);
  BASEREG_CHECK(x86_64::ecx, x86_64::rcx);
  BASEREG_CHECK(x86_64::rcx, x86_64::rcx);

  BASEREG_CHECK(x86_64::si, x86_64::rsi);
  BASEREG_CHECK(x86_64::esi, x86_64::rsi);
  BASEREG_CHECK(x86_64::sil, x86_64::rsi);
  BASEREG_CHECK(x86_64::rsi, x86_64::rsi);

  BASEREG_CHECK(x86_64::di, x86_64::rdi);
  BASEREG_CHECK(x86_64::edi, x86_64::rdi);
  BASEREG_CHECK(x86_64::dil, x86_64::rdi);
  BASEREG_CHECK(x86_64::rdi, x86_64::rdi);

  BASEREG_CHECK(x86_64::bp, x86_64::rbp);
  BASEREG_CHECK(x86_64::ebp, x86_64::rbp);
  BASEREG_CHECK(x86_64::bpl, x86_64::rbp);
  BASEREG_CHECK(x86_64::rbp, x86_64::rbp);
  
  BASEREG_CHECK(x86_64::r15b, x86_64::r15);
  BASEREG_CHECK(x86_64::r15w, x86_64::r15);
  BASEREG_CHECK(x86_64::r15d, x86_64::r15);
  BASEREG_CHECK(x86_64::r15, x86_64::r15);

  BASEREG_CHECK(x86_64::eip, x86_64::rip);
  BASEREG_CHECK(x86_64::rip, x86_64::rip);

  BASEREG_CHECK(x86_64::sp, x86_64::rsp);
  BASEREG_CHECK(x86_64::esp, x86_64::rsp);
  BASEREG_CHECK(x86_64::rsp, x86_64::rsp);

  BASEREG_CHECK(x86_64::flags, x86_64::flags);
  BASEREG_CHECK(x86_64::cf, x86_64::flags);
  BASEREG_CHECK(x86_64::flag1, x86_64::flags);
  BASEREG_CHECK(x86_64::pf, x86_64::flags);
  BASEREG_CHECK(x86_64::flag3, x86_64::flags);
  BASEREG_CHECK(x86_64::af, x86_64::flags);
  BASEREG_CHECK(x86_64::flag5, x86_64::flags);
  BASEREG_CHECK(x86_64::zf, x86_64::flags);
  BASEREG_CHECK(x86_64::sf, x86_64::flags);
  BASEREG_CHECK(x86_64::tf, x86_64::flags);
  BASEREG_CHECK(x86_64::if_, x86_64::flags);
  BASEREG_CHECK(x86_64::df, x86_64::flags);
  BASEREG_CHECK(x86_64::of, x86_64::flags);
  BASEREG_CHECK(x86_64::flagc, x86_64::flags);
  BASEREG_CHECK(x86_64::flagd, x86_64::flags);
  BASEREG_CHECK(x86_64::nt_, x86_64::flags);
  BASEREG_CHECK(x86_64::flagf, x86_64::flags);
  BASEREG_CHECK(x86_64::rf, x86_64::flags);
  BASEREG_CHECK(x86_64::vm, x86_64::flags);
  BASEREG_CHECK(x86_64::ac, x86_64::flags);
  BASEREG_CHECK(x86_64::vif, x86_64::flags);
  BASEREG_CHECK(x86_64::vip, x86_64::flags);
  BASEREG_CHECK(x86_64::id, x86_64::flags);

  BASEREG_CHECK(x86_64::mm0, x86_64::st0);
  BASEREG_CHECK(x86_64::mm7, x86_64::st7);

  BASEREG_CHECK(x86_64::xmm0, x86_64::zmm0);
  BASEREG_CHECK(x86_64::xmm31, x86_64::zmm31);

  BASEREG_CHECK(x86_64::ymm0, x86_64::zmm0);
  BASEREG_CHECK(x86_64::ymm31, x86_64::zmm31);

  BASEREG_CHECK(x86_64::zmm0, x86_64::zmm0);
  BASEREG_CHECK(x86_64::zmm31, x86_64::zmm31);

  BASEREG_CHECK(x86_64::ds, x86_64::ds);
  BASEREG_CHECK(x86_64::cr0, x86_64::cr0);
  BASEREG_CHECK(x86_64::dr0, x86_64::dr0);
  BASEREG_CHECK(x86_64::st0, x86_64::st0);
  BASEREG_CHECK(x86_64::k0, x86_64::k0);
  BASEREG_CHECK(x86_64::fcw, x86_64::fcw);
  BASEREG_CHECK(x86_64::fsw, x86_64::fsw);
  BASEREG_CHECK(x86_64::mxcsr, x86_64::mxcsr);

  return EXIT_SUCCESS;
}
