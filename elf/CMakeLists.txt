# CMake configuration for dynElf (elf) directory

if (NOT UNIX)
  return()
endif()

include_directories (
  ${PROJECT_SOURCE_DIR}/elf/src
  )

set (SRC_LIST
     src/Elf_X.C
  )

if (SYSTAP_PARSE)
set (SRC_LIST
     ${SRC_LIST}
     src/SystemTap.C
  )
endif()

add_library (dynElf ${SRC_LIST})
target_link_libraries (dynElf ${LIBELF_LIBRARIES})

INSTALL (DIRECTORY h/ DESTINATION include FILES_MATCHING PATTERN "*.h")

