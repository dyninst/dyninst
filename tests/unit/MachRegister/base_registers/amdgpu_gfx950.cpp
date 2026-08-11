#include "basereg_check.h"
#include "registers/AMDGPU/amdgpu_gfx950_regs.h"

int main() {
  BASEREG_CHECK(amdgpu_gfx950::ttmp0, amdgpu_gfx950::ttmp0);
  BASEREG_CHECK(amdgpu_gfx950::ttmp15, amdgpu_gfx950::ttmp15);

  BASEREG_CHECK(amdgpu_gfx950::attr0, amdgpu_gfx950::attr0);
  BASEREG_CHECK(amdgpu_gfx950::attr32, amdgpu_gfx950::attr32);

  BASEREG_CHECK(amdgpu_gfx950::s0, amdgpu_gfx950::s0);
  BASEREG_CHECK(amdgpu_gfx950::s101, amdgpu_gfx950::s101);

  BASEREG_CHECK(amdgpu_gfx950::v0, amdgpu_gfx950::v0);
  BASEREG_CHECK(amdgpu_gfx950::v101, amdgpu_gfx950::v101);

  BASEREG_CHECK(amdgpu_gfx950::acc0, amdgpu_gfx950::acc0);
  BASEREG_CHECK(amdgpu_gfx950::acc255, amdgpu_gfx950::acc255);
  BASEREG_CHECK(amdgpu_gfx950::vcc_lo, amdgpu_gfx950::vcc_lo);
  BASEREG_CHECK(amdgpu_gfx950::vcc_hi, amdgpu_gfx950::vcc_hi);
  BASEREG_CHECK(amdgpu_gfx950::exec_lo, amdgpu_gfx950::exec_lo);
  BASEREG_CHECK(amdgpu_gfx950::exec_hi, amdgpu_gfx950::exec_hi);
  BASEREG_CHECK(amdgpu_gfx950::flat_scratch_lo, amdgpu_gfx950::flat_scratch_lo);
  BASEREG_CHECK(amdgpu_gfx950::flat_scratch_hi, amdgpu_gfx950::flat_scratch_hi);

  return EXIT_SUCCESS;
}
