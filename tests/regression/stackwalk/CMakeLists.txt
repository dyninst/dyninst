include_guard(GLOBAL)

# The check attaches to the target with ptrace.
dyninst_add_test_executable(
  undefined_ra_target
  SOURCE_FILES undefined_ra_target.c
  DEPS Threads::Threads
  REQUIRES dynamic
)

# Skips unless the host is AArch64 with glibc >= 2.34.
dyninst_add_test(
  stackwalk_undefined_ra
  TYPE regression
  SOURCE_FILES undefined_ra_check.cpp
  DEPS stackwalk
  ARGS $<TARGET_FILE:undefined_ra_target>
  REQUIRES dynamic
  PROPERTIES TIMEOUT 60
)

add_subdirectory(vdso)
