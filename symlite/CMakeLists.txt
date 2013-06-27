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
add_definitions(-DSYMLITE_LIB)

add_library (symLite ${SRC_LIST})
FILE (GLOB headers "h/*.h")
set_target_properties (symLite PROPERTIES PUBLIC_HEADER "${headers}")

target_link_libraries (symLite ${LIBELF_LIBRARIES})

INSTALL (TARGETS symLite
        EXPORT DyninstTargets
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})

