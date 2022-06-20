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

target_link_libraries(
    parseThat
    PRIVATE Dyninst::dyninstAPI
            Dyninst::patchAPI
            Dyninst::parseAPI
            Dyninst::instructionAPI
            Dyninst::stackwalk
            Dyninst::symtabAPI
            Dyninst::common
            Dyninst::pcontrol
            Dyninst::dynDwarf
            Dyninst::dynElf
            Dyninst::Boost
            Dyninst::ElfUtils)

if(UNIX AND NOT APPLE)
    set_target_properties(
        parseThat
        PROPERTIES
            INSTALL_RPATH_USE_LINK_PATH ON
            INSTALL_RPATH
            "\$ORIGIN/../${INSTALL_LIB_DIR}:\$ORIGIN/../${INSTALL_LIB_DIR}/${TPL_INSTALL_LIB_DIR}"
        )
endif()

if(USE_OpenMP)
    if(NOT OpenMP_FOUND)
        find_package(OpenMP REQUIRED)
    endif()
    target_link_libraries(parseThat PRIVATE OpenMP::OpenMP_CXX)
endif()

install(TARGETS parseThat RUNTIME DESTINATION bin)
