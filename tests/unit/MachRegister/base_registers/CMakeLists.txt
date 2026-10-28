include_guard(GLOBAL)

foreach(
  _arch
  x86
  x86_64
  aarch64
  ppc32
  ppc64
  amdgpu_gfx908
  amdgpu_gfx90a
  amdgpu_gfx940
  amdgpu_gfx950
  riscv64
)
  dyninst_add_test(
    machregister_basereg_${_arch}
    TYPE unit
    SOURCE_FILES ${_arch}.cpp
    DEPS common
  )
endforeach()
