#include "basereg_check.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"

int main() {
  BASEREG_CHECK(amdgpu_gfx908::s0, amdgpu_gfx908::s0);
  BASEREG_CHECK(amdgpu_gfx908::s101, amdgpu_gfx908::s0);

  BASEREG_CHECK(amdgpu_gfx908::v0, amdgpu_gfx908::v0);
  BASEREG_CHECK(amdgpu_gfx908::v101, amdgpu_gfx908::v0);

  return EXIT_SUCCESS;
}
