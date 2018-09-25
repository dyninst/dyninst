

add_executable(unstrip unstrip_src/unstrip.C
                       unstrip_src/util.C
                       unstrip_src/types.C
                       unstrip_src/semanticDescriptor.C
                       unstrip_src/database.C
                       unstrip_src/fingerprint.C
                       unstrip_src/callback.C)
add_dependencies(unstrip parseAPI symtabAPI instructionAPI common)
target_link_libraries(unstrip parseAPI symtabAPI instructionAPI common)
file(COPY unstrip_src/ddb.db DESTINATION ${INSTALL_BIN_DIR})
file(COPY unstrip_src/params.db DESTINATION ${INSTALL_BIN_DIR})
file(COPY unstrip_src/unistd.db DESTINATION ${INSTALL_BIN_DIR})

add_executable(codeCoverage codeCoverage_src/codeCoverage.C)
add_dependencies(codeCoverage dyninstAPI patchAPI parseAPI symtabAPI instructionAPI pcontrol common)
target_link_libraries(codeCoverage dyninstAPI patchAPI parseAPI symtabAPI instructionAPI pcontrol common)

add_library(Inst SHARED codeCoverage_src/libInst.C)

add_executable(cfg_to_dot ../parseAPI/doc/example.cc)
add_dependencies(cfg_to_dot parseAPI symtabAPI)
target_link_libraries(cfg_to_dot parseAPI symtabAPI
        )
#add_executable(retee)

install (TARGETS cfg_to_dot unstrip codeCoverage Inst
        RUNTIME DESTINATION ${INSTALL_BIN_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
   

