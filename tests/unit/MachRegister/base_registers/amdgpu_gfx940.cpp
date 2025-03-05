#include "basereg_check.h"
#include "registers/AMDGPU/amdgpu_gfx940_regs.h"

int main() {
  BASEREG_CHECK(amdgpu_gfx940::ttmp0, amdgpu_gfx940::ttmp0);
  BASEREG_CHECK(amdgpu_gfx940::ttmp15, amdgpu_gfx940::ttmp15);

  BASEREG_CHECK(amdgpu_gfx940::attr0, amdgpu_gfx940::attr0);
  BASEREG_CHECK(amdgpu_gfx940::attr32, amdgpu_gfx940::attr32);

  BASEREG_CHECK(amdgpu_gfx940::s0, amdgpu_gfx940::s0);
  BASEREG_CHECK(amdgpu_gfx940::s101, amdgpu_gfx940::s101);

  BASEREG_CHECK(amdgpu_gfx940::v0, amdgpu_gfx940::v0);
  BASEREG_CHECK(amdgpu_gfx940::v101, amdgpu_gfx940::v101);

  BASEREG_CHECK(amdgpu_gfx940::acc0, amdgpu_gfx940::acc0);
  BASEREG_CHECK(amdgpu_gfx940::acc255, amdgpu_gfx940::acc255);

  BASEREG_CHECK(amdgpu_gfx940::vcc, amdgpu_gfx940::vcc);
  BASEREG_CHECK(amdgpu_gfx940::vcc_lo, amdgpu_gfx940::vcc);
  BASEREG_CHECK(amdgpu_gfx940::vcc_hi, amdgpu_gfx940::vcc);

  BASEREG_CHECK(amdgpu_gfx940::exec, amdgpu_gfx940::exec);
  BASEREG_CHECK(amdgpu_gfx940::exec_lo, amdgpu_gfx940::exec);
  BASEREG_CHECK(amdgpu_gfx940::exec_hi, amdgpu_gfx940::exec);

  BASEREG_CHECK(amdgpu_gfx940::flat_scratch_all, amdgpu_gfx940::flat_scratch_all);
  BASEREG_CHECK(amdgpu_gfx940::flat_scratch_lo, amdgpu_gfx940::flat_scratch_all);
  BASEREG_CHECK(amdgpu_gfx940::flat_scratch_hi, amdgpu_gfx940::flat_scratch_all);

  return EXIT_SUCCESS;
}
