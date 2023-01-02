# CMake configuration for dynElf (elf) directory

if(NOT DYNINST_OS_UNIX)
  if(NOT TARGET dynElf)
    add_library(dynElf INTERFACE)
  endif()
  return()
endif()

include_directories(src h)

set(SRC_LIST src/Elf_X.C)

dyninst_library(dynElf)
target_link_libraries(dynElf PUBLIC Dyninst::ElfUtils)
target_link_libraries(dynElf PRIVATE Dyninst::Boost_headers)

add_definitions(-DDYNELF_LIB)
