add_executable(parseThat src/parseThat.C src/config.C src/ipc.C src/record.C src/strlist.C src/reglist.C src/log.C src/utils.C src/sha1.C src/dyninstCore.C src/dyninstCompat.v5.C)
add_definitions(-DHAVE_BPATCH_PROCESS_H)

if (USE_OpenMP MATCHES "ON")
set_target_properties (parseThat PROPERTIES LINK_FLAGS "-fopenmp")
endif()

target_link_private_libraries(parseThat dyninstAPI patchAPI parseAPI instructionAPI stackwalk symtabAPI common pcontrol dynDwarf dynElf ${Boost_LIBRARIES})
install(TARGETS parseThat RUNTIME DESTINATION bin)
