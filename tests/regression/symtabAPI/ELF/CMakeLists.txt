include_guard(GLOBAL)

dyninst_add_test_executable(
  lsda_encoding_check
  SOURCE_FILES lsda_encoding.cpp
  DEPS symtabAPI
)

foreach(kind IN ITEMS pcrel omit invalid oversized)
  add_library(lsda_${kind} SHARED lsda_fixture.S)
  set_target_properties(lsda_${kind} PROPERTIES LINKER_LANGUAGE C)
  target_link_options(lsda_${kind} PRIVATE -nostdlib)
  if(NOT kind STREQUAL "pcrel")
    target_compile_definitions(lsda_${kind} PRIVATE OMIT_LPSTART)
  endif()
  if(kind STREQUAL "invalid")
    target_compile_definitions(lsda_${kind} PRIVATE INVALID_FORMAT)
  endif()
  if(kind STREQUAL "oversized")
    target_compile_definitions(lsda_${kind} PRIVATE OVERSIZED_TABLE)
  endif()

  # Bound the original infinite-loop regression.
  dyninst_add_test(
    symtabAPI_lsda_${kind}
    TYPE regression
    COMMAND lsda_encoding_check $<TARGET_FILE:lsda_${kind}> ${kind}
    PROPERTIES TIMEOUT 5
  )
endforeach()

# An aarch64 library with a TLSDESC relocation in .rela.plt. It is read and
# rewritten, never run, so the test runs on any host.
dyninst_add_test_executable(plt_tlsdesc SOURCE_FILES plt_tlsdesc.cpp DEPS symtabAPI)
dyninst_add_test(
  symtabAPI_plt_tlsdesc_aarch64
  TYPE regression
  COMMAND
    plt_tlsdesc "${DYNINST_TEST_BINARIES_DIR}/symtabAPI/plt_stubs/aarch64/libtlsdesc.so"
    ext_func 0x2a0 ext_tls "${CMAKE_CURRENT_BINARY_DIR}/libtlsdesc.rewritten.so"
  REQUIRES test_binaries
)
