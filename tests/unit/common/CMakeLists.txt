include_guard(GLOBAL)

dyninst_add_test(
  common_filesystem
  TYPE unit
  SOURCE_FILES filesystem.cpp
  DEPS common
  PROPERTIES SKIP_REGULAR_EXPRESSION "not defined in environment"
)

dyninst_add_test(common_bitmath_test TYPE unit SOURCE_FILES bitmath.cpp DEPS common)

dyninst_add_test(
  common_arch_aarch64_test
  TYPE unit
  SOURCE_FILES arch_aarch64.cpp
  DEPS common
)

add_subdirectory(registers)
