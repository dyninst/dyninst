# CMake configuration for symlite directory

if (NOT UNIX)
  return()
endif()

include_directories (
  ${PROJECT_SOURCE_DIR}/symlite/src
  )

set (SRC_LIST
     src/SymLite-elf.C
  )

add_library (symLite ${SRC_LIST})
target_link_libraries (symLite ${LIBELF_LIBRARIES})

INSTALL (DIRECTORY h/ DESTINATION include FILES_MATCHING PATTERN "*.h")

