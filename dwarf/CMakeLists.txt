# CMake configuration for dynDwarf (dwarf) directory

if (NOT UNIX)
  return()
endif()

include_directories (
  ${PROJECT_SOURCE_DIR}/dwarf/src
  )

set (SRC_LIST
     src/dwarfResult.C
     src/dwarfExprParser.C
     src/dwarfFrameParser.C
     src/dwarfHandle.C
  )

add_library (dynDwarf ${SRC_LIST})
target_link_libraries (dynDwarf ${LIBDWARF_LIBRARIES})

FILE (GLOB headers "h/*.h")
set_target_properties (dynDwarf PROPERTIES PUBLIC_HEADER "${headers}")

INSTALL (TARGETS dynDwarf
        EXPORT dyninst
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})

