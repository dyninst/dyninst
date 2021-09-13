# CMake configuration for symlite directory

if (NOT UNIX)
    return()
endif()

set (SRC_LIST
    src/SymLite-elf.C
)

dyninst_library(symLite
    SOURCES
        ${SRC_LIST}
    COMPILE_DEFINITIONS
        PRIVATE SYMLITE_LIB
    INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
    LINK_LIBRARIES
        Dyninst::common
        Dyninst::dynElf
        PRIVATE
            Dyninst::ElfUtils)
