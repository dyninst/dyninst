# CMake configuration for dynElf (elf) directory

if (NOT UNIX)
  return()
endif()

include_directories (
  src
  h
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

dyninst_library(dynElf ${LIBELF_LIBRARIES})

add_definitions(-DDYNELF_LIB)
if (USE_COTIRE)
    cotire(dynElf)
endif()
