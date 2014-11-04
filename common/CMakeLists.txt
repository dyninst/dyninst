# CMake configuration for common directory

if(NOT WIN32)
include_directories (
    ${PROJECT_SOURCE_DIR}/common/h 
    ${PROJECT_SOURCE_DIR}/common/src
  )
else()
include_directories (
    ${PROJECT_SOURCE_DIR}/common/h # don't include common/src; anything from there can still collide with default includes.
	# stupid Windows case-insensitive naming.
  )
endif()


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
    src/MachSyscall.C
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

if (PLATFORM MATCHES nt OR PLATFORM MATCHES windows)
  set (SRC_LIST ${SRC_LIST}
    src/ntKludges.C
    src/timing-nt.C
#    src/dthread-win.C
#	src/dthread.C
    src/addrtranslate-win.C
  )
  add_definitions(-DWIN32 -D_WIN32_WINNT=0x500 -Dsnprintf=_snprintf)
endif()  


SET_SOURCE_FILES_PROPERTIES(${SRC_LIST} PROPERTIES LANGUAGE CXX)

ADD_DEFINITIONS(-DCOMMON_LIB)

add_library (common ${SRC_LIST})
add_library (common_static STATIC ${SRC_LIST})
FILE (GLOB headers "h/*.h")
set_target_properties (common common_static PROPERTIES PUBLIC_HEADER "${headers}")

if (IBERTY_FOUND)
target_link_private_libraries (common ${IBERTY_LIBRARIES})
endif()

if(PLATFORM MATCHES nt OR PLATFORM MATCHES windows)
  target_link_libraries (common Psapi WS2_32 dbghelp)
endif()

INSTALL (TARGETS common common_static
        EXPORT DyninstTargets
	RUNTIME DESTINATION ${INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})
