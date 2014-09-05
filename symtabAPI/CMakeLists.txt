# CMake configuration for symtabAPI directory

include_directories (
    ${PROJECT_SOURCE_DIR}/symtabAPI/src
  )


set (SRC_LIST
                src/Object.C 
                src/Aggregate.C 
                src/Function.C 
                src/Variable.C 
                src/Symbol.C 
                src/LineInformation.C 
                src/Symtab.C 
                src/Symtab-edit.C 
                src/Symtab-lookup.C 
                src/Symtab-deprecated.C 
                src/Module.C 
                src/Region.C 
                src/Collections.C 
                src/Type.C 
                src/AddrLookup.C 
                src/annotations.C 
                src/debug.C 
                src/SymtabReader.C 
  )

if (PLATFORM MATCHES freebsd OR 
    PLATFORM MATCHES linux OR 
    PLATFORM MATCHES bgq OR
    PLATFORM MATCHES cnl)

set (SRC_LIST ${SRC_LIST}
    src/Object-elf.C 
    src/Archive.C 
    src/Archive-elf.C 
    src/parseDwarf.C 
    src/parseStab.C 
    src/emitElf.C 
    src/LinkMap.C 
    src/emitElfStatic.C 
    src/dwarfWalker.C
)

if (PLATFORM MATCHES x86_64 OR PLATFORM MATCHES amd64)
set (SRC_LIST ${SRC_LIST}
    src/emitElf-64.C
    src/emitElfStatic-x86.C 
    src/relocationEntry-elf-x86.C
    )
elseif (PLATFORM MATCHES i386)
set (SRC_LIST ${SRC_LIST}
    src/emitElfStatic-x86.C
    src/relocationEntry-elf-x86.C
    )
elseif (PLATFORM MATCHES ppc32)
set (SRC_LIST ${SRC_LIST}
    src/emitElfStatic-ppc32.C 
    src/relocationEntry-elf-ppc32.C
    )
elseif (PLATFORM MATCHES ppc64)
set (SRC_LIST ${SRC_LIST}
    src/emitElf-64.C
    src/emitElfStatic-ppc64.C
    src/relocationEntry-elf-ppc64.C
    )
endif()
endif()


if (PLATFORM MATCHES nt)
set (SRC_LIST ${SRC_LIST}
    src/Object-nt.C
	src/emitWin.C
	src/relocationEntry-stub.C
)
endif()

SET_SOURCE_FILES_PROPERTIES(${SRC_LIST} PROPERTIES LANGUAGE CXX)

ADD_DEFINITIONS(-DSYMTAB_LIB)
  

add_library (symtabAPI ${SRC_LIST})
add_library (symtabAPI_static STATIC  ${SRC_LIST})
target_link_private_libraries (symtabAPI common)

FILE (GLOB headers "h/*.h")
set_target_properties (symtabAPI symtabAPI_static PROPERTIES PUBLIC_HEADER "${headers}")

if (UNIX)
  target_link_private_libraries (symtabAPI dynElf ${LIBELF_LIBRARIES})
  target_link_private_libraries (symtabAPI dynDwarf ${LIBDWARF_LIBRARIES})
endif()

INSTALL (TARGETS symtabAPI symtabAPI_static
        EXPORT DyninstTargets
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})

