

add_executable(unstrip unstrip/unstrip.C
                       unstrip/util.C
                       unstrip/types.C
                       unstrip/semanticDescriptor.C
                       unstrip/database.C
                       unstrip/fingerprint.C
                       unstrip/callback.C)
add_dependencies(unstrip parseAPI symtabAPI instructionAPI common)
target_link_libraries(unstrip parseAPI symtabAPI instructionAPI common)
file(COPY unstrip/ddb.db DESTINATION ${INSTALL_BIN_DIR})
file(COPY unstrip/params.db DESTINATION ${INSTALL_BIN_DIR})
file(COPY unstrip/unistd.db DESTINATION ${INSTALL_BIN_DIR})

add_executable(codeCoverage codeCoverage/codeCoverage.C)
add_dependencies(codeCoverage dyninstAPI patchAPI parseAPI symtabAPI instructionAPI pcontrol common)
target_link_libraries(codeCoverage dyninstAPI patchAPI parseAPI symtabAPI instructionAPI pcontrol common)

add_library(Inst SHARED codeCoverage/libInst.C)

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
   

