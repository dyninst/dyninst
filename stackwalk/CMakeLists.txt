# CMake configuration for proccontrol directory

include_directories (
    ${PROJECT_SOURCE_DIR}/stackwalk/src
  )

set (SRC_LIST
    src/frame.C 
    src/framestepper.C 
    src/swk_errors.C 
    src/symlookup.C 
    src/walker.C 
    src/procstate.C 
    src/steppergroup.C 
    src/libstate.C 
    src/sw_c.C 
    src/sw_pcontrol.C  
)

if (PLATFORM MATCHES freebsd)
set (SRC_LIST ${SRC_LIST}
     src/freebsd-swk.C 
     src/x86-swk.C
     src/symtab-swk.C 
     src/dbginfo-stepper.C 
     src/x86-wanderer.C 
     src/linuxbsd-swk.C
     src/linuxbsd-x86-swk.C 
     src/freebsd-x86-swk.C
)
elseif (PLATFORM MATCHES linux OR PLATFORM MATCHES freebsd)
    set (SRC_LIST ${SRC_LIST}
        src/symtab-swk.C 
        src/linuxbsd-swk.C 
        src/linux-swk.C
    )
    if (PLATFORM MATCHES ppc)
        set (SRC_LIST ${SRC_LIST}
            src/linux-ppc-swk.C 
            src/ppc-swk.C
        )

    elseif (PLATFORM MATCHES i386 OR PLATFORM MATCHES x86_64)
        set (SRC_LIST ${SRC_LIST}
            src/dbginfo-stepper.C 
            src/linux-x86-swk.C 
            src/x86-wanderer.C 
            src/linuxbsd-x86-swk.C 
            src/x86-swk.C 
        )
    endif()
elseif (PLATFORM MATCHES bgq)
    set (SRC_LIST ${SRC_LIST}
        src/ppc-swk.C 
        src/symtab-swk.C 
        src/linuxbsd-swk.C
        src/bluegeneq-swk.C
    )
endif()
if (PLATFORM MATCHES nt OR PLATFORM MATCHES windows)
    set (SRC_LIST ${SRC_LIST}
        src/x86-wanderer.C 
        src/x86-swk.C
        src/win-x86-swk.C
        src/symtab-swk.C
    )
endif()

if (SW_ANALYSIS_STEPPER)
    set (SRC_LIST ${SRC_LIST}
        src/analysis_stepper.C
        src/callchecker-IAPI.C
    )
else ()
    set (SRC_LIST ${SRC_LIST}
        src/callchecker.C
    )
endif()

SET_SOURCE_FILES_PROPERTIES(${SRC_LIST} PROPERTIES LANGUAGE CXX)

ADD_DEFINITIONS(-DSTACKWALKER_EXPORTS)

add_library (stackwalk ${SRC_LIST})
add_library (stackwalk_static STATIC ${SRC_LIST})

target_link_private_libraries (stackwalk common)
target_link_private_libraries (stackwalk instructionAPI)
target_link_private_libraries (stackwalk pcontrol)

if (UNIX)
target_link_private_libraries (stackwalk dynDwarf)
target_link_private_libraries (stackwalk dynElf)
endif()

if (LIGHTWEIGHT_SYMTAB)
target_link_private_libraries(stackwalk symLite)
else()
target_link_private_libraries(stackwalk symtabAPI)
endif()

if (SW_ANALYSIS_STEPPER)
target_link_private_libraries(stackwalk parseAPI)
endif()

FILE (GLOB headers "h/*.h")
set_target_properties (stackwalk stackwalk_static PROPERTIES PUBLIC_HEADER "${headers}")

INSTALL (TARGETS stackwalk stackwalk_static
        EXPORT DyninstTargets
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
