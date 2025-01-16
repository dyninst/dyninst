#include "basereg_check.h"
#include "registers/AMDGPU/amdgpu_gfx90a_regs.h"

int main() {
  BASEREG_CHECK(amdgpu_gfx90a::s0, amdgpu_gfx90a::s0);
  BASEREG_CHECK(amdgpu_gfx90a::s101, amdgpu_gfx90a::s0);

  BASEREG_CHECK(amdgpu_gfx90a::v0, amdgpu_gfx90a::v0);
  BASEREG_CHECK(amdgpu_gfx90a::v101, amdgpu_gfx90a::v0);

  return EXIT_SUCCESS;
}
