# The requirements tests name in dyninst_add_test(... REQUIRES ...). See
# DyninstAddTest.cmake. Each condition is evaluated where the test is added.

include_guard(GLOBAL)

include(DyninstAddTest)

# Every Dyninst library, the arguments static_lib:<lib> accepts
get_property(_dyninst_libraries GLOBAL PROPERTY DYNINST_LIBRARIES)

dyninst_test_requirement(
  test_binaries
  WHEN DYNINST_ENABLE_EXTERNAL_BINARY_TESTS
  REASON "needs the test_binaries repository (DYNINST_ENABLE_TESTS=ALL)"
)
dyninst_test_requirement(
  capstone
  WHEN DYNINST_ENABLE_CAPSTONE
  REASON "needs Capstone (DYNINST_ENABLE_CAPSTONE=OFF)"
)
dyninst_test_requirement(objdump WHEN CMAKE_OBJDUMP REASON "objdump not found")
dyninst_test_requirement(
  static_exe
  WHEN DYNINST_CXX_STATIC_EXE_SUPPORTED
  REASON "the toolchain cannot link a static executable"
)
dyninst_test_requirement(
  static_lib:<arg>
  WHEN TARGET <arg>_static
  VALUES ${_dyninst_libraries}
  REASON "needs the static <arg> (ENABLE_STATIC_LIBS=OFF)"
)
dyninst_test_requirement(
  codegen:<arg>
  WHEN DYNINST_CODEGEN_ARCH_<ARG>
  # The DYNINST_CODEGEN_ARCH_* that DyninstPlatform.cmake sets
  VALUES
    x86_64
    i386
    aarch64
    ppc64le
    riscv64
    amdgpu_gfx908
    amdgpu_gfx90a
    amdgpu_gfx940
    amdgpu_gfx950
  REASON "needs code generation for <arg>"
)
dyninst_test_requirement(
  dynamic
  WHEN DYNINST_ENABLE_DYNAMIC_TESTS
  REASON "needs a live process (DYNINST_ENABLE_DYNAMIC_TESTS=OFF)"
)
dyninst_test_requirement(
  userns
  WHEN DYNINST_ENABLE_USERNS_TESTS
  REASON "needs unprivileged user namespaces (DYNINST_ENABLE_USERNS_TESTS=OFF)"
)
