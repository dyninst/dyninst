# CMake configuration for symlite directory

if(NOT UNIX)
    return()
endif()

set(SRC_LIST src/SymLite-elf.C)

dyninst_library(
    symLite
    SOURCES ${SRC_LIST}
    COMPILE_DEFINITIONS PRIVATE SYMLITE_LIB
    LINK_LIBRARIES Dyninst::headers Dyninst::common Dyninst::dynElf)
