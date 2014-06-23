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
add_library (dynElf_static STATIC ${SRC_LIST})
add_definitions(-DDYNELF_LIB)

FILE (GLOB headers "h/*.h")
set_target_properties (dynElf dynElf_static PROPERTIES PUBLIC_HEADER "${headers}")
target_link_private_libraries (dynElf ${LIBELF_LIBRARIES})

INSTALL (TARGETS dynElf dynElf_static
        EXPORT DyninstTargets
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})

