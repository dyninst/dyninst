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
