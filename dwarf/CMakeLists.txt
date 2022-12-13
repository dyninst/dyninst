# CMake configuration for dynDwarf (dwarf) directory

if(NOT UNIX)
    return()
endif()

include_directories(src h ${PROJECT_SOURCE_DIR}/elf/h ${PROJECT_SOURCE_DIR}/common/src)
add_definitions(-DDYNDWARF_LIB)

set(SRC_LIST src/dwarfResult.C src/dwarfExprParser.C src/dwarfFrameParser.C
             src/dwarfHandle.C)

dyninst_library(dynDwarf dynElf common)
target_link_libraries(dynDwarf PUBLIC Dyninst::Boost_headers)
target_link_libraries(dynDwarf PRIVATE Dyninst::ElfUtils)