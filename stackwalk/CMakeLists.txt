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

INSTALL (DIRECTORY h/ DESTINATION include FILES_MATCHING PATTERN "*.h")
INSTALL (TARGETS stackwalker
        LIBRARY DESTINATION lib)
