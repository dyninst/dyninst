#include "basereg_check.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"

int main() {
  BASEREG_CHECK(amdgpu_gfx908::ttmp0, amdgpu_gfx908::ttmp0);
  BASEREG_CHECK(amdgpu_gfx908::ttmp15, amdgpu_gfx908::ttmp15);

  BASEREG_CHECK(amdgpu_gfx908::attr0, amdgpu_gfx908::attr0);
  BASEREG_CHECK(amdgpu_gfx908::attr32, amdgpu_gfx908::attr32);

  BASEREG_CHECK(amdgpu_gfx908::s0, amdgpu_gfx908::s0);
  BASEREG_CHECK(amdgpu_gfx908::s101, amdgpu_gfx908::s101);

  BASEREG_CHECK(amdgpu_gfx908::v0, amdgpu_gfx908::v0);
  BASEREG_CHECK(amdgpu_gfx908::v101, amdgpu_gfx908::v101);

  BASEREG_CHECK(amdgpu_gfx908::acc0, amdgpu_gfx908::acc0);
  BASEREG_CHECK(amdgpu_gfx908::acc255, amdgpu_gfx908::acc255);

  BASEREG_CHECK(amdgpu_gfx908::vcc, amdgpu_gfx908::vcc);
  BASEREG_CHECK(amdgpu_gfx908::vcc_lo, amdgpu_gfx908::vcc);
  BASEREG_CHECK(amdgpu_gfx908::vcc_hi, amdgpu_gfx908::vcc);

  BASEREG_CHECK(amdgpu_gfx908::exec, amdgpu_gfx908::exec);
  BASEREG_CHECK(amdgpu_gfx908::exec_lo, amdgpu_gfx908::exec);
  BASEREG_CHECK(amdgpu_gfx908::exec_hi, amdgpu_gfx908::exec);

  BASEREG_CHECK(amdgpu_gfx908::flat_scratch_all, amdgpu_gfx908::flat_scratch_all);
  BASEREG_CHECK(amdgpu_gfx908::flat_scratch_lo, amdgpu_gfx908::flat_scratch_all);
  BASEREG_CHECK(amdgpu_gfx908::flat_scratch_hi, amdgpu_gfx908::flat_scratch_all);

  return EXIT_SUCCESS;
}
