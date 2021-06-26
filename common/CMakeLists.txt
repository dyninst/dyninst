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
    src/pfq-rwlock.C
    src/concurrent.C
    src/string-regex.C
    src/Timer.C 
    src/Types.C 
    src/lprintf.C
    src/pathName.C 
    src/fraction.C
    src/stats.C 
    src/Annotatable.C 
    src/MappedFile.C 
    src/sha1.C 
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
	src/arch-aarch64.C
    src/debug_common.C 
    src/VariableLocation.C 
    src/Buffer.C
    src/MachSyscall.C
  )

set (C_SRC_LIST
)

if (PLATFORM MATCHES freebsd)
  set (SRC_LIST ${SRC_LIST} 
    src/freebsdKludges.C
    src/addrtranslate-sysv.C
    src/addrtranslate-freebsd.C
    src/symbolDemangleWithCache.C
  )
  set (C_SRC_LIST ${C_SRC_LIST}
    src/symbolDemangle.c
  )
endif()

if (PLATFORM MATCHES linux)
  set (SRC_LIST ${SRC_LIST}
    src/linuxKludges.C
    src/parseauxv.C
    src/addrtranslate-sysv.C
    src/addrtranslate-auxv.C
    src/addrtranslate-linux.C
    src/symbolDemangleWithCache.C
  )
  set (C_SRC_LIST ${C_SRC_LIST}
    src/symbolDemangle.c
  )
endif()

if (PLATFORM MATCHES cnl)
  set (SRC_LIST ${SRC_LIST}
    src/linuxKludges.C
    src/parseauxv.C
    src/addrtranslate-sysv.C
    src/addrtranslate-auxv.C
    src/addrtranslate-linux.C
  )
endif()  

if (PLATFORM MATCHES nt OR PLATFORM MATCHES windows)
  set (SRC_LIST ${SRC_LIST}
    src/ntKludges.C
#    src/dthread-win.C
#	src/dthread.C
    src/addrtranslate-win.C
  )
  add_definitions(-DWIN32 -D_WIN32_WINNT=0x500)
endif()  


SET_SOURCE_FILES_PROPERTIES(${SRC_LIST} PROPERTIES LANGUAGE CXX)
SET_SOURCE_FILES_PROPERTIES(${C_SRC_LIST} PROPERTIES LANGUAGE C)
set (SRC_LIST ${SRC_LIST}
  ${C_SRC_LIST}
)

ADD_DEFINITIONS(-DCOMMON_LIB)

dyninst_library(common)

if(TARGET LibIberty)
  add_dependencies(common LibIberty)
  target_link_libraries(common PRIVATE LibIberty)

  if(${ENABLE_STATIC_LIBS})
	  target_link_libraries(common_static PRIVATE LibIberty)
  endif()
endif()

if(TARGET TBB)
    add_dependencies(common TBB)
endif()
if(PLATFORM MATCHES nt OR PLATFORM MATCHES windows)
  target_link_private_libraries(common Psapi WS2_32 dbghelp)
endif()
target_link_private_libraries(common ${Boost_LIBRARIES})
target_link_libraries(common PUBLIC ${TBB_LIBRARIES})

if(USE_OpenMP)
  set_target_properties(common PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS} LINK_FLAGS ${OpenMP_CXX_FLAGS})
endif()

