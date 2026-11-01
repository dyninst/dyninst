#include "basereg_check.h"
#include "registers/amdgpu/amdgpu_gfx942_regs.h"

int main() {
  BASEREG_CHECK(amdgpu_gfx942::ttmp0, amdgpu_gfx942::ttmp0);
  BASEREG_CHECK(amdgpu_gfx942::ttmp15, amdgpu_gfx942::ttmp15);

  BASEREG_CHECK(amdgpu_gfx942::attr0, amdgpu_gfx942::attr0);
  BASEREG_CHECK(amdgpu_gfx942::attr32, amdgpu_gfx942::attr32);

  BASEREG_CHECK(amdgpu_gfx942::s0, amdgpu_gfx942::s0);
  BASEREG_CHECK(amdgpu_gfx942::s101, amdgpu_gfx942::s101);

  BASEREG_CHECK(amdgpu_gfx942::v0, amdgpu_gfx942::v0);
  BASEREG_CHECK(amdgpu_gfx942::v101, amdgpu_gfx942::v101);

  BASEREG_CHECK(amdgpu_gfx942::acc0, amdgpu_gfx942::acc0);
  BASEREG_CHECK(amdgpu_gfx942::acc255, amdgpu_gfx942::acc255);
  BASEREG_CHECK(amdgpu_gfx942::vcc_lo, amdgpu_gfx942::vcc_lo);
  BASEREG_CHECK(amdgpu_gfx942::vcc_hi, amdgpu_gfx942::vcc_hi);
  BASEREG_CHECK(amdgpu_gfx942::exec_lo, amdgpu_gfx942::exec_lo);
  BASEREG_CHECK(amdgpu_gfx942::exec_hi, amdgpu_gfx942::exec_hi);
  BASEREG_CHECK(amdgpu_gfx942::flat_scratch_lo, amdgpu_gfx942::flat_scratch_lo);
  BASEREG_CHECK(amdgpu_gfx942::flat_scratch_hi, amdgpu_gfx942::flat_scratch_hi);

  return EXIT_SUCCESS;
}
