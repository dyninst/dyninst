# CMake configuration for common directory

include_directories (
    ${PROJECT_SOURCE_DIR}/common/h 
    ${PROJECT_SOURCE_DIR}/common/src
  )


set (SRC_LIST
    src/Ident.C
    src/string-regex.C 
    src/Timer.C 
    src/Types.C 
    src/debugOstream.C 
    src/lprintf.C 
    src/pathName.C 
    src/Time.C 
    src/fraction.C 
    src/timing.C 
    src/stats.C 
    src/Annotatable.C 
    src/MappedFile.C 
    src/sha1.C 
    src/serialize-bin.C 
    src/serialize-xml.C 
    src/util.C 
    src/Node.C 
    src/Graph.C 
    src/Edge.C 
    src/DOT.C 
    src/dyn_regs.C 
    src/AST.C 
    src/addrtranslate.C 
    src/arch-x86.C 
    src/arch-power.C 
    src/debug_common.C 
    src/VariableLocation.C 
    src/Buffer.C
  )

if (PLATFORM MATCHES freebsd)
  set (SRC_LIST ${SRC_LIST} 
    src/timing-freebsd.C
    src/freebsdKludges.C
    src/addrtranslate-sysv.C
    src/addrtranslate-freebsd.C
  )
endif()

if (PLATFORM MATCHES linux)
  set (SRC_LIST ${SRC_LIST}
    src/linuxKludges.C
    src/timing-linux.C
    src/parseauxv.C
    src/addrtranslate-sysv.C
    src/addrtranslate-auxv.C
    src/addrtranslate-linux.C
  )
endif()

if (PLATFORM MATCHES bgq)
  set (SRC_LIST ${SRC_LIST}
    src/linuxKludges.C
    src/bluegeneKludges.C
    src/timing-linux.C
    src/parseauxv.C
    src/addrtranslate-sysv.C
    src/addrtranslate-auxv.C
    src/addrtranslate-bluegene.C
  )
endif()

if (PLATFORM MATCHES cnl)
  set (SRC_LIST ${SRC_LIST}
    src/linuxKludges.C
    src/timing-linux.C
    src/parseauxv.C
    src/addrtranslate-sysv.C
    src/addrtranslate-auxv.C
    src/addrtranslate-linux.C
  )
endif()  

add_library (common ${SRC_LIST})

if (${IBERTY_LIBRARY} MATCHES iberty)
target_link_libraries (common ${IBERTY_LIBRARY})
endif()

INSTALL (TARGETS common
        LIBRARY DESTINATION lib)
