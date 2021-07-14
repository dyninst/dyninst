# CMake configuration for proccontrol directory

include_directories (
  ${PROJECT_SOURCE_DIR}/proccontrol/h
  ${PROJECT_SOURCE_DIR}/elf/h
  ${PROJECT_SOURCE_DIR}/dwarf/h
  src
  h
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

    elseif (PLATFORM MATCHES aarch64)
        set (SRC_LIST ${SRC_LIST}
            src/linux-aarch64-swk.C 
            src/aarch64-swk.C
            src/dbginfo-stepper.C 
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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FORCE_FRAME_POINTER}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FORCE_FRAME_POINTER}")

set(DEPS common instructionAPI pcontrol)
if (UNIX)
set (DEPS ${DEPS} dynDwarf dynElf)
endif()

set (DEPS ${DEPS} ${SYMREADER})

if (SW_ANALYSIS_STEPPER)
set (DEPS ${DEPS} parseAPI)
endif()


dyninst_library(stackwalk ${DEPS})
target_link_private_libraries(stackwalk ${Boost_LIBRARIES})
