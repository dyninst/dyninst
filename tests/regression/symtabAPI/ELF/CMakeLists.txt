include_guard(GLOBAL)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  return()
endif()

add_executable(lsda_encoding_check lsda_encoding.cpp)
target_link_libraries(lsda_encoding_check PRIVATE symtabAPI)

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
  add_test(NAME symtabAPI_lsda_${kind}
           COMMAND lsda_encoding_check $<TARGET_FILE:lsda_${kind}> ${kind})
  # Bound the original infinite-loop regression.
  set_tests_properties(symtabAPI_lsda_${kind} PROPERTIES LABELS regression TIMEOUT 5)
endforeach()
