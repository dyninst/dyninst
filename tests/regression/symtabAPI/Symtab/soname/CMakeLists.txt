include_guard(GLOBAL)

# The checker
dyninst_add_test_executable(getSOName SOURCE_FILES getSOName.cpp DEPS symtabAPI)

# 1. The ordinary case: CMake gives a shared library a SONAME equal to its file name.
add_library(soname_default SHARED test.cpp)

# 2. No SONAME at all.
add_library(soname_absent SHARED test.cpp)
set_target_properties(soname_absent PROPERTIES NO_SONAME TRUE)

# 3. The case ProcControl cares about: the SONAME and the file name disagree. This is the
#    shape glibc-hwcaps produces, where libpthread.so.0 resolves to a file called
#    libpthread-<version>.so under a microarchitecture directory.
add_library(soname_mismatch SHARED test.cpp)
set_target_properties(
  soname_mismatch
  PROPERTIES NO_SONAME TRUE OUTPUT_NAME "soname_on_disk_name"
)
target_link_options(soname_mismatch PRIVATE "-Wl,-soname,libdyninst_declared_name.so.7")

dyninst_add_test(
  symtab_soname_default
  TYPE regression
  COMMAND getSOName $<TARGET_FILE:soname_default> $<TARGET_FILE_NAME:soname_default>
)
dyninst_add_test(
  symtab_soname_absent
  TYPE regression
  COMMAND getSOName $<TARGET_FILE:soname_absent> ""
)
dyninst_add_test(
  symtab_soname_mismatch
  TYPE regression
  COMMAND getSOName $<TARGET_FILE:soname_mismatch> "libdyninst_declared_name.so.7"
)

# The mismatch case must be a real mismatch, or it proves nothing.
dyninst_add_test(
  symtab_soname_mismatch_is_a_mismatch
  TYPE regression
  COMMAND getSOName $<TARGET_FILE:soname_mismatch> $<TARGET_FILE_NAME:soname_mismatch>
  PROPERTIES WILL_FAIL TRUE
)
