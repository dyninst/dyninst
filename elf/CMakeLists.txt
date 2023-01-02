include_guard(GLOBAL)

include(DyninstLibrary)

if(NOT DYNINST_OS_UNIX)
  if(NOT TARGET dynElf)
    add_library(dynElf INTERFACE)
  endif()
  return()
endif()

set(_public_headers h/Elf_X.h)
set(_sources src/Elf_X.C)

# cmake-format: off
dyninst_library(
  dynElf
  PUBLIC_HEADER_FILES ${_public_headers}
  SOURCE_FILES ${_sources}
  DEFINES DYNELF_LIB
  PUBLIC_DEPS Dyninst::ElfUtils common
  PRIVATE_DEPS Dyninst::Boost_headers
)
# cmake-format: on

foreach(t ${dynElf_TARGETS})
  target_include_directories(
    ${t}
    PUBLIC
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src;${CMAKE_CURRENT_SOURCE_DIR}/h>")
endforeach()
