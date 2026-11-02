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
  REASON "needs the static library <arg>_static (ENABLE_STATIC_LIBS=OFF)"
)
# The IA-32 emitter is built for both 32- and 64-bit x86; this takes precedence over
# the family below.
dyninst_test_requirement(
  codegen:i386
  WHEN DYNINST_CODEGEN_ARCH_I386 OR DYNINST_CODEGEN_ARCH_X86_64
  REASON "needs code generation for i386 or x86_64"
)
dyninst_test_requirement(
  codegen:<arg>
  WHEN DYNINST_CODEGEN_ARCH_<ARG>
  VALUES ${DYNINST_CODEGEN_ARCHES}
  REASON "needs code generation for <arg>"
)
dyninst_test_requirement(
  host:<arg>
  WHEN DYNINST_HOST_ARCH_<ARG>
  VALUES ${DYNINST_HOST_ARCHES}
  REASON "needs the host to be <arg>"
)
# A program Dyninst instruments or rewrites runs on the host only when Dyninst
# generates code for the host's architecture.
dyninst_test_requirement(
  native
  WHEN DYNINST_CODEGEN_NATIVE
  REASON
    "needs code generation for the host (DYNINST_CODEGEN_ARCH=${DYNINST_CODEGEN_ARCH})"
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
