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

# The same mutatee as a non-PIE executable, at the linker's default address.
dyninst_add_test_executable(
  far_rip_relative_mutatee_nopie
  SOURCE_FILES far_rip_relative_mutatee.c
  COMPILE_OPTIONS -fno-pie
  REQUIRES host:x86_64 native
)
if(TARGET far_rip_relative_mutatee_nopie)
  target_link_options(far_rip_relative_mutatee_nopie PRIVATE -no-pie)
endif()

# The same mutatee as a shared library linked at 0, loaded by
# far_rip_relative_host. The host has no build RPATH, so that the test can make
# it load the rewritten library through LD_LIBRARY_PATH.
dyninst_add_test_executable(
  far_rip_relative_host
  SOURCE_FILES far_rip_relative_host.c
  REQUIRES host:x86_64 native
)
if(TARGET far_rip_relative_host)
  add_library(far_rip_relative_shlib SHARED far_rip_relative_mutatee.c)
  target_compile_definitions(far_rip_relative_shlib PRIVATE FAR_RIP_RELATIVE_SHLIB)
  set_target_properties(far_rip_relative_shlib PROPERTIES C_VISIBILITY_PRESET hidden)
  target_link_libraries(far_rip_relative_host PRIVATE far_rip_relative_shlib)
  set_target_properties(far_rip_relative_host PROPERTIES SKIP_BUILD_RPATH TRUE)
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

dyninst_add_test(
  dyninstAPI_far_rip_relative_nopie
  TYPE regression
  COMMAND
    $<TARGET_FILE:dyninstAPI_far_rip_relative>
    $<TARGET_FILE:far_rip_relative_mutatee_nopie> ${CMAKE_CURRENT_BINARY_DIR}/rewritten_nopie
  REQUIRES host:x86_64 native
  PROPERTIES TIMEOUT 120 ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
)

dyninst_add_test(
  dyninstAPI_far_rip_relative_shlib
  TYPE regression
  COMMAND
    $<TARGET_FILE:dyninstAPI_far_rip_relative>
    $<TARGET_FILE:far_rip_relative_shlib> ${CMAKE_CURRENT_BINARY_DIR}/rewritten_shlib
    $<TARGET_FILE:far_rip_relative_host>
  REQUIRES host:x86_64 native
  PROPERTIES TIMEOUT 120 ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
)
