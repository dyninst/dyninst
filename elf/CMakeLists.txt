# CMake configuration for dynElf (elf) directory

if(NOT UNIX)
    return()
endif()

set(SRC_LIST
    src/Elf_X.C)

if(ENABLE_DEBUGINFOD AND LibDebuginfod_FOUND)
    set(_PRIVATE_COMPILE_DEFS DEBUGINFOD_LIB)
endif()

dyninst_library(dynElf
    SOURCES
        ${SRC_LIST}
    COMPILE_DEFINITIONS
        PRIVATE DYNELF_LIB ${_PRIVATE_COMPILE_DEFS}
    INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/h>
        $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
    DEPENDS
        TBB-External
        ElfUtils-External
    LINK_LIBRARIES
        Dyninst::Boost
        Dyninst::TBB
        PRIVATE
            Dyninst::ElfUtils
            ${_PRIVATE_LINK_LIBS})
