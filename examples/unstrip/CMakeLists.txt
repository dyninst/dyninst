add_executable(unstrip unstrip.C
                       util.C
                       types.C
                       semanticDescriptor.C
                       database.C
                       fingerprint.C
                       callback.C)
add_dependencies(unstrip parseAPI symtabAPI instructionAPI common)
target_link_libraries(unstrip parseAPI symtabAPI instructionAPI common)
file(COPY ddb.db DESTINATION ${INSTALL_BIN_DIR})
file(COPY params.db DESTINATION ${INSTALL_BIN_DIR})
file(COPY unistd.db DESTINATION ${INSTALL_BIN_DIR})

install (TARGETS unstrip
        RUNTIME DESTINATION ${INSTALL_BIN_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
