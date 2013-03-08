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

INSTALL (DIRECTORY h/ DESTINATION include FILES_MATCHING PATTERN "*.h")

