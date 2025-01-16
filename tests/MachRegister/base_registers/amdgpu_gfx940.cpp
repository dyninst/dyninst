#include "basereg_check.h"
#include "registers/AMDGPU/amdgpu_gfx940_regs.h"

int main() {
  BASEREG_CHECK(amdgpu_gfx940::s0, amdgpu_gfx940::s0);
  BASEREG_CHECK(amdgpu_gfx940::s101, amdgpu_gfx940::s0);

  BASEREG_CHECK(amdgpu_gfx940::v0, amdgpu_gfx940::v0);
  BASEREG_CHECK(amdgpu_gfx940::v101, amdgpu_gfx940::v0);

  return EXIT_SUCCESS;
}
