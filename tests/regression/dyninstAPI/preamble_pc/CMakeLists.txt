include_guard(GLOBAL)

# The skipped TOC preamble exists only on ppc64 (ELFv2 global entry points),
# and the test instruments a live mutatee.
dyninst_add_test_executable(
  preamble_pc_mutatee
  SOURCE_FILES preamble_pc_mutatee.c
  REQUIRES host:ppc64le native dynamic
)

dyninst_add_test(
  dyninstAPI_preamble_pc_test
  TYPE regression
  SOURCE_FILES preamble_pc.cpp
  DEPS dyninstAPI
  ARGS $<TARGET_FILE:preamble_pc_mutatee> $<TARGET_FILE_DIR:dyninstAPI_RT>
  REQUIRES host:ppc64le native dynamic
  PROPERTIES TIMEOUT 120 ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
)
