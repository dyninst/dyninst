

add_executable(unstrip unstrip.dir/unstrip.C
                       unstrip.dir/util.C
                       unstrip.dir/types.C
                       unstrip.dir/semanticDescriptor.C
                       unstrip.dir/database.C
                       unstrip.dir/fingerprint.C
                       unstrip.dir/callback.C)
add_dependencies(unstrip parseAPI symtabAPI instructionAPI common)
target_link_libraries(unstrip parseAPI symtabAPI instructionAPI common dynDwarf dynElf ${Boost_LIBRARIES} ${ElfUtils_LIBRARIES})

add_executable(codeCoverage codeCoverage.dir/codeCoverage.C)
add_dependencies(codeCoverage dyninstAPI patchAPI parseAPI symtabAPI instructionAPI pcontrol common stackwalk dynDwarf dynElf)
target_link_libraries(codeCoverage dyninstAPI patchAPI parseAPI symtabAPI instructionAPI pcontrol common stackwalk dynDwarf dynElf ${Boost_LIBRARIES} ${ElfUtils_LIBRARIES})

add_library(Inst SHARED codeCoverage.dir/libInst.C)
if(TARGET TBB)
    add_dependencies(Inst TBB)
endif()

add_executable(cfg_to_dot ../parseAPI/doc/example.cc)
add_dependencies(cfg_to_dot parseAPI symtabAPI instructionAPI common dynDwarf dynElf)
target_link_libraries(cfg_to_dot parseAPI symtabAPI instructionAPI common dynDwarf dynElf ${Boost_LIBRARIES} ${ElfUtils_LIBRARIES})
#add_executable(retee)

if (USE_OpenMP)
set_target_properties (unstrip PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS} LINK_FLAGS ${OpenMP_CXX_FLAGS})
set_target_properties (codeCoverage PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS} LINK_FLAGS ${OpenMP_CXX_FLAGS})
set_target_properties (cfg_to_dot PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS} LINK_FLAGS ${OpenMP_CXX_FLAGS})
endif()


install (TARGETS cfg_to_dot unstrip codeCoverage Inst
        RUNTIME DESTINATION ${INSTALL_BIN_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR}
)

install (
        FILES unstrip.dir/ddb.db unstrip.dir/params.db unstrip.dir/unistd.db DESTINATION ${INSTALL_BIN_DIR}
)
