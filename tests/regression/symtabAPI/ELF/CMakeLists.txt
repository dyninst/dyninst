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

# A library with a TLSDESC relocation among its PLT relocations, for each
# architecture whose TLSDESC relocations go there, with the address of its one
# stub. It is read and rewritten, never run, so the test runs on any host.
dyninst_add_test_executable(plt_tlsdesc SOURCE_FILES plt_tlsdesc.cpp DEPS symtabAPI)
foreach(_case IN ITEMS aarch64:0x2a0 x86_64:0x1010 i686:0x1010)
  string(REPLACE ":" ";" _case "${_case}")
  list(GET _case 0 _arch)
  list(GET _case 1 _stub)
  dyninst_add_test(
    symtabAPI_plt_tlsdesc_${_arch}
    TYPE regression
    COMMAND
      plt_tlsdesc
      "${DYNINST_TEST_BINARIES_DIR}/symtabAPI/plt_stubs/${_arch}/libtlsdesc.so" ext_func
      ${_stub} ext_tls "${CMAKE_CURRENT_BINARY_DIR}/libtlsdesc.${_arch}.rewritten.so"
    REQUIRES test_binaries
  )
endforeach()
