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

dyninst_library(dynElf ${LibElf_LIBRARIES})
if(TARGET TBB)
    add_dependencies(dynElf TBB)
endif()
add_dependencies(dynElf ElfUtils)
target_link_private_libraries(dynElf ${ElfUtils_LIBRARIES})

if (ENABLE_DEBUGINFOD AND LibDebuginfod_FOUND)
  add_definitions(-DDEBUGINFOD_LIB)
endif()

add_definitions(-DDYNELF_LIB)
