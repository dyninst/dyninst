project(tests_dataflowAPI LANGUAGES CXX)

add_subdirectory(rose)

dyninst_add_test(
  dataflowAPI_stack_analysis_concurrent_cache
  TYPE unit
  SOURCE_FILES stack_analysis/concurrent_cache.cpp
  DEPS parseAPI Threads::Threads
  # Not i386: parsing the host's x86_64 libc crashes when Dyninst
  # generates code only for i386 (#2397).
  REQUIRES
    codegen:x86_64,aarch64,ppc64le,riscv64,amdgpu_gfx908,amdgpu_gfx90a,amdgpu_gfx940,amdgpu_gfx950
)
