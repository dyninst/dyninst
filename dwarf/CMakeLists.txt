# CMake configuration for dynDwarf (dwarf) directory

if(NOT UNIX)
    add_library(Dyninst::dynDwarf ALIAS dynDummy)
    return()
endif()

set(SRC_LIST src/dwarfResult.C src/dwarfExprParser.C src/dwarfFrameParser.C
             src/dwarfHandle.C)

dyninst_library(
    dynDwarf
    SOURCES ${SRC_LIST}
    COMPILE_DEFINITIONS PRIVATE DYNDWARF_LIB
    INCLUDE_DIRECTORIES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/h>
                        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/common/src>
    DEPENDS ElfUtils-External
    LINK_LIBRARIES Dyninst::headers Dyninst::common Dyninst::dynElf)
