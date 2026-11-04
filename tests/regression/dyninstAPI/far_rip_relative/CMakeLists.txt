include_guard(GLOBAL)

# The mutatee must be a position-independent x86_64 executable whose
# addresses start above 4 GiB, out of reach of a 32-bit absolute address.
# Its functions that the test relies on are written in assembly.
dyninst_add_test_executable(
  far_rip_relative_mutatee
  SOURCE_FILES far_rip_relative_mutatee.c
  COMPILE_OPTIONS -fPIE
  REQUIRES host:x86_64 native
)
if(TARGET far_rip_relative_mutatee)
  target_link_options(
    far_rip_relative_mutatee
    PRIVATE -pie -Wl,-Ttext-segment=0x100000000
  )
endif()

# The test rewrites the mutatee and runs the rewritten copy.
dyninst_add_test(
  dyninstAPI_far_rip_relative
  TYPE regression
  SOURCE_FILES far_rip_relative.cpp
  DEPS dyninstAPI symtabAPI
  ARGS $<TARGET_FILE:far_rip_relative_mutatee> ${CMAKE_CURRENT_BINARY_DIR}/rewritten
  REQUIRES host:x86_64 native
  PROPERTIES TIMEOUT 120 ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
)
if(TARGET dyninstAPI_far_rip_relative)
  add_dependencies(dyninstAPI_far_rip_relative dyninstAPI_RT)
endif()
