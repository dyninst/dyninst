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
add_library (symLite_static STATIC ${SRC_LIST})
FILE (GLOB headers "h/*.h")
set_target_properties (symLite symLite_static PROPERTIES PUBLIC_HEADER "${headers}")

target_link_private_libraries (symLite common)
target_link_private_libraries (symLite dynElf)

INSTALL (TARGETS symLite symLite_static
        EXPORT DyninstTargets
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})

