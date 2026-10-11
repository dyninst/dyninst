include_guard(GLOBAL)

# parseThat requires dyninstAPI which doesn't work with symlite
if(LIGHTWEIGHT_SYMTAB)
  message(STATUS "LIGHTWEIGHT_SYMTAB enabled; parseThat not built.")
  return()
endif()

add_executable(
  parseThat
  src/parseThat.C
  src/config.C
  src/ipc.C
  src/record.C
  src/strlist.C
  src/reglist.C
  src/log.C
  src/utils.C
  src/sha1.C
  src/dyninstCore.C
  src/dyninstCompat.v5.C)

target_compile_definitions(parseThat PRIVATE HAVE_BPATCH_PROCESS_H)

target_link_libraries(parseThat PRIVATE dyninstAPI)

install(TARGETS parseThat RUNTIME DESTINATION "${DYNINST_INSTALL_BINDIR}")
