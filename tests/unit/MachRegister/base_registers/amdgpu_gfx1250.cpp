#include "basereg_check.h"
#include "registers/AMDGPU/amdgpu_gfx1250_regs.h"

int main() {
  BASEREG_CHECK(amdgpu_gfx1250::ttmp0, amdgpu_gfx1250::ttmp0);
  BASEREG_CHECK(amdgpu_gfx1250::ttmp15, amdgpu_gfx1250::ttmp15);

  BASEREG_CHECK(amdgpu_gfx1250::s0, amdgpu_gfx1250::s0);
  BASEREG_CHECK(amdgpu_gfx1250::s105, amdgpu_gfx1250::s105);

  BASEREG_CHECK(amdgpu_gfx1250::v0, amdgpu_gfx1250::v0);
  BASEREG_CHECK(amdgpu_gfx1250::v101, amdgpu_gfx1250::v101);

  BASEREG_CHECK(amdgpu_gfx1250::vcc_lo, amdgpu_gfx1250::vcc_lo);
  BASEREG_CHECK(amdgpu_gfx1250::vcc_hi, amdgpu_gfx1250::vcc_hi);
  BASEREG_CHECK(amdgpu_gfx1250::exec_lo, amdgpu_gfx1250::exec_lo);
  BASEREG_CHECK(amdgpu_gfx1250::exec_hi, amdgpu_gfx1250::exec_hi);

  return EXIT_SUCCESS;
}
