# CMake configuration for symtabAPI directory

include_directories(src h ${PROJECT_SOURCE_DIR}/dwarf/h ${PROJECT_SOURCE_DIR}/elf/h)

if(NOT DYNINST_OS_Windows)
    include_directories(${PROJECT_SOURCE_DIR}/common/src)
endif()

set(SRC_LIST
    src/Object.C
    src/Aggregate.C
    src/Function.C
    src/Variable.C
    src/Symbol.C
    src/LineInformation.C
    src/Symtab.C
    src/Symtab-edit.C
    src/Symtab-lookup.C
    src/Module.C
    src/Region.C
    src/Collections.C
    src/Type.C
    src/AddrLookup.C
    src/annotations.C
    src/debug.C
    src/SymtabReader.C)

if(DYNINST_OS_UNIX)

    set(SRC_LIST
        ${SRC_LIST}
        src/Object-elf.C
        src/Archive.C
        src/Archive-elf.C
        src/parseDwarf.C
        src/LinkMap.C
        src/emitElf.C
        src/emitElfStatic.C
        src/dwarfWalker.C)

    if(DYNINST_ARCH_x86_64)
        set(SRC_LIST ${SRC_LIST} src/emitElfStatic-x86.C src/relocationEntry-elf-x86.C)
    elseif(DYNINST_ARCH_i386)
        set(SRC_LIST ${SRC_LIST} src/emitElfStatic-x86.C src/relocationEntry-elf-x86.C)
    elseif(DYNINST_ARCH_ppc64le)
        set(SRC_LIST ${SRC_LIST} src/emitElfStatic-ppc64.C
                     src/relocationEntry-elf-ppc64.C)
    elseif(DYNINST_ARCH_aarch64)
        set(SRC_LIST ${SRC_LIST} src/emitElfStatic-aarch64.C
                     src/relocationEntry-elf-aarch64.C)
    else(TRUE)
        set(SRC_LIST ${SRC_LIST} src/emitElfStatic-stub.C src/relocationEntry-stub.C)
    endif()
endif()

if(DYNINST_OS_Windows)
    set(SRC_LIST ${SRC_LIST} src/Object-nt.C src/emitWin.C src/relocationEntry-stub.C)
endif()

set_source_files_properties(${SRC_LIST} PROPERTIES LANGUAGE CXX)

add_definitions(-DSYMTAB_LIB)

set(DEPS common)

if(DYNINST_OS_UNIX)
    list(APPEND DEPS dynElf dynDwarf)
else()
    list(APPEND DEPS dbghelp)
endif()

dyninst_library(symtabAPI ${DEPS})
target_link_libraries(symtabAPI PRIVATE Dyninst::TBB OpenMP::OpenMP_CXX)
target_link_libraries(symtabAPI PUBLIC Dyninst::ElfUtils)
target_link_libraries(symtabAPI PUBLIC Dyninst::Boost)
