add_executable(parseThat src/parseThat.C src/config.C src/ipc.C src/record.C src/strlist.C src/reglist.C src/log.C src/utils.C src/sha1.C src/dyninstCore.C src/dyninstCompat.v5.C)
add_definitions(-DHAVE_BPATCH_PROCESS_H)
target_link_private_libraries(parseThat dyninstAPI)
install(TARGETS parseThat RUNTIME DESTINATION bin)
