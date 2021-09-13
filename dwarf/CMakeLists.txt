# CMake configuration for dynDwarf (dwarf) directory

if (NOT UNIX)
    add_library(Dyninst::dynDwarf ALIAS dynDummy)
    return()
endif()

set (SRC_LIST
    src/dwarfResult.C
    src/dwarfExprParser.C
    src/dwarfFrameParser.C
    src/dwarfHandle.C
)

dyninst_library(dynDwarf
    SOURCES
        ${SRC_LIST}
    COMPILE_DEFINITIONS
        PRIVATE DYNDWARF_LIB
    INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/h>
        $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
    DEPENDS
        ElfUtils-External
    LINK_LIBRARIES
        Dyninst::common
        Dyninst::dynElf)
