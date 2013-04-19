# CMake configuration for proccontrol directory

include_directories (
    ${PROJECT_SOURCE_DIR}/proccontrol/src
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
     src/dbginfo-stepper.C 
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


add_library (stackwalker ${SRC_LIST})

target_link_libraries (stackwalker pcontrol)

if (UNIX)
target_link_libraries (stackwalker ${LIBELF_LIBRARIES})
target_link_libraries (stackwalker ${LIBDWARF_LIBRARIES})
endif()

if (LIGHTWEIGHT_SYMTAB)
target_link_libraries(stackwalker symLite)
else()
target_link_libraries(stackwalker symtabAPI)
endif()

if (SW_ANALYSIS_STEPPER)
target_link_libraries(stackwalker parseAPI)
endif()

FILE (GLOB headers "h/*.h")
set_target_properties (stackwalker PROPERTIES PUBLIC_HEADER "${headers}")

INSTALL (TARGETS stackwalker
        EXPORT DyninstTargets
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
