#include "basereg_check.h"
#include "registers/AMDGPU/amdgpu_gfx90a_regs.h"

int main() {
  BASEREG_CHECK(amdgpu_gfx90a::ttmp0, amdgpu_gfx90a::ttmp0);
  BASEREG_CHECK(amdgpu_gfx90a::ttmp15, amdgpu_gfx90a::ttmp15);

  BASEREG_CHECK(amdgpu_gfx90a::attr0, amdgpu_gfx90a::attr0);
  BASEREG_CHECK(amdgpu_gfx90a::attr32, amdgpu_gfx90a::attr32);

  BASEREG_CHECK(amdgpu_gfx90a::s0, amdgpu_gfx90a::s0);
  BASEREG_CHECK(amdgpu_gfx90a::s101, amdgpu_gfx90a::s101);

  BASEREG_CHECK(amdgpu_gfx90a::v0, amdgpu_gfx90a::v0);
  BASEREG_CHECK(amdgpu_gfx90a::v101, amdgpu_gfx90a::v101);

  BASEREG_CHECK(amdgpu_gfx90a::acc0, amdgpu_gfx90a::acc0);
  BASEREG_CHECK(amdgpu_gfx90a::acc255, amdgpu_gfx90a::acc255);

  BASEREG_CHECK(amdgpu_gfx90a::vcc, amdgpu_gfx90a::vcc);
  BASEREG_CHECK(amdgpu_gfx90a::vcc_lo, amdgpu_gfx90a::vcc);
  BASEREG_CHECK(amdgpu_gfx90a::vcc_hi, amdgpu_gfx90a::vcc);

  BASEREG_CHECK(amdgpu_gfx90a::exec, amdgpu_gfx90a::exec);
  BASEREG_CHECK(amdgpu_gfx90a::exec_lo, amdgpu_gfx90a::exec);
  BASEREG_CHECK(amdgpu_gfx90a::exec_hi, amdgpu_gfx90a::exec);

  BASEREG_CHECK(amdgpu_gfx90a::flat_scratch_all, amdgpu_gfx90a::flat_scratch_all);
  BASEREG_CHECK(amdgpu_gfx90a::flat_scratch_lo, amdgpu_gfx90a::flat_scratch_all);
  BASEREG_CHECK(amdgpu_gfx90a::flat_scratch_hi, amdgpu_gfx90a::flat_scratch_all);

  return EXIT_SUCCESS;
}
