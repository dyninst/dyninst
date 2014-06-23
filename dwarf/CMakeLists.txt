# CMake configuration for dynDwarf (dwarf) directory

if (NOT UNIX)
  return()
endif()

include_directories (
  ${PROJECT_SOURCE_DIR}/dwarf/src
  )
add_definitions(-DDYNDWARF_LIB)

set (SRC_LIST
     src/dwarfResult.C
     src/dwarfExprParser.C
     src/dwarfFrameParser.C
     src/dwarfHandle.C
  )

add_library (dynDwarf ${SRC_LIST})
add_library (dynDwarf_static STATIC ${SRC_LIST})
target_link_private_libraries (dynDwarf ${LIBDWARF_LIBRARIES})
target_link_private_libraries (dynDwarf dynElf)
target_link_private_libraries (dynDwarf common)

FILE (GLOB headers "h/*.h")
set_target_properties (dynDwarf dynDwarf_static PROPERTIES PUBLIC_HEADER "${headers}")

INSTALL (TARGETS dynDwarf dynDwarf_static
        EXPORT DyninstTargets
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})

